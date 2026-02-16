#include "SpellChecker.h"

#include "SpellCheckManager.h"

#include <hunspell.h>

namespace Tw {
namespace Document {

std::shared_ptr<Hunhandle> SpellChecker::DictRef::getHunhandle() const
{
	std::shared_ptr<Hunhandle> retVal = hunhandle.lock();
	if (!retVal) {
		retVal = SpellCheckManager::getDictionary(language);
		hunhandle = retVal;
	}
	return retVal;
}

bool SpellChecker::DictRef::operator==(const DictRef & other) const
{
	return (language == other.language && getHunhandle() == other.getHunhandle() && codec == other.codec);
}

bool SpellChecker::DictRef::isValid() const
{
	return (!language.isEmpty() && codec != nullptr && getHunhandle());
}

SpellChecker::SpellChecker(const QString & language)
	: SpellChecker()
{
	setLanguages(QStringList{language});
}

bool SpellChecker::operator==(const SpellChecker & other) const
{
	return m_dicts == other.m_dicts;
}

bool SpellChecker::isValid() const
{
	// Return true if we have (at least) one valid dictionary
	for (const DictRef & dictRef : m_dicts) {
		if (dictRef) {
			return true;
		}
	}
	return false;
}

QStringList SpellChecker::languages() const
{
	QStringList retVal;
	for (const DictRef & dictRef : m_dicts) {
		if (dictRef) {
			retVal.push_back(dictRef.language);
		}
	}
	return retVal;
}

bool SpellChecker::setLanguages(const QStringList & languages)
{
	bool ok{true};
	m_dicts.clear();
	for (const QString & language : languages) {
		std::shared_ptr<Hunhandle> ptrHunhandle = SpellCheckManager::getDictionary(language);
		if (!ptrHunhandle) {
			ok = false;
			continue;
		}
		DictRef dictRef;
		dictRef.hunhandle = ptrHunhandle;
		dictRef.language = language;
		dictRef.codec = QTextCodec::codecForName(Hunspell_get_dic_encoding(ptrHunhandle.get()));
		if (dictRef.codec == nullptr) {
			dictRef.codec = QTextCodec::codecForLocale();
		}
		m_dicts.push_back(std::move(dictRef));
	}
	return ok;
}

bool SpellChecker::isWordCorrect(const QString & word) const
{
	for (const DictRef & dictRef : m_dicts) {
		if (!dictRef) {
			continue;
		}
		std::shared_ptr<Hunhandle> ptrHunhandle = dictRef.getHunhandle();
		if (Hunspell_spell(ptrHunhandle.get(), dictRef.codec->fromUnicode(word).data()) != 0) {
			return true;
		}
	}
	return false;
}

QList<QString> SpellChecker::suggestionsForWord(const QString & word) const
{
	QList<QString> suggestions;

	for (const DictRef & dictRef : m_dicts) {
		if (!dictRef) {
			continue;
		}
		std::shared_ptr<Hunhandle> ptrHunhandle = dictRef.getHunhandle();
		char ** suggestionList{nullptr};

		int numSuggestions = Hunspell_suggest(ptrHunhandle.get(), &suggestionList, dictRef.codec->fromUnicode(word).data());
		suggestions.reserve(suggestions.size() + numSuggestions);
		for (int iSuggestion = 0; iSuggestion < numSuggestions; ++iSuggestion) {
			suggestions.append(dictRef.codec->toUnicode(suggestionList[iSuggestion]));
		}

		Hunspell_free_list(ptrHunhandle.get(), &suggestionList, numSuggestions);
	}
	return suggestions;
}

void SpellChecker::ignoreWord(const QString & word)
{
	// FIXME: this adds the word to the first (valid) Hunhandle, which
	// (from the user's perspective) corresponds to an arbitrary language;
	// if that language is deactivated later the ignore list disappears as
	// well
	for (const DictRef & dictRef : m_dicts) {
		if (!dictRef) {
			continue;
		}
		std::shared_ptr<Hunhandle> ptrHunhandle = dictRef.getHunhandle();
		// note that this is not persistent after quitting TW
		Hunspell_add(ptrHunhandle.get(), dictRef.codec->fromUnicode(word).data());
		return;
	}
}

} // namespace Document
} // namespace Tw
