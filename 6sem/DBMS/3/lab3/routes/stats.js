var express = require('express');
var router = express.Router();

const { getClickhouse, getRedis } = require('../database.js');

// Главная страница общей статистики систем
router.get('/', async function(req, res, next) {
    // Используем вашу стандартную проверку сессии
    let sessionData = session.auth(req);
    let user = sessionData ? sessionData.user : null;
    
    // Если пользователь не авторизован, отправляем на главную
    if (!user) {
        return res.redirect('/');
    }

    try {
        // 1. ДАННЫЕ ИЗ POSTGRESQL (Справочники для сопоставления)
        let products = await req.db.any('SELECT id, label FROM products ORDER BY id');
        let statuses = await req.db.any('SELECT id, label FROM order_statuses ORDER BY id');
        
        // Карта статусов для подстановки текстовых названий вместо ID
        let statusMap = {};
        for (let s of statuses) {
            statusMap[s.id] = s.label;
        }

        // 2. ДАННЫЕ ИЗ REDIS (Счетчики завершенных покупок товаров)
        let redisStats = [];
        try {
            let redis = await getRedis();
            // Используем ваш паттерн ключа 'product:{id}:sales'
            let keys = products.map((p) => 'product:' + p.id + ':sales');
            
            // Запрашиваем значения всех счетчиков одновременно
            let values = await Promise.all(keys.map((key) => redis.get(key)));
            
            redisStats = products.map((p, idx) => ({
                id: p.id,
                label: p.label,
                purchases: parseInt(values[idx] || '0', 10)
            }));
        } catch (redisErr) {
            console.error('Ошибка Redis в stats.js:', redisErr);
            redisStats = products.map(p => ({ id: p.id, label: p.label, purchases: 0 }));
        }

        // 3. ДАННЫЕ ИЗ CLICKHOUSE (Лента логов изменения статусов заказов)
        let clickhouseStats = [];
        try {
            let clickhouse = getClickhouse();
            
            // Выполняем аналитическую выборку истории
            let result = await clickhouse.query({
                query: 'SELECT order_id, status_id, ts_changed FROM orders_log ORDER BY ts_changed DESC',
                format: 'JSONEachRow'
            });
            
            // Читаем JSON ответ через метод официального клиента
            let rows = await result.json();
            
            clickhouseStats = rows.map((row) => ({
                order_id: Number(row.order_id),
                status_id: Number(row.status_id),
                status_label: statusMap[row.status_id] || ('Статус ' + row.status_id),
                ts_changed: row.ts_changed
            }));
        } catch (chErr) {
            console.error('Ошибка ClickHouse в stats.js:', chErr);
        }

        // 4. ОТПРАВКА В ШАБЛОНИЗАТОР PUG
        res.render('stats/index', {
            title: 'Статистика',
            redisStats: redisStats,
            clickhouseStats: clickhouseStats,
            user: user
        });

    } catch (err) {
        next(err);
    }
});

module.exports = router;
