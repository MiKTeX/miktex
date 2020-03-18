/*************************************************************************
** BasicDVIReader.cpp                                                   **
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

#include <algorithm>
#include "BasicDVIReader.hpp"

using namespace std;


BasicDVIReader::BasicDVIReader (std::istream &is) : StreamReader(is), _dviVersion(DVI_NONE)
{
}


/** Evaluates the next DVI command, and computes the corresponding handler.
 *  @param[out] handler handler for current DVI command
 *  @param[out] param the handler must be called with this parameter
 *  @return opcode of current DVI command */
int BasicDVIReader::evalCommand (CommandHandler &handler, int &param) {
	struct DVICommand {
		CommandHandler handler;
		int length;  // number of parameter bytes
	};

	/* Each cmdFOO command reads the necessary number of bytes from the stream, so executeCommand
	doesn't need to know the exact DVI command format. Some cmdFOO methods are used for multiple
	DVI commands because they only differ in length of their parameters. */
	static const DVICommand commands[] = {
		{&BasicDVIReader::cmdSetChar, 1},  {&BasicDVIReader::cmdSetChar, 2},  // 128-129
		{&BasicDVIReader::cmdSetChar, 3},  {&BasicDVIReader::cmdSetChar, 4},  // 130-131
		{&BasicDVIReader::cmdSetRule, 8},                                     // 132
		{&BasicDVIReader::cmdPutChar, 1},  {&BasicDVIReader::cmdPutChar, 2},  // 133-134
		{&BasicDVIReader::cmdPutChar, 3},  {&BasicDVIReader::cmdPutChar, 4},  // 135-136
		{&BasicDVIReader::cmdPutRule, 8},                                     // 137
		{&BasicDVIReader::cmdNop, 0},                                         // 138
		{&BasicDVIReader::cmdBop, 44},     {&BasicDVIReader::cmdEop, 0},      // 139-140
		{&BasicDVIReader::cmdPush, 0},     {&BasicDVIReader::cmdPop, 0},      // 141-142
		{&BasicDVIReader::cmdRight, 1},    {&BasicDVIReader::cmdRight, 2},    // 143-144
		{&BasicDVIReader::cmdRight, 3},    {&BasicDVIReader::cmdRight, 4},    // 145-146
		{&BasicDVIReader::cmdW0, 0},                                          // 147
		{&BasicDVIReader::cmdW, 1},        {&BasicDVIReader::cmdW, 2},        // 148-149
		{&BasicDVIReader::cmdW, 3},        {&BasicDVIReader::cmdW, 4},        // 150-151
		{&BasicDVIReader::cmdX0, 0},                                          // 152
		{&BasicDVIReader::cmdX, 1},        {&BasicDVIReader::cmdX, 2},        // 153-154
		{&BasicDVIReader::cmdX, 3},        {&BasicDVIReader::cmdX, 4},        // 155-156
		{&BasicDVIReader::cmdDown, 1},     {&BasicDVIReader::cmdDown, 2},     // 157-158
		{&BasicDVIReader::cmdDown, 3},     {&BasicDVIReader::cmdDown, 4},     // 159-160
		{&BasicDVIReader::cmdY0, 0},                                          // 161
		{&BasicDVIReader::cmdY, 1},        {&BasicDVIReader::cmdY, 2},        // 162-163
		{&BasicDVIReader::cmdY, 3},        {&BasicDVIReader::cmdY, 4},        // 164-165
		{&BasicDVIReader::cmdZ0, 0},                                          // 166
		{&BasicDVIReader::cmdZ, 1},        {&BasicDVIReader::cmdZ, 2},        // 167-168
		{&BasicDVIReader::cmdZ, 3},        {&BasicDVIReader::cmdZ, 4},        // 169-170

		{&BasicDVIReader::cmdFontNum, 1},  {&BasicDVIReader::cmdFontNum, 2},  // 235-236
		{&BasicDVIReader::cmdFontNum, 3},  {&BasicDVIReader::cmdFontNum, 4},  // 237-238
		{&BasicDVIReader::cmdXXX, 1},      {&BasicDVIReader::cmdXXX, 2},      // 239-240
		{&BasicDVIReader::cmdXXX, 3},      {&BasicDVIReader::cmdXXX, 4},      // 241-242
		{&BasicDVIReader::cmdFontDef, 1},  {&BasicDVIReader::cmdFontDef, 2},  // 243-244
		{&BasicDVIReader::cmdFontDef, 3},  {&BasicDVIReader::cmdFontDef, 4},  // 245-246
		{&BasicDVIReader::cmdPre, 0},      {&BasicDVIReader::cmdPost, 0},     // 247-248
		{&BasicDVIReader::cmdPostPost, 0},                                    // 249
	};

	const int opcode = readByte();
	if (!isStreamValid() || opcode < 0)  // at end of file
		throw InvalidDVIFileException("invalid DVI file");

	int num_param_bytes = 0;
	param = -1;
	if (opcode >= OP_SETCHAR0 && opcode <= OP_SETCHAR127) {
		handler = &BasicDVIReader::cmdSetChar0;
		param = opcode;
	}
	else if (opcode >= OP_FNTNUM0 && opcode <= OP_FNTNUM63) {
		handler = &BasicDVIReader::cmdFontNum0;
		param = opcode-OP_FNTNUM0;
	}
	else if (evalXDVOpcode(opcode, handler))
		num_param_bytes = 0;
	else if (_dviVersion == DVI_PTEX && opcode == OP_DIR) {  // direction command set by pTeX?
		handler = &BasicDVIReader::cmdDir;
		num_param_bytes = 1;
	}
	else if (opcode > OP_POSTPOST)
		throw DVIException("undefined DVI command (opcode " + to_string(opcode) + ")");
	else {
		const int offset = opcode < OP_FNTNUM0 ? OP_SET1 : (OP_FNTNUM63+1)-(OP_FNTNUM0-OP_SET1);
		handler = commands[opcode-offset].handler;
		num_param_bytes = commands[opcode-offset].length;
	}
	if (param < 0)
		param = num_param_bytes;
	return opcode;
}


/** Checks if a given opcode belongs to an XDV extension.
 *  @param[in] op the opcode to check
 *  @param[out] handler corresponding command handler if opcode is valid */
bool BasicDVIReader::evalXDVOpcode (int op, CommandHandler &handler) const {
	static const struct {
		int min, max;  // minimal and maximal opcode in XDV section
	} xdvranges[] = {
		{251, 254},  // XDV5
		{252, 253},  // XDV6
		{252, 254},  // XDV7
	};
	int index = _dviVersion-DVI_XDV5;
	if (_dviVersion < DVI_XDV5 || _dviVersion > DVI_XDV7 || op < xdvranges[index].min || op > xdvranges[index].max)
		return false;

	static const CommandHandler handlers[] = {
		&BasicDVIReader::cmdXPic,             // 251 (XDV5 only)
		&BasicDVIReader::cmdXFontDef,         // 252
		&BasicDVIReader::cmdXGlyphArray,      // 253
		&BasicDVIReader::cmdXTextAndGlyphs,   // 254 (XDV7 only)
		&BasicDVIReader::cmdXGlyphString      // 254 (XDV5 only)
	};
	index = op-251;
	if (_dviVersion == DVI_XDV5 && op == 254)
		index++;
	handler = handlers[index];
	return true;
}


/** Reads a single DVI command from the current position of the input stream and calls the
 *  corresponding cmdFOO method.
 *  @return opcode of the executed command */
int BasicDVIReader::executeCommand () {
	CommandHandler handler;
	int param; // parameter of handler
	int opcode = evalCommand(handler, param);
	(this->*handler)(param);
	return opcode;
}


void BasicDVIReader::executePostPost () {
	clearStream();  // reset all status bits
	if (!isStreamValid())
		throw DVIException("invalid DVI file");

	seek(-1, ios::end);       // stream pointer to last byte
	int count=0;
	while (peek() == DVI_FILL) {   // count trailing fill bytes
		seek(-1, ios::cur);
		count++;
	}
	if (count < 4)  // the standard requires at least 4 trailing fill bytes
		throw DVIException("missing fill bytes at end of file");

	setDVIVersion((DVIVersion)readUnsigned(1));
}


void BasicDVIReader::executeAllPages () {
	if (_dviVersion == DVI_NONE)
		executePostPost();   // get version ID from post_post
	seek(0);                // go to preamble
	while (executeCommand() != OP_POST);  // execute all commands until postamble is reached
}


void BasicDVIReader::setDVIVersion (DVIVersion version) {
	_dviVersion = max(_dviVersion, version);
	switch (_dviVersion) {
		case DVI_STANDARD:
		case DVI_PTEX:
		case DVI_XDV5:
		case DVI_XDV6:
		case DVI_XDV7:
			break;
		default:
			throw DVIException("DVI version " + to_string(_dviVersion) + " not supported");
	}
}

/////////////////////////////////////

/** Executes preamble command.
 *  Format: pre i[1] num[4] den[4] mag[4] k[1] x[k] */
void BasicDVIReader::cmdPre (int) {
	setDVIVersion((DVIVersion)readUnsigned(1)); // identification number
	seek(12, ios::cur);           // skip numerator, denominator, and mag factor
	uint32_t k = readUnsigned(1); // length of following comment
	seek(k, ios::cur);            // skip comment
}


/** Executes postamble command.
 *  Format: post p[4] num[4] den[4] mag[4] l[4] u[4] s[2] t[2] */
void BasicDVIReader::cmdPost (int) {
	seek(28, ios::cur);
}


/** Executes postpost command.
 *  Format: postpost q[4] i[1] 223's[>= 4] */
void BasicDVIReader::cmdPostPost (int) {
	seek(4, ios::cur);
	setDVIVersion((DVIVersion)readUnsigned(1));  // identification byte
	while (readUnsigned(1) == DVI_FILL);  // skip fill bytes (223), eof bit should be set now
}


/** Executes bop (begin of page) command.
 *  Format: bop c0[+4] ... c9[+4] p[+4] */
void BasicDVIReader::cmdBop (int)         {seek(44, ios::cur);}
void BasicDVIReader::cmdEop (int)         {}
void BasicDVIReader::cmdPush (int)        {}
void BasicDVIReader::cmdPop (int)         {}
void BasicDVIReader::cmdSetChar0 (int)    {}
void BasicDVIReader::cmdSetChar (int len) {seek(len, ios::cur);}
void BasicDVIReader::cmdPutChar (int len) {seek(len, ios::cur);}
void BasicDVIReader::cmdSetRule (int)     {seek(8, ios::cur);}
void BasicDVIReader::cmdPutRule (int)     {seek(8, ios::cur);}
void BasicDVIReader::cmdRight (int len)   {seek(len, ios::cur);}
void BasicDVIReader::cmdDown (int len)    {seek(len, ios::cur);}
void BasicDVIReader::cmdX0 (int)          {}
void BasicDVIReader::cmdY0 (int)          {}
void BasicDVIReader::cmdW0 (int)          {}
void BasicDVIReader::cmdZ0 (int)          {}
void BasicDVIReader::cmdX (int len)       {seek(len, ios::cur);}
void BasicDVIReader::cmdY (int len)       {seek(len, ios::cur);}
void BasicDVIReader::cmdW (int len)       {seek(len, ios::cur);}
void BasicDVIReader::cmdZ (int len)       {seek(len, ios::cur);}
void BasicDVIReader::cmdNop (int)         {}
void BasicDVIReader::cmdDir (int)         {seek(1, ios::cur);}
void BasicDVIReader::cmdFontNum0 (int)    {}
void BasicDVIReader::cmdFontNum (int len) {seek(len, ios::cur);}
void BasicDVIReader::cmdXXX (int len)     {seek(readUnsigned(len), ios::cur);}


/** Executes fontdef command.
 *  Format: fontdef k[len] c[4] s[4] d[4] a[1] l[1] n[a+l]
 * @param[in] len size of font number variable (in bytes) */
void BasicDVIReader::cmdFontDef (int len) {
	seek(len+12, ios::cur);               // skip font number
	uint32_t pathlen  = readUnsigned(1);  // length of font path
	uint32_t namelen  = readUnsigned(1);  // length of font name
	seek(pathlen+namelen, ios::cur);
}


/** XDV extension: include image or pdf file.
 *  parameters: box[1] matrix[4][6] p[2] len[2] path[l] */
void BasicDVIReader::cmdXPic (int) {
	seek(1+24+2, ios::cur);
	uint16_t len = readUnsigned(2);
	seek(len, ios::cur);
}


void BasicDVIReader::cmdXFontDef (int) {
	seek(4+4, ios::cur);
	uint16_t flags = readUnsigned(2);
	uint8_t len = readUnsigned(1);
	if (_dviVersion == DVI_XDV5)
		len += readUnsigned(1)+readUnsigned(1);
	seek(len, ios::cur);
	if (_dviVersion >= DVI_XDV6)
		seek(4, ios::cur); // skip subfont index
	if (flags & 0x0200)   // colored?
		seek(4, ios::cur);
	if (flags & 0x1000)   // extend?
		seek(4, ios::cur);
	if (flags & 0x2000)   // slant?
		seek(4, ios::cur);
	if (flags & 0x4000)   // embolden?
		seek(4, ios::cur);
	if ((flags & 0x0800) && (_dviVersion == DVI_XDV5)) { // variations?
		uint16_t num_variations = readSigned(2);
		seek(4*num_variations, ios::cur);
	}
}


/** XDV extension: prints an array of characters where each character
 *  can take independent x and y coordinates.
 *  parameters: w[4] n[2] xy[(4+4)n] g[2n] */
void BasicDVIReader::cmdXGlyphArray (int) {
	seek(4, ios::cur);
	uint16_t num_glyphs = readUnsigned(2);
	seek(10*num_glyphs, ios::cur);
}


/** XDV extension: prints an array/string of characters where each character
 *  can take independent x coordinates whereas all share a single y coordinate.
 *  parameters: w[4] n[2] x[4n] y[4] g[2n] */
void BasicDVIReader::cmdXGlyphString (int) {
	seek(4, ios::cur);
	uint16_t num_glyphs = readUnsigned(2);
	seek(6*num_glyphs, ios::cur);
}


/** XDV extension: Same as cmdXGlyphArray plus a leading array of UTF-16 characters
 *  that specify the "actual text" represented by the glyphs to be printed. It usually
 *  contains the text with special characters (like ligatures) expanded so that it
 *  can be used for text search, plain text copy & paste etc. This XDV command was
 *  introduced with XeTeX 0.99995 and can be triggered by <tt>\\XeTeXgenerateactualtext1</tt>.
 *  parameters: l[2] t[2l] w[4] n[2] xy[8n] g[2n] */
void BasicDVIReader::cmdXTextAndGlyphs (int) {
	uint16_t l = readUnsigned(2);
	seek(2*l, ios::cur);
	cmdXGlyphArray(0);
}
