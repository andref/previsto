#pragma once

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QUrl>

////////////////////////////////////////////////////////////////////////////////////////////////////

class MarkdownDoc : public QObject
{
    Q_OBJECT

public:

    explicit MarkdownDoc(QUrl inputUrl, QObject *parent = 0);
    ~MarkdownDoc();

public:

    QUrl inputUrl() const;
    QString data() const;

public slots:

    void update();

signals:

    void ready();
    void error();

private slots:

    void onProcessFinished(int,QProcess::ExitStatus);
    void onProcessError(QProcess::ProcessError);

private:

    QFile* _input;
    QProcess* _process;
    QString _data;
    bool _error;
};
