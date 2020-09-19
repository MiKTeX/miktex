/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "PythonScript.h"

// Python uses the name "slots", which Qt hijacks. So we temporarily undefine
// it, then include the python headers, then redefine it
#undef slots
#ifdef __APPLE__ // can't use Q_OS_DARWIN as it's not defined yet!
#include <Python/Python.h>
#else
#include <Python.h>
#endif
#define slots Q_SLOTS

#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QStringList>
#include <QTextStream>
#include <QRegularExpression>

namespace Tw {
namespace Scripting {

/* macros that may not be available in older python headers */
#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif
#ifndef Py_RETURN_TRUE
#define Py_RETURN_TRUE return Py_INCREF(Py_True), Py_True
#endif
#ifndef Py_RETURN_FALSE
#define Py_RETURN_FALSE return Py_INCREF(Py_False), Py_False
#endif

/* To encapsulate C pointers, PyCObject was replaced by PyCapsule in Python 3.2 */
#if PY_VERSION_HEX < 0x03020000
	#define ENCAPSULATE_C_POINTER(ptr) PyCObject_FromVoidPtr((ptr), nullptr)
	#define IS_ENCAPSULATED_C_POINTER(obj) PyCObject_Check((obj))
	#define GET_ENCAPSULATED_C_POINTER(obj) PyCObject_AsVoidPtr((obj))
#else
	#define ENCAPSULATE_C_POINTER(ptr) PyCapsule_New((ptr), nullptr, nullptr)
	#define IS_ENCAPSULATED_C_POINTER(obj) PyCapsule_CheckExact((obj))
	#define GET_ENCAPSULATED_C_POINTER(obj) PyCapsule_GetPointer((obj), nullptr)
#endif

/* Py_ssize_t is new in Python 2.5 */
#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

/** \brief	Structure to hold data for the pyQObject wrapper */
typedef struct {
	PyObject_HEAD
	/* Type-specific fields go here. */
	PyObject * _TWcontext;	///< pointer to the QObject wrapped by this object
} pyQObject;

/** \brief	Structure to hold data for the pyQObjectMethodObject wrapper */
typedef struct {
	PyObject_HEAD
	/* Type-specific fields go here. */
	PyObject * _TWcontext;	///< pointer to the QObject the method wrapped by this object belongs to
	PyObject * _methodName;	///< string describing the method name wrapped by this object
} pyQObjectMethodObject;
static PyTypeObject pyQObjectType;
static PyTypeObject pyQObjectMethodType;


static void QObjectDealloc(pyQObject * self) {
	Py_XDECREF(self->_TWcontext);
	((PyObject*)self)->ob_type->tp_free((PyObject*)self);
}
static void QObjectMethodDealloc(pyQObjectMethodObject * self) {
	Py_XDECREF(self->_TWcontext);
	Py_XDECREF(self->_methodName);
	((PyObject*)self)->ob_type->tp_free((PyObject*)self);
}

bool PythonScript::execute(ScriptAPIInterface * tw) const
{
	// Load the script
	QFile scriptFile(m_Filename);
	if (!scriptFile.open(QIODevice::ReadOnly)) {
		// handle error
		return false;
	}
	QString contents = m_Codec->toUnicode(scriptFile.readAll());
	scriptFile.close();

	// Python seems to require Unix style line endings
	if (contents.contains("\r"))
		contents.replace(QRegularExpression("\r\n?"), "\n");

	// Remember the current thread state so we can restore it at the end
	PyThreadState* origThreadState = PyThreadState_Get();

	// Create a separate sub-interpreter for this script
	PyThreadState* interpreter = Py_NewInterpreter();

	// Register the types
	if (!registerPythonTypes(tw->GetResult())) {
		Py_EndInterpreter(interpreter);
		// Restore the original thread state
		PyThreadState_Swap(origThreadState);
		return false;
	}
	
	pyQObject *TW;
	
	TW = (pyQObject*)QObjectToPython(tw->self());
	if (!TW) {
		tw->SetResult(tr("Could not create TW"));
		Py_EndInterpreter(interpreter);
		// Restore the original thread state
		PyThreadState_Swap(origThreadState);
		return false;
	}
	
	// Run the script
	PyObject * globals, * locals;
	globals = PyDict_New();
	locals = PyDict_New();
	
	// Create a dictionary of global variables
	// without the __builtins__ module, nothing would work!
	PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
	PyDict_SetItemString(globals, "TW", (PyObject*)TW);

	PyObject * ret = nullptr;
	
	if (globals && locals)
		ret = PyRun_String(qPrintable(contents), Py_file_input, globals, locals);
	
	Py_XDECREF(globals);
	Py_XDECREF(locals);
	Py_XDECREF(ret);
	Py_XDECREF(TW);

	// Check for exceptions
	if (PyErr_Occurred()) {
		PyObject * errType, * errValue, * errTraceback;
		PyErr_Fetch(&errType, &errValue, &errTraceback);
		
		PyObject * tmp = PyObject_Str(errValue);
		QString errString;
		if (!asQString(tmp, errString)) {
			Py_XDECREF(tmp);
			tw->SetResult(tr("Unknown error"));
			return false;
		}
		Py_XDECREF(tmp);
		tw->SetResult(errString);
		
		/////////////////////DEBUG
		// This prints the python error in the usual python way to stdout
		// Simply comment this block to prevent this behavior
		Py_XINCREF(errType);
		Py_XINCREF(errValue);
		Py_XINCREF(errTraceback);
		PyErr_Restore(errType, errValue, errTraceback);
		PyErr_Print();
		/////////////////////DEBUG
		
		Py_XDECREF(errType);
		Py_XDECREF(errValue);
		Py_XDECREF(errTraceback);

		Py_EndInterpreter(interpreter);
		// Restore the original thread state
		PyThreadState_Swap(origThreadState);
		return false;
	}

	// Finish
	Py_EndInterpreter(interpreter);

	// Restore the original thread state
	PyThreadState_Swap(origThreadState);
	return true;
}

bool PythonScript::registerPythonTypes(QVariant & errMsg) const
{
	// Register the Qobject wrapper
	pyQObjectType.tp_name = "QObject";
	pyQObjectType.tp_basicsize = sizeof(pyQObject);
	pyQObjectType.tp_dealloc = (destructor)QObjectDealloc;
	pyQObjectType.tp_flags = Py_TPFLAGS_DEFAULT;
	pyQObjectType.tp_doc = "QObject wrapper";
	pyQObjectType.tp_getattro = PythonScript::getAttribute;
	pyQObjectType.tp_setattro = PythonScript::setAttribute;
	
	if (PyType_Ready(&pyQObjectType) < 0) {
		errMsg = "Could not register QObject wrapper";
		return false;
	}

	// Register the TW method object
	pyQObjectMethodType.tp_name = "QObjectMethod";
	pyQObjectMethodType.tp_basicsize = sizeof(pyQObjectMethodObject);
	pyQObjectMethodType.tp_dealloc = (destructor)QObjectMethodDealloc;
	pyQObjectMethodType.tp_flags = Py_TPFLAGS_DEFAULT;
	pyQObjectMethodType.tp_doc = "QObject method wrapper";
	pyQObjectMethodType.tp_call = PythonScript::callMethod;
	
	
	if (PyType_Ready(&pyQObjectMethodType) < 0) {
		errMsg = "Could not register QObject method wrapper";
		return false;
	}
	return true;
}

/*static*/
PyObject * PythonScript::QObjectToPython(QObject * o)
{
	pyQObject * obj;
	obj = PyObject_New(pyQObject, &pyQObjectType);
	
	if (!obj) return nullptr;
	
	obj = (pyQObject*)PyObject_Init((PyObject*)obj, &pyQObjectType);
	obj->_TWcontext = ENCAPSULATE_C_POINTER(o);
	return (PyObject*)obj;
}

/*static*/
PyObject* PythonScript::getAttribute(PyObject * o, PyObject * attr_name)
{
	QObject * obj;
	QString propName;
	QVariant result;
	pyQObjectMethodObject * pyMethod;

	// Get the QObject* we operate on
	if (!PyObject_TypeCheck(o, &pyQObjectType)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("getattr: not a valid TW object")));
		return nullptr;
	}
	if (!IS_ENCAPSULATED_C_POINTER(((pyQObject*)o)->_TWcontext)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("getattr: not a valid TW object")));
		return nullptr;
	}
	obj = (QObject*)GET_ENCAPSULATED_C_POINTER((PyObject*)(((pyQObject*)o)->_TWcontext));
	
	if (!asQString(attr_name, propName)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("getattr: invalid property name")));
		return nullptr;
	}
	
	if (propName.length() > 1 && propName.endsWith(QChar('_')))
		propName.chop(1);
	
	switch (doGetProperty(obj, propName, result)) {
		case Property_DoesNotExist:
			PyErr_Format(PyExc_AttributeError, qPrintable(tr("getattr: object doesn't have property/method %s")), qPrintable(propName));
			return nullptr;
		case Property_NotReadable:
			PyErr_Format(PyExc_AttributeError, qPrintable(tr("getattr: property %s is not readable")), qPrintable(propName));
			return nullptr;
		case Property_Method:
			pyMethod = PyObject_New(pyQObjectMethodObject, &pyQObjectMethodType);
			pyMethod = (pyQObjectMethodObject*)PyObject_Init((PyObject*)pyMethod, &pyQObjectMethodType);
			Py_INCREF(pyMethod);
			pyMethod->_TWcontext = ENCAPSULATE_C_POINTER(obj);
			Py_XINCREF(attr_name);
			pyMethod->_methodName = (PyObject*)attr_name;
			return (PyObject*)pyMethod;
		case Property_OK:
			return PythonScript::VariantToPython(result);
		default:
			break;
	}
	// we should never reach this point
	return nullptr;
}

/*static*/
int PythonScript::setAttribute(PyObject * o, PyObject * attr_name, PyObject * v)
{
	QObject * obj;
	QString propName;
	QMetaProperty prop;

	// Get the QObject* we operate on
	if (!PyObject_TypeCheck(o, &pyQObjectType)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("setattr: not a valid TW object")));
		return -1;
	}
	if (!IS_ENCAPSULATED_C_POINTER(((pyQObject*)o)->_TWcontext)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("setattr: not a valid TW object")));
		return -1;
	}
	obj = (QObject*)GET_ENCAPSULATED_C_POINTER((PyObject*)(((pyQObject*)o)->_TWcontext));

	// Get the parameters
	if (!asQString(attr_name, propName)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("setattr: invalid property name")));
		return -1;
	}

	switch (doSetProperty(obj, propName, PythonScript::PythonToVariant(v))) {
		case Property_DoesNotExist:
			PyErr_Format(PyExc_AttributeError, qPrintable(tr("setattr: object doesn't have property %s")), qPrintable(propName));
			return -1;
		case Property_NotWritable:
			PyErr_Format(PyExc_AttributeError, qPrintable(tr("setattr: property %s is not writable")), qPrintable(propName));
			return -1;
		case Property_OK:
			return 0;
		default:
			break;
	}
	// we should never reach this point
	return -1;
}

/*static*/
PyObject * PythonScript::callMethod(PyObject * o, PyObject * pyArgs, PyObject * kw)
{
	Q_UNUSED(kw)
	QObject * obj;
	QString methodName;
	QVariantList args;
	QVariant result;
	int i;
	
	// Get the QObject* we operate on
	obj = (QObject*)GET_ENCAPSULATED_C_POINTER((PyObject*)(((pyQObjectMethodObject*)o)->_TWcontext));

	if (!asQString((PyObject*)(((pyQObjectMethodObject*)o)->_methodName), methodName)) {
		PyErr_SetString(PyExc_TypeError, qPrintable(tr("call: invalid method name")));
		return nullptr;
	}
	
	for (i = 0; i < PyTuple_Size(pyArgs); ++i) {
		args.append(PythonScript::PythonToVariant(PyTuple_GetItem(pyArgs, i)));
	}
	if (methodName.length() > 1 && methodName.endsWith(QChar('_')))
		methodName.chop(1);
	switch (doCallMethod(obj, methodName, args, result)) {
		case Method_OK:
			return PythonScript::VariantToPython(result);
		case Method_DoesNotExist:
			PyErr_Format(PyExc_TypeError, qPrintable(tr("call: the method %s doesn't exist")), qPrintable(methodName));
			return nullptr;
		case Method_WrongArgs:
			PyErr_Format(PyExc_TypeError, qPrintable(tr("call: couldn't call %s with the given arguments")), qPrintable(methodName));
			return nullptr;
		case Method_Failed:
			PyErr_Format(PyExc_TypeError, qPrintable(tr("call: internal error while executing %s")), qPrintable(methodName));
			return nullptr;
		default:
			break;
	}
	
	// we should never reach this point
	return nullptr;
}


/*static*/
PyObject * PythonScript::VariantToPython(const QVariant & v)
{
	int i;
	QVariantList::const_iterator iList;
	QVariantList list;
	QVariantHash::const_iterator iHash;
	QVariantHash hash;
	QVariantMap::const_iterator iMap;
	QVariantMap map;
	PyObject * pyList, * pyDict;

	if (v.isNull()) Py_RETURN_NONE;

	switch (static_cast<int>(v.type())) {
		case QVariant::Double:
			return Py_BuildValue("d", v.toDouble());
		case QVariant::Bool:
			if (v.toBool()) Py_RETURN_TRUE;
			else Py_RETURN_FALSE;
		case QVariant::Int:
			return Py_BuildValue("i", v.toInt());
		case QVariant::LongLong:
			return Py_BuildValue("L", v.toLongLong());
		case QVariant::UInt:
			return Py_BuildValue("I", v.toUInt());
		case QVariant::ULongLong:
			return Py_BuildValue("K", v.toULongLong());
		case QVariant::Char:
		case QVariant::String:
#ifdef Py_UNICODE_WIDE
			{
				QVector<uint> tmp = v.toString().toUcs4();
				return Py_BuildValue("u#", tmp.constData(), tmp.count());
			}
#else
			return Py_BuildValue("u", v.toString().constData());
#endif
		case QVariant::List:
		case QVariant::StringList:
			list = v.toList();

			pyList = PyList_New(list.size());
			for (i = 0, iList = list.begin(); iList != list.end(); ++iList, ++i) {
				PyList_SetItem(pyList, i, PythonScript::VariantToPython(*iList));
			}
			return pyList;
		case QVariant::Hash:
			hash = v.toHash();
			
			pyDict = PyDict_New();
			for (iHash = hash.begin(); iHash != hash.end(); ++iHash) {
				PyDict_SetItemString(pyDict, qPrintable(iHash.key()), PythonScript::VariantToPython(iHash.value()));
			}
			return pyDict;
		case QVariant::Map:
			map = v.toMap();
			
			pyDict = PyDict_New();
			for (iMap = map.begin(); iMap != map.end(); ++iMap) {
				PyDict_SetItemString(pyDict, qPrintable(iMap.key()), PythonScript::VariantToPython(iMap.value()));
			}
			return pyDict;
		case QMetaType::QObjectStar:
			return PythonScript::QObjectToPython(v.value<QObject*>());
		default:
			PyErr_Format(PyExc_TypeError, qPrintable(tr("the type %s is currently not supported")), v.typeName());
			return nullptr;
	}
	Py_RETURN_NONE;
}

/*static*/
QVariant PythonScript::PythonToVariant(PyObject * o)
{
	QVariantList list;
	QVariantMap map;
	PyObject * key, * value;
	Py_ssize_t i = 0;
	QString str;

	if (o == Py_None)
		return QVariant();
	// in Python 3.x, the PyInt_* were removed in favor of PyLong_*
#if PY_MAJOR_VERSION < 3
	if (PyInt_Check(o)) return QVariant((int)PyInt_AsLong(o));
#endif
	if (PyBool_Check(o)) return QVariant((o == Py_True));
	if (PyLong_Check(o)) return QVariant((qlonglong)PyLong_AsLong(o));
	if (PyFloat_Check(o)) return QVariant(PyFloat_AsDouble(o));
	if (asQString(o, str)) return str;
	if (PyTuple_Check(o)) {
		for (i = 0; i < PyTuple_Size(o); ++i) {
			list.append(PythonToVariant(PyTuple_GetItem(o, i)));
		}
		return list;
	}
	if (PyList_Check(o)) {
		for (i = 0; i < PyList_Size(o); ++i) {
			list.append(PythonToVariant(PyList_GetItem(o, i)));
		}
		return list;
	}
	if (PyDict_Check(o)) {
		while (PyDict_Next(o, &i, &key, &value)) {
			map.insert(PythonScript::PythonToVariant(key).toString(), PythonScript::PythonToVariant(value));
		}
		return map;
	}
	if (PyObject_TypeCheck(o, &pyQObjectType)) {
		return QVariant::fromValue((QObject*)GET_ENCAPSULATED_C_POINTER(((pyQObject*)o)->_TWcontext));
	}
	// \TODO Complex numbers, byte arrays
	PyErr_Format(PyExc_TypeError, qPrintable(tr("the python type %s is currently not supported")), o->ob_type->tp_name);
	return QVariant();
}

/*static*/
bool PythonScript::asQString(PyObject * obj, QString & str)
{
	// Get the parameters
	// In Python 3.x, the PyString_* were replaced by PyBytes_*
#if PY_MAJOR_VERSION < 3
	if (PyString_Check(obj)) {
		str = PyString_AsString(obj);
		return true;
	}
#else
	if (PyBytes_Check(obj)) {
		str = PyBytes_AsString(obj);
		return true;
	}
#endif
	if (PyUnicode_Check(obj)) {
		PyObject * tmp = PyUnicode_AsUTF8String(obj);
#if PY_MAJOR_VERSION < 3
		str = QString::fromUtf8(PyString_AsString(tmp));
#else
		str = QString::fromUtf8(PyBytes_AsString(tmp));
#endif
		Py_XDECREF(tmp);
		return true;
	}
	return false;
}

} // namespace Scripting
} // namespace Tw
