/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2023  Stefan Löffler

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
#ifndef ScriptObject_H
#define ScriptObject_H

#include <scripting/Script.h>

#include <QObject>
#include <memory>

namespace Tw {
namespace Scripting {

// A simple wrapper around a Script that turns it into a QObject
//
// NB: As Script objects can be created/used in (scripting) plugins, those
// plugins generally include a full implementation. As QObjects have a (type-
// specific) static meta object, they can't be used across plugin boundaries, or
// else we'd end up with two different instances of the same static meta object,
// violating the one-definition-rule (odr).
// To work around this problem, Script is not derived from QObject, and instead
// the Tw core code can wrap it into a ScriptObject to get all the QObject
// goodies.
class ScriptObject : public QObject
{
	Q_OBJECT
	std::unique_ptr<Script> m_script;
public:
	explicit ScriptObject(std::unique_ptr<Script> && script, QObject * parent = nullptr);

	Q_PROPERTY(QString fileName READ getFilename)
	Q_PROPERTY(QString title READ getTitle)
	Q_PROPERTY(QString description READ getDescription)
	Q_PROPERTY(QString author READ getAuthor)
	Q_PROPERTY(QString version READ getVersion)

	bool isEnabled() const { return (m_script ? m_script->isEnabled() : false); }
	bool hasChanged() const { return (m_script ? m_script->hasChanged() : false); }

	Script::ScriptType getType() const { return (m_script ? m_script->getType() : Script::ScriptUnknown); }
	const QString getFilename() const { return (m_script ? m_script->getFilename() : QString()); }
	const QString getTitle() const { return (m_script ? m_script->getTitle() : QString()); }
	const QString getDescription() const { return (m_script ? m_script->getDescription() : QString()); }
	const QString getAuthor() const { return (m_script ? m_script->getAuthor() : QString()); }
	const QString getVersion() const { return (m_script ? m_script->getVersion() : QString()); }
	const QString getHook() const { return (m_script ? m_script->getHook() : QString()); }
	const QString getContext() const { return (m_script ? m_script->getContext() : QString()); }
	const QKeySequence getKeySequence() const { return (m_script ? m_script->getKeySequence() : QKeySequence()); }
	const QObject * getScriptLanguagePlugin() const { return (m_script ? m_script->getScriptLanguagePlugin() : nullptr); }

	bool run(Tw::Scripting::ScriptAPIInterface & api) { return (m_script ? m_script->run(api) : false); }

	Script * getScript() { return m_script.get(); }

	Q_INVOKABLE void setGlobal(const QString& key, const QVariant& val) { if (m_script) { m_script->setGlobal(key, val); } }
	Q_INVOKABLE void unsetGlobal(const QString& key) { if (m_script) { m_script->unsetGlobal(key); } }
	Q_INVOKABLE bool hasGlobal(const QString& key) const { return (m_script ? m_script->hasGlobal(key) : false); }
	Q_INVOKABLE QVariant getGlobal(const QString& key) const { return (m_script ? m_script->getGlobal(key) : QVariant()); }
};

} // namespace Scripting
} // namespace Tw

#endif // !defined(ScriptObject_H)
