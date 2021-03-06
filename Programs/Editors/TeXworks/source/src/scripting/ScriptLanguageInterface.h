/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019-2020  Stefan Löffler

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

#ifndef ScriptLanguageInterface_H
#define ScriptLanguageInterface_H

#include <QFileInfo>
#include <QString>

namespace Tw {
namespace Scripting {

class Script;

/** \brief	Interface all TW scripting plugins must implement */
class ScriptLanguageInterface
{
public:
	virtual ~ScriptLanguageInterface() = default;

	/** \brief	Method to create a new script wrapper
	 *
	 * This method must be implemented in derived classes;
	 * it should create the script wrapper for the given file.
	 * This method does NOT call parseHeader(), so until this
	 * is done, the script is not necessarily valid.
	 * \return	the script wrapper, or nullptr if the file cannot be found
	 */
	virtual Tw::Scripting::Script* newScript(const QString& fileName) = 0;

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

} // namespace Scripting
} // namespace Tw

Q_DECLARE_INTERFACE(Tw::Scripting::ScriptLanguageInterface, "org.tug.texworks.ScriptLanguageInterface/0.3.2")

#endif // defined(ScriptLanguageInterface_H)
