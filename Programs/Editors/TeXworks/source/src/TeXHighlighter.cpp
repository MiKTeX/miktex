/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#include <miktex/miktex-texworks.hpp>
#endif
#include <QTextCursor>

#include "TeXHighlighter.h"
#include "TWUtils.h"
#include "document/TeXDocument.h"

#include <climits> // for INT_MAX

QList<TeXHighlighter::HighlightingSpec> *TeXHighlighter::syntaxRules = nullptr;
QList<TeXHighlighter::TagPattern> *TeXHighlighter::tagPatterns = nullptr;

TeXHighlighter::TeXHighlighter(Tw::Document::TeXDocument * parent)
	: NonblockingSyntaxHighlighter(parent)
	, highlightIndex(-1)
	, isTagging(true)
	, _dictionary(nullptr)
	, texDoc(parent)
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
				if (!_dictionary->isWordCorrect(text.mid(start, end - start)))
					setFormat(start, end - start, spellFormat);
			}
		}
		index = end;
	}
}

void TeXHighlighter::highlightBlock(const QString &text)
{
	int charPos = 0;
	if (highlightIndex >= 0 && highlightIndex < syntaxRules->count()) {
		QList<HighlightingRule>& highlightingRules = (*syntaxRules)[highlightIndex].rules;
		// Go through the whole text...
		while (charPos < text.length()) {
			// ... and find the highlight pattern that matches closest to the
			// current character index
			int firstIndex = INT_MAX, len;
			const HighlightingRule* firstRule = nullptr;
			QRegularExpressionMatch firstMatch;
			for (int i = 0; i < highlightingRules.size(); ++i) {
				HighlightingRule &rule = highlightingRules[i];
				QRegularExpressionMatch m = rule.pattern.match(text, charPos);
				if (m.capturedStart() >= 0 && m.capturedStart() < firstIndex) {
					firstIndex = m.capturedStart();
					firstMatch = m;
					firstRule = &rule;
				}
			}
			// If we found a rule, apply it and advance the character index to
			// the end of the highlighted range
			if (firstRule && firstMatch.hasMatch() && (len = firstMatch.capturedLength()) > 0) {
				if (_dictionary && firstIndex > charPos)
					spellCheckRange(text, charPos, firstIndex, spellFormat);
				setFormat(firstIndex, len, firstRule->format);
				charPos = firstIndex + len;
				if (_dictionary && firstRule->spellCheck)
					spellCheckRange(text, firstIndex, charPos, firstRule->spellFormat);
			}
			// If no rule matched, we can break out of the loop
			else
				break;
		}
	}
	if (_dictionary)
		spellCheckRange(text, charPos, text.length(), spellFormat);

	if (texDoc) {
		texDoc->removeTags(currentBlock().position(), currentBlock().length());
		if (isTagging) {
			int index = 0;
			while (index < text.length()) {
				int firstIndex = INT_MAX, len;
				TagPattern* firstPatt = nullptr;
				QRegularExpressionMatch firstMatch;
				for (int i = 0; i < tagPatterns->count(); ++i) {
					TagPattern& patt = (*tagPatterns)[i];
					QRegularExpressionMatch m = patt.pattern.match(text, index);
					if (m.capturedStart() >= 0 && m.capturedStart() < firstIndex) {
						firstIndex = m.capturedStart();
						firstMatch = m;
						firstPatt = &patt;
					}
				}
				if (firstPatt && firstMatch.hasMatch() && (len = firstMatch.capturedLength()) > 0) {
					QTextCursor	cursor(document());
					cursor.setPosition(currentBlock().position() + firstIndex);
					cursor.setPosition(currentBlock().position() + firstIndex + len, QTextCursor::KeepAnchor);
					QString tagText = firstMatch.captured(1);
					if (tagText.isEmpty())
						tagText = firstMatch.captured(0);
					texDoc->addTag(cursor, firstPatt->level, tagText);
					index = firstIndex + len;
				}
				else
					break;
			}
		}
	}
}

void TeXHighlighter::setActiveIndex(int index)
{
	int oldIndex = highlightIndex;
	highlightIndex = (index >= 0 && index < syntaxRules->count()) ? index : -1;
	if (oldIndex != highlightIndex)
		rehighlight();
}

void TeXHighlighter::setSpellChecker(Tw::Document::SpellChecker::Dictionary * dictionary)
{
	if (_dictionary != dictionary) {
#if defined(MIKTEX)
          MIKTEX_INFO(fmt::format("setting spell checker: {0}", dictionary->getLanguage().toUtf8().data()));
#endif
		_dictionary = dictionary;
		QTimer::singleShot(1, this, SLOT(rehighlight()));
	}
}

QStringList TeXHighlighter::syntaxOptions()
{
	loadPatterns();

	QStringList options;
	if (syntaxRules)
		foreach (const HighlightingSpec& spec, *syntaxRules)
			options << spec.name;
	return options;
}

void TeXHighlighter::loadPatterns()
{
	if (syntaxRules)
		return;

	QDir configDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration")));
	QRegularExpression whitespace(QStringLiteral("\\s+"));

	if (!syntaxRules) {
		syntaxRules = new QList<HighlightingSpec>;
		QFile syntaxFile(configDir.filePath(QString::fromLatin1("syntax-patterns.txt")));
		QRegularExpression sectionRE(QStringLiteral("^\\[([^\\]]+)\\]"));
		if (syntaxFile.open(QIODevice::ReadOnly)) {
			HighlightingSpec spec;
			spec.name = tr("default");
			while (true) {
				QByteArray ba = syntaxFile.readLine();
				if (ba.size() == 0)
					break;
				if (ba[0] == '#' || ba[0] == '\n')
					continue;
				QString line = QString::fromUtf8(ba.data(), ba.size());
				QRegularExpressionMatch sectionMatch = sectionRE.match(line);
				if (sectionMatch.capturedStart() == 0) {
					if (spec.rules.count() > 0)
						syntaxRules->append(spec);
					spec.rules.clear();
					spec.name = sectionMatch.captured(1);
					continue;
				}
				QStringList parts = line.split(whitespace, QString::SkipEmptyParts);
				if (parts.size() != 3)
					continue;
				QStringList styles = parts[0].split(QChar::fromLatin1(';'));
				QStringList colors = styles[0].split(QChar::fromLatin1('/'));
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
					if (styles[1].contains(QChar::fromLatin1('B')))
						rule.format.setFontWeight(QFont::Bold);
					if (styles[1].contains(QChar::fromLatin1('I')))
						rule.format.setFontItalic(true);
					if (styles[1].contains(QChar::fromLatin1('U')))
						rule.format.setFontUnderline(true);
				}
				if (parts[1].compare(QChar::fromLatin1('Y'), Qt::CaseInsensitive) == 0) {
					rule.spellCheck = true;
					rule.spellFormat = rule.format;
					// TODO: We should use QTextCharFormat::SpellCheckUnderline here, but that
					// causes problems for some fonts/font sizes in Qt 5 (QTBUG-50499)
					rule.spellFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
					rule.spellFormat.setUnderlineColor(Qt::red);
				}
				else
					rule.spellCheck = false;
				rule.pattern = QRegularExpression(parts[2]);
				if (rule.pattern.isValid())
					spec.rules.append(rule);
			}
			if (spec.rules.count() > 0)
				syntaxRules->append(spec);
		}
	}

	if (!tagPatterns) {
		// read tag-recognition patterns
		tagPatterns = new QList<TagPattern>;
		QFile tagPatternFile(configDir.filePath(QString::fromLatin1("tag-patterns.txt")));
		if (tagPatternFile.open(QIODevice::ReadOnly)) {
			while (true) {
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
				patt.level = parts[0].toUInt(&ok);
				if (ok) {
					patt.pattern = QRegularExpression(parts[1]);
					if (patt.pattern.isValid())
						tagPatterns->append(patt);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// NonblockingSyntaxHighlighter
///////////////////////////////////////////////////////////////////////////////

void NonblockingSyntaxHighlighter::setDocument(QTextDocument * doc)
{
	if (_parent)
		disconnect(_parent);
	_parent = doc;
	_highlightRanges.clear();
	_dirtyRanges.clear();
	if (_parent) {
		connect(_parent, SIGNAL(destroyed(QObject*)), this, SLOT(unlinkFromDocument()));
		connect(_parent, SIGNAL(contentsChange(int,int,int)), this, SLOT(maybeRehighlightText(int, int, int)));
		rehighlight();
	}
}

void NonblockingSyntaxHighlighter::rehighlight()
{
	if (!_parent)
		return;

	_highlightRanges.clear();
	range r;
	r.from = 0;
	r.to = _parent->characterCount();
	_highlightRanges.push_back(r);
	processWhenIdle();
}

void NonblockingSyntaxHighlighter::rehighlightBlock(const QTextBlock & block)
{
	pushHighlightBlock(block);
	processWhenIdle();
}

void NonblockingSyntaxHighlighter::maybeRehighlightText(int position, int charsRemoved, int charsAdded)
{
	if (!_parent)
		return;

	// Adjust ranges already present in _highlightRanges
	for (int i = 0; i < _highlightRanges.size(); ++i) {
		// Adjust front (if necessary)
		if (_highlightRanges[i].from >= position + charsRemoved)
			_highlightRanges[i].from += charsAdded - charsRemoved;
		else if (_highlightRanges[i].from >= position) // && _highlightRanges[i].from < position + charsRemoved
			_highlightRanges[i].from = position + charsAdded;
		// Adjust back (if necessary)
		if (_highlightRanges[i].to >= position + charsRemoved)
			_highlightRanges[i].to += charsAdded - charsRemoved;
		else if (_highlightRanges[i].to >= position) // && _highlightRanges[i].to < position + charsRemoved
			_highlightRanges[i].to = position;
	}
	// NB: pushHighlightRange() implicitly calls sanitizeHighlightRanges() so
	// there is no need to call it here explicitly

	// NB: Don't subtract charsRemoved. If charsAdded = 0 and charsRemoved > 0
	// we still want to rehighlight that line
	// Add 1 because of the following cases:
	// a) if charsAdded = 0, we still need to have at least one character in the range
	// b) if the insertion ends in a newline character (0x2029), for some
	//    reason the line immediately following the inserted line loses
	//    highlighting. Adding 1 ensures that that next line is rehighlighted
	//    as well.
	// c) if the insertion does not end in a newline character, adding 1 does
	//    not extend the range to a new line, so it doesn't matter (as
	//    highlighting is only performed line-wise).
	pushHighlightRange(position, position + charsAdded + 1);

	processWhenIdle();
}

void NonblockingSyntaxHighlighter::sanitizeHighlightRanges()
{
	// 1) clip ranges
	int n = (_parent ? _parent->characterCount() : 0);
	for (int i = 0; i < _highlightRanges.size(); ++i) {
		if (_highlightRanges[i].from < 0) _highlightRanges[i].from = 0;
		if (_highlightRanges[i].to > n) _highlightRanges[i].to = n;
	}

	// 2) remove any invalid ranges
	for (int i = _highlightRanges.size() - 1; i >= 0; --i) {
		if (_highlightRanges[i].to <= _highlightRanges[i].from)
			_highlightRanges.remove(i);
	}
	// 3) merge adjacent (or overlapping) ranges
	// NB: There must not be any invalid ranges in here for this or else the
	// merging algorithm would fail
	for (int i = _highlightRanges.size() - 1; i >= 1; --i) {
		if (_highlightRanges[i].from <= _highlightRanges[i - 1].to) {
			if (_highlightRanges[i - 1].from > _highlightRanges[i].from)
				_highlightRanges[i - 1].from = _highlightRanges[i].from;
			if (_highlightRanges[i - 1].to < _highlightRanges[i].to)
				_highlightRanges[i - 1].to = _highlightRanges[i].to;
			_highlightRanges.remove(i);
		}
	}
}


void NonblockingSyntaxHighlighter::process()
{
	_processingPending = false;
	if (!_parent)
		return;

	QTime start = QTime::currentTime();

	while (start.msecsTo(QTime::currentTime()) < MAX_TIME_MSECS && hasBlocksToHighlight()) {
		const QTextBlock & block = nextBlockToHighlight();
		if (block.isValid()) {
			int prevUserState = block.userState();
			_currentBlock = block;
			_currentFormatRanges.clear();
			highlightBlock(block.text());

			block.layout()->setAdditionalFormats(_currentFormatRanges);

			// If the userState has changed, make sure the next block is rehighlighted
			// as well
			if (block.userState() != prevUserState)
				pushHighlightBlock(block.next());
			blockHighlighted(block);
		}
	}

	// Notify the document of our changes
	markDirtyContent();

	// if there is more work, queue another round
	if (hasBlocksToHighlight())
		processWhenIdle();
}

void NonblockingSyntaxHighlighter::pushHighlightBlock(const QTextBlock & block)
{
	if (block.isValid())
		pushHighlightRange(block.position(), block.position() + block.length());
}

void NonblockingSyntaxHighlighter::pushHighlightRange(const int from, const int to)
{
	int i;
	range r;
	r.from = from;
	r.to = to;

	// Find the first old range such that the start of the new range is before
	// the end of the old range
	for (i = 0; i < _highlightRanges.size(); ++i) {
		if (from < _highlightRanges[i].from)
			break;
	}

	if (i == _highlightRanges.size())
		_highlightRanges.push_back(r);
	else
		_highlightRanges.insert(i, r);

	sanitizeHighlightRanges();
}

void NonblockingSyntaxHighlighter::popHighlightRange(const int from, const int to)
{
	for (int i = _highlightRanges.size() - 1; i >= 0 && _highlightRanges[i].to > from; --i) {
		// Case 1: crop the end of the range (or the whole range)
		if (to >= _highlightRanges[i].to) {
			if (from <= _highlightRanges[i].from)
				_highlightRanges.remove(i);
			else
				_highlightRanges[i].to = from;
		}
		// Case 2: crop the middle
		else if (from > _highlightRanges[i].from) {
			// Split the range into two
			range r = _highlightRanges[i];
			_highlightRanges[i].from = to;
			r.to = from;
			_highlightRanges.insert(i, r);
			--i;
		}
		// Case 3: crop the front of the range
		else if (to > _highlightRanges[i].from) {
			_highlightRanges[i].from = to;
		}
		// Case 4: to <= _highlightRanges[i].from
		// no overlap => do nothing
	}
}

void NonblockingSyntaxHighlighter::blockHighlighted(const QTextBlock &block)
{
	popHighlightRange(block.position(), block.position() + block.length());
	pushDirtyRange(block);
}

const QTextBlock NonblockingSyntaxHighlighter::nextBlockToHighlight() const
{
	if (!_parent || _highlightRanges.empty()) return QTextBlock();
	return _parent->findBlock(_highlightRanges[0].from);
}

void NonblockingSyntaxHighlighter::pushDirtyRange(const int from, const int length)
{
	// NB: we currently use (at most) one range as it seems that
	// QTextDocument::markContentsDirty() operates not only on the given lines
	// but also on all later lines. Thus, calling it repeatedly with adjacent
	// lines would create a huge, unncessary overhead compared to calling it
	// once.

	int to = from + length;
	if (_dirtyRanges.empty()) {
		range r;
		r.from = from;
		r.to = to;
		_dirtyRanges.push_back(r);
	}
	else {
		if (_dirtyRanges[0].from > from) _dirtyRanges[0].from = from;
		if (_dirtyRanges[0].to < to) _dirtyRanges[0].to = to;
	}
}

void NonblockingSyntaxHighlighter::markDirtyContent()
{
	if (!_parent)
		return;

	foreach(range r, _dirtyRanges)
		_parent->markContentsDirty(r.from, r.to - r.from);
	_dirtyRanges.clear();
}


void NonblockingSyntaxHighlighter::setFormat(const int start, const int count, const QTextCharFormat & format)
{
	QTextLayout::FormatRange formatRange;
	formatRange.start = start;
	formatRange.length = count;
	formatRange.format = format;
	_currentFormatRanges << formatRange;
}

void NonblockingSyntaxHighlighter::processWhenIdle()
{
	if (!_processingPending) {
		_processingPending = true;
		QTimer::singleShot(IDLE_DELAY_TIME, this, SLOT(process()));
	}
}
