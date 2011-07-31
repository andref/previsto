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
    , _error(false)
{
    // Prepare the process

    _process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(onProcessFinished(int,QProcess::ExitStatus)));
    connect(_process, SIGNAL(error(QProcess::ProcessError)),
            SLOT(onProcessError(QProcess::ProcessError)));

    // Trigger the first update.

    update();
}

MarkdownDoc::~MarkdownDoc()
{}

/// Markdown Processing ////////////////////////////////////////////////////////////////////////////

// The markdown processor is installed for us in the release version
// at the appropriate place.

#if defined QT_NO_DEBUG
static const QString markdownPath = "/usr/local/lib/previsto/multimarkdown";
#else
static const QString markdownPath = "dist/peg-multimarkdown/multimarkdown";
#endif


void MarkdownDoc::update()
{
    if (_process->state() == QProcess::Running) {
        return;
    }

    QStringList arguments;
    arguments << "--extensions"
              << "--to" << "html"
              << _input->fileName();

    _error = false;
    _process->start(markdownPath, arguments);
}

void MarkdownDoc::onProcessFinished(int, QProcess::ExitStatus exitStatus)
{
    // Drain the read channels. The Qt docs do not specify what happens
    // to the channels when the process is started anew. Anyway, we

    QByteArray output = _process->readAllStandardOutput();
    _process->readAllStandardError();

    // Well, something bad happened.

    if (exitStatus == QProcess::CrashExit || _error) {
        emit error();
        return;
    }

    // Data is good, proceed.

    _data = QString::fromUtf8(output);
    emit ready();
}

void MarkdownDoc::onProcessError(QProcess::ProcessError)
{
    _process->readAllStandardOutput();
    _process->readAllStandardError();

    _error = true;
    emit error();
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
