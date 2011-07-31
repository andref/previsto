#pragma once

#include <QList>
#include <QUrl>
#include <QWidget>
#include <QSharedPointer>

#include "markdowndoc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Ui {
    class PreviewWindow;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class PreviewWindow : public QWidget
{
    Q_OBJECT

public:

    explicit PreviewWindow(QWidget* parent = 0);
    ~PreviewWindow();

    void setDocument(QSharedPointer<MarkdownDoc> document);

signals:

    void urlsDropped(const QList<QUrl>& uriList);

protected:

    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
    void closeEvent(QCloseEvent* e);
    void changeEvent(QEvent* e);

private slots:

    void onWebpageLinkClicked(const QUrl &url);
    void onDocumentReady();
    void onDocumentError(MarkdownDoc::Error cause);

private:

    QSharedPointer<MarkdownDoc> _doc;
    Ui::PreviewWindow *_ui;
    bool _firstTime;
};
