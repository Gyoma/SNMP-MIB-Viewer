#pragma once
#include <QDialog>
#include <Module.h>

namespace Ui
{
	class MIBManagmentWindow;
}

class MIBManagmentDialog: public QDialog
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
        MIBEvent(MIBEventType Type = MIBEventType::None, const ModuleMetaDataTable& Table = {});
        MIBEvent(MIBEventType Type, ModuleMetaDataTable&& Table);
        MIBEvent(const MIBEvent& other);
        MIBEvent(MIBEvent&& other) noexcept;

        MIBEvent& operator=(const MIBEvent& other);
        MIBEvent& operator=(MIBEvent&& other) noexcept;

        MIBEventType type;
        ModuleMetaDataTable table;
    };

    using MIBEventList = std::vector<MIBEvent>;

    explicit MIBManagmentDialog(QWidget* parent = nullptr,  
        const ModuleMetaDataTable& ModulesInfoTable = {});

	~MIBManagmentDialog();

    MIBEventList getMIBEventList();
	Q_SLOT void selectFolder();
	Q_SLOT void loadModule();
	Q_SLOT void unloadModule();
	Q_SLOT void cancelСonfigure();
	Q_SLOT void doneСonfigure();

private:

    void saveData();
    void loadSavedData();

    void updateOrMoveToHistory(MIBEventType Type, ModuleMetaDataTable&& Data);
    void addAllModules(const std::string& RootModuleName);
    Strs forWhichModulesIsRoot(const std::string& RootModuleName);
    void removeAllModules(const std::string& RootModuleName);

	Ui::MIBManagmentWindow* _ui;
	QString                 _lastFolderPath;
    ModuleMetaDataTable         _modulesInfoTable;
    MIBEventList               _eventList;
};