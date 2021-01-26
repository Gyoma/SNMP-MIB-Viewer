#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <MIBManagerDialog.h>
#include <Parser.h>
#include <fstream>
#include <ui/ui_MIBManagerDialog.h>

MIBManagerDialog::MIBManagerDialog(QWidget * parent, const ModuleMetaDataTable::Ptr& ModulesDataTable) :
    QDialog(parent, Qt::WindowCloseButtonHint),
    _ui(new Ui::MIBManagerWindow),
    _modulesDataTable(ModulesDataTable)
{
    _ui->setupUi(this);
    _ui->loadModuleButton->setEnabled(false);
    _ui->unloadModuleButton->setEnabled(false);
    this->setWindowModality(Qt::WindowModal);
    this->loadSavedData();

    for (auto const& moduleInfo : _modulesDataTable->asVector())
    {
        if (moduleInfo->needToLoad)
            _ui->loadedModulesList->addItem(QString::fromStdString(moduleInfo->moduleName));
        else
            _ui->availableModulesList->addItem(QString::fromStdString(moduleInfo->moduleName));
    }

    connect(_ui->selectFolderButton, &QPushButton::clicked, this, &MIBManagerDialog::selectFolder);
    connect(_ui->loadModuleButton, &QPushButton::clicked, this, &MIBManagerDialog::loadModule);
    connect(_ui->unloadModuleButton, &QPushButton::clicked, this, &MIBManagerDialog::unloadModule);
    connect(_ui->okButton, &QPushButton::clicked, this, &MIBManagerDialog::doneСonfigure);
    connect(_ui->cancelButton, &QPushButton::clicked, this, &MIBManagerDialog::cancelСonfigure);

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

MIBManagerDialog::~MIBManagerDialog()
{
    delete _ui;
}

MIBManagerDialog::MIBEventList MIBManagerDialog::getMIBEventList()
{
    return _eventList;
}

Q_SLOT void MIBManagerDialog::selectFolder()
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
        _modulesDataTable->clear();

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

                if (token.type == LT::DEFINITIONS)
                {
                    if (_ui->availableModulesList->findItems(QString::fromStdString(moduleName), Qt::MatchExactly).isEmpty())
                    {
                        _ui->availableModulesList->addItem(QString::fromStdString(moduleName));

                        //Список импортируемых модулей
                        Strs importedModules;
                        //Список импортируемых cущностей
                        Strs importedLabels;

                        //Получаем список импортируемых модулей
                        while (token.type != LT::SEMI && token.type != LT::ENDOFFILE)
                        {
                            //Если импортируема сущность не является встроенной
                            if (token.type == LT::LABEL)
                            {
                                importedLabels.push_back(token.lexem);
                            }
                            else if (token.type == LT::FROM)
                            {
                                parser.parseToken(file, token);

                                //Если имеет смысл импортировать этот модуль
                                if (token.type == LT::LABEL && !importedLabels.empty())
                                    importedModules.push_back(token.lexem);

                                importedLabels.clear();
                            }

                            parser.parseToken(file, token);
                        }

                        _modulesDataTable->addModuleData(ModuleMetaData::Ptr(new ModuleMetaData(moduleName, modulePath, false, importedModules)));

                        //_modulesDataTable[moduleName] = ModuleMetaData::Ptr(new ModuleMetaData(moduleName, modulePath, false, importedModules));
                    }
                }
            }
        }

        updateOrMoveToHistory(MIBEventType::Refresh, "");
    }
}

Q_SLOT void MIBManagerDialog::loadModule()
{
    auto moduleiItemsToLoad = _ui->availableModulesList->selectedItems();

    while (!moduleiItemsToLoad.isEmpty())
    {
        addAllModules(moduleiItemsToLoad.first()->text().toStdString());
        moduleiItemsToLoad = _ui->availableModulesList->selectedItems();
    }
}

Q_SLOT void MIBManagerDialog::unloadModule()
{
    auto moduleiItemsToUnload = _ui->loadedModulesList->selectedItems();

    while (!moduleiItemsToUnload.isEmpty())
    {
        removeAllModules(moduleiItemsToUnload.first()->text().toStdString());
        moduleiItemsToUnload = _ui->loadedModulesList->selectedItems();
    }
}

Q_SLOT void MIBManagerDialog::cancelСonfigure()
{
    this->reject();
}

Q_SLOT void MIBManagerDialog::doneСonfigure()
{
    this->saveData();
    this->accept();
}

void MIBManagerDialog::updateOrMoveToHistory(MIBEventType Type, std::string&& ModuleName)
{
    if (Type == MIBEventType::Refresh)
    {
        _eventList.clear();
    }
    else
    {
        for (auto it = _eventList.begin(); it != _eventList.end(); ++it)
            if (it->type != MIBEventType::Refresh && it->moduleName == ModuleName)
            {
                // Если противоположные операции, то делать ничего не нужно
                if ((it->type == MIBEventType::Load && Type == MIBEventType::Unload) ||
                    (it->type == MIBEventType::Unload && Type == MIBEventType::Load))
                    _eventList.erase(it);

                return;
            }
    }

    _eventList.emplace_back(Type, std::move(ModuleName));
}

void MIBManagerDialog::addAllModules(const std::string& RootModuleName)
{
    //если такого модуля нет в доступных
    if (_modulesDataTable->findModuleData(RootModuleName) == nullptr)
        return;

    QString qRootName = QString::fromStdString(RootModuleName);
    std::list<std::string> checklist = { RootModuleName };

    for (auto const& ModuleName : checklist)
    {
        auto const& moduleImports = _modulesDataTable->findModuleData(ModuleName)->moduleImports;

        for (size_t i = 0, size = moduleImports.size(); i < size; ++i)
        {
            auto res = _modulesDataTable->findModuleData(moduleImports[i]);
            Strs imports = { moduleImports[i] };

            if (res == nullptr)
            {
                auto replacements = Parser::getModuleReplacements(moduleImports[i]);
                bool canReplace = true;

                if (replacements.empty())
                    canReplace = false;
                else
                {
                    for (auto const& replacement : replacements)
                    {
                        auto res = _modulesDataTable->findModuleData(replacement);
                        if (res == nullptr)
                        {
                            canReplace = false;
                            break;
                        }
                    }
                }

                if (!canReplace)
                {
                    QString message = QString::fromUtf8("Ошибка при загрузке ") +
                        qRootName +
                        QString::fromUtf8("\n\nОтсутсвует импортируемый модуль ") +
                        QString::fromStdString(moduleImports[i]) +
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

                imports = std::move(replacements);
            }

            for (auto const& import : imports)
                if (std::find(checklist.begin(), checklist.end(), import) == checklist.end())
                    checklist.push_back(import);
        }
    }

    for (auto& moduleName : checklist)
    {
        QString ModuleName = QString::fromStdString(moduleName);
        // Если модуль еще не загружен
        if (_ui->loadedModulesList->findItems(ModuleName, Qt::MatchExactly).isEmpty())
        {
            // Отображаем модуль в списке загруженных
            _ui->loadedModulesList->addItem(ModuleName);
            // Убираем модуль из списка доступных для загрузки
            delete _ui->availableModulesList->findItems(ModuleName, Qt::MatchExactly).first();
            // Добавляем в историю модуль, который нужно загрузить
            updateOrMoveToHistory(MIBEventType::Load, std::move(moduleName));
        }
    }
}

Strs MIBManagerDialog::forWhichModulesIsRoot(const std::string & RootModuleName)
{
    Strs res = { RootModuleName };

    for (size_t i = 0; i < res.size(); ++i)
    {
        auto RootName = res[i];

        for (auto modulePair : _modulesDataTable->asVector())
        {
            auto const& moduleName = modulePair->moduleName;
            auto const& moduleImports = modulePair->moduleImports;

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

void MIBManagerDialog::removeAllModules(const std::string & RootModuleName)
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
        messageBox.setEscapeButton(QMessageBox::No);

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

    for (auto& moduleToRemove : modules)
    {
        QString ModuleName = QString::fromStdString(moduleToRemove);
        _ui->availableModulesList->addItem(ModuleName);
        delete _ui->loadedModulesList->findItems(ModuleName, Qt::MatchExactly).first();
        updateOrMoveToHistory(MIBEventType::Unload, std::move(moduleToRemove));
    }
}

void MIBManagerDialog::saveData()
{
    QDir dir;

    if (!dir.exists("data"))
        dir.mkpath("data");

    QFile saveFile("data/mibmanager.json");

    if (!saveFile.open(QIODevice::WriteOnly))
        return;

    QJsonObject MIBDialogData;

    MIBDialogData["LastFolderPath"] = _lastFolderPath;
    MIBDialogData["DisableWarnings"] = _ui->warningBox->isChecked();

    saveFile.write(QJsonDocument(MIBDialogData).toJson());
}

void MIBManagerDialog::loadSavedData()
{
    QFile loadFile("data/mibmanager.json");

    if (!loadFile.open(QIODevice::ReadOnly))
        return;

    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll()));
    QJsonObject MIBDialogData(loadDoc.object());

    if (MIBDialogData.contains("LastFolderPath") && MIBDialogData["LastFolderPath"].isString())
        _lastFolderPath = MIBDialogData["LastFolderPath"].toString();

    if (MIBDialogData.contains("DisableWarnings") && MIBDialogData["DisableWarnings"].isBool())
        _ui->warningBox->setChecked(MIBDialogData["DisableWarnings"].toBool());
}

MIBManagerDialog::MIBEvent::MIBEvent(MIBEventType Type, const std::string & ModuleName) :
    type(Type),
    moduleName(ModuleName)
{}

MIBManagerDialog::MIBEvent::MIBEvent(MIBEventType Type, std::string && ModuleName) :
    type(Type),
    moduleName(std::move(ModuleName))
{}

MIBManagerDialog::MIBEvent::MIBEvent(const MIBEvent & other) :
    MIBEvent(other.type, other.moduleName)
{}

MIBManagerDialog::MIBEvent::MIBEvent(MIBEvent && other) noexcept :
    type(other.type),
    moduleName(std::move(other.moduleName))
{
    other.type = MIBEventType::None;
}

MIBManagerDialog::MIBEvent & MIBManagerDialog::MIBEvent::operator=(const MIBEvent & other)
{
    if (this != &other)
    {
        type = other.type;
        moduleName = other.moduleName;
    }

    return *this;
}

MIBManagerDialog::MIBEvent & MIBManagerDialog::MIBEvent::operator=(MIBEvent && other) noexcept
{
    if (this != &other)
    {
        type = other.type;
        moduleName = std::move(other.moduleName);
        other.type = MIBEventType::None;
    }

    return *this;
}