/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019-2020  Stefan Löffler

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
#include "LineNumberWidget.h"

#include <QPainter>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>

namespace Tw {
namespace UI {

LineNumberWidget::LineNumberWidget(QTextEdit * parent)
	: QWidget(parent)
	, _editor(parent)
	, _bgColor(palette().color(QPalette::Mid))
{
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
}

QSize LineNumberWidget::sizeHint() const
{
	int digits = 1;

	if (_editor) {
		int max = qMax(1, _editor->document()->blockCount());
		while (max >= 10) {
			max /= 10;
			++digits;
		}
	}

	int space = 3 + fontMetrics().width(QChar::fromLatin1('9')) * digits;
	return QSize(space, 0);
}

void LineNumberWidget::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.fillRect(event->rect(), _bgColor);

	if (!_editor)
		return;

	QTextBlock block = _editor->document()->begin();
	int blockNumber = 1;

	QAbstractTextDocumentLayout *layout = _editor->document()->documentLayout();
	int top = static_cast<int>(layout->blockBoundingRect(block).top() - _editor->verticalScrollBar()->value());
	int bottom = static_cast<int>(top + layout->blockBoundingRect(block).height());

	while (block.isValid() && top <= event->rect().bottom()) {
		if (bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber);
			painter.drawText(0, top, width() - 1, fontMetrics().height(),
							 Qt::AlignRight, number);
		}

		block = block.next();
		if (block == _editor->document()->end())
			break;
		// NB: The top of this block may not coincide with the bottom of the
		// previous block in case the line spacing is not 100%
		top = static_cast<int>(layout->blockBoundingRect(block).top() - _editor->verticalScrollBar()->value());
		bottom = top + static_cast<int>(layout->blockBoundingRect(block).height());
		++blockNumber;
	}
}

void LineNumberWidget::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::ParentChange) {
		_editor = qobject_cast<QTextEdit*>(parentWidget());
	}
	QWidget::changeEvent(event);
}

} // namespace UI
} // namespace Tw
