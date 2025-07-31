#include "ProductWidget.h"
#include "CsvQueryHandler.h"
#include "../../src/TableModel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <memory>

ProductWidget::ProductWidget(QWidget *parent)
    : QWidget(parent)
    , model(nullptr)
{
    setupUi();
    connectSignals();
    
    // Создаем обработчик CSV и модель
    // Ищем файлы от корня проекта QMetaModel
    QString projectRoot;
    
    // Поднимаемся по каталогам до тех пор, пока не найдем QMetaModel.pro
    QDir searchDir(QDir::currentPath());
    while (!searchDir.exists("QMetaModel.pro") && searchDir.cdUp()) {
        // Продолжаем поиск
    }
    
    if (searchDir.exists("QMetaModel.pro")) {
        projectRoot = searchDir.absolutePath();
    } else {
        // Fallback - используем относительный путь
        projectRoot = "../../..";
    }
    
    QString csvPath = projectRoot + "/examples/csv_demo/products.csv";
    QString schemaPath = projectRoot + "/examples/csv_demo/ProductModel.yml";
    
    qDebug() << "CSV path:" << csvPath;
    qDebug() << "Schema path:" << schemaPath;
    
    // Проверяем существование файлов
    if (!QFile::exists(csvPath)) {
        QMessageBox::critical(this, "Ошибка", "CSV файл не найден: " + csvPath);
        return;
    }
    
    if (!QFile::exists(schemaPath)) {
        QMessageBox::critical(this, "Ошибка", "Файл схемы не найден: " + schemaPath);
        return;
    }
    
    qDebug() << "Files exist, creating handler...";
    
    auto csvHandler = std::make_shared<QForge::nsModel::CsvQueryHandler>(csvPath);
    
    // Создаем функцию-обертку для обработчика
    QForge::nsModel::QueryHandler handler = [csvHandler](const QForge::nsModel::QueryContext& ctx) {
        return (*csvHandler)(ctx);
    };
    
    qDebug() << "Creating TableModel...";
    
    model = new QForge::nsModel::TableModel(schemaPath, handler, this);
    
    qDebug() << "TableModel created, checking validity...";
    
    if (!model->isValid()) {
        QString error = model->getLastError();
        qDebug() << "Model is not valid. Error:" << error;
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить модель: " + error);
        return;
    }
    
    qDebug() << "Model is valid!";
    
    tableView->setModel(model);
    
    // Подключаем сигналы асинхронного выполнения (старый синтаксис для совместимости)
    connect(model, SIGNAL(executionStarted(QUuid)), this, SLOT(onQueryStarted(QUuid)));
    connect(model, SIGNAL(executionFinished(QUuid)), this, SLOT(onQueryFinished(QUuid)));
    connect(model, SIGNAL(executionFailed(QUuid,QString)), this, SLOT(onQueryFailed(QUuid,QString)));
    
    // Настраиваем внешний вид таблицы
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setStretchLastSection(true);
    
    // Скрываем ID колонку (индекс 0)
    tableView->hideColumn(0);
    
    // Загружаем данные при старте (синхронно, чтобы не блокировать UI)
    auto result = model->execute("load_all");
    if (result.ok) {
        statusLabel->setText(QString("Загружено записей: %1").arg(result.rows.size()));
    } else {
        statusLabel->setText("Ошибка загрузки: " + result.errors_log.join(", "));
    }
}

ProductWidget::~ProductWidget()
{
    // model удалится автоматически как child объект
}

void ProductWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Панель управления
    auto* controlPanel = new QHBoxLayout();
    
    loadAllButton = new QPushButton("Загрузить все", this);
    filterInStockButton = new QPushButton("Только в наличии", this);
    refreshButton = new QPushButton("Обновить", this);
    
    controlPanel->addWidget(loadAllButton);
    controlPanel->addWidget(filterInStockButton);
    
    controlPanel->addWidget(new QLabel("Категория:", this));
    categoryCombo = new QComboBox(this);
    categoryCombo->addItems({"Все", "Electronics", "Appliances", "Furniture", "Other"});
    controlPanel->addWidget(categoryCombo);
    
    controlPanel->addWidget(refreshButton);
    controlPanel->addStretch();
    
    mainLayout->addLayout(controlPanel);
    
    // Таблица
    tableView = new QTableView(this);
    mainLayout->addWidget(tableView);
    
    // Статусная строка
    statusLabel = new QLabel("Готов к работе", this);
    mainLayout->addWidget(statusLabel);
    
    // Прогресс бар (скрыт по умолчанию)
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0); // Неопределенный прогресс
    progressBar->setFormat("Выполняется запрос... %p%");
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setMinimumHeight(30);
    progressBar->hide();
    mainLayout->addWidget(progressBar);
    
    // Таймер для анимации прогресса
    progressTimer = new QTimer(this);
    
    setWindowTitle("Демо: Продукты из CSV");
    resize(900, 600);
}

void ProductWidget::connectSignals()
{
    connect(loadAllButton, &QPushButton::clicked, this, &ProductWidget::onLoadAllClicked);
    connect(filterInStockButton, &QPushButton::clicked, this, &ProductWidget::onFilterInStockClicked);
    connect(refreshButton, &QPushButton::clicked, this, &ProductWidget::onRefreshClicked);
    connect(categoryCombo, &QComboBox::currentTextChanged, this, &ProductWidget::onCategoryFilterChanged);
}

void ProductWidget::onLoadAllClicked()
{
    statusLabel->setText("Загрузка всех данных...");
    
    // Полная блокировка интерфейса
    setEnabled(false);
    progressBar->setFormat("Загрузка всех данных из CSV...");
    progressBar->show();
    
    model->executeAsync("load_all");
}

void ProductWidget::onFilterInStockClicked()
{
    statusLabel->setText("Фильтрация по наличию...");
    
    // Полная блокировка интерфейса
    setEnabled(false);
    progressBar->setFormat("Поиск товаров в наличии...");
    progressBar->show();
    
    model->executeAsync("filter_in_stock");
}

void ProductWidget::onCategoryFilterChanged(const QString& category)
{
    if (category == "Все") {
        onLoadAllClicked();
    } else {
        statusLabel->setText(QString("Фильтрация по категории '%1'...").arg(category));
        
        // Полная блокировка интерфейса
        setEnabled(false);
        progressBar->setFormat(QString("Поиск товаров в категории '%1'...").arg(category));
        progressBar->show();
        
        QVariantMap params;
        params["category"] = category;
        
        model->executeAsync("filter_category", params);
    }
}

void ProductWidget::onRefreshClicked()
{
    // Перезагружаем CSV файл и пересоздаем модель для обновления схемы
    if (model) {
        model->deleteLater();
        model = nullptr;
    }
    
    // Создаем заново как в конструкторе
    QString projectRoot;
    QDir searchDir(QDir::currentPath());
    while (!searchDir.exists("QMetaModel.pro") && searchDir.cdUp()) {}
    
    if (searchDir.exists("QMetaModel.pro")) {
        projectRoot = searchDir.absolutePath();
    } else {
        projectRoot = "../../..";
    }
    
    QString csvPath = projectRoot + "/examples/csv_demo/products.csv";
    QString schemaPath = projectRoot + "/examples/csv_demo/ProductModel.yml";
    
    auto csvHandler = std::make_shared<QForge::nsModel::CsvQueryHandler>(csvPath);
    QForge::nsModel::QueryHandler handler = [csvHandler](const QForge::nsModel::QueryContext& ctx) {
        return (*csvHandler)(ctx);
    };
    
    model = new QForge::nsModel::TableModel(schemaPath, handler, this);
    
    if (model->isValid()) {
        tableView->setModel(model);
        
        // Подключаем сигналы асинхронного выполнения (старый синтаксис для совместимости)
        connect(model, SIGNAL(executionStarted(QUuid)), this, SLOT(onQueryStarted(QUuid)));
        connect(model, SIGNAL(executionFinished(QUuid)), this, SLOT(onQueryFinished(QUuid)));
        connect(model, SIGNAL(executionFailed(QUuid,QString)), this, SLOT(onQueryFailed(QUuid,QString)));
        
        // Скрываем ID колонку (индекс 0)
        tableView->hideColumn(0);
        
        // Загружаем данные синхронно при обновлении
        auto result = model->execute("load_all");
        if (result.ok) {
            statusLabel->setText(QString("Данные и схема обновлены. Записей: %1").arg(result.rows.size()));
        } else {
            statusLabel->setText("Ошибка загрузки после обновления: " + result.errors_log.join(", "));
        }
    } else {
        statusLabel->setText("Ошибка обновления: " + model->getLastError());
    }
}

void ProductWidget::onQueryStarted(const QUuid& queryId)
{
    qDebug() << "Query started:" << queryId;
    // Индикатор загрузки уже установлен в методах выше
}

void ProductWidget::onQueryFinished(const QUuid& queryId)
{
    qDebug() << "Query finished:" << queryId;
    
    // Разблокируем интерфейс
    setEnabled(true);
    progressBar->hide();
    
    // Обновляем статус с количеством строк
    int rowCount = model->rowCount();
    statusLabel->setText(QString("Загружено записей: %1").arg(rowCount));
}

void ProductWidget::onQueryFailed(const QUuid& queryId, const QString& error)
{
    qDebug() << "Query failed:" << queryId << "Error:" << error;
    
    // Разблокируем интерфейс
    setEnabled(true);
    progressBar->hide();
    
    statusLabel->setText("Ошибка запроса: " + error);
}