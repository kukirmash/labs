#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Минимальный mock-сервер для демонстрации Try it out в Swagger UI.

Запуск:
    python3 mock_server.py

Сервер слушает http://localhost:8000/api/v1/contracts и возвращает
пример успешного ответа 201. В Swagger UI выберите сервер
«Локальный mock-сервер для тестирования» и нажмите Execute.
"""
import json
from datetime import datetime, timedelta, timezone
from http.server import BaseHTTPRequestHandler, HTTPServer
from uuid import uuid4

HOST, PORT = '127.0.0.1', 8000


def iso_now() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat(timespec='seconds')


class Handler(BaseHTTPRequestHandler):
    def _cors(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type, Idempotency-Key')
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS')

    def _json(self, code: int, payload: dict):
        body = json.dumps(payload, ensure_ascii=False, indent=2).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(body)))
        self._cors()
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(204)
        self._cors()
        self.end_headers()

    def do_POST(self):
        if self.path.rstrip('/') != '/api/v1/contracts':
            self._json(404, {
                'type': 'https://api.excursion-bureau.ru/problems/not-found',
                'title': 'Ресурс не найден', 'status': 404,
            })
            return

        length = int(self.headers.get('Content-Length', 0))
        try:
            data = json.loads(self.rfile.read(length) or b'{}')
        except json.JSONDecodeError:
            self._json(400, {
                'type': 'https://api.excursion-bureau.ru/problems/bad-request',
                'title': 'Некорректный JSON', 'status': 400,
            })
            return

        participants = data.get('participants') or []
        if not data.get('excursionId') or not participants:
            self._json(422, {
                'type': 'https://api.excursion-bureau.ru/problems/validation-error',
                'title': 'Ошибка валидации', 'status': 422,
                'detail': 'Поля excursionId и participants обязательны',
                'instance': '/api/v1/contracts',
            })
            return

        contract_id = str(uuid4())
        number = 'Д-2026-' + contract_id[:6].upper()
        price = 1500.00 * len(participants)
        tickets = []
        for p in participants:
            name = ' '.join(filter(None, [p.get('lastName'), p.get('firstName'), p.get('middleName')]))
            tickets.append({
                'id': str(uuid4()),
                'qrCode': 'QR-' + uuid4().hex[:8].upper(),
                'participantName': name,
                'attended': False,
                'status': 'active',
            })
        contract = {
            'id': contract_id,
            'number': number,
            'excursionId': data['excursionId'],
            'totalPrice': price,
            'status': 'pending_payment',
            'createdAt': iso_now(),
            'paymentDeadline': (datetime.now().astimezone() + timedelta(minutes=10)).isoformat(timespec='seconds'),
            'participants': participants,
            'tickets': tickets,
        }
        self.send_response(201)
        payload = json.dumps({
            'contract': contract,
            'clientToken': 'ct_demo_' + contract_id.replace('-', '')[:16],
        }, ensure_ascii=False, indent=2).encode('utf-8')
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(payload)))
        self.send_header('Location', f'/api/v1/contracts/{contract_id}')
        self._cors()
        self.end_headers()
        self.wfile.write(payload)

    def log_message(self, fmt, *args):
        print('[mock]', fmt % args)


if __name__ == '__main__':
    print(f'Mock API: http://{HOST}:{PORT}/api/v1/contracts')
    HTTPServer((HOST, PORT), Handler).serve_forever()
