// -*- related-file-name: "../include/efont/t1unparser.hh" -*-

/* t1unparser.{cc,hh} -- debug printing of Type 1 fonts
 *
 * Copyright (c) 1998-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <efont/t1unparser.hh>
namespace Efont {

CharstringUnparser::CharstringUnparser()
    : CharstringInterp(),
      _one_command_per_line(false), _start_of_line(true)
{
}

CharstringUnparser::CharstringUnparser(const CharstringUnparser &o)
    : CharstringInterp(o),
      _one_command_per_line(o._one_command_per_line),
      _start_of_line(o._start_of_line)
{
}

void
CharstringUnparser::clear()
{
    _sa.clear();
    _start_of_line = true;
}

bool
CharstringUnparser::number(double n)
{
    if (_start_of_line) {
        _sa << _indent;
        _start_of_line = false;
    } else
        _sa << ' ';
    _sa << n;
    return true;
}

bool
CharstringUnparser::type1_command(int cmd)
{
    if (_start_of_line) {
        _sa << _indent;
        _start_of_line = false;
    } else
        _sa << ' ';
    if (cmd >= 0 && cmd <= Cs::cLastCommand)
        _sa << Cs::command_names[cmd];
    else
        _sa << "UNKNOWN_12_" << (cmd - 32);
    if (_one_command_per_line) {
        _sa << '\n';
        _start_of_line = true;
    }
    return true;
}

bool
CharstringUnparser::type2_command(int cmd, const unsigned char *data, int *left)
{
    if (_start_of_line) {
        _sa << _indent;
        _start_of_line = false;
    } else
        _sa << ' ';

    if (cmd >= 0 && cmd <= Cs::cLastCommand)
        _sa << Cs::command_names[cmd];
    else
        _sa << "UNKNOWN_12_" << (cmd - 32);

    switch (cmd) {
      case Cs::cHstem: case Cs::cHstemhm: case Cs::cVstem: case Cs::cVstemhm:
      case Cs::cHintmask: case Cs::cCntrmask:
        CharstringInterp::type2_command(cmd, data, left);
        break;
    }

    if (_one_command_per_line) {
        _sa << '\n';
        _start_of_line = true;
    }

    return true;
}

void
CharstringUnparser::act_hintmask(int, const unsigned char *data, int nhints)
{
    _sa << '[';
    for (int i = 0; i < nhints; i++, data++)
        sprintf(_sa.extend(2), "%02X", *data);
    _sa << ']';
}

String
CharstringUnparser::value()
{
    _start_of_line = true;
    return _sa.take_string();
}

String
CharstringUnparser::unparse(const Charstring *cs)
{
    if (cs) {
        CharstringUnparser u;
        u.interpret(0, cs);
        return u.value();
    } else
        return "(null)";
}

String
CharstringUnparser::unparse(const Charstring &cs)
{
    CharstringUnparser u;
    u.interpret(0, &cs);
    return u.value();
}

}
