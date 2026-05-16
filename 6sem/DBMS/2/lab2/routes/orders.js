var express = require('express');
var router = express.Router();

// 1. Страница со списком всех заказов (Доступна всем авторизованным)
router.get('/', async function(req, res, next) {
    // 1. Достаем информацию о текущем пользователе из сессии
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
    
    // 2. ВАЖНО: Передаем переменную user в шаблон вместе с заказами и клиентами!
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

    // БЕЗОПАСНЫЙ ВАРИАНТ: Берем только нужные поля и передаем через $1, $2, $3
    await req.db.none(
        'INSERT INTO orders(label, id_client, amount) VALUES($1, $2, $3)', 
        [req.body.label, req.body.id_client, req.body.amount]
    );
    
    res.json({msg: ''});
});

// 3. Просмотр конкретного заказа с его элементами и платежами
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
        SELECT p.amount, pt.label as type_label 
        FROM payments p 
        JOIN payment_types pt ON p.id_payment_type = pt.id 
        WHERE p.id_order = $1
    `, [id]);
    
    let statuses = await req.db.any('SELECT * FROM order_statuses');
    let payment_types = await req.db.any('SELECT * FROM payment_types');

    res.render('orders/view', { 
        title: 'Заказ ' + order.label, 
        order: order, items: items, payments: payments, 
        statuses: statuses, payment_types: payment_types, user: user 
    });
});

// 4. Добавление элемента в заказ (Админ = 1 или Сотрудник = 3)
router.post('/:id/items', async function(req, res) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 3)) {
        return res.json({msg: 'Нет прав на добавление элементов'});
    }
    
    await req.db.none('INSERT INTO order_items(label, id_order, amount) VALUES($1, $2, $3)',
        [req.body.label, req.params.id, req.body.amount]);
    res.json({msg: ''});
});

// 5. Изменение статуса заказа (Админ = 1 или Руководитель = 2)
router.post('/:id/status', async function(req, res) {
    let user = session.auth(req).user;
    if (!user || (user.id_role !== 1 && user.id_role !== 2)) {
        return res.json({msg: 'Нет прав на изменение статуса'});
    }
    
    await req.db.none('UPDATE orders SET id_status = $1 WHERE id = $2', 
        [req.body.id_status, req.params.id]);
    res.json({msg: ''});
});

module.exports = router;