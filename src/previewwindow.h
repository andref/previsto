#pragma once

#include <QList>
#include <QUrl>
#include <QWidget>

class MarkdownDoc;

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


    Q_PROPERTY(MarkdownDoc* document READ document WRITE setDocument)
    MarkdownDoc* document() const;
    Q_SLOT void setDocument(MarkdownDoc* document);

signals:

    void urlsAccepted(const QList<QUrl>& uriList);

protected:

    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
    void closeEvent(QCloseEvent* e);
    void changeEvent(QEvent* e);

private slots:

    void onDocumentReady();
    void onDocumentError();

private:

    MarkdownDoc* _doc;
    Ui::PreviewWindow *_ui;
};
