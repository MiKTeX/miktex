#include "Engine.h"
#include "TWApp.h"

Engine::Engine()
	: QObject()
	, _showPdf(false)
{
}

Engine::Engine(const QString& name, const QString& program, const QStringList arguments, bool showPdf)
	: QObject(), _name(name), _program(program), _arguments(arguments), _showPdf(showPdf)
{
}

Engine::Engine(const Engine& orig)
	: QObject(), _name(orig._name), _program(orig._program), _arguments(orig._arguments), _showPdf(orig._showPdf)
{
}

Engine& Engine::operator=(const Engine& rhs)
{
	_name = rhs._name;
	_program = rhs._program;
	_arguments = rhs._arguments;
	_showPdf = rhs._showPdf;
	return *this;
}

const QString Engine::name() const
{
	return _name;
}

const QString Engine::program() const
{
	return _program;
}

const QStringList Engine::arguments() const
{
	return _arguments;
}

bool Engine::showPdf() const
{
	return _showPdf;
}

void Engine::setName(const QString& name)
{
	_name = name;
}

void Engine::setProgram(const QString& program)
{
	_program = program;
}

void Engine::setArguments(const QStringList& arguments)
{
	_arguments = arguments;
}

void Engine::setShowPdf(bool showPdf)
{
	_showPdf = showPdf;
}

bool Engine::isAvailable() const
{
	return !(programPath(program()).isEmpty());
}

//static
QStringList Engine::binPaths()
{
	QStringList env = QProcess::systemEnvironment();
	return TWApp::instance()->getBinaryPaths(env);
}

// static
QString Engine::programPath(const QString & prog)
{
	if (prog.isEmpty()) return QString();

	return TWApp::instance()->findProgram(prog, binPaths());
}


QProcess * Engine::run(QFileInfo input, QObject * parent /* = nullptr */)
{
	QString exeFilePath = programPath(program());
	if (exeFilePath.isEmpty())
		return nullptr;

	QStringList env = QProcess::systemEnvironment();
	QProcess * process = new QProcess(parent);

	QString workingDir = input.canonicalPath();
#if defined(Q_OS_WIN)
	// files in the root directory of the current drive have to be handled specially
	// because QFileInfo::canonicalPath() returns a path without trailing slash
	// (i.e., a bare drive letter)
	if (workingDir.length() == 2 && workingDir.endsWith(QChar::fromLatin1(':')))
		workingDir.append(QChar::fromLatin1('/'));
#endif
	process->setWorkingDirectory(workingDir);


#if !defined(Q_OS_DARWIN) // not supported on OS X yet :(
	// Add a (customized) TEXEDIT environment variable
	env << QString::fromLatin1("TEXEDIT=%1 --position=%d %s").arg(QCoreApplication::applicationFilePath());

	#if defined(Q_OS_WIN) // MiKTeX apparently uses it's own variable
	env << QString::fromLatin1("MIKTEX_EDITOR=%1 --position=%l \"%f\"").arg(QCoreApplication::applicationFilePath());
	#endif
#endif

	QStringList args = arguments();

	// for old MikTeX versions: delete $synctexoption if it causes an error
	static bool checkedForSynctex = false;
	static bool synctexSupported = true;
	if (!checkedForSynctex) {
		QString pdftex = programPath(QString::fromLatin1("pdftex"));
		if (!pdftex.isEmpty()) {
			int result = QProcess::execute(pdftex, QStringList() << QString::fromLatin1("-synctex=1") << QString::fromLatin1("-version"));
			synctexSupported = (result == 0);
		}
		checkedForSynctex = true;
	}
	if (!synctexSupported)
		args.removeAll(QString::fromLatin1("$synctexoption"));

	args.replaceInStrings(QString::fromLatin1("$synctexoption"), QString::fromLatin1("-synctex=1"));
	args.replaceInStrings(QString::fromLatin1("$fullname"), input.fileName());
	args.replaceInStrings(QString::fromLatin1("$basename"), input.completeBaseName());
	args.replaceInStrings(QString::fromLatin1("$suffix"), input.suffix());
	args.replaceInStrings(QString::fromLatin1("$directory"), input.absoluteDir().absolutePath());

	process->setEnvironment(env);
	process->setProcessChannelMode(QProcess::MergedChannels);

	process->start(exeFilePath, args);

	return process;
}
