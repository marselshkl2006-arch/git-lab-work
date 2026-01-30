-- ПОЛНАЯ СХЕМА БАЗЫ ДАННЫХ
BEGIN TRANSACTION;

-- Химикаты
CREATE TABLE chemicals (
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
CREATE TABLE storage_zones (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    temperature REAL NOT NULL,
    max_capacity REAL NOT NULL,
    current_load REAL DEFAULT 0,
    description TEXT
);

-- Партии химикатов (СВЯЗЬ химикатов и зон)
CREATE TABLE batches (
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

-- ВСТАВКА ДАННЫХ
INSERT INTO chemicals (name, formula, type, danger_class, storage_temp, shelf_life_months, quantity, unit) VALUES
('Серная кислота', 'H2SO4', 'Кислота', 1, 20.0, 36, 5000, 'мл'),
('Соляная кислота', 'HCl', 'Кислота', 1, 20.0, 24, 3000, 'мл'),
('Гидроксид натрия', 'NaOH', 'Основание', 2, 25.0, 24, 2000, 'г'),
('Этанол', 'C2H5OH', 'Органическое', 3, 15.0, 12, 10000, 'мл'),
('Ацетон', 'C3H6O', 'Органическое', 3, 20.0, 24, 5000, 'мл'),
('Перекись водорода', 'H2O2', 'Окислитель', 1, 4.0, 6, 2000, 'мл'),
('Натрий хлорид', 'NaCl', 'Соль', 0, 25.0, 999, 10000, 'г');

INSERT INTO storage_zones (name, temperature, max_capacity, current_load, description) VALUES
('Холодильник кислот', 4.0, 100.0, 25.0, 'Для кислот и термочувствительных веществ'),
('Основное хранилище', 20.0, 500.0, 350.0, 'Основной склад'),
('Пожароопасная зона', 15.0, 80.0, 45.0, 'Для легковоспламеняющихся веществ'),
('Морозильная камера', -18.0, 50.0, 10.0, 'Для длительного хранения');

INSERT INTO batches (chemical_id, zone_id, quantity, production_date, expiration_date, supplier) VALUES
(1, 1, 1000, '2024-01-15', '2025-01-15', 'ХимРеактивСнаб'),
(1, 2, 2000, '2024-02-01', '2025-02-01', 'Лабораторные системы'),
(2, 1, 500, '2024-02-01', '2025-02-01', 'Лабораторные системы'),
(3, 2, 1000, '2024-01-01', '2026-01-01', 'Научные материалы'),
(4, 3, 5000, '2024-01-01', '2024-07-01', 'Научные материалы'),
(5, 3, 3000, '2024-03-01', '2025-03-01', 'ХимРеактивСнаб'),
(6, 1, 500, '2024-01-01', '2024-04-01', 'Лабораторные системы'),
(4, 2, 5000, '2024-02-01', '2024-08-01', 'ХимРеактивСнаб');

-- ОБНОВЛЯЕМ загрузку зон на основе партий
UPDATE storage_zones SET current_load = (
    SELECT COALESCE(SUM(b.quantity), 0) 
    FROM batches b 
    WHERE b.zone_id = storage_zones.id
);

COMMIT;
