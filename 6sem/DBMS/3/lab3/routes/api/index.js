let express = require('express');
let router = express.Router();

const { getMongoDb, getRedis, getClickhouse } = require('../database.js');

// ==========================================
// 1. МАРШРУТЫ ДЛЯ MONGODB И REDIS (КЭШИРОВАНИЕ ХАРАКТЕРИСТИК)
// ==========================================

// Получить детальную информацию о товаре (сначала из кэша Redis, если нет — из Mongo)
router.get('/products/:id/details', async function(req, res, next) {
	let productId = parseInt(req.params.id, 10);
	if (isNaN(productId)) {
		res.status(400).json({ msg: 'Invalid product id' });
		return;
	}

	try {
		let redis = await getRedis();
		// Используем ваш паттерн ключа cache:product:{id}
		let cacheKey = 'cache:product:' + productId;
		let cached = await redis.get(cacheKey);
		
		if (cached) {
			res.json({ source: 'redis', data: JSON.parse(cached) });
			return;
		}

		let mongoDb = await getMongoDb();
		// Ищем по _id, так как мы договорились, что _id в Mongo равен id из Postgres
		let doc = await mongoDb.collection('product_details').findOne({ _id: productId });
		if (!doc) {
			res.status(404).json({ msg: 'Not found' });
			return;
		}

		// Сохраняем весь документ в кэш на 1 час
		await redis.set(cacheKey, JSON.stringify(doc), { EX: 3600 });
		res.json({ source: 'mongo', data: doc });
	} catch (err) {
		next(err);
	}
});

// Сохранить/обновить характеристики товара в Mongo и инвалидировать кэш в Redis
router.post('/products/:id/details', async function(req, res, next) {
	let productId = parseInt(req.params.id, 10);
	if (isNaN(productId)) {
		res.status(400).json({ msg: 'Invalid product id' });
		return;
	}

	let details = req.body;
	if (!details || typeof details !== 'object') {
		res.status(400).json({ msg: 'Invalid details' });
		return;
	}

	try {
		let mongoDb = await getMongoDb();
		// Сохраняем в коллекцию, используя productId как основной ключ _id
		await mongoDb.collection('product_details').updateOne(
			{ _id: productId },
			{ $set: details },
			{ upsert: true }
		);

		let redis = await getRedis();
		let cacheKey = 'cache:product:' + productId;
		// Обновляем кэш в Redis
		await redis.set(cacheKey, JSON.stringify({ _id: productId, ...details }), { EX: 3600 });
		res.json({ msg: 'ok' });
	} catch (err) {
		next(err);
	}
});

// ==========================================
// 2. МАРШРУТЫ ДЛЯ REDIS (СТАТИСТИКА И ПОКУПКИ)
// ==========================================

// Получить счетчик покупок товара из Redis
router.get('/products/:id/stats', async function(req, res, next) {
	let productId = parseInt(req.params.id, 10);
	if (isNaN(productId)) {
		res.status(400).json({ msg: 'Invalid product id' });
		return;
	}

	try {
		let redis = await getRedis();
		// Используем ваш паттерн ключа product:{id}:sales
		let key = 'product:' + productId + ':sales';
		let value = await redis.get(key);
		res.json({ product_id: productId, purchases: parseInt(value || '0', 10) });
	} catch (err) {
		next(err);
	}
});

// Имитация покупки товара (увеличиваем счетчик в Redis)
router.post('/products/:id/purchase', async function(req, res, next) {
	let productId = parseInt(req.params.id, 10);
	if (isNaN(productId)) {
		res.status(400).json({ msg: 'Invalid product id' });
		return;
	}

	try {
		let redis = await getRedis();
		let key = 'product:' + productId + ':sales';
		// Атомарно увеличиваем счетчик на 1
		let newValue = await redis.incr(key);
		res.json({ msg: 'ok', purchases: newValue });
	} catch (err) {
		next(err);
	}
});

// ==========================================
// 3. МАРШРУТЫ ДЛЯ CLICKHOUSE (ЛОГИРОВАНИЕ СТАТУСОВ ЗАКАЗОВ)
// ==========================================

// Записать событие изменения статуса заказа в ClickHouse
router.post('/orders/:id/log-status', async function(req, res, next) {
	let orderId = parseInt(req.params.id, 10);
	let statusId = parseInt(req.body.status_id, 10);

	if (isNaN(orderId) || isNaN(statusId)) {
		res.status(400).json({ msg: 'Invalid order_id or status_id' });
		return;
	}

	try {
		let ch = getClickhouse();
		
		// Генерируем случайный ID для лога и текущее время
		const logRow = {
			id: Math.floor(Math.random() * 1000000), 
			order_id: orderId,
			status_id: statusId,
			ts_changed: new Date().toISOString()
		};

		// Выполняем вставку в таблицу orders_log
		await ch.insert({
			table: 'orders_log',
			values: [logRow],
			format: 'JSONEachRow'
		});

		res.json({ msg: 'Status change logged to ClickHouse', data: logRow });
	} catch (err) {
		next(err);
	}
});

module.exports = router;
