/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2014  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TEX_HIGHLIGHTER_H
#define TEX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QTextCharFormat>

#include <hunspell.h>

class QTextDocument;
class QTextCodec;
class TeXDocument;

class TeXHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	TeXHighlighter(QTextDocument *parent, TeXDocument *texDocument = NULL);
	
	void setActiveIndex(int index);

	void setSpellChecker(Hunhandle *h, QTextCodec *codec);

	QString getSyntaxMode() const {
		return (highlightIndex >= 0 && highlightIndex < syntaxOptions().size())
				? syntaxOptions().at(highlightIndex) : QString();
	}
	
	static QStringList syntaxOptions();

protected:
	void highlightBlock(const QString &text);

	void spellCheckRange(const QString &text, int index, int limit, const QTextCharFormat &spellFormat);

private:
	static void loadPatterns();

	struct HighlightingRule {
		QRegExp pattern;
		QTextCharFormat format;
		QTextCharFormat	spellFormat;
		bool spellCheck;
	};
	typedef QList<HighlightingRule> HighlightingRules;
	struct HighlightingSpec {
		QString				name;
		HighlightingRules	rules;
	};
	static QList<HighlightingSpec> *syntaxRules;

	QTextCharFormat spellFormat;

	struct TagPattern {
		QRegExp pattern;
		unsigned int level;
	};
	static QList<TagPattern> *tagPatterns;
	
	TeXDocument	*texDoc;

	int highlightIndex;
	bool isTagging;

	Hunhandle	*pHunspell;
	QTextCodec	*spellingCodec;

	QTextDocument * textDoc;
};

#endif
