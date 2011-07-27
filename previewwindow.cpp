#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPoint>
#include <QSettings>
#include <QWebFrame>

#include "ui_previewwindow.h"
#include "markdowndoc.h"
#include "previewwindow.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

PreviewWindow::PreviewWindow(QWidget *parent)
    : QWidget(parent)
    , _doc(0)
    , _ui(new Ui::PreviewWindow)
{
    _ui->setupUi(this);
    _ui->documentView->setAcceptDrops(false);
    this->setAcceptDrops(true);

    // Restore the window geometry from last time.

    QSettings settings;
    settings.beginGroup("Preview Window");
    this->restoreGeometry(settings.value("geometry").toByteArray());
}

PreviewWindow::~PreviewWindow()
{
    delete _ui;
}

//// Document Changes //////////////////////////////////////////////////////////////////////////////

void PreviewWindow::setDocument(MarkdownDoc *document)
{
    if (document == _doc) {
        return;
    }

    _doc = document;
    connect(_doc, SIGNAL(ready()), this, SLOT(onDocumentReady()));
    connect(_doc, SIGNAL(error(QString)), this, SLOT(onDocumentError(QString)));
}

MarkdownDoc* PreviewWindow::document() const
{
    return _doc;
}

void PreviewWindow::onDocumentReady()
{
    QPoint currentScroll = _ui->documentView->page()->mainFrame()->scrollPosition();
    _ui->documentView->setHtml(_doc->data());
    _ui->documentView->page()->mainFrame()->setScrollPosition(currentScroll);
}

void PreviewWindow::onDocumentError(const QString &error)
{
    Q_UNUSED(error); // Really?
}

//// Drag and Drop /////////////////////////////////////////////////////////////////////////////////

void PreviewWindow::dragEnterEvent(QDragEnterEvent* e)
{
    const QMimeData* data = e->mimeData();

    if (!data->hasFormat("text/uri-list")) {
        return QWidget::dragEnterEvent(e);
    }

    e->acceptProposedAction();
}

void PreviewWindow::dropEvent(QDropEvent* e)
{
    const QMimeData* data = e->mimeData();

    if (!data->hasFormat("text/uri-list")) {
        return QWidget::dropEvent(e);
    }

    emit urlsAccepted(data->urls());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PreviewWindow::closeEvent(QCloseEvent* e)
{
    // Save the window geometry for next time.

    QSettings settings;
    settings.beginGroup("Preview Window");
    settings.setValue("geometry", this->saveGeometry());
    QWidget::closeEvent(e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PreviewWindow::changeEvent(QEvent *e)
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
