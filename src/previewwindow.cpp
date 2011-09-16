#include <QDebug>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPoint>
#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QWebFrame>

#include "ui_previewwindow.h"
#include "markdowndoc.h"
#include "previewwindow.h"
#include "aboutwindow.h"


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

/// Construct a preview window. This is the main application Window
/// for Previsto. The window has three pages:
///
/// - Welcome: invites the user to drop a file
/// - Error: displayed when something goes wrong
/// - Preview: contains the document preview
///
/// When created, the preview window displayes the "welcome" page.
///
/// @param parent
///     The owner widget. This is generally `0`, because this is a
///     top level window.
PreviewWindow::PreviewWindow(QWidget *parent)
    : QWidget(parent)
    , _doc(0)
    , _ui(new Ui::PreviewWindow)
    , _firstTime(false)
{
    _ui->setupUi(this);
    _ui->stack->setCurrentWidget(_ui->welcomePage);

    // Configure the menu

    configureMenu();

    // We will handle external links since we are *not* a web browser.

    connect(_ui->documentView, SIGNAL(linkClicked(QUrl)),
            this, SLOT(onWebpageLinkClicked(QUrl)));

    // Restore the window geometry from last time.

    QSettings settings;
    settings.beginGroup("Preview Window");
    this->restoreGeometry(settings.value("geometry").toByteArray());
}

/// Configure the context menus. Each of the pages has a
/// different context menu.
void PreviewWindow::configureMenu()
{
    // This action stuff makes one wish every C++ API was
    // a bit like Qt.
    //
    // We define reusable actions and register
    // them with each of the views. Back in the .ui file, the
    // objects receiving actions have had their contextMenuPolicy
    // set to "ActionsContextMenu".

    // Create all the menu actions

    // TODO: Open file?

    QAction* copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(onCopy()));

    QAction* refreshAction = new QAction(tr("&Refresh"), this);
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(onRefresh()));

    QAction* printAction = new QAction(tr("&Print"), this);
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, SIGNAL(triggered()), this, SLOT(onPrint()));

    QAction* separator = new QAction(this);
    separator->setSeparator(true);

    QAction* aboutAction = new QAction(tr("&About Previsto"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(onAbout()));

    // Assemble the menu for the welcome page:

    _ui->welcomePage->insertAction(0, aboutAction);

    // Assemble the menu for the error page:

    _ui->errorPage->insertAction(0, aboutAction);

    // Assemble the menu for the document view:

    _ui->documentView->insertAction(0, copyAction);
    _ui->documentView->insertAction(0, refreshAction);
    _ui->documentView->insertAction(0, printAction);
    _ui->documentView->insertAction(0, separator);
    _ui->documentView->insertAction(0, aboutAction);
}


PreviewWindow::~PreviewWindow()
{
    delete _ui;
}

//// Menu Actions //////////////////////////////////////////////////////////////////////////////////

/// Called when the user activates the "Copy" menu item.
void PreviewWindow::onCopy()
{
    _ui->documentView->triggerPageAction(QWebPage::Copy);
}

/// Called when the user activates the "Refresh" menu item.
/// Usually, this is not really needed, but sometimes thing
/// may go wrong. We fake the current document's url being
/// dropped again and hope it is enough.
void PreviewWindow::onRefresh()
{
    if (!_doc) {
        // No document, nothing to refresh...
        return;
    }

    QList<QUrl> urls;
    urls << _doc->inputUrl();
    emit urlsDropped(urls);
}

/// Called when the user activates the "Print" menu item.
void PreviewWindow::onPrint()
{
    if (!_doc) {
        return;
    }

    // Printing. In FOUR lines. I ♥ Qt.
    // Exporting to PDF is just as easy.

    QPrintDialog printDialog;
    if (printDialog.exec() == QDialog::Accepted) {
        _ui->documentView->page()->mainFrame()->print(printDialog.printer());
    }
}

void PreviewWindow::onAbout()
{
    AboutWindow aboutWindow(this);
    aboutWindow.exec();
}

void PreviewWindow::onSettings()
{}

//// Document Changes //////////////////////////////////////////////////////////////////////////////

/// Set the document to be displayed.
///
/// @param document
///     The new document that will be displayed. It needs not
///     be fully loaded yet. We listen to its `ready` and `error`
///     signals.
void PreviewWindow::setDocument(QSharedPointer<MarkdownDoc> document)
{
    if (document == _doc) {
        return;
    }

    if (_doc) {
        // Goodbye. We don't want to hear from you again.
        _doc->disconnect(this);
    }

    // We are seeing this document for the first time.

    _doc = document;
    _firstTime = true;
    _ui->stack->setCurrentWidget(_ui->previewPage);

    // The ugly part of using QSharedPointer is that we have to explicitly
    // retrieve the raw pointer to connect signals.

    connect(_doc.data(), SIGNAL(ready()), SLOT(onDocumentReady()));
    connect(_doc.data(), SIGNAL(error(MarkdownDoc::Error)),
            SLOT(onDocumentError(MarkdownDoc::Error)));
}

/// Triggered when the current document is ready.
void PreviewWindow::onDocumentReady()
{
    QPoint currentScroll = _ui->documentView->page()->mainFrame()->scrollPosition();
    QSize docSize = _ui->documentView->page()->mainFrame()->contentsSize();
    QSize viewSize = _ui->documentView->size();

    // Check if the viewport is at the bottom of the document. If it is, we will "glue" the
    // viewport there because the document might have shrunk or grown.

    bool atBottom = currentScroll.y() >= docSize.height() - viewSize.height();

    // Prepare the data.

    QString html = HtmlProlog + _doc->data() + HtmlEpilog;

    _ui->documentView->setHtml(html, _doc->inputUrl());
    _ui->documentView->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);

    // Restore the scroll position of the internal frame so that it
    // doesn't jump back to the top. But do that only if this is the same
    // document.

    if (!_firstTime) {

        if (atBottom) {
            QSize newDocSize = _ui->documentView->page()->mainFrame()->contentsSize();
            currentScroll.setY(newDocSize.height() - viewSize.height());
        }

        _ui->documentView->page()->mainFrame()->setScrollPosition(currentScroll);
    }
    else {
        _firstTime = false;
    }
}

/// Triggered when there is an error with the current document.
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

/// Triggered when a link on the document is clicked. Internal
/// links are not handled by this method, however.
///
/// @param url
///     The url of the link that was clicked.
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

/// Triggered when the user drops "something" into the preview window
/// We again inspect to see what it is and if it is an URI List, we
/// tell the world the news.
///
/// @param e
///     Information about the drop event.
void PreviewWindow::dropEvent(QDropEvent* e)
{
    const QMimeData* data = e->mimeData();

    if (!data->hasFormat("text/uri-list")) {
        return QWidget::dropEvent(e);
    }

    emit urlsDropped(data->urls());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// Triggered when the window is closed (and the application is about to
/// quit).
///
/// @param e
///     Information about the close event.
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
