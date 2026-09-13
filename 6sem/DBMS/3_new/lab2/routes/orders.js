var express = require('express');
var router = express.Router();

// 1. Список всех заказов
router.get('/', async function(req, res, next) {
    let sessionData = session.auth(req);
    let user = sessionData ? sessionData.user : null;

    let orders = await req.db.any(`
        SELECT orders.id AS id, orders.label AS label, order_statuses.label AS order_status_label,
               clients.label AS client_label, orders.amount AS amount
        FROM orders
        INNER JOIN clients ON clients.id = orders.id_client
        INNER JOIN order_statuses ON order_statuses.id = orders.id_status
        ORDER BY orders.id DESC
    `);
    
    let clients = await req.db.any(`SELECT * FROM clients`);
    res.render('orders/list', { title: 'Заказы', orders: orders, clients: clients, user: user });
});

// 2. Создание нового заказа
router.post('/create', async function(req, res, next) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 3)) {
        return res.json({msg: 'У вас нет прав на создание заказа'});
    }
    await req.db.none('INSERT INTO orders(label, id_client, amount) VALUES($1, $2, $3)', 
        [req.body.label, req.body.id_client, req.body.amount]);
    res.json({msg: ''});
});

// 3. Просмотр заказа (Передаем товары для добавления в Dropdown)
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

    let items = await req.db.any('SELECT * FROM order_items WHERE id_order = $1', [id]);
    let payments = await req.db.any(`
        SELECT p.amount, pt.label as type_label FROM payments p 
        JOIN payment_types pt ON p.id_payment_type = pt.id WHERE p.id_order = $1
    `, [id]);
    
    let statuses = await req.db.any('SELECT * FROM order_statuses');
    let payment_types = await req.db.any('SELECT * FROM payment_types');
    
    // Получаем список ВСЕХ товаров для выпадающего списка выбора
    let products = await req.db.any('SELECT * FROM products ORDER BY label');

    let logs = [];
    try {
        const chResult = await req.clickhouse.query({
            query: `SELECT status_id, ts_changed FROM orders_log WHERE order_id = ${parseInt(id)} ORDER BY ts_changed DESC`,
            format: 'JSONEachRow'
        });
        logs = await chResult.json();
    } catch (err) { console.error(err); }

    res.render('orders/view', { 
        title: 'Заказ ' + order.label, order: order, items: items, payments: payments, 
        statuses: statuses, payment_types: payment_types, products: products, logs: logs, user: user 
    });
});

// 4. Добавление товара в состав заказа (Сохраняем связь по id_product)
router.post('/:id/items', async function(req, res) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 3)) {
        return res.json({msg: 'Нет прав на добавление элементов'});
    }
    
    try {
        let productId = parseInt(req.body.id_product);
        // Тянем инфо о выбранном товаре из Postgres
        let product = await req.db.one('SELECT * FROM products WHERE id = $1', [productId]);

        // Вставляем запись с привязкой по id_product
        await req.db.none('INSERT INTO order_items(label, id_order, id_product, amount) VALUES($1, $2, $3, $4)',
            [product.label, req.params.id, product.id, product.base_price]);
            
        res.json({msg: ''});
    } catch (err) {
        res.json({msg: 'Ошибка: ' + err.message});
    }
});

// 5. Изменение статуса заказа (Синхронизация Postgres -> ClickHouse -> Redis)
router.post('/:id/status', async function(req, res) {
    let user = session.auth(req).user;
    let newStatusId = parseInt(req.body.id_status);
    let orderId = parseInt(req.params.id);

    if (!user || (user.id_role !== 1 && user.id_role !== 2)) {
        return res.json({msg: 'Нет прав на изменение статуса'});
    }
    
    // а) Обновляем статус в PostgreSQL
    await req.db.none('UPDATE orders SET id_status = $1 WHERE id = $2', [newStatusId, orderId]);

    // б) Записываем событие лога в ClickHouse
    try {
        await req.clickhouse.insert({
            table: 'orders_log',
            values: [{ order_id: orderId, status_id: newStatusId }],
            format: 'JSONEachRow'
        });
    } catch (err) { console.error("ClickHouse error:", err); }

    // в) СВЯЗЬ С REDIS: Если статус «Завершен» (30) — инкрементируем счетчики проданных товаров!
    if (newStatusId === 30) {
        try {
            // Получаем все товары из этого завершенного заказа
            let orderedItems = await req.db.any('SELECT id_product FROM order_items WHERE id_order = $1', [orderId]);
            
            for (let item of orderedItems) {
                if (item.id_product) {
                    // Принудительно увеличиваем счетчик продаж данного товара в Redis на +1
                    await req.redis.incr(`purchases:${item.id_product}`);
                }
            }
        } catch (err) { console.error("Redis counter increment error:", err); }
    }

    res.json({msg: ''});
});

module.exports = router;