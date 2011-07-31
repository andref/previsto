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

//// Error Messages ////////////////////////////////////////////////////////////////////////////////

static const char* ProcessorNotAvailableMessage = QT_TR_NOOP_UTF8(
    "<p>The Previsto installation might be broken: one of its "
    "components—the Markdown processor—is not available. "
    "Please, reinstall Previsto and try again.</p>"
);

static const char* ProcessorFailedMessage = QT_TR_NOOP_UTF8(
    "<p>The document could not be processed. Although "
    "it’s very hard to type invalid Markdown files, please "
    "check that the syntax of the file is correct.</p>"
);

static const char* DocumentNotFoundMessage = QT_TR_NOOP_UTF8(
    "<p>The document wasn’t found. Please make sure "
    "it was not erased, renamed or moved to another "
    "folder.</p>"
);

static const char* DocumentNotReadableMessage = QT_TR_NOOP_UTF8(
    "<p>The document cannot be accessed. We tried to change "
    "its permissions, but that didn’t work. Please, make "
    "sure you are allowed to access the document and try "
    "again.</p>"
);

//// Html Stuff ////////////////////////////////////////////////////////////////////////////////////

static const QString HtmlProlog =
    "<!doctype html>"
    "<html>"
    "<head>"
    "<meta charset='utf-8'>"
    "<link rel='stylesheet' href='qrc:///Default.css'>"
    "</head>"
    "<body>";

static const QString HtmlEpilog =
    "</body>"
    "</html>";

////////////////////////////////////////////////////////////////////////////////////////////////////

PreviewWindow::PreviewWindow(QWidget *parent)
    : QWidget(parent)
    , _doc(0)
    , _ui(new Ui::PreviewWindow)
    , _firstTime(false)
{
    _ui->setupUi(this);
    _ui->stack->setCurrentWidget(_ui->previewPage);

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

void PreviewWindow::setDocument(QSharedPointer<MarkdownDoc> document)
{
    if (document == _doc) {
        return;
    }

    // We are seeing this document for the first time.

    _doc = document;
    _firstTime = true;
    _ui->stack->setCurrentWidget(_ui->previewPage);

    connect(_doc.data(), SIGNAL(ready()), SLOT(onDocumentReady()));
    connect(_doc.data(), SIGNAL(error(MarkdownDoc::Error)),
            SLOT(onDocumentError(MarkdownDoc::Error)));
}

void PreviewWindow::onDocumentReady()
{
    QPoint currentScroll = _ui->documentView->page()->mainFrame()->scrollPosition();

    // Prepare the data.

    QString html = HtmlProlog + _doc->data() + HtmlEpilog;

    _ui->documentView->setHtml(html);
    _ui->documentView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    // Restore the scroll position of the internal frame so that it
    // doesn't jump back to the top. But do that only if this is the same
    // document.

    if (!_firstTime) {
        _ui->documentView->page()->mainFrame()->setScrollPosition(currentScroll);
    }
    else {
        _firstTime = false;
    }
}

void PreviewWindow::onDocumentError(MarkdownDoc::Error cause)
{
    switch (cause) {
    case MarkdownDoc::ProcessorNotAvailable:
        _ui->errorMessage->setText(tr(::ProcessorNotAvailableMessage));
        break;
    case MarkdownDoc::DocumentNotFound:
        _ui->errorMessage->setText(tr(::DocumentNotFoundMessage));
        break;
    case MarkdownDoc::DocumentNotReadable:
        _ui->errorMessage->setText(tr(::DocumentNotReadableMessage));
        break;
    case MarkdownDoc::ProcessorFailed:
    default:
        _ui->errorMessage->setText(tr(::ProcessorFailedMessage));
        break;
    }

    _ui->stack->setCurrentWidget(_ui->errorPage);
}

//// Webpage Links /////////////////////////////////////////////////////////////////////////////////

void PreviewWindow::onWebpageLinkClicked(const QUrl& url)
{
    // XXX: Shouldn't we give this back to the controller instead?
    QDesktopServices::openUrl(url);
}

//// Drag and Drop /////////////////////////////////////////////////////////////////////////////////

/// Inspect the data being dragged. The drag operation will only be accepted
/// if the data contains an URI list whose first element points to a local
/// file.
///
/// @param e
///     The drag event being scrutinized.
void PreviewWindow::dragEnterEvent(QDragEnterEvent* e)
{
    const QMimeData* data = e->mimeData();

    if (!data->hasFormat("text/uri-list")) {
        return QWidget::dragEnterEvent(e);
    }

    // I believe this is unlikely, but let's test for it anyway.

    QList<QUrl> list = data->urls();
    if (list.isEmpty()) {
        return QWidget::dragEnterEvent(e);
    }

    // We only handle local files.

    QUrl first = list.takeFirst();
    if (first.scheme() != "file") {
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

    emit urlsDropped(data->urls());
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
