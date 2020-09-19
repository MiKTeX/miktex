/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "PythonScriptInterface.h"
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

namespace Tw {
namespace Scripting {

PythonScriptInterface::PythonScriptInterface()
{
	// Initialize the python interpretor
	Py_Initialize();
}

PythonScriptInterface::~PythonScriptInterface()
{
	// Uninitialize the python interpreter
	Py_Finalize();
}

Script * PythonScriptInterface::newScript(const QString& fileName)
{
	return new PythonScript(this, fileName);
}

} // namespace Scripting
} // namespace Tw
