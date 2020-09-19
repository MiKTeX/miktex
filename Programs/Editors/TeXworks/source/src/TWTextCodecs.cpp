/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2012-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include <TWTextCodecs.h>

// NOTE: The convert*Unicode functions are modeled after those in the files in
// <Qt>/src/corelib/codecs/


// This data is extracted from http://en.wikipedia.org/w/index.php?title=Macintosh_Central_European_encoding&oldid=450446783
ushort MacCentralEurRomanCodes[] = {
	0x00C4, 0x0100, 0x0101, 0x00C9, 0x0104, 0x00D6, 0x00DC, 0x00E1,
	0x0105, 0x010C, 0x00E4, 0x010D, 0x0106, 0x0107, 0x00E9, 0x0179,
	0x017A, 0x010E, 0x00ED, 0x010F, 0x0112, 0x0113, 0x0116, 0x00F3,
	0x0117, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x011A, 0x011B, 0x00FC,
	0x2020, 0x00B0, 0x0118, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
	0x00AE, 0x00A9, 0x2122, 0x0119, 0x00A8, 0x2260, 0x0123, 0x012E,
	0x012F, 0x012A, 0x2264, 0x2265, 0x012B, 0x0136, 0x2202, 0x2211,
	0x0142, 0x013B, 0x013C, 0x013D, 0x013E, 0x0139, 0x013A, 0x0145,
	0x0146, 0x0143, 0x00AC, 0x221A, 0x0144, 0x0147, 0x2206, 0x00AB,
	0x00BB, 0x2026, 0x00A0, 0x0148, 0x0150, 0x00D5, 0x0151, 0x014C,
	0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
	0x014D, 0x0154, 0x0155, 0x0158, 0x2039, 0x203A, 0x0159, 0x0156,
	0x0157, 0x0160, 0x201A, 0x201E, 0x0161, 0x015A, 0x015B, 0x00C1,
	0x0164, 0x0165, 0x00CD, 0x017D, 0x017E, 0x016A, 0x00D3, 0x00D4,
	0x016B, 0x016E, 0x00DA, 0x016F, 0x0170, 0x0171, 0x0172, 0x0173,
	0x00DD, 0x00FD, 0x0137, 0x017B, 0x0141, 0x017C, 0x0122, 0x02C7
};

QList<QByteArray> MacCentralEurRomanCodec::aliases() const
{
	QList<QByteArray> list;
	list << "MacCentralEuropeanRoman" << "MacCentralEurRoman";
	return list;
}

QByteArray MacCentralEurRomanCodec::convertFromUnicode(const QChar * input, int length, ConverterState * state) const
{
	const uchar replacement = (state && state->flags & ConvertInvalidToNull) ? 0 : '?';
	QByteArray r(length, Qt::Uninitialized);
	uchar * d = reinterpret_cast<uchar*>(r.data());
	int invalid = 0;
	int i, j;

	for (i = 0; i < length; ++i) {
		uchar c = replacement;
		ushort uc = input[i].unicode();
		if (uc < 0x0080)
			c = static_cast<uchar>(uc);
		else {
			for (j = 0; j < 128; ++j) {
				if (MacCentralEurRomanCodes[j] == uc) {
					c = static_cast<uchar>(j + 0x80);
					break;
				}
			}
			if (j >= 128) {
				c = replacement;
			}
		}
		d[i] = c;
	}
	if (state) {
		state->remainingChars = 0;
		state->invalidChars += invalid;
	}
	return r;
}

QString MacCentralEurRomanCodec::convertToUnicode(const char * chars, int len, ConverterState * state) const
{
	Q_UNUSED(state)
	if (!chars)
		return QString();

	QString str = QString::fromLatin1(chars, len);
	QChar *uc = str.data();
	while(len--) {
		// NOTE: uc->unicode() should always be <= 0xff!
		if (uc->unicode() >= 0x80 && uc->unicode() <= 0xff)
			*uc = MacCentralEurRomanCodes[uc->unicode() - 0x80];
		uc++;
	}
	return str;
}

