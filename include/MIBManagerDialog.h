#pragma once
#include <QDialog>
#include <Module.h>

namespace Ui
{
	class MIBManagerWindow;
}

class MIBManagerDialog: public QDialog
{
public:

    enum class MIBEventType
    {
        Refresh,
        Load,
        Unload,
        None
    };

    struct MIBEvent
    {
        //MIBEvent(MIBEventType Type, const  ModuleMetaDataTable::Ptr& Table = nullptr);
        //MIBEvent(MIBEventType Type, ModuleMetaDataTable::Ptr&& Table);

        MIBEvent(MIBEventType Type, const std::string& ModuleName = "");
        MIBEvent(MIBEventType Type, std::string&& ModuleName);
        MIBEvent(const MIBEvent& other);
        MIBEvent(MIBEvent&& other) noexcept;

        MIBEvent& operator=(const MIBEvent& other);
        MIBEvent& operator=(MIBEvent&& other) noexcept;

        MIBEventType type;
        std::string moduleName;
        //ModuleMetaDataTable::Ptr table;
    };

    using MIBEventList = std::vector<MIBEvent>;

    explicit MIBManagerDialog(QWidget* parent = nullptr,  
        const ModuleMetaDataTable::Ptr& ModulesDataTable = nullptr);

	~MIBManagerDialog();

    MIBEventList getMIBEventList();
	Q_SLOT void selectFolder();
	Q_SLOT void loadModule();
	Q_SLOT void unloadModule();
	Q_SLOT void cancelСonfigure();
	Q_SLOT void doneСonfigure();

private:

    void saveData();
    void loadSavedData();

    //void updateOrMoveToHistory(MIBEventType Type, ModuleMetaDataTable::Ptr&& Data);
    void updateOrMoveToHistory(MIBEventType Type, std::string&& Data);
    void addAllModules(const std::string& RootModuleName);
    Strs forWhichModulesIsRoot(const std::string& RootModuleName);
    void removeAllModules(const std::string& RootModuleName);

	Ui::MIBManagerWindow*       _ui;
	QString                     _lastFolderPath;
    ModuleMetaDataTable::Ptr    _modulesDataTable;
    MIBEventList                _eventList;
};