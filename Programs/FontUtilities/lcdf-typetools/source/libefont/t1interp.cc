// -*- related-file-name: "../include/efont/t1interp.hh" -*-

/* t1interp.{cc,hh} -- Type 1/2 charstring interpretation
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
#include <efont/t1interp.hh>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef WIN32
# define random() rand()
#endif

#define CHECK_STACK(numargs)    do { if (size() < numargs) return error(errUnderflow, cmd); } while (0)
#define CHECK_STATE()           do { if (_state < S_IPATH) return error(errOrdering, cmd); } while (0)
#define CHECK_PATH_START()      do { _state = S_PATH; } while (0)
#define CHECK_PATH_END()        do { if (_state == S_PATH) { act_closepath(cmd); } _state = S_IPATH; } while (0)

#ifndef static_assert
# define static_assert(c, msg)  switch ((int) (c)) case 0: case (c):
#endif

namespace Efont {

static const char * const error_formats[] = {
    "charstring OK",                            // errOK
    "charstring internal error in '%C'",        // errInternal
    "charstring commands past end",             // errRunoff
    "charstring command '%C' unimplemented",    // errUnimplemented
    "charstring stack overflow",                // errOverflow
    "charstring stack underflow in '%C'",       // errUnderflow
    "charstring bad vector operation in '%C'",  // errVector
    "charstring bad value in '%C'",             // errValue
    "charstring bad subroutine number %d",      // errSubr
    "charstring bad glyph number '%d'",         // errGlyph
    "charstring no current point in '%C'",      // errCurrentPoint
    "charstring flex error",                    // errFlex
    "charstring multiple master error in '%C'", // errMultipleMaster
    "charstring open stroke",                   // errOpenStroke
    "charstring late sidebearing command '%C'", // errLateSidebearing
    "charstring bad othersubr number %d",       // errOthersubr
    "charstring ordering constraints violated at '%C'", // errOrdering
    "charstring inappropriate hintmask",        // errHintmask
    "charstring subrs nested too deep at '%d'"  // errSubrDepth
};

double CharstringInterp::double_for_error;


CharstringInterp::CharstringInterp()
    : _error(errOK), _careful(false), _sp(0), _ps_sp(0),
      _scratch_vector(SCRATCH_SIZE, 0), _program(0)
{
}

CharstringInterp::CharstringInterp(const Vector<double> &weight_vector)
    : _error(errOK), _careful(false), _sp(0), _ps_sp(0),
      _weight_vector(weight_vector),
      _scratch_vector(SCRATCH_SIZE, 0), _program(0)
{
}

void
CharstringInterp::initialize()
{
    clear();
    ps_clear();
    _lsb = _cp = _seac_origin = Point(0, 0);
    _state = S_INITIAL;
    _flex = false;
    _t2nhints = 0;
    _subr_depth = 0;
    _done = false;
    _error = errOK;
}

bool
CharstringInterp::interpret(const CharstringProgram *program, const Charstring *cs)
{
    if (cs) {
        initialize();
        _program = program;
        cs->process(*this);
        return _error == errOK;
    } else
        return error(errGlyph, 0);
}

bool
CharstringInterp::error(int err, int error_data)
{
    _error = err;
    _error_data = error_data;
    return false;
}

String
CharstringInterp::error_string(int error, int error_data)
{
    static_assert(-errLastError == (sizeof(error_formats) / sizeof(error_formats[0])) - 1,
                  "errLastError defined incorrectly");
    if (error >= 0)
        return error_formats[0];
    else if (error < errLastError)
        return "charstring unknown error number " + String(error);
    String format = error_formats[-error];
    int percent = format.find_left('%');
    if (percent >= 0 && format[percent + 1] == 'C')
        return format.substring(0, percent) + Charstring::command_name(error_data) + format.substring(percent + 2);
    else if (percent >= 0 && format[percent + 1] == 'd')
        return format.substring(0, percent) + String(error_data) + format.substring(percent + 2);
    else
        return format;
}

bool
CharstringInterp::number(double v)
{
    push(v);
    return true;
}

void
CharstringInterp::fetch_weight_vector()
{
    if (_program)
        if (Vector<double> *wv = _program->mm_vector(CharstringProgram::VEC_WEIGHT, false))
            _weight_vector = *wv;
}

bool
CharstringInterp::vector_command(int cmd)
{
    CharstringProgram::VectorType which_vector;
    int vectoroff, offset, num, i;
    Vector<double> *v = 0;

    switch (cmd) {

      case Cs::cPut:
        CHECK_STACK(2);
        offset = (int)top(0);
        vec(&_scratch_vector, offset) = top(1);
        pop(2);
        break;

      case Cs::cGet:
        CHECK_STACK(1);
        offset = (int)top();
        top() = vec(&_scratch_vector, offset);
        break;

      case Cs::cStore:
        CHECK_STACK(4);
        which_vector = (CharstringProgram::VectorType)((int)top(3));
        vectoroff = (int)top(2);
        offset = (int)top(1);
        num = (int)top(0);
        pop(4);

        if (!_program)
            return error(errVector, cmd);
        v = _program->mm_vector(which_vector, true);
        if (!v)
            return error(errVector, cmd);

        for (i = 0; i < num; i++, offset++, vectoroff++)
            vec(v, vectoroff) = vec(&_scratch_vector, offset);
        // erase our weight vector if the global weight vector has changed
        if (which_vector == CharstringProgram::VEC_WEIGHT)
            _weight_vector.clear();
        break;

      case Cs::cLoad:
        CHECK_STACK(3);
        which_vector = (CharstringProgram::VectorType)((int)top(2));
        offset = (int)top(1);
        num = (int)top(0);
        pop(3);

        if (!_program)
            return error(errVector, cmd);
        v = _program->mm_vector(which_vector, false);
        // use our weight vector if appropriate
        if (!v && which_vector == CharstringProgram::VEC_WEIGHT && _weight_vector.size())
            v = &_weight_vector;
        if (!v)
            return error(errVector, cmd);

        for (i = 0; i < num; i++, offset++)
            vec(&_scratch_vector, offset) = vec(v, i);
        break;

      default:
        return error(errUnimplemented, cmd);

    }

    return true;
}

bool
CharstringInterp::blend_command()
{
    const int cmd = Cs::cBlend;
    CHECK_STACK(1);
    int nargs = (int)pop();

    ensure_weight_vector();
    if (!_weight_vector.size())
        return error(errVector, cmd);

    int nmasters = _weight_vector.size();
    CHECK_STACK(nargs * nmasters);

    int base = _sp - nargs * nmasters;
    int off = base + nargs;
    for (int j = 0; j < nargs; j++) {
        double &val = _s[base + j];
        for (int i = 1; i < nmasters; i++, off++)
            val += _weight_vector.at_u(i) * _s[off];
    }

    pop(nargs * (nmasters - 1));
    return true;
}

bool
CharstringInterp::roll_command()
{
    const int cmd = Cs::cRoll;
    CHECK_STACK(2);
    int amount = (int)pop();
    int n = (int)pop();
    if (n <= 0)
        return error(errValue, cmd);
    CHECK_STACK(n);

    int base = _sp - n;
    while (amount < 0)
        amount += n;

    int i;
    double copy_stack[STACK_SIZE];
    for (i = 0; i < n; i++)
        copy_stack[i] = _s[ base + (i+amount) % n ];
    for (i = 0; i < n; i++)
        _s[base + i] = copy_stack[i];

    return true;
}

bool
CharstringInterp::arith_command(int cmd)
{
    int i;
    double d;

    switch (cmd) {

      case Cs::cBlend:
        return blend_command();

      case Cs::cAbs:
        CHECK_STACK(1);
        if (top() < 0)
            top() = -top();
        break;

      case Cs::cAdd:
        CHECK_STACK(1);
        d = pop();
        top() += d;
        break;

      case Cs::cSub:
        CHECK_STACK(1);
        d = pop();
        top() -= d;
        break;

      case Cs::cDiv:
        CHECK_STACK(2);
        d = pop();
        top() /= d;
        break;

      case Cs::cNeg:
        CHECK_STACK(1);
        top() = -top();
        break;

      case Cs::cRandom: {
          double d;
          do {
              d = random() / ((double)RAND_MAX);
          } while (d == 0);
          push(d);
          break;
      }

      case Cs::cMul:
        CHECK_STACK(2);
        d = pop();
        top() *= d;
        break;

      case Cs::cSqrt:
        CHECK_STACK(1);
        if (top() < 0)
            return error(errValue, cmd);
        top() = sqrt(top());
        break;

      case Cs::cDrop:
        CHECK_STACK(1);
        pop();
        break;

      case Cs::cExch:
        CHECK_STACK(2);
        d = top(0);
        top(0) = top(1);
        top(1) = d;
        break;

      case Cs::cIndex:
        CHECK_STACK(1);
        i = (int)top();
        if (i < 0)
            return error(errValue, cmd);
        CHECK_STACK(i + 2);
        top() = top(i+1);
        break;

      case Cs::cRoll:
        return roll_command();

      case Cs::cDup:
        CHECK_STACK(1);
        push(top());
        break;

      case Cs::cAnd:
        CHECK_STACK(2);
        d = pop();
        top() = (top() != 0) && (d != 0);
        break;

      case Cs::cOr:
        CHECK_STACK(2);
        d = pop();
        top() = (top() != 0) || (d != 0);
        break;

      case Cs::cNot:
        CHECK_STACK(1);
        top() = (top() == 0);
        break;

      case Cs::cEq:
        CHECK_STACK(2);
        d = pop();
        top() = (top() == d);
        break;

      case Cs::cIfelse:
        CHECK_STACK(4);
        if (top(1) > top(0))
            top(3) = top(2);
        pop(3);
        break;

      case Cs::cPop:
        if (ps_size() < 1)
            return error(errUnderflow, cmd);
        push(ps_pop());
        break;

      case 15:
        // this command is found with no explanation in JansonText-Roman
        CHECK_STACK(2);
        pop(2);
        return true;

      default:
        return error(errUnimplemented, cmd);

    }

    return true;
}

bool
CharstringInterp::callsubr_command()
{
    const int cmd = Cs::cCallsubr;
    CHECK_STACK(1);
    int which = (int)pop();

    Charstring *subr_cs = get_subr(which);
    if (!subr_cs)
        return error(errSubr, which);

    if (_subr_depth >= MAX_SUBR_DEPTH)
        return error(errSubrDepth, which);
    _subr_depth++;

    subr_cs->process(*this);

    _subr_depth--;
    if (_error != errOK)
        return false;
    return !done();
}

bool
CharstringInterp::callgsubr_command()
{
    const int cmd = Cs::cCallgsubr;
    CHECK_STACK(1);
    int which = (int)pop();

    Charstring *subr_cs = get_gsubr(which);
    if (!subr_cs)
        return error(errSubr, which);

    if (_subr_depth >= MAX_SUBR_DEPTH)
        return error(errSubrDepth, which);
    _subr_depth++;

    subr_cs->process(*this);

    _subr_depth--;
    if (_error != errOK)
        return false;
    return !done();
}

bool
CharstringInterp::mm_command(int command, int on_stack)
{
    ensure_weight_vector();
    if (!_weight_vector.size())
        return error(errVector, command);

    int nargs;
    switch (command) {
      case Cs::othcMM1: nargs = 1; break;
      case Cs::othcMM2: nargs = 2; break;
      case Cs::othcMM3: nargs = 3; break;
      case Cs::othcMM4: nargs = 4; break;
      case Cs::othcMM6: nargs = 6; break;
      default: return error(errInternal, command);
    }

    int nmasters = _weight_vector.size();
    if (size() < nargs * nmasters
        || on_stack != nargs * nmasters)
        return error(errMultipleMaster, command);

    int base = size() - on_stack;

    int off = base + nargs;
    for (int j = 0; j < nargs; j++) {
        double &val = at(base + j);
        for (int i = 1; i < nmasters; i++, off++)
            val += _weight_vector.at_u(i) * at(off);
    }

    for (int i = nargs - 1; i >= 0; i--)
        ps_push(at(base + i));

    pop(on_stack);
    return true;
}

bool
CharstringInterp::itc_command(int command, int on_stack)
{
    ensure_weight_vector();
    if (!_weight_vector.size())
        return error(errVector, command);

    int base = size() - on_stack;
    switch (command) {

      case Cs::othcITC_load: {
          if (on_stack != 1)
              return error(errOthersubr, command);
          int offset = (int)at(base);
          for (int i = 0; i < _weight_vector.size(); i++)
              vec(&_scratch_vector, offset+i) = _weight_vector.at_u(i);
          break;
      }

      case Cs::othcITC_put: {
          if (on_stack != 2)
              return error(errOthersubr, command);
          int offset = (int)at(base+1);
          vec(&_scratch_vector, offset) = at(base);
          break;
      }

      case Cs::othcITC_get: {
          if (on_stack != 1)
              return error(errOthersubr, command);
          int offset = (int)at(base);
          ps_push(vec(&_scratch_vector, offset));
          break;
      }

      case Cs::othcITC_add: {
          if (on_stack != 2)
              return error(errOthersubr, command);
          ps_push(at(base) + at(base+1));
          break;
      }

      case Cs::othcITC_sub: {
          if (on_stack != 2)
              return error(errOthersubr, command);
          ps_push(at(base) - at(base+1));
          break;
      }

      case Cs::othcITC_mul: {
          if (on_stack != 2)
              return error(errOthersubr, command);
          ps_push(at(base) * at(base+1));
          break;
      }

      case Cs::othcITC_div: {
          if (on_stack != 2)
              return error(errOthersubr, command);
          ps_push(at(base) / at(base+1));
          break;
      }

      case Cs::othcITC_ifelse: {
          if (on_stack != 4)
              return error(errOthersubr, command);
          if (at(base+2) <= at(base+3))
              ps_push(at(base));
          else
              ps_push(at(base+1));
          break;
      }

      default:
        return error(errOthersubr, command);

    }

    pop(on_stack);
    return true;
}


inline void
CharstringInterp::actp_rmoveto(int /*cmd*/, double dx, double dy)
{
    _cp.shift(dx, dy);
}

inline void
CharstringInterp::actp_rlineto(int cmd, double dx, double dy)
{
    Point p0(_cp);
    _cp.shift(dx, dy);
    act_line(cmd, p0, _cp);
}

void
CharstringInterp::actp_rrcurveto(int cmd, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
{
    Point p0(_cp);
    Point p1(p0, dx1, dy1);
    Point p2(p1, dx2, dy2);
    _cp = p2.shifted(dx3, dy3);
    act_curve(cmd, p0, p1, p2, _cp);
}

void
CharstringInterp::actp_rrflex(int cmd, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3, double dx4, double dy4, double dx5, double dy5, double dx6, double dy6, double flex_depth)
{
    Point p0(_cp);
    Point p1(p0, dx1, dy1);
    Point p2(p1, dx2, dy2);
    Point p3_4(p2, dx3, dy3);
    Point p5(p3_4, dx4, dy4);
    Point p6(p5, dx5, dy5);
    _cp = p6.shifted(dx6, dy6);
    act_flex(cmd, p0, p1, p2, p3_4, p5, p6, _cp, flex_depth);
}


bool
CharstringInterp::callothersubr_command(int othersubrnum, int n)
{
    switch (othersubrnum) {

      case Cs::othcFlexend:
        if (n != 3)
            goto unknown;
        if (!_flex || ps_size() != 16)
            return error(errFlex);
        CHECK_PATH_START();
        act_flex(Cs::cCallothersubr, Point(ps_at(0), ps_at(1)),
                 Point(ps_at(4), ps_at(5)), Point(ps_at(6), ps_at(7)),
                 Point(ps_at(8), ps_at(9)), Point(ps_at(10), ps_at(11)),
                 Point(ps_at(12), ps_at(13)), Point(ps_at(14), ps_at(15)),
                 top(2));
        ps_clear();
        ps_push(top(0));
        ps_push(top(1));
        _flex = false;
        _state = S_PATH;
        break;

      case Cs::othcFlexbegin:
        if (n != 0)
            goto unknown;
        if (_flex)
            return error(errFlex);
        ps_clear();
        ps_push(_cp.x);
        ps_push(_cp.y);
        _flex = true;
        _state = S_IPATH;
        break;

      case Cs::othcFlexmiddle:
        if (n != 0)
            goto unknown;
        if (!_flex)
            return error(errFlex);
        ps_push(_cp.x);
        ps_push(_cp.y);
        break;

      case Cs::othcReplacehints:
        if (n != 1)
            goto unknown;
        ps_clear();
        ps_push(top());
        break;

      case Cs::othcMM1:
      case Cs::othcMM2:
      case Cs::othcMM3:
      case Cs::othcMM4:
      case Cs::othcMM6:
        return mm_command(othersubrnum, n);

      case Cs::othcITC_load:
      case Cs::othcITC_add:
      case Cs::othcITC_sub:
      case Cs::othcITC_mul:
      case Cs::othcITC_div:
      case Cs::othcITC_put:
      case Cs::othcITC_get:
      case Cs::othcITC_unknown:
      case Cs::othcITC_ifelse:
      case Cs::othcITC_random:
        return itc_command(othersubrnum, n);

      default:                  // unknown
      unknown:
        ps_clear();
        for (int i = 0; i < n; i++)
            ps_push(top(i));
        break;

    }

    pop(n);
    return true;
}

bool
CharstringInterp::type1_command(int cmd)
{
    switch (cmd) {

      case Cs::cReturn:
        return false;

      case Cs::cHsbw:
        CHECK_STACK(2);
        if (_state > S_SEAC && _careful)
            return error(errOrdering, cmd);
        else {
            _lsb = _cp = _seac_origin.shifted(at(0), 0);
            if (_state == S_INITIAL) {
                act_sidebearing(cmd, _lsb);
                act_width(cmd, Point(at(1), 0));
            }
            if (_state <= S_SEAC)
                _state = S_SBW;
        }
        break;

      case Cs::cSbw:
        CHECK_STACK(4);
        if (_state > S_SEAC && _careful)
            return error(errOrdering, cmd);
        else {
            _lsb = _cp = _seac_origin.shifted(at(0), at(1));
            if (_state == S_INITIAL) {
                act_sidebearing(cmd, _lsb);
                act_width(cmd, Point(at(2), at(3)));
            }
            if (_state <= S_SEAC)
                _state = S_SBW;
        }
        break;

      case Cs::cSeac:
        CHECK_STACK(5);
        if (_state > S_SBW)
            return error(errOrdering, cmd);
        act_seac(cmd, at(0), at(1), at(2), (int)at(3), (int)at(4));
        clear();
        return false;

      case Cs::cCallsubr:
        return callsubr_command();

      case Cs::cCallothersubr: {
          CHECK_STACK(2);
          int othersubrnum = (int)top(0);
          int n = (int)top(1);
          pop(2);
          if (othersubrnum < 0 || size() < n)
              return error(errOthersubr, cmd);
          return callothersubr_command(othersubrnum, n);
      }

      case Cs::cPut:
      case Cs::cGet:
      case Cs::cStore:
      case Cs::cLoad:
        return vector_command(cmd);

      case Cs::cBlend:
      case Cs::cAbs:
      case Cs::cAdd:
      case Cs::cSub:
      case Cs::cDiv:
      case Cs::cNeg:
      case Cs::cRandom:
      case Cs::cMul:
      case Cs::cSqrt:
      case Cs::cDrop:
      case Cs::cExch:
      case Cs::cIndex:
      case Cs::cRoll:
      case Cs::cDup:
      case Cs::cAnd:
      case Cs::cOr:
      case Cs::cNot:
      case Cs::cEq:
      case Cs::cIfelse:
      case Cs::cPop:
        return arith_command(cmd);

      case Cs::cHlineto:
        CHECK_STACK(1);
        CHECK_PATH_START();
        actp_rlineto(cmd, at(0), 0);
        break;

      case Cs::cHmoveto:
        CHECK_STACK(1);
        CHECK_PATH_END();
        actp_rmoveto(cmd, at(0), 0);
        break;

      case Cs::cHvcurveto:
        CHECK_STACK(4);
        CHECK_PATH_START();
        actp_rrcurveto(cmd, at(0), 0, at(1), at(2), 0, at(3));
        break;

      case Cs::cRlineto:
        CHECK_STACK(2);
        CHECK_PATH_START();
        actp_rlineto(cmd, at(0), at(1));
        break;

      case Cs::cRmoveto:
        CHECK_STACK(2);
        CHECK_PATH_END();
        actp_rmoveto(cmd, at(0), at(1));
        break;

      case Cs::cRrcurveto:
        CHECK_STACK(6);
        CHECK_PATH_START();
        actp_rrcurveto(cmd, at(0), at(1), at(2), at(3), at(4), at(5));
        break;

      case Cs::cVhcurveto:
        CHECK_STACK(4);
        CHECK_PATH_START();
        actp_rrcurveto(cmd, 0, at(0), at(1), at(2), at(3), 0);
        break;

      case Cs::cVlineto:
        CHECK_STACK(1);
        CHECK_PATH_START();
        actp_rlineto(cmd, 0, at(0));
        break;

      case Cs::cVmoveto:
        CHECK_STACK(1);
        CHECK_PATH_END();
        actp_rmoveto(cmd, 0, at(0));
        break;

      case Cs::cDotsection:
        break;

      case Cs::cHstem:
        CHECK_STACK(2);
        act_hstem(cmd, _lsb.y + at(0), at(1));
        break;

      case Cs::cHstem3:
        CHECK_STACK(6);
        act_hstem3(cmd, _lsb.y + at(0), at(1), _lsb.y + at(2), at(3), _lsb.y + at(4), at(5));
        break;

      case Cs::cVstem:
        CHECK_STACK(2);
        act_vstem(cmd, _lsb.x + at(0), at(1));
        break;

      case Cs::cVstem3:
        CHECK_STACK(6);
        act_vstem3(cmd, _lsb.x + at(0), at(1), _lsb.x + at(2), at(3), _lsb.x + at(4), at(5));
        break;

      case Cs::cSetcurrentpoint:
        CHECK_STACK(2);
        _cp = Point(at(0), at(1));
        break;

      case Cs::cClosepath:
        CHECK_PATH_END();
        break;

      case Cs::cEndchar:
        CHECK_PATH_END();
        set_done();
        return false;

      case Cs::cError:
      default:
        return error(errUnimplemented, cmd);

    }

    clear();
    return error() >= 0;
}


#undef DEBUG_TYPE2

int
CharstringInterp::type2_handle_width(int cmd, bool have_width)
{
    _cp = _lsb = _seac_origin;
    if (_state != S_INITIAL)
        /* ignore width */;
    else if (have_width)
        act_nominal_width_delta(cmd, at(0));
    else
        act_default_width(cmd);
    return (have_width ? 1 : 0);
}

bool
CharstringInterp::type2_command(int cmd, const uint8_t *data, int *left)
{
    int bottom = 0;

#ifdef DEBUG_TYPE2
    fprintf(stderr, "%s [%d/%d]\n", Charstring::command_name(cmd).c_str(), _t2nhints, size());
#endif

    switch (cmd) {

      case Cs::cHstem:
      case Cs::cHstemhm:
        CHECK_STACK(2);
        if (_state <= S_SEAC)
            bottom = type2_handle_width(cmd, (size() % 2) == 1);
        if (_state > S_HSTEM)
            return error(errOrdering, cmd);
        _state = S_HSTEM;
        for (double pos = 0; bottom + 1 < size(); bottom += 2) {
            _t2nhints++;
            act_hstem(cmd, pos + at(bottom), at(bottom + 1));
            pos += at(bottom) + at(bottom + 1);
        }
        break;

      case Cs::cVstem:
      case Cs::cVstemhm:
        CHECK_STACK(2);
        if (_state <= S_SEAC)
            bottom = type2_handle_width(cmd, (size() % 2) == 1);
        if (_state > S_VSTEM)
            return error(errOrdering, cmd);
        _state = S_VSTEM;
        for (double pos = 0; bottom + 1 < size(); bottom += 2) {
            _t2nhints++;
            act_vstem(cmd, pos + at(bottom), at(bottom + 1));
            pos += at(bottom) + at(bottom + 1);
        }
        break;

      case Cs::cHintmask:
      case Cs::cCntrmask:
        if (_state <= S_SEAC && size() >= 1) {
            bottom = type2_handle_width(cmd, (size() % 2) == 1);
            for (double pos = 0; bottom + 1 < size(); bottom += 2) {
                _t2nhints++;
                act_hstem(cmd, pos + at(bottom), at(bottom + 1));
                pos += at(bottom) + at(bottom + 1);
            }
        }
        if ((_state == S_HSTEM || _state == S_VSTEM) && size() >= 2)
            for (double pos = 0; bottom + 1 < size(); bottom += 2) {
                _t2nhints++;
                act_vstem(cmd, pos + at(bottom), at(bottom + 1));
                pos += at(bottom) + at(bottom + 1);
            }
        if (_state < S_HINTMASK)
            _state = S_HINTMASK;
        if (_t2nhints == 0)
            return error(errHintmask, cmd);
        if (!data || !left)
            return error(errInternal, cmd);
        if (((_t2nhints - 1) >> 3) + 1 > *left)
            return error(errRunoff, cmd);
        act_hintmask(cmd, data, _t2nhints);
        *left -= ((_t2nhints - 1) >> 3) + 1;
        break;

      case Cs::cRmoveto:
        CHECK_STACK(2);
        if (_state <= S_SEAC)
            bottom = type2_handle_width(cmd, size() > 2);
        CHECK_PATH_END();
        actp_rmoveto(cmd, at(bottom), at(bottom + 1));
#if DEBUG_TYPE2
        bottom += 2;
#endif
        break;

      case Cs::cHmoveto:
        CHECK_STACK(1);
        if (_state <= S_SEAC)
            bottom = type2_handle_width(cmd, size() > 1);
        CHECK_PATH_END();
        actp_rmoveto(cmd, at(bottom), 0);
#if DEBUG_TYPE2
        bottom++;
#endif
        break;

      case Cs::cVmoveto:
        CHECK_STACK(1);
        if (_state <= S_SEAC)
            bottom = type2_handle_width(cmd, size() > 1);
        CHECK_PATH_END();
        actp_rmoveto(cmd, 0, at(bottom));
#if DEBUG_TYPE2
        bottom++;
#endif
        break;

      case Cs::cRlineto:
        CHECK_STACK(2);
        CHECK_STATE();
        CHECK_PATH_START();
        for (; bottom + 1 < size(); bottom += 2)
            actp_rlineto(cmd, at(bottom), at(bottom + 1));
        break;

      case Cs::cHlineto:
        CHECK_STACK(1);
        CHECK_STATE();
        CHECK_PATH_START();
        while (bottom < size()) {
            actp_rlineto(cmd, at(bottom++), 0);
            if (bottom < size())
                actp_rlineto(cmd, 0, at(bottom++));
        }
        break;

      case Cs::cVlineto:
        CHECK_STACK(1);
        CHECK_STATE();
        CHECK_PATH_START();
        while (bottom < size()) {
            actp_rlineto(cmd, 0, at(bottom++));
            if (bottom < size())
                actp_rlineto(cmd, at(bottom++), 0);
        }
        break;

      case Cs::cRrcurveto:
        CHECK_STACK(6);
        CHECK_STATE();
        CHECK_PATH_START();
        for (; bottom + 5 < size(); bottom += 6)
            actp_rrcurveto(cmd, at(bottom), at(bottom + 1), at(bottom + 2), at(bottom + 3), at(bottom + 4), at(bottom + 5));
        break;

      case Cs::cHhcurveto:
        CHECK_STACK(4);
        CHECK_STATE();
        CHECK_PATH_START();
        if (size() % 2 == 1) {
            actp_rrcurveto(cmd, at(bottom + 1), at(bottom), at(bottom + 2), at(bottom + 3), at(bottom + 4), 0);
            bottom += 5;
        }
        for (; bottom + 3 < size(); bottom += 4)
            actp_rrcurveto(cmd, at(bottom), 0, at(bottom + 1), at(bottom + 2), at(bottom + 3), 0);
        break;

      case Cs::cHvcurveto:
        CHECK_STACK(4);
        CHECK_STATE();
        CHECK_PATH_START();
        while (bottom + 3 < size()) {
            double dx3 = (bottom + 5 == size() ? at(bottom + 4) : 0);
            actp_rrcurveto(cmd, at(bottom), 0, at(bottom + 1), at(bottom + 2), dx3, at(bottom + 3));
            bottom += 4;
            if (bottom + 3 < size()) {
                double dy3 = (bottom + 5 == size() ? at(bottom + 4) : 0);
                actp_rrcurveto(cmd, 0, at(bottom), at(bottom + 1), at(bottom + 2), at(bottom + 3), dy3);
                bottom += 4;
            }
        }
#if DEBUG_TYPE2
        if (bottom + 1 == size())
            bottom++;
#endif
        break;

      case Cs::cRcurveline:
        CHECK_STACK(8);
        CHECK_STATE();
        CHECK_PATH_START();
        for (; bottom + 7 < size(); bottom += 6)
            actp_rrcurveto(cmd, at(bottom), at(bottom + 1), at(bottom + 2), at(bottom + 3), at(bottom + 4), at(bottom + 5));
        actp_rlineto(cmd, at(bottom), at(bottom + 1));
#if DEBUG_TYPE2
        bottom += 2;
#endif
        break;

      case Cs::cRlinecurve:
        CHECK_STACK(8);
        CHECK_STATE();
        CHECK_PATH_START();
        for (; bottom + 7 < size(); bottom += 2)
            actp_rlineto(cmd, at(bottom), at(bottom + 1));
        actp_rrcurveto(cmd, at(bottom), at(bottom + 1), at(bottom + 2), at(bottom + 3), at(bottom + 4), at(bottom + 5));
#if DEBUG_TYPE2
        bottom += 6;
#endif
        break;

      case Cs::cVhcurveto:
        CHECK_STACK(4);
        CHECK_STATE();
        CHECK_PATH_START();
        while (bottom + 3 < size()) {
            double dy3 = (bottom + 5 == size() ? at(bottom + 4) : 0);
            actp_rrcurveto(cmd, 0, at(bottom), at(bottom + 1), at(bottom + 2), at(bottom + 3), dy3);
            bottom += 4;
            if (bottom + 3 < size()) {
                double dx3 = (bottom + 5 == size() ? at(bottom + 4) : 0);
                actp_rrcurveto(cmd, at(bottom), 0, at(bottom + 1), at(bottom + 2), dx3, at(bottom + 3));
                bottom += 4;
            }
        }
#if DEBUG_TYPE2
        if (bottom + 1 == size())
            bottom++;
#endif
        break;

      case Cs::cVvcurveto:
        CHECK_STACK(4);
        CHECK_STATE();
        CHECK_PATH_START();
        if (size() % 2 == 1) {
            actp_rrcurveto(cmd, at(bottom), at(bottom + 1), at(bottom + 2), at(bottom + 3), 0, at(bottom + 4));
            bottom += 5;
        }
        for (; bottom + 3 < size(); bottom += 4)
            actp_rrcurveto(cmd, 0, at(bottom), at(bottom + 1), at(bottom + 2), 0, at(bottom + 3));
        break;

      case Cs::cFlex:
        CHECK_STACK(13);
        CHECK_STATE();
        CHECK_PATH_START();
        assert(bottom == 0);
        actp_rrflex(cmd,
                    at(0), at(1), at(2), at(3), at(4), at(5),
                    at(6), at(7), at(8), at(9), at(10), at(11),
                    at(12));
#if DEBUG_TYPE2
        bottom += 13;
#endif
        break;

      case Cs::cHflex:
        CHECK_STACK(7);
        CHECK_STATE();
        CHECK_PATH_START();
        assert(bottom == 0);
        actp_rrflex(cmd,
                    at(0), 0, at(1), at(2), at(3), 0,
                    at(4), 0, at(5), -at(2), at(6), 0,
                    50);
#if DEBUG_TYPE2
        bottom += 7;
#endif
        break;

      case Cs::cHflex1:
        CHECK_STACK(9);
        CHECK_STATE();
        CHECK_PATH_START();
        assert(bottom == 0);
        actp_rrflex(cmd,
                    at(0), at(1), at(2), at(3), at(4), 0,
                    at(5), 0, at(6), at(7), at(8), -(at(1) + at(3) + at(7)),
                    50);
#if DEBUG_TYPE2
        bottom += 9;
#endif
        break;

      case Cs::cFlex1: {
          CHECK_STACK(11);
          CHECK_STATE();
          CHECK_PATH_START();
          assert(bottom == 0);
          double dx = at(0) + at(2) + at(4) + at(6) + at(8);
          double dy = at(1) + at(3) + at(5) + at(7) + at(9);
          if (fabs(dx) > fabs(dy))
              actp_rrflex(cmd,
                          at(0), at(1), at(2), at(3), at(4), at(5),
                          at(6), at(7), at(8), at(9), at(10), -dy,
                          50);
          else
              actp_rrflex(cmd,
                          at(0), at(1), at(2), at(3), at(4), at(5),
                          at(6), at(7), at(8), at(9), -dx, at(10),
                          50);
          break;
#if DEBUG_TYPE2
          bottom += 11;
#endif
      }

      case Cs::cEndchar:
        if (_state <= S_SEAC)
            bottom = type2_handle_width(cmd, size() > 0 && size() != 4);
        if (bottom + 3 < size() && _state == S_INITIAL)
            act_seac(cmd, 0, at(bottom), at(bottom + 1), (int)at(bottom + 2), (int)at(bottom + 3));
        CHECK_PATH_END();
        set_done();
        clear();
        return false;

      case Cs::cReturn:
        return false;

      case Cs::cCallsubr:
        return callsubr_command();

      case Cs::cCallgsubr:
        return callgsubr_command();

      case Cs::cPut:
      case Cs::cGet:
      case Cs::cStore:
      case Cs::cLoad:
        return vector_command(cmd);

      case Cs::cBlend:
      case Cs::cAbs:
      case Cs::cAdd:
      case Cs::cSub:
      case Cs::cDiv:
      case Cs::cNeg:
      case Cs::cRandom:
      case Cs::cMul:
      case Cs::cSqrt:
      case Cs::cDrop:
      case Cs::cExch:
      case Cs::cIndex:
      case Cs::cRoll:
      case Cs::cDup:
      case Cs::cAnd:
      case Cs::cOr:
      case Cs::cNot:
      case Cs::cEq:
      case Cs::cIfelse:
      case Cs::cPop:
        return arith_command(cmd);

      case Cs::cDotsection:
        break;

      case Cs::cError:
      default:
        return error(errUnimplemented, cmd);

    }

#if DEBUG_TYPE2
    if (bottom != size())
        fprintf(stderr, "[left %d on stack] ", size() - bottom);
#endif

    clear();
    return error() >= 0;
}


void
CharstringInterp::act_sidebearing(int, const Point &)
{
    /* do nothing */
}

void
CharstringInterp::act_width(int, const Point &)
{
    /* do nothing */
}

void
CharstringInterp::act_default_width(int cmd)
{
    double d = (_program ? _program->global_width_x(false) : UNKDOUBLE);
    if (KNOWN(d))
        act_width(cmd, Point(d, 0));
}

void
CharstringInterp::act_nominal_width_delta(int cmd, double delta)
{
    double d = (_program ? _program->global_width_x(true) : UNKDOUBLE);
    if (KNOWN(d))
        act_width(cmd, Point(d + delta, 0));
}

void
CharstringInterp::act_seac(int cmd, double asb, double adx, double ady, int bchar, int achar)
{
    Charstring *acs = 0, *bcs = 0;
    if (achar < 0 || achar >= 256 || bchar < 0 || bchar >= 256
        || !_program || _program->parent_program()
        || !(acs = get_glyph(Charstring::standard_encoding[achar]))
        || !(bcs = get_glyph(Charstring::standard_encoding[bchar]))) {
        error(errGlyph, cmd);
        return;
    }

    Point apos = Point(adx + _lsb.x - asb, ady + _lsb.y);
    Point save_lsb = _lsb;
    Point save_seac_origin = _seac_origin;

    CharstringInterp::initialize();
    _seac_origin = apos;
    _state = S_SEAC;
    acs->process(*this);
    if (error() == errOK) {
        CharstringInterp::initialize();
        _seac_origin = save_seac_origin;
        _state = S_SEAC;
        bcs->process(*this);
    }

    _lsb = save_lsb;
}

void
CharstringInterp::act_line(int cmd, const Point &p0, const Point &p1)
{
    act_curve(cmd, p0, p0, p1, p1);
}

void
CharstringInterp::act_curve(int cmd, const Point &, const Point &, const Point &, const Point &)
{
    error(errUnimplemented, cmd);
}

void
CharstringInterp::act_flex(int cmd, const Point &p0, const Point &p1, const Point &p2, const Point &p3_4, const Point &p5, const Point &p6, const Point &p7, double flex_depth)
{
    (void) flex_depth;
    act_curve(cmd, p0, p1, p2, p3_4);
    act_curve(cmd, p3_4, p5, p6, p7);
}

void
CharstringInterp::act_closepath(int)
{
    /* do nothing */
}

void
CharstringInterp::act_hstem(int, double, double)
{
    /* do nothing */
}

void
CharstringInterp::act_vstem(int, double, double)
{
    /* do nothing */
}

void
CharstringInterp::act_hstem3(int cmd, double y0, double dy0, double y1, double dy1, double y2, double dy2)
{
    act_hstem(cmd, y0, dy0);
    act_hstem(cmd, y1, dy1);
    act_hstem(cmd, y2, dy2);
}

void
CharstringInterp::act_vstem3(int cmd, double x0, double dx0, double x1, double dx1, double x2, double dx2)
{
    act_vstem(cmd, x0, dx0);
    act_vstem(cmd, x1, dx1);
    act_vstem(cmd, x2, dx2);
}

void
CharstringInterp::act_hintmask(int, const uint8_t *, int)
{
    /* do nothing */
}

}
