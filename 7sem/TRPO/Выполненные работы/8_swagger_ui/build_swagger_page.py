# -*- coding: utf-8 -*-
"""Генерация self-contained страницы Swagger UI с встроенной спецификацией."""
import json
import yaml

BASE = '/home/kukirmash/labs/7sem/TRPO/Выполненные работы/8_swagger_ui'
with open(f'{BASE}/openapi.yaml', encoding='utf-8') as f:
    spec = yaml.safe_load(f)

spec_json = json.dumps(spec, ensure_ascii=False, indent=2)

html = f'''<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>API «Экскурсионное бюро» — Swagger UI</title>
  <link rel="stylesheet" href="./swagger-ui.css">
  <style>
    html {{ box-sizing: border-box; overflow-y: scroll; }}
    *, *:before, *:after {{ box-sizing: inherit; }}
    body {{ margin: 0; background: #fafafa; }}
    .app-header {{
      background: #1b3a5c;
      color: #fff;
      padding: 18px 32px 16px;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
    }}
    .app-header h1 {{ margin: 0 0 4px; font-size: 24px; font-weight: 600; }}
    .app-header p {{ margin: 0; opacity: .85; font-size: 14px; }}
    .app-header code {{ background: rgba(255,255,255,.15); padding: 1px 5px; border-radius: 3px; }}
    .swagger-ui .topbar {{ display: none; }}
  </style>
</head>
<body>
  <header class="app-header">
    <h1>ИС «Экскурсионное бюро»</h1>
    <p>Интерактивная документация API · конечная точка <code>POST /contracts</code> · OpenAPI 3.0.3</p>
  </header>
  <div id="swagger-ui"></div>

  <script src="./swagger-ui-bundle.js"></script>
  <script src="./swagger-ui-standalone-preset.js"></script>
  <script>
    const spec = {spec_json};

    window.ui = SwaggerUIBundle({{
      spec: spec,
      dom_id: '#swagger-ui',
      deepLinking: true,
      docExpansion: 'full',
      defaultModelsExpandDepth: 1,
      defaultModelExpandDepth: 1,
      displayRequestDuration: true,
      filter: true,
      persistAuthorization: true,
      tryItOutEnabled: true,
      presets: [
        SwaggerUIBundle.presets.apis,
        SwaggerUIStandalonePreset
      ],
      plugins: [
        SwaggerUIBundle.plugins.DownloadUrl
      ],
      layout: 'StandaloneLayout'
    }});
  </script>
</body>
</html>
'''

with open(f'{BASE}/index.html', 'w', encoding='utf-8') as f:
    f.write(html)
print('index.html written, spec size:', len(spec_json))
