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
#ifndef SpellChecker_H
#define SpellChecker_H

#include <QObject>
#include <QHash>
#include <QTextCodec>

struct Hunhandle;

namespace Tw {
namespace Document {

class SpellChecker : public QObject {
	Q_OBJECT

	SpellChecker() = default;
	~SpellChecker() override = default;
	SpellChecker(const SpellChecker & other) = delete;
	SpellChecker & operator=(const SpellChecker & other) = delete;

public:
	class Dictionary {
		friend class SpellChecker;

		QString _language;
		Hunhandle * _hunhandle;
		QTextCodec * _codec;

		Dictionary(const QString & language, Hunhandle * hunhandle);
	public:
		virtual ~Dictionary();
		QString getLanguage() const { return _language; }
		bool isWordCorrect(const QString & word) const;
		QList<QString> suggestionsForWord(const QString & word) const;
		// note that this is not persistent after quitting TW
		void ignoreWord(const QString & word);
	};

	static SpellChecker * instance() { return _instance; }

	// get list of available dictionaries
	static QHash<QString, QString> * getDictionaryList(const bool forceReload = false);

	// get dictionary for a given language
	static Dictionary * getDictionary(const QString& language);
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
	static SpellChecker * _instance;
	static QHash<QString, QString> * dictionaryList;
	static QHash<const QString,SpellChecker::Dictionary*> * dictionaries;
};

} // namespace Document
} // namespace Tw

#endif // !defined(SpellChecker_H)
