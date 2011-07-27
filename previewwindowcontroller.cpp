#include <QDebug>
#include <QFileSystemWatcher>

#include "previewwindowcontroller.h"
#include "previewwindow.h"
#include "markdowndoc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

PreviewWindowController::PreviewWindowController(QObject *parent)
    : QObject(parent)
    , _window(new PreviewWindow())
    , _doc(0)
    , _watcher(new QFileSystemWatcher(this))
{
    connect(_window, SIGNAL(urlsAccepted(QList<QUrl>)),
            this, SLOT(onUrlsDropped(QList<QUrl>)));

    connect(this, SIGNAL(documentChanged(MarkdownDoc*)),
            _window, SLOT(setDocument(MarkdownDoc*)));

    // Wire up the watcher

    connect(_watcher, SIGNAL(fileChanged(QString)), this, SLOT(onDocumentPathChanged(QString)));

}

/**
    Start the application and show the window.
*/
void PreviewWindowController::start()
{
    _doc = 0;
    _window->show();
}

/**
    Handle and URL being dropped inside the application window.

    @param urls
        The list of dropped URLs. The current version ignores all URLs
        after the first.
*/
void PreviewWindowController::onUrlsDropped(QList<QUrl> urls)
{
    QUrl url = urls.takeFirst();
    if (url.scheme() != "file") {
        return;
    }

    if (_doc) {
        // Avoid unnecessary updates and notifications.
        _watcher->removePath(_doc->inputUrl().toLocalFile());

        // Get rid of the current document.
        _doc->deleteLater();
        _doc = 0; // neat freak.
    }

    _doc = new MarkdownDoc(url, this);

    // Make the document update itself when the file changes.

    _watcher->addPath(_doc->inputUrl().toLocalFile());
    emit documentChanged(_doc);
}

/**
    Update the document when the original file is changed.

    @param path
        The path to the changed file. Used to test if the filesystem watcher still
        contains the file.
*/
void PreviewWindowController::onDocumentPathChanged(const QString& path)
{
    _doc->update();

    // Lots of applications perform a save by first writing to a temp file
    // and then moving the temp file over the original file. It seems that
    // the filesystem watcher stops watching the original file in this
    // case. For this reason, we'll add it again. According to the Qt docs
    // there's no problem if we add the same path more than once, but we
    // like to be extra-neat.

    QStringList paths = _watcher->files();
    if (!paths.contains(path)) {
        _watcher->addPath(_doc->inputUrl().toLocalFile());
    }
}
