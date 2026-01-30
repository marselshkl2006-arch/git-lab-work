#include "dialogs.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QDate>
#include <QFileInfo>
#include <QDir>

// ============ ChemicalDialog ============

ChemicalDialog::ChemicalDialog(QWidget *parent, Database *db, int chemicalId)
    : QDialog(parent)
    , m_db(db)
    , m_chemicalId(chemicalId)
{
    setWindowTitle(chemicalId == -1 ? "🧪 Добавить химикат" : "✏️ Редактировать химикат");
    setMinimumWidth(600);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Форма
    QFormLayout *formLayout = new QFormLayout();
    
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Название химиката");
    formLayout->addRow("Название *:", nameEdit);
    
    formulaEdit = new QLineEdit();
    formulaEdit->setPlaceholderText("Например: H2O, C6H12O6");
    formLayout->addRow("Химическая формула:", formulaEdit);
    
    casEdit = new QLineEdit();
    casEdit->setPlaceholderText("Например: 7732-18-5");
    formLayout->addRow("CAS номер:", casEdit);
    
    manufacturerEdit = new QLineEdit();
    manufacturerEdit->setPlaceholderText("Производитель");
    formLayout->addRow("Производитель:", manufacturerEdit);
    
    supplierEdit = new QLineEdit();
    supplierEdit->setPlaceholderText("Поставщик");
    formLayout->addRow("Поставщик:", supplierEdit);
    
    QHBoxLayout *purityLayout = new QHBoxLayout();
    puritySpin = new QDoubleSpinBox();
    puritySpin->setRange(0, 100);
    puritySpin->setDecimals(1);
    puritySpin->setSuffix(" %");
    puritySpin->setValue(100.0);
    purityLayout->addWidget(puritySpin);
    purityLayout->addStretch();
    formLayout->addRow("Чистота:", purityLayout);
    
    QHBoxLayout *quantityLayout = new QHBoxLayout();
    quantitySpin = new QDoubleSpinBox();
    quantitySpin->setRange(0, 1000000);
    quantitySpin->setDecimals(2);
    quantitySpin->setValue(100.0);
    quantityLayout->addWidget(quantitySpin);
    
    unitCombo = new QComboBox();
    unitCombo->addItems({"г", "кг", "мл", "л", "шт", "моль"});
    quantityLayout->addWidget(unitCombo);
    quantityLayout->addStretch();
    formLayout->addRow("Количество *:", quantityLayout);
    
    hazardSpin = new QSpinBox();
    hazardSpin->setRange(1, 5);
    hazardSpin->setValue(3);
    formLayout->addRow("Класс опасности (1-5) *:", hazardSpin);
    
    storageEdit = new QTextEdit();
    storageEdit->setMaximumHeight(60);
    storageEdit->setPlaceholderText("Условия хранения, требования безопасности...");
    formLayout->addRow("Условия хранения:", storageEdit);
    
    expirationEdit = new QDateEdit();
    expirationEdit->setDate(QDate::currentDate().addYears(1));
    expirationEdit->setCalendarPopup(true);
    expirationEdit->setDisplayFormat("dd.MM.yyyy");
    formLayout->addRow("Срок годности:", expirationEdit);
    
    notesEdit = new QTextEdit();
    notesEdit->setMaximumHeight(80);
    notesEdit->setPlaceholderText("Дополнительные примечания...");
    formLayout->addRow("Примечания:", notesEdit);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton(chemicalId == -1 ? "➕ Добавить" : "💾 Сохранить");
    QPushButton *cancelButton = new QPushButton("❌ Отмена");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Подключаем сигналы
    connect(saveButton, &QPushButton::clicked, this, &ChemicalDialog::saveChemical);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(nameEdit, &QLineEdit::textChanged, this, &ChemicalDialog::validateForm);
    connect(quantitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChemicalDialog::validateForm);
    
    // Загружаем данные, если редактируем
    if (chemicalId != -1) {
        loadChemicalData();
    }
    
    validateForm();
}

void ChemicalDialog::loadChemicalData()
{
    QMap<QString, QVariant> chemical = m_db->getChemicalById(m_chemicalId);
    if (chemical.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Химикат не найден");
        reject();
        return;
    }
    
    nameEdit->setText(chemical["name"].toString());
    formulaEdit->setText(chemical["formula"].toString());
    casEdit->setText(chemical["cas_number"].toString());
    manufacturerEdit->setText(chemical["manufacturer"].toString());
    supplierEdit->setText(chemical["supplier"].toString());
    puritySpin->setValue(chemical["purity"].toDouble());
    quantitySpin->setValue(chemical["quantity"].toDouble());
    
    QString unit = chemical["unit"].toString();
    int unitIndex = unitCombo->findText(unit);
    if (unitIndex != -1) {
        unitCombo->setCurrentIndex(unitIndex);
    }
    
    hazardSpin->setValue(chemical["hazard_class"].toInt());
    storageEdit->setText(chemical["storage_conditions"].toString());
    
    QDate expDate = chemical["expiration_date"].toDate();
    if (expDate.isValid()) {
        expirationEdit->setDate(expDate);
    }
    
    notesEdit->setText(chemical["notes"].toString());
}

void ChemicalDialog::saveChemical()
{
    QMap<QString, QVariant> data;
    data["name"] = nameEdit->text().trimmed();
    data["formula"] = formulaEdit->text().trimmed();
    data["cas_number"] = casEdit->text().trimmed();
    data["manufacturer"] = manufacturerEdit->text().trimmed();
    data["supplier"] = supplierEdit->text().trimmed();
    data["purity"] = puritySpin->value();
    data["quantity"] = quantitySpin->value();
    data["unit"] = unitCombo->currentText();
    data["hazard_class"] = hazardSpin->value();
    data["storage_conditions"] = storageEdit->toPlainText().trimmed();
    data["expiration_date"] = expirationEdit->date();
    data["notes"] = notesEdit->toPlainText().trimmed();
    
    QString error;
    if (!Database::validateChemicalData(data, error)) {
        QMessageBox::warning(this, "Ошибка валидации", error);
        return;
    }
    
    bool success;
    if (m_chemicalId == -1) {
        success = m_db->addChemical(data);
    } else {
        success = m_db->updateChemical(m_chemicalId, data);
    }
    
    if (success) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось сохранить химикат:\n" + m_db->getLastError());
    }
}

void ChemicalDialog::validateForm()
{
    // Валидация формы
    
    // Подсвечиваем обязательные поля
    QColor requiredColor = nameEdit->text().trimmed().isEmpty() ? 
                          QColor(255, 200, 200) : QColor(255, 255, 255);
    nameEdit->setStyleSheet(QString("background-color: %1").arg(requiredColor.name()));
    
    requiredColor = quantitySpin->value() <= 0 ? 
                   QColor(255, 200, 200) : QColor(255, 255, 255);
    QString style = QString("QDoubleSpinBox { background-color: %1; }").arg(requiredColor.name());
    quantitySpin->setStyleSheet(style);
}

// ============ ZoneDialog ============

ZoneDialog::ZoneDialog(QWidget *parent, Database *db, int zoneId)
    : QDialog(parent)
    , m_db(db)
    , m_zoneId(zoneId)
{
    setWindowTitle(zoneId == -1 ? "🏢 Добавить зону хранения" : "⚙️ Редактировать зону хранения");
    setMinimumWidth(500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Форма
    QFormLayout *formLayout = new QFormLayout();
    
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Например: Холодильная камера +4°C");
    formLayout->addRow("Название *:", nameEdit);
    
    descriptionEdit = new QTextEdit();
    descriptionEdit->setMaximumHeight(60);
    descriptionEdit->setPlaceholderText("Описание зоны, назначение...");
    formLayout->addRow("Описание:", descriptionEdit);
    
    QHBoxLayout *tempLayout = new QHBoxLayout();
    tempMinSpin = new QDoubleSpinBox();
    tempMinSpin->setRange(-100, 100);
    tempMinSpin->setDecimals(1);
    tempMinSpin->setSuffix(" °C");
    tempMinSpin->setSpecialValueText("Не задано");
    
    tempMaxSpin = new QDoubleSpinBox();
    tempMaxSpin->setRange(-100, 100);
    tempMaxSpin->setDecimals(1);
    tempMaxSpin->setSuffix(" °C");
    tempMaxSpin->setSpecialValueText("Не задано");
    
    tempLayout->addWidget(new QLabel("от"));
    tempLayout->addWidget(tempMinSpin);
    tempLayout->addWidget(new QLabel("до"));
    tempLayout->addWidget(tempMaxSpin);
    tempLayout->addStretch();
    formLayout->addRow("Температура:", tempLayout);
    
    QHBoxLayout *humLayout = new QHBoxLayout();
    humMinSpin = new QDoubleSpinBox();
    humMinSpin->setRange(0, 100);
    humMinSpin->setDecimals(1);
    humMinSpin->setSuffix(" %");
    humMinSpin->setSpecialValueText("Не задано");
    
    humMaxSpin = new QDoubleSpinBox();
    humMaxSpin->setRange(0, 100);
    humMaxSpin->setDecimals(1);
    humMaxSpin->setSuffix(" %");
    humMaxSpin->setSpecialValueText("Не задано");
    
    humLayout->addWidget(new QLabel("от"));
    humLayout->addWidget(humMinSpin);
    humLayout->addWidget(new QLabel("до"));
    humLayout->addWidget(humMaxSpin);
    humLayout->addStretch();
    formLayout->addRow("Влажность:", humLayout);
    
    lightingEdit = new QLineEdit();
    lightingEdit->setPlaceholderText("Например: Темнота, естественное освещение");
    formLayout->addRow("Освещение:", lightingEdit);
    
    securitySpin = new QSpinBox();
    securitySpin->setRange(1, 5);
    securitySpin->setValue(1);
    formLayout->addRow("Уровень безопасности (1-5):", securitySpin);
    
    capacitySpin = new QDoubleSpinBox();
    capacitySpin->setRange(0.1, 1000000);
    capacitySpin->setDecimals(2);
    capacitySpin->setValue(1000.0);
    formLayout->addRow("Максимальная емкость *:", capacitySpin);
    
    activeCheck = new QCheckBox("Активная зона");
    activeCheck->setChecked(true);
    formLayout->addRow("Статус:", activeCheck);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton(zoneId == -1 ? "➕ Добавить" : "💾 Сохранить");
    QPushButton *cancelButton = new QPushButton("❌ Отмена");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Подключаем сигналы
    connect(saveButton, &QPushButton::clicked, this, &ZoneDialog::saveZone);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Загружаем данные, если редактируем
    if (zoneId != -1) {
        loadZoneData();
    }
}

void ZoneDialog::loadZoneData()
{
    QMap<QString, QVariant> zone = m_db->getZoneById(m_zoneId);
    if (zone.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Зона не найдена");
        reject();
        return;
    }
    
    nameEdit->setText(zone["name"].toString());
    descriptionEdit->setText(zone["description"].toString());
    tempMinSpin->setValue(zone["temperature_min"].toDouble());
    tempMaxSpin->setValue(zone["temperature_max"].toDouble());
    humMinSpin->setValue(zone["humidity_min"].toDouble());
    humMaxSpin->setValue(zone["humidity_max"].toDouble());
    lightingEdit->setText(zone["lighting_conditions"].toString());
    securitySpin->setValue(zone["security_level"].toInt());
    capacitySpin->setValue(zone["max_capacity"].toDouble());
    activeCheck->setChecked(zone["is_active"].toBool());
}

void ZoneDialog::saveZone()
{
    QMap<QString, QVariant> data;
    data["name"] = nameEdit->text().trimmed();
    data["description"] = descriptionEdit->toPlainText().trimmed();
    data["temperature_min"] = tempMinSpin->value();
    data["temperature_max"] = tempMaxSpin->value();
    data["humidity_min"] = humMinSpin->value();
    data["humidity_max"] = humMaxSpin->value();
    data["lighting_conditions"] = lightingEdit->text().trimmed();
    data["security_level"] = securitySpin->value();
    data["max_capacity"] = capacitySpin->value();
    data["is_active"] = activeCheck->isChecked();
    
    QString error;
    if (!Database::validateZoneData(data, error)) {
        QMessageBox::warning(this, "Ошибка валидации", error);
        return;
    }
    
    bool success;
    if (m_zoneId == -1) {
        success = m_db->addZone(data);
    } else {
        success = m_db->updateZone(m_zoneId, data);
    }
    
    if (success) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось сохранить зону:\n" + m_db->getLastError());
    }
}

// ============ DistributeDialog ============

DistributeDialog::DistributeDialog(QWidget *parent, Database *db)
    : QDialog(parent)
    , m_db(db)
{
    setWindowTitle("📦 Распределить химикат по зонам");
    setMinimumWidth(500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Форма
    QFormLayout *formLayout = new QFormLayout();
    
    chemicalCombo = new QComboBox();
    chemicalCombo->addItem("Выберите химикат...", -1);
    formLayout->addRow("Химикат *:", chemicalCombo);
    
    zoneCombo = new QComboBox();
    zoneCombo->addItem("Выберите зону...", -1);
    formLayout->addRow("Зона хранения *:", zoneCombo);
    
    availableLabel = new QLabel("Доступно: 0");
    availableLabel->setStyleSheet("color: #2E86C1; font-weight: bold;");
    formLayout->addRow("Доступное количество:", availableLabel);
    
    zoneCapacityLabel = new QLabel("Свободно в зоне: 0");
    zoneCapacityLabel->setStyleSheet("color: #27AE60; font-weight: bold;");
    formLayout->addRow("Доступная емкость:", zoneCapacityLabel);
    
    QHBoxLayout *quantityLayout = new QHBoxLayout();
    quantitySpin = new QDoubleSpinBox();
    quantitySpin->setRange(0.01, 1000000);
    quantitySpin->setDecimals(2);
    quantitySpin->setValue(100.0);
    quantityLayout->addWidget(quantitySpin);
    quantityLayout->addStretch();
    formLayout->addRow("Количество для распределения *:", quantityLayout);
    
    notesEdit = new QTextEdit();
    notesEdit->setMaximumHeight(60);
    notesEdit->setPlaceholderText("Примечания к распределению...");
    formLayout->addRow("Примечания:", notesEdit);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *distributeButton = new QPushButton("📦 Распределить");
    QPushButton *cancelButton = new QPushButton("❌ Отмена");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(distributeButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Подключаем сигналы
    connect(distributeButton, &QPushButton::clicked, this, &DistributeDialog::distribute);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(chemicalCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DistributeDialog::chemicalSelected);
    connect(zoneCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DistributeDialog::zoneSelected);
    connect(quantitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DistributeDialog::updateAvailableQuantity);
    
    // Загружаем данные
    loadChemicals();
    loadZones();
    
    updateAvailableQuantity();
}

void DistributeDialog::loadChemicals()
{
    QList<QMap<QString, QVariant>> chemicals = m_db->getAllChemicals();
    for (const auto &chem : chemicals) {
        QString text = QString("%1 (%2, доступно: %3 %4)")
                      .arg(chem["name"].toString())
                      .arg(chem["formula"].toString())
                      .arg(QString::number(chem["quantity"].toDouble(), 'f', 2))
                      .arg(chem["unit"].toString());
        chemicalCombo->addItem(text, chem["id"].toInt());
    }
}

void DistributeDialog::loadZones()
{
    QList<QMap<QString, QVariant>> zones = m_db->getAllStorageZones();
    for (const auto &zone : zones) {
        if (!zone["is_active"].toBool()) continue;
        
        double loadPercent = zone["load_percentage"].toDouble();
        QString status = loadPercent > 90 ? "🔴" : 
                        loadPercent > 70 ? "🟡" : "🟢";
        
        QString text = QString("%1 %2 (загрузка: %3%)")
                      .arg(status)
                      .arg(zone["name"].toString())
                      .arg(QString::number(loadPercent, 'f', 1));
        zoneCombo->addItem(text, zone["id"].toInt());
    }
}

void DistributeDialog::chemicalSelected(int index)
{
    if (index <= 0) {
        availableLabel->setText("Доступно: 0");
        return;
    }
    
    int chemicalId = chemicalCombo->currentData().toInt();
    QMap<QString, QVariant> chemical = m_db->getChemicalById(chemicalId);
    if (!chemical.isEmpty()) {
        availableLabel->setText(QString("Доступно: %1 %2")
                               .arg(QString::number(chemical["quantity"].toDouble(), 'f', 2))
                               .arg(chemical["unit"].toString()));
        quantitySpin->setMaximum(chemical["quantity"].toDouble());
        updateAvailableQuantity();
    }
}

void DistributeDialog::zoneSelected(int index)
{
    if (index <= 0) {
        zoneCapacityLabel->setText("Свободно в зоне: 0");
        return;
    }
    
    int zoneId = zoneCombo->currentData().toInt();
    double availableCapacity = m_db->getZoneAvailableCapacity(zoneId);
    zoneCapacityLabel->setText(QString("Свободно в зоне: %1").arg(availableCapacity));
    
    // Обновляем максимальное количество, которое можно распределить
    if (chemicalCombo->currentIndex() > 0) {
        int chemicalId = chemicalCombo->currentData().toInt();
        QMap<QString, QVariant> chemical = m_db->getChemicalById(chemicalId);
        if (!chemical.isEmpty()) {
            double maxQty = qMin(chemical["quantity"].toDouble(), availableCapacity);
            quantitySpin->setMaximum(maxQty);
            updateAvailableQuantity();
        }
    }
}

void DistributeDialog::updateAvailableQuantity()
{
    // Проверка доступности
    
    // Подсвечиваем поля
    QColor errorColor(255, 200, 200);
    QColor okColor(255, 255, 255);
    
    chemicalCombo->setStyleSheet(chemicalCombo->currentIndex() <= 0 ? 
        QString("QComboBox { background-color: %1; }").arg(errorColor.name()) : "");
    
    zoneCombo->setStyleSheet(zoneCombo->currentIndex() <= 0 ? 
        QString("QComboBox { background-color: %1; }").arg(errorColor.name()) : "");
    
    quantitySpin->setStyleSheet(quantitySpin->value() <= 0 ? 
        QString("QDoubleSpinBox { background-color: %1; }").arg(errorColor.name()) : "");
    
    // Проверяем превышение доступного количества
    if (chemicalCombo->currentIndex() > 0) {
        int chemicalId = chemicalCombo->currentData().toInt();
        QMap<QString, QVariant> chemical = m_db->getChemicalById(chemicalId);
        if (!chemical.isEmpty()) {
            double available = chemical["quantity"].toDouble();
            if (quantitySpin->value() > available) {
                quantitySpin->setStyleSheet("QDoubleSpinBox { background-color: #FFCCCC; color: #FF0000; }");
                availableLabel->setStyleSheet("color: #FF0000; font-weight: bold;");
            } else {
                availableLabel->setStyleSheet("color: #2E86C1; font-weight: bold;");
            }
        }
    }
    
    // Проверяем превышение доступной емкости зоны
    if (zoneCombo->currentIndex() > 0) {
        int zoneId = zoneCombo->currentData().toInt();
        double availableCapacity = m_db->getZoneAvailableCapacity(zoneId);
        if (quantitySpin->value() > availableCapacity) {
            quantitySpin->setStyleSheet("QDoubleSpinBox { background-color: #FFCCCC; color: #FF0000; }");
            zoneCapacityLabel->setStyleSheet("color: #FF0000; font-weight: bold;");
        } else {
            zoneCapacityLabel->setStyleSheet("color: #27AE60; font-weight: bold;");
        }
    }
}

void DistributeDialog::distribute()
{
    int chemicalId = chemicalCombo->currentData().toInt();
    int zoneId = zoneCombo->currentData().toInt();
    double quantity = quantitySpin->value();
    QString notes = notesEdit->toPlainText().trimmed();
    
    if (chemicalId <= 0 || zoneId <= 0 || quantity <= 0) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все обязательные поля");
        return;
    }
    
    QString error;
    if (!Database::validateDistributionData(chemicalId, zoneId, quantity, error)) {
        QMessageBox::warning(this, "Ошибка валидации", error);
        return;
    }
    
    if (m_db->distributeChemical(chemicalId, zoneId, quantity, notes)) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось распределить химикат:\n" + m_db->getLastError());
    }
}

// ============ BackupDialog ============

BackupDialog::BackupDialog(QWidget *parent, Database *db, bool manageMode)
    : QDialog(parent)
    , m_db(db)
    , m_manageMode(manageMode)
{
    setWindowTitle(m_manageMode ? "📊 Управление резервными копиями" : "🔄 Восстановление из резервной копии");
    setMinimumSize(700, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    if (!m_manageMode) {
        // Режим восстановления - показываем создание бекапа
        QGroupBox *createGroup = new QGroupBox("💾 Создать новую резервную копию");
        QVBoxLayout *createLayout = new QVBoxLayout(createGroup);
        
        QHBoxLayout *commentLayout = new QHBoxLayout();
        commentLayout->addWidget(new QLabel("Комментарий:"));
        commentEdit = new QLineEdit();
        commentEdit->setPlaceholderText("Комментарий к резервной копии (необязательно)");
        commentLayout->addWidget(commentEdit);
        
        QPushButton *createButton = new QPushButton("💾 Создать бекап");
        commentLayout->addWidget(createButton);
        
        createLayout->addLayout(commentLayout);
        mainLayout->addWidget(createGroup);
        
        connect(createButton, &QPushButton::clicked, this, &BackupDialog::createBackup);
    }
    
    // Список существующих бекапов
    QGroupBox *backupsGroup = new QGroupBox("📁 Существующие резервные копии");
    QVBoxLayout *backupsLayout = new QVBoxLayout(backupsGroup);
    
    backupsTable = new QTableWidget();
    backupsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    backupsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    backupsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    backupsTable->setAlternatingRowColors(true);
    
    QStringList headers = {"Дата создания", "Имя файла", "Размер", "Комментарий", "Статус"};
    backupsTable->setColumnCount(headers.size());
    backupsTable->setHorizontalHeaderLabels(headers);
    
    backupsTable->setColumnWidth(0, 150);
    backupsTable->setColumnWidth(1, 200);
    backupsTable->setColumnWidth(2, 100);
    backupsTable->setColumnWidth(3, 250);
    backupsTable->setColumnWidth(4, 100);
    
    backupsLayout->addWidget(backupsTable);
    
    // Кнопки управления
    QHBoxLayout *manageLayout = new QHBoxLayout();
    
    QPushButton *refreshButton = new QPushButton("🔄 Обновить");
    restoreButton = new QPushButton("🔄 Восстановить");
    deleteButton = new QPushButton("🗑️ Удалить");
    
    if (m_manageMode) {
        manageLayout->addWidget(refreshButton);
        manageLayout->addStretch();
        manageLayout->addWidget(deleteButton);
    } else {
        manageLayout->addWidget(refreshButton);
        manageLayout->addStretch();
        manageLayout->addWidget(restoreButton);
    }
    
    QPushButton *closeButton = new QPushButton("Закрыть");
    manageLayout->addWidget(closeButton);
    
    backupsLayout->addLayout(manageLayout);
    mainLayout->addWidget(backupsGroup);
    
    // Подключаем сигналы
    connect(refreshButton, &QPushButton::clicked, this, &BackupDialog::refreshBackups);
    connect(restoreButton, &QPushButton::clicked, this, &BackupDialog::restoreSelected);
    connect(deleteButton, &QPushButton::clicked, this, &BackupDialog::deleteSelected);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(backupsTable, &QTableWidget::cellClicked, this, &BackupDialog::backupSelected);
    
    // Загружаем список бекапов
    loadBackups();
}

void BackupDialog::loadBackups()
{
    QList<QMap<QString, QVariant>> backups = m_db->getAllBackups();
    backupsTable->setRowCount(backups.size());
    
    for (int i = 0; i < backups.size(); ++i) {
        const auto &backup = backups[i];
        
        // Дата создания
        backupsTable->setItem(i, 0, new QTableWidgetItem(backup["local_timestamp"].toString()));
        
        // Имя файла
        backupsTable->setItem(i, 1, new QTableWidgetItem(backup["filename"].toString()));
        
        // Размер
        double sizeMB = backup["file_size_mb"].toDouble();
        backupsTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(sizeMB, 'f', 2) + " MB"));
        
        // Комментарий
        backupsTable->setItem(i, 3, new QTableWidgetItem(backup["comment"].toString()));
        
        // Статус
        bool restored = backup["restored"].toBool();
        bool fileExists = backup["file_exists"].toBool();
        
        QString status = "";
        if (!fileExists) {
            status = "❌ Файл не найден";
        } else if (restored) {
            status = "✅ Восстановлен";
        } else {
            status = "🟢 Доступен";
        }
        
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        if (!fileExists) {
            statusItem->setBackground(Qt::red);
            statusItem->setForeground(Qt::white);
        } else if (restored) {
            statusItem->setBackground(QColor(144, 238, 144));
        }
        backupsTable->setItem(i, 4, statusItem);
    }
    
    // Отключаем кнопки, если нет выбранной строки
    backupSelected(-1);
}

void BackupDialog::refreshBackups()
{
    loadBackups();
}

void BackupDialog::createBackup()
{
    QString comment = commentEdit->text().trimmed();
    
    if (m_db->createBackup(comment)) {
        QMessageBox::information(this, "Успех", "✅ Резервная копия успешно создана");
        commentEdit->clear();
        loadBackups();
    } else {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось создать резервную копию:\n" + m_db->getLastError());
    }
}

void BackupDialog::restoreSelected()
{
    int row = backupsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, выберите резервную копию для восстановления");
        return;
    }
    
    QString filename = backupsTable->item(row, 1)->text();
    QString filePath = m_db->getBackupDir() + QDir::separator() + filename;
    
    // Проверяем существование файла
    if (!QFile::exists(filePath)) {
        QMessageBox::critical(this, "Ошибка", 
            QString("Файл резервной копии не найден:\n%1").arg(filePath));
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение восстановления",
        QString("Вы уверены, что хотите восстановить базу данных из резервной копии?\n\n"
               "Файл: %1\n"
               "Дата создания: %2\n\n"
               "⚠️  Текущая база данных будет заменена!\n"
               "Это действие нельзя отменить!")
               .arg(filename)
               .arg(backupsTable->item(row, 0)->text()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_db->restoreBackup(filePath)) {
            QMessageBox::information(this, "Успех", 
                "✅ База данных успешно восстановлена из резервной копии\n"
                "Приложение будет перезагружено для применения изменений.");
            accept();
        } else {
            QMessageBox::critical(this, "Ошибка", 
                "Не удалось восстановить базу данных:\n" + m_db->getLastError());
        }
    }
}

void BackupDialog::deleteSelected()
{
    int row = backupsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, выберите резервную копию для удаления");
        return;
    }
    
    int backupId = m_db->getAllBackups()[row]["id"].toInt();
    QString filename = backupsTable->item(row, 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления",
        QString("Вы уверены, что хотите удалить резервную копию?\n\n"
               "Файл: %1\n"
               "Дата создания: %2\n\n"
               "Это действие нельзя отменить!")
               .arg(filename)
               .arg(backupsTable->item(row, 0)->text()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_db->deleteBackup(backupId)) {
            QMessageBox::information(this, "Успех", "✅ Резервная копия успешно удалена");
            loadBackups();
        } else {
            QMessageBox::critical(this, "Ошибка", 
                "Не удалось удалить резервную копию:\n" + m_db->getLastError());
        }
    }
}

void BackupDialog::backupSelected(int row)
{
    bool enabled = row >= 0;
    
    if (m_manageMode) {
        deleteButton->setEnabled(enabled);
    } else {
        if (enabled) {
            // Проверяем, существует ли файл
            QString filename = backupsTable->item(row, 1)->text();
            QString filePath = m_db->getBackupDir() + QDir::separator() + filename;
            bool fileExists = QFile::exists(filePath);
            restoreButton->setEnabled(fileExists);
        } else {
            restoreButton->setEnabled(false);
        }
    }
}
