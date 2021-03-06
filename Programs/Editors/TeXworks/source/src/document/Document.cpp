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

#include "document/Document.h"

#include <QImage>

namespace Tw {
namespace Document {

// static
bool isPDFfile(const QString& fileName)
{
	QFile theFile(fileName);
	if (theFile.open(QIODevice::ReadOnly)) {
		QByteArray ba = theFile.peek(8);
		if (ba.startsWith("%PDF-1."))
			return true;
	}
	return false;
}

// static
bool isImageFile(const QString& fileName)
{
	QImage image(fileName);
	return !image.isNull();
}

// static
bool isPostscriptFile(const QString& fileName)
{
	QFile theFile(fileName);
	if (theFile.open(QIODevice::ReadOnly)) {
		QByteArray ba = theFile.peek(4);
		if (ba.startsWith("%!PS"))
			return true;
	}
	return false;
}

} // namespace Document
} // namespace Tw
