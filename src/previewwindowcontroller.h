#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QUrl>
#include <QObject>

class QFileSystemWatcher;
class PreviewWindow;
class MarkdownDoc;

////////////////////////////////////////////////////////////////////////////////////////////////////

class PreviewWindowController
        : public QObject
{
    Q_OBJECT

public:

    explicit PreviewWindowController(QObject* parent = 0);

public slots:

    void start();

private slots:

    void onUrlsDropped(QList<QUrl> urls);
    void onDocumentPathChanged(const QString& path);

signals:

    void documentChanged(MarkdownDoc* newDocument);

private:

    PreviewWindow* _window;
    MarkdownDoc* _doc;
    QFileSystemWatcher* _watcher;
};
