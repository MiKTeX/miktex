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
#if defined(MIKTEX)
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <miktex/Core/AutoResource>
#include <miktex/miktex-texworks.hpp>
#endif
#include "document/SpellChecker.h"
#include "TWUtils.h" // for TWUtils::getLibraryPath

#include <hunspell.h>

namespace Tw {
namespace Document {

QHash<QString, QString> * SpellChecker::dictionaryList = nullptr;
QHash<const QString,SpellChecker::Dictionary*> * SpellChecker::dictionaries = nullptr;
SpellChecker * SpellChecker::_instance = new SpellChecker();

// static
QHash<QString, QString> * SpellChecker::getDictionaryList(const bool forceReload /* = false */)
{
	if (dictionaryList) {
		if (!forceReload)
			return dictionaryList;
		delete dictionaryList;
	}

	dictionaryList = new QHash<QString, QString>();
	const QStringList dirs = TWUtils::getLibraryPaths(QStringLiteral("dictionaries"));
	foreach (QDir dicDir, dirs) {
		foreach (QFileInfo dicFileInfo, dicDir.entryInfoList(QStringList(QString::fromLatin1("*.dic")),
					QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
			QFileInfo affFileInfo(dicFileInfo.dir(), dicFileInfo.completeBaseName() + QLatin1String(".aff"));
			if (affFileInfo.isReadable())
				dictionaryList->insertMulti(dicFileInfo.canonicalFilePath(), dicFileInfo.completeBaseName());
		}
	}
#if defined(MIKTEX)
        {
          std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
          for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
          {
            MiKTeX::Util::PathName dicPath = session->GetRootDirectoryPath(r) / MiKTeX::Util::PathName(MIKTEX_PATH_HUNSPELL_DICT_DIR);
            QDir dicDir(QString::fromUtf8(dicPath.GetData()));
            for(const auto& dicFileInfo : dicDir.entryInfoList({ QStringLiteral("*.dic") }, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase))
            {
              QFileInfo affFileInfo(dicFileInfo.dir(), dicFileInfo.completeBaseName() + QStringLiteral(".aff"));
              if (affFileInfo.isReadable())
              {
                dictionaryList->insertMulti(dicFileInfo.canonicalFilePath(), dicFileInfo.completeBaseName());
              }
            }
          }
        }
#endif

	emit SpellChecker::instance()->dictionaryListChanged();
	return dictionaryList;
}

// static
SpellChecker::Dictionary * SpellChecker::getDictionary(const QString& language)
{
	if (language.isEmpty())
		return nullptr;

	if (!dictionaries)
		dictionaries = new QHash<const QString, Dictionary*>;

	if (dictionaries->contains(language))
		return dictionaries->value(language);

	const QStringList dirs = TWUtils::getLibraryPaths(QStringLiteral("dictionaries"));
	foreach (QDir dicDir, dirs) {
		QFileInfo affFile(dicDir, language + QLatin1String(".aff"));
		QFileInfo dicFile(dicDir, language + QLatin1String(".dic"));
		if (affFile.isReadable() && dicFile.isReadable()) {
#if defined(MIKTEX)
                  MIKTEX_INFO(fmt::format("loading dictionary: {0}", language.toUtf8().data()));
#endif
#if defined(MIKTEX_WINDOWS)
                  Hunhandle* h = Hunspell_create(affFile.canonicalFilePath().toUtf8().data(), dicFile.canonicalFilePath().toUtf8().data());
#else
			Hunhandle * h = Hunspell_create(affFile.canonicalFilePath().toLocal8Bit().data(),
								dicFile.canonicalFilePath().toLocal8Bit().data());
#endif
			dictionaries->insert(language, new Dictionary(language, h));
			return dictionaries->value(language);
		}
	}
#if defined(MIKTEX)
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
  MIKTEX_AUTO(session->UnloadFilenameDatabase());
  for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
  {
    MiKTeX::Util::PathName dicPath = session->GetRootDirectoryPath(r) / MiKTeX::Util::PathName(MIKTEX_PATH_HUNSPELL_DICT_DIR);
    const QString dictPath = QString::fromStdWString(dicPath.ToWideCharString());
    QFileInfo affFile(dictPath + "/" + language + ".aff");
    QFileInfo dicFile(dictPath + "/" + language + ".dic");
    if (affFile.isReadable() && dicFile.isReadable())
    {
#if defined(MIKTEX)
      MIKTEX_INFO(fmt::format("loading dictionary: {0}", language.toUtf8().data()));
#endif
      Hunhandle* h = Hunspell_create(affFile.canonicalFilePath().toUtf8().data(), dicFile.canonicalFilePath().toUtf8().data());
      dictionaries->insert(language, new Dictionary(language, h));
      return dictionaries->value(language);
    }
  }
#endif
	return nullptr;
}

// static
void SpellChecker::clearDictionaries()
{
	if (!dictionaries)
		return;

	foreach(Dictionary * d, *dictionaries)
		delete d;

	delete dictionaries;
	dictionaries = nullptr;
}

SpellChecker::Dictionary::Dictionary(const QString & language, Hunhandle * hunhandle)
	: _language(language)
	, _hunhandle(hunhandle)
	, _codec(nullptr)
{
	if (_hunhandle)
		_codec = QTextCodec::codecForName(Hunspell_get_dic_encoding(_hunhandle));
	if (!_codec)
		_codec = QTextCodec::codecForLocale(); // almost certainly wrong, if we couldn't find the actual name!
}

SpellChecker::Dictionary::~Dictionary()
{
	if (_hunhandle)
		Hunspell_destroy(_hunhandle);
}

bool SpellChecker::Dictionary::isWordCorrect(const QString & word) const
{
	return (Hunspell_spell(_hunhandle, _codec->fromUnicode(word).data()) != 0);
}

QList<QString> SpellChecker::Dictionary::suggestionsForWord(const QString & word) const
{
	QList<QString> suggestions;
	char ** suggestionList;

	int numSuggestions = Hunspell_suggest(_hunhandle, &suggestionList, _codec->fromUnicode(word).data());
	suggestions.reserve(numSuggestions);
	for (int iSuggestion = 0; iSuggestion < numSuggestions; ++iSuggestion)
		suggestions.append(_codec->toUnicode(suggestionList[iSuggestion]));

	Hunspell_free_list(_hunhandle, &suggestionList, numSuggestions);

	return suggestions;
}

void SpellChecker::Dictionary::ignoreWord(const QString & word)
{
	// note that this is not persistent after quitting TW
	Hunspell_add(_hunhandle, _codec->fromUnicode(word).data());
}

} // namespace Document
} // namespace Tw
