INSERT INTO account_statuses(id,label)
VALUES (1, 'Активный'), (2, 'Неактивный'), (3, 'Системный');

INSERT INTO clients(id,label)
VALUES (1, 'client1'), (2, 'client2'), (3, 'client3'), (4, 'client4');

INSERT INTO accounts(id,balance,is_system,id_client)
VALUES (1, 0.0, 0, 1), (2, 0.0, 0, 2), (3, 0.0, 1, 3), (4, 0.0, 0, 4), (5, 0.0, 0, 4);

INSERT INTO payments(dt, amount, id_account_from, id_account_to)
VALUES ('2022-09-01', 100.0, 1, 2), ('2022-09-02', 200.0, 3, 2), ('2022-09-03', 150.0, 3, 4), ('2022-09-03', 150.0, 4, 5);