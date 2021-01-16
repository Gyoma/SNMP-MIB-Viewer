#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <MIBBrowserWindow.h>
#include <MIBManagmentDialog.h>
#include <ui/ui_MIBBrowserWindow.h>

MIBBrowserWindow::MIBBrowserWindow(QWidget* parent) :
    QMainWindow(parent),
    _ui(new Ui::MainMIBWindow),
    _treeModel(new TreeModel(ModuleTable::Ptr(new ModuleTable)))
{
    _ui->setupUi(this);
    _ui->treeView->setModel(_treeModel.get());
    _ui->splitter->setStretchFactor(0, 1);

    connect(_ui->actionMIBManagment, &QAction::triggered, this, &MIBBrowserWindow::execMIBManagment);

    loadSavedData();

#ifdef Q_OS_WIN
    this->setStyleSheet(
        "QHeaderView::section{"
        "border-top: 0px solid #D8D8D8;"
        "border-left: 0px solid #D8D8D8;"
        "border-right: 1px solid #D8D8D8;"
        "border-bottom: 1px solid #D8D8D8;"
        "background-color: white;"
        "padding: 4px;"
        "}");
#endif
}

MIBBrowserWindow::~MIBBrowserWindow()
{
    saveData();
    delete _ui;
}

Q_SLOT void MIBBrowserWindow::execMIBManagment()
{
    MIBManagmentDialog moduleDialog(this, _lastFolderPath, _modulesInfoTable);

    if (moduleDialog.exec())
    {
        for (auto& event : moduleDialog.getEventList())
        {
            switch (event.type)
            {
            case MIBManagmentDialog::EventType::Refresh: //была загружена другая папка или нужно перезагрузить текущую
            {
                //очищаем всю текущую информацию в дереве
                for (auto const& moduleInfo : _modulesInfoTable)
                    _treeModel->removeModule(moduleInfo.second->moduleName);

                //получаем новый список информации о модулях
                _modulesInfoTable = std::move(event.table);

                //заносим сперва всю информацию о путях до модулей и их именах
                for (auto const& moduleInfo : _modulesInfoTable)
                    _treeModel->updateModuleInfo(moduleInfo.second);

                _lastFolderPath = moduleDialog.getFolderPath();

                break;
            }
            case MIBManagmentDialog::EventType::Load: //модуль нужно загрузить
            {
                auto const& moduleName = event.table.begin()->first;
                _modulesInfoTable[moduleName]->needToLoad = true;
                _treeModel->linkupModule(moduleName);

                break;
            }
            case MIBManagmentDialog::EventType::Unload: //модуль нужно отсоединить
            {
                auto const& moduleName = event.table.begin()->first;
                _modulesInfoTable[moduleName]->needToLoad = false;
                _treeModel->linkupModule(moduleName);

                break;
            }
            default:
                //ничего не произошло
                break;
            }
        }
    }
}

Q_SLOT void MIBBrowserWindow::showRelevantNodeInfo()
{
    return Q_SLOT void();
}

void MIBBrowserWindow::loadSavedData()
{
    QFile loadFile("ModulesInfo.json");

    if (!loadFile.open(QIODevice::ReadOnly))
        return;

    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));
    QJsonObject json(loadDoc.object());

    if (json.contains("LastMIBFolderPath") && json["LastMIBFolderPath"].isString())
        _lastFolderPath = json["LastMIBFolderPath"].toString();

    if (json.contains("ModulesInfo") && json["ModulesInfo"].isArray())
    {
        QJsonArray modulesInfoList = json["ModulesInfo"].toArray();
        //_modulesInfoList.reserve(modulesInfoList.size());
        Strs modulesToLoad;

        for (int i = 0; i < modulesInfoList.size(); ++i)
        {
            QJsonObject modulesInfoItem = modulesInfoList[i].toObject();
            std::string ModuleName, ModulePath;
            Strs ModuleImports;
            bool NeedToLoad = false;


            if (modulesInfoItem.contains("ModuleName") && modulesInfoItem["ModuleName"].isString())
                ModuleName = modulesInfoItem["ModuleName"].toString().toStdString();
            else
                continue;

            if (modulesInfoItem.contains("ModulePath") && modulesInfoItem["ModulePath"].isString())
                ModulePath = modulesInfoItem["ModulePath"].toString().toStdString();
            else
                continue;

            if (modulesInfoItem.contains("NeedToLoad") && modulesInfoItem["NeedToLoad"].isBool())
                NeedToLoad = modulesInfoItem["NeedToLoad"].toBool();
            else
                continue;

            if (modulesInfoItem.contains("ModuleImports") && modulesInfoItem["ModuleImports"].isArray())
                for (auto const& import : modulesInfoItem["ModuleImports"].toArray())
                    ModuleImports.push_back(import.toString().toStdString());
            else
                continue;

            ModuleInfo::Ptr moduleInfo(new ModuleInfo(ModuleName, ModulePath, NeedToLoad, ModuleImports));
            _modulesInfoTable[ModuleName] = moduleInfo;

            _treeModel->updateModuleInfo(moduleInfo);

            if (moduleInfo->needToLoad)
                modulesToLoad.push_back(ModuleName);
        }

        for (size_t i = 0; i < modulesToLoad.size(); ++i)
            _treeModel->loadModule(modulesToLoad[i]);
    }
}

void MIBBrowserWindow::saveData()
{
    QFile saveFile("ModulesInfo.json");

    if (!saveFile.open(QIODevice::WriteOnly))
        return;

    QJsonObject saveData;
    QJsonArray modulesInfoList;
    QJsonObject modulesInfoItem;

    saveData["LastMIBFolderPath"] = _lastFolderPath;

    for (auto& moduleInfoPair : _modulesInfoTable)
    {
        auto& moduleInfo = moduleInfoPair.second;
        QJsonArray moduleImports;

        modulesInfoItem["ModuleName"] = QString::fromStdString(moduleInfo->moduleName);
        modulesInfoItem["ModulePath"] = QString::fromStdString(moduleInfo->modulePath);
        modulesInfoItem["NeedToLoad"] = moduleInfo->needToLoad;

        for (auto const& import : moduleInfo->moduleImports)
            moduleImports.append(QString::fromStdString(import));

        modulesInfoItem["ModuleImports"] = moduleImports;

        modulesInfoList.append(modulesInfoItem);
    }

    saveData["ModulesInfo"] = modulesInfoList;

    saveFile.write(QJsonDocument(saveData).toJson());
}
