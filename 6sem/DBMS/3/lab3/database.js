const { MongoClient } = require('mongodb');
const { createClient: createRedisClient } = require('redis');
const { createClient: createClickhouseClient } = require('@clickhouse/client');

// --- НАСТРОЙКИ MONGODB ---
// Авторизация admin:admin и база данных lab_db из docker-compose
const mongoUrl = 'mongodb://admin:admin@localhost:27017';
const mongoDbName = 'lab_db'; 
let mongoDbPromise;

async function getMongoDb() {
  if (!mongoDbPromise) {
    const client = new MongoClient(mongoUrl);
    mongoDbPromise = client.connect().then((connected) => connected.db(mongoDbName));
  }
  return mongoDbPromise;
}

// --- НАСТРОЙКИ REDIS ---
// Авторизация с паролем adminpassword из команды запуска redis-server
const redisUrl = 'redis://:adminpassword@localhost:6379';
const redisClient = createRedisClient({ url: redisUrl });
let redisReady = false;

redisClient.on('error', (err) => {
  console.error('Redis error:', err);
});

async function getRedis() {
  if (!redisReady) {
    await redisClient.connect();
    redisReady = true;
  }
  return redisClient;
}

// --- НАСТРОЙКИ CLICKHOUSE ---
// Авторизация admin:admin и база данных lab_db из docker-compose
const clickhouseClient = createClickhouseClient({
  url: 'http://localhost:8123',
  username: 'admin',
  password: 'admin',
  database: 'lab_db'
});

function getClickhouse() {
  return clickhouseClient;
}

module.exports = {
  getMongoDb,
  getRedis,
  getClickhouse
};
