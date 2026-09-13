var express = require('express');
var router = express.Router();

const { getClickhouse, getRedis } = require('../database.js');

// Главная страница статистики по товарам и логам заказов
router.get('/', async function(req, res, next) {
    // Используем вашу стандартную проверку сессии
    let sessionData = session.auth(req);
    let user = sessionData ? sessionData.user : null;
    
    // Если пользователь не авторизован, перенаправляем на главную
    if (!user) {
        return res.redirect('/');
    }

    try {
        // 1. ПОЛУЧЕНИЕ ДАННЫХ ИЗ POSTGRESQL
        // Загружаем список товаров и справочник статусов
        let products = await req.db.any('SELECT id, label FROM products ORDER BY id');
        let statuses = await req.db.any('SELECT id, label FROM order_statuses ORDER BY id');
        
        // Создаем карту (map) для быстрого сопоставления ID статуса с его текстовым названием
        let statusMap = {};
        for (let s of statuses) {
            statusMap[s.id] = s.label;
        }

        // 2. ИНТЕГРАЦИЯ REDIS (Статистика продаж в реальном времени)
        let redisStats = [];
        try {
            let redis = await getRedis();
            // Генерируем массив ваших ключей 'product:{id}:sales'
            let keys = products.map((p) => 'product:' + p.id + ':sales');
            
            // Параллельно запрашиваем значения всех ключей из Redis
            let values = await Promise.all(keys.map((key) => redis.get(key)));
            
            // Сопоставляем информацию о товарах с полученными счетчиками
            redisStats = products.map((p, idx) => ({
                id: p.id,
                label: p.label,
                purchases: parseInt(values[idx] || '0', 10)
            }));
        } catch (redisErr) {
            console.error('Ошибка получения статистики из Redis:', redisErr);
            // Если Redis недоступен, выводим товары со счетчиком 0
            redisStats = products.map(p => ({ id: p.id, label: p.label, purchases: 0 }));
        }

        // 3. ИНТЕГРАЦИЯ CLICKHOUSE (История изменения статусов для аналитики)
        let clickhouseStats = [];
        try {
            let clickhouse = getClickhouse();
            
            // Делаем аналитический запрос к логам
            let result = await clickhouse.query({
                query: 'SELECT order_id, status_id, ts_changed FROM orders_log ORDER BY ts_changed DESC',
                format: 'JSONEachRow'
            });
            
            // Официальный @clickhouse/client парсит JSON через метод .json()
            let rows = await result.json();
            
            // Форматируем строки для передачи в шаблон
            clickhouseStats = rows.map((row) => ({
                order_id: Number(row.order_id),
                status_id: Number(row.status_id),
                // Подставляем текстовый статус из Postgres, либо пишем дефолтный текст
                status_label: statusMap[row.status_id] || ('Статус ' + row.status_id),
                ts_changed: row.ts_changed
            }));
        } catch (chErr) {
            console.error('Ошибка получения логов из ClickHouse:', chErr);
        }

        // 4. РЕНДЕРИНГ ШАБЛОНА
        // Передаем собранные данные из всех 3-х СУБД в Pug-шаблон
        res.render('stats/index', {
            title: 'Статистика и Аналитика',
            redisStats: redisStats,
            clickhouseStats: clickhouseStats,
            user: user
        });

    } catch (err) {
        next(err);
    }
});

module.exports = router;
