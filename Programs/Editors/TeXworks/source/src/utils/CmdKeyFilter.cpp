/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2022  Jonathan Kew, Stefan Löffler

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

#include "CmdKeyFilter.h"

#include <QKeyEvent>

namespace Tw {

namespace Utils {

// on OS X only, the singleton CmdKeyFilter object is attached to all TeXDocument editor widgets
// to stop Command-keys getting inserted into edit text items

CmdKeyFilter *CmdKeyFilter::filterObj = nullptr;

CmdKeyFilter *CmdKeyFilter::filter()
{
	if (!filterObj)
		filterObj = new CmdKeyFilter;
	return filterObj;
}

bool CmdKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
#if defined(Q_OS_DARWIN)
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if ((keyEvent->modifiers() & Qt::ControlModifier) != 0) {
			if (keyEvent->key() <= 0x0ff
				&& keyEvent->key() != Qt::Key_Z
				&& keyEvent->key() != Qt::Key_X
				&& keyEvent->key() != Qt::Key_C
				&& keyEvent->key() != Qt::Key_V
				&& keyEvent->key() != Qt::Key_A)
				return true;
		}
	}
#endif
	return QObject::eventFilter(obj, event);
}

} // namespace Utils

} // namespace Tw
