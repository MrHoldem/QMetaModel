#pragma once

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QUuid>
#include <QProgressBar>
#include <QTimer>

// Forward declaration
namespace QForge {
namespace nsModel {
    class TableModel;
}
}

class ProductWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ProductWidget(QWidget *parent = nullptr);
    ~ProductWidget();
    
private slots:
    void onLoadAllClicked();
    void onFilterInStockClicked();
    void onCategoryFilterChanged(const QString& category);
    void onRefreshClicked();
    
    // Асинхронные слоты
public slots:
    void onQueryStarted(const QUuid& queryId);
    void onQueryFinished(const QUuid& queryId);
    void onQueryFailed(const QUuid& queryId, const QString& error);
    
private:
    void setupUi();
    void connectSignals();
    
    // UI элементы
    QTableView* tableView;
    QPushButton* loadAllButton;
    QPushButton* filterInStockButton;
    QPushButton* refreshButton;
    QComboBox* categoryCombo;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    QTimer* progressTimer;
    
    // Модель
    QForge::nsModel::TableModel* model;
};