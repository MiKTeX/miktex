/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2022  Jonathan Kew, Stefan Löffler

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

#ifndef Utils_CmdKeyFilter_H
#define Utils_CmdKeyFilter_H

#include <QObject>

namespace Tw {

namespace Utils {

// filter used to stop Command-keys getting inserted into edit text items
// (only used on Mac OS X)
class CmdKeyFilter: public QObject
{
	Q_OBJECT

public:
	static CmdKeyFilter *filter();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	static CmdKeyFilter *filterObj;
};

} // namespace Utils

} // namespace Tw

#endif // !defined(Utils_CmdKeyFilter_H)
