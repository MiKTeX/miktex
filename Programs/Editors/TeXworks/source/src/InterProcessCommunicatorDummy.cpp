/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Stefan Löffler

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
#include "InterProcessCommunicator.h"

// This is a dummy implementation; all instances are assumed to be first
// instances, and any call to `send` functions directly raises the corresponding
// `received` function

namespace Tw {

class InterProcessCommunicatorPrivate {
};

InterProcessCommunicator::InterProcessCommunicator()
	: _private(nullptr)
{
}

InterProcessCommunicator::~InterProcessCommunicator() = default;

void InterProcessCommunicator::sendBringToFront()
{
	emit receivedBringToFront();
}

void InterProcessCommunicator::sendOpenFile(const QString & path, const int position)
{
	emit receivedOpenFile(path, position);
}

} // namespace Tw
