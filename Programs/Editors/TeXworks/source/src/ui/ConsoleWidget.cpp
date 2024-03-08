/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2023  Stefan Löffler

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

#include "ui/ConsoleWidget.h"

namespace Tw {
namespace UI {

ConsoleWidget::ConsoleWidget(QWidget * parent /* = nullptr */)
 : QTextEdit(parent)
{
}

ConsoleWidget::~ConsoleWidget()
{
	setProcess(nullptr);
}

void ConsoleWidget::setProcess(QProcess *p, const bool clearConsole /* = true */)
{
	if (m_process) {
		m_process->disconnect(this);
	}
	if (clearConsole) {
		clear();
	}
	m_unicodeCarry.clear();
	m_process = p;
	if (m_process) {
		connect(m_process, &QProcess::readyReadStandardOutput, this, [&]() {appendOutput(m_process->readAllStandardOutput()); });
		connect(m_process, &QProcess::destroyed, this, [&]() {setProcess(nullptr, false);});
	}
}

void ConsoleWidget::echo(const QString &str, const QColor foregroundColor /* = {} */)
{
	QTextCursor curs(document());
	using pos_type = decltype(curs.position());
	curs.setPosition(static_cast<pos_type>(toPlainText().length()));
	setTextCursor(curs);
	QTextCharFormat inputFormat(currentCharFormat());
	if (foregroundColor.isValid()) {
		inputFormat.setForeground(QBrush(foregroundColor));
	}
	insertPlainText(str);
	curs.movePosition(QTextCursor::PreviousCharacter);
	curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, static_cast<pos_type>(str.length() - 1));
	curs.setCharFormat(inputFormat);
}

void ConsoleWidget::appendOutput(QByteArray output)
{
	processIncompleteUTF8Codes(output);
	QTextCursor cursor(document());
	cursor.select(QTextCursor::Document);
	cursor.setPosition(cursor.selectionEnd());
	cursor.insertText(QString::fromUtf8(output.constData()));
	setTextCursor(cursor);
}

void ConsoleWidget::processIncompleteUTF8Codes(QByteArray &data)
{
	using size_type = decltype(data.size());

	if (!m_unicodeCarry.isEmpty()) {
		data.prepend(m_unicodeCarry);
		m_unicodeCarry.clear();
	}

	unsigned char mask{0xF8};
	unsigned char value{0xF0};

	for (size_type codePointLength = 4; codePointLength > 1; --codePointLength, mask <<= 1, value <<= 1) {
		for (size_type offset = 1; offset < qMin(data.size(), codePointLength); ++offset) {
			if ((data[data.size() - offset] & mask) == value) {
				m_unicodeCarry.append(data.right(offset));
				data.remove(data.size() - offset, offset);
				break;
			}
		}
	}
}

} // namespace UI
} // namespace Tw
