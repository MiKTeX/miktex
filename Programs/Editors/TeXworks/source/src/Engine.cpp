/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2018-2020  Jonathan Kew, Stefan Löffler

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	For links to further information, or to contact the authors,
	see <http://www.tug.org/texworks/>.
*/

#include "Engine.h"
#include "TWApp.h"

#include <QDir>

Engine::Engine(const QString& name, const QString& program, const QStringList & arguments, bool showPdf)
	: _name(name), _program(program), _arguments(arguments), _showPdf(showPdf)
{
}

Engine::Engine(const Engine& orig)
	: _name(orig._name), _program(orig._program), _arguments(orig._arguments), _showPdf(orig._showPdf)
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
	return TWApp::instance()->getBinaryPaths();
}

// static
QString Engine::programPath(const QString & prog)
{
	if (prog.isEmpty()) return QString();

	return TWApp::instance()->findProgram(prog, binPaths());
}


QProcess * Engine::run(const QFileInfo & input, QObject * parent /* = nullptr */)
{
	QString exeFilePath = programPath(program());
	if (exeFilePath.isEmpty())
		return nullptr;

	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
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
	env.insert(QStringLiteral("TEXEDIT"), QStringLiteral("%1 --position=%d %s").arg(QCoreApplication::applicationFilePath()));

	// MiKTeX apparently uses it's own variable
	env.insert(QStringLiteral("MIKTEX_EDITOR"), QStringLiteral("%1 --position=%l \"%f\"").arg(QCoreApplication::applicationFilePath()));
#endif

#if defined(Q_OS_DARWIN)
	// On Mac OS X, append the path to the typesetting tool to the PATH
	// environment variable.
	// In recent versions of Mac OS X and Qt, GUI applications (like Tw) get
	// different values for PATH than console applications (because .bashrc etc.
	// don't get parsed). Typesetting tools still run correctly (as they are
	// invoked with a full path), but if they in turn try to run other tools
	// (like epstopdf) without full path, the process will fail.
	// Appending the path to the typesetting tool to PATH acts as a fallback and
	// implicitly assumes that the tool itself and all tools it relies on are in
	// the same (standard) location.
	QStringList envPaths = env.value(QStringLiteral("PATH")).split(QStringLiteral(PATH_LIST_SEP));
	envPaths.append(QFileInfo(exeFilePath).dir().absolutePath());
	env.insert(QStringLiteral("PATH"), envPaths.join(QStringLiteral(PATH_LIST_SEP)));
#endif

	QStringList args = arguments();

#if !defined(MIKTEX)
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
#endif

	args.replaceInStrings(QString::fromLatin1("$synctexoption"), QString::fromLatin1("-synctex=1"));
	args.replaceInStrings(QString::fromLatin1("$fullname"), input.fileName());
	args.replaceInStrings(QString::fromLatin1("$basename"), input.completeBaseName());
	args.replaceInStrings(QString::fromLatin1("$suffix"), input.suffix());
	args.replaceInStrings(QString::fromLatin1("$directory"), input.absoluteDir().absolutePath());

	process->setEnvironment(env.toStringList());
	process->setProcessChannelMode(QProcess::MergedChannels);

	process->start(exeFilePath, args);

	return process;
}
