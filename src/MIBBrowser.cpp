#include <MIBBrowser.h>
#include <ui/ui_MIBBrowser.h>

MIBBrowser::MIBBrowser(QWidget* parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _treeModel(new TreeModel(ModuleTable::Ptr(new ModuleTable("")))),
    _parser(nullptr)
{
    _ui->setupUi(this);
    _ui->treeView->setModel(_treeModel.get());
}
