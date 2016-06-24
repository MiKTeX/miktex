/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2016  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include <QRegExp>
#include <QTextCodec>
#include <QTextCursor>

#include "TeXHighlighter.h"
#include "TeXDocument.h"
#include "TWUtils.h"

#include <limits.h> // for INT_MAX

QList<TeXHighlighter::HighlightingSpec> *TeXHighlighter::syntaxRules = NULL;
QList<TeXHighlighter::TagPattern> *TeXHighlighter::tagPatterns = NULL;

TeXHighlighter::TeXHighlighter(QTextDocument *parent, TeXDocument *texDocument)
	: QSyntaxHighlighter(parent)
	, texDoc(texDocument)
	, highlightIndex(-1)
	, isTagging(true)
	, pHunspell(NULL)
	, spellingCodec(NULL)
	, textDoc(parent)
{
	loadPatterns();
	// TODO: We should use QTextCharFormat::SpellCheckUnderline here, but that
	// causes problems for some fonts/font sizes in Qt 5 (QTBUG-50499)
	spellFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
	spellFormat.setUnderlineColor(Qt::red);
}

void TeXHighlighter::spellCheckRange(const QString &text, int index, int limit, const QTextCharFormat &spellFormat)
{
	while (index < limit) {
		int start, end;
		if (TWUtils::findNextWord(text, index, start, end)) {
			if (start < index)
				start = index;
			if (end > limit)
				end = limit;
			if (start < end) {
				QString word = text.mid(start, end - start);
				int spellResult = Hunspell_spell(pHunspell, spellingCodec->fromUnicode(word).data());
				if (spellResult == 0)
					setFormat(start, end - start, spellFormat);
			}
		}
		index = end;
	}
}

void TeXHighlighter::highlightBlock(const QString &text)
{
	int index = 0;
	if (highlightIndex >= 0 && highlightIndex < syntaxRules->count()) {
		QList<HighlightingRule>& highlightingRules = (*syntaxRules)[highlightIndex].rules;
		while (index < text.length()) {
			int firstIndex = INT_MAX, len;
			const HighlightingRule* firstRule = NULL;
			for (int i = 0; i < highlightingRules.size(); ++i) {
				HighlightingRule &rule = highlightingRules[i];
				int foundIndex = text.indexOf(rule.pattern, index);
				if (foundIndex >= 0 && foundIndex < firstIndex) {
					firstIndex = foundIndex;
					firstRule = &rule;
				}
			}
			if (firstRule != NULL && (len = firstRule->pattern.matchedLength()) > 0) {
				if (pHunspell != NULL && firstIndex > index)
					spellCheckRange(text, index, firstIndex, spellFormat);
				setFormat(firstIndex, len, firstRule->format);
				index = firstIndex + len;
				if (pHunspell != NULL && firstRule->spellCheck)
					spellCheckRange(text, firstIndex, index, firstRule->spellFormat);
			}
			else
				break;
		}
	}
	if (pHunspell != NULL)
		spellCheckRange(text, index, text.length(), spellFormat);

#if QT_VERSION >= 0x040400	/* the currentBlock() method is not available in 4.3.x */
	if (texDoc != NULL) {
		bool changed = false;
		if (texDoc->removeTags(currentBlock().position(), currentBlock().length()) > 0)
			changed = true;
		if (isTagging) {
			int index = 0;
			while (index < text.length()) {
				int firstIndex = INT_MAX, len;
				TagPattern* firstPatt = NULL;
				for (int i = 0; i < tagPatterns->count(); ++i) {
					TagPattern& patt = (*tagPatterns)[i];
					int foundIndex = text.indexOf(patt.pattern, index);
					if (foundIndex >= 0 && foundIndex < firstIndex) {
						firstIndex = foundIndex;
						firstPatt = &patt;
					}
				}
				if (firstPatt != NULL && (len = firstPatt->pattern.matchedLength()) > 0) {
					QTextCursor	cursor(document());
					cursor.setPosition(currentBlock().position() + firstIndex);
					cursor.setPosition(currentBlock().position() + firstIndex + len, QTextCursor::KeepAnchor);
					QString text = firstPatt->pattern.cap(1);
					if (text.isEmpty())
						text = firstPatt->pattern.cap(0);
					texDoc->addTag(cursor, firstPatt->level, text);
					index = firstIndex + len;
					changed = true;
				}
				else
					break;
			}
		}
		if (changed)	
			texDoc->tagsChanged();
	}
#endif
}

void TeXHighlighter::setActiveIndex(int index)
{
	int oldIndex = highlightIndex;
	highlightIndex = (index >= 0 && index < syntaxRules->count()) ? index : -1;
	if (oldIndex != highlightIndex)
		rehighlight();
}

void TeXHighlighter::setSpellChecker(Hunhandle* h, QTextCodec* codec)
{
	if (pHunspell != h || spellingCodec != codec) {
		pHunspell = h;
		spellingCodec = codec;
		QTimer::singleShot(1, this, SLOT(rehighlight()));
	}
}

QStringList TeXHighlighter::syntaxOptions()
{
	loadPatterns();
	
	QStringList options;
	if (syntaxRules != NULL)
		foreach (const HighlightingSpec& spec, *syntaxRules)
			options << spec.name;
	return options;
}

void TeXHighlighter::loadPatterns()
{
	if (syntaxRules != NULL)
		return;

	QDir configDir(TWUtils::getLibraryPath("configuration"));
	QRegExp whitespace("\\s+");

	if (syntaxRules == NULL) {
		syntaxRules = new QList<HighlightingSpec>;
		QFile syntaxFile(configDir.filePath("syntax-patterns.txt"));
		QRegExp sectionRE("^\\[([^\\]]+)\\]");
		if (syntaxFile.open(QIODevice::ReadOnly)) {
			HighlightingSpec spec;
			spec.name = tr("default");
			while (1) {
				QByteArray ba = syntaxFile.readLine();
				if (ba.size() == 0)
					break;
				if (ba[0] == '#' || ba[0] == '\n')
					continue;
				QString line = QString::fromUtf8(ba.data(), ba.size());
				if (sectionRE.indexIn(line) == 0) {
					if (spec.rules.count() > 0)
						syntaxRules->append(spec);
					spec.rules.clear();
					spec.name = sectionRE.cap(1);
					continue;
				}
				QStringList parts = line.split(whitespace, QString::SkipEmptyParts);
				if (parts.size() != 3)
					continue;
				QStringList styles = parts[0].split(QChar(';'));
				QStringList colors = styles[0].split(QChar('/'));
				QColor fg, bg;
				if (colors.size() <= 2) {
					if (colors.size() == 2)
						bg = QColor(colors[1]);
					fg = QColor(colors[0]);
				}
				HighlightingRule rule;
				if (fg.isValid())
					rule.format.setForeground(fg);
				if (bg.isValid())
					rule.format.setBackground(bg);
				if (styles.size() > 1) {
					if (styles[1].contains('B'))
						rule.format.setFontWeight(QFont::Bold);
					if (styles[1].contains('I'))
						rule.format.setFontItalic(true);
					if (styles[1].contains('U'))
						rule.format.setFontUnderline(true);
				}
				if (parts[1].compare("Y", Qt::CaseInsensitive) == 0) {
					rule.spellCheck = true;
					rule.spellFormat = rule.format;
					rule.spellFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
				}
				else
					rule.spellCheck = false;
				rule.pattern = QRegExp(parts[2]);
				if (rule.pattern.isValid() && !rule.pattern.isEmpty())
					spec.rules.append(rule);
			}
			if (spec.rules.count() > 0)
				syntaxRules->append(spec);
		}
	}
	
	if (tagPatterns == NULL) {
		// read tag-recognition patterns
		tagPatterns = new QList<TagPattern>;
		QFile tagPatternFile(configDir.filePath("tag-patterns.txt"));
		if (tagPatternFile.open(QIODevice::ReadOnly)) {
			while (1) {
				QByteArray ba = tagPatternFile.readLine();
				if (ba.size() == 0)
					break;
				if (ba[0] == '#' || ba[0] == '\n')
					continue;
				QString line = QString::fromUtf8(ba.data(), ba.size());
				QStringList parts = line.split(whitespace, QString::SkipEmptyParts);
				if (parts.size() != 2)
					continue;
				TagPattern patt;
				bool ok;
				patt.level = parts[0].toInt(&ok);
				if (ok) {
					patt.pattern = QRegExp(parts[1]);
					if (patt.pattern.isValid() && !patt.pattern.isEmpty())
						tagPatterns->append(patt);
				}
			}
		}
	}
}
