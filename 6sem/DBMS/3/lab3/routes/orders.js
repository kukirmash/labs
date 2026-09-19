var express = require('express');
var router = express.Router();

// Подключаем клиенты ClickHouse и Redis из вашего локального файла конфигурации
const { getClickhouse, getRedis } = require('../database.js');

// 1. Страница со списком всех заказов (Доступна всем авторизованным)
router.get('/', async function(req, res, next) {
    let sessionData = session.auth(req);
    let user = sessionData ? sessionData.user : null;

    let orders = await req.db.any(`
        SELECT
            orders.id AS id,
            orders.label AS label,
            order_statuses.label AS order_status_label,
            clients.label AS client_label,
            orders.amount AS amount
        FROM
            orders
        INNER JOIN clients ON clients.id = orders.id_client
        INNER JOIN order_statuses ON order_statuses.id = orders.id_status
    `);
    
    let clients = await req.db.any(`SELECT * FROM clients`);
    
    res.render('orders/list', { 
        title: 'Заказы', 
        orders: orders, 
        clients: clients,
        user: user 
    });
});

// 2. Создание нового заказа (Админ = 1 или Сотрудник = 3)
router.post('/create', async function(req, res, next) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 3)) {
        return res.json({msg: 'У вас нет прав на создание заказа'});
    }

    await req.db.none(
        'INSERT INTO orders(label, id_client, amount) VALUES($1, $2, $3)', 
        [req.body.label, req.body.id_client, req.body.amount]
    );
    
    res.json({msg: ''});
});

// 3. Просмотр конкретного заказа со всеми дочерними элементами
router.get('/:id', async function(req, res) {
    let id = req.params.id;
    let user = session.auth(req).user;
    if (!user) return res.redirect('/');

    let order = await req.db.one(`
        SELECT orders.id AS id, orders.label AS label, order_statuses.label AS order_status_label,
               clients.label AS client_label, orders.amount AS amount
        FROM orders
        INNER JOIN clients ON clients.id = orders.id_client
        INNER JOIN order_statuses ON order_statuses.id = orders.id_status
        WHERE orders.id = $1
    `, [id]);

    let items = await req.db.any(`
        SELECT oi.*, p.label AS product_label 
        FROM order_items oi 
        LEFT JOIN products p ON p.id = oi.id_product 
        WHERE oi.id_order = $1 
        ORDER BY oi.id
    `, [id]);

    let payments = await req.db.any(`
        SELECT p.amount, pt.label as type_label 
        FROM payments p 
        JOIN payment_types pt ON p.id_payment_type = pt.id 
        WHERE p.id_order = $1
    `, [id]);
    
    let statuses = await req.db.any('SELECT * FROM order_statuses');
    let payment_types = await req.db.any('SELECT * FROM payment_types');
    let products = await req.db.any('SELECT * FROM products ORDER BY id');

    res.render('orders/view', { 
        title: 'Заказ ' + order.label, 
        order: order, 
        items: items, 
        payments: payments, 
        statuses: statuses, 
        payment_types: payment_types, 
        products: products, 
        user: user 
    });
});

// 4. Добавление элемента в заказ
router.post('/:id/items', async function(req, res) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 3)) {
        return res.json({msg: 'Нет прав на добавление элементов'});
    }
    
    let orderId = parseInt(req.params.id, 10);
    let productId = parseInt(req.body.id_product, 10); 
    let amount = req.body.amount ? parseFloat(req.body.amount) : 0;

    try {
        // Записываем в базу id_product. Если товар не выделили, пишем NULL
        await req.db.none(
            'INSERT INTO order_items(label, id_product, id_order, amount) VALUES($1, $2, $3, $4)',
            [req.body.label, isNaN(productId) ? null : productId, orderId, amount]
        );
        res.json({msg: ''});
    } catch (err) {
        console.error('Ошибка добавления позиции заказа в Postgres:', err);
        res.json({msg: 'Не удалось сохранить позицию заказа'});
    }
});

// 5. Изменение статуса заказа с логированием в ClickHouse и инкрементом в Redis
router.post('/:id/status', async function(req, res) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 2)) {
        return res.json({msg: 'Нет прав на изменение статуса'});
    }
    
    let orderId = parseInt(req.params.id, 10);
    let newStatusId = parseInt(req.body.id_status, 10);

    try {
        // Узнаем текущий статус заказа, чтобы проверить факт изменения
        let currentOrder = await req.db.oneOrNone('SELECT id_status FROM orders WHERE id = $1', [orderId]);
        if (!currentOrder) {
            return res.json({msg: 'Заказ не найден'});
        }

        let currentStatusId = parseInt(currentOrder.id_status, 10);

        // Если статус реально меняется на новый
        if (currentStatusId !== newStatusId) {
            
            // 1. Фиксируем изменение в основной базе PostgreSQL
            await req.db.none('UPDATE orders SET id_status = $1 WHERE id = $2', [newStatusId, orderId]);

            // 2. ИНТЕГРАЦИЯ CLICKHOUSE: Форматируем и отправляем лог
            try {
                let clickhouse = getClickhouse();
                
                // Приводим ISO строку к формату ClickHouse: YYYY-MM-DD HH:mm:ss
                const formattedDate = new Date().toISOString().slice(0, 19).replace('T', ' ');

                await clickhouse.insert({
                    table: 'orders_log',
                    values: [
                        {
                            id: Math.floor(Math.random() * 2147483647), // Безопасный ID в пределах Int32/UInt64
                            order_id: Number(orderId),
                            status_id: Number(newStatusId),
                            ts_changed: formattedDate
                        }
                    ],
                    format: 'JSONEachRow'
                });
                console.log('=== ЛОГ УСПЕШНО ЗАПИСАН В CLICKHOUSE ===');
            } catch (chErr) {
                console.error('Критическая ошибка аналитики ClickHouse:', chErr);
            }

            // 3. ИНТЕГРАЦИЯ REDIS: Если новый статус равен 30 ("Завершен")
            if (newStatusId === 30) {
                try {
                    let redis = await getRedis();
                    
                    // Вытаскиваем все id_product, добавленные в этот заказ
                    let items = await req.db.any(
                        'SELECT id_product FROM order_items WHERE id_order = $1 AND id_product IS NOT NULL',
                        [orderId]
                    );
                    
                    // Атомарно увеличиваем счетчики продаж для каждого купленного товара в Redis
                    for (let item of items) {
                        let redisKey = 'product:' + item.id_product + ':sales';
                        await redis.incr(redisKey);
                    }
                } catch (redisErr) {
                    console.error('Критическая ошибка кэша Redis:', redisErr);
                }
            }
            
            res.json({msg: ''});
        } else {
            res.json({msg: 'Статус совпадает с текущим'});
        }

    } catch (err) {
        console.error('Ошибка выполнения смены статуса:', err);
        res.json({msg: 'Внутренняя ошибка сервера при изменении статуса'});
    }
});

module.exports = router;
