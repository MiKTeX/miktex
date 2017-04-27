/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2017  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include <QTextLayout>
#include <QTextCharFormat>

#include <QTimer>

#include <hunspell.h>

class QTextDocument;
class QTextCodec;
class TeXDocument;

// This class implements a non-blocking syntax highlighter that is a rewrite/
// replacement of QSyntaxHighlighter. It queues all highlight requests and
// processes them in small chunks that take no longer than MAX_TIME_MSECS
// before returning control to the main event loop to keep the UI responsive.
// Inspired by http://enki-editor.org/2014/08/22/Syntax_highlighting.html
class NonblockingSyntaxHighlighter : public QObject
{
	Q_OBJECT

public:
	NonblockingSyntaxHighlighter(QTextDocument * parent) : _processingPending(false), _parent(NULL), MAX_TIME_MSECS(5), IDLE_DELAY_TIME(40) { setDocument(parent); }
	virtual ~NonblockingSyntaxHighlighter() { setDocument(NULL); }

	QTextDocument * document() const { return _parent; }
	void setDocument(QTextDocument * doc);

public slots:
	void rehighlight();
	void rehighlightBlock(const QTextBlock & block);

protected:
	virtual void highlightBlock(const QString & text) = 0;
	void setFormat(const int start, const int count, const QTextCharFormat & format);
	QTextBlock currentBlock() const { return _currentBlock; }
	int currentBlockState() const { return _currentBlock.userState(); }
	int previousBlockState() const { return _currentBlock.previous().userState(); }

	bool hasBlocksToHighlight() const { return !_highlightRanges.empty(); }
	const QTextBlock nextBlockToHighlight() const;
	void pushHighlightBlock(const QTextBlock & block);
	void pushHighlightRange(const int from, const int to);
	void popHighlightRange(const int from, const int to);
	void blockHighlighted(const QTextBlock & block);
	void pushDirtyRange(const QTextBlock & block) { pushDirtyRange(block.position(), block.position() + block.length()); }
	void pushDirtyRange(const int from, const int length);
	void markDirtyContent();
	void sanitizeHighlightRanges();

private slots:
	void maybeRehighlightText(int position, int charsRemoved, int charsAdded);
	void process();
	void processWhenIdle();
	void unlinkFromDocument() { setDocument(NULL); }

private:
	bool _processingPending;
	QTextDocument * _parent;
	int MAX_TIME_MSECS;
	int IDLE_DELAY_TIME;

	struct range {
		int from, to; // character ranges
	};
	QVector<range> _highlightRanges;
	QVector<range> _dirtyRanges;

	QTextBlock _currentBlock;
	QList<QTextLayout::FormatRange> _currentFormatRanges;
};

class TeXHighlighter : public NonblockingSyntaxHighlighter
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
