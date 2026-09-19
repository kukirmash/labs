CREATE TABLE IF NOT EXISTS orders_log (
    id UUID DEFAULT generateUUIDv4(),
    order_id Int32,
    status_id Int32,
    ts_changed DateTime DEFAULT now()
) ENGINE = MergeTree()
ORDER BY ts_changed;

-- Добавим тестовую запись для заказа №1 из PostgreSQL
INSERT INTO orders_log (order_id, status_id, ts_changed) VALUES (1, 10, now());