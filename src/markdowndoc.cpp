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
            SLOT(onProcessError()));
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

/// Update the document. The update can only proceed if the processor is available
/// and the document exists and is readable. If an update is already in progress,
/// this operation has no effect.
void MarkdownDoc::update()
{
    if (_process->state() != QProcess::NotRunning) {
        return;
    }

    if (!QFile::exists(markdownPath)) {
        emit error(MarkdownDoc::ProcessorNotAvailable);
        return;
    }

    if (!QFile::permissions(markdownPath) & QFile::ExeUser) {
        emit error(MarkdownDoc::ProcessorNotAvailable);
        return;
    }

    if (!_input->exists()) {
        emit error(MarkdownDoc::DocumentNotFound);
        return;
    }

    if (!_input->isReadable()) {
        if (!_input->setPermissions(_input->permissions() | QFile::ReadUser)) {
            emit error(MarkdownDoc::DocumentNotReadable);
            return;
        }
    }

    QStringList arguments;
    arguments << "--extensions"
              << "--to" << "html"
              << _input->fileName();

    _error = false;
    _process->start(markdownPath, arguments);
}

void MarkdownDoc::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Drain the read channels. The Qt docs do not specify what happens
    // to the channels when the process is started anew. Anyway, we

    QByteArray output = _process->readAllStandardOutput();
    _process->readAllStandardError();

    // Something bad happenned and we cannot trust that the
    // output is what we want.

    if (exitCode != 0 || exitStatus == QProcess::CrashExit || _error) {
        emit error(MarkdownDoc::ProcessorFailed);
        return;
    }

    // Data is good, proceed.

    _data = QString::fromUtf8(output);
    emit ready();
}

/// Handle process failures
void MarkdownDoc::onProcessError()
{
    _process->readAllStandardOutput();
    _process->readAllStandardError();

    _error = true;
    emit error(MarkdownDoc::ProcessorFailed);
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
