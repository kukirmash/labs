var express = require('express');
var router = express.Router();

// 1. Список всех товаров
router.get('/', async function(req, res, next) {
    try {
        let products = await req.db.any('SELECT * FROM products ORDER BY id DESC');
        res.render('products/list', { title: 'Управление товарами', products: products });
    } catch (err) {
        console.error(err);
        res.status(500).send("Ошибка загрузки товаров");
    }
});

// 2. Создание товара (PostgreSQL + MongoDB + Redis)
router.post('/create', async function(req, res, next) {
    try {
        const { sku, label, base_price, brand, category, color } = req.body;

        // а) Сохраняем базу в PostgreSQL и получаем сгенерированный ID
        let newProduct = await req.db.one(
            'INSERT INTO products(sku, label, base_price) VALUES($1, $2, $3) RETURNING id',
            [sku, label, parseFloat(base_price)]
        );
        let productId = newProduct.id;

        // б) Сохраняем сложные JSON-свойства в MongoDB (БЕЗ габаритов и документов)
        await req.mongoDb.collection('products_info').insertOne({
            product_id: productId,
            brand: brand || "Не указан",
            category: category || "Общая",
            color: color || "Не указан"
        });

        // в) Инициализируем счетчик покупок в Redis со значения 0
        await req.redis.set(`purchases:${productId}`, 0);

        res.json({ msg: '' });
    } catch (err) {
        console.error(err);
        res.json({ msg: 'Ошибка при создании товара: ' + err.message });
    }
});

// 3. Просмотр карточки одного товара (PostgreSQL + MongoDB + Redis)
router.get('/:id', async function(req, res, next) {
    let productId = parseInt(req.params.id);
    try {
        let pgProduct = await req.db.oneOrNone('SELECT * FROM products WHERE id = $1', [productId]);
        if (!pgProduct) return res.status(404).send('Товар не найден');

        let mongoInfo = await req.mongoDb.collection('products_info').findOne({ product_id: productId });
        let purchasesCount = await req.redis.get(`purchases:${productId}`);

        res.render('products/view', { 
            title: pgProduct.label, 
            product: pgProduct, 
            details: mongoInfo, 
            purchases: purchasesCount || 0 
        });
    } catch (err) {
        console.error(err);
        res.status(500).send("Ошибка сервера");
    }
});

// 4. Удаление товара (PostgreSQL + MongoDB)
router.post('/delete/:id', async function(req, res) {
    let productId = parseInt(req.params.id);
    try {
        // Удаляем из MongoDB характеристики
        await req.mongoDb.collection('products_info').deleteOne({ product_id: productId });
        
        // Удаляем из PostgreSQL основную запись
        await req.db.none('DELETE FROM products WHERE id = $1', [productId]);
        
        res.json({ msg: '' });
    } catch (err) {
        console.error(err);
        res.json({ msg: 'Нельзя удалить товар, так как он используется в заказах!' });
    }
});

// 5. Изменение статуса заказа (Синхронизация Postgres -> ClickHouse -> Redis)
router.post('/:id/status', async function(req, res) {
    let user = session.auth(req).user;
    let newStatusId = parseInt(req.body.id_status);
    let orderId = parseInt(req.params.id);

    // ВЫВОДИМ В ТЕРМИНАЛ: пришел ли правильный статус
    console.log(`[DEBUG] Запрос на смену статуса заказа #${orderId} на статус ID: ${newStatusId}`);

    if (!user || (user.id_role !== 1 && user.id_role !== 2)) {
        return res.json({msg: 'Нет прав на изменение статуса'});
    }
    
    // а) Обновляем статус в PostgreSQL
    await req.db.none('UPDATE orders SET id_status = $1 WHERE id = $2', [newStatusId, orderId]);
    console.log(`[DEBUG] Успешно обновлено в PostgreSQL`);

    // б) Записываем событие лога в ClickHouse
    try {
        await req.clickhouse.insert({
            table: 'orders_log',
            values: [{ order_id: orderId, status_id: newStatusId }],
            format: 'JSONEachRow'
        });
        console.log(`[DEBUG] Лог успешно записан в ClickHouse!`);
    } catch (err) { 
        console.error("[ОШИБКА ClickHouse]:", err); 
    }

    // в) СВЯЗЬ С REDIS: Если статус «Завершен» (ID 30)
    if (newStatusId === 30) {
        try {
            let orderedItems = await req.db.any('SELECT id_product FROM order_items WHERE id_order = $1', [orderId]);
            console.log(`[DEBUG] В завершенном заказе найдено товаров: ${orderedItems.length}`);
            
            for (let item of orderedItems) {
                if (item.id_product) {
                    await req.redis.incr(`purchases:${item.id_product}`);
                    console.log(`[DEBUG] Увеличен счетчик покупок Redis для товара #${item.id_product}`);
                }
            }
        } catch (err) { 
            console.error("[ОШИБКА Redis]:", err); 
        }
    }

    res.json({msg: ''});
});

module.exports = router;