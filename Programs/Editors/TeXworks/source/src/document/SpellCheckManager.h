/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019-2025  Stefan Löffler

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	For links to further information, or to contact the authors,
	see <https://tug.org/texworks/>.
*/
#ifndef SpellCheckManager_H
#define SpellCheckManager_H

#include <memory>
#include <QHash>
#include <QObject>
#include <QTextCodec>

struct Hunhandle;

namespace Tw {
namespace Document {

class SpellChecker;

class SpellCheckManager : public QObject {
	Q_OBJECT

	friend class SpellChecker;

	SpellCheckManager() = default;
	~SpellCheckManager() override = default;
	SpellCheckManager(const SpellCheckManager &) = delete;
	SpellCheckManager(SpellCheckManager &&) = delete;
	SpellCheckManager & operator=(const SpellCheckManager &) = delete;
	SpellCheckManager & operator=(SpellCheckManager &&) = delete;

	static std::shared_ptr<Hunhandle> getDictionary(const QString & language);

public:
	static SpellCheckManager * instance() { return _instance; }
	static QString labelForDict(QString & dict);

	// get list of available dictionaries
	static QMultiHash<QString, QString> * getDictionaryList(const bool forceReload = false);

	// deallocates all dictionaries
	// WARNING: Don't call this while some window is using a dictionary as that
	// window won't be notified; deactivate spell checking in all windows first
	// (see TWApp::reloadSpellchecker())
	static void clearDictionaries();

signals:
	// emitted when getDictionaryList reloads the dictionary list;
	// windows can connect to it to rebuild, e.g., a spellchecking menu
	void dictionaryListChanged() const;

private:
	static SpellCheckManager * _instance;
	static QMultiHash<QString, QString> * dictionaryList;
	static QHash<const QString,std::shared_ptr<Hunhandle>> * dictionaries;
};

} // namespace Document
} // namespace Tw

#endif // !defined(SpellCheckManager_H)
