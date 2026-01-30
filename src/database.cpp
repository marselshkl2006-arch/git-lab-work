#include "database.h"
#include <iostream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>
#include <QTextStream>

// ============ ОСНОВНЫЕ МЕТОДЫ ============

Database::Database(QObject *parent) : QObject(parent) {
    std::cout << "Database constructor" << std::endl;
    m_settings = new QSettings("ChemicalLab", "ChemicalLabQt");
}

Database::~Database() {
    std::cout << "Database destroyed" << std::endl;
    if (m_db.isOpen()) {
        m_db.close();
    }
    if (m_settings) {
        delete m_settings;
    }
}

Database& Database::instance() {
    static Database instance;
    return instance;
}

bool Database::initializeDatabase() {
    std::cout << "Database::initializeDatabase()" << std::endl;
    
    // Определяем путь к базе данных
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }
    
    m_databasePath = dataDir + QDir::separator() + "chemicallab.db";
    std::cout << "Database path: " << m_databasePath.toStdString() << std::endl;
    
    // Открываем базу данных
    m_db = QSqlDatabase::addDatabase("QSQLITE", "chemical_lab_connection");
    m_db.setDatabaseName(m_databasePath);
    
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        std::cerr << "Failed to open database: " << m_lastError.toStdString() << std::endl;
        return false;
    }
    
    // Создаем таблицы
    if (!createTables()) {
        std::cerr << "Failed to create tables" << std::endl;
        return false;
    }
    
    // Вставляем тестовые данные если таблицы пустые
    if (getChemicalCount() == 0 && getZoneCount() == 0) {
        insertTestData();
    }
    
    std::cout << "Database initialized successfully" << std::endl;
    return true;
}

bool Database::createTables() {
    QSqlQuery query(m_db);
    
    // Таблица химикатов
    if (!query.exec("CREATE TABLE IF NOT EXISTS chemicals ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL,"
               "formula TEXT,"
               "cas_number TEXT,"
               "manufacturer TEXT,"
               "supplier TEXT,"
               "purity REAL DEFAULT 100.0,"
               "quantity REAL NOT NULL DEFAULT 0.0,"
               "unit TEXT DEFAULT 'г',"
               "hazard_class INTEGER DEFAULT 3,"
               "storage_conditions TEXT,"
               "expiration_date DATE,"
               "arrival_date DATE DEFAULT CURRENT_DATE,"
               "notes TEXT,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)")) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Таблица зон хранения
    if (!query.exec("CREATE TABLE IF NOT EXISTS storage_zones ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL,"
               "description TEXT,"
               "temperature_min REAL,"
               "temperature_max REAL,"
               "humidity_min REAL,"
               "humidity_max REAL,"
               "lighting_conditions TEXT,"
               "security_level INTEGER DEFAULT 1,"
               "max_capacity REAL NOT NULL DEFAULT 1000.0,"
               "current_load REAL NOT NULL DEFAULT 0.0,"
               "is_active INTEGER DEFAULT 1,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)")) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Таблица распределения
    if (!query.exec("CREATE TABLE IF NOT EXISTS batches ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "chemical_id INTEGER NOT NULL,"
               "zone_id INTEGER NOT NULL,"
               "quantity REAL NOT NULL,"
               "notes TEXT,"
               "placed_date DATE DEFAULT CURRENT_DATE,"
               "FOREIGN KEY (chemical_id) REFERENCES chemicals(id) ON DELETE CASCADE,"
               "FOREIGN KEY (zone_id) REFERENCES storage_zones(id) ON DELETE CASCADE)")) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Таблица резервных копий
    query.exec("CREATE TABLE IF NOT EXISTS backups ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "filename TEXT NOT NULL,"
               "file_size_mb REAL,"
               "comment TEXT,"
               "restored INTEGER DEFAULT 0,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
    
    // Таблица оповещений
    query.exec("CREATE TABLE IF NOT EXISTS alerts ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "alert_type TEXT NOT NULL,"
               "message TEXT NOT NULL,"
               "severity INTEGER DEFAULT 1,"
               "is_resolved INTEGER DEFAULT 0,"
               "resolved_by TEXT,"
               "resolved_at TIMESTAMP,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
    
    // Таблица журнала действий
    query.exec("CREATE TABLE IF NOT EXISTS activity_log ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "action TEXT NOT NULL,"
               "details TEXT,"
               "user TEXT DEFAULT 'system',"
               "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
    
    return true;
}

bool Database::insertTestData() {
    std::cout << "Inserting test data..." << std::endl;
    
    QSqlQuery query(m_db);
    
    // Добавляем тестовые зоны
    QStringList zones = {
        "Холодильная камера +4°C",
        "Морозильная камера -20°C",
        "Шкаф для горючих веществ",
        "Шкаф для кислот",
        "Шкаф для щелочей"
    };
    
    for (int i = 0; i < zones.size(); ++i) {
        query.prepare("INSERT INTO storage_zones (name, description, max_capacity) "
                      "VALUES (:name, :desc, :capacity)");
        query.bindValue(":name", zones[i]);
        query.bindValue(":desc", QString("Тестовая зона %1").arg(i+1));
        query.bindValue(":capacity", 1000.0 + (i * 500));
        query.exec();
    }
    
    // Добавляем тестовые химикаты
    QStringList chemicals = {
        "Вода дистиллированная", "Этанол", "Соляная кислота", "Гидроксид натрия",
        "Ацетон", "Метанол", "Бензол", "Толуол"
    };
    
    QStringList formulas = {
        "H2O", "C2H5OH", "HCl", "NaOH", "CH3COCH3", "CH3OH", "C6H6", "C7H8"
    };
    
    for (int i = 0; i < chemicals.size(); ++i) {
        query.prepare("INSERT INTO chemicals (name, formula, quantity, unit, hazard_class, purity) "
                      "VALUES (:name, :formula, :qty, :unit, :hazard, :purity)");
        query.bindValue(":name", chemicals[i]);
        query.bindValue(":formula", formulas[i]);
        query.bindValue(":qty", 100.0 + (i * 50));
        query.bindValue(":unit", i % 2 == 0 ? "л" : "кг");
        query.bindValue(":hazard", (i % 5) + 1);
        query.bindValue(":purity", 95.0 + (i % 5));
        query.exec();
    }
    
    logActivity("Инициализация", "Добавлены тестовые данные");
    return true;
}

// ============ ХИМИКАТЫ ============

bool Database::addChemical(const QMap<QString, QVariant> &data) {
    QString error;
    if (!validateChemicalData(data, error)) {
        m_lastError = error;
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO chemicals (name, formula, cas_number, manufacturer, supplier, "
                  "purity, quantity, unit, hazard_class, storage_conditions, expiration_date, "
                  "arrival_date, notes) "
                  "VALUES (:name, :formula, :cas, :manuf, :supp, :purity, :qty, "
                  ":unit, :hazard, :storage, :exp_date, DATE('now'), :notes)");
    
    query.bindValue(":name", data["name"].toString());
    query.bindValue(":formula", data["formula"].toString());
    query.bindValue(":cas", data["cas_number"].toString());
    query.bindValue(":manuf", data["manufacturer"].toString());
    query.bindValue(":supp", data["supplier"].toString());
    query.bindValue(":purity", data["purity"].toDouble());
    query.bindValue(":qty", data["quantity"].toDouble());
    query.bindValue(":unit", data["unit"].toString());
    query.bindValue(":hazard", data["hazard_class"].toInt());
    query.bindValue(":storage", data["storage_conditions"].toString());
    query.bindValue(":exp_date", data["expiration_date"].toDate());
    query.bindValue(":notes", data["notes"].toString());
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Добавление химиката", data["name"].toString());
    emit dataChanged();
    return true;
}

bool Database::updateChemical(int id, const QMap<QString, QVariant> &data) {
    QString error;
    if (!validateChemicalData(data, error)) {
        m_lastError = error;
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE chemicals SET "
                  "name = :name, formula = :formula, cas_number = :cas, "
                  "manufacturer = :manuf, supplier = :supp, "
                  "purity = :purity, quantity = :qty, unit = :unit, "
                  "hazard_class = :hazard, storage_conditions = :storage, "
                  "expiration_date = :exp_date, notes = :notes "
                  "WHERE id = :id");
    
    query.bindValue(":id", id);
    query.bindValue(":name", data["name"].toString());
    query.bindValue(":formula", data["formula"].toString());
    query.bindValue(":cas", data["cas_number"].toString());
    query.bindValue(":manuf", data["manufacturer"].toString());
    query.bindValue(":supp", data["supplier"].toString());
    query.bindValue(":purity", data["purity"].toDouble());
    query.bindValue(":qty", data["quantity"].toDouble());
    query.bindValue(":unit", data["unit"].toString());
    query.bindValue(":hazard", data["hazard_class"].toInt());
    query.bindValue(":storage", data["storage_conditions"].toString());
    query.bindValue(":exp_date", data["expiration_date"].toDate());
    query.bindValue(":notes", data["notes"].toString());
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Обновление химиката", QString("ID: %1, Название: %2").arg(id).arg(data["name"].toString()));
    emit dataChanged();
    return true;
}

bool Database::deleteChemical(int id) {
    // Начинаем транзакцию
    m_db.transaction();
    
    QSqlQuery query(m_db);
    
    // Удаляем все распределения этого химиката
    query.prepare("DELETE FROM batches WHERE chemical_id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Удаляем химикат
    query.prepare("DELETE FROM chemicals WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    m_db.commit();
    
    logActivity("Удаление химиката", QString("ID: %1").arg(id));
    emit dataChanged();
    return true;
}

QList<QMap<QString, QVariant>> Database::getAllChemicals() {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM chemicals ORDER BY name");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

QList<QMap<QString, QVariant>> Database::searchChemicals(const QString &queryText, int hazardClass) {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    QString sql = "SELECT * FROM chemicals WHERE ";
    QStringList conditions;
    
    if (!queryText.isEmpty()) {
        conditions.append("(name LIKE :query OR formula LIKE :query OR cas_number LIKE :query "
                          "OR manufacturer LIKE :query OR supplier LIKE :query)");
    }
    
    if (hazardClass > 0) {
        conditions.append("hazard_class = :hazard");
    }
    
    if (conditions.isEmpty()) {
        sql = "SELECT * FROM chemicals";
    } else {
        sql += conditions.join(" AND ");
    }
    
    sql += " ORDER BY name";
    
    query.prepare(sql);
    
    if (!queryText.isEmpty()) {
        query.bindValue(":query", QString("%%1%").arg(queryText));
    }
    
    if (hazardClass > 0) {
        query.bindValue(":hazard", hazardClass);
    }
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

QMap<QString, QVariant> Database::getChemicalById(int id) {
    QMap<QString, QVariant> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM chemicals WHERE id = :id");
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            result[record.fieldName(i)] = record.value(i);
        }
    }
    
    return result;
}

QList<QMap<QString, QVariant>> Database::getChemicalsByZone(int zoneId) {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT c.* FROM chemicals c "
                  "JOIN batches b ON c.id = b.chemical_id "
                  "WHERE b.zone_id = :zone_id");
    query.bindValue(":zone_id", zoneId);
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

int Database::getChemicalCount() const {
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM chemicals");
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

// ============ ЗОНЫ ХРАНЕНИЯ ============

bool Database::addZone(const QMap<QString, QVariant> &data) {
    QString error;
    if (!validateZoneData(data, error)) {
        m_lastError = error;
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO storage_zones (name, description, temperature_min, temperature_max, "
                  "humidity_min, humidity_max, lighting_conditions, security_level, "
                  "max_capacity, is_active) "
                  "VALUES (:name, :desc, :temp_min, :temp_max, "
                  ":hum_min, :hum_max, :lighting, :security, :capacity, :active)");
    
    query.bindValue(":name", data["name"].toString());
    query.bindValue(":desc", data["description"].toString());
    query.bindValue(":temp_min", data["temperature_min"].toDouble());
    query.bindValue(":temp_max", data["temperature_max"].toDouble());
    query.bindValue(":hum_min", data["humidity_min"].toDouble());
    query.bindValue(":hum_max", data["humidity_max"].toDouble());
    query.bindValue(":lighting", data["lighting_conditions"].toString());
    query.bindValue(":security", data["security_level"].toInt());
    query.bindValue(":capacity", data["max_capacity"].toDouble());
    query.bindValue(":active", data["is_active"].toBool());
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Добавление зоны", data["name"].toString());
    emit dataChanged();
    return true;
}

bool Database::updateZone(int id, const QMap<QString, QVariant> &data) {
    QString error;
    if (!validateZoneData(data, error)) {
        m_lastError = error;
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("UPDATE storage_zones SET "
                  "name = :name, description = :desc, "
                  "temperature_min = :temp_min, temperature_max = :temp_max, "
                  "humidity_min = :hum_min, humidity_max = :hum_max, "
                  "lighting_conditions = :lighting, security_level = :security, "
                  "max_capacity = :capacity, is_active = :active "
                  "WHERE id = :id");
    
    query.bindValue(":id", id);
    query.bindValue(":name", data["name"].toString());
    query.bindValue(":desc", data["description"].toString());
    query.bindValue(":temp_min", data["temperature_min"].toDouble());
    query.bindValue(":temp_max", data["temperature_max"].toDouble());
    query.bindValue(":hum_min", data["humidity_min"].toDouble());
    query.bindValue(":hum_max", data["humidity_max"].toDouble());
    query.bindValue(":lighting", data["lighting_conditions"].toString());
    query.bindValue(":security", data["security_level"].toInt());
    query.bindValue(":capacity", data["max_capacity"].toDouble());
    query.bindValue(":active", data["is_active"].toBool());
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Обновление зоны", QString("ID: %1, Название: %2").arg(id).arg(data["name"].toString()));
    emit dataChanged();
    return true;
}

bool Database::deleteZone(int id) {
    // Проверяем, нет ли химикатов в зоне
    QList<QMap<QString, QVariant>> chemicals = getChemicalsByZone(id);
    if (!chemicals.isEmpty()) {
        m_lastError = "Невозможно удалить зону: в ней находятся химикаты";
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM storage_zones WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Удаление зоны", QString("ID: %1").arg(id));
    emit dataChanged();
    return true;
}

QList<QMap<QString, QVariant>> Database::getAllStorageZones() {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT *, "
                  "(SELECT COUNT(*) FROM batches WHERE zone_id = storage_zones.id) as chemical_count, "
                  "CASE WHEN max_capacity > 0 THEN (current_load / max_capacity) * 100 ELSE 0 END as load_percentage "
                  "FROM storage_zones ORDER BY name");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

QMap<QString, QVariant> Database::getZoneById(int id) {
    QMap<QString, QVariant> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM storage_zones WHERE id = :id");
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            result[record.fieldName(i)] = record.value(i);
        }
    }
    
    return result;
}

double Database::getZoneAvailableCapacity(int zoneId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT max_capacity - current_load FROM storage_zones WHERE id = :id AND is_active = 1");
    query.bindValue(":id", zoneId);
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

int Database::getZoneCount() const {
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM storage_zones");
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

// ============ РАСПРЕДЕЛЕНИЕ ============

bool Database::distributeChemical(int chemicalId, int zoneId, double quantity, const QString &notes) {
    QString error;
    if (!validateDistributionData(chemicalId, zoneId, quantity, error)) {
        m_lastError = error;
        return false;
    }
    
    // Получаем химикат
    QMap<QString, QVariant> chemical = getChemicalById(chemicalId);
    if (chemical.isEmpty()) {
        m_lastError = "Химикат не найден";
        return false;
    }
    
    // Проверяем доступное количество
    double availableQty = chemical["quantity"].toDouble();
    if (quantity > availableQty) {
        m_lastError = QString("Недостаточно химиката. Доступно: %1, требуется: %2")
                     .arg(availableQty).arg(quantity);
        return false;
    }
    
    // Проверяем доступную емкость зоны
    double availableCapacity = getZoneAvailableCapacity(zoneId);
    if (quantity > availableCapacity) {
        m_lastError = QString("Недостаточно места в зоне. Доступно: %1, требуется: %2")
                     .arg(availableCapacity).arg(quantity);
        return false;
    }
    
    // Начинаем транзакцию
    m_db.transaction();
    
    QSqlQuery query(m_db);
    
    // Уменьшаем количество химиката на складе
    query.prepare("UPDATE chemicals SET quantity = quantity - :qty WHERE id = :id");
    query.bindValue(":id", chemicalId);
    query.bindValue(":qty", quantity);
    
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Увеличиваем загрузку зоны
    query.prepare("UPDATE storage_zones SET current_load = current_load + :qty WHERE id = :id");
    query.bindValue(":id", zoneId);
    query.bindValue(":qty", quantity);
    
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Добавляем запись о распределении
    query.prepare("INSERT INTO batches (chemical_id, zone_id, quantity, notes, placed_date) "
                  "VALUES (:chem_id, :zone_id, :qty, :notes, DATE('now'))");
    query.bindValue(":chem_id", chemicalId);
    query.bindValue(":zone_id", zoneId);
    query.bindValue(":qty", quantity);
    query.bindValue(":notes", notes);
    
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    m_db.commit();
    
    logActivity("Распределение", QString("Хим: %1 -> Зона: %2, Количество: %3")
                .arg(chemicalId).arg(zoneId).arg(quantity));
    emit dataChanged();
    return true;
}

bool Database::removeFromZone(int batchId, double quantity) {
    // Получаем информацию о распределении
    QMap<QString, QVariant> batch = getBatchById(batchId);
    if (batch.isEmpty()) {
        m_lastError = "Распределение не найдено";
        return false;
    }
    
    double currentQty = batch["quantity"].toDouble();
    int chemicalId = batch["chemical_id"].toInt();
    int zoneId = batch["zone_id"].toInt();
    
    if (quantity > currentQty) {
        m_lastError = "Нельзя удалить больше, чем есть в зоне";
        return false;
    }
    
    // Начинаем транзакцию
    m_db.transaction();
    
    QSqlQuery query(m_db);
    
    if (quantity == currentQty) {
        // Удаляем всю запись
        query.prepare("DELETE FROM batches WHERE id = :id");
        query.bindValue(":id", batchId);
    } else {
        // Уменьшаем количество
        query.prepare("UPDATE batches SET quantity = quantity - :qty WHERE id = :id");
        query.bindValue(":id", batchId);
        query.bindValue(":qty", quantity);
    }
    
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Возвращаем химикат на склад
    query.prepare("UPDATE chemicals SET quantity = quantity + :qty WHERE id = :id");
    query.bindValue(":id", chemicalId);
    query.bindValue(":qty", quantity);
    
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    // Уменьшаем загрузку зоны
    query.prepare("UPDATE storage_zones SET current_load = current_load - :qty WHERE id = :id");
    query.bindValue(":id", zoneId);
    query.bindValue(":qty", quantity);
    
    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        return false;
    }
    
    m_db.commit();
    
    logActivity("Удаление из зоны", QString("ID: %1, количество: %2").arg(batchId).arg(quantity));
    emit dataChanged();
    return true;
}

QList<QMap<QString, QVariant>> Database::getAllBatches() {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT b.*, c.name as chemical_name, z.name as zone_name "
                  "FROM batches b "
                  "JOIN chemicals c ON b.chemical_id = c.id "
                  "JOIN storage_zones z ON b.zone_id = z.id "
                  "ORDER BY b.placed_date DESC");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

QMap<QString, QVariant> Database::getBatchById(int id) {
    QMap<QString, QVariant> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT b.*, c.name as chemical_name, z.name as zone_name "
                  "FROM batches b "
                  "JOIN chemicals c ON b.chemical_id = c.id "
                  "JOIN storage_zones z ON b.zone_id = z.id "
                  "WHERE b.id = :id");
    query.bindValue(":id", id);
    
    if (query.exec() && query.next()) {
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            result[record.fieldName(i)] = record.value(i);
        }
    }
    
    return result;
}

// ============ СТАТИСТИКА ============

QMap<QString, QVariant> Database::getStatistics() {
    QMap<QString, QVariant> stats;
    
    stats["chemical_count"] = getChemicalCount();
    stats["zone_count"] = getZoneCount();
    
    QSqlQuery query(m_db);
    
    // Общее количество
    query.prepare("SELECT SUM(quantity) FROM chemicals");
    if (query.exec() && query.next()) {
        stats["total_quantity"] = query.value(0).toDouble();
    }
    
    // Средний класс опасности
    query.prepare("SELECT AVG(hazard_class) FROM chemicals");
    if (query.exec() && query.next()) {
        stats["avg_hazard"] = query.value(0).toDouble();
    }
    
    return stats;
}

// ============ БЭКАПЫ ============

bool Database::createBackup(const QString &comment) {
    // Закрываем текущее соединение
    if (m_db.isOpen()) {
        m_db.close();
    }
    
    // Создаем имя файла для бекапа
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backupDir = getBackupDir();
    QDir dir(backupDir);
    if (!dir.exists()) {
        dir.mkpath(backupDir);
    }
    
    QString backupFile = backupDir + QDir::separator() + QString("backup_%1.db").arg(timestamp);
    
    // Копируем файл базы данных
    QFile dbFile(m_databasePath);
    if (!dbFile.exists()) {
        m_lastError = "Файл базы данных не найден";
        m_db.open();
        return false;
    }
    
    if (!dbFile.copy(backupFile)) {
        m_lastError = QString("Не удалось создать резервную копию: %1").arg(dbFile.errorString());
        m_db.open();
        return false;
    }
    
    // Открываем базу снова
    if (!m_db.open()) {
        m_lastError = "Не удалось открыть базу данных после создания бекапа";
        return false;
    }
    
    // Записываем информацию о бекапе в базу
    QFileInfo fi(backupFile);
    double sizeMB = fi.size() / (1024.0 * 1024.0);
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO backups (filename, file_size_mb, comment) "
                  "VALUES (:filename, :size, :comment)");
    query.bindValue(":filename", fi.fileName());
    query.bindValue(":size", sizeMB);
    query.bindValue(":comment", comment);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Создание резервной копии", QString("Файл: %1").arg(fi.fileName()));
    emit backupCreated(backupFile);
    return true;
}

bool Database::restoreBackup(const QString &backupPath) {
    if (!QFile::exists(backupPath)) {
        m_lastError = "Файл резервной копии не найден";
        return false;
    }
    
    // Закрываем текущее соединение
    if (m_db.isOpen()) {
        m_db.close();
    }
    
    // Создаем резервную копию текущей базы
    QString currentBackup = m_databasePath + ".backup";
    if (QFile::exists(m_databasePath)) {
        QFile::remove(currentBackup);
        QFile::copy(m_databasePath, currentBackup);
    }
    
    // Копируем бекап на место текущей базы
    if (QFile::exists(m_databasePath)) {
        QFile::remove(m_databasePath);
    }
    
    if (!QFile::copy(backupPath, m_databasePath)) {
        // Восстанавливаем из бекапа
        if (QFile::exists(currentBackup)) {
            QFile::copy(currentBackup, m_databasePath);
        }
        m_lastError = "Не удалось восстановить базу данных";
        m_db.open();
        return false;
    }
    
    // Открываем базу данных
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    
    logActivity("Восстановление из резервной копии", QString("Файл: %1").arg(QFileInfo(backupPath).fileName()));
    emit backupRestored(true);
    emit dataChanged();
    return true;
}

QList<QMap<QString, QVariant>> Database::getAllBackups() {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT *, datetime(created_at, 'localtime') as local_timestamp FROM backups "
                  "ORDER BY created_at DESC");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            
            // Проверяем существование файла
            QString backupDir = getBackupDir();
            QString filePath = backupDir + QDir::separator() + record["filename"].toString();
            record["file_exists"] = QFile::exists(filePath);
            
            result.append(record);
        }
    }
    
    return result;
}

bool Database::deleteBackup(int backupId) {
    // Получаем информацию о бекапе
    QSqlQuery query(m_db);
    query.prepare("SELECT filename FROM backups WHERE id = :id");
    query.bindValue(":id", backupId);
    
    if (!query.exec() || !query.next()) {
        m_lastError = "Бекап не найден";
        return false;
    }
    
    QString filename = query.value(0).toString();
    QString filePath = getBackupDir() + QDir::separator() + filename;
    
    // Удаляем файл
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }
    
    // Удаляем запись из базы данных
    query.prepare("DELETE FROM backups WHERE id = :id");
    query.bindValue(":id", backupId);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Удаление бекапа", filename);
    return true;
}

QString Database::getBackupDir() const {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataDir + QDir::separator() + "backups";
}

// ============ ЛОГИРОВАНИЕ ============

void Database::logActivity(const QString &action, const QString &details, const QString &user) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO activity_log (action, details, user) "
                  "VALUES (:action, :details, :user)");
    query.bindValue(":action", action);
    query.bindValue(":details", details);
    query.bindValue(":user", user);
    query.exec();
}

QList<QMap<QString, QVariant>> Database::getDetailedActivityLog(int limit) {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT *, datetime(timestamp, 'localtime') as local_time "
                  "FROM activity_log ORDER BY timestamp DESC LIMIT :limit");
    query.bindValue(":limit", limit);
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

bool Database::clearOldLogs(int daysToKeep) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM activity_log WHERE "
                  "julianday('now') - julianday(timestamp) > :days");
    query.bindValue(":days", daysToKeep);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Очистка журнала", QString("Удаление записей старше %1 дней").arg(daysToKeep));
    return true;
}

// ============ ОПОВЕЩЕНИЯ ============

void Database::checkAlerts() {
    // Простая проверка без сложной логики
}

QList<QMap<QString, QVariant>> Database::getActiveAlerts() {
    QList<QMap<QString, QVariant>> result;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT *, datetime(created_at, 'localtime') as local_created FROM alerts "
                  "WHERE is_resolved = 0 ORDER BY created_at DESC");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> record;
            QSqlRecord sqlRecord = query.record();
            for (int i = 0; i < sqlRecord.count(); ++i) {
                record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
            }
            result.append(record);
        }
    }
    
    return result;
}

void Database::addAlert(const QString &type, const QString &message, int severity) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO alerts (alert_type, message, severity) "
                  "VALUES (:type, :message, :severity)");
    query.bindValue(":type", type);
    query.bindValue(":message", message);
    query.bindValue(":severity", severity);
    query.exec();
}

bool Database::markAlertAsResolved(int alertId, const QString &resolvedBy) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE alerts SET is_resolved = 1, resolved_by = :resolved_by, "
                  "resolved_at = datetime('now', 'localtime') WHERE id = :id");
    query.bindValue(":id", alertId);
    query.bindValue(":resolved_by", resolvedBy);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    logActivity("Разрешение оповещения", QString("ID: %1").arg(alertId));
    return true;
}

// ============ ЭКСПОРТ ============

bool Database::exportChemicalsToCSV(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Не удалось открыть файл: %1").arg(file.errorString());
        return false;
    }
    
    QTextStream out(&file);
    
    // Заголовок
    out << "ID;Название;Формула;CAS;Производитель;Количество;Ед.изм;Класс опасности;Срок годности\n";
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM chemicals ORDER BY name");
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    
    while (query.next()) {
        out << query.value("id").toString() << ";"
            << query.value("name").toString() << ";"
            << query.value("formula").toString() << ";"
            << query.value("cas_number").toString() << ";"
            << query.value("manufacturer").toString() << ";"
            << query.value("quantity").toString() << ";"
            << query.value("unit").toString() << ";"
            << query.value("hazard_class").toString() << ";"
            << query.value("expiration_date").toString() << "\n";
    }
    
    file.close();
    logActivity("Экспорт в CSV", QString("Файл: %1").arg(filePath));
    return true;
}

bool Database::exportToCSV(const QString &filePath) {
    return exportChemicalsToCSV(filePath);
}

// ============ НАСТРОЙКИ ============

QVariant Database::getSetting(const QString &key, const QVariant &defaultValue) {
    return m_settings->value(key, defaultValue);
}

void Database::setSetting(const QString &key, const QVariant &value) {
    m_settings->setValue(key, value);
}

// ============ ВАЛИДАЦИЯ ============

bool Database::validateChemicalData(const QMap<QString, QVariant> &data, QString &error) {
    if (!data.contains("name") || data["name"].toString().trimmed().isEmpty()) {
        error = "Название химиката обязательно";
        return false;
    }
    
    if (data.contains("quantity") && data["quantity"].toDouble() < 0) {
        error = "Количество не может быть отрицательным";
        return false;
    }
    
    return true;
}

bool Database::validateZoneData(const QMap<QString, QVariant> &data, QString &error) {
    if (!data.contains("name") || data["name"].toString().trimmed().isEmpty()) {
        error = "Название зоны обязательно";
        return false;
    }
    
    return true;
}

bool Database::validateDistributionData(int chemicalId, int zoneId, double quantity, QString &error) {
    if (chemicalId <= 0) {
        error = "Неверный идентификатор химиката";
        return false;
    }
    
    if (zoneId <= 0) {
        error = "Неверный идентификатор зоны";
        return false;
    }
    
    if (quantity <= 0) {
        error = "Количество должно быть положительным";
        return false;
    }
    
    return true;
}

// ============ УТИЛИТЫ ============

QSqlDatabase Database::getDatabase() const {
    return m_db;
}

QString Database::getDatabasePath() const {
    return m_databasePath;
}

QString Database::getLastError() const {
    return m_lastError;
}
