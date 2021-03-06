/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2020  Stefan Löffler

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

#ifndef ScriptAPIInterface_H
#define ScriptAPIInterface_H

#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QVariant>

namespace Tw {
namespace Scripting {

class ScriptAPIInterface
{
public:
	enum SystemAccessResult {
		SystemAccess_OK = 0,
		SystemAccess_Failed,
		SystemAccess_PermissionDenied
	};

	virtual ~ScriptAPIInterface() = default;

	virtual QObject* clone() const = 0;

	virtual QObject* self() = 0;

	virtual QObject* GetApp() = 0;
	virtual QObject* GetTarget() = 0;
	virtual QObject* GetScript() = 0;
	virtual QVariant& GetResult() = 0;

	virtual void SetResult(const QVariant& rval) = 0;


	// provide utility functions for scripts, implemented as methods on the TW object

	// length of a string in UTF-16 code units, useful if script language uses a different encoding form
	virtual int strlen(const QString& str) const = 0;

	// return the host platform name
	virtual QString platform() const = 0;

	// return the Qt version Tw was built against (0xMMNNPP)
	virtual int getQtVersion() const = 0;

	// System access
	// for script access to arbitrary commands
	// Returned is a map with the fields:
	// - "status" => one of SystemAccessResult
	// - "result" => the return code of the command
	// - "message" => warning/error message
	// - "output" => the output of the command
	virtual QMap<QString, QVariant> system(const QString& cmdline, bool waitForResult = true) = 0;

	// launch file from the desktop with default app
	// Returned is a map with the fields:
	// - "status" => one of SystemAccessResult
	// - "message" => warning/error message
	// Note: SystemAccess_OK is no guarantee the file was actually opened, as
	//       error reporting on this is system dependent
	virtual QMap<QString, QVariant> launchFile(const QString& fileName) const = 0;

	// Return type is one of SystemAccessResult
	// Content is written in text-mode in utf8 encoding
	virtual int writeFile(const QString& filename, const QString& content) const = 0;

	// Returned is a map with the fields:
	// - "status" => one of SystemAccessResult
	// - "result" => content of file
	// - "message" => warning/error message
	// Content is read in text-mode in utf8 encoding
	virtual QMap<QString, QVariant> readFile(const QString& filename) const = 0;

	// Returns one of one of SystemAccessResult
	// Can be (ab)used to check folders, too
	virtual int fileExists(const QString& filename) const = 0;

	// QMessageBox functions to display alerts
	virtual int information(QWidget* parent,
					const QString& title, const QString& text,
					int buttons = QMessageBox::Ok,
					int defaultButton = QMessageBox::NoButton) = 0;
	virtual int question(QWidget* parent,
				 const QString& title, const QString& text,
				 int buttons = QMessageBox::Ok,
				 int defaultButton = QMessageBox::NoButton) = 0;
	virtual int warning(QWidget* parent,
				const QString& title, const QString& text,
				int buttons = QMessageBox::Ok,
				int defaultButton = QMessageBox::NoButton) = 0;
	virtual int critical(QWidget* parent,
				 const QString& title, const QString& text,
				 int buttons = QMessageBox::Ok,
				 int defaultButton = QMessageBox::NoButton) = 0;

	// QInputDialog functions
	// These return QVariant rather than simple types, so that they can return null
	// to indicate that the dialog was cancelled.
	virtual QVariant getInt(QWidget* parent, const QString& title, const QString& label,
					int value = 0, int min = -2147483647, int max = 2147483647, int step = 1) = 0;
	virtual QVariant getDouble(QWidget* parent, const QString& title, const QString& label,
					   double value = 0, double min = -2147483647, double max = 2147483647, int decimals = 1) = 0;
	virtual QVariant getItem(QWidget* parent, const QString& title, const QString& label,
					 const QStringList& items, int current = 0, bool editable = true) = 0;
	virtual QVariant getText(QWidget* parent, const QString& title, const QString& label,
					 const QString& text = QString()) = 0;

	virtual void yield() = 0;

	// Allow script to create a QProgressDialog
	virtual QWidget * progressDialog(QWidget * parent) = 0;

	// functions to create windows from .ui data or files using QUiLoader
	virtual QWidget * createUIFromString(const QString& uiSpec, QWidget * parent = nullptr) = 0;

	virtual QWidget * createUI(const QString& filename, QWidget * parent = nullptr) = 0;

	// to find children of a widget
	virtual QWidget * findChildWidget(QWidget* parent, const QString& name) = 0;

	// to make connections among widgets (or other objects)
	virtual bool makeConnection(QObject* sender, const QString& signal, QObject* receiver, const QString& slot) = 0;

	//////////////// Wrapper around selected TWUtils functions ////////////////
	// Returns a map of the type "language code => array(filenames)"
	// "filenames" are paths to *.dic files associated with the respective
	// language.
	// Note: forceReload only reloads the list of available dictionaries; it
	// doesn't actually reinitialize the spell checker
	virtual QMap<QString, QVariant> getDictionaryList(const bool forceReload = false) = 0;
	//////////////// Wrapper around selected TWUtils functions ////////////////

	// Wrapper around TWApp::getEngineList()
	// Currently, only the name is returned
	virtual QList<QVariant> getEngineList() const = 0;

	virtual bool mayExecuteSystemCommand(const QString& cmd, QObject * context) const = 0;
	virtual bool mayWriteFile(const QString& filename, QObject * context) const = 0;
	virtual bool mayReadFile(const QString& filename, QObject * context) const = 0;
};

} // namespace Scripting
} // namespace Tw

#endif /* ScriptAPIInterface_H */
