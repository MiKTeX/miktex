/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2013  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TW_LUA_PLUGIN_H
#define TW_LUA_PLUGIN_H

#include "TWScript.h"

#include "lua.hpp"

#include <QMetaMethod>
#include <QMetaProperty>
#include <QVariant>

/** \brief Implementation of the script plugin interface */
class TWLuaPlugin : public QObject, public TWScriptLanguageInterface
{
	Q_OBJECT
	Q_INTERFACES(TWScriptLanguageInterface)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.tug.texworks.ScriptPlugins.LuaPlugin")
#endif

public:
	/** \brief Constructor
	 *
	 * Initializes the lua state
	 */
	TWLuaPlugin();

	/** \brief Destructor
	 *
	 * Closes the lua state
	 */
	virtual ~TWLuaPlugin();

	/** \brief Script factory
	 *
	 * \return	pointer to a new LuaScript object cast to TWScript as the
	 * 			interface requires; the caller owns the object and must delete
	 * 			it.
	 */
	virtual TWScript* newScript(const QString& fileName);
	
	/** \brief	Get the supported script language name
	 *
	 * \return	the name of the scripting language
	 */
	virtual QString scriptLanguageName() const { return QString("Lua"); }
	
	/** \brief	Get a URL for information on the supported script language
	 *
	 * \return	a string with a URL for information about the language
	 */
	virtual QString scriptLanguageURL() const { return QString("http://www.lua.org/"); }
	
    /** \brief  Return whether the given file is handled by this scripting language plugin
	 */
	virtual bool canHandleFile(const QFileInfo& fileInfo) const { return fileInfo.suffix() == QString("lua"); }

	lua_State * getLuaState() { return luaState; }
	
protected:
	lua_State * luaState;	///< property to hold the lua state
};

/** \brief Class for handling lua scripts */
class LuaScript : public TWScript
{
	Q_OBJECT
	Q_INTERFACES(TWScript)
		
public:
	/** \brief Constructor
	 *
	 * Initializes m_LuaPlugin
	 * \param	lua	pointer to the plugin that holds the lua state to operate on
	 */
	LuaScript(TWLuaPlugin* lua, const QString& fileName) : TWScript(lua, fileName), m_LuaPlugin(lua) { }
	
	/** \brief Parse the script header
	 *
	 * \return	\c true if successful, \c false if not (e.g. because the file
	 * 			is no valid Tw lua script)
	 */
	virtual bool parseHeader() { return doParseHeader("--[[", "]]", ""); }

protected:
	/** \brief Run the lua script
	 *
	 * \param	tw	the TW interface object, exposed to the script as the TW global
	 *
	 * \return	\c true on success, \c false if an error occured
	 */
	virtual bool execute(TWScriptAPI *tw) const;
	
	/** \brief Convenience function to wrap a QObject and push it onto the stack
	 *
	 * \param	L	the lua state to operate on
	 * \param	obj	the QObject to expose to python
	 * \param	throwError	currently unused
	 * \return	the number of values pushed onto the stack; 1 on success, 0 on
	 * 			failure
	 */
	static int pushQObject(lua_State * L, QObject * obj, const bool throwError = true);

	/** \brief Convenience function to convert a QVariant and push it onto the stack
	 *
	 * \note	QList will be converted to lua tables with numeric, one-based
	 * 			indices.
	 * \param	L	the lua state to operate on
	 * \param	v	the QVariant to convert and push
	 * \param	throwError	if \c true, luaL_error is used to report errors;
	 * 				this should only be used in protected mode (i.e. from inside
	 * 				the lua engine, as it could cause Tw to crash otherwise)
	 * \return	the number of values pushed onto the stack; 1 on success, 0 on
	 * 			failure
	 */
	static int pushVariant(lua_State * L, const QVariant & v, const bool throwError = true);

	/** \brief Convenience function to convert a stack value to a QVariant
	 *
	 * \note	Tables with only numeric, consecutive, one-based indices will be
	 * 			converted to QList. If the value can't be converted, an error is
	 * 			raised and an empty QVariant is returned.
	 * \param	L	the lua state to operate on
	 * \param	idx	the index of the value on the stack (may be negative)
	 * \param	throwError	if \c true, luaL_error is used to report errors;
	 * 				this should only be used in protected mode (i.e. from inside
	 * 				the lua engine, as it could cause Tw to crash otherwise)
	 * \return	the QVariant
	 */
	static QVariant getLuaStackValue(lua_State * L, int idx, const bool throwError = true);
	
	/** \brief Handler for property requests on QObjects
	 *
	 * On success, the value of the property is pushed onto the stack
	 * \note	The QObject* must be supplied as first upvalue.
	 * \param	L	the lua state to operate on
	 * \return	the number of values pushed onto the stack; 1 on success, 0 on
	 * 			failure
	 */
	static int getProperty(lua_State * L);

	/** \brief Handler for setting attribute values on QObjects
	 *
	 * \note	The QObject* must be supplied as first upvalue.
	 * \param	L	the lua state to operate on
	 * \return	0 (the number of values pushed onto the stack)
	 */
	static int setProperty(lua_State * L);

	/** \brief Handler for calling methods of QObjects
	 *
	 * \note	The QObject* must be supplied as first upvalue and the method
	 * 			name as second upvalue.
	 * \note	For void methods, nil is pushed onto the stack
	 * \param	L	the lua state to operate on
	 * \return	the number of values pushed onto the stack; 1 on success, 0 on
	 * 			failure
	 */
	static int callMethod(lua_State * L);

	TWLuaPlugin * m_LuaPlugin;	///< pointer to the lua plugin holding the lua state
};

#endif // !defined(TW_LUA_PLUGIN_H)
