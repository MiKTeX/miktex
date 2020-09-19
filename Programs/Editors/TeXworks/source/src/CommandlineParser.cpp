/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "CommandlineParser.h"
#include <QFileInfo>

bool CommandlineParser::parse()
{
	foreach (const QString& rawItem, m_rawItems) {
		bool found = false;
		
		foreach (const CommandlineItemSpec& spec, m_specs) {
			CommandlineItem item;
			item.type = spec.type;
			item.longName = spec.longName;
			item.processed = false;

			QString strLong = QLatin1String("--") + spec.longName;
			QString strShort = QLatin1String("-") + spec.shortName;
			
			switch (spec.type) {
				case Commandline_Option:
					if (!spec.longName.isEmpty() && rawItem.startsWith(strLong + QLatin1String("="))) {
						item.value = rawItem.mid(strLong.length() + 1);
						found = true;
					}
					else if (!spec.shortName.isEmpty() && rawItem.startsWith(strShort + QLatin1String("="))) {
						item.value = rawItem.mid(strShort.length() + 1);
						found = true;
					}
					break;
				case Commandline_Switch:
					if (!spec.longName.isEmpty() && rawItem == strLong)
						found = true;
					else if (!spec.shortName.isEmpty() && rawItem == strShort)
						found = true;
					break;
				default:
					break;
			}
			
			if (found) {
				m_items << item;
				break;
			}
		}
		
		if (!found) {
			CommandlineItem item;
			item.type = Commandline_Argument;
			item.processed = false;
			item.value = rawItem;
			m_items << item;
		}
	}

	return true;
}

void CommandlineParser::printUsage(QTextStream & stream)
{
	stream << "Usage: " << QFileInfo(QCoreApplication::applicationFilePath()).fileName() << " [opts/args]\n\n";

	foreach (const CommandlineItemSpec& spec, m_specs) {
		switch (spec.type) {
			case Commandline_Option:
				stream << "   --" << spec.longName << "=...";
				if (!spec.shortName.isEmpty())
					stream << ", -" << spec.shortName << "=...";
				break;
			case Commandline_Switch:
				stream << "   --" << spec.longName;
				if (!spec.shortName.isEmpty())
					stream << ", -" << spec.shortName;
				break;
			default:
				continue;
		}
		stream << "   " << spec.description << "\n";
	}
	stream.flush();
}

int CommandlineParser::getNextArgument(int index /* = -1 */, CommandlineGetItemFlags flags /* = GetFlag_Default */) const
{
	for (++index; index < m_items.size(); ++index) {
		if (m_items[index].type == Commandline_Argument) {
			if ((flags & GetFlag_IncludeProcessed) || !m_items[index].processed)
				return index;
		}
	}
	return -1;
}

int CommandlineParser::getPrevArgument(int index /* = -1 */, CommandlineGetItemFlags flags /* = GetFlag_Default */) const
{
	for (--index; index >= 0; --index) {
		if (m_items[index].type == Commandline_Argument) {
			if ((flags & GetFlag_IncludeProcessed) || !m_items[index].processed)
				return index;
		}
	}
	return -1;
}

int CommandlineParser::getNextOption(
	const QString & longName /* = QString() */,
	int index /* = -1 */,
	CommandlineGetItemFlags flags /* = GetFlag_Default */
) const
{
	for (++index; index < m_items.size(); ++index) {
		if (m_items[index].type == Commandline_Option && m_items[index].longName == longName) {
			if ((flags & GetFlag_IncludeProcessed) || !m_items[index].processed)
				return index;
		}
	}
	return -1;
}

int CommandlineParser::getPrevOption(
	const QString & longName /* = QString() */,
	int index /* = -1 */,
	CommandlineGetItemFlags flags /* = GetFlag_Default */
) const
{
	for (--index; index >= 0; --index) {
		if (m_items[index].type == Commandline_Option && m_items[index].longName == longName) {
			if ((flags & GetFlag_IncludeProcessed) || !m_items[index].processed)
				return index;
		}
	}
	return -1;
}

int CommandlineParser::getNextSwitch(
	const QString & longName /* = QString() */,
	int index /* = -1 */,
	CommandlineGetItemFlags flags /* = GetFlag_Default */
) const
{
	for (++index; index < m_items.size(); ++index) {
		if (m_items[index].type == Commandline_Switch && m_items[index].longName == longName) {
			if ((flags & GetFlag_IncludeProcessed) || !m_items[index].processed)
				return index;
		}
	}
	return -1;
}

int CommandlineParser::getPrevSwitch(
	const QString & longName /* = QString() */,
	int index /* = -1 */,
	CommandlineGetItemFlags flags /* = GetFlag_Default */
) const
{
	for (--index; index >= 0; --index) {
		if (m_items[index].type == Commandline_Switch && m_items[index].longName == longName) {
			if ((flags & GetFlag_IncludeProcessed) || !m_items[index].processed)
				return index;
		}
	}
	return -1;
}
