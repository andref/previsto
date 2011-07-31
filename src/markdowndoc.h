#pragma once

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QUrl>

////////////////////////////////////////////////////////////////////////////////////////////////////

class MarkdownDoc : public QObject
{
    Q_OBJECT
    Q_ENUMS(Error)

public:

    enum Error {
        ProcessorNotAvailable,
        DocumentNotFound,
        DocumentNotReadable,
        ProcessorFailed
    };

    explicit MarkdownDoc(QUrl inputUrl, QObject *parent = 0);
    ~MarkdownDoc();

public:

    QUrl inputUrl() const;
    QString data() const;

public slots:

    void update();

signals:

    void ready();
    void error(MarkdownDoc::Error cause);

private slots:

    void onProcessFinished(int, QProcess::ExitStatus);
    void onProcessError();

private:

    QFile* _input;
    QProcess* _process;
    QString _data;
    bool _error;
};
