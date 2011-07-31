#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QUrl>
#include <QObject>
#include <QSharedPointer>

#include "markdowndoc.h"

class QFileSystemWatcher;
class PreviewWindow;

////////////////////////////////////////////////////////////////////////////////////////////////////

class AppController
        : public QObject
{
    Q_OBJECT

public:

    explicit AppController(QObject* parent = 0);

public slots:

    void start();

private slots:

    void onUrlsDropped(QList<QUrl> urls);
    void onDocumentChanged(const QString& path);

private:

    PreviewWindow* _window;
    QSharedPointer<MarkdownDoc> _doc;
    QFileSystemWatcher* _watcher;
};
