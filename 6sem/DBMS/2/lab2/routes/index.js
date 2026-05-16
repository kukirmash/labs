var express = require('express');
var router = express.Router();

router.get('/', function(req, res, next) {
    let sessionData = session.auth(req);
    let user = sessionData ? sessionData.user : null;
    
    // Настраиваем права доступа в зависимости от id_role
    let can_view_users = false;
    let can_view_orders = false;
    let can_view_payments = false;
    let can_view_clients = false;

    if (user) {
        can_view_clients = true; // Клиентов видят все авторизованные
        
        if (user.id_role === 1) { // Админ может всё
            can_view_users = true;
            can_view_orders = true;
            can_view_payments = true;
        } else if (user.id_role === 2) { // Менеджер работает с заказами и платежами
            can_view_orders = true;
            can_view_payments = true;
        } else if (user.id_role === 3) { // Сотрудник работает только с заказами
            can_view_orders = true;
        }
    }

    res.render('index', {
        title: "Главная страница",
        user: user,
        can_view_users: can_view_users,
        can_view_clients: can_view_clients,
        can_view_orders: can_view_orders,
        can_view_payments: can_view_payments
    });
});

module.exports = router;