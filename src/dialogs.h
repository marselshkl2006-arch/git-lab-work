#ifndef DIALOGS_H
#define DIALOGS_H

#include <QDialog>
#include <QMap>

// Вперед объявления Qt классов
class QLabel;
class QLineEdit;
class QTextEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QDateEdit;
class QCheckBox;
class QTableWidget;
class QPushButton;

class Database;

// Диалог добавления/редактирования химиката
class ChemicalDialog : public QDialog
{
    Q_OBJECT
    
public:
    ChemicalDialog(QWidget *parent, Database *db, int chemicalId = -1);
    
private slots:
    void saveChemical();
    void validateForm();
    
private:
    Database *m_db;
    int m_chemicalId;
    
    QLineEdit *nameEdit;
    QLineEdit *formulaEdit;
    QLineEdit *casEdit;
    QLineEdit *manufacturerEdit;
    QLineEdit *supplierEdit;
    QDoubleSpinBox *puritySpin;
    QDoubleSpinBox *quantitySpin;
    QComboBox *unitCombo;
    QSpinBox *hazardSpin;
    QTextEdit *storageEdit;
    QDateEdit *expirationEdit;
    QTextEdit *notesEdit;
    
    void loadChemicalData();
};

// Диалог добавления/редактирования зоны хранения
class ZoneDialog : public QDialog
{
    Q_OBJECT
    
public:
    ZoneDialog(QWidget *parent, Database *db, int zoneId = -1);
    
private slots:
    void saveZone();
    
private:
    Database *m_db;
    int m_zoneId;
    
    QLineEdit *nameEdit;
    QTextEdit *descriptionEdit;
    QDoubleSpinBox *tempMinSpin;
    QDoubleSpinBox *tempMaxSpin;
    QDoubleSpinBox *humMinSpin;
    QDoubleSpinBox *humMaxSpin;
    QLineEdit *lightingEdit;
    QSpinBox *securitySpin;
    QDoubleSpinBox *capacitySpin;
    QCheckBox *activeCheck;
    
    void loadZoneData();
};

// Диалог распределения химиката по зонам
class DistributeDialog : public QDialog
{
    Q_OBJECT
    
public:
    DistributeDialog(QWidget *parent, Database *db);
    
private slots:
    void chemicalSelected(int index);
    void zoneSelected(int index);
    void updateAvailableQuantity();
    void distribute();
    
private:
    Database *m_db;
    
    QComboBox *chemicalCombo;
    QComboBox *zoneCombo;
    QDoubleSpinBox *quantitySpin;
    QTextEdit *notesEdit;
    QLabel *availableLabel;
    QLabel *zoneCapacityLabel;
    
    void loadChemicals();
    void loadZones();
};

// Диалог управления бекапами
class BackupDialog : public QDialog
{
    Q_OBJECT
    
public:
    BackupDialog(QWidget *parent, Database *db, bool manageMode = false);
    
private slots:
    void refreshBackups();
    void createBackup();
    void restoreSelected();
    void deleteSelected();
    void backupSelected(int row);
    
private:
    Database *m_db;
    bool m_manageMode;
    
    QTableWidget *backupsTable;
    QPushButton *restoreButton;
    QPushButton *deleteButton;
    QLineEdit *commentEdit;
    
    void loadBackups();
};

#endif // DIALOGS_H
