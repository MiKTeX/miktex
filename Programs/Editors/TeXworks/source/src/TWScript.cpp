/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2013  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TWScript.h"
#include "TWScriptAPI.h"
#include "ConfigurableApp.h"
#include "DefaultPrefs.h"

#include <QTextStream>
#include <QMetaObject>
#include <QMetaMethod>
#include <QApplication>
#include <QTextCodec>
#include <QDir>

TWScript::TWScript(QObject * plugin, const QString& fileName)
	: m_Plugin(plugin), m_Filename(fileName), m_Type(ScriptUnknown), m_Enabled(true), m_FileSize(0)
{
	m_Codec = QTextCodec::codecForName("UTF-8");
	if (!m_Codec)
		m_Codec = QTextCodec::codecForLocale();
}

bool TWScript::run(QObject *context, QVariant& result)
{
	TWScriptAPI tw(this, qApp, context, result);
	return execute(&tw);
}

bool TWScript::hasChanged() const
{
	QFileInfo fi(m_Filename);
	return (fi.size() != m_FileSize || fi.lastModified() != m_LastModified);
}

bool TWScript::doParseHeader(const QString& beginComment, const QString& endComment,
							 const QString& Comment, bool skipEmpty /* = true */)
{
	QFile file(m_Filename);
	QStringList lines;
	QString line;
	bool codecChanged = true;
	bool success = false;
	QTextCodec* codec;

	if (!file.exists() || !file.open(QIODevice::ReadOnly))
		return false;
	
	m_Codec = QTextCodec::codecForName("UTF-8");
	if (!m_Codec)
		m_Codec = QTextCodec::codecForLocale();

	while (codecChanged) {
		codec = m_Codec;
		file.seek(0);
		lines = codec->toUnicode(file.readAll()).split(QRegExp("\r\n|[\n\r]"));
	
		// skip any empty lines
		if (skipEmpty) {
			while (!lines.isEmpty() && lines.first().isEmpty())
				lines.removeFirst();
		}
		if (lines.isEmpty())
			break;
	
		// is this a valid TW script?
		line = lines.takeFirst();
		if (!beginComment.isEmpty()) {
			if (!line.startsWith(beginComment))
				break;
			line = line.mid(beginComment.size()).trimmed();
		}
		else if (!Comment.isEmpty()) {
			if (!line.startsWith(Comment))
				break;
			line = line.mid(Comment.size()).trimmed();
		}
		if (!line.startsWith("TeXworksScript"))
			break;
	
		// scan to find the extent of the header lines
		QStringList::iterator i;
		for (i = lines.begin(); i != lines.end(); ++i) {
			// have we reached the end?
			if (skipEmpty && i->isEmpty()) {
				i = lines.erase(i);
				--i;
				continue;
			}
			if (!endComment.isEmpty()) {
				if (i->startsWith(endComment))
					break;
			}
			if (!i->startsWith(Comment))
				break;
			*i = i->mid(Comment.size()).trimmed();
		}
		lines.erase(i, lines.end());
		
		codecChanged = false;
		switch (doParseHeader(lines)) {
			case ParseHeader_OK:
				success = true;
				break;
			case ParseHeader_Failed:
				success = false;
				break;
			case ParseHeader_CodecChanged:
				codecChanged = true;
				break;
		}
	}
	
	file.close();
	return success;
}

TWScript::ParseHeaderResult TWScript::doParseHeader(const QStringList & lines)
{
	QString line, key, value;
	QFileInfo fi(m_Filename);
	
	m_FileSize = fi.size();
	m_LastModified = fi.lastModified();
	
	foreach (line, lines) {
		key = line.section(':', 0, 0).trimmed();
		value = line.section(':', 1).trimmed();
		
		if (key == "Title") m_Title = value;
		else if (key == "Description") m_Description = value;
		else if (key == "Author") m_Author = value;
		else if (key == "Version") m_Version = value;
		else if (key == "Script-Type") {
			if (value == "hook") m_Type = ScriptHook;
			else if (value == "standalone") m_Type = ScriptStandalone;
			else m_Type = ScriptUnknown;
		}
		else if (key == "Hook") m_Hook = value;
		else if (key == "Context") m_Context = value;
		else if (key == "Shortcut") m_KeySequence = QKeySequence(value);
		else if (key == "Encoding") {
			QTextCodec * codec = QTextCodec::codecForName(value.toUtf8());
			if (codec) {
				if (!m_Codec || codec->name() != m_Codec->name()) {
					m_Codec = codec;
					return ParseHeader_CodecChanged;
				}
			}
		}
	}
	
	if (m_Type != ScriptUnknown && !m_Title.isEmpty())
		return ParseHeader_OK;
	return ParseHeader_Failed;
}

/*static*/
TWScript::PropertyResult TWScript::doGetProperty(const QObject * obj, const QString& name, QVariant & value)
{
	int iProp, i;
	QMetaProperty prop;
	
	if (!obj || !(obj->metaObject()))
		return Property_Invalid;
	
	// Get the parameters
	iProp = obj->metaObject()->indexOfProperty(qPrintable(name));
	
	// if we didn't find a property maybe it's a method
	if (iProp < 0) {
		for (i = 0; i < obj->metaObject()->methodCount(); ++i) {
			#if QT_VERSION >= 0x050000
			if (QString(obj->metaObject()->method(i).methodSignature()).startsWith(name + "("))
				return Property_Method;
			#else
			if (QString(obj->metaObject()->method(i).signature()).startsWith(name + "("))
				return Property_Method;
			#endif
		}
		return Property_DoesNotExist;
	}
	
	prop = obj->metaObject()->property(iProp);
	
	// If we can't get the property's value, abort
	if (!prop.isReadable())
		return Property_NotReadable;
	
	value = prop.read(obj);
	return Property_OK;
}

/*static*/
TWScript::PropertyResult TWScript::doSetProperty(QObject * obj, const QString& name, const QVariant & value)
{
	int iProp;
	QMetaProperty prop;
	
	if (!obj || !(obj->metaObject()))
		return Property_Invalid;
	
	iProp = obj->metaObject()->indexOfProperty(qPrintable(name));
	
	// if we didn't find the property abort
	if (iProp < 0)
		return Property_DoesNotExist;
	
	prop = obj->metaObject()->property(iProp);
	
	// If we can't set the property's value, abort
	if (!prop.isWritable())
		return Property_NotWritable;
	
	prop.write(obj, value);
	return Property_OK;
}

/*static*/
TWScript::MethodResult TWScript::doCallMethod(QObject * obj, const QString& name,
											  QVariantList & arguments, QVariant & result)
{
	const QMetaObject * mo;
	bool methodExists = false;
	QList<QGenericArgument> genericArgs;
	int type, typeOfArg, i, j;
	QString typeName;
	char * strTypeName;
	QMetaMethod mm;
	QGenericReturnArgument retValArg;
	void * retValBuffer = NULL;
	TWScript::MethodResult status;
	void * myNullPtr = NULL;
	
	if (!obj || !(obj->metaObject()))
		return Method_Invalid;
	
	mo = obj->metaObject();
	
	for (i = 0; i < mo->methodCount(); ++i) {
		mm = mo->method(i);
		// Check for the method name
		#if QT_VERSION >= 0x050000
		if (!QString(mm.methodSignature()).startsWith(name + "("))
			continue;
		#else
		if (!QString(mm.signature()).startsWith(name + "("))
			continue;
		#endif
		// we can only call public methods
		if (mm.access() != QMetaMethod::Public)
			continue;
		
		methodExists = true;
		
		// we need the correct number of arguments
		if (mm.parameterTypes().count() != arguments.count())
			continue;
		
		// Check if the given arguments are compatible with those taken by the
		// method
		for (j = 0; j < arguments.count(); ++j) {
			// QVariant can be passed as-is
			if (mm.parameterTypes()[j] == "QVariant")
				continue;
			
			type = QMetaType::type(mm.parameterTypes()[j]);
			typeOfArg = (int)arguments[j].type();
			if (typeOfArg == (int)type)
				continue;
			if (arguments[j].canConvert((QVariant::Type)type))
				continue;
			// allow invalid===NULL for pointers
			#if QT_VERSION >= 0x050000
			if (typeOfArg == QVariant::Invalid && type == QMetaType::QObjectStar)
				continue;
			#else
			if (typeOfArg == QVariant::Invalid && (type == QMetaType::QObjectStar || type == QMetaType::QWidgetStar))
				continue;
			// QObject* and QWidget* may be convertible
			if (typeOfArg == QMetaType::QWidgetStar && type == QMetaType::QObjectStar)
				continue;
			if (typeOfArg == QMetaType::QObjectStar && type == QMetaType::QWidgetStar && (arguments[j].value<QObject*>() == NULL || qobject_cast<QWidget*>(arguments[j].value<QObject*>())))
				continue;
			#endif
			break;
		}
		if (j < arguments.count())
			continue;
		
		// Convert the arguments into QGenericArgument structures
		for (j = 0; j < arguments.count() && j < 10; ++j) {
			typeName = mm.parameterTypes()[j];
			type = QMetaType::type(qPrintable(typeName));
			typeOfArg = (int)arguments[j].type();
			
			// allocate type name on the heap so it survives the method call
			strTypeName = new char[typeName.size() + 1];
			strcpy(strTypeName, qPrintable(typeName));
			
			if (typeName == "QVariant") {
				genericArgs.append(QGenericArgument(strTypeName, &arguments[j]));
				continue;
			}
			if (arguments[j].canConvert((QVariant::Type)type))
				arguments[j].convert((QVariant::Type)type);
			#if QT_VERSION >= 0x050000
			else if (typeOfArg == QVariant::Invalid && type == QMetaType::QObjectStar) {
				genericArgs.append(QGenericArgument(strTypeName, &myNullPtr));
				continue;
			}
			#else
			else if (typeOfArg == QVariant::Invalid && (type == QMetaType::QObjectStar || type == QMetaType::QWidgetStar)) {
				genericArgs.append(QGenericArgument(strTypeName, &myNullPtr));
				continue;
			}
			else if (typeOfArg == QMetaType::QWidgetStar && type == QMetaType::QObjectStar)
				arguments[j] = QVariant::fromValue(qobject_cast<QObject*>(arguments[j].value<QWidget*>()));
			else if (typeOfArg == QMetaType::QObjectStar && type == QMetaType::QWidgetStar && (arguments[j].value<QObject*>() == NULL || qobject_cast<QWidget*>(arguments[j].value<QObject*>())))
				arguments[j] = QVariant::fromValue(qobject_cast<QWidget*>(arguments[j].value<QObject*>()));
			#endif
			// \TODO	handle failure during conversion
			else { }
			
			// Note: This line is a hack!
			// QVariant::data() is undocumented; QGenericArgument should not be
			// called directly; if this ever causes problems, think of another
			// (better) way to do this
			genericArgs.append(QGenericArgument(strTypeName, arguments[j].data()));
		}
		// Fill up the list so we get the 10 values we need later on
		for (; j < 10; ++j)
			genericArgs.append(QGenericArgument());
		
		typeName = mm.typeName();
		if (typeName.isEmpty()) {
			// no return type
			retValArg = QGenericReturnArgument();
		}
		else if (typeName == "QVariant") {
			// QMetaType can't construct QVariant objects
			retValArg = Q_RETURN_ARG(QVariant, result);
		}
		else {
			// Note: These two lines are a hack!
			// QGenericReturnArgument should not be constructed directly; if
			// this ever causes problems, think of another (better) way to do this
			#if QT_VERSION >= 0x050000
			retValBuffer = QMetaType::create(QMetaType::type(mm.typeName()));
			#else
			retValBuffer = QMetaType::construct(QMetaType::type(mm.typeName()));
			#endif
			retValArg = QGenericReturnArgument(mm.typeName(), retValBuffer);
		}
		
		if (mo->invokeMethod(obj, qPrintable(name),
							 Qt::DirectConnection,
							 retValArg,
							 genericArgs[0],
							 genericArgs[1],
							 genericArgs[2],
							 genericArgs[3],
							 genericArgs[4],
							 genericArgs[5],
							 genericArgs[6],
							 genericArgs[7],
							 genericArgs[8],
							 genericArgs[9])
		   ) {
			if (retValBuffer)
				result = QVariant(QMetaType::type(mm.typeName()), retValBuffer);
			else if (typeName == "QVariant")
				; // don't do anything here; the return valus is already in result
			else
				result = QVariant();
			status = Method_OK;
		}
		else
			status = Method_Failed;
		
		if (retValBuffer)
			QMetaType::destroy(QMetaType::type(mm.typeName()), retValBuffer);
		
		for (j = 0; j < arguments.count() && j < 10; ++j) {
			// we pushed the data on the heap, we need to remove it from there
			delete[] genericArgs[j].name();
		}
		
		return status;
	}
	
	if (methodExists)
		return Method_WrongArgs;
	return Method_DoesNotExist;
}

void TWScript::setGlobal(const QString& key, const QVariant& val)
{
	QVariant v = val;

	if (key.isEmpty())
		return;

	// For objects on the heap make sure we are notified when their lifetimes
	// end so that we can remove them from our hash accordingly
	switch ((QMetaType::Type)val.type()) {
		case QMetaType::QObjectStar:
			connect(v.value<QObject*>(), SIGNAL(destroyed(QObject*)), this, SLOT(globalDestroyed(QObject*)));
			break;
		#if QT_VERSION < 0x050000
		case QMetaType::QWidgetStar:
			connect((QWidget*)v.data(), SIGNAL(destroyed(QObject*)), this, SLOT(globalDestroyed(QObject*)));
			break;
		#endif
		default: break;
	}
	m_globals[key] = v;
}

void TWScript::globalDestroyed(QObject * obj)
{
	QHash<QString, QVariant>::iterator i = m_globals.begin();
	
	while (i != m_globals.end()) {
		switch ((QMetaType::Type)i.value().type()) {
			case QMetaType::QObjectStar:
				if (i.value().value<QObject*>() == obj)
					i = m_globals.erase(i);
				else
					++i;
				break;
			#if QT_VERSION < 0x050000
			case QMetaType::QWidgetStar:
				if (i.value().value<QWidget*>() == obj)
					i = m_globals.erase(i);
				else
					++i;
				break;
			#endif
			default:
				++i;
				break;
		}
	}
}


bool TWScript::mayExecuteSystemCommand(const QString& cmd, QObject * context) const
{
	Q_UNUSED(cmd)
	Q_UNUSED(context)
	
	// cmd may be a true command line, or a single file/directory to run or open
	QSETTINGS_OBJECT(settings);
	return settings.value("allowSystemCommands", false).toBool();
}

bool TWScript::mayWriteFile(const QString& filename, QObject * context) const
{
	Q_UNUSED(filename)
	Q_UNUSED(context)
	
	QSETTINGS_OBJECT(settings);
	return settings.value("allowScriptFileWriting", false).toBool();
}

bool TWScript::mayReadFile(const QString& filename, QObject * context) const
{
	QSETTINGS_OBJECT(settings);
	QDir scriptDir(QFileInfo(m_Filename).absoluteDir());
	QVariant targetFile;
	QDir targetDir;
	
	if (settings.value("allowScriptFileReading", kDefault_AllowScriptFileReading).toBool())
		return true;
	
	// even if global reading is disallowed, some exceptions may apply
	QFileInfo fi(QDir::cleanPath(filename));

	// reading in subdirectories of the script file's directory is always allowed
	if (!scriptDir.relativeFilePath(fi.absolutePath()).startsWith(".."))
		return true;

	if (context) {
		// reading subdirectories of the current file is always allowed
		targetFile = context->property("fileName");
		if (targetFile.isValid() && !targetFile.toString().isEmpty()) {
			targetDir = QFileInfo(targetFile.toString()).absoluteDir();
			if (!targetDir.relativeFilePath(fi.absolutePath()).startsWith(".."))
				return true;
		}
		// reading subdirectories of the root file is always allowed
		targetFile = context->property("rootFileName");
		if (targetFile.isValid() && !targetFile.toString().isEmpty()) {
			targetDir = QFileInfo(targetFile.toString()).absoluteDir();
			if (!targetDir.relativeFilePath(fi.absolutePath()).startsWith(".."))
				return true;
		}
	}
	
	return false;
}

