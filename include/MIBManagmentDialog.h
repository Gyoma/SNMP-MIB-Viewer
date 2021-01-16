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

    enum class EventType
    {
        Refresh,
        Load,
        Unload,
        None
    };

    struct Event
    {
        Event(EventType Type = EventType::None, const ModuleInfoTable& Table = {});
        Event(EventType Type, ModuleInfoTable&& Table);
        Event(const Event& other);
        Event(Event&& other) noexcept;

        Event& operator=(const Event& other);
        Event& operator=(Event&& other) noexcept;

        EventType type;
        ModuleInfoTable table;
    };

    using EventList = std::vector<Event>;

    explicit MIBManagmentDialog(QWidget* parent = nullptr, 
        const QString& LastFolderPath = "", 
        const ModuleInfoTable& ModulesInfoTable = {});

	~MIBManagmentDialog();

	QString getFolderPath();
    EventList getEventList();

	Q_SLOT void selectFolder();
	Q_SLOT void loadModule();
	Q_SLOT void unloadModule();
	Q_SLOT void cancelСonfigure();
	Q_SLOT void doneСonfigure();

private:

    void updateOrMoveToHistory(EventType Type, ModuleInfoTable&& Data);
    void addAllModules(const std::string& RootModuleName);
    Strs forWhichModulesIsRoot(const std::string& RootModuleName);
    void removeAllModules(const std::string& RootModuleName);

	Ui::MIBManagmentWindow* _ui;
	QString                 _lastFolderPath;
    ModuleInfoTable         _modulesInfoTable;
    EventList               _eventList;
};