#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Сбор данных DummyJSON, формирование Postman-коллекции и окружения."""

import json
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlencode, quote

import requests

ROOT = Path(__file__).resolve().parent
BASE = "https://dummyjson.com"
COLLECTION_PATH = ROOT / "postman" / "ТРПО_Практика_1_DummyJSON_v2.postman_collection.json"
ENVIRONMENT_PATH = ROOT / "postman" / "ТРПО_Группа_Фамилия_v2.postman_environment.json"
DATA_DIR = ROOT / "data"
RESP_DIR = DATA_DIR / "responses"

REQUESTS = [
    {
        "key": "test",
        "folder": "01. Тестовый маршрут",
        "name": "01.01. Проверка доступности API — GET /test",
        "method": "GET",
        "path": "/test",
        "query": [],
        "status": 200,
        "task": "Тестовый маршрут. Проверьте, работает ли ваш интернет.",
        "analysis": "Сервис доступен, возвращает статус ok и метод GET.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("status = ok", function () { pm.expect(j.status).to.eql("ok"); });',
            'pm.test("method = GET", function () { pm.expect(j.method).to.eql("GET"); });',
        ],
    },
    {
        "key": "users",
        "folder": "02. Основные сущности",
        "name": "02.01. Список пользователей — GET /users",
        "method": "GET",
        "path": "/users",
        "query": [],
        "status": 200,
        "task": "Получить список пользователей.",
        "analysis": "По умолчанию возвращаются 30 пользователей из 208; в ответе есть total, skip и limit.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 208", function () { pm.expect(j.total).to.eql(208); });',
            'pm.test("limit = 30", function () { pm.expect(j.limit).to.eql(30); });',
            'pm.test("users.length = 30", function () { pm.expect(j.users.length).to.eql(30); });',
        ],
    },
    {
        "key": "products",
        "folder": "02. Основные сущности",
        "name": "02.02. Список продуктов — GET /products",
        "method": "GET",
        "path": "/products",
        "query": [],
        "status": 200,
        "task": "Получить список продуктов.",
        "analysis": "Возвращаются 30 товаров из 194; структура ответа аналогична списку пользователей.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 194", function () { pm.expect(j.total).to.eql(194); });',
            'pm.test("products.length = 30", function () { pm.expect(j.products.length).to.eql(30); });',
        ],
    },
    {
        "key": "users_select",
        "folder": "03. Выборка полей (select)",
        "name": "03.01. Пользователи: имя, пол, возраст — GET /users?select=...",
        "method": "GET",
        "path": "/users",
        "query": [("select", "firstName,gender,age"), ("limit", "0")],
        "status": 200,
        "task": "Выбрать пользователей с именем, полом и возрастом.",
        "analysis": "Параметр select оставляет только запрошенные поля (id добавляется автоматически); получены все 208 пользователей.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("users.length = 208", function () { pm.expect(j.users.length).to.eql(208); });',
            'pm.test("Только выбранные поля", function () {',
            '  const keys = Object.keys(j.users[0]).sort();',
            '  pm.expect(keys).to.eql(["age", "firstName", "gender", "id"]);',
            '});',
        ],
    },
    {
        "key": "posts_select_tags",
        "folder": "03. Выборка полей (select)",
        "name": "03.02. Посты: только теги — GET /posts?select=tags",
        "method": "GET",
        "path": "/posts",
        "query": [("select", "tags"), ("limit", "0")],
        "status": 200,
        "task": "Выбрать все посты, оставив только теги.",
        "analysis": "Получены все 251 пост; в каждом объекте остались только id и tags.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("posts.length = 251", function () { pm.expect(j.posts.length).to.eql(251); });',
            'pm.test("Только id и tags", function () {',
            '  pm.expect(Object.keys(j.posts[0]).sort()).to.eql(["id", "tags"]);',
            '  pm.expect(Array.isArray(j.posts[0].tags)).to.be.true;',
            '});',
        ],
    },
    {
        "key": "user_3",
        "folder": "04. Выборка по идентификатору",
        "name": "04.01. Пользователь с id = 3 — GET /users/3",
        "method": "GET",
        "path": "/users/3",
        "query": [],
        "status": 200,
        "task": "Выбрать пользователя с идентификатором 3.",
        "analysis": "Получена запись Sophia Brown (id = 3, возраст 43).",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("id = 3", function () { pm.expect(j.id).to.eql(3); });',
            'pm.test("firstName = Sophia", function () { pm.expect(j.firstName).to.eql("Sophia"); });',
            'pm.test("lastName = Brown", function () { pm.expect(j.lastName).to.eql("Brown"); });',
        ],
    },
    {
        "key": "filter_sophia",
        "folder": "05. Фильтрация (filter)",
        "name": "05.01. Пользователь Sophia — GET /users/filter",
        "method": "GET",
        "path": "/users/filter",
        "query": [("key", "firstName"), ("value", "Sophia")],
        "status": 200,
        "task": "Получить информацию о пользователе Sophia.",
        "analysis": "Найдены две записи с именем Sophia: Sophia Brown (id 3) и Sophia Jones (id 109).",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 2", function () { pm.expect(j.total).to.eql(2); });',
            'pm.test("Все записи — Sophia", function () {',
            '  pm.expect(j.users.every(u => u.firstName === "Sophia")).to.be.true;',
            '});',
        ],
    },
    {
        "key": "filter_brown_hair",
        "folder": "05. Фильтрация (filter)",
        "name": "05.02. Шатены — GET /users/filter?key=hair.color",
        "method": "GET",
        "path": "/users/filter",
        "query": [("key", "hair.color"), ("value", "Brown")],
        "status": 200,
        "task": "Найти шатенов среди пользователей.",
        "analysis": "Найдено 23 пользователя с цветом волос Brown; фильтр поддерживает вложенные поля через точку.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 23", function () { pm.expect(j.total).to.eql(23); });',
            'pm.test("У всех волосы Brown", function () {',
            '  pm.expect(j.users.every(u => u.hair && u.hair.color === "Brown")).to.be.true;',
            '});',
        ],
    },
    {
        "key": "search_sophia",
        "folder": "06. Поиск (search)",
        "name": "06.01. Поиск Sophia — GET /users/search?q=Sophia",
        "method": "GET",
        "path": "/users/search",
        "query": [("q", "Sophia")],
        "status": 200,
        "task": "Найти пользователя по имени Sophia.",
        "analysis": "Поиск по строке находит двух пользователей: Sophia Brown и Sophia Jones.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 2", function () { pm.expect(j.total).to.eql(2); });',
            'pm.test("Имя содержит Sophia", function () {',
            '  pm.expect(j.users.every(u => u.firstName.includes("Sophia"))).to.be.true;',
            '});',
        ],
    },
    {
        "key": "sort_age_asc",
        "folder": "07. Сортировка (sortBy)",
        "name": "07.01. Сортировка по возрасту — GET /users?sortBy=age&order=asc",
        "method": "GET",
        "path": "/users",
        "query": [("sortBy", "age"), ("order", "asc"), ("limit", "0")],
        "status": 200,
        "task": "Отсортировать пользователей по возрасту.",
        "analysis": "Все 208 пользователей упорядочены по возрастанию возраста: от 23 лет (Olivia Wilson) до 46 лет (James Davis).",
        "tests": [
            'const j = pm.response.json();',
            'const ages = j.users.map(u => u.age);',
            'pm.test("208 пользователей", function () { pm.expect(j.users.length).to.eql(208); });',
            'pm.test("Возраст по возрастанию", function () {',
            '  pm.expect(ages).to.eql([...ages].sort((a, b) => a - b));',
            '});',
            'pm.test("Минимальный возраст = 23", function () { pm.expect(ages[0]).to.eql(23); });',
            'pm.test("Максимальный возраст = 46", function () { pm.expect(ages[ages.length - 1]).to.eql(46); });',
        ],
    },
    {
        "key": "page_10_skip_10",
        "folder": "08. Пагинация (limit, skip)",
        "name": "08.01. Страница 10 + 10 — GET /users?limit=10&skip=10",
        "method": "GET",
        "path": "/users",
        "query": [("limit", "10"), ("skip", "10")],
        "status": 200,
        "task": "Пагинация данных с параметрами top и skip.",
        "analysis": "Возвращены 10 записей, начиная с 11-й (id = 11); skip = 10, limit = 10.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("users.length = 10", function () { pm.expect(j.users.length).to.eql(10); });',
            'pm.test("skip = 10", function () { pm.expect(j.skip).to.eql(10); });',
            'pm.test("Первый id = 11", function () { pm.expect(j.users[0].id).to.eql(11); });',
        ],
    },
    {
        "key": "limit_all",
        "folder": "08. Пагинация (limit, skip)",
        "name": "08.02. Все 208 пользователей — GET /users?limit=0",
        "method": "GET",
        "path": "/users",
        "query": [("limit", "0")],
        "status": 200,
        "task": "Исследуйте работу параметра limit. Например, получить список всех (208) пользователей.",
        "analysis": "limit=0 отключает ограничение: возвращаются все 208 пользователей; сервер нормализует значение limit до 208.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("users.length = 208", function () { pm.expect(j.users.length).to.eql(208); });',
            'pm.test("limit нормализован до 208", function () { pm.expect(j.limit).to.eql(208); });',
        ],
    },
    {
        "key": "todos_user1",
        "folder": "09. Вложенные ресурсы",
        "name": "09.01. Задачи пользователя 1 — GET /users/1/todos",
        "method": "GET",
        "path": "/todos/user/1",
        "query": [],
        "status": 200,
        "task": "Получить список всех задач пользователя 1.",
        "analysis": "У пользователя 1 две задачи: Learn Javascript (не выполнена) и Listen to a new music genre (выполнена).",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 2", function () { pm.expect(j.total).to.eql(2); });',
            'pm.test("Все задачи пользователя 1", function () {',
            '  pm.expect(j.todos.every(t => t.userId === 1)).to.be.true;',
            '});',
        ],
    },
    {
        "key": "posts_user7",
        "folder": "09. Вложенные ресурсы",
        "name": "09.02. Посты пользователя 7 — GET /users/7/posts",
        "method": "GET",
        "path": "/posts/user/7",
        "query": [],
        "status": 200,
        "task": "Получить список публикаций (постов) пользователя 7.",
        "analysis": "У Alexander Jones (id 7) два поста: id 121 «Revolution and youth are closely allied.» и id 227 «The historic castle loomed over the town below».",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 2", function () { pm.expect(j.total).to.eql(2); });',
            'pm.test("id постов = 121 и 227", function () {',
            '  pm.expect(j.posts.map(p => p.id)).to.eql([121, 227]);',
            '});',
        ],
    },
    {
        "key": "post_ids_user7",
        "folder": "09. Вложенные ресурсы",
        "name": "09.03. Только id постов пользователя 7 — GET /users/7/posts?select=id",
        "method": "GET",
        "path": "/posts/user/7",
        "query": [("select", "id")],
        "status": 200,
        "task": "Получить список идентификаторов публикаций пользователя 7.",
        "analysis": "В ответе остаются только идентификаторы: 121 и 227.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("id = [121, 227]", function () {',
            '  pm.expect(j.posts.map(p => p.id)).to.eql([121, 227]);',
            '});',
            'pm.test("Только поле id", function () {',
            '  pm.expect(Object.keys(j.posts[0])).to.eql(["id"]);',
            '});',
        ],
    },
    {
        "key": "comments_post227",
        "folder": "09. Вложенные ресурсы",
        "name": "09.04. Комментарии к посту 227 — GET /posts/227/comments",
        "method": "GET",
        "path": "/comments/post/227",
        "query": [],
        "status": 200,
        "task": "Alexander Jones разместил пост «Над городом возвышался исторический замок». Найдите комментарии к этому посту.",
        "analysis": "К посту 227 два комментария: «Doesn't it feel good to do such great work?» (Nora Russell) и «You shake my world.» (Connor Baker).",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 2", function () { pm.expect(j.total).to.eql(2); });',
            'pm.test("Комментарий 1", function () {',
            '  pm.expect(j.comments.map(c => c.body).join(" ")).to.include("Doesn\'t it feel good");',
            '});',
            'pm.test("Комментарий 2", function () {',
            '  pm.expect(j.comments.map(c => c.body).join(" ")).to.include("You shake my world");',
            '});',
        ],
    },
    {
        "key": "comments_all",
        "folder": "09. Вложенные ресурсы",
        "name": "09.05. Все комментарии и фильтр по user.id = 3 — GET /comments?limit=0",
        "method": "GET",
        "path": "/comments",
        "query": [("limit", "0")],
        "status": 200,
        "task": "Найдите все комментарии пользователя 3.",
        "analysis": "Получены все 340 комментариев; после фильтрации по user.id = 3 остаётся один комментарий к посту 222: «When I hear your voice, my morning is all my day.»",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 340", function () { pm.expect(j.total).to.eql(340); });',
            'pm.test("У пользователя 3 один комментарий", function () {',
            '  const c = j.comments.filter(x => x.user && x.user.id === 3);',
            '  pm.expect(c.length).to.eql(1);',
            '  pm.expect(c[0].postId).to.eql(222);',
            '});',
        ],
    },
    {
        "key": "comments_post4",
        "folder": "09. Вложенные ресурсы",
        "name": "09.06. Комментарии к посту 4 — GET /posts/4/comments",
        "method": "GET",
        "path": "/comments/post/4",
        "query": [],
        "status": 200,
        "task": "Найти все комментарии пользователя 5 к посту 4.",
        "analysis": "У поста 4 два комментария (Lucas Gray и Aubrey Gutierrez); комментариев пользователя 5 среди них нет — результат 0.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 2", function () { pm.expect(j.total).to.eql(2); });',
            'pm.test("Комментариев пользователя 5 нет", function () {',
            '  const c = j.comments.filter(x => x.user && x.user.id === 5);',
            '  pm.expect(c.length).to.eql(0);',
            '});',
        ],
    },
    {
        "key": "cart_user6",
        "folder": "09. Вложенные ресурсы",
        "name": "09.07. Корзина пользователя 6 — GET /carts/user/6",
        "method": "GET",
        "path": "/carts/user/6",
        "query": [],
        "status": 200,
        "task": "Получите корзину пользователя 6 по его идентификатору.",
        "analysis": "У пользователя 6 одна корзина (id 6) с четырьмя товарами; итоговая стоимость 55 453,86, со скидкой — 46 641,06.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 1", function () { pm.expect(j.total).to.eql(1); });',
            'pm.test("id корзины = 6", function () { pm.expect(j.carts[0].id).to.eql(6); });',
            'pm.test("4 товара", function () { pm.expect(j.carts[0].products.length).to.eql(4); });',
        ],
    },
    {
        "key": "posts_tag_crime",
        "folder": "10. Публикации по тегу",
        "name": "10.01. Посты с тегом crime — GET /posts/tag/crime",
        "method": "GET",
        "path": "/posts/tag/crime",
        "query": [],
        "status": 200,
        "task": "Найдите публикации, связанные с криминалом.",
        "analysis": "Тег crime найден у 43 публикаций; например, пост 1 «His mother had always taught him» и пост 5 «Hopes and dreams were dashed that day.»",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("total = 43", function () { pm.expect(j.total).to.eql(43); });',
            'pm.test("У всех постов есть тег crime", function () {',
            '  pm.expect(j.posts.every(p => p.tags.includes("crime"))).to.be.true;',
            '});',
        ],
    },
    {
        "key": "add_user",
        "folder": "11. Добавление",
        "name": "11.01. Добавить пользователя — POST /users/add",
        "method": "POST",
        "path": "/users/add",
        "query": [],
        "body": {
            "firstName": "Ivan",
            "lastName": "Petrov",
            "age": 20,
            "gender": "male",
            "email": "ivan.petrov@example.com",
            "address": {"country": "Russia"},
        },
        "status": 201,
        "task": "Добавьте себя в список пользователей.",
        "analysis": "Создан пользователь Ivan Petrov с id 209. Сервер возвращает объект с заполненными полями; запись не сохраняется в базе.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("id = 209", function () { pm.expect(j.id).to.eql(209); });',
            'pm.test("firstName = Ivan", function () { pm.expect(j.firstName).to.eql("Ivan"); });',
            'pm.test("country = Russia", function () { pm.expect(j.address.country).to.eql("Russia"); });',
        ],
    },
    {
        "key": "add_product",
        "folder": "11. Добавление",
        "name": "11.02. Добавить товар — POST /products/add",
        "method": "POST",
        "path": "/products/add",
        "query": [],
        "body": {
            "title": "Телевизор",
            "price": 49999,
            "category": "electronics",
            "description": "Тестовый товар для практики",
        },
        "status": 201,
        "task": "Добавить новый товар.",
        "analysis": "Создан товар «Телевизор» с id 195 и ценой 49 999; ответ содержит только переданные поля.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("id = 195", function () { pm.expect(j.id).to.eql(195); });',
            'pm.test("title = Телевизор", function () { pm.expect(j.title).to.eql("Телевизор"); });',
            'pm.test("price = 49999", function () { pm.expect(j.price).to.eql(49999); });',
        ],
    },
    {
        "key": "update_user1",
        "folder": "12. Обновление",
        "name": "12.01. Переселить пользователя 1 в Россию — PATCH /users/1",
        "method": "PATCH",
        "path": "/users/1",
        "query": [],
        "body": {"address": {"country": "Russia"}},
        "status": 200,
        "task": "Обновить пользователя. Обновлять можно любой набор параметров, не обязательно сразу все. Переселите пользователя 1 в Россию.",
        "analysis": "Ответ содержит обновлённое поле address.country = Russia; остальные поля пользователя сохранились. На сервере изменение не сохраняется — это имитация.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("id = 1", function () { pm.expect(j.id).to.eql(1); });',
            'pm.test("country = Russia", function () { pm.expect(j.address.country).to.eql("Russia"); });',
            'pm.test("Остальные поля сохранены", function () { pm.expect(j.firstName).to.eql("Emily"); });',
        ],
    },
    {
        "key": "delete_user13",
        "folder": "13. Удаление",
        "name": "13.01. Удалить пользователя 13 — DELETE /users/13",
        "method": "DELETE",
        "path": "/users/13",
        "query": [],
        "status": 200,
        "task": "Удалите пользователя 13.",
        "analysis": "Сервер возвращает объект пользователя с isDeleted = true и deletedOn; реального удаления не происходит.",
        "tests": [
            'const j = pm.response.json();',
            'pm.test("id = 13", function () { pm.expect(j.id).to.eql(13); });',
            'pm.test("isDeleted = true", function () { pm.expect(j.isDeleted).to.be.true; });',
        ],
    },
]

# Дополнительные проверки для запросов, где важна только структура ответа
GENERIC_TESTS = [
    'pm.test("Ответ в формате JSON", function () { pm.response.to.be.json; });',
]


def build_url(path, query):
    if not query:
        return path
    return path + "?" + urlencode(query, safe=",.")


def postman_url(path, query):
    raw = "{{baseUrl}}" + build_url(path, query)
    url = {"raw": raw, "host": ["{{baseUrl}}"], "path": [p for p in path.split("/") if p]}
    if query:
        url["query"] = [{"key": k, "value": v} for k, v in query]
    return url


def make_item(req):
    request = {
        "method": req["method"],
        "header": [],
        "url": postman_url(req["path"], req["query"]),
        "description": req["task"],
    }
    if req.get("body") is not None:
        request["header"] = [{"key": "Content-Type", "value": "application/json"}]
        request["body"] = {
            "mode": "raw",
            "raw": json.dumps(req["body"], ensure_ascii=False, indent=2),
            "options": {"raw": {"language": "json"}},
        }
    script = [
        f'pm.test("Статус {req["status"]}", function () {{ pm.response.to.have.status({req["status"]}); }});'
    ] + GENERIC_TESTS + req.get("tests", [])
    return {
        "name": req["name"],
        "request": request,
        "event": [{"listen": "test", "script": {"exec": script, "type": "text/javascript"}}],
    }


def build_collection():
    folders = []
    for req in REQUESTS:
        folder = next((f for f in folders if f["name"] == req["folder"]), None)
        if folder is None:
            folder = {"name": req["folder"], "item": []}
            folders.append(folder)
        folder["item"].append(make_item(req))
    return {
        "info": {
            "name": "ТРПО",
            "description": "Коллекция запросов к DummyJSON для практики 1 по дисциплине ТРПО.",
            "schema": "https://schema.getpostman.com/json/collection/v2.1.0/collection.json",
        },
        "item": [
            {
                "name": "Практика 1_ DummyJSON",
                "description": "Тематические папки с запросами к DummyJSON.",
                "item": folders,
            }
        ],
        "variable": [{"key": "baseUrl", "value": BASE, "type": "string"}],
    }


def build_environment():
    return {
        "id": "trpo-dummyjson-env",
        "name": "ТРПО_Группа_Фамилия",
        "values": [
            {"key": "baseUrl", "value": BASE, "type": "default", "enabled": True},
            {"key": "apiKey", "value": "", "type": "secret", "enabled": True},
        ],
        "_postman_variable_scope": "environment",
        "_postman_exported_at": datetime.now(timezone.utc).isoformat(),
        "_postman_exported_using": "Postman/1.90.0",
    }


def execute_requests():
    session = requests.Session()
    index = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "base_url": BASE,
        "requests": [],
    }
    for req in REQUESTS:
        url = BASE + build_url(req["path"], req["query"])
        started = time.perf_counter()
        if req["method"] == "GET":
            resp = session.get(url, timeout=30)
        elif req["method"] == "POST":
            resp = session.post(url, json=req["body"], timeout=30)
        elif req["method"] == "PATCH":
            resp = session.patch(url, json=req["body"], timeout=30)
        elif req["method"] == "DELETE":
            resp = session.delete(url, timeout=30)
        else:
            raise ValueError(req["method"])
        elapsed = (time.perf_counter() - started) * 1000
        try:
            payload = resp.json()
        except ValueError:
            payload = {"_raw": resp.text}
        (RESP_DIR / f"{req['key']}.json").write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
        )
        record = {
            "key": req["key"],
            "folder": req["folder"],
            "name": req["name"],
            "method": req["method"],
            "path": req["path"],
            "query": req["query"],
            "url": url,
            "status": resp.status_code,
            "expected_status": req["status"],
            "elapsed_ms": round(elapsed),
            "bytes": len(resp.content),
            "task": req["task"],
            "analysis": req["analysis"],
            "body": req.get("body"),
            "tests_count": 1 + len(GENERIC_TESTS) + len(req.get("tests", [])),
        }
        index["requests"].append(record)
        print(f"{resp.status_code} {req['method']:6} {build_url(req['path'], req['query'])} ({elapsed:.0f} ms)")
    (DATA_DIR / "index.json").write_text(
        json.dumps(index, ensure_ascii=False, indent=2), encoding="utf-8"
    )


def main():
    RESP_DIR.mkdir(parents=True, exist_ok=True)
    collection = build_collection()
    COLLECTION_PATH.write_text(
        json.dumps(collection, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    ENVIRONMENT_PATH.write_text(
        json.dumps(build_environment(), ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print("Коллекция:", COLLECTION_PATH)
    print("Окружение:", ENVIRONMENT_PATH)
    execute_requests()
    print("Данные:", DATA_DIR / "index.json")


if __name__ == "__main__":
    main()
