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

#include "TWLuaPlugin.h"
#include "TWScriptAPI.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QtPlugin>
#include <QMetaObject>
#include <QStringList>

TWLuaPlugin::TWLuaPlugin()
{
	// Initialize lua state
	luaState = luaL_newstate();
	if (luaState) {
		luaL_openlibs(luaState);
	}
}

TWLuaPlugin::~TWLuaPlugin()
{
	if (luaState)
		lua_close(luaState);
}

TWScript* TWLuaPlugin::newScript(const QString& fileName)
{
	return new LuaScript(this, fileName);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(TWLuaPlugin, TWLuaPlugin)
#endif


bool LuaScript::execute(TWScriptAPI *tw) const
{
	int status;
	lua_State * L = m_LuaPlugin->getLuaState();

	if (!L)
		return false;

	// register the TW interface for use in lua
	if (!LuaScript::pushQObject(L, tw, false)) {
		tw->SetResult(tr("Could not register TW"));
		return false;
	}
	lua_setglobal(L, "TW");
	
	status = luaL_loadfile(L, qPrintable(m_Filename));
	if (status != 0) {
		tw->SetResult(getLuaStackValue(L, -1, false).toString());
		lua_pop(L, 1);
		return false;
	}

	// call the script
	status = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (status != 0) {
		tw->SetResult(getLuaStackValue(L, -1, false).toString());
		lua_pop(L, 1);
		return false;
	}
	
	lua_pushnil(L);
	lua_setglobal(L, "TW");

	return true;
}

/*static*/
int LuaScript::pushQObject(lua_State * L, QObject * obj, const bool throwError /* = true */)
{
	Q_UNUSED(throwError)
	
	if (!L || !obj)
		return 0;
	
	lua_newtable(L);

	// register callback for all get/set operations on object properties and
	// all call operations on object methods
	if (lua_getmetatable(L, -1) == 0)
		lua_newtable(L);

	lua_pushlightuserdata(L, obj);
	lua_setfield(L, -2, "__qobject");
	
	lua_pushlightuserdata(L, obj);
	lua_pushcclosure(L, LuaScript::setProperty, 1);
	lua_setfield(L, -2, "__newindex");

	lua_pushlightuserdata(L, obj);
	lua_pushcclosure(L, LuaScript::getProperty, 1);
	lua_setfield(L, -2, "__index");

	lua_pushlightuserdata(L, obj);
	lua_pushcclosure(L, LuaScript::callMethod, 1);
	lua_setfield(L, -2, "__call");

	lua_setmetatable(L, -2);
	return 1;
}

/*static*/
int LuaScript::pushVariant(lua_State * L, const QVariant & v, const bool throwError /* = true */)
{
	int i;
	QVariantList::const_iterator iList;
	QVariantList list;
#if QT_VERSION >= 0x040500
	QVariantHash::const_iterator iHash;
	QVariantHash hash;
#endif
	QVariantMap::const_iterator iMap;
	QVariantMap map;

	if (!L)
		return 0;
	if (v.isNull()) {
		lua_pushnil(L);
		return 1;
	}
	
	switch ((QMetaType::Type)v.type()) {
		case QVariant::Bool:
			lua_pushboolean(L, v.toBool());
			return 1;
		case QVariant::Double:
		case QVariant::Int:
		case QVariant::LongLong:
		case QVariant::UInt:
		case QVariant::ULongLong:
			lua_pushnumber(L, v.toDouble());
			return 1;
		case QVariant::Char:
		case QVariant::String:
			lua_pushstring(L, v.toString().toUtf8().constData());
			return 1;
		case QVariant::List:
		case QVariant::StringList:
			list = v.toList();

			lua_newtable(L);
			for (i = 1, iList = list.begin(); iList != list.end(); ++iList, ++i) {
				lua_pushnumber(L, i);
				LuaScript::pushVariant(L, *iList);
				lua_settable(L, -3);
			}
			return 1;
#if QT_VERSION >= 0x040500
		case QVariant::Hash:
			hash = v.toHash();
			
			lua_newtable(L);
			for (iHash = hash.begin(); iHash != hash.end(); ++iHash) {
				LuaScript::pushVariant(L, iHash.value());
				lua_setfield(L, -2, qPrintable(iHash.key()));
			}
			return 1;
#endif
		case QVariant::Map:
			map = v.toMap();
			
			lua_newtable(L);
			for (iMap = map.begin(); iMap != map.end(); ++iMap) {
				LuaScript::pushVariant(L, iMap.value());
				lua_setfield(L, -2, qPrintable(iMap.key()));
			}
			return 1;
		case QMetaType::QObjectStar:
			return LuaScript::pushQObject(L, v.value<QObject*>(), throwError);
		#if QT_VERSION < 0x050000
		case QMetaType::QWidgetStar:
			return LuaScript::pushQObject(L, qobject_cast<QObject*>(v.value<QWidget*>()), throwError);
		#endif
		default:
			// Don't throw errors if we are not in protected mode in lua, i.e.
			// if the call to this function originated from C code, not in response
			// to a lua request (e.g. during initialization or finalization) as that
			// would crash Tw
			if (throwError) luaL_error(L, "the type %s is currently not supported", v.typeName());
	}
	return 0;
}

/*static*/
int LuaScript::getProperty(lua_State * L)
{
	QObject * obj;
	QString propName;
	QVariant result;

	// We should have the lua table (=object) we're called from and the property
	// name we should get; if not, something is wrong
	if (lua_gettop(L) != 2) {
		luaL_error(L, qPrintable(tr("__get: invalid call -- expected exactly 2 arguments, got %f")), lua_gettop(L));
		return 0;
	}

	// Get the QObject* we operate on
	obj = (QObject*)lua_topointer(L, lua_upvalueindex(1));
	
	// Get the parameters
	propName = lua_tostring(L, 2);
	
	switch (doGetProperty(obj, propName, result)) {
		case Property_DoesNotExist:
			luaL_error(L, qPrintable(tr("__get: object doesn't have property/method %s")), qPrintable(propName));
			return 0;
		case Property_NotReadable:
			luaL_error(L, qPrintable(tr("__get: property %s is not readable")), qPrintable(propName));
			return 0;
		case Property_Method:
			lua_pushlightuserdata(L, obj);
			lua_pushstring(L, qPrintable(propName));
			lua_pushcclosure(L, LuaScript::callMethod, 2);
			return 1;
		case Property_OK:
			return LuaScript::pushVariant(L, result);
		default:
			break;
	}
	// we should never reach this point
	return 0;

}

/*static*/
int LuaScript::callMethod(lua_State * L)
{
	int i;
	QObject * obj;
	QString methodName;
	QList<QVariant> args;
	QVariant result;

	// Get the QObject* we operate on
	obj = (QObject*)lua_topointer(L, lua_upvalueindex(1));

	methodName = lua_tostring(L, lua_upvalueindex(2));
	
	for (i = 1; i <= lua_gettop(L); ++i) {
		args.append(getLuaStackValue(L, i));
	}

	switch (doCallMethod(obj, methodName, args, result)) {
		case Method_OK:
			return LuaScript::pushVariant(L, result);
		case Method_DoesNotExist:
			luaL_error(L, qPrintable(tr("__call: the method %s doesn't exist")), qPrintable(methodName));
			return 0;
		case Method_WrongArgs:
			luaL_error(L, qPrintable(tr("__call: couldn't call %s with the given arguments")), qPrintable(methodName));
			return 0;
		case Method_Failed:
			luaL_error(L, qPrintable(tr("__call: internal error while executing %s")), qPrintable(methodName));
			return 0;
		default:
			break;
	}
	
	// we should never reach this point
	return 0;
}

/*static*/
int LuaScript::setProperty(lua_State * L)
{
	QObject * obj;
	QString propName;

	// We should have the lua table (=object) we're called from, the property
	// name we should set, and the new value; if not, something is wrong
	if (lua_gettop(L) != 3) {
		luaL_error(L, qPrintable(tr("__set: invalid call -- expected exactly 3 arguments, got %f")), lua_gettop(L));
		return 0;
	}

	// Get the QObject* we operate on
	obj = (QObject*)lua_topointer(L, lua_upvalueindex(1));

	// Get the parameters
	propName = lua_tostring(L, 2);

	switch (doSetProperty(obj, propName, LuaScript::getLuaStackValue(L, 3))) {
		case Property_DoesNotExist:
			luaL_error(L, qPrintable(tr("__set: object doesn't have property %s")), qPrintable(propName));
			return 0;
		case Property_NotWritable:
			luaL_error(L, qPrintable(tr("__set: property %s is not writable")), qPrintable(propName));
			return 0;
		case Property_OK:
			return 0;
		default:
			break;
	}
	// we should never reach this point
	return 0;
}

/*static*/
QVariant LuaScript::getLuaStackValue(lua_State * L, int idx, const bool throwError /* = true */)
{
	bool isArray = true, isMap = true, isQObject = false;
	QVariantList vl;
	QVariantMap vm;
	int i, n, iMax;

	if (!L) return QVariant();
	
	switch (lua_type(L, idx)) {
		case LUA_TNIL:
			return QVariant();
		case LUA_TNUMBER:
			return QVariant(lua_tonumber(L, idx));
		case LUA_TBOOLEAN:
			return QVariant(lua_toboolean(L, idx) == 1);
		case LUA_TSTRING:
			return QVariant(QString::fromUtf8(lua_tostring(L, idx)));
		case LUA_TTABLE:
			// convert index to an absolute value since we'll be messing with
			// the stack
			if (idx < 0) idx += lua_gettop(L) + 1;
			
			// Check if we're dealing with a QObject* wrapper
			if (lua_getmetatable(L, idx)) {

				i = lua_gettop(L);
				lua_pushnil(L);
				// see if the metatable contains the key "__qobject"; if it
				// doesn't, trying to get it later could result in an error
				while (lua_next(L, i)) {
					lua_pop(L, 1); // pop the value (we don't need it)
					if (!lua_isstring(L, -1))
						continue;
					lua_pushvalue(L, -1); // duplicate the key so we don't disturb lua_next
					if (QString(lua_tostring(L, -1)) == "__qobject")
						isQObject = true;
					lua_pop(L, 1); // pop the duplicate key
				}

				if (isQObject) {
					lua_getfield(L, -1, "__qobject");
					if (lua_islightuserdata(L, -1)) {
						QObject * obj = (QObject*)lua_touserdata(L, -1);
						lua_pop(L, 2);
						return QVariant::fromValue(obj);
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 1); // pop the metatable
			}
			
			// Special treatment for tables
			// If all keys are in the form 1..n, we can convert it to a QList
			
			// taken from the lua reference of lua_next()
			lua_pushnil(L);
			n = 0;
			iMax = 0;
			while (lua_next(L, idx)) {
				if (isArray) {
					if (!lua_isnumber(L, -2))
						isArray = false;
					else {
						++n;
						if (lua_tonumber(L, -2) > iMax)
							iMax = lua_tonumber(L, -2);
					}
				}
				if (isMap) {
					// keys must be convertable to string
					if (!lua_isstring(L, -2))
						isMap = false;
					// some value types are not supported by QVariant
					if (lua_isfunction(L, -1) ||
						lua_islightuserdata(L, -1) ||
						lua_isthread(L, -1) ||
						lua_isuserdata(L, -1) )
						isMap = false;
				}
				lua_pop(L, 1);
				++i;
			}
			if (n != iMax)
				isArray = false;
			
			// Lua is picky about the correct type of index for accessing table
			// members. Hence we can't simply retrieve the table items by index
			// because they must only be _convertible_ to numbers, so 1 and "1"
			// should be treated the same (though they are not by Lua). Since
			// they keys need not be ordered when calling lua_next, we have to
			// allocate the complete list first and overwrite the items as we
			// get them.
			if (isArray) {
				for (i = 0; i < n; ++i)
					vl.append(QVariant());
				
				lua_pushnil(L);
				while (lua_next(L, idx)) {
					vl[(int)lua_tonumber(L, -2) - 1] = LuaScript::getLuaStackValue(L, -1);
					lua_pop(L, 1);
				}
				return vl;
			}
			// use QMap here because Lua doesn't support multiple values for one
			// key (those are converted to lists implicitly) and QVariantMap is
			// backwards compatible
			if (isMap) {
				lua_pushnil(L);
				while (lua_next(L, idx)) {
					// duplicate the key. If we didn't, lua_tostring could
					// convert it, thereby confusing lua_next later on
					lua_pushvalue(L, -2);
					vm.insert(lua_tostring(L, -2), LuaScript::getLuaStackValue(L, -1));
					lua_pop(L, 2);
				}
				return vm;
			}
			
			// deliberately no break here; if the table could not be converted
			// to QList or QMap, we have to treat it as unsupported
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
		default:
			// Don't throw errors if we are not in protected mode in lua, i.e.
			// if the call to this function originated from C code, not in response
			// to a lua request (e.g. during initialization or finalization) as that
			// would crash Tw
			if (throwError)
				luaL_error(L, qPrintable(tr("the lua type %s is currently not supported")), lua_typename(L, lua_type(L, idx)));
	}
	return QVariant();
}
