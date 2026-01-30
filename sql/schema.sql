-- База данных химической лаборатории
BEGIN TRANSACTION;

-- Химикаты
CREATE TABLE IF NOT EXISTS chemicals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    formula TEXT,
    type TEXT NOT NULL,
    danger_class INTEGER NOT NULL CHECK(danger_class >= 0 AND danger_class <= 5),
    storage_temp REAL NOT NULL,
    shelf_life_months INTEGER NOT NULL,
    quantity REAL DEFAULT 0,
    unit TEXT DEFAULT 'г',
    container TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Зоны хранения
CREATE TABLE IF NOT EXISTS storage_zones (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    temperature REAL NOT NULL,
    max_capacity REAL NOT NULL,
    current_load REAL DEFAULT 0,
    description TEXT
);

-- Партии химикатов
CREATE TABLE IF NOT EXISTS batches (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    chemical_id INTEGER NOT NULL,
    zone_id INTEGER,
    quantity REAL NOT NULL CHECK(quantity > 0),
    production_date TEXT,
    expiration_date TEXT,
    supplier TEXT,
    FOREIGN KEY (chemical_id) REFERENCES chemicals(id) ON DELETE CASCADE,
    FOREIGN KEY (zone_id) REFERENCES storage_zones(id)
);

COMMIT;
