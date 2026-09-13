// Переключаемся на базу lab_db
db = db.getSiblingDB('lab_db');

// Создаем коллекцию
db.createCollection('products_info');

// Заполняем сложной структурой данных
db.products_info.insertMany([
    {
        product_id: 101, // Связь с PostgreSQL
        brand: "Apple",
        category: "Smartphones",
        color: "Titanium",
        dimensions: { width: 70.6, height: 146.6, depth: 8.25 },
        documents: ["manual.pdf", "warranty.pdf"]
    },
    {
        product_id: 102, // Связь с PostgreSQL
        brand: "AudioPro",
        category: "Headphones",
        color: "White",
        type: "TWS",
        documents: ["quick_start.pdf"]
    }
]);