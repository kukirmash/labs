-- 1. Сначала удаляем дочерние таблицы и таблицы с внешними связями
DROP TABLE IF EXISTS payments;
DROP TABLE IF EXISTS order_items; -- Удаляем перед таблицей products, так как тут лежит внешний ключ

-- 2. Удаляем таблицы, на которые раньше ссылались дочерние
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS products;     -- НОВАЯ СТРОКА: Удаляем таблицу товаров

-- 3. Удаляем справочники и независимые таблицы
DROP TABLE IF EXISTS payment_types;
DROP TABLE IF EXISTS order_statuses;
DROP TABLE IF EXISTS clients;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS roles;
