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

#if defined(MIKTEX)
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <miktex/Core/AutoResource>
#include <miktex/Core/Paths>
#include <miktex/miktex-texworks.hpp>
#endif
#include "document/SpellCheckManager.h"

#include "utils/ResourcesLibrary.h"

#include <hunspell.h>

#include <QLocale>

namespace Tw {
namespace Document {

QMultiHash<QString, QString> * SpellCheckManager::dictionaryList = nullptr;
QHash<const QString,std::shared_ptr<Hunhandle>> * SpellCheckManager::dictionaries = nullptr;
SpellCheckManager * SpellCheckManager::_instance = new SpellCheckManager();

// static
QString SpellCheckManager::labelForDict(QString &dict)
{
	QLocale loc{dict};

	if (loc.language() != QLocale::C) {
		const QString languageString = QLocale::languageToString(loc.language());
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
		const QString territoryString = (loc.country() != QLocale::AnyCountry ? QLocale::countryToString(loc.country()) : QString());
#else
		const QString territoryString = (loc.territory() != QLocale::AnyTerritory ? QLocale::territoryToString(loc.territory()) : QString());
#endif
		if (!territoryString.isEmpty()) {
			//: Format to display spell-checking dictionaries (ex. "English - United States (en_US)")
			return tr("%1 - %2 (%3)").arg(languageString, territoryString, dict);
		}
		else {
			//: Format to display spell-checking dictionaries (ex. "English (en)")
			return tr("%1 (%2)").arg(languageString, dict);
		}
	}
	return dict;
}

QMultiHash<QString, QString> * SpellCheckManager::getDictionaryList(const bool forceReload /* = false */)
{
	if (dictionaryList) {
		if (!forceReload)
			return dictionaryList;
		delete dictionaryList;
	}

	dictionaryList = new QMultiHash<QString, QString>();
	const QStringList dirs = Tw::Utils::ResourcesLibrary::getLibraryPaths(QStringLiteral("dictionaries"));
	foreach (QDir dicDir, dirs) {
		foreach (QFileInfo dicFileInfo, dicDir.entryInfoList(QStringList(QString::fromLatin1("*.dic")),
					QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
			QFileInfo affFileInfo(dicFileInfo.dir(), dicFileInfo.completeBaseName() + QLatin1String(".aff"));
			if (affFileInfo.isReadable())
				dictionaryList->insert(dicFileInfo.canonicalFilePath(), dicFileInfo.completeBaseName());
		}
	}
#if defined(MIKTEX)
	{
		std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
		for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
		{
			MiKTeX::Util::PathName dicPath = session->GetRootDirectoryPath(r) / MIKTEX_PATH_HUNSPELL_DICT_DIR;
			QDir dicDir(QString::fromUtf8(dicPath.GetData()));
			for (const auto& dicFileInfo : dicDir.entryInfoList({ QStringLiteral("*.dic") }, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase))
			{
				QFileInfo affFileInfo(dicFileInfo.dir(), dicFileInfo.completeBaseName() + QStringLiteral(".aff"));
				if (affFileInfo.isReadable())
				{
					dictionaryList->insert(dicFileInfo.canonicalFilePath(), dicFileInfo.completeBaseName());
				}
			}
		}
	}
#endif

	emit SpellCheckManager::instance()->dictionaryListChanged();
	return dictionaryList;
}

// static
std::shared_ptr<Hunhandle> SpellCheckManager::getDictionary(const QString & language)
{
	if (language.isEmpty())
		return nullptr;

	if (!dictionaries)
		dictionaries = new QHash<const QString, std::shared_ptr<Hunhandle>>;

	if (dictionaries->contains(language))
		return dictionaries->value(language);

	const QStringList dirs = Tw::Utils::ResourcesLibrary::getLibraryPaths(QStringLiteral("dictionaries"));
	foreach (QDir dicDir, dirs) {
		QFileInfo affFile(dicDir, language + QLatin1String(".aff"));
		QFileInfo dicFile(dicDir, language + QLatin1String(".dic"));
		if (affFile.isReadable() && dicFile.isReadable()) {
#if defined(MIKTEX)
			MIKTEX_INFO(fmt::format("loading dictionary: {0}", language.toUtf8().data()));
#endif
#if defined(MIKTEX)
			MIKTEX_INFO(fmt::format("loading dictionary: {0}", language.toUtf8().data()));
#endif
#if defined(MIKTEX_WINDOWS)
			auto h = std::shared_ptr<Hunhandle>(Hunspell_create(affFile.canonicalFilePath().toUtf8().data(),
								dicFile.canonicalFilePath().toUtf8().data()), Hunspell_destroy);
#else
			auto h = std::shared_ptr<Hunhandle>(Hunspell_create(affFile.canonicalFilePath().toLocal8Bit().data(),
								dicFile.canonicalFilePath().toLocal8Bit().data()), Hunspell_destroy);
#endif
			dictionaries->insert(language, h);
			return h;
		}
	}
#if defined(MIKTEX)
	std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
	MIKTEX_AUTO(session->UnloadFilenameDatabase());
	for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
	{
		MiKTeX::Util::PathName dicPath = session->GetRootDirectoryPath(r) / MIKTEX_PATH_HUNSPELL_DICT_DIR;
		const QString dictPath = QString::fromStdWString(dicPath.ToWideCharString());
		QFileInfo affFile(dictPath + "/" + language + ".aff");
		QFileInfo dicFile(dictPath + "/" + language + ".dic");
		if (affFile.isReadable() && dicFile.isReadable())
		{
			MIKTEX_INFO(fmt::format("loading dictionary: {0}", language.toUtf8().data()));
#if defined(MIKTEX_WINDOWS)
			auto h = std::shared_ptr<Hunhandle>(Hunspell_create(affFile.canonicalFilePath().toUtf8().data(),
								dicFile.canonicalFilePath().toUtf8().data()), Hunspell_destroy);
#else
			auto h = std::shared_ptr<Hunhandle>(Hunspell_create(affFile.canonicalFilePath().toLocal8Bit().data(),
								dicFile.canonicalFilePath().toLocal8Bit().data()), Hunspell_destroy);
#endif
			dictionaries->insert(language, h);
			return dictionaries->value(language);
		}
	}
#endif
	return nullptr;
}

// static
void SpellCheckManager::clearDictionaries()
{
	if (!dictionaries)
		return;

	dictionaries->clear();

	delete dictionaries;
	dictionaries = nullptr;
}


} // namespace Document
} // namespace Tw
