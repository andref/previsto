#include <QtGui/QApplication>
#include <QTextCodec>

#include "appcontroller.h"
#include "markdowndoc.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize text processing.

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    // Set the application metainfo.

    a.setApplicationName("Previsto");
    a.setApplicationVersion("0.1dev");
    a.setOrganizationDomain("andre.filli.pe");
    a.setOrganizationName("andre.filli.pe");

    // Start the window controller.

    AppController controller;
    controller.start();

    // And off we go.

    return a.exec();
}
