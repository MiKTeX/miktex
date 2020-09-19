/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TWTextCodecs_H
#define TWTextCodecs_H

#include <QTextCodec>

class MacCentralEurRomanCodec : public QTextCodec
{
public:
	MacCentralEurRomanCodec() = default;
	~MacCentralEurRomanCodec() override = default;

	// NOTE: The mib number is arbitrary since this encoding is not in the IANA
	// list (http://www.iana.org/assignments/character-sets).
	int mibEnum() const override { return -4000; }
	QByteArray name() const override { return "Mac Central European Roman"; }
	QList<QByteArray> aliases() const override;

protected:
	QByteArray convertFromUnicode(const QChar * input, int length, ConverterState * state) const override;
	QString convertToUnicode(const char * chars, int len, ConverterState * state) const override;
};

#endif // !defined(TWTextCodecs)
