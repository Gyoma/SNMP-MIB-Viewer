#pragma once
#include <QMainWindow>
#include <TreeModel.h>
#include <Parser.h>

namespace Ui
{
    class MainMIBWindow;
}

class ModuleManagmentDialog;

class MIBBrowserWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MIBBrowserWindow(QWidget* parent = nullptr);
    ~MIBBrowserWindow();

private:

    Q_SLOT void execMIBManagment();
    Q_SLOT void showRelevantNodeInfo(const class QModelIndex& index);
    Q_SLOT void shrinkViewToFit();

    void loadSavedData();
    void saveData();

    Ui::MainMIBWindow*				_ui;
    TreeModel::Ptr					_treeModel;
    ModuleMetaDataTable::Ptr        _modulesDataTable;
};