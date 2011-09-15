#include "aboutwindow.h"
#include "ui_aboutwindow.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::AboutWindow)
{
    _ui->setupUi(this);
}

AboutWindow::~AboutWindow()
{
    delete _ui;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AboutWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        _ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
