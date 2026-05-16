var express = require('express');
var router = express.Router();

router.get('/', async function(req, res, next) {

    let payments = await req.db.any(`
        SELECT
            payments.id AS id,
            orders.label AS order_label,
            payment_types.label AS payment_type_label,
            payments.amount AS amount
        FROM
            payments
        INNER JOIN
            payment_types ON payment_types.id = payments.id_payment_type
        INNER JOIN
            orders ON orders.id = payments.id_order
    `)
    console.log(payments)
    res.render('payments/list', { title: 'Платежи', payments: payments })

});

// Добавление платежа (Руководитель или Админ)
router.post('/create', async function(req, res) {
    let user = session.auth(req).user;
    // Разрешаем только Администратору (1) и Руководителю (2)
    if (!user || (user.id_role !== 1 && user.id_role !== 2)) {
        return res.json({msg: 'Нет прав на проведение оплаты'});
    }

    await req.db.none('INSERT INTO payments(id_order, id_payment_type, amount) VALUES($1, $2, $3)',
        [req.body.id_order, req.body.id_payment_type, req.body.amount]);
    res.json({msg: ''});
});

module.exports = router;
