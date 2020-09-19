/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef PythonScriptInterface_H
#define PythonScriptInterface_H

#include "scripting/Script.h"
#include "scripting/ScriptLanguageInterface.h"

namespace Tw {
namespace Scripting {

/** \brief Implementation of the script plugin interface */
class PythonScriptInterface : public QObject, public ScriptLanguageInterface
{
	Q_OBJECT
	Q_INTERFACES(Tw::Scripting::ScriptLanguageInterface)
	Q_PLUGIN_METADATA(IID "org.tug.texworks.ScriptPlugins.PythonPlugin")

public:
	/** \brief Constructor
	 *
	 * Initializes the python instance
	 */
	PythonScriptInterface();

	/** \brief Destructor
	 *
	 * Finalizes the python instance
	 */
	~PythonScriptInterface() override;

	/** \brief Script factory
	 *
	 * \return	pointer to a new PythonScript object cast to Tw::Scripting::Script as the
	 * 			interface requires; the caller owns the object and must delete
	 * 			it.
	 */
	Script * newScript(const QString& fileName) override;

	/** \brief	Get the supported script language name
	 *
	 * \return	the name of the scripting language
	 */
	QString scriptLanguageName() const override { return QStringLiteral("Python"); }

	/** \brief	Get a URL for information on the supported script language
	 *
	 * \return	a string with a URL for information about the language
	 */
	QString scriptLanguageURL() const override { return QStringLiteral("http://www.python.org/"); }

	/** \brief  Return whether the given file is handled by this scripting language plugin
	 */
	bool canHandleFile(const QFileInfo& fileInfo) const override { return fileInfo.suffix() == QStringLiteral("py"); }
};

} // namespace Scripting
} // namespace Tw

#endif // !defined(PythonScriptInterface_H)
