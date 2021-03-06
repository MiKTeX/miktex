/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef COMMANDLINE_PARSER_H
#define COMMANDLINE_PARSER_H

#include <QCoreApplication>
#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QVariant>

namespace Tw {
namespace Utils {

class CommandlineParser
{
public:
	enum CommandlineItemType { Commandline_Argument, Commandline_Option, Commandline_Switch };
	enum CommandlineGetItemFlags { GetFlag_Default = 0, GetFlag_IncludeProcessed = 1 };

	struct CommandlineItem {
		CommandlineItemType type;
		QString longName;
		QVariant value;
		bool processed;
	};

	// First item is app name
	CommandlineParser() : m_rawItems(QCoreApplication::arguments().mid(1)) { }
	explicit CommandlineParser(const QStringList & argv) : m_rawItems(argv.mid(1)) { }

	void registerSwitch(const QString longName, const QString description = QString(), const QString shortName = QString()) {
		CommandlineItemSpec spec;
		spec.type = Commandline_Switch;
		spec.longName = longName;
		spec.shortName = shortName;
		spec.description = description;
		m_specs << spec;
	}

	void registerOption(const QString longName, const QString description = QString(), const QString shortName = QString()) {
		CommandlineItemSpec spec;
		spec.type = Commandline_Option;
		spec.longName = longName;
		spec.shortName = shortName;
		spec.description = description;
		m_specs << spec;
	}

	int getNextArgument(int index = -1, CommandlineGetItemFlags flags = GetFlag_Default) const;
	int getPrevArgument(int index = -1, CommandlineGetItemFlags flags = GetFlag_Default) const;
	int getNextOption(const QString & longName = QString(), int index = -1, CommandlineGetItemFlags flags = GetFlag_Default) const;
	int getPrevOption(const QString & longName = QString(), int index = -1, CommandlineGetItemFlags flags = GetFlag_Default) const;
	int getNextSwitch(const QString & longName = QString(), int index = -1, CommandlineGetItemFlags flags = GetFlag_Default) const;
	int getPrevSwitch(const QString & longName = QString(), int index = -1, CommandlineGetItemFlags flags = GetFlag_Default) const;

	CommandlineItem & at(int idx) {
		return m_items[idx];
	}

	bool parse();
	void printUsage(QTextStream &stream);

private:
	struct CommandlineItemSpec {
		CommandlineItemType type;
		QString shortName;
		QString longName;
		QString description;
	};

	QStringList m_rawItems;
	QList<CommandlineItem> m_items;
	QList<CommandlineItemSpec> m_specs;
};

} // namespace Utils
} // namespace Tw

#endif // !defined(COMMANDLINE_PARSER_H)
