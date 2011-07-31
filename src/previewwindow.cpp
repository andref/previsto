#include <QDebug>
#include <QDesktopServices>
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
    , _newDoc(false)
{
    _ui->setupUi(this);

    // We will handle all link clicks since we are *not* a web browser.

    connect(_ui->documentView, SIGNAL(linkClicked(QUrl)),
            this, SLOT(onWebpageLinkClicked(QUrl)));

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
    _newDoc = true;

    connect(_doc, SIGNAL(ready()), this, SLOT(onDocumentReady()));
    connect(_doc, SIGNAL(error()), this, SLOT(onDocumentError()));
}

MarkdownDoc* PreviewWindow::document() const
{
    return _doc;
}

void PreviewWindow::onDocumentReady()
{

    QPoint currentScroll = _ui->documentView->page()->mainFrame()->scrollPosition();
    _ui->documentView->setHtml(_doc->data());
    _ui->documentView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    // Restore the scroll position of the internal frame so that it
    // doesn't jump back to the top. But do that only if this is the same
    // document.

    if (!_newDoc) {
        _ui->documentView->page()->mainFrame()->setScrollPosition(currentScroll);
    }
    else {
        _newDoc = false;
    }
}

void PreviewWindow::onDocumentError()
{
}

//// Webpage Links /////////////////////////////////////////////////////////////////////////////////

void PreviewWindow::onWebpageLinkClicked(const QUrl& url)
{
    // XXX: Shouldn't we give this back to the controller instead?
    QDesktopServices::openUrl(url);
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
