-- 1. Список клиентов, являющихся плательщиками
SELECT DISTINCT 
    c.label AS payers
FROM payments AS p
INNER JOIN accounts AS a 
    ON p.id_account_from = a.id
INNER JOIN clients AS c 
    ON a.id_client = c.id;

-- 2. Список клиентов, являющихся получателями
SELECT DISTINCT 
    c.label AS receivers
FROM payments AS p
INNER JOIN accounts AS a 
    ON p.id_account_to = a.id
INNER JOIN clients AS c 
    ON a.id_client = c.id;

-- 3. id платежа с самой большой суммой
SELECT 
    id AS max_amount_payment_id
FROM payments
ORDER BY amount DESC
LIMIT 1;

-- 4. id трёх последних по дате платежей
SELECT 
    id AS last_3_payments_ids
FROM payments
ORDER BY dt DESC
LIMIT 3;

-- 5. Общая сумма всех проведённых платежей
SELECT 
    SUM(amount) AS total_payments_amount
FROM payments;

-- 6. Количество исходящих платежей по каждому счёту
SELECT 
    a.id AS account_id, 
    COUNT(p.id) AS outgoing_payments_count
FROM accounts AS a
LEFT JOIN payments AS p 
    ON a.id = p.id_account_from
GROUP BY a.id
ORDER BY a.id;

-- 7. Количество входящих платежей по каждому счёту
SELECT 
    a.id AS account_id, 
    COUNT(p.id) AS incoming_payments_count
FROM accounts AS a
LEFT JOIN payments AS p 
    ON a.id = p.id_account_to
GROUP BY a.id
ORDER BY a.id;

-- 8. Все платежи с подробной информацией
SELECT 
    p.dt AS payment_date,
    p.amount AS payment_amount,
    p.id_account_to AS receiver_account_id,
    c_to.label AS receiver_client_name,
    p.id_account_from AS payer_account_id,
    c_from.label AS payer_client_name
FROM payments AS p
INNER JOIN accounts AS a_from 
    ON p.id_account_from = a_from.id
INNER JOIN clients AS c_from 
    ON a_from.id_client = c_from.id
INNER JOIN accounts AS a_to 
    ON p.id_account_to = a_to.id
INNER JOIN clients AS c_to 
    ON a_to.id_client = c_to.id
ORDER BY p.dt DESC;