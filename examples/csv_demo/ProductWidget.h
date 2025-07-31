#pragma once

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

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
    
    // Модель
    QForge::nsModel::TableModel* model;
};