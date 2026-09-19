var createError = require('http-errors');
var express = require('express');
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');

// --- 1. Подключение PostgreSQL ---
const pgp = require('pg-promise')(/* options */);
const db = pgp('postgres://admin:admin@localhost:5432/lab_db'); // Исправлено на lab_db

// --- 2. Подключение MongoDB ---
const { MongoClient } = require('mongodb');
const mongoClient = new MongoClient('mongodb://admin:admin@localhost:27017');
let mongoDb;
mongoClient.connect().then(() => {
    mongoDb = mongoClient.db('lab_db');
    console.log('✅ Успешно подключено к MongoDB');
}).catch(err => console.error('Ошибка MongoDB:', err));

// --- 3. Подключение ClickHouse ---
const { createClient } = require('@clickhouse/client');
const clickhouse = createClient({
    url: 'http://localhost:8123',
    username: 'admin',
    password: 'admin',
    database: 'lab_db'
});

// --- 4. Подключение Redis ---
const redis = require('redis');
const redisClient = redis.createClient({
    url: 'redis://:adminpassword@localhost:6379'
});
redisClient.connect()
    .then(() => console.log('✅ Успешно подключено к Redis'))
    .catch(err => console.error('Ошибка Redis:', err));


var indexRouter = require('./routes/index');
var usersRouter = require('./routes/users');
var clientsRouter = require('./routes/clients');
var ordersRouter = require('./routes/orders');
var paymentsRouter = require('./routes/payments');
var productsRouter = require('./routes/products');

var app = express();
session = require("./session.js");

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'pug');

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

// Прокидываем ВСЕ базы данных в объект req
app.use(function(req, res, next){
    req.db = db;                  // PostgreSQL
    req.mongoDb = mongoDb;        // MongoDB
    req.clickhouse = clickhouse;  // ClickHouse
    req.redis = redisClient;      // Redis
    next();
});

app.use('/', indexRouter);
app.use('/users', usersRouter);
app.use('/clients', clientsRouter);
app.use('/orders', ordersRouter);
app.use('/payments', paymentsRouter);
app.use('/products', productsRouter);

let api = require('./routes/api');
app.use('/api', api);
let api_auth = require('./routes/api/auth');
api.use('/auth', api_auth);
let api_users = require('./routes/api/users');
api.use('/users', api_users);



// catch 404 and forward to error handler
app.use(function(req, res, next) {
  next(createError(404));
});

// error handler
app.use(function(err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});

module.exports = app;
