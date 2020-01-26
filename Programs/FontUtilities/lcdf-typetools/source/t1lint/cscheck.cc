/* cscheck.{cc,hh} -- checking Type 1 charstrings for validity
 *
 * Copyright (c) 1999-2019 Eddie Kohler
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
#include "cscheck.hh"
#include <efont/t1item.hh>
#include <lcdf/error.hh>
#include <efont/t1unparser.hh>

using namespace Efont;

#define CHECK_STACK_GE(numargs)	do { if (size() < numargs) return error(errUnderflow, cmd); } while (0)
#define CHECK_STACK_EQ(numargs)	do { CHECK_STACK_GE(numargs); if (size() > numargs) _errh->warning("too many arguments to %<%s%> (have %d, expected %d)", Charstring::command_name(cmd).c_str(), size(), numargs); } while (0)
#define CHECK_STACK_CPEQ(numargs) do { CHECK_STACK_EQ(numargs); if (!_cp_exists) return error(errCurrentPoint, cmd); } while (0)


CharstringChecker::CharstringChecker()
    : CharstringInterp(), _errh(0)
{
    set_careful(true);
}

CharstringChecker::CharstringChecker(const Vector<double> &weight)
    : CharstringInterp(weight), _errh(0)
{
    set_careful(true);
}


void
CharstringChecker::stem(double y, double dy, const char *cmd_name)
{
    bool is_v = (cmd_name[0] == 'v');
    Vector<double> &hints = (is_v ? _h_vstem : _h_hstem);
    const char *dimen_name = (is_v ? "x" : "y");
    if (dy < 0) {
	y += dy;
	dy = -dy;
    }
    if (dy < 0.5)
	_errh->warning("small delta-%s in %<%s%> (%g)", dimen_name, cmd_name, dy);
    for (int i = 0; i < hints.size(); i += 2)
	if ((hints[i] >= y && hints[i+1] <= y)
	    || (hints[i] >= y+dy && hints[i+1] <= y+dy))
	    _errh->warning("overlapping %<%s%> hints", cmd_name);
    hints.push_back(y);
    hints.push_back(y+dy);
}

void
CharstringChecker::check_stem3(const char *cmd_name)
{
    bool is_v = (cmd_name[0] == 'v');
    Vector<double> &hints = (is_v ? _h_vstem : _h_hstem);
    Vector<double> &old_hints = (is_v ? _h_vstem3 : _h_hstem3);
    assert(hints.size() == 6);

    // sort hints
    int i0, i1, i2;
    if (hints[0] > hints[2])
	i0 = 2, i1 = 0;
    else
	i0 = 0, i1 = 2;
    if (hints[4] < hints[i0])
	i2 = i1, i1 = i0, i0 = 4;
    else if (hints[4] < hints[i1])
	i2 = i1, i1 = 4;
    else
	i2 = 4;

    // check constraints. count "almost equal" as equal
    double stemw0 = hints[i0+1] - hints[i0];
    double stemw2 = hints[i2+1] - hints[i2];
    if ((int)(1024*(stemw0 - stemw2) + .5) != 0)
	_errh->error("bad %<%s%>: extreme stem widths unequal (%g, %g)", cmd_name, stemw0, stemw2);

    double c0 = (hints[i0] + hints[i0+1])/2;
    double c1 = (hints[i1] + hints[i1+1])/2;
    double c2 = (hints[i2] + hints[i2+1])/2;
    if ((int)(1024*((c1 - c0) - (c2 - c1)) + .5) != 0)
	_errh->error("bad %<%s%>: stem gaps unequal (%g, %g)", cmd_name, c1-c0, c2-c1);

    // compare to old hints
    if (old_hints.size()) {
	for (int i = 0; i < old_hints.size(); i++)
	    if (hints[i] != old_hints[i]) {
		_errh->warning("%<%s%> conflicts with old %<%s%>", cmd_name, cmd_name);
		break;
	    }
    }
    old_hints = hints;
}

void
CharstringChecker::moveto(double, double, bool cp_exists)
{
    _cp_exists = cp_exists;
}

void
CharstringChecker::rmoveto(double, double)
{
    _cp_exists = true;
    _just_flexed = false;
}

void
CharstringChecker::rlineto(double, double)
{
}

void
CharstringChecker::rrcurveto(double, double, double, double, double, double)
{
}


bool
CharstringChecker::error(int which, int data)
{
    CharstringInterp::error(which, data);
    _errh->error("%s", error_string().c_str());
    return false;
}

bool
CharstringChecker::callothersubr()
{
    int othersubrnum = (int)top(0);
    int n = (int)top(1);
    int i;

    pop(2);
    if (othersubrnum < 0 || size() < n)
        return false;

    if (!_started && (othersubrnum < Cs::othcCountercontrolpart
                      || othersubrnum > Cs::othcMM6))
        _errh->warning("first command not %<hsbw%> or %<sbw%>");

    bool retval = true;

    switch (othersubrnum) {

    case Cs::othcFlexend:
        if (n != 3) {
            _errh->error("wrong number of arguments to Flex");
            goto unknown;
        }
        if (!_flex) {
            _errh->error("no Flex in progress");
            retval = false;
        } else if (ps_size() != 16) {
            _errh->error("Flex needs 16 arguments, have %d", ps_size());
            retval = false;
        } else {
            ps_clear();
            ps_push(top(0));
            ps_push(top(1));
            _flex = false;
        }
        break;

    case Cs::othcFlexbegin:
        if (n != 0) {
            _errh->error("wrong number of arguments to Flex");
            goto unknown;
        }
        ps_clear();
        ps_push(_cp.x);
        ps_push(_cp.y);
        _flex = true;
        _just_flexed = true;
        //_flex_connect = _connect;
        break;

    case Cs::othcFlexmiddle:
        if (n != 0) {
            _errh->error("wrong number of arguments to Flex");
            goto unknown;
        }
        if (!_flex)
            retval = error(errFlex, 0);
        else {
            if (_just_flexed)
                _errh->error("Flex control points must be separated by a moveto");
            ps_push(_cp.x);
            ps_push(_cp.y);
            _just_flexed = true;
        }
        break;

    case Cs::othcReplacehints:
        if (n != 1) {
            _errh->error("wrong number of arguments to hint replacement");
            goto unknown;
        }
        ps_clear();
        ps_push(top());
        _h_hstem.clear();
        _h_vstem.clear();
        break;

    case Cs::othcCountercontrolpart:
    case Cs::othcCountercontrol:
        if (_counter_controlled)
            _errh->error("duplicate counter control instructions");
        else if (_started
                 && _last_command != 256 + Cs::othcCountercontrolpart
                 && _last_command != Cs::cSbw
                 && _last_command != Cs::cHsbw)
            _errh->error("counter control must appear immediately after %<sbw%> or %<hsbw%>");
        if (n < 0 || n > 22)
            _errh->error("too many arguments to counter control, max 22");
        else if (n != size()) {
            _errh->error("too few arguments to counter control, expected %d", size());
            n = size();
        }
        ps_clear();
        _counter_controlled = (othersubrnum == Cs::othcCountercontrol);
        break;

    case Cs::othcMM1:
    case Cs::othcMM2:
    case Cs::othcMM3:
    case Cs::othcMM4:
    case Cs::othcMM6:
        retval = mm_command(othersubrnum, n);
        goto skip_pop;

    default:			// unknown
    unknown:
        _errh->warning("unknown callothersubr %<%d%>", othersubrnum);
        ps_clear();
        for (i = 0; i < n; i++)
            ps_push(top(i));
        break;

    }

    pop(n);
    if (_last_command == 256 + Cs::othcCountercontrolpart
        && othersubrnum != Cs::othcCountercontrol)
        _errh->error("partial counter control instruction");
    _last_command = 256 + othersubrnum;
 skip_pop:
    return retval;
}

//#define DEBUG(s) printf s
#define DEBUG(s)

bool
CharstringChecker::type1_command(int cmd)
{
    if (_subrno < 0)
	++_ncommand;
    if (cmd == Cs::cCallsubr) {
	int old_subrno = _subrno;
	_subrno = (size() > 1 ? (int) top(0) : -300);
	bool result = callsubr_command();
	_subrno = old_subrno;
	return result;
    } else if (cmd == Cs::cCallothersubr) {
	CHECK_STACK_GE(2);
	return callothersubr();
    } else if (cmd == Cs::cReturn) {
	return false;
    } else if (cmd == Cs::cPop || cmd == Cs::cDiv) {
	return arith_command(cmd);
    }

    if (cmd != Cs::cHsbw && cmd != Cs::cSbw) {
        if (!_started)
            _errh->warning("first command not %<hsbw%> or %<sbw%>");
    } else {
        if (_started)
            _errh->error("duplicate %<hsbw%> or %<sbw%>");
    }
    _started = true;

  switch (cmd) {

   case Cs::cHsbw:
    CHECK_STACK_EQ(2);
    moveto(at(0), 0, false);
    clear();
    break;

   case Cs::cSbw:
    CHECK_STACK_EQ(4);
    moveto(at(0), at(1), false);
    clear();
    break;

   case Cs::cClosepath:
    _cp_exists = false;
    clear();
    break;

   case Cs::cHlineto:
    CHECK_STACK_CPEQ(1);
    rlineto(at(0), 0);
    clear();
    break;

   case Cs::cHmoveto:
    CHECK_STACK_EQ(1);
    rmoveto(at(0), 0);
    clear();
    break;

   case Cs::cHvcurveto:
    CHECK_STACK_CPEQ(4);
    rrcurveto(at(0), 0, at(1), at(2), 0, at(3));
    clear();
    break;

   case Cs::cRlineto:
    CHECK_STACK_CPEQ(2);
    rlineto(at(0), at(1));
    clear();
    break;

   case Cs::cRmoveto:
    CHECK_STACK_EQ(2);
    rmoveto(at(0), at(1));
    clear();
    break;

   case Cs::cRrcurveto:
    CHECK_STACK_CPEQ(6);
    rrcurveto(at(0), at(1), at(2), at(3), at(4), at(5));
    clear();
    break;

   case Cs::cVhcurveto:
    CHECK_STACK_CPEQ(4);
    rrcurveto(0, at(0), at(1), at(2), at(3), 0);
    clear();
    break;

   case Cs::cVlineto:
    CHECK_STACK_CPEQ(1);
    rlineto(0, at(0));
    clear();
    break;

   case Cs::cVmoveto:
    CHECK_STACK_EQ(1);
    rmoveto(0, at(0));
    clear();
    break;

   case Cs::cHstem:
    CHECK_STACK_EQ(2);
    if (_hstem3 && !_hstem)
      _errh->error("charstring has both %<hstem%> and %<hstem3%>");
    _hstem = true;
    stem(at(0), at(1), "hstem");
    clear();
    break;

   case Cs::cVstem:
    CHECK_STACK_EQ(2);
    if (_vstem3 && !_vstem)
      _errh->error("charstring has both %<vstem%> and %<vstem3%>");
    _vstem = true;
    stem(at(0), at(1), "vstem");
    clear();
    break;

   case Cs::cEndchar:
    set_done();
    _last_command = cmd;
    return false;

   case Cs::cDotsection:
    break;

   case Cs::cVstem3:
    CHECK_STACK_EQ(6);
    if (_vstem && !_vstem3)
      _errh->error("charstring has both %<vstem%> and %<vstem3%>");
    _vstem3 = true;
    _h_vstem.clear();
    stem(at(0), at(1), "vstem3");
    stem(at(2), at(3), "vstem3");
    stem(at(4), at(5), "vstem3");
    check_stem3("vstem3");
    clear();
    break;

   case Cs::cHstem3:
    CHECK_STACK_EQ(6);
    if (_hstem && !_hstem3)
      _errh->error("charstring has both %<hstem%> and %<hstem3%>");
    _hstem3 = true;
    _h_hstem.clear();
    stem(at(0), at(1), "hstem3");
    stem(at(2), at(3), "hstem3");
    stem(at(4), at(5), "hstem3");
    check_stem3("hstem3");
    clear();
    break;

   case Cs::cSeac: {
     CHECK_STACK_EQ(5);
#if 0
     double asb = at(0);
     double adx = at(1);
     double ady = at(2);
     int bchar = (int)at(3);
     int achar = (int)at(4);

     double ax = adx - asb + _sidebearing.x;
     double ay = ady + _sidebearing.y;

     Point real_sidebearing = _sidebearing;
     Point real_char_width = _char_width;
     Point original_origin = _origin;

     Type1Encoding *adobe = Type1Encoding::standard_encoding();
     if (!adobe)
       return error(errInternal, cmd);
     Type1Charstring *t1cs = get_glyph((*adobe)[achar]);
     if (!t1cs) ERROR(errGlyph);
     _origin = Point(ax, ay);
     init();
     t1cs->run(*this);
     _origin = original_origin;
     if (error()) return false;

     t1cs = get_glyph((*adobe)[bchar]);
     if (!t1cs) ERROR(errGlyph);
     init();
     t1cs->run(*this);
     _sidebearing = real_sidebearing;
     _char_width = real_char_width;
#endif
     return false;
   }

   case Cs::cSetcurrentpoint:
    CHECK_STACK_EQ(2);
    _cp = Point(at(0), at(1));
    _cp_exists = true;
    clear();
    break;

   case Cs::cPut:
   case Cs::cGet:
   case Cs::cStore:
   case Cs::cLoad:
    return vector_command(cmd);

   default:
    return arith_command(cmd);

  }

  if (_last_command == 256 + Cs::othcCountercontrolpart)
      _errh->error("partial counter control instruction");
  _last_command = cmd;
  return true;
}

bool
CharstringChecker::check(const CharstringContext &g, ErrorHandler *errh)
{
    CharstringCheckerErrorHandler<CharstringChecker> merrh(errh, this);
    _errh = &merrh;
    int old_errors = errh->nerrors();

    _started = false;
    _flex = false;
    _hstem = _hstem3 = _vstem = _vstem3 = false;
    _just_flexed = _counter_controlled = false;
    _last_command = -1;
    _h_hstem.clear();
    _h_vstem.clear();
    _h_hstem3.clear();
    _h_vstem3.clear();
    _ncommand = 0;
    _subrno = -1;

    CharstringInterp::interpret(g);

    return errh->nerrors() == old_errors;
}



CharstringSubrChecker::CharstringSubrChecker()
    : CharstringInterp(), _errh(0)
{
    set_careful(true);
}

CharstringSubrChecker::CharstringSubrChecker(const Vector<double> &weight)
    : CharstringInterp(weight), _errh(0)
{
    set_careful(true);
}

bool
CharstringSubrChecker::error(int error, int data)
{
    CharstringInterp::error(error, data);
    _errh->error("%s", error_string().c_str());
    return false;
}

bool
CharstringSubrChecker::type1_command(int cmd)
{
    switch (cmd) {
    case Cs::cReturn:
	_returned = true;
	return false;
    default:
	clear();
	return true;
    }
}

bool
CharstringSubrChecker::check(const CharstringContext &g, ErrorHandler *errh)
{
    CharstringCheckerErrorHandler<CharstringSubrChecker> merrh(errh, this);
    _errh = &merrh;
    int old_errors = errh->nerrors();

    _returned = false;

    CharstringInterp::interpret(g);

    if (!_returned)
	_errh->error("subroutine does not return");
    return errh->nerrors() == old_errors;
}
