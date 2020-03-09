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

#ifndef JSScript_H
#define JSScript_H

#include "scripting/Script.h"

namespace Tw {
namespace Scripting {

class JSScript : public Script
{
	Q_OBJECT
	Q_INTERFACES(Tw::Scripting::Script)

public:
	JSScript(QObject * plugin, const QString& filename)
		: Tw::Scripting::Script(plugin, filename) { }

	bool parseHeader() override { return doParseHeader(QString(), QString(), QString::fromLatin1("//")); }

protected:
	bool execute(ScriptAPIInterface *tw) const override;
};

} // namespace Scripting
} // namespace Tw

#endif // !defined(JSScript_H)
