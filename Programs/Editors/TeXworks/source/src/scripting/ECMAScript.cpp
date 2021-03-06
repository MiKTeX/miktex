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
#include "scripting/ECMAScript.h"
#include "scripting/ScriptAPIInterface.h"

#include <QJSEngine>

namespace Tw {
namespace Scripting {

bool ECMAScript::execute(ScriptAPIInterface *tw) const
{
	QFile scriptFile(m_Filename);
	if (!scriptFile.open(QIODevice::ReadOnly)) {
		// handle error
		return false;
	}
	QString contents = m_Codec->toUnicode(scriptFile.readAll());
	scriptFile.close();


	QJSEngine engine;
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	engine.installExtensions(QJSEngine::AllExtensions);
#endif
	QJSValue twObject = engine.newQObject(tw->clone());
	engine.globalObject().setProperty(QString::fromLatin1("TW"), twObject);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QJSValue val = engine.evaluate(contents, m_Filename);
#else
	QStringList exceptionStackTrace;
	QJSValue val = engine.evaluate(contents, m_Filename, 1, &exceptionStackTrace);
#endif

	if (val.isError()) {
		tw->SetResult(val.toString() +
									tr("\n\nStack trace:\n") +
									val.property(QStringLiteral("stack")).toString());
		return false;
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	if (!exceptionStackTrace.isEmpty()) {
		tw->SetResult(val.toString());
		return false;
	}
#endif
	if (!val.isUndefined())
		tw->SetResult(val.toVariant());
	return true;
}

} // namespace Scripting
} // namespace Tw
