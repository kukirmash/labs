Лабораторная работа 8 — «Документирование веб-API»
====================================================

Интерактивная документация Swagger UI для конечной точки POST /contracts
ИС «Экскурсионное бюро».

Состав папки:
  index.html          — страница Swagger UI со встроенной спецификацией
  openapi.yaml        — спецификация OpenAPI 3.0.3 (одна конечная точка)
  swagger-ui.css      — стили Swagger UI (swagger-ui-dist 5)
  swagger-ui-bundle.js
  swagger-ui-standalone-preset.js
  mock_server.py      — локальный mock-сервер для Try it out

Как открыть:
  1. Откройте index.html в браузере (двойной щелчок или file://).
     Спецификация встроена в страницу, интернет не требуется.
  2. Для тестирования через Try it out:
       python3 mock_server.py
     В Swagger UI выберите сервер
     «Локальный mock-сервер для тестирования»
     и нажмите Execute.
     Mock-сервер отвечает 201 и возвращает договор с билетами.

Примечание: swagger-ui-* — официальные файлы дистрибутива Swagger UI,
загруженные с unpkg.com (swagger-ui-dist@5) для автономной работы страницы.

Обновление страницы:
  Если вы изменили openapi.yaml, пересоберите index.html:
      python3 build_swagger_page.py
  Скрипт встроит актуальную спецификацию в страницу.

Проверка спецификации:
      python3 -m openapi_spec_validator openapi.yaml

Генерация клиента (пример):
      npx @openapitools/openapi-generator-cli generate \
        -i openapi.yaml -g python -o generated/python-client \
        --additional-properties=packageName=excursion_api
