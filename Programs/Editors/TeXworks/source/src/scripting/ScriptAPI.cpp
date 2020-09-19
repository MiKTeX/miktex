/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "scripting/ScriptAPI.h"
#include "TWSystemCmd.h"
#include "TWUtils.h"
#include "TWApp.h"
#include "Settings.h"
#include "Engine.h"
#include "document/SpellChecker.h"
#include "scripting/Script.h"
#include "DefaultPrefs.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QUiLoader>
#include <QBuffer>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>

namespace Tw {
namespace Scripting {

ScriptAPI::ScriptAPI(Script* script, QObject* twapp, QObject* ctx, QVariant& res)
	: m_script(script),
	  m_app(twapp),
	  m_target(ctx),
	  m_result(res)
{
}

QObject* ScriptAPI::GetScript()
{
	return m_script;
}

void ScriptAPI::SetResult(const QVariant& rval)
{
	m_result = rval;
}

int ScriptAPI::strlen(const QString& str) const
{
	return str.length();
}

QString ScriptAPI::platform() const
{
#if defined(Q_OS_DARWIN)
	return QString::fromLatin1("MacOSX");
#elif defined(Q_OS_WIN)
	return QString::fromLatin1("Windows");
#elif defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
	return QString::fromLatin1("X11");
#else
	return QString::fromLatin1("unknown");
#endif
}

int ScriptAPI::information(QWidget* parent,
				const QString& title, const QString& text,
				int buttons,
				int defaultButton)
{
	return static_cast<int>(QMessageBox::information(parent, title, text,
										 static_cast<QMessageBox::StandardButtons>(buttons),
										 static_cast<QMessageBox::StandardButton>(defaultButton)));
}

int ScriptAPI::question(QWidget* parent,
			 const QString& title, const QString& text,
			 int buttons,
			 int defaultButton)
{
	return static_cast<int>(QMessageBox::question(parent, title, text,
									  static_cast<QMessageBox::StandardButtons>(buttons),
									  static_cast<QMessageBox::StandardButton>(defaultButton)));
}

int ScriptAPI::warning(QWidget* parent,
			const QString& title, const QString& text,
			int buttons,
			int defaultButton)
{
	return static_cast<int>(QMessageBox::warning(parent, title, text,
									 static_cast<QMessageBox::StandardButtons>(buttons),
									 static_cast<QMessageBox::StandardButton>(defaultButton)));
}

int ScriptAPI::critical(QWidget* parent,
			 const QString& title, const QString& text,
			 int buttons,
			 int defaultButton)
{
	return static_cast<int>(QMessageBox::critical(parent, title, text,
									  static_cast<QMessageBox::StandardButtons>(buttons),
									  static_cast<QMessageBox::StandardButton>(defaultButton)));
}

QVariant ScriptAPI::getInt(QWidget* parent, const QString& title, const QString& label,
				int value, int min, int max, int step)
{
	bool ok;
	int i = QInputDialog::getInt(parent, title, label, value, min, max, step, &ok);
	return ok ? QVariant(i) : QVariant();
}

QVariant ScriptAPI::getDouble(QWidget* parent, const QString& title, const QString& label,
				   double value, double min, double max, int decimals)
{
	bool ok;
	double d = QInputDialog::getDouble(parent, title, label, value, min, max, decimals, &ok);
	return ok ? QVariant(d) : QVariant();
}

QVariant ScriptAPI::getItem(QWidget* parent, const QString& title, const QString& label,
				 const QStringList& items, int current, bool editable)
{
	bool ok;
	QString s = QInputDialog::getItem(parent, title, label, items, current, editable, &ok);
	return ok ? QVariant(s) : QVariant();
}

QVariant ScriptAPI::getText(QWidget* parent, const QString& title, const QString& label,
				 const QString& text)
{
	bool ok;
	QString s = QInputDialog::getText(parent, title, label, QLineEdit::Normal, text, &ok);
	return ok ? QVariant(s) : QVariant();
}
	
void ScriptAPI::yield()
{
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}
	
QWidget * ScriptAPI::progressDialog(QWidget * parent)
{
	QProgressDialog * dlg = new QProgressDialog(parent);
	connect(this, SIGNAL(destroyed(QObject*)), dlg, SLOT(deleteLater()));
	dlg->setCancelButton(nullptr);
	dlg->show();
	return dlg;
}
	
QWidget * ScriptAPI::createUIFromString(const QString& uiSpec, QWidget * parent)
{
	QByteArray ba(uiSpec.toUtf8());
	QBuffer buffer(&ba);
	QUiLoader loader;
	QWidget *widget = loader.load(&buffer, parent);
	if (widget) {
		// ensure that the window is app-modal regardless of what flags might be set
		//! \TODO revisit this when we get asynchronous scripting
		widget->setWindowModality(Qt::ApplicationModal);
		widget->show();
	}
	return widget;
}

QWidget * ScriptAPI::createUI(const QString& filename, QWidget * parent)
{
	QFileInfo fi(QFileInfo(m_script->getFilename()).absoluteDir(), filename);
	if (!fi.isReadable())
		return nullptr;
	QFile file(fi.canonicalFilePath());
	QUiLoader loader;
	QWidget *widget = loader.load(&file, parent);
	if (widget) {
		// ensure that the window is app-modal regardless of what flags might be set
		//! \TODO revisit this when we get asynchronous scripting
		widget->setWindowModality(Qt::ApplicationModal);
		widget->show();
	}
	return widget;
}
	
QWidget * ScriptAPI::findChildWidget(QWidget* parent, const QString& name)
{
	QWidget* child = parent->findChild<QWidget*>(name);
	return child;
}
	
bool ScriptAPI::makeConnection(QObject* sender, const QString& signal, QObject* receiver, const QString& slot)
{
	return QObject::connect(sender, QString::fromLatin1("2%1").arg(signal).toUtf8().data(),
	                        receiver, QString::fromLatin1("1%1").arg(slot).toUtf8().data());
}


QMap<QString, QVariant> ScriptAPI::system(const QString& cmdline, bool waitForResult)
{
	QMap<QString, QVariant> retVal;

	retVal[QString::fromLatin1("status")] = SystemAccess_PermissionDenied;
	retVal[QString::fromLatin1("result")] = QVariant();
	retVal[QString::fromLatin1("message")] = QVariant();
	retVal[QString::fromLatin1("output")] = QVariant();

	// Paranoia
	if (!m_script) {
		retVal[QString::fromLatin1("message")] = tr("Internal error");
		return retVal;
	}

	if (mayExecuteSystemCommand(cmdline, m_target)) {
		TWSystemCmd *process = new TWSystemCmd(this, waitForResult, !waitForResult);
		if (waitForResult) {
			process->setProcessChannelMode(QProcess::MergedChannels);
			process->start(cmdline);
			// make sure events (in particular GUI update events that should
			// inform the user of the progress) are processed before we make a
			// call that possibly blocks for a considerable amount of time
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
			if (!process->waitForStarted()) {
				retVal[QString::fromLatin1("status")] = SystemAccess_Failed;
				retVal[QString::fromLatin1("message")] = tr("Failed to execute system command: %1").arg(cmdline);
				process->deleteLater();
				return retVal;
			}
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
			if (!process->waitForFinished()) {
				retVal[QString::fromLatin1("status")] = SystemAccess_Failed;
				retVal[QString::fromLatin1("result")] = process->exitCode();
				retVal[QString::fromLatin1("output")] = process->getResult();
				retVal[QString::fromLatin1("message")] = tr("Error executing system command: %1").arg(cmdline);
				process->deleteLater();
				return retVal;
			}
			retVal[QString::fromLatin1("status")] = SystemAccess_OK;
			retVal[QString::fromLatin1("result")] = process->exitCode();
			retVal[QString::fromLatin1("output")] = process->getResult();
			process->deleteLater();
		}
		else {
			process->closeReadChannel(QProcess::StandardOutput);
			process->closeReadChannel(QProcess::StandardError);
			process->start(cmdline);
			retVal[QString::fromLatin1("status")] = SystemAccess_OK;
		}
	}
	else
		retVal[QString::fromLatin1("message")] = tr("System command execution is disabled (see Preferences)");
	return retVal;
}

QMap<QString, QVariant> ScriptAPI::launchFile(const QString& fileName) const
{
	QFileInfo finfo(fileName);
	QMap<QString, QVariant> retVal;
	
	retVal[QString::fromLatin1("status")] = SystemAccess_PermissionDenied;
	retVal[QString::fromLatin1("message")] = QVariant();

	// it's OK to "launch" a directory, as that doesn't normally execute anything
	if (finfo.isDir() || mayExecuteSystemCommand(fileName, m_target)) {
		if (QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)))
			retVal[QString::fromLatin1("status")] = SystemAccess_OK;
		else {
			retVal[QString::fromLatin1("status")] = SystemAccess_Failed;
			retVal[QString::fromLatin1("message")] = tr("\"%1\" could not be opened.").arg(fileName);
		}
	}
	else
		retVal[QString::fromLatin1("message")] = tr("System command execution is disabled (see Preferences)");
	return retVal;
}

//Q_INVOKABLE
int ScriptAPI::writeFile(const QString& filename, const QString& content) const
{
	// relative paths are taken to be relative to the folder containing the
	// executing script's file
	QDir scriptDir(QFileInfo(m_script->getFilename()).dir());
	QString path = scriptDir.absoluteFilePath(filename);

	if (!mayWriteFile(path, m_target))
		return ScriptAPI::SystemAccess_PermissionDenied;
	
	QFile fout(path);
	qint64 numBytes = -1;
	
	if (!fout.open(QIODevice::WriteOnly | QIODevice::Text))
		return ScriptAPI::SystemAccess_Failed;
	
	numBytes = fout.write(content.toUtf8());
	fout.close();

	return (numBytes < 0 ? ScriptAPI::SystemAccess_Failed : ScriptAPI::SystemAccess_OK);
}

//Q_INVOKABLE
QMap<QString, QVariant> ScriptAPI::readFile(const QString& filename) const
{
	// relative paths are taken to be relative to the folder containing the
	// executing script's file
	QMap<QString, QVariant> retVal;
	
	retVal[QString::fromLatin1("status")] = SystemAccess_PermissionDenied;
	retVal[QString::fromLatin1("result")] = QVariant();
	retVal[QString::fromLatin1("message")] = QVariant();

	QDir scriptDir(QFileInfo(m_script->getFilename()).dir());
	QString path = scriptDir.absoluteFilePath(filename);

	if (!mayReadFile(path, m_target)) {
		retVal[QString::fromLatin1("message")] = tr("Reading all files is disabled (see Preferences)");
		retVal[QString::fromLatin1("status")] = ScriptAPI::SystemAccess_PermissionDenied;
		return retVal;
	}
	
	QFile fin(path);
	
	if (!fin.open(QIODevice::ReadOnly | QIODevice::Text)) {
		retVal[QString::fromLatin1("message")] = tr("The file \"%1\" could not be opened for reading").arg(path);
		retVal[QString::fromLatin1("status")] = ScriptAPI::SystemAccess_Failed;
		return retVal;
	}
	
	// with readAll, there's no way to detect an error during the actual read
	retVal[QString::fromLatin1("result")] = QString::fromUtf8(fin.readAll().constData());
	retVal[QString::fromLatin1("status")] = ScriptAPI::SystemAccess_OK;
	fin.close();

	return retVal;
}

int ScriptAPI::fileExists(const QString& filename) const
{
	QDir scriptDir(QFileInfo(m_script->getFilename()).dir());
	QString path = scriptDir.absoluteFilePath(filename);

	if (!mayReadFile(path, m_target))
		return SystemAccess_PermissionDenied;
	return (QFileInfo(path).exists() ? SystemAccess_OK : SystemAccess_Failed);
}

//////////////// Wrapper around selected TWUtils functions ////////////////
Q_INVOKABLE
QMap<QString, QVariant> ScriptAPI::getDictionaryList(const bool forceReload /* = false */)
{
	QMap<QString, QVariant> retVal;
	const QHash<QString, QString> * h = Tw::Document::SpellChecker::getDictionaryList(forceReload);
	for (QHash<QString, QString>::const_iterator it = h->begin(); it != h->end(); ++it) {
		if (!retVal.contains(it.value()))
			retVal[it.value()] = QVariant::fromValue((QList<QVariant>() << it.key()));
		else
			retVal[it.value()] = (retVal[it.value()].toList() << it.key());
	}
	
	return retVal;
}
//////////////// Wrapper around selected TWUtils functions ////////////////


// Wrapper around TWApp::getEngineList()
Q_INVOKABLE
QList<QVariant> ScriptAPI::getEngineList() const
{
	QList<QVariant> retVal;
	const QList<Engine> engines = TWApp::instance()->getEngineList();

	foreach (const Engine& e, engines) {
		QMap<QString, QVariant> s;
		s[QString::fromLatin1("name")] = e.name();
		retVal.append(s);
	}

	return retVal;
}

bool ScriptAPI::mayExecuteSystemCommand(const QString& cmd, QObject * context) const
{
	Q_UNUSED(cmd)
	Q_UNUSED(context)

	// cmd may be a true command line, or a single file/directory to run or open
	Tw::Settings settings;
	return settings.value(QString::fromLatin1("allowSystemCommands"), false).toBool();
}

bool ScriptAPI::mayWriteFile(const QString& filename, QObject * context) const
{
	Q_UNUSED(filename)
	Q_UNUSED(context)

	Tw::Settings settings;
	return settings.value(QString::fromLatin1("allowScriptFileWriting"), false).toBool();
}

bool ScriptAPI::mayReadFile(const QString& filename, QObject * context) const
{
	Tw::Settings settings;
	if (!m_script)
		return false;

	QDir scriptDir(QFileInfo(m_script->getFilename()).absoluteDir());
	QVariant targetFile;
	QDir targetDir;

	if (settings.value(QString::fromLatin1("allowScriptFileReading"), kDefault_AllowScriptFileReading).toBool())
		return true;

	// even if global reading is disallowed, some exceptions may apply
	QFileInfo fi(QDir::cleanPath(filename));

	// reading in subdirectories of the script file's directory is always allowed
	if (!scriptDir.relativeFilePath(fi.absolutePath()).startsWith(QLatin1String("..")))
		return true;

	if (context) {
		// reading subdirectories of the current file is always allowed
		targetFile = context->property("fileName");
		if (targetFile.isValid() && !targetFile.toString().isEmpty()) {
			targetDir = QFileInfo(targetFile.toString()).absoluteDir();
			if (!targetDir.relativeFilePath(fi.absolutePath()).startsWith(QLatin1String("..")))
				return true;
		}
		// reading subdirectories of the root file is always allowed
		targetFile = context->property("rootFileName");
		if (targetFile.isValid() && !targetFile.toString().isEmpty()) {
			targetDir = QFileInfo(targetFile.toString()).absoluteDir();
			if (!targetDir.relativeFilePath(fi.absolutePath()).startsWith(QLatin1String("..")))
				return true;
		}
	}

	return false;
}

} // namespace Scripting
} // namespace Tw
