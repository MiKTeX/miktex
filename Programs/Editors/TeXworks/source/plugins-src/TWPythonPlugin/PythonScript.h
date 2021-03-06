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

#ifndef PythonScript_H
#define PythonScript_H

#include "PythonScriptInterface.h"
#include "scripting/Script.h"
#include "scripting/ScriptAPIInterface.h"

// Forward declaration taken from the Python headers to avoid having to include
// Python in this header file
struct _object;
typedef _object PyObject;

namespace Tw {
namespace Scripting {

/** \brief Class for handling python scripts */
class PythonScript : public Script
{
	Q_OBJECT
	Q_INTERFACES(Tw::Scripting::Script)

public:
	/** \brief Constructor
	 *
	 * Does nothing
	 */
	PythonScript(PythonScriptInterface * interface, const QString& fileName)
		: Script(interface, fileName) { }

	/** \brief Parse the script header
	 *
	 * \return	\c true if successful, \c false if not (e.g. because the file
	 * 			is no valid Tw python script)
	 */
	bool parseHeader() override { return doParseHeader("", "", "#"); }

protected:
	/** \brief Run the python script
	 *
	 * \note	Every python script is run in its own interpreter.
	 *
	 * \param	tw	the TW interface object, exposed to the script as the TW global
     *
	 * \return	\c true on success, \c false if an error occured
	 */
	bool execute(ScriptAPIInterface *tw) const override;

	/** \brief Handler for attribute requests on QObjects
	 *
	 * \param	o			the pyQObject of which to retrieve the attribute value
	 * \param	attr_name	the name of the attribute
	 * \return	the python value on success, \c nullptr if an error occured
	 */
	static PyObject* getAttribute(PyObject * o, PyObject * attr_name);

	/** \brief Handler for setting attribute values on QObjects
	 *
	 * \param	o			the pyQObject for which to set the attribute value
	 * \param	attr_name	the name of the attribute
	 * \param	v			the new value
	 * \return	0 on success, -1 if an error occured
	 */
	static int setAttribute(PyObject * o, PyObject * attr_name, PyObject * v);

	/** \brief Handler for calling methods of QObjects
	 *
	 * \note	Calling by keywords is currently not supported
	 * \param	o		the pyQObjectMethodObject to call
	 * \param	pyArgs	python tuple of arguments
	 * \param	kw		dictionary of key-value argument pairs (not supported)
	 * \return	the return value of the method (PyNone for void functions) on
	 * 			success, \c nullptr if an error occured
	 */
	static PyObject * callMethod(PyObject * o, PyObject * pyArgs, PyObject * kw);

	/** \brief Convenience function to convert a QObject to a pyQObject
	 *
	 * \param	o	the QObject to expose to python
	 * \return	the pyQObject that can be used in python
	 */
	static PyObject * QObjectToPython(QObject * o);

	/** \brief Convenience function to convert a QVariant to a python object
	 *
	 * \note	QObject* instances will be converted by QObjectToPython. Empty
	 * 			variants will be converted to PyNone. QList will be converted to
	 * 			python lists. If the value can't be converted, an error is
	 * 			raised and \c nullptr is returned.
	 * \param	v	the QVariant to convert to a python value
	 * \return	the python object on success, \c nullptr if an error occured
	 */
	static PyObject * VariantToPython(const QVariant & v);

	/** \brief Convenience function to convert a python object to a QVariant
	 *
	 * \note	Python tuples and lists are converted to QList. pyQObject is not
	 * 			supported. If the value can't be converted, an error is raised
	 * 			and an empty QVariant is returned.
	 * \param	o	the python object to convert
	 * \return	the QVariant
	 */
	static QVariant PythonToVariant(PyObject * o);

	/** \brief Register Tw-specific python types
	 *
	 * Registers pyQObject and pyQObjectMethodObject for use in python.
	 * \param	errMsg	if an error occurs this variable receives a string
	 * 					describing it
	 * \return	\c true on succes, \c false otherwise
	 */
	bool registerPythonTypes(QVariant & errMsg) const;

	/** \brief	Convenience function to convert a python object to a QString
	 *
	 * This function handles conversion of PyBytes and PyUnicode objects.
	 * \param	obj	python object to convert to a QString
	 * \param	str	QString to receive the string on success
	 * \return	\c true on succes, \c false otherwise
	 */
	static bool asQString(PyObject * obj, QString & str);
};

} // namespace Scripting
} // namespace Tw

#endif // !defined(PythonScript_H)
