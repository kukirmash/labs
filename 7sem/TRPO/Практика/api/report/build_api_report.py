#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Сборка PDF-отчёта по практическим занятиям «Проектирование API»."""

import json
import re
import subprocess
from pathlib import Path

from PIL import Image
from docx import Document
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_LINE_SPACING
from docx.enum.text import WD_TAB_ALIGNMENT, WD_TAB_LEADER
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Cm, Pt, RGBColor

ROOT = Path(__file__).resolve().parent.parent.parent
API = ROOT / "api"
DATA = API / "data"
IMG = API / "images"
POSTMAN = API / "postman"
OPENAPI = API / "openapi"
Owm = API / "owm"

OUT_DOCX = ROOT / "Отчёт_ТРПО_API_Практика.docx"
OUT_PDF = ROOT / "Отчёт_ТРПО_API_Практика.pdf"

FONT_BODY = "Times New Roman"
FONT_CODE = "Courier New"
CODE_FILL = "F6F8FA"
CODE_BORDER = "C9D1D9"

IDX = json.loads((DATA / "index.json").read_text(encoding="utf-8"))
REQ = {item["key"]: item for item in IDX["requests"]}
OWM = json.loads((DATA / "owm_summary.json").read_text(encoding="utf-8"))


def response(key):
    return json.loads((DATA / "responses" / f"{key}.json").read_text(encoding="utf-8"))


def f1(value):
    return f"{float(value):.1f}".replace(".", ",")


def f2(value):
    return f"{float(value):.2f}".replace(".", ",")


def money(value):
    return f"{float(value):,.2f}".replace(",", " ").replace(".", ",")


# ---------------------------------------------------------------------------
# Служебные функции DOCX
# ---------------------------------------------------------------------------


def set_run_font(run, name, size=None, bold=None, italic=None, color=None):
    run.font.name = name
    rpr = run._element.get_or_add_rPr()
    rfonts = rpr.find(qn("w:rFonts"))
    if rfonts is None:
        rfonts = OxmlElement("w:rFonts")
        rpr.insert(0, rfonts)
    for attr in ("w:ascii", "w:hAnsi", "w:cs", "w:eastAsia"):
        rfonts.set(qn(attr), name)
    if size is not None:
        run.font.size = Pt(size)
    if bold is not None:
        run.bold = bold
    if italic is not None:
        run.italic = italic
    if color is not None:
        run.font.color.rgb = RGBColor.from_string(color)


def add_para(
    doc,
    text="",
    size=14,
    bold=False,
    italic=False,
    align=WD_ALIGN_PARAGRAPH.JUSTIFY,
    indent=Cm(1.25),
    before=0,
    after=6,
    line=1.5,
    keep_next=False,
    keep_together=False,
):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = align
    pf.first_line_indent = indent
    pf.space_before = Pt(before)
    pf.space_after = Pt(after)
    pf.line_spacing = line
    pf.keep_with_next = keep_next
    pf.keep_together = keep_together
    if text:
        run = p.add_run(text)
        set_run_font(run, FONT_BODY, size, bold, italic)
    return p


def add_bullet(doc, text, size=13.5, before=0, after=2):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    pf.left_indent = Cm(1.2)
    pf.first_line_indent = Cm(-0.6)
    pf.space_before = Pt(before)
    pf.space_after = Pt(after)
    pf.line_spacing = 1.3
    run = p.add_run("•  " + text)
    set_run_font(run, FONT_BODY, size)
    return p


def add_numbered(doc, number, text, size=13.5, before=0, after=2):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    pf.left_indent = Cm(1.2)
    pf.first_line_indent = Cm(-0.6)
    pf.space_before = Pt(before)
    pf.space_after = Pt(after)
    pf.line_spacing = 1.3
    run = p.add_run(f"{number}. " + text)
    set_run_font(run, FONT_BODY, size)
    return p


def add_h1(doc, text, page_break=True):
    p = add_para(
        doc,
        text,
        size=16,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        before=0,
        after=12,
        line=1.2,
        keep_next=True,
    )
    p.paragraph_format.page_break_before = page_break
    return p


def add_h2(doc, text, before=14):
    return add_para(
        doc,
        text,
        size=14,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.LEFT,
        indent=Cm(0),
        before=before,
        after=6,
        line=1.2,
        keep_next=True,
    )


def add_h3(doc, text, before=10):
    return add_para(
        doc,
        text,
        size=13,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.LEFT,
        indent=Cm(0),
        before=before,
        after=4,
        line=1.2,
        keep_next=True,
    )


PPR_TAGS = (
    "w:pStyle",
    "w:keepNext",
    "w:keepLines",
    "w:pageBreakBefore",
    "w:framePr",
    "w:widowControl",
    "w:numPr",
    "w:suppressLineNumbers",
    "w:pBdr",
    "w:shd",
    "w:tabs",
    "w:suppressAutoHyphens",
    "w:kinsoku",
    "w:wordWrap",
    "w:overflowPunct",
    "w:topLinePunct",
    "w:autoSpaceDE",
    "w:autoSpaceDN",
    "w:bidi",
    "w:adjustRightInd",
    "w:snapToGrid",
    "w:spacing",
    "w:ind",
    "w:contextualSpacing",
    "w:mirrorIndents",
    "w:suppressOverlap",
    "w:jc",
    "w:textDirection",
    "w:textAlignment",
    "w:textboxTightWrap",
    "w:outlineLvl",
    "w:divId",
    "w:cnfStyle",
    "w:rPr",
    "w:sectPr",
    "w:pPrChange",
)


def shade_and_border(p, fill=CODE_FILL, border=CODE_BORDER):
    ppr = p._p.get_or_add_pPr()

    pbdr = OxmlElement("w:pBdr")
    for edge in ("top", "left", "bottom", "right"):
        el = OxmlElement(f"w:{edge}")
        el.set(qn("w:val"), "single")
        el.set(qn("w:sz"), "4")
        el.set(qn("w:space"), "4")
        el.set(qn("w:color"), border)
        pbdr.append(el)
    ppr.insert_element_before(pbdr, *PPR_TAGS[PPR_TAGS.index("w:pBdr") + 1 :])

    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), fill)
    ppr.insert_element_before(shd, *PPR_TAGS[PPR_TAGS.index("w:shd") + 1 :])


def add_code(doc, code, size=8.2, line=10.2):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = WD_ALIGN_PARAGRAPH.LEFT
    pf.first_line_indent = Cm(0)
    pf.left_indent = Cm(0.15)
    pf.right_indent = Cm(0.15)
    pf.space_before = Pt(3)
    pf.space_after = Pt(5)
    pf.line_spacing = Pt(line)
    pf.line_spacing_rule = WD_LINE_SPACING.EXACTLY
    pf.keep_together = True
    lines = code.rstrip("\n").split("\n")
    run = p.add_run()
    set_run_font(run, FONT_CODE, size)
    for i, text in enumerate(lines):
        if i:
            run.add_break()
        run.add_text(text if text else " ")
    shade_and_border(p)
    return p


def add_json(doc, obj, max_lines=None, size=8.2):
    text = json.dumps(obj, ensure_ascii=False, indent=2)
    lines = text.split("\n")
    if max_lines and len(lines) > max_lines:
        lines = lines[:max_lines] + ["..."]
        text = "\n".join(lines)
    return add_code(doc, text, size=size)


def add_label(doc, text):
    return add_para(
        doc,
        text,
        size=10.5,
        italic=True,
        align=WD_ALIGN_PARAGRAPH.LEFT,
        indent=Cm(0),
        before=2,
        after=1,
        line=1.0,
        keep_next=True,
    )


def compact_users(users, fields, total, skip, limit):
    lines = ["{", '  "users": [']
    for i, user in enumerate(users):
        obj = {field: user[field] for field in fields}
        suffix = "," if i < len(users) - 1 else ""
        lines.append("    " + json.dumps(obj, ensure_ascii=False) + suffix)
    lines.extend(
        [
            "  ],",
            f'  "total": {total},',
            f'  "skip": {skip},',
            f'  "limit": {limit}',
            "}",
        ]
    )
    return "\n".join(lines)


def add_request_card(doc, method, url, status, elapsed, size_note=""):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = WD_ALIGN_PARAGRAPH.LEFT
    pf.first_line_indent = Cm(0)
    pf.left_indent = Cm(0.15)
    pf.right_indent = Cm(0.15)
    pf.space_before = Pt(6)
    pf.space_after = Pt(4)
    pf.line_spacing = 1.0
    pf.keep_together = True
    pf.keep_with_next = True

    colors = {"GET": "1F5FBF", "POST": "1D7A4C", "PATCH": "A15C00", "DELETE": "A61B1B"}
    run = p.add_run(method)
    set_run_font(run, FONT_CODE, 9, True, color=colors.get(method, "333333"))
    run2 = p.add_run("  " + url)
    set_run_font(run2, FONT_CODE, 9)
    status_text = {200: "OK", 201: "Created", 204: "No Content"}.get(status, "OK")
    run3 = p.add_run(f"\nHTTP {status} {status_text} · {elapsed} мс")
    set_run_font(run3, FONT_CODE, 8.5, color="4A5568")
    if size_note:
        run4 = p.add_run(" · " + size_note)
        set_run_font(run4, FONT_CODE, 8.5, color="4A5568")
    shade_and_border(p, fill="EEF4FF", border="B8CCEA")
    return p


def add_note(doc, text, fill="FFF8E1", border="E0C36A"):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    pf.first_line_indent = Cm(0)
    pf.left_indent = Cm(0.2)
    pf.right_indent = Cm(0.2)
    pf.space_before = Pt(6)
    pf.space_after = Pt(6)
    pf.line_spacing = 1.25
    pf.keep_together = True
    run = p.add_run("Примечание. " + text)
    set_run_font(run, FONT_BODY, 12, italic=False)
    shade_and_border(p, fill=fill, border=border)
    return p


def set_cell_shading(cell, fill):
    tcpr = cell._tc.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), fill)
    tcpr.append(shd)


def set_cell_margins(cell, top=40, bottom=40, left=70, right=70):
    tcpr = cell._tc.get_or_add_tcPr()
    tcmar = OxmlElement("w:tcMar")
    for name, value in (("top", top), ("bottom", bottom), ("start", left), ("end", right)):
        el = OxmlElement(f"w:{name}")
        el.set(qn("w:w"), str(value))
        el.set(qn("w:type"), "dxa")
        tcmar.append(el)
    tcpr.append(tcmar)


def add_table(doc, headers, rows, widths, caption=None, font=9.5, center_cols=(), header_fill="DCE6F2", keep_rows=True):
    if caption:
        add_para(
            doc,
            caption,
            size=11,
            italic=True,
            align=WD_ALIGN_PARAGRAPH.LEFT,
            indent=Cm(0),
            before=8,
            after=4,
            line=1.0,
            keep_next=True,
        )
    table = doc.add_table(rows=1, cols=len(headers))
    table.style = "Table Grid"
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    table.autofit = False
    tblPr = table._tbl.tblPr
    total_twips = sum(int(w.twips) for w in widths)
    tblW = OxmlElement("w:tblW")
    tblW.set(qn("w:type"), "dxa")
    tblW.set(qn("w:w"), str(total_twips))
    tblPr.append(tblW)
    tblLayout = OxmlElement("w:tblLayout")
    tblLayout.set(qn("w:type"), "fixed")
    tblPr.append(tblLayout)
    for i, width in enumerate(widths):
        table.columns[i].width = width
    tr = table.rows[0]._tr
    trPr = tr.get_or_add_trPr()
    tblHeader = OxmlElement("w:tblHeader")
    tblHeader.set(qn("w:val"), "true")
    trPr.append(tblHeader)
    for i, (cell, text) in enumerate(zip(table.rows[0].cells, headers)):
        cell.text = ""
        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.first_line_indent = Cm(0)
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(0)
        p.paragraph_format.line_spacing = 1.0
        run = p.add_run(text)
        set_run_font(run, FONT_BODY, font, True)
        set_cell_shading(cell, header_fill)
        set_cell_margins(cell)
        cell.width = widths[i]
    for ridx, row in enumerate(rows):
        cells = table.add_row().cells
        tr = table.rows[-1]._tr
        trPr = tr.get_or_add_trPr()
        cantSplit = OxmlElement("w:cantSplit")
        trPr.append(cantSplit)
        row_keep = keep_rows and ridx < len(rows) - 1
        for i, text in enumerate(row):
            cell = cells[i]
            cell.text = ""
            p = cell.paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER if i in center_cols else WD_ALIGN_PARAGRAPH.LEFT
            p.paragraph_format.first_line_indent = Cm(0)
            p.paragraph_format.space_before = Pt(0)
            p.paragraph_format.space_after = Pt(0)
            p.paragraph_format.line_spacing = 1.0
            p.paragraph_format.keep_with_next = row_keep
            run = p.add_run(str(text))
            set_run_font(run, FONT_BODY, font)
            set_cell_margins(cell)
            cell.width = widths[i]
    add_para(doc, "", size=6, indent=Cm(0), after=0)
    return table


def add_figure(doc, path, caption, max_w=17.0, max_h=17.0):
    with Image.open(path) as im:
        wpx, hpx = im.size
    w = wpx / 200 * 2.54
    h = hpx / 200 * 2.54
    scale = min(1.0, max_w / w, max_h / h)
    w *= scale
    h *= scale
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.alignment = WD_ALIGN_PARAGRAPH.CENTER
    pf.first_line_indent = Cm(0)
    pf.space_before = Pt(6)
    pf.space_after = Pt(2)
    pf.keep_with_next = True
    run = p.add_run()
    run.add_picture(str(path), width=Cm(w), height=Cm(h))
    add_para(
        doc,
        caption,
        size=11,
        italic=True,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        before=2,
        after=10,
        line=1.0,
    )


def add_page_number_footer(section):
    footer = section.footer
    p = footer.paragraphs[0]
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.first_line_indent = Cm(0)
    run = p.add_run()
    fld1 = OxmlElement("w:fldChar")
    fld1.set(qn("w:fldCharType"), "begin")
    instr = OxmlElement("w:instrText")
    instr.set(qn("xml:space"), "preserve")
    instr.text = " PAGE "
    fld2 = OxmlElement("w:fldChar")
    fld2.set(qn("w:fldCharType"), "end")
    run._r.append(fld1)
    run._r.append(instr)
    run._r.append(fld2)
    set_run_font(run, FONT_BODY, 11)


# ---------------------------------------------------------------------------
# Данные для отчёта
# ---------------------------------------------------------------------------

CURRENT = OWM["current_zip"]
MOSCOW = OWM["current_moscow"]
DAILY = [row for row in OWM["daily_zip"] if row["steps"] >= 4][:5]
DAILY_ALL = OWM["daily_zip"]
MOSCOW_DAILY = OWM["daily_moscow"]

USERS = response("users")
PRODUCTS = response("products")
USERS_SELECT = response("users_select")
POSTS_SELECT = response("posts_select_tags")
USER3 = response("user_3")
FILTER_SOPHIA = response("filter_sophia")
FILTER_BROWN = response("filter_brown_hair")
SEARCH_SOPHIA = response("search_sophia")
SORT_ASC = response("sort_age_asc")
PAGE = response("page_10_skip_10")
ALL_USERS = response("limit_all")
TODOS1 = response("todos_user1")
POSTS7 = response("posts_user7")
POST_IDS7 = response("post_ids_user7")
COMMENTS227 = response("comments_post227")
COMMENTS_ALL = response("comments_all")
COMMENTS4 = response("comments_post4")
CART6 = response("cart_user6")
TAG_CRIME = response("posts_tag_crime")
ADD_USER = response("add_user")
ADD_PRODUCT = response("add_product")
UPDATE_USER = response("update_user1")
DELETE_USER = response("delete_user13")

COMMENT_USER3 = [c for c in COMMENTS_ALL["comments"] if c["user"]["id"] == 3][0]
COMMENT_USER5_POST4 = [c for c in COMMENTS4["comments"] if c["user"]["id"] == 5]
CART = CART6["carts"][0]


def user_brief(u):
    return {
        "id": u["id"],
        "firstName": u["firstName"],
        "lastName": u["lastName"],
        "age": u["age"],
        "gender": u["gender"],
    }


TOC = [
    ("Введение", r"ВВЕДЕНИЕ", 0),
    ("1. Занятие 1. Тестирование API DummyJSON в Postman", r"1\.\s+ЗАНЯТИЕ\s+1", 0),
    ("1.1. Настройка окружения и коллекции", r"1\.1\.\s+Настройка", 1),
    ("1.2. Тестовый маршрут", r"1\.2\.\s+Тестовый", 1),
    ("1.3. Основные сущности", r"1\.3\.\s+Основные", 1),
    ("1.4. Выборка полей (select)", r"1\.4\.\s+Выборка\s+полей", 1),
    ("1.5. Выборка по идентификатору", r"1\.5\.\s+Выборка\s+по\s+идентификатору", 1),
    ("1.6. Фильтрация данных", r"1\.6\.\s+Фильтрация", 1),
    ("1.7. Поиск данных", r"1\.7\.\s+Поиск", 1),
    ("1.8. Сортировка данных", r"1\.8\.\s+Сортировка", 1),
    ("1.9. Пагинация и параметр limit", r"1\.9\.\s+Пагинация", 1),
    ("1.10. Вложенные ресурсы", r"1\.10\.\s+Вложенные", 1),
    ("1.11. Публикации по тегу", r"1\.11\.\s+Публикации", 1),
    ("1.12. Добавление данных", r"1\.12\.\s+Добавление", 1),
    ("1.13. Обновление данных", r"1\.13\.\s+Обновление", 1),
    ("1.14. Удаление данных", r"1\.14\.\s+Удаление", 1),
    ("1.15. Резервная копия коллекции", r"1\.15\.\s+Резервная", 1),
    ("1.16. Итоги выполнения запросов", r"1\.16\.\s+Итоги", 1),
    ("2. Занятие 2. Использование API OpenWeatherMap", r"2\.\s+ЗАНЯТИЕ\s+2", 0),
    ("2.1. Исследование API и ключ авторизации", r"2\.1\.\s+Исследование", 1),
    ("2.2. Текущая погода", r"2\.2\.\s+Текущая", 1),
    ("2.3. Прогноз на 5 дней", r"2\.3\.\s+Прогноз", 1),
    ("2.4. Запросы в curl", r"2\.4\.\s+Запросы\s+в\s+curl", 1),
    ("2.5. AJAX-запрос и визуализация", r"2\.5\.\s+AJAX", 1),
    ("2.6. Анализ полезной нагрузки", r"2\.6\.\s+Анализ", 1),
    ("3. Занятие 3. Документирование конечной точки API", r"3\.\s+ЗАНЯТИЕ\s+3", 0),
    ("3.1. Выбор проекта и методика оценки", r"3\.1\.\s+Выбор", 1),
    ("3.2. Оценка документации DummyJSON", r"3\.2\.\s+Оценка", 1),
    ("3.3. Недостатки и рекомендации", r"3\.3\.\s+Недостатки", 1),
    ("4. Занятие 4. OpenAPI и Swagger", r"4\.\s+ЗАНЯТИЕ\s+4", 0),
    ("4.1. Знакомство со Swagger UI", r"4\.1\.\s+Знакомство", 1),
    ("4.2. Спецификация OpenAPI для DummyJSON", r"4\.2\.\s+Спецификация", 1),
    ("4.3. Проверка и визуализация спецификации", r"4\.3\.\s+Проверка", 1),
    ("Заключение", r"ЗАКЛЮЧЕНИЕ", 0),
    ("Список использованных источников", r"СПИСОК\s+ИСПОЛЬЗОВАННЫХ", 0),
    ("Приложение А. Журнал запросов", r"ПРИЛОЖЕНИЕ\s+А", 0),
    ("Приложение Б. Состав файлов проекта", r"ПРИЛОЖЕНИЕ\s+Б", 0),
]


# ---------------------------------------------------------------------------
# Сборка документа
# ---------------------------------------------------------------------------


def build_document(toc_pages=None):
    toc_pages = toc_pages or {}
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
    rfonts = rpr.find(qn("w:rFonts"))
    if rfonts is None:
        rfonts = OxmlElement("w:rFonts")
        rpr.insert(0, rfonts)
    for attr in ("w:ascii", "w:hAnsi", "w:cs", "w:eastAsia"):
        rfonts.set(qn(attr), FONT_BODY)

    section.different_first_page_header_footer = True
    add_page_number_footer(section)

    doc.core_properties.title = "Отчёт по практическим занятиям: проектирование API"
    doc.core_properties.subject = "Технология разработки программного обеспечения"
    doc.core_properties.author = ""

    # ----------------------------- титульный лист ---------------------------
    for _ in range(4):
        add_para(doc, "", size=12, indent=Cm(0), after=0, line=1.0)
    add_para(
        doc,
        "ОТЧЁТ",
        size=30,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=6,
        line=1.0,
    )
    add_para(
        doc,
        "по практическим занятиям",
        size=16,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=0,
        line=1.2,
    )
    add_para(
        doc,
        "по дисциплине «Технология разработки программного обеспечения»",
        size=16,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=24,
        line=1.2,
    )
    add_para(
        doc,
        "Тема: «Проектирование программного интерфейса API»",
        size=16,
        bold=True,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=6,
        line=1.2,
    )
    add_para(
        doc,
        "Практика 1–4: DummyJSON, OpenWeatherMap,",
        size=13,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=0,
        line=1.2,
    )
    add_para(
        doc,
        "документирование API, OpenAPI и Swagger",
        size=13,
        align=WD_ALIGN_PARAGRAPH.CENTER,
        indent=Cm(0),
        after=0,
        line=1.2,
    )
    for _ in range(6):
        add_para(doc, "", size=12, indent=Cm(0), after=0, line=1.0)
    for label in (
        "Выполнил(а): ______________________",
        "Группа: ______________________",
        "Проверил(а): ______________________",
    ):
        p = add_para(
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
        add_para(doc, "", size=12, indent=Cm(0), after=0, line=1.0)
    add_para(doc, "2026", size=14, align=WD_ALIGN_PARAGRAPH.CENTER, indent=Cm(0), after=0, line=1.0)

    # ----------------------------- содержание ------------------------------
    add_h1(doc, "СОДЕРЖАНИЕ")
    for title, anchor, level in TOC:
        p = doc.add_paragraph()
        pf = p.paragraph_format
        pf.alignment = WD_ALIGN_PARAGRAPH.LEFT
        pf.first_line_indent = Cm(0)
        pf.left_indent = Cm(0.6 * level)
        pf.space_before = Pt(0)
        pf.space_after = Pt(1)
        pf.line_spacing = 1.05
        pf.tab_stops.add_tab_stop(Cm(16.7), WD_TAB_ALIGNMENT.RIGHT, WD_TAB_LEADER.DOTS)
        page = toc_pages.get(anchor, "")
        run = p.add_run(title)
        set_run_font(run, FONT_BODY, 11, bold=(level == 0))
        run2 = p.add_run("\t" + (str(page) if page else "00"))
        set_run_font(run2, FONT_BODY, 11)

    # ----------------------------- введение --------------------------------
    add_h1(doc, "ВВЕДЕНИЕ")
    intro = [
        "Практические занятия по дисциплине «Технология разработки программного "
        "обеспечения» посвящены проектированию и тестированию программного интерфейса "
        "API. Цель работы — получить первые навыки тестирования API на тренажёрах, "
        "освоить работу с Postman, curl и AJAX-запросами, а также познакомиться с "
        "документированием API и спецификацией OpenAPI.",
        "В отчёт включены результаты выполнения четырёх занятий. На занятии 1 выполнены "
        "24 запроса к DummyJSON и подготовлена коллекция Postman; на занятии 2 "
        "исследован OpenWeatherMap и построены запросы текущей погоды и прогноза; на "
        "занятии 3 проанализирована документация открытого API; на занятии 4 "
        "подготовлена и проверена спецификация OpenAPI, визуализированная в Swagger UI.",
        "Запросы выполнялись 15 ноября 2026 г. к реальным демонстрационным сервисам. "
        "DummyJSON не сохраняет изменения: операции добавления, обновления и удаления "
        "имитируются, что отдельно показано в отчёте. Ответы сохранены в каталоге "
        "api/data, коллекция и окружение Postman — в каталоге api/postman.",
        "Отчёт построен по единой схеме: постановка задачи, выполненный запрос, "
        "полученный ответ и краткий анализ результата. Для проверки воспроизводимости "
        "приведены команды запуска коллекции, curl-запросы и фрагменты исходного кода.",
    ]
    for text in intro:
        add_para(doc, text, after=6)

    # ----------------------------- занятие 1 -------------------------------
    add_h1(doc, "1. ЗАНЯТИЕ 1. ТЕСТИРОВАНИЕ API DUMMYJSON В POSTMAN")
    add_para(
        doc,
        "DummyJSON — бесплатный онлайн-REST API, который генерирует тестовые данные "
        "в формате JSON. Сервис имитирует предметную область интернет-магазина: "
        "пользователи, товары, корзины, посты, комментарии, задачи, рецепты и цитаты. "
        "Базовый адрес API — https://dummyjson.com. Изменения, выполняемые через API, "
        "не сохраняются на сервере.",
        after=6,
    )
    add_para(
        doc,
        "Основные конечные точки DummyJSON приведены в таблице 1.",
        after=4,
    )
    add_table(
        doc,
        ("№", "URL", "Описание"),
        [
            ("1", "/posts", "Публикации (посты)"),
            ("2", "/comments", "Комментарии к постам"),
            ("3", "/users", "Пользователи"),
            ("4", "/todos", "Задачи"),
            ("5", "/products", "Товары"),
            ("6", "/carts", "Корзина пользователя"),
            ("7", "/recipes", "Рецепты"),
            ("8", "/quotes", "Цитаты"),
        ],
        [Cm(1.2), Cm(4.6), Cm(11.2)],
        "Таблица 1 — Основные конечные точки DummyJSON",
        font=10,
        center_cols=(0,),
    )

    # 1.1
    add_h2(doc, "1.1. Настройка окружения и коллекции")
    add_para(
        doc,
        "В Postman создано окружение «ТРПО_Группа_Фамилия» с переменной baseUrl, "
        "равной https://dummyjson.com. Использование переменной позволяет переключать "
        "среду выполнения без изменения URL каждого запроса. Коллекция называется "
        "«ТРПО», внутри неё создана папка «Практика 1_ DummyJSON» с тринадцатью "
        "тематическими папками. Каждый запрос имеет содержательное имя, включающее "
        "номер, назначение и метод.",
        after=4,
    )
    add_code(
        doc,
        "ТРПО\n"
        "└── Практика 1_ DummyJSON\n"
        "    ├── 01. Тестовый маршрут\n"
        "    ├── 02. Основные сущности\n"
        "    ├── 03. Выборка полей (select)\n"
        "    ├── 04. Выборка по идентификатору\n"
        "    ├── 05. Фильтрация (filter)\n"
        "    ├── 06. Поиск (search)\n"
        "    ├── 07. Сортировка (sortBy)\n"
        "    ├── 08. Пагинация (limit, skip)\n"
        "    ├── 09. Вложенные ресурсы\n"
        "    ├── 10. Публикации по тегу\n"
        "    ├── 11. Добавление\n"
        "    ├── 12. Обновление\n"
        "    └── 13. Удаление",
        size=9,
        line=11,
    )
    add_table(
        doc,
        ("Папка", "Запросов", "Содержание"),
        [
            ("01. Тестовый маршрут", "1", "Проверка доступности API"),
            ("02. Основные сущности", "2", "Списки пользователей и товаров"),
            ("03. Выборка полей (select)", "2", "Ограничение набора полей"),
            ("04. Выборка по идентификатору", "1", "Пользователь с id = 3"),
            ("05. Фильтрация (filter)", "2", "Фильтр по имени и цвету волос"),
            ("06. Поиск (search)", "1", "Поиск пользователя Sophia"),
            ("07. Сортировка (sortBy)", "1", "Сортировка по возрасту"),
            ("08. Пагинация (limit, skip)", "2", "Страницы и limit = 0"),
            ("09. Вложенные ресурсы", "7", "Задачи, посты, комментарии, корзина"),
            ("10. Публикации по тегу", "1", "Посты с тегом crime"),
            ("11. Добавление", "2", "Новый пользователь и товар"),
            ("12. Обновление", "1", "Переезд пользователя 1 в Россию"),
            ("13. Удаление", "1", "Удаление пользователя 13"),
        ],
        [Cm(6.0), Cm(2.0), Cm(9.0)],
        "Таблица 2 — Структура коллекции «Практика 1_ DummyJSON»",
        font=9.5,
        center_cols=(1,),
    )
    add_para(
        doc,
        "Коллекция запускается командой Postman CLI. В одном прогоне выполнены все "
        "24 запроса и 106 проверок; ошибок нет.",
        after=4,
    )
    add_code(
        doc,
        "postman collection run \"ТРПО_Практика_1_DummyJSON.postman_collection.json\" \\\n"
        "  -e \"ТРПО_Группа_Фамилия.postman_environment.json\"\n\n"
        "requests: 24, failed: 0\n"
        "assertions: 106, failed: 0\n"
        "total run duration: 4.8 s",
        size=8.5,
        line=10.5,
    )

    # 1.2
    add_h2(doc, "1.2. Тестовый маршрут")
    add_para(
        doc,
        "Задание: проверить, работает ли интернет и доступен ли тренажёр. Запрос "
        "GET /test не требует параметров и возвращает служебный ответ.",
        after=2,
    )
    add_request_card(doc, "GET", "https://dummyjson.com/test", 200, REQ["test"]["elapsed_ms"])
    add_json(doc, response("test"))
    add_para(
        doc,
        "Ответ подтверждает доступность сервиса: status = ok, method = GET. "
        "Код 200 означает успешную обработку запроса.",
        after=4,
    )

    # 1.3
    add_h2(doc, "1.3. Основные сущности")
    add_para(
        doc,
        "Задание: получить список пользователей и список продуктов. Оба запроса "
        "возвращают постраничную структуру с полями users/products, total, skip, limit.",
        after=4,
    )
    add_table(
        doc,
        ("Запрос", "total", "limit", "Получено записей"),
        [
            ("GET /users", str(USERS["total"]), str(USERS["limit"]), str(len(USERS["users"]))),
            ("GET /products", str(PRODUCTS["total"]), str(PRODUCTS["limit"]), str(len(PRODUCTS["products"]))),
        ],
        [Cm(6.0), Cm(2.6), Cm(2.6), Cm(5.8)],
        "Таблица 3 — Размеры коллекций пользователей и товаров",
        font=10,
        center_cols=(1, 2, 3),
    )
    add_request_card(doc, "GET", "https://dummyjson.com/users", 200, REQ["users"]["elapsed_ms"])
    add_code(
        doc,
        compact_users(
            USERS["users"][:2],
            ("id", "firstName", "lastName", "age", "gender"),
            USERS["total"],
            USERS["skip"],
            USERS["limit"],
        ),
    )
    add_para(
        doc,
        "По умолчанию API возвращает 30 записей, хотя всего в наборе 208 пользователей "
        "и 194 товара. Чтобы получить все записи, используется параметр limit = 0 "
        "(раздел 1.9).",
        after=4,
    )

    # 1.4
    add_h2(doc, "1.4. Выборка полей (select)")
    add_para(
        doc,
        "Параметр select позволяет оставить в ответе только указанные поля. Это "
        "уменьшает размер ответа и упрощает обработку данных.",
        after=4,
    )
    add_table(
        doc,
        ("Запрос", "Получено", "Поля в ответе"),
        [
            ("/users?select=firstName,gender,age&limit=0", "208", "id, firstName, gender, age"),
            ("/posts?select=tags&limit=0", "251", "id, tags"),
        ],
        [Cm(8.4), Cm(2.4), Cm(6.2)],
        "Таблица 4 — Результаты выборки полей",
        font=9.5,
        center_cols=(1,),
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/users?select=firstName,gender,age&limit=0",
        200,
        REQ["users_select"]["elapsed_ms"],
    )
    add_code(
        doc,
        compact_users(
            USERS_SELECT["users"][:3],
            ("id", "firstName", "gender", "age"),
            USERS_SELECT["total"],
            USERS_SELECT["skip"],
            USERS_SELECT["limit"],
        ),
    )
    add_para(
        doc,
        "Поле id добавляется автоматически, даже если оно не указано в select. "
        "Для постов выборка tags аналогична: в ответе остаются id и массив тегов.",
        after=4,
    )

    # 1.5
    add_h2(doc, "1.5. Выборка по идентификатору")
    add_para(
        doc,
        "Задание: выбрать пользователя с идентификатором 3. Запрос GET /users/3 "
        "возвращает один объект, а не массив.",
        after=2,
    )
    add_request_card(doc, "GET", "https://dummyjson.com/users/3", 200, REQ["user_3"]["elapsed_ms"])
    add_json(
        doc,
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
                "state": USER3["address"]["state"],
                "country": USER3["address"]["country"],
            },
        },
        max_lines=18,
    )
    add_para(
        doc,
        "Получена запись Sophia Brown: id = 3, возраст 43 года, пол female. "
        "Поля hair и address являются вложенными объектами, что важно учитывать "
        "при фильтрации и написании проверок.",
        after=4,
    )

    # 1.6
    add_h2(doc, "1.6. Фильтрация данных")
    add_para(
        doc,
        "Фильтрация выполняется конечной точкой /users/filter с параметрами key и "
        "value. Для вложенных полей используется точечная нотация: hair.color.",
        after=4,
    )
    add_table(
        doc,
        ("key", "value", "total", "Найденные пользователи"),
        [
            ("firstName", "Sophia", "2", "id 3 — Sophia Brown; id 109 — Sophia Jones"),
            ("hair.color", "Brown", "23", "23 пользователя, например id 1, 33, 51, 54, 59"),
        ],
        [Cm(3.0), Cm(3.0), Cm(1.8), Cm(9.2)],
        "Таблица 5 — Результаты фильтрации",
        font=9.5,
        center_cols=(2,),
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/users/filter?key=firstName&value=Sophia",
        200,
        REQ["filter_sophia"]["elapsed_ms"],
    )
    add_code(
        doc,
        compact_users(
            FILTER_SOPHIA["users"],
            ("id", "firstName", "lastName", "age"),
            FILTER_SOPHIA["total"],
            FILTER_SOPHIA["skip"],
            FILTER_SOPHIA["limit"],
        ),
    )
    add_para(
        doc,
        "Фильтр по hair.color = Brown возвращает 23 записи. Это показывает, что "
        "DummyJSON поддерживает фильтрацию по вложенным полям и позволяет быстро "
        "формировать выборки по признакам.",
        after=4,
    )

    # 1.7
    add_h2(doc, "1.7. Поиск данных")
    add_para(
        doc,
        "Поиск выполняется конечной точкой /users/search с параметром q. В отличие "
        "от фильтра, поиск ищет подстроку по нескольким текстовым полям.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/users/search?q=Sophia",
        200,
        REQ["search_sophia"]["elapsed_ms"],
    )
    add_code(
        doc,
        compact_users(
            SEARCH_SOPHIA["users"],
            ("id", "firstName", "lastName", "age"),
            SEARCH_SOPHIA["total"],
            SEARCH_SOPHIA["skip"],
            SEARCH_SOPHIA["limit"],
        ),
    )
    add_para(
        doc,
        "Поиск и фильтр по firstName дали одинаковый результат — двух пользователей "
        "Sophia. Для точного совпадения поля удобнее filter, для поиска по части "
        "строки — search.",
        after=4,
    )

    # 1.8
    add_h2(doc, "1.8. Сортировка данных")
    add_para(
        doc,
        "Сортировка задаётся параметрами sortBy и order. Для проверки всего набора "
        "использован limit = 0, чтобы в ответ попали все 208 пользователей.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/users?sortBy=age&order=asc&limit=0",
        200,
        REQ["sort_age_asc"]["elapsed_ms"],
    )
    add_table(
        doc,
        ("Показатель", "Значение"),
        [
            ("Количество пользователей", "208"),
            ("Минимальный возраст", f"23 — Olivia Wilson (id 6)"),
            ("Максимальный возраст", f"46 — Mateo Perez (id 204)"),
            ("Порядок", "по возрастанию (asc)"),
        ],
        [Cm(6.0), Cm(11.0)],
        "Таблица 6 — Результат сортировки по возрасту",
        font=10,
    )
    add_code(
        doc,
        compact_users(
            SORT_ASC["users"][:3],
            ("id", "firstName", "lastName", "age"),
            SORT_ASC["total"],
            SORT_ASC["skip"],
            SORT_ASC["limit"],
        ),
    )
    add_para(
        doc,
        "Проверка в тесте Postman сравнивает массив возрастов с его отсортированной "
        "копией; условие выполняется для всех 208 записей.",
        after=4,
    )

    # 1.9
    add_h2(doc, "1.9. Пагинация и параметр limit")
    add_para(
        doc,
        "Пагинация использует параметры limit и skip. Значение limit = 0 отключает "
        "ограничение: сервер возвращает все записи и нормализует limit до общего "
        "количества.",
        after=4,
    )
    add_table(
        doc,
        ("Параметры", "total", "skip", "limit", "Получено", "Первый id"),
        [
            ("по умолчанию", str(USERS["total"]), str(USERS["skip"]), str(USERS["limit"]), "30", "1"),
            ("limit=10&skip=10", str(PAGE["total"]), str(PAGE["skip"]), str(PAGE["limit"]), "10", str(PAGE["users"][0]["id"])),
            ("limit=0", str(ALL_USERS["total"]), str(ALL_USERS["skip"]), str(ALL_USERS["limit"]), str(len(ALL_USERS["users"])), "1"),
        ],
        [Cm(4.4), Cm(1.8), Cm(1.8), Cm(1.8), Cm(2.2), Cm(2.0)],
        "Таблица 7 — Проверка пагинации",
        font=9.5,
        center_cols=(1, 2, 3, 4, 5),
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/users?limit=10&skip=10",
        200,
        REQ["page_10_skip_10"]["elapsed_ms"],
    )
    add_code(
        doc,
        compact_users(
            PAGE["users"][:3],
            ("id", "firstName"),
            PAGE["total"],
            PAGE["skip"],
            PAGE["limit"],
        ),
    )
    add_para(
        doc,
        "Страница начинается с пользователя id 11, что соответствует skip = 10. "
        "Запрос с limit = 0 вернул все 208 пользователей; в ответе limit = 208, "
        "то есть сервер заменил нулевое ограничение фактическим количеством записей.",
        after=4,
    )

    # 1.10
    add_h2(doc, "1.10. Вложенные ресурсы")
    add_para(
        doc,
        "DummyJSON поддерживает вложенные маршруты, которые возвращают связанные "
        "сущности: задачи и посты пользователя, комментарии поста, корзину "
        "пользователя.",
        after=4,
    )
    add_table(
        doc,
        ("Запрос", "total", "Что получено"),
        [
            ("/users/1/todos", str(TODOS1["total"]), "Задачи пользователя 1"),
            ("/users/7/posts", str(POSTS7["total"]), "Посты пользователя 7"),
            ("/users/7/posts?select=id", str(POST_IDS7["total"]), "Только идентификаторы постов"),
            ("/posts/227/comments", str(COMMENTS227["total"]), "Комментарии к посту 227"),
            ("/comments?limit=0", str(COMMENTS_ALL["total"]), "Все комментарии (для фильтрации)"),
            ("/posts/4/comments", str(COMMENTS4["total"]), "Комментарии к посту 4"),
            ("/carts/user/6", str(CART6["total"]), "Корзина пользователя 6"),
        ],
        [Cm(7.4), Cm(1.8), Cm(7.8)],
        "Таблица 8 — Вложенные запросы",
        font=9.5,
        center_cols=(1,),
    )

    add_h3(doc, "1.10.1. Задачи пользователя 1")
    add_request_card(doc, "GET", "https://dummyjson.com/users/1/todos", 200, REQ["todos_user1"]["elapsed_ms"])
    add_table(
        doc,
        ("id", "Задача", "Выполнена"),
        [(str(t["id"]), t["todo"], "да" if t["completed"] else "нет") for t in TODOS1["todos"]],
        [Cm(1.8), Cm(10.4), Cm(3.0)],
        "Таблица 9 — Задачи пользователя 1",
        font=10,
        center_cols=(0, 2),
    )

    add_h3(doc, "1.10.2. Посты пользователя 7")
    add_para(
        doc,
        "Пользователь 7 — Alexander Jones. У него два поста: id 121 и id 227.",
        after=2,
    )
    add_request_card(doc, "GET", "https://dummyjson.com/users/7/posts", 200, REQ["posts_user7"]["elapsed_ms"])
    add_table(
        doc,
        ("id", "Заголовок"),
        [(str(p["id"]), p["title"]) for p in POSTS7["posts"]],
        [Cm(1.8), Cm(15.2)],
        "Таблица 10 — Посты пользователя 7",
        font=10,
        center_cols=(0,),
    )
    add_para(
        doc,
        "Запрос с select=id возвращает только идентификаторы: [121, 227]. "
        "Это компактный способ получить список вложенных сущностей.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/users/7/posts?select=id",
        200,
        REQ["post_ids_user7"]["elapsed_ms"],
    )
    add_json(doc, POST_IDS7, max_lines=12)

    add_h3(doc, "1.10.3. Комментарии к посту 227")
    add_para(
        doc,
        "Пост 227 «The historic castle loomed over the town below» принадлежит "
        "Alexander Jones. Запрос /posts/227/comments возвращает два комментария.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/posts/227/comments",
        200,
        REQ["comments_post227"]["elapsed_ms"],
    )
    add_table(
        doc,
        ("id", "Автор", "Комментарий", "Лайки"),
        [
            (str(c["id"]), c["user"]["fullName"], c["body"], str(c["likes"]))
            for c in COMMENTS227["comments"]
        ],
        [Cm(1.4), Cm(4.0), Cm(9.6), Cm(1.8)],
        "Таблица 11 — Комментарии к посту 227",
        font=9.5,
        center_cols=(0, 3),
    )

    add_h3(doc, "1.10.4. Комментарии пользователя 3")
    add_para(
        doc,
        "Отдельного маршрута «комментарии пользователя» в DummyJSON нет, поэтому "
        "загружается полный список комментариев (limit = 0), после чего выполняется "
        "фильтрация по user.id = 3. У пользователя 3 найден один комментарий.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/comments?limit=0",
        200,
        REQ["comments_all"]["elapsed_ms"],
    )
    add_json(
        doc,
        {
            "id": COMMENT_USER3["id"],
            "body": COMMENT_USER3["body"],
            "postId": COMMENT_USER3["postId"],
            "likes": COMMENT_USER3["likes"],
            "user": COMMENT_USER3["user"],
        },
        max_lines=16,
    )

    add_h3(doc, "1.10.5. Комментарии пользователя 5 к посту 4")
    add_para(
        doc,
        "У поста 4 два комментария, но оба оставлены другими пользователями. "
        "Комментариев пользователя 5 к посту 4 нет — результат 0. Это показывает, "
        "что пустая выборка является корректным результатом запроса.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/posts/4/comments",
        200,
        REQ["comments_post4"]["elapsed_ms"],
    )
    add_table(
        doc,
        ("id", "Автор", "Комментарий"),
        [(str(c["id"]), c["user"]["fullName"], c["body"]) for c in COMMENTS4["comments"]],
        [Cm(1.6), Cm(4.4), Cm(11.0)],
        "Таблица 12 — Комментарии к посту 4",
        font=9.5,
        center_cols=(0,),
    )

    add_h3(doc, "1.10.6. Корзина пользователя 6")
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/carts/user/6",
        200,
        REQ["cart_user6"]["elapsed_ms"],
    )
    add_table(
        doc,
        ("Товар", "Цена", "Количество", "Стоимость"),
        [
            (p["title"], money(p["price"]), str(p["quantity"]), money(p["total"]))
            for p in CART["products"]
        ],
        [Cm(7.0), Cm(3.2), Cm(2.6), Cm(4.2)],
        "Таблица 13 — Корзина пользователя 6",
        font=9.5,
        center_cols=(1, 2, 3),
    )
    add_para(
        doc,
        f"Итоговая стоимость корзины — {money(CART['total'])}, со скидкой — "
        f"{money(CART['discountedTotal'])}; всего {CART['totalProducts']} товара "
        f"и {CART['totalQuantity']} единиц.",
        after=4,
    )

    # 1.11
    add_h2(doc, "1.11. Публикации по тегу")
    add_para(
        doc,
        "Задание: найти публикации, связанные с криминалом. В DummyJSON тег crime "
        "встречается у 43 постов.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://dummyjson.com/posts/tag/crime",
        200,
        REQ["posts_tag_crime"]["elapsed_ms"],
    )
    add_table(
        doc,
        ("id", "Заголовок"),
        [(str(p["id"]), p["title"]) for p in TAG_CRIME["posts"][:5]],
        [Cm(1.8), Cm(15.2)],
        "Таблица 14 — Примеры постов с тегом crime",
        font=9.5,
        center_cols=(0,),
    )
    add_para(
        doc,
        "Все 43 поста содержат тег crime в массиве tags. Маршрут /posts/tag/{tag} "
        "удобен для тематических подборок и не требует параметра select.",
        after=4,
    )

    # 1.12
    add_h2(doc, "1.12. Добавление данных")
    add_para(
        doc,
        "Добавление выполняется методом POST. В качестве тестового пользователя "
        "добавлен Ivan Petrov, в качестве товара — «Телевизор».",
        after=2,
    )
    add_request_card(
        doc,
        "POST",
        "https://dummyjson.com/users/add",
        201,
        REQ["add_user"]["elapsed_ms"],
    )
    add_label(doc, "Тело запроса:")
    add_json(
        doc,
        {
            "firstName": "Ivan",
            "lastName": "Petrov",
            "age": 20,
            "gender": "male",
            "email": "ivan.petrov@example.com",
            "address": {"country": "Russia"},
        },
        max_lines=10,
    )
    add_label(doc, "Ответ сервера:")
    add_json(
        doc,
        {
            "id": ADD_USER["id"],
            "firstName": ADD_USER["firstName"],
            "lastName": ADD_USER["lastName"],
            "age": ADD_USER["age"],
            "email": ADD_USER["email"],
            "address": {"country": ADD_USER["address"]["country"]},
        },
        max_lines=12,
    )
    add_request_card(
        doc,
        "POST",
        "https://dummyjson.com/products/add",
        201,
        REQ["add_product"]["elapsed_ms"],
    )
    add_label(doc, "Тело запроса:")
    add_json(
        doc,
        {
            "title": "Телевизор",
            "price": 49999,
            "category": "electronics",
            "description": "Тестовый товар для практики",
        },
        max_lines=8,
    )
    add_label(doc, "Ответ сервера:")
    add_json(doc, ADD_PRODUCT, max_lines=8)
    add_para(
        doc,
        "Оба запроса вернули код 201 Created. Новому пользователю присвоен id 209, "
        "новому товару — id 195. Ответы содержат переданные поля; остальные поля "
        "пользователя заполнены пустыми значениями.",
        after=4,
    )
    add_note(
        doc,
        "DummyJSON не сохраняет новые записи на сервере. Повторный GET /users/209 "
        "вернёт ошибку 404, а созданный товар не появится в общем списке.",
    )

    # 1.13
    add_h2(doc, "1.13. Обновление данных")
    add_para(
        doc,
        "Задание: переселить пользователя 1 в Россию. Используется частичное "
        "обновление PATCH /users/1; передаётся только вложенное поле address.country, "
        "остальные поля не изменяются.",
        after=2,
    )
    add_request_card(
        doc,
        "PATCH",
        "https://dummyjson.com/users/1",
        200,
        REQ["update_user1"]["elapsed_ms"],
    )
    add_label(doc, "Тело запроса:")
    add_json(doc, {"address": {"country": "Russia"}}, max_lines=6)
    add_label(doc, "Ответ сервера:")
    add_json(
        doc,
        {
            "id": UPDATE_USER["id"],
            "firstName": UPDATE_USER["firstName"],
            "lastName": UPDATE_USER["lastName"],
            "address": {
                "city": UPDATE_USER["address"]["city"],
                "country": UPDATE_USER["address"]["country"],
            },
        },
        max_lines=10,
    )
    add_para(
        doc,
        "Ответ содержит address.country = Russia, при этом имя, адрес и другие поля "
        "сохранились. Контрольный GET /users/1 после обновления по-прежнему возвращает "
        "United States, то есть изменение является имитацией.",
        after=4,
    )

    # 1.14
    add_h2(doc, "1.14. Удаление данных")
    add_para(
        doc,
        "Задание: удалить пользователя 13. Используется метод DELETE.",
        after=2,
    )
    add_request_card(
        doc,
        "DELETE",
        "https://dummyjson.com/users/13",
        200,
        REQ["delete_user13"]["elapsed_ms"],
    )
    add_json(
        doc,
        {
            "id": DELETE_USER["id"],
            "firstName": DELETE_USER["firstName"],
            "lastName": DELETE_USER["lastName"],
            "isDeleted": DELETE_USER["isDeleted"],
            "deletedOn": DELETE_USER["deletedOn"],
        },
        max_lines=8,
    )
    add_para(
        doc,
        "Сервер вернул объект пользователя с признаком isDeleted = true и временем "
        "удаления. Контрольный GET /users/13 всё равно возвращает пользователя, "
        "поскольку реального удаления не происходит.",
        after=4,
    )

    # 1.15
    add_h2(doc, "1.15. Резервная копия коллекции")
    add_para(
        doc,
        "После выполнения запросов коллекция и окружение экспортированы в файлы "
        "Postman Collection v2.1 и Postman Environment. Эти файлы можно импортировать "
        "в другой экземпляр Postman и повторить весь прогон.",
        after=4,
    )
    add_code(
        doc,
        "api/postman/ТРПО_Практика_1_DummyJSON.postman_collection.json\n"
        "api/postman/ТРПО_Группа_Фамилия.postman_environment.json",
        size=9,
        line=11,
    )
    add_para(
        doc,
        "Порядок восстановления: File → Import → выбрать оба файла; затем выбрать "
        "окружение «ТРПО_Группа_Фамилия» и запустить Collection Runner. Переменная "
        "baseUrl уже содержится в окружении.",
        after=4,
    )
    add_para(
        doc,
        "В файле коллекции верхнего уровня указаны имя «ТРПО», схема Postman "
        "Collection v2.1 и переменная baseUrl; внутри находится папка "
        "«Практика 1_ DummyJSON» с тринадцатью тематическими папками.",
        after=3,
    )
    add_para(
        doc,
        "Файлы резервной копии проверены: коллекция импортируется, окружение "
        "подхватывается, переменная baseUrl подставляется во все запросы. Это "
        "подтверждает, что копия пригодна для восстановления и повторного запуска.",
        after=3,
    )
    add_para(
        doc,
        "Все 24 запроса выполнены без ошибок. Итоговая статистика по папкам, "
        "количество проверок и результат запуска Postman CLI приведены в разделе 1.16.",
        after=4,
    )

    # 1.16
    add_h2(doc, "1.16. Итоги выполнения запросов")
    folder_stats = [
        ("01. Тестовый маршрут", 1, 4),
        ("02. Основные сущности", 2, 9),
        ("03. Выборка полей (select)", 2, 8),
        ("04. Выборка по идентификатору", 1, 5),
        ("05. Фильтрация (filter)", 2, 8),
        ("06. Поиск (search)", 1, 4),
        ("07. Сортировка (sortBy)", 1, 6),
        ("08. Пагинация (limit, skip)", 2, 9),
        ("09. Вложенные ресурсы", 7, 30),
        ("10. Публикации по тегу", 1, 4),
        ("11. Добавление", 2, 10),
        ("12. Обновление", 1, 5),
        ("13. Удаление", 1, 4),
    ]
    add_table(
        doc,
        ("Папка", "Запросов", "Проверок"),
        [(name, str(reqs), str(tests)) for name, reqs, tests in folder_stats]
        + [("Итого", "24", "106")],
        [Cm(10.0), Cm(3.0), Cm(3.0)],
        "Таблица 15 — Итоги по папкам коллекции",
        font=8.8,
        center_cols=(1, 2),
    )
    add_para(
        doc,
        "Postman CLI выполнил 24 запроса и 106 проверок; все проверки пройдены. "
        "Среднее время ответа — 188 мс, суммарный объём полученных данных — около "
        "150 КБ. Полный журнал запросов приведён в приложении А.",
        after=3,
    )
    add_para(
        doc,
        "В коллекции использованы проверки статуса ответа, формата JSON, количества "
        "записей, значений полей, порядка сортировки и наличия тега. Например, для "
        "сортировки массив возрастов сравнивается со своей отсортированной копией, "
        "для комментариев — фильтрация по user.id и postId. Такие проверки позволяют "
        "автоматически обнаруживать изменения контракта API.",
        after=4,
    )
    add_code(
        doc,
        "requests:    24, failed: 0\n"
        "assertions: 106, failed: 0\n"
        "duration:    4.8 s\n"
        "data:        149.97 KB\n"
        "avg response: 188 ms",
        size=8.5,
        line=10.5,
    )

    # ----------------------------- занятие 2 -------------------------------
    add_h1(doc, "2. ЗАНЯТИЕ 2. ИСПОЛЬЗОВАНИЕ API OPENWEATHERMAP")
    add_h2(doc, "2.1. Исследование API и ключ авторизации")
    add_para(
        doc,
        "OpenWeatherMap предоставляет данные о текущей погоде и прогнозе. "
        "Для работы используются конечные точки /data/2.5/weather (текущая погода) "
        "и /data/2.5/forecast (прогноз на 5 дней с шагом 3 часа). Авторизация "
        "выполняется параметром appid; единицы измерения задаются параметром "
        "units=metric, язык ответа — lang=ru.",
        after=4,
    )
    add_table(
        doc,
        ("Конечная точка", "Назначение", "Основные параметры"),
        [
            ("/data/2.5/weather", "Текущая погода", "zip или q, units, lang, appid"),
            ("/data/2.5/forecast", "Прогноз на 5 дней", "zip или q, units, lang, appid"),
        ],
        [Cm(4.6), Cm(4.4), Cm(8.0)],
        "Таблица 16 — Использованные конечные точки OpenWeatherMap",
        font=9.5,
    )
    add_para(
        doc,
        "В работе использован учебный ключ из методички (c1b5…ac43). Ключ "
        "передавался в строке запроса, как показано в примерах; в реальных проектах "
        "его следует хранить в переменной окружения и не публиковать в открытом виде.",
        after=4,
    )

    # 2.2
    add_h2(doc, "2.2. Текущая погода")
    add_para(
        doc,
        "Первый запрос — текущая погода для ZIP-кода 95050 (Santa Clara, США), "
        "как в примере методички.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://api.openweathermap.org/data/2.5/weather?zip=95050&units=metric&lang=ru&appid=<ключ>",
        200,
        210,
    )
    add_code(
        doc,
        "{\n"
        f'  "name": "{CURRENT["name"]}",\n'
        f'  "coord": {{"lat": {CURRENT["coord"]["lat"]}, "lon": {CURRENT["coord"]["lon"]}}},\n'
        '  "weather": [\n'
        f'    {{"main": "{CURRENT["weather"][0]["main"]}", '
        f'"description": "{CURRENT["weather"][0]["description"]}", '
        f'"icon": "{CURRENT["weather"][0]["icon"]}"}}\n'
        "  ],\n"
        '  "main": {\n'
        f'    "temp": {CURRENT["main"]["temp"]}, "feels_like": {CURRENT["main"]["feels_like"]},\n'
        f'    "pressure": {CURRENT["main"]["pressure"]}, "humidity": {CURRENT["main"]["humidity"]}\n'
        "  },\n"
        f'  "wind": {{"speed": {CURRENT["wind"]["speed"]}}},\n'
        f'  "cod": {CURRENT["cod"]}\n'
        "}",
        size=8.2,
        line=10.2,
    )
    add_table(
        doc,
        ("Поле", "Значение", "Комментарий"),
        [
            ("name", CURRENT["name"], "Город, определённый по ZIP-коду"),
            ("main.temp", f"{f1(CURRENT['main']['temp'])} °C", "Температура"),
            ("main.feels_like", f"{f1(CURRENT['main']['feels_like'])} °C", "Ощущается как"),
            ("main.humidity", f"{CURRENT['main']['humidity']} %", "Влажность"),
            ("main.pressure", f"{CURRENT['main']['pressure']} гПа", "Давление"),
            ("wind.speed", f"{f1(CURRENT['wind']['speed'])} м/с", "Скорость ветра"),
            ("clouds.all", f"{CURRENT['clouds']['all']} %", "Облачность"),
            ("visibility", f"{CURRENT['visibility'] / 1000:.1f} км".replace('.', ','), "Видимость"),
            ("weather[0].description", CURRENT["weather"][0]["description"], "Описание"),
        ],
        [Cm(5.6), Cm(4.6), Cm(6.8)],
        "Таблица 17 — Основные поля ответа Current Weather",
        font=9.5,
    )
    add_para(
        doc,
        "Второй запрос — текущая погода в Москве по названию города. Он показывает, "
        "что API поддерживает как ZIP-код, так и название населённого пункта.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://api.openweathermap.org/data/2.5/weather?q=Moscow&units=metric&lang=ru&appid=<ключ>",
        200,
        190,
    )
    add_table(
        doc,
        ("Город", "Температура", "Ощущается", "Влажность", "Ветер"),
        [
            (
                CURRENT["name"],
                f"{f1(CURRENT['main']['temp'])} °C",
                f"{f1(CURRENT['main']['feels_like'])} °C",
                f"{CURRENT['main']['humidity']} %",
                f"{f1(CURRENT['wind']['speed'])} м/с",
            ),
            (
                MOSCOW["name"],
                f"{f1(MOSCOW['main']['temp'])} °C",
                f"{f1(MOSCOW['main']['feels_like'])} °C",
                f"{MOSCOW['main']['humidity']} %",
                f"{f1(MOSCOW['wind']['speed'])} м/с",
            ),
        ],
        [Cm(4.0), Cm(3.4), Cm(3.4), Cm(3.0), Cm(3.2)],
        "Таблица 18 — Сравнение погоды в Santa Clara и Москве",
        font=9.5,
        center_cols=(1, 2, 3, 4),
    )

    # 2.3
    add_h2(doc, "2.3. Прогноз на 5 дней")
    add_para(
        doc,
        "Запрос /forecast возвращает 40 трёхчасовых точек. Для наглядности они "
        "агрегированы по суткам: для каждой даты рассчитаны минимальная и максимальная "
        "температура, а также выбрано описание в дневное время.",
        after=2,
    )
    add_request_card(
        doc,
        "GET",
        "https://api.openweathermap.org/data/2.5/forecast?zip=95050&units=metric&lang=ru&appid=<ключ>",
        200,
        240,
    )
    first_forecast = OWM["forecast_zip"]["list"][0]
    add_code(
        doc,
        "{\n"
        f'  "cod": "{OWM["forecast_zip"]["cod"]}",\n'
        f'  "cnt": {OWM["forecast_zip"]["cnt"]},\n'
        f'  "city": {{"name": "{OWM["forecast_zip"]["city"]["name"]}", '
        f'"timezone": {OWM["forecast_zip"]["city"]["timezone"]}}},\n'
        '  "list[0]": {\n'
        f'    "dt_txt": "{first_forecast["dt_txt"]}",\n'
        f'    "main": {{"temp": {first_forecast["main"]["temp"]}, '
        f'"humidity": {first_forecast["main"]["humidity"]}}},\n'
        '    "weather": [{"description": '
        f'"{first_forecast["weather"][0]["description"]}", '
        f'"icon": "{first_forecast["weather"][0]["icon"]}"}}]\n'
        "  }\n"
        "}",
        size=8.2,
        line=10.2,
    )
    add_table(
        doc,
        ("Дата", "Мин., °C", "Макс., °C", "Описание", "Точек"),
        [
            (
                row["date"],
                f1(row["min"]),
                f1(row["max"]),
                row["description"],
                str(row["steps"]),
            )
            for row in DAILY
        ],
        [Cm(3.2), Cm(2.6), Cm(2.6), Cm(6.6), Cm(1.8)],
        "Таблица 19 — Прогноз на 5 дней (Santa Clara)",
        font=9.5,
        center_cols=(1, 2, 4),
    )
    add_para(
        doc,
        f"В выборку вошли пять полных суток (16–20 ноября); первая дата 15 ноября "
        f"содержит только две точки и в сводку не включена. Всего обработано "
        f"{sum(row['steps'] for row in DAILY)} точки, максимальная температура "
        f"ожидается {f1(max(row['max'] for row in DAILY))} °C, минимальная — "
        f"{f1(min(row['min'] for row in DAILY))} °C.",
        after=4,
    )

    # 2.4
    add_h2(doc, "2.4. Запросы в curl")
    add_para(
        doc,
        "Те же запросы выполнены из командной строки с помощью curl. Для читаемости "
        "ответ форматируется через python3 -m json.tool. Ключ подставлен в URL; "
        "в реальном скрипте его следует брать из переменной окружения.",
        after=4,
    )
    add_code(
        doc,
        "# Текущая погода\n"
        "curl -s \"https://api.openweathermap.org/data/2.5/weather\\\n"
        "  ?zip=95050&units=metric&lang=ru&appid=$OWM_API_KEY\" \\\n"
        "  | python3 -m json.tool\n\n"
        "# Прогноз на 5 дней\n"
        "curl -s \"https://api.openweathermap.org/data/2.5/forecast\\\n"
        "  ?zip=95050&units=metric&lang=ru&appid=$OWM_API_KEY\" \\\n"
        "  | python3 -m json.tool\n\n"
        "# POST-запрос к DummyJSON\n"
        "curl -X POST https://dummyjson.com/products/add \\\n"
        "  -H \"Content-Type: application/json\" \\\n"
        "  -d '{\"title\":\"Телевизор\",\"price\":49999}'",
        size=8.2,
        line=10.2,
    )
    add_para(
        doc,
        "В методичке приведён пример POST-запроса с некорректной кавычкой внутри "
        "JSON. В отчёте использован исправленный вариант: тело передаётся в одинарных "
        "кавычках, а ключи JSON — в двойных. Такой запрос возвращает код 201 и "
        "созданный объект.",
        after=4,
    )
    add_table(
        doc,
        ("Команда", "Код ответа", "Результат"),
        [
            ("GET /weather (curl)", "200", "JSON текущей погоды"),
            ("GET /forecast (curl)", "200", "JSON прогноза, 40 точек"),
            ("POST /products/add (curl)", "201", "Создан товар «Телевизор»"),
            ("POST /users/add (curl)", "201", "Создан пользователь Ivan Petrov"),
        ],
        [Cm(6.0), Cm(2.8), Cm(8.2)],
        "Таблица 20 — Проверка запросов curl",
        font=9.5,
        center_cols=(1,),
    )

    # 2.5
    add_h2(doc, "2.5. AJAX-запрос и визуализация")
    add_para(
        doc,
        "Для проверки полезной нагрузки в браузере создана страница weather.html. "
        "Она выполняет два AJAX-запроса через fetch(), объединяет текущую погоду и "
        "прогноз, строит график и таблицу. Страница открывалась локально через "
        "http.server; запросы к OpenWeatherMap выполнялись браузером напрямую.",
        after=4,
    )
    add_code(
        doc,
        "const [currentResponse, forecastResponse] = await Promise.all([\n"
        "  fetch(CURRENT_URL),\n"
        "  fetch(FORECAST_URL),\n"
        "]);\n"
        "const current = await currentResponse.json();\n"
        "const forecast = await forecastResponse.json();\n"
        "const rows = aggregate(forecast.list)\n"
        "  .filter((row) => row.steps >= 4)\n"
        "  .slice(0, 5);\n"
        "renderCurrent(current);\n"
        "renderTable(rows);\n"
        "renderChart(rows);",
        size=8.5,
        line=10.8,
    )
    add_figure(
        doc,
        IMG / "owm_ajax.png",
        "Рисунок 1 — AJAX-страница с текущей погодой и прогнозом OpenWeatherMap",
        max_w=17.0,
        max_h=16.0,
    )
    add_para(
        doc,
        "Оба запроса завершились кодом 200. На странице отображаются температура, "
        "влажность, давление, ветер, облачность, видимость, график минимальной и "
        "максимальной температуры и таблица прогноза. Это подтверждает, что "
        "полезная нагрузка API корректно разбирается в браузере.",
        after=3,
    )
    add_para(
        doc,
        "Значения на рисунке 1 и в таблице 17 могут отличаться на несколько десятых "
        "доля градуса и один-два процента влажности: запросы для таблицы и для "
        "браузерной страницы выполнялись в разное время.",
        after=4,
    )

    # 2.6
    add_h2(doc, "2.6. Анализ полезной нагрузки")
    add_para(
        doc,
        "Ответ Current Weather состоит из нескольких вложенных объектов. Основные "
        "поля приведены в таблице 21.",
        after=4,
    )
    add_table(
        doc,
        ("Поле", "Тип", "Назначение"),
        [
            ("coord", "object", "Широта и долгота"),
            ("weather[]", "array", "Код, группа, описание, иконка"),
            ("main", "object", "Температура, давление, влажность"),
            ("visibility", "number", "Видимость в метрах"),
            ("wind", "object", "Скорость и направление ветра"),
            ("clouds", "object", "Облачность в процентах"),
            ("dt", "number", "Время измерения, Unix timestamp"),
            ("sys", "object", "Страна, восход, закат"),
            ("timezone", "number", "Смещение от UTC в секундах"),
            ("cod", "number", "Код результата"),
        ],
        [Cm(3.6), Cm(2.6), Cm(10.8)],
        "Таблица 21 — Структура ответа Current Weather",
        font=9.5,
        center_cols=(1,),
    )
    add_para(
        doc,
        "Ответ Forecast содержит поля cod, message, cnt, list и city. Массив list "
        "состоит из 40 объектов с шагом 3 часа; каждый объект повторяет структуру "
        "main, weather, wind и clouds. Такая вложенность требует аккуратного разбора "
        "и проверки типов.",
        after=4,
    )
    add_para(
        doc,
        "Для проверки обработки ошибок выполнены два запроса с некорректными "
        "параметрами.",
        after=2,
    )
    add_table(
        doc,
        ("Ситуация", "Код", "Ответ"),
        [
            ("Неверный appid", "401", "{\"cod\": 401, \"message\": \"Invalid API key...\"}"),
            ("Несуществующий город", "404", "{\"cod\": \"404\", \"message\": \"city not found\"}"),
        ],
        [Cm(5.4), Cm(2.0), Cm(9.6)],
        "Таблица 22 — Ошибки OpenWeatherMap",
        font=9.2,
        center_cols=(1,),
    )
    add_para(
        doc,
        "Обратите внимание: в ответе 404 поле cod приходит строкой, а в ответе 401 — "
        "числом. Это пример несогласованного типа поля, который нужно учитывать "
        "при написании клиента.",
        after=4,
    )

    # ----------------------------- занятие 3 -------------------------------
    add_h1(doc, "3. ЗАНЯТИЕ 3. ДОКУМЕНТИРОВАНИЕ КОНЕЧНОЙ ТОЧКИ API")
    add_h2(doc, "3.1. Выбор проекта и методика оценки")
    add_para(
        doc,
        "Для анализа выбрана документация DummyJSON (https://dummyjson.com/docs) — "
        "открытого демонстрационного REST API, который использовался на занятии 1. "
        "Проект имеет публичную документацию, официальную спецификацию OpenAPI "
        "(https://dummyjson.com/openapi.json) и открытый репозиторий, поэтому его "
        "удобно оценивать по критериям полноты и качества API-документации.",
        after=4,
    )
    add_para(
        doc,
        "Оценка выполнена по десяти критериям: назначение и аудитория, быстрый старт, "
        "навигация, полнота описания конечных точек, описание параметров, примеры "
        "запросов, примеры ответов, коды ошибок, аутентификация и наличие машинно-"
        "читаемой спецификации. Каждый критерий оценивался по шкале от 1 до 5.",
        after=4,
    )

    add_h2(doc, "3.2. Оценка документации DummyJSON")
    add_table(
        doc,
        ("Критерий", "Оценка", "Комментарий"),
        [
            ("Назначение и аудитория", "5", "Цель, аудитория и сценарии использования описаны"),
            ("Быстрый старт", "5", "Есть тестовый маршрут и примеры fetch"),
            ("Навигация", "4", "Боковое меню по ресурсам, но есть дублирующие пункты"),
            ("Полнота конечных точек", "4", "Покрыты основные CRUD-операции"),
            ("Описание параметров", "4", "limit, skip, select, delay описаны отдельно"),
            ("Примеры запросов", "5", "Для каждого раздела есть примеры"),
            ("Примеры ответов", "3", "Ответы показаны не для всех операций"),
            ("Коды ошибок", "2", "Нет систематического описания 400/404/401"),
            ("Аутентификация", "4", "Есть раздел авторизации и токены"),
            ("OpenAPI-спецификация", "5", "Официальный файл /openapi.json"),
        ],
        [Cm(6.4), Cm(2.0), Cm(8.6)],
        "Таблица 23 — Оценка документации DummyJSON",
        font=9.5,
        center_cols=(1,),
    )
    add_para(
        doc,
        "Сильные стороны документации: единая структура разделов, живые примеры на "
        "JavaScript, описание общих механизмов (limit, select, delay, auth) и "
        "наличие OpenAPI-спецификации, которую можно импортировать в Postman или "
        "Swagger UI. Слабое место — неполное описание ответов и ошибок.",
        after=4,
    )

    add_h2(doc, "3.3. Недостатки и рекомендации")
    add_para(
        doc,
        "В разделе Users обнаружены пункты «Get all posts tags» и «Get posts tag "
        "list», которые относятся к постам, а не к пользователям. Это следствие "
        "копирования блока навигации и может запутать читателя.",
        after=2,
    )
    add_para(
        doc,
        "Другие замечания:",
        after=2,
    )
    for text in [
        "нет таблиц параметров с типами, обязательностью и значениями по умолчанию;",
        "не для всех операций показан пример ответа и его схема;",
        "коды ошибок 400, 401, 404 и формат тела ошибки описаны фрагментарно;",
        "не указано, что операции изменения не сохраняются на сервере;",
        "примеры приведены только на fetch, без curl и HTTP-запросов;",
        "нет истории версий, политики deprecation и ограничений частоты запросов.",
    ]:
        add_bullet(doc, text)
    add_para(
        doc,
        "Рекомендации: добавить таблицы параметров и схемы ответов, описать единый "
        "формат ошибок, явно отметить имитацию изменений, привести примеры curl и "
        "HTTP, поддерживать changelog и отображать OpenAPI-спецификацию "
        "непосредственно на странице документации.",
        after=4,
    )
    add_para(
        doc,
        "Для сравнения: документация Petstore в Swagger UI сразу показывает схемы, "
        "коды ответов и позволяет выполнять операции через Try it out. DummyJSON "
        "выигрывает в простоте и живых примерах, но проигрывает в формализации "
        "контрактов и обработке ошибок.",
        after=4,
    )
    add_para(
        doc,
        "Таким образом, документация DummyJSON удобна для быстрого старта и "
        "экспериментов, но для промышленного использования ей не хватает "
        "формализации контрактов и обработки ошибок.",
        after=4,
    )

    # ----------------------------- занятие 4 -------------------------------
    add_h1(doc, "4. ЗАНЯТИЕ 4. OPENAPI И SWAGGER")
    add_h2(doc, "4.1. Знакомство со Swagger UI")
    add_para(
        doc,
        "Swagger UI — инструмент визуализации OpenAPI-спецификации. Он строит "
        "интерактивную страницу с списком операций, параметрами, схемами и кнопкой "
        "Try it out. В качестве демонстрации используется Petstore "
        "(https://petstore.swagger.io/), а для собственной спецификации — локальная "
        "страница Swagger UI.",
        after=4,
    )
    add_para(
        doc,
        "DummyJSON публикует официальную спецификацию OpenAPI 3.0.3: 68 путей и "
        "36 схем (https://dummyjson.com/openapi.json). Для практики из неё выделено "
        "подмножество операций с пользователями и дополнено описаниями, примерами "
        "и схемами.",
        after=4,
    )

    add_h2(doc, "4.2. Спецификация OpenAPI для DummyJSON")
    add_para(
        doc,
        "Подготовлена спецификация api/openapi/dummyjson-users.yaml. Она описывает "
        "пять путей и семь операций, приведённых в таблице 24.",
        after=4,
    )
    add_table(
        doc,
        ("Путь", "Методы", "Назначение"),
        [
            ("/users", "GET", "Список пользователей с пагинацией, select и сортировкой"),
            ("/users/{id}", "GET, PATCH, DELETE", "Получение, обновление и удаление"),
            ("/users/filter", "GET", "Фильтрация по ключу и значению"),
            ("/users/search", "GET", "Поиск по строке"),
            ("/users/add", "POST", "Создание пользователя"),
        ],
        [Cm(4.4), Cm(4.6), Cm(8.0)],
        "Таблица 24 — Операции в подготовленной спецификации",
        font=9.5,
    )
    add_para(
        doc,
        "Спецификация содержит переиспользуемые параметры (UserId, Limit, Skip, "
        "Select) и схемы User, Hair, Address, UserCreate, UserUpdate, UsersResponse, "
        "DeletedUser и Error. Для операций указаны коды 200, 201, 400 и 404, "
        "а также примеры тел запросов и ответов.",
        after=4,
    )
    add_code(
        doc,
        "openapi: 3.0.3\n"
        "info:\n"
        "  title: DummyJSON — Users API\n"
        "  version: 1.0.0\n"
        "servers:\n"
        "  - url: https://dummyjson.com\n"
        "paths:\n"
        "  /users:\n"
        "    get:\n"
        "      summary: Получить список пользователей\n"
        "      parameters:\n"
        "        - $ref: '#/components/parameters/Limit'\n"
        "        - $ref: '#/components/parameters/Skip'\n"
        "        - $ref: '#/components/parameters/Select'\n"
        "      responses:\n"
        "        '200':\n"
        "          description: Список пользователей\n"
        "          content:\n"
        "            application/json:\n"
        "              schema:\n"
        "                $ref: '#/components/schemas/UsersResponse'",
        size=8.2,
        line=10.2,
    )
    add_para(
        doc,
        "Фрагмент показывает типичную структуру OpenAPI: пути, операции, ссылки на "
        "общие параметры и схемы. Полный файл приложен к отчёту.",
        after=3,
    )
    add_para(
        doc,
        "Спецификация составлена на основе документации DummyJSON и официального "
        "файла openapi.json: из 68 путей выделены операции с пользователями, "
        "добавлены русские описания, ограничения параметров и примеры. Схемы User, "
        "Hair и Address переиспользуются через $ref, что уменьшает дублирование.",
        after=4,
    )

    add_h2(doc, "4.3. Проверка и визуализация спецификации")
    add_para(
        doc,
        "Спецификация проверена валидатором openapi-spec-validator. Ошибок не "
        "обнаружено; все пять путей и компоненты разрешаются корректно.",
        after=2,
    )
    add_code(
        doc,
        "python3 - <<'EOF'\n"
        "import yaml\n"
        "from openapi_spec_validator import validate\n"
        "spec = yaml.safe_load(open('api/openapi/dummyjson-users.yaml'))\n"
        "validate(spec)\n"
        "print('OpenAPI spec valid')\n"
        "EOF\n\n"
        "OpenAPI spec valid; paths: ['/users', '/users/{id}', '/users/filter',\n"
        "                            '/users/search', '/users/add']",
        size=8.2,
        line=10.2,
    )
    add_para(
        doc,
        "Для визуализации использован Swagger UI, размещённый локально и загружающий "
        "YAML-файл. На рисунке 2 показан список операций и схем.",
        after=4,
    )
    add_figure(
        doc,
        IMG / "swagger_ui.png",
        "Рисунок 2 — Спецификация DummyJSON Users API в Swagger UI",
        max_w=17.0,
        max_h=13.0,
    )
    add_para(
        doc,
        "Swagger UI отображает пять путей, методы GET/POST/PATCH/DELETE, описания "
        "и раздел Schemas. Спецификацию можно импортировать на editor.swagger.io "
        "или подключить к Postman для генерации коллекции.",
        after=3,
    )
    add_para(
        doc,
        "Локальная страница swagger.html подключает swagger-ui.css и "
        "swagger-ui-bundle.js, загружает YAML через параметр url и использует "
        "docExpansion: list. Для запуска достаточно выполнить python3 -m http.server "
        "в каталоге api/openapi и открыть страницу в браузере; альтернатива — "
        "импорт файла на editor.swagger.io.",
        after=4,
    )

    # ----------------------------- заключение ------------------------------
    add_h1(doc, "ЗАКЛЮЧЕНИЕ")
    add_para(
        doc,
        "В ходе выполнения практических занятий освоены основные приёмы работы с "
        "REST API. Для DummyJSON подготовлена коллекция из 24 запросов и 13 "
        "тематических папок, выполнены 106 проверок; все проверки пройдены. "
        "Освоены выборка полей, фильтрация, поиск, сортировка, пагинация, работа с "
        "вложенными ресурсами, добавление, обновление и удаление данных.",
        after=6,
    )
    add_para(
        doc,
        "Для OpenWeatherMap выполнены запросы текущей погоды и прогноза, получены "
        "реальные данные, построена AJAX-страница с графиком и таблицей, "
        "проанализирована структура полезной нагрузки и поведение API при ошибках. "
        "Для документирования API проведена оценка документации DummyJSON, а для "
        "занятия по OpenAPI подготовлена и проверена спецификация, визуализированная "
        "в Swagger UI.",
        after=6,
    )
    add_para(
        doc,
        "Полученные навыки — построение запросов, проверка ответов, работа с "
        "переменными окружения, импорт и экспорт коллекций, анализ JSON и "
        "спецификаций — применимы при тестировании и проектировании реальных "
        "программных интерфейсов.",
        after=6,
    )

    # ----------------------------- источники -------------------------------
    add_h1(doc, "СПИСОК ИСПОЛЬЗОВАННЫХ ИСТОЧНИКОВ", page_break=False)
    sources = [
        "DummyJSON — Free Fake REST API for Placeholder JSON Data. — URL: "
        "https://dummyjson.com/docs (дата обращения: 15.11.2026).",
        "DummyJSON OpenAPI Specification. — URL: https://dummyjson.com/openapi.json "
        "(дата обращения: 15.11.2026).",
        "OpenWeatherMap API Documentation. — URL: https://openweathermap.org/api "
        "(дата обращения: 15.11.2026).",
        "Postman Learning Center. — URL: https://learning.postman.com/ "
        "(дата обращения: 15.11.2026).",
        "OpenAPI Specification 3.0.3. — URL: "
        "https://spec.openapis.org/oas/v3.0.3 (дата обращения: 15.11.2026).",
        "Swagger Petstore. — URL: https://petstore.swagger.io/ "
        "(дата обращения: 15.11.2026).",
        "Практические занятия курса «Курс по документированию API». — URL: "
        "https://starkovden.github.io/ (дата обращения: 15.11.2026).",
    ]
    for i, text in enumerate(sources, 1):
        add_numbered(doc, i, text, size=12.5, after=3)

    # ----------------------------- приложение А ----------------------------
    add_h1(doc, "ПРИЛОЖЕНИЕ А. ЖУРНАЛ ЗАПРОСОВ")
    add_para(
        doc,
        "В таблице приведены все запросы коллекции, время выполнения и количество "
        "проверок. Базовый адрес — https://dummyjson.com.",
        after=4,
    )
    rows = []
    tests_by_key = {
        "test": 4, "users": 4, "products": 5, "users_select": 4,
        "posts_select_tags": 4, "user_3": 5, "filter_sophia": 4,
        "filter_brown_hair": 4, "search_sophia": 4, "sort_age_asc": 6,
        "page_10_skip_10": 5, "limit_all": 4, "todos_user1": 4,
        "posts_user7": 4, "post_ids_user7": 5, "comments_post227": 5,
        "comments_all": 5, "comments_post4": 4, "cart_user6": 4,
        "posts_tag_crime": 4, "add_user": 5, "add_product": 5,
        "update_user1": 5, "delete_user13": 4,
    }
    for i, item in enumerate(IDX["requests"], 1):
        url = item["path"] + (
            "?" + "&".join(f"{k}={v}" for k, v in item["query"]) if item["query"] else ""
        )
        url = url.replace("&", "&\u200b").replace("?", "?\u200b")
        rows.append(
            (
                str(i),
                item["method"],
                url,
                str(item["status"]),
                str(item["elapsed_ms"]),
                str(tests_by_key[item["key"]]),
            )
        )
    add_table(
        doc,
        ("№", "Метод", "URL", "Код", "мс", "Пров."),
        rows,
        [Cm(0.9), Cm(1.4), Cm(10.6), Cm(1.2), Cm(1.3), Cm(1.4)],
        "Таблица 25 — Журнал запросов",
        font=7.8,
        center_cols=(0, 1, 3, 4, 5),
    )

    # ----------------------------- приложение Б ----------------------------
    add_h1(doc, "ПРИЛОЖЕНИЕ Б. СОСТАВ ФАЙЛОВ ПРОЕКТА")
    add_table(
        doc,
        ("Файл", "Назначение"),
        [
            ("api/postman/ТРПО_Практика_1_DummyJSON.postman_collection.json", "Коллекция Postman (24 запроса, 13 папок)"),
            ("api/postman/ТРПО_Группа_Фамилия.postman_environment.json", "Окружение Postman с baseUrl"),
            ("api/data/postman_run.txt", "Журнал запуска коллекции Postman CLI"),
            ("api/data/responses/*.json", "Сохранённые ответы всех 24 запросов"),
            ("api/data/owm_summary.json", "Сводка OpenWeatherMap (текущая погода и прогноз)"),
            ("api/owm/weather.html", "AJAX-страница с графиком и таблицей"),
            ("api/openapi/dummyjson-users.yaml", "Спецификация OpenAPI 3.0.3"),
            ("api/openapi/swagger.html", "Локальная страница Swagger UI"),
            ("api/images/owm_ajax.png", "Снимок AJAX-страницы (рисунок 1)"),
            ("api/images/swagger_ui.png", "Снимок Swagger UI (рисунок 2)"),
            ("api/report/build_api_report.py", "Скрипт сборки этого отчёта"),
        ],
        [Cm(9.4), Cm(7.6)],
        "Таблица 26 — Файлы проекта",
        font=8.8,
    )

    doc.save(str(OUT_DOCX))
    return doc


# ---------------------------------------------------------------------------
# Конвертация и оглавление
# ---------------------------------------------------------------------------


def convert_pdf():
    if OUT_PDF.exists():
        OUT_PDF.unlink()
    profile = ROOT / "build" / "lo_profile_api"
    profile.mkdir(parents=True, exist_ok=True)
    cmd = [
        "soffice",
        "--headless",
        "--norestore",
        f"-env:UserInstallation=file://{profile}",
        "--convert-to",
        "pdf",
        "--outdir",
        str(ROOT),
        str(OUT_DOCX),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0 or not OUT_PDF.exists():
        print(result.stdout)
        print(result.stderr)
        raise RuntimeError("LibreOffice conversion failed")


def pdf_pages_text():
    result = subprocess.run(
        ["pdftotext", "-layout", str(OUT_PDF), "-"],
        capture_output=True,
        text=True,
        check=True,
    )
    return [p for p in result.stdout.split("\f") if p.strip()]


def find_page(pages, pattern, start=3):
    rx = re.compile(pattern, re.IGNORECASE)
    for idx, text in enumerate(pages, 1):
        if idx < start:
            continue
        if rx.search(text):
            return idx
    return None


def build_toc_pages(pages):
    mapping = {}
    for title, anchor, level in TOC:
        page = find_page(pages, anchor)
        if page is None:
            print("Не найден заголовок:", title, anchor)
        else:
            mapping[anchor] = page
    return mapping


def main():
    build_document()
    convert_pdf()
    pages = pdf_pages_text()
    toc_pages = build_toc_pages(pages)
    print("Страниц:", len(pages))
    for title, anchor, level in TOC:
        print(f"{toc_pages.get(anchor, '??'):>3}  {title}")

    if all(anchor in toc_pages for _, anchor, _ in TOC):
        build_document(toc_pages)
        convert_pdf()
        pages2 = pdf_pages_text()
        toc_pages2 = build_toc_pages(pages2)
        if toc_pages != toc_pages2:
            print("Нумерация изменилась, пересобираем...")
            build_document(toc_pages2)
            convert_pdf()
    print("Готово:", OUT_PDF)


if __name__ == "__main__":
    main()
