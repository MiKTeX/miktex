/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019  Stefan Löffler

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
#ifndef Document_Document_H
#define Document_Document_H

#include <QFileInfo>

namespace Tw {
namespace Document {

class Document
{
public:
	virtual ~Document() = default;

	virtual QFileInfo getFileInfo() const { return _fileInfo; }
	virtual void setFileInfo(const QFileInfo & fileInfo) { _fileInfo = fileInfo; }
	// Returns whether this document is based on an existing file (e.g., loaded
	// from the file referenced by getFileInfo() or at some point saved to that
	// file.
	bool isStoredInFilesystem() const { return _isStoredInFilesystem; }
	// FIXME: _isStoredInFilesystem should actually be set by load and save
	// methods
	/// \deprecated
	void setStoredInFilesystem(const bool isStored = true) { _isStoredInFilesystem = isStored; }
	virtual QString absoluteFilePath() const {
		if (!_fileInfo.filePath().isEmpty())
			return _fileInfo.absoluteFilePath();
		return {};
	}
protected:
	bool _isStoredInFilesystem{false};
	QFileInfo _fileInfo;
};

} // namespace Document
} // namespace Tw

#endif // !defined(Document_Document_H)
