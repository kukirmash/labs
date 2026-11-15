#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Версия 2 отчёта по API: краткий формат «задание — запрос — результат»."""

import json
import re
import subprocess
import sys
from pathlib import Path

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_TAB_ALIGNMENT, WD_TAB_LEADER
from docx.shared import Cm, Pt

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_api_report as base  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
API = ROOT / "api"
DATA = API / "data"
DATA2 = DATA / "v2"
IMG = API / "images"
POSTMAN = API / "postman"

OUT_DOCX = ROOT / "Отчёт_ТРПО_API_Практика_v2.docx"
OUT_PDF = ROOT / "Отчёт_ТРПО_API_Практика_v2.pdf"

FONT_BODY = "Times New Roman"


def load(name):
    return json.loads((DATA / name).read_text(encoding="utf-8"))


def load2(name):
    path = DATA2 / name
    if not path.suffix:
        path = path.with_suffix(".json")
    return json.loads(path.read_text(encoding="utf-8"))


USERS = base.response("users")
PRODUCTS = base.response("products")
USERS_SELECT = base.response("users_select")
POSTS_SELECT = base.response("posts_select_tags")
USER3 = base.response("user_3")
FILTER_SOPHIA = base.response("filter_sophia")
FILTER_BROWN = base.response("filter_brown_hair")
SEARCH_SOPHIA = base.response("search_sophia")
SORT_ASC = base.response("sort_age_asc")
PAGE = base.response("page_10_skip_10")
ALL_USERS = base.response("limit_all")
COMMENTS_ALL = base.response("comments_all")
ADD_USER = base.response("add_user")
ADD_PRODUCT = base.response("add_product")
UPDATE_USER = base.response("update_user1")
DELETE_USER = base.response("delete_user13")

TODOS1 = load2("todos_user1")
POSTS7 = load2("posts_user7")
POST_IDS7 = load2("post_ids_user7")
COMMENTS227 = load2("comments_post227")
COMMENTS4 = load2("comments_post4")
CART6 = load2("cart_user6")

CURRENT = base.OWM["current_zip"]
MOSCOW = base.OWM["current_moscow"]
DAILY = [row for row in base.OWM["daily_zip"] if row["steps"] >= 4][:5]

PET_POST = load("petstore_post.json")
PET_GET = load("petstore_get.json")
PET_XML = (DATA / "petstore_post.xml").read_text(encoding="utf-8").strip()
SUNRISE = load("sunrise_response.json")
GH = load("github_project.json")

COMMENT_USER3 = [c for c in COMMENTS_ALL["comments"] if c["user"]["id"] == 3][0]
COMMENT_USER5_POST4 = [c for c in COMMENTS4["comments"] if c["user"]["id"] == 5]
CART = CART6["carts"][0]


# ---------------------------------------------------------------------------
# Оформление
# ---------------------------------------------------------------------------


def add_title_page(doc):
    for _ in range(4):
        base.add_para(doc, "", size=12, indent=Cm(0), after=0, line=1.0)
    base.add_para(
        doc,
        "ОТЧЁТ",
        size=30,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=6,
        line=1.0,
    )
    base.add_para(
        doc,
        "по практическим занятиям",
        size=16,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=0,
        line=1.2,
    )
    base.add_para(
        doc,
        "по дисциплине «Технология разработки программного обеспечения»",
        size=16,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=24,
        line=1.2,
    )
    base.add_para(
        doc,
        "Тема: «Проектирование программного интерфейса API»",
        size=16,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=6,
        line=1.2,
    )
    base.add_para(
        doc,
        "Практика 1–4",
        size=13,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=30,
        line=1.2,
    )
    for label in (
        "Выполнил(а): ______________________",
        "Группа: ______________________",
        "Проверил(а): ______________________",
    ):
        p = base.add_para(
            doc,
            label,
            size=14,
            align=WD_ALIGN_PARAGRAPH.LEFT,
            indent=Cm(0),
            before=0,
            after=8,
            line=1.2,
        )
        p.paragraph_format.left_indent = Cm(8.5)
    for _ in range(3):
        base.add_para(doc, "", size=12, indent=Cm(0), after=0, line=1.0)
    base.add_para(
        doc,
        "2026",
        size=14,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=0,
        line=1.0,
    )


def add_toc(doc, pages):
    base.add_h1(doc, "СОДЕРЖАНИЕ")
    for title, anchor, level in TOC:
        p = doc.add_paragraph()
        pf = p.paragraph_format
        pf.alignment = WD_ALIGN_PARAGRAPH.LEFT
        pf.first_line_indent = Cm(0)
        pf.left_indent = Cm(0.6 * level)
        pf.space_before = Pt(0)
        pf.space_after = Pt(2)
        pf.line_spacing = 1.08
        pf.tab_stops.add_tab_stop(Cm(16.7), WD_TAB_ALIGNMENT.RIGHT, WD_TAB_LEADER.DOTS)
        page = pages.get(anchor, "")
        run = p.add_run(title)
        base.set_run_font(run, FONT_BODY, 11, bold=(level == 0))
        run2 = p.add_run("\t" + (str(page) if page else "00"))
        base.set_run_font(run2, FONT_BODY, 11)


def req_text(method, url, body=None):
    text = f"{method} {url}"
    if body is not None:
        text += "\nContent-Type: application/json\n\n" + json.dumps(body, ensure_ascii=False, indent=2)
    return text


def show(doc, title, method, url, status, result, body=None, level=2, max_lines=None, note=None):
    (base.add_h2 if level == 2 else base.add_h3)(doc, title)
    request_para = base.add_code(doc, req_text(method, url, body), size=8.3, line=10.3)
    request_para.paragraph_format.keep_with_next = True
    base.add_label(doc, f"Результат (HTTP {status}):")
    if isinstance(result, str):
        base.add_code(doc, result, size=8.3, line=10.3)
    else:
        base.add_json(doc, result, max_lines=max_lines)
    if note:
        base.add_para(doc, note, size=11, italic=True, indent=Cm(0), before=2, after=6, line=1.15)


def compact_users(users, fields, total, skip, limit):
    return base.compact_users(users, fields, total, skip, limit)


def comments_result(comments, total):
    lines = ["{", '  "comments": [']
    for i, c in enumerate(comments):
        obj = {"id": c["id"], "body": c["body"], "user": c["user"]["fullName"]}
        suffix = "," if i < len(comments) - 1 else ""
        lines.append("    " + json.dumps(obj, ensure_ascii=False) + suffix)
    lines.extend(["  ],", f'  "total": {total}', "}"])
    return "\n".join(lines)


def compact_response(key, items, total, skip, limit):
    lines = ["{", f'  "{key}": [']
    for i, item in enumerate(items):
        suffix = "," if i < len(items) - 1 else ""
        lines.append("    " + json.dumps(item, ensure_ascii=False) + suffix)
    lines.extend(["  ],", f'  "total": {total},', f'  "skip": {skip},', f'  "limit": {limit}', "}"])
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Занятие 1
# ---------------------------------------------------------------------------


def add_lesson1(doc):
    base.add_h1(doc, "ЗАНЯТИЕ 1. ТЕСТИРОВАНИЕ API DUMMYJSON В POSTMAN")

    base.add_h2(doc, "Настройте POSTMAN")
    base.add_code(
        doc,
        "Окружение: ТРПО_Группа_Фамилия\n"
        "Переменная: baseUrl = https://dummyjson.com\n"
        "Коллекция: ТРПО\n"
        "Папка: Практика 1_ DummyJSON\n"
        "Тематических папок: 13\n"
        "Именованных запросов: 24",
        size=9,
        line=11,
    )
    base.add_label(doc, "Результат:")
    base.add_para(
        doc,
        "Окружение, коллекция и папки созданы; запросы именованы; коллекция и окружение "
        "экспортированы в файлы Postman Collection v2.1 и Postman Environment.",
        after=4,
    )

    show(
        doc,
        "Тестовый маршрут. Проверьте, работает ли ваш интернет",
        "GET",
        "https://dummyjson.com/test",
        200,
        base.response("test"),
    )

    base.add_h2(doc, "Основные сущности")
    show(
        doc,
        "Получить список пользователей",
        "GET",
        "https://dummyjson.com/users",
        200,
        compact_response(
            "users",
            [
                {
                    "id": u["id"],
                    "firstName": u["firstName"],
                    "lastName": u["lastName"],
                    "age": u["age"],
                    "gender": u["gender"],
                }
                for u in USERS["users"][:3]
            ],
            USERS["total"],
            USERS["skip"],
            USERS["limit"],
        ),
        level=3,
    )
    show(
        doc,
        "Получить список продуктов",
        "GET",
        "https://dummyjson.com/products",
        200,
        compact_response(
            "products",
            [
                {
                    "id": p["id"],
                    "title": p["title"],
                    "price": p["price"],
                    "category": p["category"],
                }
                for p in PRODUCTS["products"][:2]
            ],
            PRODUCTS["total"],
            PRODUCTS["skip"],
            PRODUCTS["limit"],
        ),
        level=3,
    )

    base.add_h2(doc, "Выборка данных с параметром select.")
    show(
        doc,
        "Выбрать пользователей с именем, полом и возрастом",
        "GET",
        "https://dummyjson.com/users?select=firstName,gender,age&limit=0",
        200,
        compact_users(USERS_SELECT["users"][:3], ("id", "firstName", "gender", "age"), 208, 0, 208),
        level=3,
    )
    show(
        doc,
        "Выбрать все посты, оставив только теги",
        "GET",
        "https://dummyjson.com/posts?select=tags&limit=0",
        200,
        compact_response(
            "posts",
            [{"id": p["id"], "tags": p["tags"]} for p in POSTS_SELECT["posts"][:3]],
            POSTS_SELECT["total"],
            POSTS_SELECT["skip"],
            POSTS_SELECT["limit"],
        ),
        level=3,
    )

    base.add_h2(doc, "Выборка записи по идентификатору.")
    show(
        doc,
        "Выбрать пользователя с идентификатором 3",
        "GET",
        "https://dummyjson.com/users/3",
        200,
        {
            "id": USER3["id"],
            "firstName": USER3["firstName"],
            "lastName": USER3["lastName"],
            "age": USER3["age"],
            "gender": USER3["gender"],
            "email": USER3["email"],
            "hair": USER3["hair"],
            "address": {
                "city": USER3["address"]["city"],
                "country": USER3["address"]["country"],
            },
        },
        level=3,
        max_lines=18,
    )

    base.add_h2(doc, "Фильтрация данных с параметром filter.")
    show(
        doc,
        "Получить информацию о пользователе Sophia",
        "GET",
        "https://dummyjson.com/users/filter?key=firstName&value=Sophia",
        200,
        compact_users(FILTER_SOPHIA["users"], ("id", "firstName", "lastName", "age"), 2, 0, 2),
        level=3,
    )
    show(
        doc,
        "Найти шатенов среди пользователей.",
        "GET",
        "https://dummyjson.com/users/filter?key=hair.color&value=Brown",
        200,
        compact_users(
            FILTER_BROWN["users"][:5],
            ("id", "firstName", "lastName", "age"),
            23,
            0,
            23,
        ),
        level=3,
        note="В ответе 23 пользователя; показаны первые пять.",
    )

    base.add_h2(doc, "Поиск данных с параметром search.")
    show(
        doc,
        "Найти пользователя по имени Sophia.",
        "GET",
        "https://dummyjson.com/users/search?q=Sophia",
        200,
        compact_users(SEARCH_SOPHIA["users"], ("id", "firstName", "lastName", "age"), 2, 0, 2),
        level=3,
    )

    show(
        doc,
        "Сортировка данных (с параметром orderby). Отсортировать пользователей по возрасту.",
        "GET",
        "https://dummyjson.com/users?sortBy=age&order=asc&limit=0",
        200,
        compact_response(
            "users",
            [
                {"id": u["id"], "firstName": u["firstName"], "lastName": u["lastName"], "age": u["age"]}
                for u in SORT_ASC["users"][:3]
            ],
            SORT_ASC["total"],
            SORT_ASC["skip"],
            SORT_ASC["limit"],
        ),
        note="Минимальный возраст — 23, максимальный — 46.",
    )

    base.add_h2(doc, "Пагинация данных с параметрами top и skip.")
    show(
        doc,
        "Страница: limit = 10, skip = 10",
        "GET",
        "https://dummyjson.com/users?limit=10&skip=10",
        200,
        compact_response(
            "users",
            [{"id": u["id"], "firstName": u["firstName"]} for u in PAGE["users"][:3]],
            PAGE["total"],
            PAGE["skip"],
            PAGE["limit"],
        ),
    )
    show(
        doc,
        "Исследуйте работу параметра limit. Например, получить список всех (208) пользователей.",
        "GET",
        "https://dummyjson.com/users?limit=0",
        200,
        compact_response(
            "users",
            [{"id": u["id"], "firstName": u["firstName"]} for u in ALL_USERS["users"][:2]],
            ALL_USERS["total"],
            ALL_USERS["skip"],
            ALL_USERS["limit"],
        ),
        level=3,
    )

    base.add_h2(doc, "Вложенные ресурсы.")
    show(
        doc,
        "Получить список всех задач пользователя 1",
        "GET",
        "https://dummyjson.com/todos/user/1",
        200,
        TODOS1,
        level=3,
    )
    show(
        doc,
        "Получить список публикаций (постов) пользователя 7",
        "GET",
        "https://dummyjson.com/posts/user/7",
        200,
        compact_response(
            "posts",
            [
                {"id": p["id"], "title": p["title"], "userId": p["userId"]}
                for p in POSTS7["posts"]
            ],
            POSTS7["total"],
            POSTS7["skip"],
            POSTS7["limit"],
        ),
        level=3,
    )
    show(
        doc,
        "Получить список идентификаторов публикаций пользователя 7.",
        "GET",
        "https://dummyjson.com/posts/user/7?select=id",
        200,
        compact_response(
            "posts",
            [{"id": p["id"]} for p in POST_IDS7["posts"]],
            POST_IDS7["total"],
            POST_IDS7["skip"],
            POST_IDS7["limit"],
        ),
        level=3,
    )
    show(
        doc,
        "Найдите все комментарии пользователя 3",
        "GET",
        "https://dummyjson.com/comments?limit=0",
        200,
        {
            "id": COMMENT_USER3["id"],
            "body": COMMENT_USER3["body"],
            "postId": COMMENT_USER3["postId"],
            "likes": COMMENT_USER3["likes"],
            "user": COMMENT_USER3["user"],
        },
        level=3,
        note="В полном ответе 340 комментариев; после фильтра user.id = 3 остаётся один.",
    )
    show(
        doc,
        "Alexander Jones разместил пост «Над городом возвышался исторический замок». "
        "Найдите комментарии к этому посту.",
        "GET",
        "https://dummyjson.com/comments/post/227",
        200,
        comments_result(COMMENTS227["comments"], COMMENTS227["total"]),
        level=3,
    )
    show(
        doc,
        "Найти все комментарии пользователя 5 к посту 4",
        "GET",
        "https://dummyjson.com/comments/post/4",
        200,
        comments_result(COMMENTS4["comments"], COMMENTS4["total"]),
        level=3,
        note="Комментариев с user.id = 5 в ответе нет — результат 0.",
    )
    show(
        doc,
        "Получите корзину пользователя 6 по его идентификатору",
        "GET",
        "https://dummyjson.com/carts/user/6",
        200,
        {
            "carts": [
                {
                    "id": CART["id"],
                    "userId": CART["userId"],
                    "totalProducts": CART["totalProducts"],
                    "totalQuantity": CART["totalQuantity"],
                    "total": CART["total"],
                    "discountedTotal": CART["discountedTotal"],
                    "products": [
                        {"title": p["title"], "price": p["price"], "quantity": p["quantity"]}
                        for p in CART["products"]
                    ],
                }
            ],
            "total": CART6["total"],
            "skip": CART6["skip"],
            "limit": CART6["limit"],
        },
        level=3,
        max_lines=28,
    )

    show(
        doc,
        "Получение публикаций по тегу. Найдите публикации, связанные с криминалом",
        "GET",
        "https://dummyjson.com/posts/tag/crime",
        200,
        compact_response(
            "posts",
            [
                {"id": p["id"], "title": p["title"], "tags": p["tags"]}
                for p in base.response("posts_tag_crime")["posts"][:3]
            ],
            base.response("posts_tag_crime")["total"],
            0,
            30,
        ),
        note="Всего 43 публикации; показаны первые три.",
    )

    base.add_h2(doc, "Добавление.")
    show(
        doc,
        "Добавьте себя в список пользователей",
        "POST",
        "https://dummyjson.com/users/add",
        201,
        {
            "id": ADD_USER["id"],
            "firstName": ADD_USER["firstName"],
            "lastName": ADD_USER["lastName"],
            "age": ADD_USER["age"],
            "email": ADD_USER["email"],
            "address": {"country": ADD_USER["address"]["country"]},
        },
        level=3,
        body={
            "firstName": "Ivan",
            "lastName": "Petrov",
            "age": 20,
            "gender": "male",
            "email": "ivan.petrov@example.com",
            "address": {"country": "Russia"},
        },
        max_lines=14,
    )
    show(
        doc,
        "Добавить новый товар",
        "POST",
        "https://dummyjson.com/products/add",
        201,
        ADD_PRODUCT,
        level=3,
        body={
            "title": "Телевизор",
            "price": 49999,
            "category": "electronics",
            "description": "Тестовый товар для практики",
        },
        max_lines=10,
    )

    base.add_h2(doc, "Обновление.")
    show(
        doc,
        "Обновить пользователя. Обновлять можно любой набор параметров, не обязательно сразу все. "
        "Переселите пользователя 1 в Россию.",
        "PATCH",
        "https://dummyjson.com/users/1",
        200,
        {
            "id": UPDATE_USER["id"],
            "firstName": UPDATE_USER["firstName"],
            "lastName": UPDATE_USER["lastName"],
            "address": {
                "city": UPDATE_USER["address"]["city"],
                "country": UPDATE_USER["address"]["country"],
            },
        },
        level=3,
        body={"address": {"country": "Russia"}},
        max_lines=14,
    )

    show(
        doc,
        "Удаление. Удалите пользователя 13",
        "DELETE",
        "https://dummyjson.com/users/13",
        200,
        {
            "id": DELETE_USER["id"],
            "firstName": DELETE_USER["firstName"],
            "lastName": DELETE_USER["lastName"],
            "isDeleted": DELETE_USER["isDeleted"],
            "deletedOn": DELETE_USER["deletedOn"],
        },
        max_lines=10,
    )

    base.add_h2(doc, "Создайте резервную копию собственной коллекции. Эту копию можно будет импортировать.")
    base.add_label(doc, "Результат:")
    base.add_code(
        doc,
        "api/postman/ТРПО_Практика_1_DummyJSON_v2.postman_collection.json\n"
        "api/postman/ТРПО_Группа_Фамилия_v2.postman_environment.json",
        size=8.5,
        line=10.5,
    )
    base.add_para(
        doc,
        "Коллекция и окружение экспортированы в формате Postman Collection v2.1 и готовы к импорту.",
        after=6,
    )


# ---------------------------------------------------------------------------
# Занятие 2
# ---------------------------------------------------------------------------


def add_lesson2(doc):
    base.add_h1(doc, "ЗАНЯТИЕ 2. ИСПОЛЬЗОВАНИЕ API OPENWEATHERMAP", page_break=False)

    base.add_h2(doc, "Исследование API и ключ авторизации")
    base.add_para(
        doc,
        "Для текущей погоды используется /data/2.5/weather, для прогноза — "
        "/data/2.5/forecast. Авторизация — параметр appid, единицы — units=metric, язык — lang=ru.",
        after=3,
    )
    base.add_code(
        doc,
        "GET https://api.openweathermap.org/data/2.5/weather?zip=95050&units=metric&lang=ru&appid=<ключ>\n"
        "GET https://api.openweathermap.org/data/2.5/forecast?zip=95050&units=metric&lang=ru&appid=<ключ>",
        size=8.0,
        line=10.0,
    )
    base.add_para(
        doc,
        "Использован учебный ключ из методички (c1b5…ac43).",
        after=4,
    )

    base.add_h2(doc, "Текущая погода")
    show(
        doc,
        "Запрос текущей погоды (Santa Clara)",
        "GET",
        "https://api.openweathermap.org/data/2.5/weather?zip=95050&units=metric&lang=ru&appid=<ключ>",
        200,
        {
            "name": CURRENT["name"],
            "coord": CURRENT["coord"],
            "weather": [
                {
                    "main": CURRENT["weather"][0]["main"],
                    "description": CURRENT["weather"][0]["description"],
                    "icon": CURRENT["weather"][0]["icon"],
                }
            ],
            "main": {
                "temp": CURRENT["main"]["temp"],
                "feels_like": CURRENT["main"]["feels_like"],
                "pressure": CURRENT["main"]["pressure"],
                "humidity": CURRENT["main"]["humidity"],
            },
            "wind": {"speed": CURRENT["wind"]["speed"]},
            "cod": CURRENT["cod"],
        },
        max_lines=20,
    )
    base.add_table(
        doc,
        ("Поле", "Значение"),
        [
            ("Температура", f"{base.f1(CURRENT['main']['temp'])} °C"),
            ("Ощущается", f"{base.f1(CURRENT['main']['feels_like'])} °C"),
            ("Влажность", f"{CURRENT['main']['humidity']} %"),
            ("Давление", f"{CURRENT['main']['pressure']} гПа"),
            ("Ветер", f"{base.f1(CURRENT['wind']['speed'])} м/с"),
        ],
        [Cm(8.0), Cm(8.0)],
        font=9.5,
        center_cols=(1,),
    )
    show(
        doc,
        "Текущая погода в Москве",
        "GET",
        "https://api.openweathermap.org/data/2.5/weather?q=Moscow&units=metric&lang=ru&appid=<ключ>",
        200,
        {
            "name": MOSCOW["name"],
            "main": {
                "temp": MOSCOW["main"]["temp"],
                "feels_like": MOSCOW["main"]["feels_like"],
                "humidity": MOSCOW["main"]["humidity"],
            },
            "weather": MOSCOW["weather"],
        },
        max_lines=16,
    )

    base.add_h2(doc, "Прогноз на 5 дней")
    show(
        doc,
        "Запрос прогноза",
        "GET",
        "https://api.openweathermap.org/data/2.5/forecast?zip=95050&units=metric&lang=ru&appid=<ключ>",
        200,
        {
            "cod": base.OWM["forecast_zip"]["cod"],
            "cnt": base.OWM["forecast_zip"]["cnt"],
            "city": {
                "name": base.OWM["forecast_zip"]["city"]["name"],
                "timezone": base.OWM["forecast_zip"]["city"]["timezone"],
            },
            "list[0]": {
                "dt_txt": base.OWM["forecast_zip"]["list"][0]["dt_txt"],
                "main": {
                    "temp": base.OWM["forecast_zip"]["list"][0]["main"]["temp"],
                    "humidity": base.OWM["forecast_zip"]["list"][0]["main"]["humidity"],
                },
            },
        },
        max_lines=18,
    )
    base.add_table(
        doc,
        ("Дата", "Мин., °C", "Макс., °C", "Описание"),
        [
            (row["date"], base.f1(row["min"]), base.f1(row["max"]), row["description"])
            for row in DAILY
        ],
        [Cm(3.4), Cm(3.0), Cm(3.0), Cm(6.6)],
        font=9.5,
        center_cols=(1, 2),
    )

    base.add_h2(doc, "Запросы в curl")
    base.add_code(
        doc,
        "# Текущая погода\n"
        "curl -s \"https://api.openweathermap.org/data/2.5/weather?zip=95050&units=metric&lang=ru&appid=$OWM_API_KEY\"\n\n"
        "# Прогноз на 5 дней\n"
        "curl -s \"https://api.openweathermap.org/data/2.5/forecast?zip=95050&units=metric&lang=ru&appid=$OWM_API_KEY\"\n\n"
        "# POST-запрос к DummyJSON\n"
        "curl -X POST https://dummyjson.com/products/add \\\n"
        "  -H \"Content-Type: application/json\" \\\n"
        "  -d '{\"title\":\"Телевизор\",\"price\":49999}'",
        size=8.0,
        line=10.0,
    )
    base.add_para(
        doc,
        "Все команды вернули HTTP 200/201 и корректный JSON.",
        after=4,
    )

    base.add_h2(doc, "AJAX-запрос и визуализация")
    base.add_code(
        doc,
        "const [currentResponse, forecastResponse] = await Promise.all([\n"
        "  fetch(CURRENT_URL),\n"
        "  fetch(FORECAST_URL),\n"
        "]);\n"
        "const current = await currentResponse.json();\n"
        "const forecast = await forecastResponse.json();\n"
        "const rows = aggregate(forecast.list).filter(r => r.steps >= 4).slice(0, 5);\n"
        "renderCurrent(current);\n"
        "renderTable(rows);\n"
        "renderChart(rows);",
        size=8.2,
        line=10.2,
    )
    base.add_figure(
        doc,
        IMG / "owm_ajax.png",
        "Рисунок 1 — AJAX-страница с текущей погодой и прогнозом OpenWeatherMap",
        max_w=17.0,
        max_h=15.0,
    )

    base.add_h2(doc, "Анализ полезной нагрузки")
    base.add_table(
        doc,
        ("Поле", "Назначение"),
        [
            ("coord", "Широта и долгота"),
            ("weather[]", "Код, группа, описание, иконка"),
            ("main", "Температура, давление, влажность"),
            ("wind", "Скорость и направление ветра"),
            ("visibility", "Видимость, метры"),
            ("cod", "Код результата"),
        ],
        [Cm(4.0), Cm(12.0)],
        font=9.5,
    )
    base.add_para(
        doc,
        "Ответ Forecast содержит cod, cnt, list и city; list — 40 точек с шагом 3 часа.",
        after=3,
    )
    base.add_table(
        doc,
        ("Ошибка", "Код", "Ответ"),
        [
            ("Неверный appid", "401", '{"cod": 401, "message": "Invalid API key..."}'),
            ("Неизвестный город", "404", '{"cod": "404", "message": "city not found"}'),
        ],
        [Cm(5.0), Cm(2.0), Cm(9.0)],
        font=9.0,
        center_cols=(1,),
    )


# ---------------------------------------------------------------------------
# Занятие 3
# ---------------------------------------------------------------------------

SURF_ERRORS = [
    ("Метод GET/POST указан одной строкой", "Непонятно, какие операции поддерживаются и чем они отличаются. Разделить на GET и POST с отдельными описаниями."),
    ("Путь surfreport/{:beachId}", "Лишнее двоеточие и отсутствует ведущий слеш. Использовать /surfreport/{beachId}."),
    ("Не указан базовый URL", "Добавить servers/базовый адрес API."),
    ("В примере запроса домен OpenWeatherMap", "Пример не относится к surfreport. Заменить домен на базовый URL surfreport."),
    ("В примере передан appid", "Секретный ключ нельзя публиковать. Показать {API_KEY} или переменную окружения."),
    ("В примере параметр zip", "Параметра zip нет в таблице. Удалить его или описать."),
    ("Параметр time: Integer + ISO 8601", "Тип противоречит формату. Указать string (date-time)."),
    ("Пример time 20180915T155300+0500", "Не соответствует ISO 8601. Использовать 2018-09-15T15:53:00+05:00."),
    ("Параметр days без минимума", "Указать min = 1, max = 10, default = 3."),
    ("Не указан часовой пояс time", "Добавить описание часового пояса и названий дней."),
    ("Нет кодов ответов", "Добавить 200, 400, 404, 500 и примеры ошибок."),
    ("Нет схемы ошибки", "Описать единый JSON-формат ошибки."),
    ("tide: string, в примере число", "Изменить тип на number и указать единицы."),
    ("waterTemp: string, в примере число", "Использовать water_temp: number, единицы °F/°C."),
    ("surfheight: map, в примере число", "Использовать surf_height: number и единицы."),
    ("В таблице нет поля riptide", "Добавить riptide и допустимые значения."),
    ("Не описаны значения riptide", "Задать enum: none, moderate, extreme."),
    ("Не указаны единицы tide, wind, waterTemp, surfheight", "Добавить единицы в описание и примеры."),
    ("Порог wind = 15 без единиц", "Указать mph, knots или m/s."),
    ("Порог waterTemp 70/60 °F без единиц в ответе", "Указать °F или °C и не смешивать шкалы."),
    ("surfheight зависит от units, но units не описан", "Добавить параметр units и допустимые значения."),
    ("В примере ответа стоит ...", "Привести полный валидный JSON без многоточия."),
    ("{day} и {time} — динамические ключи", "Описать формат ключей и вложенность."),
    ("Нет схемы ответа", "Добавить JSON Schema/OpenAPI-схему."),
    ("Нет аутентификации, лимитов и версии", "Добавить разделы security, rate limits, versioning."),
    ("POST не имеет тела запроса", "Описать body и обязательные поля."),
    ("«Ответы» ведут в «Описание ошибок» без списка", "Привести перечень кодов и ссылку на каждый."),
    ("recommendation на жаргоне", "Дать нейтральное описание и глоссарий."),
]


def add_lesson3(doc):
    base.add_h1(doc, "ЗАНЯТИЕ 3. ДОКУМЕНТИРОВАНИЕ КОНЕЧНОЙ ТОЧКИ API")

    base.add_h2(doc, "Что не так в разделе API?")
    base.add_para(
        doc,
        "В предложенном примере surfreport найдено 28 ошибок. Они сгруппированы по "
        "методу, параметрам, примеру запроса, схеме ответа и оформлению.",
        after=4,
    )
    base.add_table(
        doc,
        ("№", "Что не так", "Как исправить"),
        [(str(i), wrong, fix) for i, (wrong, fix) in enumerate(SURF_ERRORS, 1)],
        [Cm(0.9), Cm(7.6), Cm(8.5)],
        font=8.3,
        center_cols=(0,),
        keep_rows=False,
    )

    base.add_h2(doc, "Поиск опен-сорс проекта")
    issue = GH["issue_search"]["items"][0]
    repo = GH["repo"]
    base.add_para(
        doc,
        "Поиск выполнялся в расширенном поиске GitHub по запросу "
        "label:\"help wanted\" label:documentation api. Выбран проект "
        "Santhosh-zeta/InternalXDropwing.",
        after=3,
    )
    base.add_table(
        doc,
        ("Критерий", "Результат"),
        [
            ("Проект", "Santhosh-zeta/InternalXDropwing (MIT)"),
            ("REST API", "Да: Express, базовый путь /api, cookie-сессия или Bearer"),
            ("Метки документации", "documentation, help wanted, backend"),
            ("Issue", f"#{issue['number']} «{issue['title']}»"),
            ("Состояние документации", "README с обзором API; отдельные docs/*.md; OpenAPI-спецификации нет"),
            ("Активность", f"Последний коммит {repo['pushed_at'][:10]}; {len(GH['recent_commits'])} коммитов за 30 дней"),
            ("Участники", f"{len(GH['contributors'])} контрибьюторов; {repo['open_issues_count']} открытых issue"),
            ("Ссылка", issue["html_url"]),
        ],
        [Cm(3.8), Cm(13.2)],
        font=9.2,
    )
    base.add_para(
        doc,
        "Вывод: проект подходит для практики — REST API есть, документация частичная, "
        "а issue #37 прямо требует OpenAPI-спецификацию и проверку в CI.",
        after=4,
    )

    base.add_h2(doc, "Оценка ключевых элементов API документации")
    base.add_para(
        doc,
        "Сравнивались документация DummyJSON и OpenWeatherMap по элементам, "
        "перечисленным в задании.",
        after=3,
    )
    base.add_table(
        doc,
        ("Элемент", "DummyJSON", "OpenWeatherMap"),
        [
            ("Описание ресурса", "Нет отдельного action-oriented описания; сразу примеры", "Есть вводный текст и Product concept"),
            ("Конечные точки и методы", "Сгруппированы по ресурсам; метод виден только в примере fetch", "Сгруппированы по продуктам; метод указан в вызове"),
            ("Параметры", "Общий раздел limit/skip/select; типы и минимумы не указаны", "Таблица параметров с required/optional; типы и min/max не указаны"),
            ("Пример запроса", "fetch на JavaScript, 2–4 параметра", "URL + JSON, 3–4 параметра"),
            ("Пример ответа", "JSON-фрагменты, схемы нет", "JSON/XML и таблица полей"),
            ("Вложенные структуры", "Вложенный JSON без пояснения иерархии", "Поля с точечной нотацией (main.temp и т.п.)"),
        ],
        [Cm(4.2), Cm(6.2), Cm(6.6)],
        font=8.8,
    )
    base.add_para(
        doc,
        "Вывод: OpenWeatherMap документирует параметры и поля ответа полнее; "
        "DummyJSON удобен для быстрого старта, но не даёт формального описания контракта.",
        after=4,
    )


# ---------------------------------------------------------------------------
# Занятие 4
# ---------------------------------------------------------------------------


def add_lesson4(doc):
    base.add_h1(doc, "ЗАНЯТИЕ 4. OPENAPI И SWAGGER")

    base.add_h2(doc, "Знакомство со Swagger с помощью Petstore Demo")
    base.add_para(
        doc,
        "В Swagger UI Petstore раскрыта операция POST /pet, заполнены id = 1000 и "
        "name = Puppy, выполнена кнопка Execute. Затем выполнен GET /pet/1000. "
        "Кнопка Authorize в демо предназначена для тестового api_key; запросы "
        "выполнены без реальной авторизации.",
        after=3,
    )
    base.add_figure(
        doc,
        IMG / "petstore_swagger.png",
        "Рисунок 2 — Swagger UI Petstore: операции с питомцами и кнопка Authorize",
        max_w=17.0,
        max_h=13.0,
    )
    base.add_code(
        doc,
        "POST http://127.0.0.1:8099/api/v3/pet\n"
        "Content-Type: application/json\n\n"
        '{"id": 1000, "name": "Puppy", "status": "available"}',
        size=8.3,
        line=10.3,
    )
    base.add_label(doc, "Результат POST (HTTP 200, XML):")
    base.add_code(doc, PET_XML, size=8.3, line=10.3)
    base.add_label(doc, "Результат POST (JSON):")
    base.add_json(doc, PET_POST, max_lines=10)
    get_para = base.add_code(
        doc,
        "GET http://127.0.0.1:8099/api/v3/pet/1000",
        size=8.3,
        line=10.3,
    )
    get_para.paragraph_format.keep_with_next = True
    base.add_label(doc, "Результат GET (HTTP 200):")
    base.add_json(doc, PET_GET, max_lines=10)
    base.add_para(
        doc,
        "Публичный petstore.swagger.io из сети стенда возвращал 403, поэтому использован "
        "официальный контейнер swaggerapi/petstore3; спецификация и Swagger UI те же.",
        size=10.5,
        italic=True,
        indent=Cm(0),
        after=4,
        line=1.2,
    )

    base.add_h2(doc, "Редакция существующей спецификации API")
    base.add_para(
        doc,
        "В спецификации Sunrise/sunset API изменены info.description и описание "
        "параметра lat; найден $ref на схему ответа.",
        after=3,
    )
    base.add_code(
        doc,
        "info:\n"
        "  description: \"Sunrise and sunset times ... edited for the TRPO API practice.\"\n\n"
        "parameters:\n"
        "  - name: lat\n"
        "    description: \"Latitude in decimal degrees (edited in the practice).\"\n\n"
        "responses:\n"
        "  200:\n"
        "    content:\n"
        "      application/json:\n"
        "        schema:\n"
        "          $ref: '#/components/schemas/SunriseSunsetResponse'",
        size=8.0,
        line=10.0,
    )
    base.add_para(
        doc,
        "Указатель $ref ведёт на компонент SunriseSunsetResponse.",
        after=3,
    )
    base.add_figure(
        doc,
        IMG / "sunrise_get.png",
        "Рисунок 3 — Операция GET /json в Swagger UI с кнопкой Try it out",
        max_w=17.0,
        max_h=7.0,
    )
    base.add_para(
        doc,
        "После добавления пробела перед info Swagger UI показал ошибку.",
        after=3,
    )
    base.add_figure(
        doc,
        IMG / "swagger_error.png",
        "Рисунок 4 — Ошибка разбора YAML после добавления пробела перед info",
        max_w=17.0,
        max_h=12.0,
    )
    base.add_para(
        doc,
        "После удаления пробела спецификация прошла проверку openapi-spec-validator. "
        "Запрос GET /json вернул:",
        after=3,
    )
    base.add_code(
        doc,
        "GET https://api.sunrise-sunset.org/json?lat=37.3708698&lng=-122.037593&date=today",
        size=8.0,
        line=10.0,
    )
    base.add_json(doc, SUNRISE, max_lines=24)

    base.add_h2(doc, "Наполняем интерфейс Swagger")
    base.add_para(
        doc,
        "Создана папка dist с файлами Swagger UI, спецификацией и index.html. "
        "В index.html изменён параметр url.",
        after=3,
    )
    base.add_code(
        doc,
        "dist/\n"
        "├── index.html\n"
        "├── swagger-ui.css\n"
        "├── swagger-ui-bundle.js\n"
        "├── swagger-ui-standalone-preset.js\n"
        "└── openapi_sunrise_sunset.yml\n\n"
        "// index.html\n"
        "url: \"openapi_sunrise_sunset.yml\"",
        size=8.3,
        line=10.3,
    )
    base.add_code(
        doc,
        "python3 -m http.server 8770 --directory api/swagger-dist",
        size=8.3,
        line=10.3,
    )
    base.add_figure(
        doc,
        IMG / "sunrise_swagger.png",
        "Рисунок 5 — Спецификация Sunrise/sunset API в Swagger UI",
        max_w=17.0,
        max_h=13.0,
    )
    base.add_para(
        doc,
        "Интерфейс отображает отредактированное описание, сервер "
        "https://api.sunrise-sunset.org и операцию GET /json.",
        after=4,
    )


# ---------------------------------------------------------------------------
# Сборка
# ---------------------------------------------------------------------------

TOC = [
    ("ЗАНЯТИЕ 1. ТЕСТИРОВАНИЕ API DUMMYJSON В POSTMAN", r"ЗАНЯТИЕ\s+1", 0),
    ("Настройте POSTMAN", r"Настройте\s+POSTMAN", 1),
    ("Тестовый маршрут. Проверьте, работает ли ваш интернет", r"Тестовый\s+маршрут", 1),
    ("Основные сущности", r"Основные\s+сущности", 1),
    ("Выборка данных с параметром select", r"Выборка\s+данных", 1),
    ("Выборка записи по идентификатору", r"Выборка\s+записи", 1),
    ("Фильтрация данных с параметром filter", r"Фильтрация\s+данных", 1),
    ("Поиск данных с параметром search", r"Поиск\s+данных", 1),
    ("Сортировка данных", r"Сортировка\s+данных", 1),
    ("Пагинация данных", r"Пагинация\s+данных", 1),
    ("Вложенные ресурсы", r"Вложенные\s+ресурсы", 1),
    ("Получение публикаций по тегу", r"Получение\s+публикаций", 1),
    ("Добавление", r"^Добавление\.?$", 1),
    ("Обновление", r"^Обновление\.?$", 1),
    ("Удаление", r"^Удаление\.", 1),
    ("Резервная копия коллекции", r"Создайте\s+резервную", 1),
    ("ЗАНЯТИЕ 2. ИСПОЛЬЗОВАНИЕ API OPENWEATHERMAP", r"ЗАНЯТИЕ\s+2", 0),
    ("Исследование API и ключ авторизации", r"Исследование\s+API", 1),
    ("Текущая погода", r"Текущая\s+погода", 1),
    ("Прогноз на 5 дней", r"Прогноз\s+на\s+5", 1),
    ("Запросы в curl", r"Запросы\s+в\s+curl", 1),
    ("AJAX-запрос и визуализация", r"AJAX-запрос", 1),
    ("Анализ полезной нагрузки", r"Анализ\s+полезной", 1),
    ("ЗАНЯТИЕ 3. ДОКУМЕНТИРОВАНИЕ КОНЕЧНОЙ ТОЧКИ API", r"ЗАНЯТИЕ\s+3", 0),
    ("Что не так в разделе API?", r"Что\s+не\s+так", 1),
    ("Поиск опен-сорс проекта", r"Поиск\s+опен-сорс", 1),
    ("Оценка ключевых элементов API документации", r"Оценка\s+ключевых", 1),
    ("ЗАНЯТИЕ 4. OPENAPI И SWAGGER", r"ЗАНЯТИЕ\s+4", 0),
    ("Знакомство со Swagger с помощью Petstore Demo", r"Знакомство\s+со\s+Swagger", 1),
    ("Редакция существующей спецификации API", r"Редакция\s+существующей", 1),
    ("Наполняем интерфейс Swagger", r"Наполняем\s+интерфейс", 1),
]


def build_document(pages=None):
    pages = pages or {}
    doc = Document()
    section = doc.sections[0]
    section.page_width = Cm(21.0)
    section.page_height = Cm(29.7)
    section.left_margin = Cm(2.5)
    section.right_margin = Cm(1.5)
    section.top_margin = Cm(2.0)
    section.bottom_margin = Cm(2.0)
    section.header_distance = Cm(1.0)
    section.footer_distance = Cm(1.0)

    normal = doc.styles["Normal"]
    normal.font.name = FONT_BODY
    normal.font.size = Pt(14)
    normal.paragraph_format.line_spacing = 1.5
    normal.paragraph_format.space_after = Pt(0)
    normal.paragraph_format.first_line_indent = Cm(1.25)
    normal.paragraph_format.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    rpr = normal.element.get_or_add_rPr()
    rfonts = rpr.find(base.qn("w:rFonts"))
    if rfonts is None:
        rfonts = base.OxmlElement("w:rFonts")
        rpr.insert(0, rfonts)
    for attr in ("w:ascii", "w:hAnsi", "w:cs", "w:eastAsia"):
        rfonts.set(base.qn(attr), FONT_BODY)

    section.different_first_page_header_footer = True
    base.add_page_number_footer(section)

    doc.core_properties.title = "Отчёт по практическим занятиям: проектирование API (v2)"
    doc.core_properties.subject = "Технология разработки программного обеспечения"

    add_title_page(doc)
    add_toc(doc, pages)
    add_lesson1(doc)
    add_lesson2(doc)
    add_lesson3(doc)
    add_lesson4(doc)

    doc.save(str(OUT_DOCX))
    return doc


def convert_pdf():
    if OUT_PDF.exists():
        OUT_PDF.unlink()
    profile = ROOT / "build" / "lo_profile_api_v2"
    profile.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        [
            "soffice",
            "--headless",
            "--norestore",
            f"-env:UserInstallation=file://{profile}",
            "--convert-to",
            "pdf",
            "--outdir",
            str(ROOT),
            str(OUT_DOCX),
        ],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0 or not OUT_PDF.exists():
        print(result.stdout)
        print(result.stderr)
        raise RuntimeError("LibreOffice conversion failed")


def pages_text():
    result = subprocess.run(
        ["pdftotext", "-layout", str(OUT_PDF), "-"],
        capture_output=True,
        text=True,
        check=True,
    )
    return [p for p in result.stdout.split("\f") if p.strip()]


def find_page(pages, pattern):
    rx = re.compile(pattern, re.IGNORECASE | re.MULTILINE)
    for idx, text in enumerate(pages, 1):
        if idx < 3:
            continue
        if rx.search(text):
            return idx
    return None


def build_pages(pages):
    mapping = {}
    for title, pattern, level in TOC:
        page = find_page(pages, pattern)
        if page is None:
            print("Не найден заголовок:", title, pattern)
        else:
            mapping[pattern] = page
    return mapping


def main():
    build_document()
    convert_pdf()
    pages = pages_text()
    mapping = build_pages(pages)
    print("Страниц:", len(pages))
    for title, pattern, level in TOC:
        print(f"{mapping.get(pattern, '??'):>3}  {title}")

    if all(p in mapping for _, p, _ in TOC):
        build_document(mapping)
        convert_pdf()
        pages2 = pages_text()
        mapping2 = build_pages(pages2)
        if mapping != mapping2:
            print("Пересборка оглавления...")
            build_document(mapping2)
            convert_pdf()
    print("Готово:", OUT_PDF)


if __name__ == "__main__":
    main()
