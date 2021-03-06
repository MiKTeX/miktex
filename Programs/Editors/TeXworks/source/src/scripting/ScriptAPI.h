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

#ifndef ScriptAPI_H
#define ScriptAPI_H

#include "scripting/ScriptAPIInterface.h"

#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QVariant>

namespace Tw {
namespace Scripting {

class Script;

class ScriptAPI : public QObject, public ScriptAPIInterface
{
	Q_OBJECT

	Q_PROPERTY(QObject* app READ GetApp)
	Q_PROPERTY(QObject* target READ GetTarget)
	Q_PROPERTY(QVariant result READ GetResult WRITE SetResult)
	Q_PROPERTY(QObject * script READ GetScript)

public:
	ScriptAPI(Script* script, QObject* twapp, QObject* ctx, QVariant& res);

	QObject* clone() const override { return new ScriptAPI(m_script, m_app, m_target, m_result); }

	QObject* self() override { return this; }

public:
	QObject* GetApp() override { return m_app; }
	QObject* GetTarget() override { return m_target; }
	QObject* GetScript() override;
	QVariant& GetResult() override { return m_result; }

	void SetResult(const QVariant& rval) override;

	enum SystemAccessResult {
		SystemAccess_OK = 0,
		SystemAccess_Failed,
		SystemAccess_PermissionDenied
	};

	// provide utility functions for scripts, implemented as methods on the TW object

	// length of a string in UTF-16 code units, useful if script language uses a different encoding form
	Q_INVOKABLE
	int strlen(const QString& str) const override;

	// return the host platform name
	Q_INVOKABLE
	QString platform() const override;

	// return the Qt version Tw was built against (0xMMNNPP)
	Q_INVOKABLE
	int getQtVersion() const override { return QT_VERSION; }

	// System access
	// for script access to arbitrary commands
	// Returned is a map with the fields:
	// - "status" => one of SystemAccessResult
	// - "result" => the return code of the command
	// - "message" => warning/error message
	// - "output" => the output of the command
	Q_INVOKABLE
	QMap<QString, QVariant> system(const QString& cmdline, bool waitForResult = true) override;

	// launch file from the desktop with default app
	// Returned is a map with the fields:
	// - "status" => one of SystemAccessResult
	// - "message" => warning/error message
	// Note: SystemAccess_OK is no guarantee the file was actually opened, as
	//       error reporting on this is system dependent
	Q_INVOKABLE
	QMap<QString, QVariant> launchFile(const QString& fileName) const override;

	// Return type is one of SystemAccessResult
	// Content is written in text-mode in utf8 encoding
	Q_INVOKABLE
	int writeFile(const QString& filename, const QString& content) const override;

	// Returned is a map with the fields:
	// - "status" => one of SystemAccessResult
	// - "result" => content of file
	// - "message" => warning/error message
	// Content is read in text-mode in utf8 encoding
	Q_INVOKABLE
	QMap<QString, QVariant> readFile(const QString& filename) const override;

	// Returns one of one of SystemAccessResult
	// Can be (ab)used to check folders, too
	Q_INVOKABLE
	int fileExists(const QString& filename) const override;

	// QMessageBox functions to display alerts
	Q_INVOKABLE
	int information(QWidget* parent,
					const QString& title, const QString& text,
					int buttons = QMessageBox::Ok,
					int defaultButton = QMessageBox::NoButton) override;
	Q_INVOKABLE
	int question(QWidget* parent,
				 const QString& title, const QString& text,
				 int buttons = QMessageBox::Ok,
				 int defaultButton = QMessageBox::NoButton) override;
	Q_INVOKABLE
	int warning(QWidget* parent,
				const QString& title, const QString& text,
				int buttons = QMessageBox::Ok,
				int defaultButton = QMessageBox::NoButton) override;
	Q_INVOKABLE
	int critical(QWidget* parent,
				 const QString& title, const QString& text,
				 int buttons = QMessageBox::Ok,
				 int defaultButton = QMessageBox::NoButton) override;

	// QInputDialog functions
	// These return QVariant rather than simple types, so that they can return null
	// to indicate that the dialog was cancelled.
	Q_INVOKABLE
	QVariant getInt(QWidget* parent, const QString& title, const QString& label,
					int value = 0, int min = -2147483647, int max = 2147483647, int step = 1) override;
	Q_INVOKABLE
	QVariant getDouble(QWidget* parent, const QString& title, const QString& label,
					   double value = 0, double min = -2147483647, double max = 2147483647, int decimals = 1) override;
	Q_INVOKABLE
	QVariant getItem(QWidget* parent, const QString& title, const QString& label,
					 const QStringList& items, int current = 0, bool editable = true) override;
	Q_INVOKABLE
	QVariant getText(QWidget* parent, const QString& title, const QString& label,
					 const QString& text = QString()) override;

	Q_INVOKABLE
	void yield() override;

	// Allow script to create a QProgressDialog
	Q_INVOKABLE
	QWidget * progressDialog(QWidget * parent) override;

	// functions to create windows from .ui data or files using QUiLoader
	Q_INVOKABLE
	QWidget * createUIFromString(const QString& uiSpec, QWidget * parent = nullptr) override;

	Q_INVOKABLE
	QWidget * createUI(const QString& filename, QWidget * parent = nullptr) override;

	// to find children of a widget
	Q_INVOKABLE
	QWidget * findChildWidget(QWidget* parent, const QString& name) override;

	// to make connections among widgets (or other objects)
	Q_INVOKABLE
	bool makeConnection(QObject* sender, const QString& signal, QObject* receiver, const QString& slot) override;

	//////////////// Wrapper around selected TWUtils functions ////////////////
	// Returns a map of the type "language code => array(filenames)"
	// "filenames" are paths to *.dic files associated with the respective
	// language.
	// Note: forceReload only reloads the list of available dictionaries; it
	// doesn't actually reinitialize the spell checker
	Q_INVOKABLE
	QMap<QString, QVariant> getDictionaryList(const bool forceReload = false) override;
	//////////////// Wrapper around selected TWUtils functions ////////////////

	// Wrapper around TWApp::getEngineList()
	// Currently, only the name is returned
	Q_INVOKABLE
	QList<QVariant> getEngineList() const override;

	bool mayExecuteSystemCommand(const QString& cmd, QObject * context) const override;
	bool mayWriteFile(const QString& filename, QObject * context) const override;
	bool mayReadFile(const QString& filename, QObject * context) const override;

protected:
	Script* m_script;
	QObject* m_app;
	QObject* m_target;
	QVariant& m_result;
};

} // namespace Scripting
} // namespace Tw

#endif /* ScriptAPI_H */
