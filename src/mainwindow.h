#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QDate>

// Вперед объявления
QT_BEGIN_NAMESPACE
class QTabWidget;
class QTableWidget;
class QToolBar;
class QStatusBar;
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QTextEdit;
class QDateEdit;
class QDoubleSpinBox;
class QSpinBox;
class QGroupBox;
class QCheckBox;
class QProgressBar;
class QMenu;
class QAction;
class QDialog;
class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QDialogButtonBox;
class QTimer;
class QScreen;
QT_END_NAMESPACE

class Database;
class ChemicalDialog;
class ZoneDialog;
class DistributeDialog;
class BackupDialog;
class SettingsDialog;
class ThemeManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Основные слоты
    void showAbout();
    void refreshAll();
    
    // Химикаты
    void addChemical();
    void editChemical();
    void deleteChemical();
    void searchChemicals();
    void exportChemicalsToCSV();
    void showChemicalDetails();
    void showChemicalContextMenu(const QPoint &pos);
    void chemicalCellDoubleClicked(int row, int column);
    
    // Зоны хранения
    void addStorageZone();
    void editStorageZone();
    void deleteStorageZone();
    void showZoneContextMenu(const QPoint &pos);
    void zoneCellDoubleClicked(int row, int column);
    
    // Распределение
    void distributeChemical();
    void removeDistribution();
    void showDistributionContextMenu(const QPoint &pos);
    
    // Статистика
    void updateStatistics();
    void exportStatistics();
    
    // Оповещения
    void updateAlerts();
    void markAlertAsResolved();
    void showAlertContextMenu(const QPoint &pos);
    
    // Журнал
    void updateActivityLog();
    void clearOldLogs();
    
    // Системные функции
    void backupDatabase();
    void restoreDatabase();
    void manageBackups();
    void exportToCSV();
    void importFromCSV();
    void showSettings();
    
    // Вкладки
    void tabChanged(int index);
    
    // Обновление данных
    void updateChemicalTable();
    void updateZoneTable();
    void updateDistributionTable();

private:
    // Инициализация
    void setupDatabase();
    void createMenu();
    void createToolBar();
    void createCentralWidget();
    void createStatusBar();
    void setupConnections();
    void loadInitialData();
    void setupAutoUpdates();
    
    // Создание таблиц
    void setupChemicalTable();
    void setupZoneTable();
    void setupDistributionTable();
    void setupStatisticsTab();
    void setupAlertsTable();
    void setupActivityLogTable();
    
    // Утилиты
    void updateStatus(const QString &message, int timeout = 3000);
    void showError(const QString &message);
    void showInfo(const QString &message);
    void showSuccess(const QString &message);
    QColor getHazardClassColor(int hazardClass) const;
    QString getHazardClassTooltip(int hazardClass) const;
    QColor getZoneLoadColor(double loadPercent) const;
    QColor getAlertSeverityColor(int severity) const;
    int getSelectedChemicalId() const;
    int getSelectedZoneId() const;
    int getSelectedBatchId() const;
    int getSelectedAlertId() const;
    
    // Виджеты (исправлен порядок инициализации)
    QTabWidget *tabWidget;
    QTableWidget *chemicalTable;
    QTableWidget *zoneTable;
    QTableWidget *distributionTable;
    QTableWidget *alertsTable;
    QTableWidget *activityLogTable;
    
    // Виджеты для статистики
    QLabel *statTotalChemicals;
    QLabel *statTotalZones;
    QLabel *statTotalQuantity;
    QLabel *statAverageHazard;
    QLabel *statExpiringSoon;
    QLabel *statLowStock;
    QLabel *statMostLoadedZone;
    QLabel *statLeastLoadedZone;
    QLabel *statActiveAlerts;
    QLabel *statDatabaseSize;
    QLabel *statLastBackup;
    
    // Панели инструментов
    QToolBar *mainToolBar;
    
    // Статус бар
    QStatusBar *statusBar;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    
    // Поиск и фильтры
    QLineEdit *searchChemicalEdit;
    QComboBox *hazardClassFilter;
    
    // База данных
    Database *db;
    
    // Таймеры
    QTimer *autoUpdateTimer;
    QTimer *alertCheckTimer;
};

#endif // MAINWINDOW_H
