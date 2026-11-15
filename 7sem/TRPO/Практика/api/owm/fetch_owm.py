#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Загрузка данных OpenWeatherMap и подготовка сводки для отчёта."""

import json
from collections import OrderedDict
from datetime import datetime, timezone
from pathlib import Path

import requests

ROOT = Path(__file__).resolve().parent
DATA = ROOT.parent / "data"
BASE = "https://api.openweathermap.org/data/2.5"
API_KEY = "c1b540f259064d8e68df0a0dd134ac43"
CITY_ZIP = "95050"
CITY_NAME = "Moscow"


def fetch(endpoint, params):
    url = f"{BASE}/{endpoint}"
    response = requests.get(url, params=params, timeout=30)
    response.raise_for_status()
    return response.json(), response.url


def daily_summary(forecast):
    days = OrderedDict()
    for item in forecast["list"]:
        day = item["dt_txt"][:10]
        bucket = days.setdefault(day, {"min": 999.0, "max": -999.0, "items": []})
        temp = item["main"]["temp"]
        bucket["min"] = min(bucket["min"], temp)
        bucket["max"] = max(bucket["max"], temp)
        bucket["items"].append(item)
    result = []
    for day, bucket in days.items():
        midday = min(
            bucket["items"],
            key=lambda x: abs(int(x["dt_txt"][11:13]) - 15),
        )
        result.append(
            {
                "date": day,
                "min": round(bucket["min"], 1),
                "max": round(bucket["max"], 1),
                "steps": len(bucket["items"]),
                "description": midday["weather"][0]["description"],
                "icon": midday["weather"][0]["icon"],
            }
        )
    return result


def main():
    DATA.mkdir(parents=True, exist_ok=True)
    queries = {
        "current_zip": ("weather", {"zip": CITY_ZIP, "units": "metric", "lang": "ru"}),
        "current_moscow": ("weather", {"q": CITY_NAME, "units": "metric", "lang": "ru"}),
        "forecast_zip": ("forecast", {"zip": CITY_ZIP, "units": "metric", "lang": "ru"}),
        "forecast_moscow": ("forecast", {"q": CITY_NAME, "units": "metric", "lang": "ru"}),
    }
    payloads = {}
    urls = {}
    for name, (endpoint, params) in queries.items():
        payload, url = fetch(endpoint, {**params, "appid": API_KEY})
        payloads[name] = payload
        urls[name] = url
        (DATA / f"owm_{name}.json").write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
        )
        print(name, "OK", url)

    summary = {
        "fetched_at": datetime.now(timezone.utc).isoformat(),
        "api_key": API_KEY,
        "zip": CITY_ZIP,
        "city": payloads["current_zip"].get("name"),
        "urls": urls,
        "current_zip": payloads["current_zip"],
        "current_moscow": payloads["current_moscow"],
        "forecast_zip": payloads["forecast_zip"],
        "forecast_moscow": payloads["forecast_moscow"],
        "daily_zip": daily_summary(payloads["forecast_zip"]),
        "daily_moscow": daily_summary(payloads["forecast_moscow"]),
    }
    (DATA / "owm_summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print("Сводка:", DATA / "owm_summary.json")
    for row in summary["daily_zip"]:
        print(row["date"], row["min"], row["max"], row["description"])


if __name__ == "__main__":
    main()
