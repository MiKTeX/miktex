/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2021  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "scripting/Script.h"

#include <QDir>
#include <QMetaMethod>
#include <QMetaObject>
#include <QRegularExpression>
#include <QTextCodec>
#include <QTextStream>

namespace Tw {
namespace Scripting {

Script::Script(QObject * plugin, const QString& fileName)
	: m_Plugin(plugin), m_Filename(fileName), m_Type(ScriptUnknown), m_Enabled(true), m_FileSize(0)
{
	m_Codec = QTextCodec::codecForName("UTF-8");
	if (!m_Codec)
		m_Codec = QTextCodec::codecForLocale();
}

bool Script::run(Tw::Scripting::ScriptAPIInterface & api)
{
	return execute(&api);
}

bool Script::hasChanged() const
{
	QFileInfo fi(m_Filename);
	return (fi.size() != m_FileSize || fi.lastModified() != m_LastModified);
}

bool Script::doParseHeader(const QString& beginComment, const QString& endComment,
							 const QString& Comment, bool skipEmpty /* = true */)
{
	QFile file(m_Filename);
	QStringList lines;
	QString line;
	bool codecChanged = true;
	bool success = false;

	if (!file.exists() || !file.open(QIODevice::ReadOnly))
		return false;

	m_Codec = QTextCodec::codecForName("UTF-8");
	if (!m_Codec)
		m_Codec = QTextCodec::codecForLocale();

	while (codecChanged) {
		QTextCodec * codec = m_Codec;
		file.seek(0);
		lines = codec->toUnicode(file.readAll()).split(QRegularExpression(QStringLiteral("\r\n|[\n\r]")));

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
		if (!line.startsWith(QLatin1String("TeXworksScript")))
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

Script::ParseHeaderResult Script::doParseHeader(const QStringList & lines)
{
	QString line, key, value;
	QFileInfo fi(m_Filename);

	m_FileSize = fi.size();
	m_LastModified = fi.lastModified();

	foreach (line, lines) {
		key = line.section(QChar::fromLatin1(':'), 0, 0).trimmed();
		value = line.section(QChar::fromLatin1(':'), 1).trimmed();

		if (key == QLatin1String("Title")) m_Title = value;
		else if (key == QLatin1String("Description")) m_Description = value;
		else if (key == QLatin1String("Author")) m_Author = value;
		else if (key == QLatin1String("Version")) m_Version = value;
		else if (key == QLatin1String("Script-Type")) {
			if (value == QLatin1String("hook")) m_Type = ScriptHook;
			else if (value == QLatin1String("standalone")) m_Type = ScriptStandalone;
			else m_Type = ScriptUnknown;
		}
		else if (key == QLatin1String("Hook")) m_Hook = value;
		else if (key == QLatin1String("Context")) m_Context = value;
		else if (key == QLatin1String("Shortcut")) m_KeySequence = QKeySequence(value);
		else if (key == QLatin1String("Encoding")) {
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
Script::PropertyResult Script::doGetProperty(const QObject * obj, const QString& name, QVariant & value)
{
	if (!obj || !(obj->metaObject()))
		return Property_Invalid;

	// Get the parameters
	int iProp = obj->metaObject()->indexOfProperty(qPrintable(name));

	// if we didn't find a property maybe it's a method
	if (iProp < 0) {
		for (int i = 0; i < obj->metaObject()->methodCount(); ++i) {
			if (QString::fromUtf8(obj->metaObject()->method(i).methodSignature()).startsWith(name + QChar::fromLatin1('(')))
				return Property_Method;
		}
		return Property_DoesNotExist;
	}

	QMetaProperty prop = obj->metaObject()->property(iProp);

	// If we can't get the property's value, abort
	if (!prop.isReadable())
		return Property_NotReadable;

	value = prop.read(obj);
	return Property_OK;
}

/*static*/
Script::PropertyResult Script::doSetProperty(QObject * obj, const QString& name, const QVariant & value)
{
	if (!obj || !(obj->metaObject()))
		return Property_Invalid;

	int iProp = obj->metaObject()->indexOfProperty(qPrintable(name));

	// if we didn't find the property abort
	if (iProp < 0)
		return Property_DoesNotExist;

	QMetaProperty prop = obj->metaObject()->property(iProp);

	// If we can't set the property's value, abort
	if (!prop.isWritable())
		return Property_NotWritable;

	prop.write(obj, value);
	return Property_OK;
}

/*static*/
Script::MethodResult Script::doCallMethod(QObject * obj, const QString& name,
											  QVariantList & arguments, QVariant & result)
{
	bool methodExists = false;
	QList<QGenericArgument> genericArgs;
	QString typeName;
	QMetaMethod mm;
	QGenericReturnArgument retValArg;
	void * retValBuffer = nullptr;
	void * myNullPtr = nullptr;

	if (!obj || !(obj->metaObject()))
		return Method_Invalid;

	const QMetaObject * mo = obj->metaObject();

	for (int i = 0; i < mo->methodCount(); ++i) {
		mm = mo->method(i);
		// Check for the method name
		if (!QString::fromUtf8(mm.methodSignature()).startsWith(name + QChar::fromLatin1('(')))
			continue;
		// we can only call public methods
		if (mm.access() != QMetaMethod::Public)
			continue;

		methodExists = true;

		// we need the correct number of arguments
		if (mm.parameterTypes().count() != arguments.count())
			continue;

		// Check if the given arguments are compatible with those taken by the
		// method
		int j{0};
		for (j = 0; j < arguments.count(); ++j) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			// QVariant can be passed as-is
			if (mm.parameterTypes()[j] == "QVariant")
				continue;

			int type = QMetaType::type(mm.parameterTypes()[j].constData());
			int typeOfArg = static_cast<int>(arguments[j].type());
			if (typeOfArg == type)
				continue;
			if (arguments[j].canConvert(type))
				continue;
			// allow invalid===nullptr for pointers
			if (typeOfArg == QVariant::Invalid && type == QMetaType::QObjectStar)
				continue;
#else
			// QVariant can be passed as-is
			if (mm.parameterType(j) == QMetaType::QVariant)
				continue;

			if (mm.parameterMetaType(j) == arguments[j].metaType())
				continue;
			if (arguments[j].canConvert(mm.parameterMetaType(j)))
				continue;
			// allow invalid===nullptr for pointers
			if (arguments[j].metaType().id() == QMetaType::UnknownType && mm.parameterType(j) == QMetaType::QObjectStar)
				continue;
#endif
			break;
		}
		if (j < arguments.count())
			continue;

		// Convert the arguments into QGenericArgument structures
		for (j = 0; j < arguments.count() && j < 10; ++j) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			typeName = QString::fromUtf8(mm.parameterTypes()[j].constData());
			int type = QMetaType::type(qPrintable(typeName));
			int typeOfArg = static_cast<int>(arguments[j].type());

			// allocate type name on the heap so it survives the method call
			char * strTypeName = new char[typeName.size() + 1];
			strcpy(strTypeName, qPrintable(typeName));

			if (typeName == QString::fromLatin1("QVariant")) {
				genericArgs.append(QGenericArgument(strTypeName, &arguments[j]));
				continue;
			}
			if (arguments[j].canConvert(type))
				arguments[j].convert(type);
			else if (typeOfArg == QVariant::Invalid && type == QMetaType::QObjectStar) {
				genericArgs.append(QGenericArgument(strTypeName, &myNullPtr));
				continue;
			}
			else {
				// \TODO	handle failure during conversion
			}

			// Note: This line is a hack!
			// QVariant::data() is undocumented; QGenericArgument should not be
			// called directly; if this ever causes problems, think of another
			// (better) way to do this
			genericArgs.append(QGenericArgument(strTypeName, arguments[j].data()));
#else
			const QMetaType mt = mm.parameterMetaType(j);
			if (mt.id() == QMetaType::QVariant) {
				genericArgs.append(QGenericArgument(mt.name(), &arguments[j]));
				continue;
			}
			if (arguments[j].canConvert(mt))
				arguments[j].convert(mt);
			else if (arguments[j].metaType().id() == QMetaType::UnknownType && mt.id() == QMetaType::QObjectStar) {
				genericArgs.append(QGenericArgument(mt.name(), &myNullPtr));
				continue;
			}
			// \TODO	handle failure during conversion
			else { }

			// Note: This line is a hack!
			// QGenericArgument should not be
			// called directly; if this ever causes problems, think of another
			// (better) way to do this
			genericArgs.append(QGenericArgument(mt.name(), arguments[j].constData()));
#endif
		}
		// Fill up the list so we get the 10 values we need later on
		for (; j < 10; ++j)
			genericArgs.append(QGenericArgument());

		typeName = QString::fromUtf8(mm.typeName());
		if (typeName.isEmpty()) {
			// no return type
			retValArg = QGenericReturnArgument();
		}
		else if (typeName == QString::fromLatin1("QVariant")) {
			// QMetaType can't construct QVariant objects
			retValArg = Q_RETURN_ARG(QVariant, result);
		}
		else {
			// Note: These two lines are a hack!
			// QGenericReturnArgument should not be constructed directly; if
			// this ever causes problems, think of another (better) way to do this
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			retValBuffer = QMetaType::create(QMetaType::type(mm.typeName()));
#else
			retValBuffer = mm.returnMetaType().create();
#endif
			retValArg = QGenericReturnArgument(mm.typeName(), retValBuffer);
		}

		Script::MethodResult status{Method_Failed};
		if (QMetaObject::invokeMethod(obj, qPrintable(name),
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
			if (retValBuffer) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				result = QVariant(QMetaType::type(mm.typeName()), retValBuffer);
#else
				result = QVariant(mm.returnMetaType(), retValBuffer);
#endif
			}
			else if (typeName == QString::fromLatin1("QVariant"))
				; // don't do anything here; the return valus is already in result
			else
				result = QVariant();
			status = Method_OK;
		}

		if (retValBuffer) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			QMetaType::destroy(QMetaType::type(mm.typeName()), retValBuffer);
#else
			mm.returnMetaType().destroy(retValBuffer);
#endif
		}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		for (j = 0; j < arguments.count() && j < 10; ++j) {
			// we pushed the data on the heap, we need to remove it from there
			delete[] genericArgs[j].name();
		}
#endif

		return status;
	}

	if (methodExists)
		return Method_WrongArgs;
	return Method_DoesNotExist;
}

void Script::setGlobal(const QString& key, const QVariant& val)
{
	QVariant v = val;

	if (key.isEmpty())
		return;

	// For objects on the heap make sure we are notified when their lifetimes
	// end so that we can remove them from our hash accordingly
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	switch (static_cast<QMetaType::Type>(val.type())) {
#else
	switch (val.metaType().id()) {
#endif
		case QMetaType::QObjectStar:
			connect(v.value<QObject*>(), &QObject::destroyed, this, &Script::globalDestroyed);
			break;
		default: break;
	}
	m_globals[key] = v;
}

void Script::globalDestroyed(QObject * obj)
{
	QHash<QString, QVariant>::iterator i = m_globals.begin();

	while (i != m_globals.end()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		switch (static_cast<QMetaType::Type>(i.value().type())) {
#else
		switch (i.value().metaType().id()) {
#endif
			case QMetaType::QObjectStar:
				if (i.value().value<QObject*>() == obj)
					i = m_globals.erase(i);
				else
					++i;
				break;
			default:
				++i;
				break;
		}
	}
}

} // namespace Scripting
} // namespace Tw
