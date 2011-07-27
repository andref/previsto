#include "markdowndoc.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>


MarkdownDoc::MarkdownDoc(QUrl inputUrl, QObject *parent)
    : QObject(parent)
    , _input(new QFile(inputUrl.toLocalFile()))
    , _process(new QProcess(this))
    , _data()
{
    // Prepare the process

    _process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(onProcessFinished(int,QProcess::ExitStatus)));
    connect(_process, SIGNAL(error(QProcess::ProcessError)),
            SLOT(onProcessError(QProcess::ProcessError)));

    // Open our temporary file.

    update();
}

MarkdownDoc::~MarkdownDoc()
{
}

/// Markdown Processing ////////////////////////////////////////////////////////////////////////////

/// TODO: This must go away!! Yikes!

static const QString markdownPath = "/usr/bin/multimarkdown";

static const QString HtmlProlog =
    "<!doctype html>"
    "<html>"
    "<head>"
    "<meta charset='utf-8'>"
    "<link rel='stylesheet' href='qrc:///default.css'>"
    "</head><body>";

static const QString HtmlEpilog =
    "</body>"
    "</html>";

void MarkdownDoc::update()
{
    if (_process->state() == QProcess::Running) {
        return;
    }

    QStringList arguments;
    arguments << "--extensions"
              << "--to" << "html"
              << _input->fileName();

    _process->start(markdownPath, arguments);
}

void MarkdownDoc::onProcessError(QProcess::ProcessError)
{
    QString err = QString::fromUtf8(_process->readAllStandardError());
    emit error("Failed to process the file.");
}

void MarkdownDoc::onProcessFinished(int, QProcess::ExitStatus)
{
    QByteArray output = _process->readAllStandardOutput();
    _data = HtmlProlog + QString::fromUtf8(output) + HtmlEpilog;
    emit ready();
}

/// Properties /////////////////////////////////////////////////////////////////////////////////////

QUrl MarkdownDoc::inputUrl() const
{
    return QUrl::fromLocalFile(_input->fileName());
}

QString MarkdownDoc::data() const
{
    return _data;
}
