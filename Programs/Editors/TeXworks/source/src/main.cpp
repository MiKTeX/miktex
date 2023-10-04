/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2023  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TWApp.h"
#if defined(MIKTEX)
#include <miktex/Core/Utils>
#include "miktex/miktex-texworks.hpp"
#endif

#if defined(STATIC_QT5) && defined(Q_OS_WIN)
  #include <QtPlugin>
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

struct fileToOpenStruct{
	QString filename;
	int position;
};

#if defined(MIKTEX)
#  define main Main
#endif
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#	if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#	endif
#endif
	TWApp app(argc, argv);
	return app.exec();
}
#if defined(MIKTEX)
#undef main
int main(int argc, char* argv[])
{
  MiKTeX::TeXworks::Wrapper wrapper;
  return wrapper.Run(Main, argc, argv);
}
#endif
