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
    : NonblockingSyntaxHighlighter(parent)
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
	// 1) remove any invalid ranges
	for (int i = _highlightRanges.size() - 1; i >= 0; --i) {
		if (_highlightRanges[i].to <= _highlightRanges[i].from)
			_highlightRanges.remove(i);
	}
	// 2) merge adjacent (or overlapping) ranges
	// NB: There must not be any invalid ranges in here for this or else the
	// merging algorithm would fail
	for (int i = _highlightRanges.size() - 1; i >= 1; --i) {
		if (_highlightRanges[i].from <= _highlightRanges[i - 1].to) {
			if (_highlightRanges[i - 1].from > _highlightRanges[i].from)
				_highlightRanges[i - 1].from > _highlightRanges[i].from;
			if (_highlightRanges[i - 1].to < _highlightRanges[i].to)
				_highlightRanges[i - 1].to > _highlightRanges[i].to;
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
