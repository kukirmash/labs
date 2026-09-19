-- 1. Справочники и независимые таблицы
CREATE TABLE IF NOT EXISTS roles(
	id SERIAL PRIMARY KEY,
	code TEXT,
	label TEXT
);

INSERT INTO roles(code,label) VALUES('admin', 'Администратор'),('manager', 'Руководитель'),('employee', 'Сотрудник');

CREATE TABLE IF NOT EXISTS clients(
	id SERIAL PRIMARY KEY,
	label TEXT
);

INSERT INTO clients(label) VALUES('Тестовый клиент');

CREATE TABLE IF NOT EXISTS order_statuses(
	id SERIAL PRIMARY KEY,
	label TEXT
);

INSERT INTO order_statuses(id,label) VALUES
(10, 'Проект'),
(20, 'В работе'),
(30, 'Завершён');

CREATE TABLE IF NOT EXISTS payment_types(
	id SERIAL PRIMARY KEY,
	label TEXT
);

INSERT INTO payment_types(id, label) VALUES
(10, 'Аванс'),
(20, 'Основной');

-- НОВАЯ ТАБЛИЦА: Создается до order_items для связи по внешнему ключу
CREATE TABLE IF NOT EXISTS products (
    id SERIAL PRIMARY KEY,
    sku TEXT UNIQUE NOT NULL,
    label TEXT NOT NULL,
    base_price NUMERIC(20,2) NOT NULL
);

-- Тестовые товары (согласованы по ID с MongoDB и Redis)
INSERT INTO products (id, sku, label, base_price) VALUES
(101, 'IPH-15-PRO', 'Смартфон Apple iPhone 15 Pro', 99900.00),
(102, 'TWS-AIR-4', 'Беспроводные наушники Air4', 5500.00);


-- 2. Таблицы с внешними связями (Пользователи и Заказы)
CREATE TABLE IF NOT EXISTS users(
	id SERIAL PRIMARY KEY,
	login TEXT,
	pass TEXT,
	fio TEXT,
	id_role INT REFERENCES roles (id),
	is_blocked INT default 0
);

INSERT INTO users(login, pass,fio,id_role,is_blocked) VALUES
('admin', '098f6bcd4621d373cade4e832627b4f6', 'Тестовый Администратор', 1, 0),
('manager', '098f6bcd4621d373cade4e832627b4f6', 'Тестовый Руководитель', 2, 0),
('employee', '098f6bcd4621d373cade4e832627b4f6', 'Тестовый Сотрудник', 3, 0);

CREATE TABLE IF NOT EXISTS orders(
	id SERIAL PRIMARY KEY,
	label TEXT,
	id_status INT NOT NULL DEFAULT 10,
	id_client INT REFERENCES clients(id),
	amount NUMERIC(20,2)
);

-- Тестовый заказ для демонстрации
INSERT INTO orders(id, label, id_status, id_client, amount) 
VALUES (1, 'Заказ №1', 10, 1, 105400.00);


-- 3. Дочерние таблицы (Позиции заказов и Платежи)
CREATE TABLE IF NOT EXISTS order_items(
	id SERIAL PRIMARY KEY,
	label TEXT,
	id_order INT REFERENCES orders(id),
	id_product INT REFERENCES products(id), -- ИЗМЕНЕНО: Добавлено поле связи с товаром
	amount NUMERIC(20,2)
);

-- Тестовое наполнение позиций заказа
INSERT INTO order_items(label, id_order, id_product, amount) VALUES
('iPhone для клиента', 1, 101, 99900.00),
('Наушники в подарок', 1, 102, 5500.00);

CREATE TABLE IF NOT EXISTS payments(
	id SERIAL PRIMARY KEY,
	id_order INT REFERENCES orders(id), -- Рекомендуется добавить явную связь, если id_order это ID из таблицы orders
	id_payment_type INT REFERENCES payment_types(id),
	amount NUMERIC(20,2)
);

-- Синхронизация счетчиков после ручного добавления ID
SELECT setval('orders_id_seq', (SELECT MAX(id) FROM orders));
SELECT setval('products_id_seq', (SELECT MAX(id) FROM products));
