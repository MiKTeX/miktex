/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2015  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TWScript_H
#define TWScript_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDateTime>
#include <QKeySequence>
#include <QStringList>
#include <QVariant>
#include <QHash>
#include <QTextCodec>

class TWScriptLanguageInterface;
class TWScriptAPI;

/** \brief	Abstract base class for all Tw scripts
 *
 * \note This must be derived from QObject to enable interaction with e.g. menus
 */
class TWScript : public QObject
{
	Q_OBJECT
	
public:
	/** \brief	Types of scripts */
	enum ScriptType { 
		ScriptUnknown,		///< unknown or invalid script
		ScriptHook,			///< hook, i.e. a script that is called automatically when the execution reaches a certain point
		ScriptStandalone	///< standalone script, i.e. one that can be invoked by the user
	};

	Q_PROPERTY(QString fileName READ getFilename)
	Q_PROPERTY(QString title READ getTitle)
	Q_PROPERTY(QString description READ getDescription)
	Q_PROPERTY(QString author READ getAuthor)
	Q_PROPERTY(QString version READ getVersion)

	/** \brief	Destructor
	 *
	 * Does nothing
	 */
	virtual ~TWScript() { }
	
	/** \brief  Return the enabled/disabled status of the script
	 *
	 * \return  \c true if script is enabled, \c false if disabled
	 */
	bool isEnabled() const { return m_Enabled; }
	
	/** \brief  Set the enabled/disabled status of the script
	 *
	 * \param  enable   the new enabled status to set on the script object
	 */
	void setEnabled(bool enable) { m_Enabled = enable; }
	
	/** \brief	Determine if the file has changed on the disk since it was last parsed
	 *
	 * \return	\c true if it has changed, \c false otherwise
	 */
	bool hasChanged() const;
	
	/** \brief Parse the script header
	 *
	 * \note	This method must be implemented in derived classes.
	 * \see	doParseHeader(QString, QString, QString, bool)
	 * \see	doParseHeader(QStringList)
	 * \return	\c true if successful, \c false if not (e.g. because the file
	 * 			is no valid Tw script)
	 */
	virtual bool parseHeader() = 0;
	
	/** \brief	Get the type of the script
	 *
	 * \return	the script type
	 */
	ScriptType getType() const { return m_Type; }

	/** \brief	Get the filename of the script
	 *
	 * \return	the absolute filename
	 */
	const QString& getFilename() const { return m_Filename; }

	/** \brief	Get the title of the script
	 *
	 * Used e.g. for adding a script to a menu
	 * \return	the title
	 */
	const QString& getTitle() const { return m_Title; }

	/** \brief	Get the description of the script
	 *
	 * \return	the description
	 */
	const QString& getDescription() const { return m_Description; }

	/** \brief	Get the author's name
	 *
	 * \return	the author's name
	 */
	const QString& getAuthor() const { return m_Author; }

	/** \brief	Get the script version
	 *
	 * \note	This is <i>not</i> the version <i>required to run</i> the script.
	 * \return	the script version
	 */
	const QString& getVersion() const { return m_Version; }

	/** \brief	Get the name of the hook this script should be connected to (if any)
	 *
	 * \return	the name of the hook, if this is a hook script. An empty string
	 * 			otherwise
	 */
	const QString& getHook() const { return m_Hook; }

	/** \brief	Get the name of the context where this script applies
	 *
	 * \return	The name of the window class where this script should be available
	 *			(TeXDocument or PDFDocument), or empty if the script is universal
	 */
	const QString& getContext() const { return m_Context; }
	
	/** \brief	Get the shortcut of this script
	 *
	 * \note	This is only useful for standalone scripts.
	 * \return	the shortcut
	 */
	const QKeySequence& getKeySequence() const { return m_KeySequence; }
	
	const QObject * getScriptLanguagePlugin() const { return m_Plugin; }
	
	/** \brief Run the script (public method called from the TeXworks application).
	 *
	 * This method sets up the TW object that provides scripts with access to
	 * objects and methods within the application; then it calls the language-specific
	 * execute() method to actually run the script.
	 *
	 * \param	context	the object from which the script was called; typically
	 * 					a TeXDocument or PDFDocument instance
	 * \param	result	variable to receive the result of the script execution;
	 * 					in the case of an error, this typically contains an
	 * 					error description
	 * \return	\c true on success, \c false if an error occured
	 */
	bool run(QObject *context, QVariant& result);
	
	/** \brief Check if two scripts are the same
	 *
	 * \note	This method compares the file paths
	 * \param	s	the script to compare to this one
	 * \return	\c true if *this == s, \c false otherwise
	 */
	bool operator==(const TWScript& s) const { return QFileInfo(m_Filename) == QFileInfo(s.m_Filename); }

	Q_INVOKABLE void setGlobal(const QString& key, const QVariant& val);
	Q_INVOKABLE void unsetGlobal(const QString& key) { m_globals.remove(key); }
	Q_INVOKABLE bool hasGlobal(const QString& key) const { return m_globals.contains(key); }
	Q_INVOKABLE QVariant getGlobal(const QString& key) const { return m_globals[key]; }

	bool mayExecuteSystemCommand(const QString& cmd, QObject * context) const;
	bool mayWriteFile(const QString& filename, QObject * context) const;
	bool mayReadFile(const QString& filename, QObject * context) const;

protected:
	/** \brief	Constructor
	 *
	 * Initializes a script object from the given file.
	 * Does not invoke parseHeader(), so the script object may not actually be usable.
	 */
	TWScript(QObject * plugin, const QString& filename);

	/** \brief  Execute the actual script
	 *
	 * Pure virtual method, to be implemented by each concrete TWScript subclass.
	 * This is the method that actually execute the script.
	 *
	 * \param  tw  the "TW" object that provides the script with access to
	 *             .target, .app, .result properties
	 * \return     \c true on success, \c false if an error occurred.
	 */
	virtual bool execute(TWScriptAPI* tw) const = 0;
	
	enum ParseHeaderResult {
		ParseHeader_OK,
		ParseHeader_Failed,
		ParseHeader_CodecChanged
	};
	
	/** \brief	Convenience function to parse supported key:value pairs of the header
	 *
	 * Currently supported keys:
	 * - Title
	 * - Description
	 * - Author
	 * - Version
	 * - Script-Type
	 * - Hook
	 * - Shortcut
	 * - Context
	 *
	 * \param	lines	the lines containing unparsed key:value pairs (but
	 * 					without any language-specific comment characters)
	 * \return	\c true if a title and type were found, \c false otherwise
	 */
	TWScript::ParseHeaderResult doParseHeader(const QStringList & lines);

	/** \brief	Convenience function to parse text-based script files
	 *
	 * Opens the text file specified by m_Filename, reads the first comment
	 * block and passes it on to doParseHeader(QStringList).
	 * \warning	You normally don't want to mix \a beginComment/\a endComment with
	 * 			\a Comment. In this case, the routine requires each line to be
	 * 			inside a comment block <em>and</em> start with \a Comment
	 * \param	beginComment	marker for the beginning of a comment block (e.g. /<!---->* in C++)
	 * \param	endComment		marker for the end of a comment block (e.g. *<!---->/ in C++)
	 * \param	Comment			marker for a one-line comment (e.g. /<!---->/ in C++)
	 * \param	skipEmpty		if \c true, empty lines are simply disregarded
	 * \return	\c true if a title and type were found, \c false otherwise
	 */
	bool doParseHeader(const QString& beginComment, const QString& endComment, const QString& Comment, bool skipEmpty = true);
	
	/** \brief	Possible results of calls to doGetProperty() and doSetProperty() */
	enum PropertyResult {
		Property_OK,			///< the get/set operation was successful
		Property_Method,		///< the get operation failed because the specified property is a method
		Property_DoesNotExist,	///< the specified property/method doesn't exist
		Property_NotReadable,	///< the get operation failed because the property is not readable
		Property_NotWritable,	///< the set operation failed because the property is not writable
		Property_Invalid		///< the get/set operation failed due to invalid data
	};

	/** \brief	Possible results of calls to doCallMethod() */
	enum MethodResult {
		Method_OK,				///< the call was successful
		Method_DoesNotExist,	///< the call failed because the specified method doesn't exist
		Method_WrongArgs,		///< the method exists but could not be called with the given arguments
		Method_Failed,			///< the method was called but the call failed
		Method_Invalid			///< the call failed due to invalid data
	};

	/** \brief	Get the value of the property of a QObject
	 *
	 * \note	This function relies on the meta object concept of Qt.
	 * \param	obj		pointer to the QObject the property value of which to get
	 * \param	name	the name of the property to get
	 * \param	value	variable to receive the value of the property on success
	 * \return	one of TWScript::PropertyResult
	 */
	static TWScript::PropertyResult doGetProperty(const QObject * obj, const QString& name, QVariant & value);

	/** \brief	Set the value of the property of a QObject
	 *
	 * \note	This function relies on the meta object concept of Qt.
	 * \param	obj		pointer to the QObject the property value of which to set
	 * \param	name	the name of the property to set
	 * \param	value	the new value of the property
	 * \return	one of TWScript::PropertyResult
	 */
	static TWScript::PropertyResult doSetProperty(QObject * obj, const QString& name, const QVariant & value);

	/** \brief	Call a method of a QObject
	 *
	 * \note	This function relies on the meta object concept of Qt.
	 * \param	obj		pointer to the QObject the method of which should be called
	 * \param	name	the name of the method to call
	 * \param	arguments	arguments to pass to the method
	 * \param	result	variable to receive the return value of the method on success
	 * \return	one of TWScript::MethodResult
	 */
	static TWScript::MethodResult doCallMethod(QObject * obj, const QString& name, QVariantList & arguments, QVariant & result);
	
	QObject * m_Plugin; ///< pointer to the language interface for this script
	QString m_Filename;	///< the name of the file the script is stored in
	ScriptType m_Type;	///< the type of the script (ScriptUnknown indicates invalid)
	QString m_Title;	///< the title (e.g. for display in menus)
	QString m_Description;	///< the description
	QString m_Author;	///< the author's name
	QString m_Version;	///< the version
	QString m_Hook;		///< the hook this script implements (if any)
	QString m_Context;  ///< the main window class where this script can be used
	QKeySequence m_KeySequence;	///< the keyboard shortcut associated with this script

	bool m_Enabled; ///< whether this script is enabled (runtime property, not stored in the script itself)
	
	QTextCodec * m_Codec;

private slots:
	void globalDestroyed(QObject * obj);
	
private:
	/** \brief	Constructor
	 *
	 * Private, to prevent inadvertent use of the no-arg constructor.
	 */
	TWScript() { }
	
	QDateTime m_LastModified;	///< keeps track of the file modification time so we can detect changes
	qint64	m_FileSize;	///< similar to m_LastModified
 	
 	QHash<QString, QVariant> m_globals;
};

/** \brief	Interface all TW scripting plugins must implement */
class TWScriptLanguageInterface
{
public:
	/** \brief	Constructor
	 *
	 * Does nothing
	 */
	TWScriptLanguageInterface() { }
	
	/** \brief	Destructor
	 *
	 * Does nothing
	 */
	virtual ~TWScriptLanguageInterface() { }
	
	/** \brief	Method to create a new script wrapper
	 *
	 * This method must be implemented in derived classes;
	 * it should create the script wrapper for the given file.
	 * This method does NOT call parseHeader(), so until this
	 * is done, the script is not necessarily valid.
	 * \return	the script wrapper, or NULL if the file cannot be found
	 */
	virtual TWScript* newScript(const QString& fileName) = 0;

	/** \brief	Method to report the supported script language name
	 *
	 * This method must be implemented in derived classes
	 * \return	the name of the scripting language
	 */
	virtual QString scriptLanguageName() const = 0;

	/** \brief	Method to report a URL for information on the script language
	 *
	 * This method must be implemented in derived classes
	 * \return	a string with a URL for information about the language
	 */
	virtual QString scriptLanguageURL() const = 0;

	/** \brief	Report whether the given file is potentially a valid
	 *          script file for this language.
	 *
	 * This method is not expected to actually validate the script file
	 * (e.g., by fully parsing it, or even checking the headers); it
	 * only serves to "claim" the file for use by this scripting language/
	 * plugin rather than any others. It will typically just check the
	 * filename extension.
	 *
	 * This method must be implemented in derived classes
	 */
	virtual bool canHandleFile(const QFileInfo& fileInfo) const = 0;
};

Q_DECLARE_INTERFACE(TWScript, "org.tug.texworks.Script/0.3.2")
Q_DECLARE_INTERFACE(TWScriptLanguageInterface, "org.tug.texworks.ScriptLanguageInterface/0.3.2")

#endif /* TWScript_H */
