#include <QApplication>
#include "MIBViewerWindow.h"

int main(int argc, char** argv)
{
   QApplication app(argc, argv);
   MIBViewerWindow mibwin;
   mibwin.show();
   return app.exec();
}