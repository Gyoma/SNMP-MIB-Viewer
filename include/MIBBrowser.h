#pragma once
#include <QMainWindow>
#include <TreeModel.h>
#include <Parser.h>

namespace Ui
{
	class MainWindow;
}

class MIBBrowser : public QMainWindow
{
	Q_OBJECT

public:

	explicit MIBBrowser(QWidget* parent = nullptr);

private:

	std::shared_ptr<Ui::MainWindow> _ui;
	TreeModel::Ptr					_treeModel;
	Parser::Ptr						_parser;
};