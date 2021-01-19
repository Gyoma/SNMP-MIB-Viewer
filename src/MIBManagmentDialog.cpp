#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <MIBManagmentDialog.h>
#include <Parser.h>
#include <fstream>
#include <ui/ui_MIBManagmentDialog.h>

MIBManagmentDialog::MIBManagmentDialog(QWidget * parent, const ModuleMetaDataTable& ModulesInfoTable) :
    QDialog(parent, Qt::WindowCloseButtonHint),
    _ui(new Ui::MIBManagmentWindow),
    _modulesInfoTable(ModulesInfoTable)
{
    _ui->setupUi(this);
    _ui->loadModuleButton->setEnabled(false);
    _ui->unloadModuleButton->setEnabled(false);
    this->setWindowModality(Qt::WindowModal);
    this->loadSavedData();

    for (auto& moduleInfoPair : _modulesInfoTable)
    {
        auto& moduleInfo = moduleInfoPair.second;

        if (moduleInfo->needToLoad)
            _ui->loadedModulesList->addItem(QString::fromStdString(moduleInfo->moduleName));
        else
            _ui->availableModulesList->addItem(QString::fromStdString(moduleInfo->moduleName));
    }

    connect(_ui->selectFolderButton, &QPushButton::clicked, this, &MIBManagmentDialog::selectFolder);
    connect(_ui->loadModuleButton, &QPushButton::clicked, this, &MIBManagmentDialog::loadModule);
    connect(_ui->unloadModuleButton, &QPushButton::clicked, this, &MIBManagmentDialog::unloadModule);
    connect(_ui->okButton, &QPushButton::clicked, this, &MIBManagmentDialog::doneСonfigure);
    connect(_ui->cancelButton, &QPushButton::clicked, this, &MIBManagmentDialog::cancelСonfigure);

    connect(_ui->availableModulesList, &QListWidget::itemSelectionChanged, [this]()
    {
        auto selectedItems = _ui->availableModulesList->selectedItems();

        if (selectedItems.isEmpty())
            _ui->loadModuleButton->setEnabled(false);
        else
            _ui->loadModuleButton->setEnabled(true);
    });

    connect(_ui->loadedModulesList, &QListWidget::itemSelectionChanged, [this]()
    {
        auto selectedItems = _ui->loadedModulesList->selectedItems();

        if (selectedItems.isEmpty())
            _ui->unloadModuleButton->setEnabled(false);
        else
            _ui->unloadModuleButton->setEnabled(true);
    });
}

MIBManagmentDialog::~MIBManagmentDialog()
{
    delete _ui;
}

MIBManagmentDialog::MIBEventList MIBManagmentDialog::getMIBEventList()
{
    return _eventList;
}

Q_SLOT void MIBManagmentDialog::selectFolder()
{
    QFileDialog fileDialog(this, QString::fromUtf8("Выбор папки"));
    fileDialog.setDirectory(_lastFolderPath.isEmpty() ? QDir::currentPath() : _lastFolderPath);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    fileDialog.setLabelText(QFileDialog::DialogLabel::Accept, QString::fromUtf8("Выбрать"));
    fileDialog.setLabelText(QFileDialog::DialogLabel::Reject, QString::fromUtf8("Отмена"));

    if (fileDialog.exec())
    {
        auto newOpenedFolder = fileDialog.selectedFiles().first();

        if (_lastFolderPath == newOpenedFolder)
        {
            QMessageBox messageBox(QMessageBox::Question,
                QString::fromUtf8("Повторный выбор папки"),
                QString::fromUtf8("Данная папка уже была ранее загружена. Вы хотите перезагрузить ее ?"),
                QMessageBox::Yes | QMessageBox::No,
                this);

            messageBox.setButtonText(QMessageBox::Yes, QString::fromUtf8("Да"));
            messageBox.setButtonText(QMessageBox::No, QString::fromUtf8("Нет"));
            messageBox.setEscapeButton(QMessageBox::No);

            auto reply = messageBox.exec();

            if (reply == QMessageBox::No)
                return;
        }

        if (newOpenedFolder.isEmpty())
            return;

        _lastFolderPath = newOpenedFolder;

        //_modulesLoadInfoList.clear();
        _ui->availableModulesList->clear();
        _ui->loadedModulesList->clear();
        _ui->loadModuleButton->setEnabled(false);
        _ui->unloadModuleButton->setEnabled(false);
        _modulesInfoTable.clear();
        //_eventList.push_back()
        //_changeType = MIBEventType::Refresh;

        Parser parser;
        Token token;
        QDir dir(_lastFolderPath);
        dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

        //ModuleMetaDataTable newModuleInfoTable;     
        for (auto const& fileInfo : dir.entryInfoList())
        {
            std::string modulePath = fileInfo.absoluteFilePath().toStdString();
            std::ifstream file(modulePath);

            if (file.is_open())
            {
                parser.parseToken(file, token);
                std::string moduleName = token.lexem;
                parser.parseToken(file, token);

                if (token.type == LT::eDEFINITIONS)
                {
                    if (_ui->availableModulesList->findItems(QString::fromStdString(moduleName), Qt::MatchExactly).isEmpty())
                    {
                        _ui->availableModulesList->addItem(QString::fromStdString(moduleName));

                        //Список импортируемых модулей
                        Strs importedModules;
                        //Список импортируемых cущностей
                        Strs importedLabels;

                        //Получаем список импортируемых модулей
                        while (token.type != LT::eSEMI && token.type != LT::eENDOFFILE)
                        {
                            //Если импортируема сущность не является встроенной
                            if (token.type == LT::eLABEL)
                            {
                                importedLabels.push_back(token.lexem);
                            }
                            else if (token.type == LT::eFROM)
                            {
                                parser.parseToken(file, token);

                                //Если имеет смысл импортировать этот модуль
                                if (token.type == LT::eLABEL && !importedLabels.empty())
                                    importedModules.push_back(token.lexem);

                                importedLabels.clear();
                            }

                            parser.parseToken(file, token);
                        }

                        _modulesInfoTable[moduleName] = ModuleMetaData::Ptr(new ModuleMetaData(moduleName, modulePath, false, importedModules));
                    }
                }
            }
        }

        // Резервируем место для данных обновления, а не перемещаем таблицу,
        // чтобы не иметь две копии одной таблицы.
        // В конце работы переместим туда таблицу.
        updateOrMoveToHistory(MIBEventType::Refresh, {});
    }
}

Q_SLOT void MIBManagmentDialog::loadModule()
{
    auto moduleiItemsToLoad = _ui->availableModulesList->selectedItems();

    while (!moduleiItemsToLoad.isEmpty())
    {
        addAllModules(moduleiItemsToLoad.first()->text().toStdString());
        moduleiItemsToLoad = _ui->availableModulesList->selectedItems();
    }
}

Q_SLOT void MIBManagmentDialog::unloadModule()
{
    auto moduleiItemsToUnload = _ui->loadedModulesList->selectedItems();

    while (!moduleiItemsToUnload.isEmpty())
    {
        removeAllModules(moduleiItemsToUnload.first()->text().toStdString());
        moduleiItemsToUnload = _ui->loadedModulesList->selectedItems();
    }
}

Q_SLOT void MIBManagmentDialog::cancelСonfigure()
{
    this->reject();
}

Q_SLOT void MIBManagmentDialog::doneСonfigure()
{
    if (!_eventList.empty())
    {
        auto& firstEvent = _eventList.front();

        // Если место зарезервировано для обновления
        if (firstEvent.type == MIBEventType::Refresh)
            firstEvent.table = std::move(_modulesInfoTable);
    }

    this->saveData();
    this->accept();
}

void MIBManagmentDialog::updateOrMoveToHistory(MIBEventType Type, ModuleMetaDataTable&& Data)
{
    if (Type == MIBEventType::Refresh)
    {
        _eventList.clear();
    }
    else
    {
        auto const& moduleName = Data.begin()->first;

        for (auto it = _eventList.begin(); it != _eventList.end(); ++it)
            if (it->type != MIBEventType::Refresh && it->table.begin()->first == moduleName)
            {
                // Если противоположные операции, то делать ничего не нужно
                if ((it->type == MIBEventType::Load && Type == MIBEventType::Unload) ||
                    (it->type == MIBEventType::Unload && Type == MIBEventType::Load))
                    _eventList.erase(it);

                return;
            }
    }

    // std::move здесь необходимо
    _eventList.emplace_back(Type, std::move(Data));
}

void MIBManagmentDialog::addAllModules(const std::string& RootModuleName)
{
    //если такого модуля нет в доступных
    if (_modulesInfoTable.find(RootModuleName) == _modulesInfoTable.end())
        return;

    QString qRootName = QString::fromStdString(RootModuleName);
    std::list<std::string> checklist = { RootModuleName };

    for (auto const& ModuleName : checklist)
    {
        auto const& moduleImports = _modulesInfoTable.find(ModuleName)->second->moduleImports;

        for (size_t i = 0, size = moduleImports.size(); i < size; ++i)
        {
            auto importedModule = moduleImports[i];
            auto res = _modulesInfoTable.find(importedModule);

            if (res == _modulesInfoTable.end())
            {
                auto replacementModule = Parser::getReplacementModule(importedModule);
                res = _modulesInfoTable.find(replacementModule);

                if (res == _modulesInfoTable.end())
                {
                    QString message = QString::fromUtf8("Ошибка при загрузке ") +
                        qRootName +
                        QString::fromUtf8("\n\nОтсутсвует импортируемый модуль ") +
                        QString::fromStdString(importedModule) +
                        QString::fromUtf8("\nМодуль ") +
                        qRootName +
                        QString::fromUtf8(" и импортируемые им модули загружены не будут");

                    QMessageBox::critical(this,
                        QString::fromUtf8("Отсутствующий модуль"),
                        message);

                    auto item = _ui->availableModulesList->findItems(qRootName, Qt::MatchExactly).first();
                    _ui->availableModulesList->setItemSelected(item, false);

                    return;
                }

                importedModule = replacementModule;
            }

            if (std::find(checklist.begin(), checklist.end(), importedModule) == checklist.end())
                checklist.push_back(importedModule);
        }
    }

    for (auto const& moduleName : checklist)
    {
        QString ModuleName = QString::fromStdString(moduleName);
        // Если модуль еще не загружен
        if (_ui->loadedModulesList->findItems(ModuleName, Qt::MatchExactly).isEmpty())
        {

            // Добавляем в историю модуль, который нужно загрузить
            updateOrMoveToHistory(MIBEventType::Load, ModuleMetaDataTable{ { moduleName, nullptr } });

            // Отображаем модуль в списке загруженных
            _ui->loadedModulesList->addItem(ModuleName);
            // Убираем модуль из списка доступных для загрузки
            delete _ui->availableModulesList->findItems(ModuleName, Qt::MatchExactly).first();
        }
    }
}

Strs MIBManagmentDialog::forWhichModulesIsRoot(const std::string & RootModuleName)
{
    Strs res = { RootModuleName };

    for (size_t i = 0; i < res.size(); ++i)
    {
        auto RootName = res[i];

        for (auto modulePair : _modulesInfoTable)
        {
            auto const& moduleName = modulePair.first;
            auto const& moduleImports = modulePair.second->moduleImports;

            for (auto const& importedModule : moduleImports)
            {
                if (importedModule == RootName)
                {
                    if (!_ui->loadedModulesList->findItems(QString::fromStdString(moduleName), Qt::MatchExactly).isEmpty())
                    {
                        if (std::find(res.begin(), res.end(), moduleName) == res.end())
                        {
                            res.push_back(moduleName);
                        }
                    }

                    break;
                }
            }
        }
    }

    res.erase(res.begin());
    return res;
}

void MIBManagmentDialog::removeAllModules(const std::string & RootModuleName)
{
    auto modules = forWhichModulesIsRoot(RootModuleName);

    if (!modules.empty() && !_ui->warningBox->isChecked())
    {
        QString message = QString::fromUtf8("Кроме ") +
            QString::fromStdString(RootModuleName) +
            QString::fromUtf8(" будут также удалены:\n\n");

        //for(auto const& childModule: modules)
        //    message += QString::fromStdString(childModule) + "\n";
        for (size_t i = 0, size = modules.size() - 1; i < size; ++i)
            message += QString::fromStdString(modules[i]) + ", ";
        message += QString::fromStdString(modules.back());

        message += QString::fromUtf8("\n\nХотите удалить ?");

        QMessageBox messageBox(QMessageBox::Question,
            QString::fromUtf8("Множественное удаление"),
            message,
            QMessageBox::Yes | QMessageBox::No,
            this);

        messageBox.setButtonText(QMessageBox::Yes, QString::fromUtf8("Да"));
        messageBox.setButtonText(QMessageBox::No, QString::fromUtf8("Нет"));

        auto reply = messageBox.exec();

        if (reply == QMessageBox::No)
        {
            modules.push_back(RootModuleName);
            for (size_t i = 0, size = modules.size(); i < size; ++i)
            {


                auto deselectedItem = _ui->loadedModulesList->findItems(QString::fromStdString(modules[i]), Qt::MatchExactly).first();
                _ui->loadedModulesList->setItemSelected(deselectedItem, false);
            }

            return;
        }
    }

    modules.push_back(RootModuleName);

    for (auto const& moduleToRemove : modules)
    {
        updateOrMoveToHistory(MIBEventType::Unload, ModuleMetaDataTable{ {moduleToRemove, nullptr} });

        QString ModuleName = QString::fromStdString(moduleToRemove);
        _ui->availableModulesList->addItem(ModuleName);
        delete _ui->loadedModulesList->findItems(ModuleName, Qt::MatchExactly).first();
    }
}

void MIBManagmentDialog::saveData()
{
    QDir dir;

    if (!dir.exists("data"))
        dir.mkpath("data");

    QFile saveFile("data/mibmanagment.json");

    if (!saveFile.open(QIODevice::WriteOnly))
        return;

    QJsonObject MIBDialogData;

    MIBDialogData["LastFolderPath"] = _lastFolderPath;
    MIBDialogData["DisableWarnings"] = _ui->warningBox->isChecked();

    saveFile.write(QJsonDocument(MIBDialogData).toJson());
}

void MIBManagmentDialog::loadSavedData()
{
    QFile loadFile("data/mibmanagment.json");

    if (!loadFile.open(QIODevice::ReadOnly))
        return;

    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));
    QJsonObject MIBDialogData(loadDoc.object());

    if (MIBDialogData.contains("LastFolderPath") && MIBDialogData["LastFolderPath"].isString())
        _lastFolderPath = MIBDialogData["LastFolderPath"].toString();

    if (MIBDialogData.contains("DisableWarnings") && MIBDialogData["DisableWarnings"].isBool())
        _ui->warningBox->setChecked(MIBDialogData["DisableWarnings"].toBool());
}

MIBManagmentDialog::MIBEvent::MIBEvent(MIBEventType Type, const ModuleMetaDataTable & Table) :
    type(Type),
    table(Table)
{}

MIBManagmentDialog::MIBEvent::MIBEvent(MIBEventType Type, ModuleMetaDataTable && Table) :
    type(Type),
    table(std::move(Table))
{}

MIBManagmentDialog::MIBEvent::MIBEvent(const MIBEvent & other) :
    MIBEvent(other.type, other.table)
{}

MIBManagmentDialog::MIBEvent::MIBEvent(MIBEvent && other) noexcept :
    type(other.type),
    table(std::move(other.table))
{
    other.type = MIBEventType::None;
}

MIBManagmentDialog::MIBEvent & MIBManagmentDialog::MIBEvent::operator=(const MIBEvent & other)
{
    if (this != &other)
    {
        type = other.type;
        table = other.table;
    }

    return *this;
}

MIBManagmentDialog::MIBEvent & MIBManagmentDialog::MIBEvent::operator=(MIBEvent && other) noexcept
{
    if (this != &other)
    {
        type = other.type;
        table = std::move(other.table);
        other.type = MIBEventType::None;
    }

    return *this;
}