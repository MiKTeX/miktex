/*************************************************************************
** VFReader.cpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#include <sstream>
#include "FixWord.hpp"
#include "Font.hpp"
#include "Length.hpp"
#include "VFActions.hpp"
#include "VFReader.hpp"

using namespace std;


VFActions* VFReader::replaceActions (VFActions *a) {
	VFActions *ret = _actions;
	_actions = a;
	return ret;
}


/** Reads a single VF command from the current position of the input stream and calls the
 *  corresponding cmdFOO method. The execution can be influenced by a function of type ApproveOpcode.
 *  It takes an opcode and returns true if the command is supposed to be executed.
 *  @param[in] approve function to approve invocation of the action assigned to command
 *  @return opcode of the executed command */
int VFReader::executeCommand (ApproveAction approve) {
	int opcode = readByte();
	if (!isStreamValid() || opcode < 0)  // at end of file?
		throw VFException("invalid VF file");

	bool approved = !approve || approve(opcode);
	VFActions *actions = _actions;
	if (!approved)
		replaceActions(nullptr);  // disable actions

	if (opcode <= 241)     // short character definition?
		cmdShortChar(opcode);
	else if (opcode >= 243 && opcode <= 246)   // font definition?
		cmdFontDef(opcode-243+1);
	else {
		switch (opcode) {
			case 242: cmdLongChar(); break;  // long character definition
			case 247: cmdPre();      break;  // preamble
			case 248: cmdPost();     break;  // postamble
			default : {                      // invalid opcode
				replaceActions(actions);      // reenable actions
				throw VFException("undefined VF command (opcode " + std::to_string(opcode) + ")");
			}
		}
	}
	replaceActions(actions); // reenable actions
	return opcode;
}


bool VFReader::executeAll () {
	clearStream();  // reset all status bits
	if (!isStreamValid())
		return false;
	seek(0);  // move file pointer to first byte of the input stream
	while (!eof() && executeCommand() != 248); // stop reading after post (248)
	return true;
}


bool VFReader::executePreambleAndFontDefs () {
	clearStream();
	if (!isStreamValid())
		return false;
	seek(0);  // move file pointer to first byte of the input stream
	auto is_pre_or_fontdef = [](int op) {return op > 242;};
	while (!eof() && executeCommand(is_pre_or_fontdef) > 242); // stop reading after last font definition
	return true;
}


bool VFReader::executeCharDefs () {
	clearStream();
	if (!isStreamValid())
		return false;
	seek(0);
	auto is_chardef = [](int op) {return op < 243;};
	while (!eof() && executeCommand(is_chardef) < 243); // stop reading after last char definition
	return true;
}

//////////////////////////////////////////////////////////////////////////////

/** Reads and executes DVI preamble command. */
void VFReader::cmdPre () {
	uint32_t i = readUnsigned(1);  // identification number (should be 2)
	uint32_t k = readUnsigned(1);  // length of following comment
	string cmt = readString(k);    // comment
	uint32_t cs = readUnsigned(4); // check sum to be compared with TFM cecksum
	int32_t ds = readUnsigned(4);  // design size (same as TFM design size) (fix_word)
	_designSize = double(FixWord(ds))*Length::pt2bp;
	if (i != 202)
		throw VFException("invalid identification value in preamble");
	if (_actions)
		_actions->vfPreamble(cmt, cs, ds);
}


void VFReader::cmdPost () {
	while ((readUnsigned(1)) == 248); // skip fill bytes
	if (_actions)
		_actions->vfPostamble();
}


void VFReader::cmdLongChar () {
	uint32_t pl  = readUnsigned(4);    // packet length (length of DVI subroutine)
	if (!_actions)
		seek(8+pl, ios::cur);           // skip remaining char definition bytes
	else {
		uint32_t cc  = readUnsigned(4); // character code
		readUnsigned(4);                // equals character width from corresponding TFM file
		auto dvi = readBytes(pl);       // DVI subroutine
		_actions->defineVFChar(cc, std::move(dvi)); // call template method for user actions
	}
}


/** Reads and executes short_char_x command.
 *  @param[in] pl packet length (length of DVI subroutine) */
void VFReader::cmdShortChar (int pl) {
	if (!_actions)
		seek(4+pl, ios::cur);  // skip char definition bytes
	else {
		uint32_t cc  = readUnsigned(1); // character code
		readUnsigned(3);                // character width from corresponding TFM file
		auto dvi = readBytes(pl);       // DVI subroutine
		_actions->defineVFChar(cc, std::move(dvi)); // call template method for user actions
	}
}


void VFReader::cmdFontDef (int len) {
	uint32_t fontnum  = readUnsigned(len);  // font number
	uint32_t checksum = readUnsigned(4);    // font checksum (to be compared with corresponding TFM checksum)
	uint32_t ssize    = readUnsigned(4);    // scaled size of font relative to design size (fix_word)
	uint32_t dsize    = readUnsigned(4);    // design size of font (same as TFM design size) (fix_word)
	uint32_t pathlen  = readUnsigned(1);    // length of font path
	uint32_t namelen  = readUnsigned(1);    // length of font name
	string fontpath = readString(pathlen);
	string fontname = readString(namelen);
	if (_actions) {
		double ss = double(FixWord(ssize))*Length::pt2bp;
		double ds = double(FixWord(dsize))*Length::pt2bp;
		_actions->defineVFFont(fontnum, fontpath, fontname, checksum, ds, ss*_designSize);
	}
}
