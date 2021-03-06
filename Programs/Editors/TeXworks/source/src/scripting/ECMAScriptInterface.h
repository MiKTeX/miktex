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

#ifndef ECMAScriptInterface_H
#define ECMAScriptInterface_H

#include "scripting/Script.h"
#include "scripting/ScriptLanguageInterface.h"

namespace Tw {
namespace Scripting {

class ECMAScriptInterface : public QObject, public ScriptLanguageInterface
{
	Q_OBJECT
	Q_INTERFACES(Tw::Scripting::ScriptLanguageInterface)

public:
	ECMAScriptInterface() = default;
	~ECMAScriptInterface() override = default;

	Script* newScript(const QString& fileName) override;

	QString scriptLanguageName() const override { return QStringLiteral("ECMAScript"); }
	QString scriptLanguageURL() const override { return QStringLiteral("https://doc.qt.io/qt-5/qjsengine.html"); }
	bool canHandleFile(const QFileInfo& fileInfo) const override;
};

} // namespace Scripting
} // namespace Tw

#endif // !defined(ECMAScriptInterface_H)
