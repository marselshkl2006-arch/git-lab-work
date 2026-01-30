#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QMap>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QDate>

class Database : public QObject
{
    Q_OBJECT
    
signals:
    void backupCreated(const QString &path);
    void backupRestored(bool success);
    void dataChanged();
    void alertTriggered(const QString &message);
    void chemicalExpiring(const QString &chemicalName, const QDate &expirationDate);
    void zoneOverloaded(int zoneId, double loadPercent);
    void chemicalLowStock(int chemicalId, double quantity);
    
public:
    static Database& instance();
    
    // Инициализация
    bool initializeDatabase();
    QSqlDatabase getDatabase() const;
    
    // Химикаты
    bool addChemical(const QMap<QString, QVariant> &data);
    bool updateChemical(int id, const QMap<QString, QVariant> &data);
    bool deleteChemical(int id);
    QList<QMap<QString, QVariant>> getAllChemicals();
    QList<QMap<QString, QVariant>> searchChemicals(const QString &query, int hazardClass = 0);
    QMap<QString, QVariant> getChemicalById(int id);
    QList<QMap<QString, QVariant>> getChemicalsByZone(int zoneId);
    QList<QMap<QString, QVariant>> getExpiringChemicals(int days = 30);
    QList<QMap<QString, QVariant>> getLowStockChemicals(double threshold = 10.0);
    int getChemicalCount() const;
    
    // Зоны хранения
    bool initializeStorageZones();
    QList<QMap<QString, QVariant>> getAllStorageZones();
    QList<QMap<QString, QVariant>> getStorageZonesWithLoad();
    bool addZone(const QMap<QString, QVariant> &data);
    bool updateZone(int id, const QMap<QString, QVariant> &data);
    bool deleteZone(int id);
    QMap<QString, QVariant> getZoneById(int id);
    double getZoneAvailableCapacity(int zoneId);
    bool updateZoneLoad(int zoneId, double delta);
    int getZoneCount() const;
    
    // Распределение
    bool distributeChemical(int chemicalId, int zoneId, double quantity, const QString &notes = "");
    bool removeFromZone(int batchId, double quantity);
    bool updateBatch(int batchId, double quantity, const QString &notes = "");
    QList<QMap<QString, QVariant>> getBatchesByChemical(int chemicalId);
    QList<QMap<QString, QVariant>> getBatchesByZone(int zoneId);
    QList<QMap<QString, QVariant>> getAllBatches();
    QMap<QString, QVariant> getBatchById(int id);
    
    // Статистика
    QMap<QString, QVariant> getStatistics();
    QMap<QString, QVariant> getDetailedStatistics();
    QList<QMap<QString, QVariant>> getHazardClassDistribution();
    QList<QMap<QString, QVariant>> getManufacturerDistribution();
    QList<QMap<QString, QVariant>> getMonthlyConsumption();
    QList<QMap<QString, QVariant>> getZoneLoadStatistics();
    QList<QMap<QString, QVariant>> getExpirationStatistics();
    
    // Резервное копирование
    bool createBackup(const QString &comment = "");
    bool restoreBackup(const QString &backupPath);
    QList<QMap<QString, QVariant>> getAllBackups();
    bool deleteBackup(int backupId);
    bool autoBackup();
    QString getBackupPath() const;
    
    // Логирование
    void logActivity(const QString &action, const QString &details = "", const QString &user = "system");
    QStringList getActivityLog(int limit = 100);
    QList<QMap<QString, QVariant>> getDetailedActivityLog(int limit = 100);
    bool clearOldLogs(int daysToKeep = 30);
    
    // Оповещения и проверки
    void checkAlerts();
    QList<QMap<QString, QVariant>> getActiveAlerts();
    QList<QMap<QString, QVariant>> getAllAlerts(int limit = 100);
    bool markAlertAsResolved(int alertId, const QString &resolvedBy = "system");
    void checkExpiringChemicals();
    void checkZoneOverloads();
    void checkLowStock();
    void addAlert(const QString &type, const QString &message, int severity = 1);
    
    // Экспорт
    bool exportToCSV(const QString &filePath);
    bool exportChemicalsToCSV(const QString &filePath);
    bool exportZonesToCSV(const QString &filePath);
    bool exportBatchesToCSV(const QString &filePath);
    bool exportToJSON(const QString &filePath);
    QString generatePDFReport();
    QString generateTextReport();
    
    // Настройки
    QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());
    void setSetting(const QString &key, const QVariant &value);
    void loadSettings();
    void saveSettings();
    
    // Валидация
    static bool validateChemicalData(const QMap<QString, QVariant> &data, QString &error);
    static bool validateZoneData(const QMap<QString, QVariant> &data, QString &error);
    static bool validateDistributionData(int chemicalId, int zoneId, double quantity, QString &error);
    
    // Утилиты
    QString getDatabasePath() const;
    QString getBackupDir() const;
    QString getLogsDir() const;
    QString getDatabaseHash() const;
    bool vacuumDatabase();
    bool testConnection();
    QString getLastError() const;
    
    // Деструктор должен быть публичным для singleton
    ~Database();

private:
    Database(QObject *parent = nullptr);
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    bool createIndexes();
    bool createTriggers();
    bool insertTestData();
    
    void setupAutoChecks();
    void cleanupOldBackups(int maxBackups = 10);
    QString generateBackupFilename() const;
    QString calculateFileChecksum(const QString &filePath) const;
    
    QSqlDatabase m_db;
    QString m_databasePath;
    QSettings *m_settings;
    mutable QMutex m_mutex;  // Изменяем на mutable
    QTimer *m_autoCheckTimer;
    QString m_lastError;
};

#endif // DATABASE_H
