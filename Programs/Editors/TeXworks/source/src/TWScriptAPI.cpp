/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2015  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TWScriptAPI.h"
#include "TWSystemCmd.h"
#include "TWUtils.h"
#include "TWApp.h"

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

TWScriptAPI::TWScriptAPI(TWScript* script, QObject* twapp, QObject* ctx, QVariant& res)
	: m_script(script),
	  m_app(twapp),
	  m_target(ctx),
	  m_result(res)
{
}
	
void TWScriptAPI::SetResult(const QVariant& rval)
{
	m_result = rval;
}

int TWScriptAPI::strlen(const QString& str) const
{
	return str.length();
}

QString TWScriptAPI::platform() const
{
#if defined(Q_OS_DARWIN)
	return QString("MacOSX");
#elif defined(Q_OS_WIN)
	return QString("Windows");
#elif defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
	return QString("X11");
#else
	return QString("unknown");
#endif
}

int TWScriptAPI::information(QWidget* parent,
				const QString& title, const QString& text,
				int buttons,
				int defaultButton)
{
	return (int)QMessageBox::information(parent, title, text,
										 (QMessageBox::StandardButtons)buttons,
										 (QMessageBox::StandardButton)defaultButton);
}

int TWScriptAPI::question(QWidget* parent,
			 const QString& title, const QString& text,
			 int buttons,
			 int defaultButton)
{
	return (int)QMessageBox::question(parent, title, text,
									  (QMessageBox::StandardButtons)buttons,
									  (QMessageBox::StandardButton)defaultButton);
}

int TWScriptAPI::warning(QWidget* parent,
			const QString& title, const QString& text,
			int buttons,
			int defaultButton)
{
	return (int)QMessageBox::warning(parent, title, text,
									 (QMessageBox::StandardButtons)buttons,
									 (QMessageBox::StandardButton)defaultButton);
}

int TWScriptAPI::critical(QWidget* parent,
			 const QString& title, const QString& text,
			 int buttons,
			 int defaultButton)
{
	return (int)QMessageBox::critical(parent, title, text,
									  (QMessageBox::StandardButtons)buttons,
									  (QMessageBox::StandardButton)defaultButton);
}

QVariant TWScriptAPI::getInt(QWidget* parent, const QString& title, const QString& label,
				int value, int min, int max, int step)
{
	bool ok;
#if QT_VERSION >= 0x040500
	int i = QInputDialog::getInt(parent, title, label, value, min, max, step, &ok);
#else
	int i = QInputDialog::getInteger(parent, title, label, value, min, max, step, &ok);
#endif
	return ok ? QVariant(i) : QVariant();
}

QVariant TWScriptAPI::getDouble(QWidget* parent, const QString& title, const QString& label,
				   double value, double min, double max, int decimals)
{
	bool ok;
	double d = QInputDialog::getDouble(parent, title, label, value, min, max, decimals, &ok);
	return ok ? QVariant(d) : QVariant();
}

QVariant TWScriptAPI::getItem(QWidget* parent, const QString& title, const QString& label,
				 const QStringList& items, int current, bool editable)
{
	bool ok;
	QString s = QInputDialog::getItem(parent, title, label, items, current, editable, &ok);
	return ok ? QVariant(s) : QVariant();
}

QVariant TWScriptAPI::getText(QWidget* parent, const QString& title, const QString& label,
				 const QString& text)
{
	bool ok;
	QString s = QInputDialog::getText(parent, title, label, QLineEdit::Normal, text, &ok);
	return ok ? QVariant(s) : QVariant();
}
	
void TWScriptAPI::yield()
{
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}
	
QWidget * TWScriptAPI::progressDialog(QWidget * parent)
{
	QProgressDialog * dlg = new QProgressDialog(parent);
	connect(this, SIGNAL(destroyed(QObject*)), dlg, SLOT(deleteLater()));
	dlg->setCancelButton(NULL);
	dlg->show();
	return dlg;
}
	
QWidget * TWScriptAPI::createUIFromString(const QString& uiSpec, QWidget * parent)
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

QWidget * TWScriptAPI::createUI(const QString& filename, QWidget * parent)
{
	QFileInfo fi(QFileInfo(m_script->getFilename()).absoluteDir(), filename);
	if (!fi.isReadable())
		return NULL;
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
	
QWidget * TWScriptAPI::findChildWidget(QWidget* parent, const QString& name)
{
	QWidget* child = parent->findChild<QWidget*>(name);
	return child;
}
	
bool TWScriptAPI::makeConnection(QObject* sender, const QString& signal, QObject* receiver, const QString& slot)
{
	return QObject::connect(sender, QString("2%1").arg(signal).toUtf8().data(),
							receiver, QString("1%1").arg(slot).toUtf8().data());
}


QMap<QString, QVariant> TWScriptAPI::system(const QString& cmdline, bool waitForResult)
{
	QMap<QString, QVariant> retVal;

	retVal["status"] = SystemAccess_PermissionDenied;
	retVal["result"] = QVariant();
	retVal["message"] = QVariant();
	retVal["output"] = QVariant();

	// Paranoia
	if (!m_script) {
		retVal["message"] = tr("Internal error");
		return retVal;
	}

	if (m_script->mayExecuteSystemCommand(cmdline, m_target)) {
		TWSystemCmd *process = new TWSystemCmd(this, waitForResult, !waitForResult);
		if (waitForResult) {
			process->setProcessChannelMode(QProcess::MergedChannels);
			process->start(cmdline);
			// make sure events (in particular GUI update events that should
			// inform the user of the progress) are processed before we make a
			// call that possibly blocks for a considerable amount of time
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
			if (!process->waitForStarted()) {
				retVal["status"] = SystemAccess_Failed;
				retVal["message"] = tr("Failed to execute system command: %1").arg(cmdline);
				process->deleteLater();
				return retVal;
			}
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
			if (!process->waitForFinished()) {
				retVal["status"] = SystemAccess_Failed;
				retVal["result"] = process->exitCode();
				retVal["output"] = process->getResult();
				retVal["message"] = tr("Error executing system command: %1").arg(cmdline);
				process->deleteLater();
				return retVal;
			}
			retVal["status"] = SystemAccess_OK;
			retVal["result"] = process->exitCode();
			retVal["output"] = process->getResult();
			process->deleteLater();
		}
		else {
			process->closeReadChannel(QProcess::StandardOutput);
			process->closeReadChannel(QProcess::StandardError);
			process->start(cmdline);
			retVal["status"] = SystemAccess_OK;
		}
	}
	else
		retVal["message"] = tr("System command execution is disabled (see Preferences)");
	return retVal;
}

QMap<QString, QVariant> TWScriptAPI::launchFile(const QString& fileName) const
{
	QFileInfo finfo(fileName);
	QMap<QString, QVariant> retVal;
	
	retVal["status"] = SystemAccess_PermissionDenied;
	retVal["message"] = QVariant();

	// it's OK to "launch" a directory, as that doesn't normally execute anything
	if (finfo.isDir() || (m_script && m_script->mayExecuteSystemCommand(fileName, m_target))) {
		if (QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)))
			retVal["status"] = SystemAccess_OK;
		else {
			retVal["status"] = SystemAccess_Failed;
			retVal["message"] = tr("\"%1\" could not be opened.").arg(fileName);
		}
	}
	else
		retVal["message"] = tr("System command execution is disabled (see Preferences)");
	return retVal;
}

//Q_INVOKABLE
int TWScriptAPI::writeFile(const QString& filename, const QString& content) const
{
	// relative paths are taken to be relative to the folder containing the
	// executing script's file
	QFileInfo fi(filename);
	QDir scriptDir(QFileInfo(m_script->getFilename()).dir());
	QString path = scriptDir.absoluteFilePath(filename);

	if (!m_script->mayWriteFile(path, m_target))
		return TWScriptAPI::SystemAccess_PermissionDenied;
	
	QFile fout(path);
	qint64 numBytes = -1;
	
	if (!fout.open(QIODevice::WriteOnly | QIODevice::Text))
		return TWScriptAPI::SystemAccess_Failed;
	
	numBytes = fout.write(content.toUtf8());
	fout.close();

	return (numBytes < 0 ? TWScriptAPI::SystemAccess_Failed : TWScriptAPI::SystemAccess_OK);
}

//Q_INVOKABLE
QMap<QString, QVariant> TWScriptAPI::readFile(const QString& filename) const
{
	// relative paths are taken to be relative to the folder containing the
	// executing script's file
	QMap<QString, QVariant> retVal;
	
	retVal["status"] = SystemAccess_PermissionDenied;
	retVal["result"] = QVariant();
	retVal["message"] = QVariant();

	QFileInfo fi(filename);
	QDir scriptDir(QFileInfo(m_script->getFilename()).dir());
	QString path = scriptDir.absoluteFilePath(filename);

	if (!m_script->mayReadFile(path, m_target)) {
		retVal["message"] = tr("Reading all files is disabled (see Preferences)");
		retVal["status"] = TWScriptAPI::SystemAccess_PermissionDenied;
		return retVal;
	}
	
	QFile fin(path);
	
	if (!fin.open(QIODevice::ReadOnly | QIODevice::Text)) {
		retVal["message"] = tr("The file \"%1\" could not be opened for reading").arg(path);
		retVal["status"] = TWScriptAPI::SystemAccess_Failed;
		return retVal;
	}
	
	// with readAll, there's no way to detect an error during the actual read
	retVal["result"] = QString::fromUtf8(fin.readAll().constData());
	retVal["status"] = TWScriptAPI::SystemAccess_OK;
	fin.close();

	return retVal;
}

int TWScriptAPI::fileExists(const QString& filename) const
{
	QFileInfo fi(filename);
	QDir scriptDir(QFileInfo(m_script->getFilename()).dir());
	QString path = scriptDir.absoluteFilePath(filename);

	if (!m_script->mayReadFile(path, m_target))
		return SystemAccess_PermissionDenied;
	return (QFileInfo(path).exists() ? SystemAccess_OK : SystemAccess_Failed);
}

//////////////// Wrapper around selected TWUtils functions ////////////////
Q_INVOKABLE
QMap<QString, QVariant> TWScriptAPI::getDictionaryList(const bool forceReload /* = false */)
{
	QMap<QString, QVariant> retVal;
	const QHash<QString, QString> * h = TWUtils::getDictionaryList(forceReload);
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
QList<QVariant> TWScriptAPI::getEngineList() const
{
	QList<QVariant> retVal;
	const QList<Engine> engines = TWApp::instance()->getEngineList();

	foreach (const Engine& e, engines) {
		QMap<QString, QVariant> s;
		s["name"] = e.name();
		retVal.append(s);
	}

	return retVal;
}

