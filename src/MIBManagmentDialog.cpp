#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <MIBManagmentDialog.h>
#include <Parser.h>
#include <fstream>
#include <ui/ui_MIBManagmentDialog.h>

MIBManagmentDialog::MIBManagmentDialog(QWidget * parent, const QString& LastFolderPath, const ModuleInfoTable& ModulesInfoTable) :
    QDialog(parent),
    _ui(new Ui::MIBManagmentWindow),
    _lastFolderPath(LastFolderPath),
    _modulesInfoTable(ModulesInfoTable)
{
    _ui->setupUi(this);
    _ui->loadModuleButton->setEnabled(false);
    _ui->unloadModuleButton->setEnabled(false);
    this->setWindowModality(Qt::WindowModal);

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
        if (_ui->availableModulesList->currentRow() != -1)
            _ui->loadModuleButton->setEnabled(true);
        else
            _ui->loadModuleButton->setEnabled(false);
    });

    connect(_ui->loadedModulesList, &QListWidget::itemSelectionChanged, [this]()
    {
        if (_ui->loadedModulesList->currentRow() != -1)
            _ui->unloadModuleButton->setEnabled(true);
        else
            _ui->unloadModuleButton->setEnabled(false);
    });
}

MIBManagmentDialog::~MIBManagmentDialog()
{
    delete _ui;
}

QString MIBManagmentDialog::getFolderPath()
{
    return _lastFolderPath;
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

        //ModuleInfoTable newModuleInfoTable;     
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

                        //Получаем список импортируемых модулей
                        Strs moduleImports;
                        while (token.type != LT::eSEMI && token.type != LT::eENDOFFILE)
                        {
                            if (token.type == LT::eFROM)
                            {
                                parser.parseToken(file, token);

                                if (token.type == LT::eLABEL)
                                    moduleImports.push_back(token.lexem);
                            }

                            parser.parseToken(file, token);
                        }

                        _modulesInfoTable[moduleName] = ModuleInfo::Ptr(new ModuleInfo(moduleName, modulePath, false, moduleImports));
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

    for (auto const& moduleItem : moduleiItemsToLoad)
    {
        // Отображаем все модули, которые будут загружены
        addAllModules(moduleItem->text().toStdString());
    }
}

Q_SLOT void MIBManagmentDialog::unloadModule()
{
    auto moduleiItemsToUnload = _ui->loadedModulesList->selectedItems();

    for (auto const& moduleItem : moduleiItemsToUnload)
    {
        std::string moduleName = moduleItem->text().toStdString();
        removeAllModules(moduleName);
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

    this->accept();
}

void MIBManagmentDialog::updateOrMoveToHistory(MIBEventType Type, ModuleInfoTable&& Data)
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
    QString ModuleName = QString::fromStdString(RootModuleName);

    // Если модуль еще не загружен
    if (_ui->loadedModulesList->findItems(ModuleName, Qt::MatchExactly).isEmpty())
    {

        // Добавляем в историю модуль, который нужно загрузить
        updateOrMoveToHistory(MIBEventType::Load, ModuleInfoTable{ { RootModuleName, nullptr } });

        // Отображаем модуль в списке загруженных
        _ui->loadedModulesList->addItem(ModuleName);
        // Убираем модуль из списка доступных для загрузки
        delete _ui->availableModulesList->findItems(ModuleName, Qt::MatchExactly).first();
    }

    // Находим список импортируемых модулей
    auto moduleImports = _modulesInfoTable.find(RootModuleName)->second->moduleImports;

    // Добавляем их
    for (auto const& moduleImport : moduleImports)
        addAllModules(moduleImport);
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
                        bool alreadyAdded = false;

                        for (auto const& addedModule : res)
                        {
                            if (addedModule == moduleName)
                            {
                                alreadyAdded = true;
                                break;
                            }
                        }

                        if (!alreadyAdded)
                        {
                            res.push_back(moduleName);
                            break;
                        }
                    }
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

    if (!modules.empty())
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
            return;
    }

    modules.push_back(RootModuleName);

    for (auto const& moduleToRemove : modules)
    {
        updateOrMoveToHistory(MIBEventType::Unload, ModuleInfoTable{ {moduleToRemove, nullptr} });

        QString ModuleName = QString::fromStdString(moduleToRemove);
        _ui->availableModulesList->addItem(ModuleName);
        delete _ui->loadedModulesList->findItems(ModuleName, Qt::MatchExactly).first();
    }
}

MIBManagmentDialog::MIBEvent::MIBEvent(MIBEventType Type, const ModuleInfoTable & Table) :
    type(Type),
    table(Table)
{}

MIBManagmentDialog::MIBEvent::MIBEvent(MIBEventType Type, ModuleInfoTable && Table) :
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