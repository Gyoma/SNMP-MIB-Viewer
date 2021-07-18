#pragma once
#include <QMainWindow>
#include "TreeModel.h"
#include "Parser.h"

namespace Ui
{
    class MainMIBWindow;
}

class ModuleManagmentDialog;

class MIBViewerWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MIBViewerWindow(QWidget* parent = nullptr);
    ~MIBViewerWindow();

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