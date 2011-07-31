#include <QApplication>
#include <QDebug>
#include <QFileSystemWatcher>

#include "appcontroller.h"
#include "previewwindow.h"
#include "markdowndoc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

AppController::AppController(QObject *parent)
    : QObject(parent)
    , _window(new PreviewWindow())
    , _doc(0)
    , _watcher(new QFileSystemWatcher(this))
{
    connect(_window, SIGNAL(urlsDropped(QList<QUrl>)), SLOT(onUrlsDropped(QList<QUrl>)));
    connect(_watcher, SIGNAL(fileChanged(QString)), SLOT(onDocumentChanged(QString)));
}

/// Start the application and show the window. If the app was called with a
/// file as its first argument, the document is loaded.
void AppController::start()
{
    _window->show();

    // Check if an URL was passed via arguments.

    QStringList args = QApplication::instance()->arguments();
    if (args.size() > 1) {
        QList<QUrl> urls;
        urls << QUrl::fromLocalFile(args.at(1));
        onUrlsDropped(urls);
    }
}

/// Handle and URL being dropped inside the application window. If there is
/// a current document, it will not be tracked anymore. The window is updated
/// with the new document.
///
/// @param urls
///     The list of dropped URLs. The current version ignores all URLs
///     after the first.
void AppController::onUrlsDropped(QList<QUrl> urls)
{
    QUrl url = urls.takeFirst();

    if (_doc) {
        if (url == _doc->inputUrl()) {
            return;
        }

        // Stop tracking changes to the current document.
        _watcher->removePath(_doc->inputUrl().toLocalFile());
    }

    _doc = QSharedPointer<MarkdownDoc>(new MarkdownDoc(url));
    _watcher->addPath(_doc->inputUrl().toLocalFile());
    _window->setDocument(_doc);

    // Trigger the first update.

    _doc->update();
}

/// Update the document when the original file is changed.
///
/// @param path
///     The path to the changed file. Used to test if the filesystem watcher still
///     contains the file.
void AppController::onDocumentChanged(const QString& path)
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

    // TODO: Handle the case when the document is deleted.
}
