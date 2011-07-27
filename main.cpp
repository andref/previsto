#include <QtGui/QApplication>

#include "previewwindowcontroller.h"
#include "markdowndoc.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set the application metainfo.

    a.setApplicationName("Previsto");
    a.setApplicationVersion("0.1dev");
    a.setOrganizationDomain("andre.filli.pe");
    a.setOrganizationName("andre.filli.pe");

    // Start the window controller.

    PreviewWindowController controller;
    controller.start();

    // And off we go.

    return a.exec();
}
