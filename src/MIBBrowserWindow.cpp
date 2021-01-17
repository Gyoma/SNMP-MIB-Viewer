#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QItemSelection>
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
    _ui->propertiesScrollArea->setVisible(false);
    _ui->splitter->setStretchFactor(0, 1);
    _ui->splitter->setCollapsible(0, false);
    _ui->splitter->setCollapsible(1, false);

    connect(_ui->actionMIBManagment, &QAction::triggered, this, &MIBBrowserWindow::execMIBManagment);
    connect(_ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MIBBrowserWindow::showRelevantNodeInfo);
    connect(_ui->splitter, &QSplitter::splitterMoved, this, &MIBBrowserWindow::shrinkViewToFit);

    loadSavedData();

#ifdef Q_OS_WIN
    this->setStyleSheet(
        "QHeaderView::section {"
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
        for (auto& event : moduleDialog.getMIBEventList())
        {
            switch (event.type)
            {
            case MIBManagmentDialog::MIBEventType::Refresh: //была загружена другая папка или нужно перезагрузить текущую
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
            case MIBManagmentDialog::MIBEventType::Load: //модуль нужно загрузить
            {
                auto const& moduleName = event.table.begin()->first;
                _modulesInfoTable[moduleName]->needToLoad = true;
                _treeModel->linkupModule(moduleName);

                break;
            }
            case MIBManagmentDialog::MIBEventType::Unload: //модуль нужно отсоединить
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

Q_SLOT void MIBBrowserWindow::showRelevantNodeInfo(const QItemSelection& selected)
{
    auto index = selected.indexes().first();

    if (!index.isValid())
    {
        _ui->propertiesScrollArea->setVisible(false);
        return;
    }

    _ui->propertiesScrollArea->setVisible(true);
    Node* node = static_cast<Node*>(index.internalPointer());

    if (!node->label.empty())
    {
        _ui->nodeLabelWidget->setVisible(true);
        _ui->nodeLabelLine->setText(QString::fromStdString(node->label));
    }
    else
        _ui->nodeLabelWidget->setVisible(false);

    //_ui->subIDWidget->setVisible(true);
    _ui->subIDLine->setText(QString::number(node->subid));

    //_ui->oidWidget-
    _ui->OIDLine->setText(QString::fromStdString(node->OID));
    _ui->labelOIDLine->setText(QString::fromStdString(node->labelOID));

    if (!node->modules.empty())
    {
        _ui->modulesWidget->setVisible(true);

        QString modulesList;
        for (auto const& moduleName : node->modules)
            modulesList += QString::fromStdString(moduleName) + "\n";

        modulesList.chop(1);
        _ui->modulesText->setText(modulesList);
    }
    else
        _ui->modulesWidget->setVisible(false);


    if (node->type != LT::eNA)
    {
        _ui->typeWidget->setVisible(true);
        _ui->typeLine->setText(QString::fromStdString(Parser::typeToStr(node->type)));
    }
    else
        _ui->typeWidget->setVisible(false);


    if (node->access != LT::eNA)
    {
        _ui->accessWidget->setVisible(true);
        _ui->accessLine->setText(QString::fromStdString(Parser::typeToStr(node->access)));
    }
    else
        _ui->accessWidget->setVisible(false);


    if (node->status != LT::eNA)
    {
        _ui->statusWidget->setVisible(true);
        _ui->statusLine->setText(QString::fromStdString(Parser::typeToStr(node->status)));
    }
    else
        _ui->statusWidget->setVisible(false);


    if (!node->augments.empty())
    {
        _ui->augmentsWidget->setVisible(true);
        _ui->augmentsLine->setText(QString::fromStdString(node->augments));
    }
    else
        _ui->augmentsWidget->setVisible(false);


    if (!node->reference.empty())
    {
        _ui->referenceWidget->setVisible(true);
        _ui->referenceLine->setText(QString::fromStdString(node->reference));
    }
    else
        _ui->referenceWidget->setVisible(false);


    if (!node->defaultValue.empty())
    {
        _ui->defValueWidget->setVisible(true);
        _ui->defValueLine->setText(QString::fromStdString(node->defaultValue));
    }
    else
        _ui->defValueWidget->setVisible(false);

    if (!node->enums.empty())
    {
        _ui->enumsWidget->setVisible(true);

        QString Enums;
        for (auto const& Enum : node->enums)
            Enums += QString::fromStdString(Enum.label) + " = " + QString::number(Enum.value) + '\n';

        Enums.chop(1);
        _ui->enumsText->setText(Enums);
    }
    else
        _ui->enumsWidget->setVisible(false);


    if (!node->ranges.empty())
    {
        _ui->rangeWidget->setVisible(true);

        auto const& ranges = node->ranges;

        QString strRanges;

        for (auto const& Range : ranges)
        {
            if (Range.low != Range.high)
                strRanges += QString::number(Range.low) + "..." + QString::number(Range.high) + ", ";
            else
                strRanges += QString::number(Range.low) + ", ";
        }

        strRanges.chop(2);

        _ui->rangeLine->setText(strRanges);
    }
    else
        _ui->rangeWidget->setVisible(false);

    //_ui->varbindsWidget->setVisible(false);
    if (!node->varbinds.empty())
    {
        _ui->varbindsWidget->setVisible(true);
    }
    else
        _ui->varbindsWidget->setVisible(false);

    if (!node->description.empty())
    {
        _ui->descriptionWidget->setVisible(true);
        _ui->descriptionText->setPlainText(QString::fromStdString(node->description));
    }
    else
        _ui->descriptionWidget->setVisible(false);

    shrinkViewToFit();
}

Q_SLOT void MIBBrowserWindow::shrinkViewToFit()
{
    if (_ui->modulesWidget->isVisible())
    {
        auto modulesText = _ui->modulesText;
        QMargins margins = modulesText->contentsMargins();
        int height = modulesText->document()->size().height() + margins.top() + margins.bottom();

        modulesText->setFixedHeight(height);
    }

    if (_ui->enumsWidget->isVisible())
    {
        auto enumsText = _ui->enumsText;
        QMargins margins = enumsText->contentsMargins();
        int height = enumsText->document()->size().height() + margins.top() + margins.bottom();

        enumsText->setFixedHeight(height);
    }

    if (_ui->varbindsWidget->isVisible())
    {
        auto varbindsText = _ui->enumsText;
        QMargins margins = varbindsText->contentsMargins();
        int height = varbindsText->document()->size().height() + margins.top() + margins.bottom();

        varbindsText->setFixedHeight(height);
    }

    if (_ui->descriptionWidget->isVisible())
    {
        auto descriptionText = _ui->descriptionText;
        QMargins margins = descriptionText->contentsMargins();
        int height = descriptionText->document()->size().height() + margins.top() + margins.bottom();

        descriptionText->setFixedHeight(height);
    }
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

        for (size_t i = 0, size = modulesInfoList.size(); i < size; ++i)
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

        for (size_t i = 0, size = modulesToLoad.size(); i < size; ++i)
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
