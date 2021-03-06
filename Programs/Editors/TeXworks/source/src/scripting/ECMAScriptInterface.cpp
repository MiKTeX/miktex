/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2020  Stefan Löffler

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

#include "scripting/ECMAScriptInterface.h"
#include "scripting/ECMAScript.h"

namespace Tw {
namespace Scripting {

Script* ECMAScriptInterface::newScript(const QString& fileName)
{
	return new ECMAScript(this, fileName);
}

bool ECMAScriptInterface::canHandleFile(const QFileInfo& fileInfo) const
{
	if (fileInfo.suffix() != QStringLiteral("es") && fileInfo.suffix() != QStringLiteral("js")) {
		return false;
	}
	return true;
}

} // namespace Scripting
} // namespace Tw
