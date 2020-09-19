/* maket1font.{cc,hh} -- translate CFF fonts to Type 1 fonts
 *
 * Copyright (c) 2002-2019 Eddie Kohler
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
#include "maket1font.hh"
#include <efont/t1interp.hh>
#include <efont/t1csgen.hh>
#include <lcdf/point.hh>
#include <lcdf/error.hh>
#include <efont/t1font.hh>
#include <efont/t1item.hh>
#include <efont/t1unparser.hh>

using namespace Efont;

typedef unsigned CsRef;
enum { CSR_GLYPH = 0x00000000, CSR_SUBR = 0x80000000,
       CSR_GSUBR = 0xC0000000,
       CSR_TYPE = 0xC0000000, CSR_NUM = 0x3FFFFFFF };

class MakeType1CharstringInterp : public Type1CharstringGenInterp { public:

    MakeType1CharstringInterp(int precision = 5);
    ~MakeType1CharstringInterp();

    Type1Font *output() const			{ return _output; }

    void run(const CharstringProgram *, Type1Font *, PermString glyph_definer, ErrorHandler *);
    void run(const CharstringContext &, Type1Charstring &, ErrorHandler *);

    bool type2_command(int, const uint8_t *, int *);

    String landmark(ErrorHandler *errh) const;

    class Subr;

  private:

    // output
    Type1Font *_output;
    int _flex_message;

    // subroutines
    int _subr_bias;
    int _gsubr_bias;
    mutable Vector<Subr *> _glyphs;
    mutable Vector<Subr *> _subrs;
    mutable Vector<Subr *> _gsubrs;

    Subr *_cur_subr;
    int _cur_glyph;

    Subr *csr_subr(CsRef, bool force) const;
    Type1Charstring *csr_charstring(CsRef) const;

};

class MakeType1CharstringInterp::Subr { public:

    Subr(CsRef csr)			: _csr(csr), _output_subrno(-1), _stamp(0) { }

    bool up_to_date() const		{ return _stamp == max_stamp; }
    void update()			{ _stamp = max_stamp; }
    static void bump_date()		{ max_stamp++; }

    //String name(const MakeType1CharstringInterp *) const;
    Type1Charstring *charstring(const MakeType1CharstringInterp *) const;

    int ncalls() const			{ return _calls.size(); }
    Subr *call(int i) const		{ return _calls[i]; }
    bool has_call(Subr *) const;

    struct Caller {
	Subr *subr;
	int pos;
	int len;
	Caller(Subr *s, int p, int l)
            : subr(s), pos(p), len(l) {
        }
	String charstring(MakeType1CharstringInterp *mcsi) const {
	    Type1Charstring *t1cs = subr->charstring(mcsi);
	    return t1cs->substring(pos, len);
	}
    };

    int ncallers() const		{ return _callers.size(); }
    const Caller &caller(int i) const	{ return _callers[i]; }

    void add_call(Subr *s)		{ _calls.push_back(s); }
    void add_caller(Subr *s, int pos, int len);

    int output_subrno() const		{ return _output_subrno; }
    void set_output_subrno(int n)	{ _output_subrno = n; }

    void transfer_nested_calls(int pos, int length, Subr *new_caller) const;
    void change_callers(Subr *, int pos, int length, int new_length);
    bool unify(MakeType1CharstringInterp *);

  private:

    CsRef _csr;
    Vector<Subr *> _calls;
    Vector<Caller> _callers;

    int _output_subrno;
    int _stamp;

    static int max_stamp;

    friend class MakeType1CharstringInterp;

};

int MakeType1CharstringInterp::Subr::max_stamp = 1;

inline void
MakeType1CharstringInterp::Subr::add_caller(Subr *s, int pos, int len)
{
    _callers.push_back(Caller(s, pos, len));
}

bool
MakeType1CharstringInterp::Subr::has_call(Subr *s) const
{
    for (int i = 0; i < _calls.size(); i++)
	if (_calls[i] == s)
	    return true;
    return false;
}


/*****
 * MakeType1CharstringInterp
 **/

MakeType1CharstringInterp::MakeType1CharstringInterp(int precision)
    : Type1CharstringGenInterp(precision), _flex_message(0)
{
}

MakeType1CharstringInterp::~MakeType1CharstringInterp()
{
    for (int i = 0; i < _glyphs.size(); i++)
	delete _glyphs[i];
    for (int i = 0; i < _subrs.size(); i++)
	delete _subrs[i];
    for (int i = 0; i < _gsubrs.size(); i++)
	delete _gsubrs[i];
}

String
MakeType1CharstringInterp::landmark(ErrorHandler *errh) const
{
    if (_cur_glyph >= 0 && _cur_glyph < program()->nglyphs())
	return errh->format("glyph %<%s%>", program()->glyph_name(_cur_glyph).c_str());
    else
	return String();
}


// subroutines

MakeType1CharstringInterp::Subr *
MakeType1CharstringInterp::csr_subr(CsRef csr, bool force) const
{
    Vector<Subr *> *vp;
    if ((csr & CSR_TYPE) == CSR_SUBR)
	vp = &_subrs;
    else if ((csr & CSR_TYPE) == CSR_GSUBR)
	vp = &_gsubrs;
    else if ((csr & CSR_TYPE) == CSR_GLYPH)
	vp = &_glyphs;
    else
	return 0;

    int n = (csr & CSR_NUM);
    if (n >= vp->size())
	return 0;

    Subr *&what = (*vp)[n];
    if (!what && force)
	what = new Subr(csr);
    return what;
}

#if 0
String
MakeType1CharstringInterp::Subr::name(const MakeType1CharstringInterp *mcsi) const
{
    int n = (_csr & CSR_NUM);
    switch (_csr & CSR_TYPE) {
    case CSR_SUBR:
	return "subr" + String(n);
    case CSR_GSUBR:
	return "gsubr" + String(n);
    case CSR_GLYPH:
	if (String name = mcsi->output()->glyph_name(n))
	    return name;
	else
	    return "??glyph" + String(n) + "??";
    default:
	return "";
    }
}
#endif

Type1Charstring *
MakeType1CharstringInterp::Subr::charstring(const MakeType1CharstringInterp *mcsi) const
{
    int n = (_csr & CSR_NUM);
    switch (_csr & CSR_TYPE) {
      case CSR_SUBR:
      case CSR_GSUBR:
	if (_output_subrno >= 0)
	    return static_cast<Type1Charstring *>(mcsi->output()->subr(_output_subrno));
	return 0;
      case CSR_GLYPH:
	return static_cast<Type1Charstring *>(mcsi->output()->glyph(n));
      default:
	return 0;
    }
}

void
MakeType1CharstringInterp::Subr::transfer_nested_calls(int pos, int length, Subr *new_caller) const
{
    int right = pos + length;
    for (int i = 0; i < _calls.size(); i++) {
	Subr *cs = _calls[i];
	// 11.Jul.2006 - remember not to shift the new caller's records!  (Michael Zedler)
	if (cs != new_caller)
	    for (int j = 0; j < cs->_callers.size(); j++) {
		Caller &c = cs->_callers[j];
		if (c.subr == this && pos <= c.pos && c.pos + c.len <= right) {
		    // shift caller to point at the subroutine
		    c.subr = new_caller;
		    c.pos -= pos;
		    new_caller->add_call(cs);
		}
	    }
    }
}

void
MakeType1CharstringInterp::Subr::change_callers(Subr *caller, int pos, int length, int new_length)
{
    if (up_to_date())
	return;
    update();

    int right = pos + length;
    int delta = new_length - length;
    for (int i = 0; i < _callers.size(); i++) {
	Caller &c = _callers[i];
	if (c.subr != caller)
	    /* nada */;
	else if (pos <= c.pos && c.pos + c.len <= right) {
	    // erase
	    //if (c.debug) fprintf(stderr, "  ERASE caller %08x:%d+%d [%d+%d]\n", c.subr->_csr, c.pos, c.len, pos, length);
	    c.subr = 0;
	} else if (right <= c.pos) {
	    //if (c.debug) fprintf(stderr, "  ADJUST caller %08x:%d+%d -> %d+%d [%d+%d]\n", c.subr->_csr, c.pos, c.len, c.pos+delta, c.len, pos, length);
	    c.pos += delta;
	} else if (c.pos <= pos && right <= c.pos + c.len) {
            //if (c.debug) fprintf(stderr, "  ADJUST caller %08x:%d+%d -> %d+%d [%d+%d]\n", c.subr->_csr, c.pos, c.len, c.pos, c.len+delta, pos, length);
	    c.len += delta;
	} else
	    c.subr = 0;
    }
}

bool
MakeType1CharstringInterp::Subr::unify(MakeType1CharstringInterp *mcsi)
{
    // clean up caller list
    for (int i = 0; i < _callers.size(); i++)
	if (!_callers[i].subr) {
	    _callers[i] = _callers.back();
	    _callers.pop_back();
	    i--;
	}

    if (!_callers.size())
	return false;
    assert(!_calls.size());	// because this hasn't been unified yet

    // Find the smallest shared complete charstring.
    String substr = _callers[0].charstring(mcsi);
    int suboff = 0;
    for (int i = 1; i < _callers.size(); i++) {
	String substr1 = _callers[i].charstring(mcsi);
	const char *d = substr.data() + suboff, *d1 = substr1.data();
	const char *dx = substr.data() + substr.length(), *d1x = d1 + substr1.length();
	while (dx > d && d1x > d1 && dx[-1] == d1x[-1])
	    dx--, d1x--;
        if (d1x != d1) {
            // 8.13.2013 -- We might have stopped in the middle of a number
            // or command in d1 -- even if we absorbed all of d! For
            // example, maybe d's version is "15 rlineto" (encoded "154 5"),
            // and our version is "518 rlineto" (encoded "248 154 5")! So
            // whenever we stop before the end of d1, we must adjust our
            // position to the next caret in d1, which, in the example,
            // would be after "248 154" and before "5".
            int suboff1 = Type1Charstring(substr1).first_caret_after(d1x - d1);
            dx += suboff1 - (d1x - d1);
        }
        suboff = dx - substr.data();
    }
    substr = substr.substring(Type1Charstring(substr).first_caret_after(suboff));
    if (!substr.length())
	return false;
    for (int i = 0; i < _callers.size(); i++) {
	Caller &c = _callers[i];
	if (int delta = c.len - substr.length()) {
            //if (c.debug) fprintf(stderr, "  PREFIX caller %08x:%d+%d -> %d+%d [%s]\n", c.subr->_csr, c.pos, c.len, c.pos+delta, c.len+delta, CharstringUnparser::unparse(Type1Charstring(substr)).c_str());
	    c.pos += delta;
	    c.len -= delta;
	}
    }

    // otherwise, success
    _output_subrno = mcsi->output()->nsubrs();
    mcsi->output()->set_subr(_output_subrno, Type1Charstring(substr + "\013"));

    // This subr has become real, so it is suitable for later unifications.
    // Mark it as having called any subroutines contained completely within itself.
    // How to do this?  Look at one caller, and go over all its calls.
    _callers[0].subr->transfer_nested_calls(_callers[0].pos, _callers[0].len, this);

    // adapt callers
    String callsubr_string = Type1CharstringGen::callsubr_string(_output_subrno);

    for (int i = 0; i < _callers.size(); i++)
	// 13.Jun.2003 - must check whether _callers[i].subr exists: if we
	// called a subroutine more than once, change_callers() might have
	// zeroed it out.
	if (_callers[i].subr && _callers[i].subr != this) {
	    Subr::Caller c = _callers[i];
	    c.subr->charstring(mcsi)->assign_substring(c.pos, c.len, callsubr_string);
	    Subr::bump_date();
	    for (int j = 0; j < c.subr->ncalls(); j++)
		c.subr->call(j)->change_callers(c.subr, c.pos, c.len, callsubr_string.length());
	    assert(!_callers[i].subr);
	}

    // this subr is no longer "called"/interpolated from anywhere
    _callers.clear();

    //fprintf(stderr, "Succeeded %x\n", _csr);
    return true;
}


// running

bool
MakeType1CharstringInterp::type2_command(int cmd, const uint8_t *data, int *left)
{
    switch (cmd) {

      case Cs::cCallsubr:
      case Cs::cCallgsubr:
	if (subr_depth() < MAX_SUBR_DEPTH && size() == 1) {
	    //fprintf(stderr, "succeeded %d\n", (int) top());
	    bool g = (cmd == Cs::cCallgsubr);
	    CsRef csref = ((int)top() + program()->xsubr_bias(g)) | (g ? CSR_GSUBR : CSR_SUBR);
	    Subr *callee = csr_subr(csref, true);
	    if (callee)
		_cur_subr->add_call(callee);

	    int left = csgen().length();

	    bool more = callxsubr_command(g);

	    int right = csgen().length();
	    if (error() >= 0 && callee)
		callee->add_caller(_cur_subr, left, right - left);
	    return more;
	} else {
	    //fprintf(stderr, "failed %d\n", (int) top());
	    goto normal;
	}

      normal:
      default:
	return CharstringInterp::type2_command(cmd, data, left);

    }
}

void
MakeType1CharstringInterp::run(const CharstringContext &g, Type1Charstring &out, ErrorHandler *errh)
{
    Type1CharstringGenInterp::run(g, out);

    if (Type1CharstringGenInterp::had_bad_flex() && !(_flex_message & 1)) {
	errh->lwarning(landmark(errh), "complex flex hint replaced with curves");
	errh->message("(This font contains flex hints prohibited by Type 1. They%,ve been\nreplaced by ordinary curves.)");
	_flex_message |= 1;
    }
#if !HAVE_ADOBE_CODE
    if (Type1CharstringGenInterp::had_flex() && !(_flex_message & 2)) {
        errh->lwarning(landmark(errh), "flex hints required");
        errh->message("(This program was compiled without Adobe code for flex hint support,\nso its output may not work on all devices.)");
        _flex_message |= 2;
    }
    if (Type1CharstringGenInterp::had_hr() && !(_flex_message & 4)) {
        errh->lwarning(landmark(errh), "hint replacement required");
        errh->message("(This program was compiled without Adobe code for hint replacement,\nso its output may not work on all devices.)");
        _flex_message |= 4;
    }
#endif
}

void
MakeType1CharstringInterp::run(const CharstringProgram *program, Type1Font *output, PermString glyph_definer, ErrorHandler *errh)
{
    _output = output;
    set_hint_replacement_storage(output);

    _glyphs.assign(program->nglyphs(), 0);
    _subrs.assign(program->nsubrs(), 0);
    _subr_bias = program->subr_bias();
    _gsubrs.assign(program->ngsubrs(), 0);
    _gsubr_bias = program->gsubr_bias();

    // run over the glyphs
    int nglyphs = program->nglyphs();
    Type1Charstring receptacle;
    for (int i = 0; i < nglyphs; i++) {
	_cur_subr = _glyphs[i] = new Subr(CSR_GLYPH | i);
	_cur_glyph = i;
	run(program->glyph_context(i), receptacle, errh);
#if 0
	PermString n = program->glyph_name(i);
	if (i == 408 || i == 20) {
	    fprintf(stderr, "%d: %s was %s\n", i, n.c_str(), CharstringUnparser::unparse(*program->glyph(i)).c_str());
	    fprintf(stderr, "  now %s\n", CharstringUnparser::unparse(receptacle).c_str());
            fprintf(stderr, "  *** %d.%d: %s\n", 134, 30, CharstringUnparser::unparse(Type1Charstring(receptacle.data_string().substring(134, 30))).c_str());
	}
#endif
	PermString name = program->glyph_name(i);
	if (output->glyph(name)) {
	    errh->warning("glyph %<%s%> defined more than once", name.c_str());
	    int i = 1;
	    do {
		name = program->glyph_name(i) + String(".") + String(i);
		++i;
	    } while (output->glyph(name));
	}
	output->add_glyph(Type1Subr::make_glyph(name, receptacle, glyph_definer));
    }

    // unify Subrs
    for (int i = 0; i < _subrs.size(); i++)
	if (_subrs[i])
	    _subrs[i]->unify(this);

    for (int i = 0; i < _gsubrs.size(); i++)
	if (_gsubrs[i])
	    _gsubrs[i]->unify(this);
}


/*****
 * main
 **/

static void
add_number_def(Type1Font *output, int dict, PermString name, const Cff::Font *font, Cff::DictOperator op)
{
    double v;
    if (font->dict_value(op, &v))
	output->add_definition(dict, Type1Definition::make(name, v, "def"));
}

static void
add_delta_def(Type1Font *output, int dict, PermString name, const Cff::Font *font, Cff::DictOperator op)
{
    Vector<double> vec;
    if (font->dict_value(op, vec)) {
	for (int i = 1; i < vec.size(); i++)
	    vec[i] += vec[i - 1];
	StringAccum sa;
	for (int i = 0; i < vec.size(); i++)
	    sa << (i ? ' ' : '[') << vec[i];
	sa << ']';
	output->add_definition(dict, Type1Definition::make_literal(name, sa.take_string(), (dict == Type1Font::dP ? "|-" : "readonly def")));
    }
}

Type1Font *
create_type1_font(Cff::Font *font, ErrorHandler *errh)
{
    String version = font->dict_string(Cff::oVersion);
    Type1Font *output = Type1Font::skeleton_make(font->font_name(), version);
    output->skeleton_comments_end();
    StringAccum sa;

    // FontInfo dictionary
    if (version)
	output->add_definition(Type1Font::dFI, Type1Definition::make_string("version", version, "readonly def"));
    if (String s = font->dict_string(Cff::oNotice))
	output->add_definition(Type1Font::dFI, Type1Definition::make_string("Notice", s, "readonly def"));
    if (String s = font->dict_string(Cff::oCopyright))
	output->add_definition(Type1Font::dFI, Type1Definition::make_string("Copyright", s, "readonly def"));
    if (String s = font->dict_string(Cff::oFullName))
	output->add_definition(Type1Font::dFI, Type1Definition::make_string("FullName", s, "readonly def"));
    if (String s = font->dict_string(Cff::oFamilyName))
	output->add_definition(Type1Font::dFI, Type1Definition::make_string("FamilyName", s, "readonly def"));
    if (String s = font->dict_string(Cff::oWeight))
	output->add_definition(Type1Font::dFI, Type1Definition::make_string("Weight", s, "readonly def"));
    double v;
    if (font->dict_value(Cff::oIsFixedPitch, &v))
	output->add_definition(Type1Font::dFI, Type1Definition::make_literal("isFixedPitch", (v ? "true" : "false"), "def"));
    add_number_def(output, Type1Font::dFI, "ItalicAngle", font, Cff::oItalicAngle);
    add_number_def(output, Type1Font::dFI, "UnderlinePosition", font, Cff::oUnderlinePosition);
    add_number_def(output, Type1Font::dFI, "UnderlineThickness", font, Cff::oUnderlineThickness);
    output->skeleton_fontinfo_end();

    // Encoding, other font dictionary entries
    output->add_item(font->type1_encoding_copy());
    font->dict_value(Cff::oPaintType, &v);
    output->add_definition(Type1Font::dF, Type1Definition::make("PaintType", v, "def"));
    output->add_definition(Type1Font::dF, Type1Definition::make("FontType", 1.0, "def"));
    Vector<double> vec;
    if (font->dict_value(Cff::oFontMatrix, vec) && vec.size() == 6) {
	sa << '[' << vec[0] << ' ' << vec[1] << ' ' << vec[2] << ' ' << vec[3] << ' ' << vec[4] << ' ' << vec[5] << ']';
	output->add_definition(Type1Font::dF, Type1Definition::make_literal("FontMatrix", sa.take_string(), "readonly def"));
    } else
	output->add_definition(Type1Font::dF, Type1Definition::make_literal("FontMatrix", "[0.001 0 0 0.001 0 0]", "readonly def"));
    add_number_def(output, Type1Font::dF, "StrokeWidth", font, Cff::oStrokeWidth);
    add_number_def(output, Type1Font::dF, "UniqueID", font, Cff::oUniqueID);
    if (font->dict_value(Cff::oXUID, vec) && vec.size()) {
	for (int i = 0; i < vec.size(); i++)
	    sa << (i ? ' ' : '[') << vec[i];
	sa << ']';
	output->add_definition(Type1Font::dF, Type1Definition::make_literal("XUID", sa.take_string(), "readonly def"));
    }
    if (font->dict_value(Cff::oFontBBox, vec) && vec.size() == 4) {
	sa << '{' << vec[0] << ' ' << vec[1] << ' ' << vec[2] << ' ' << vec[3] << '}';
	output->add_definition(Type1Font::dF, Type1Definition::make_literal("FontBBox", sa.take_string(), "readonly def"));
    } else
	output->add_definition(Type1Font::dF, Type1Definition::make_literal("FontBBox", "{0 0 0 0}", "readonly def"));
    output->skeleton_fontdict_end();

    // Private dictionary
    add_delta_def(output, Type1Font::dP, "BlueValues", font, Cff::oBlueValues);
    add_delta_def(output, Type1Font::dP, "OtherBlues", font, Cff::oOtherBlues);
    add_delta_def(output, Type1Font::dP, "FamilyBlues", font, Cff::oFamilyBlues);
    add_delta_def(output, Type1Font::dP, "FamilyOtherBlues", font, Cff::oFamilyOtherBlues);
    add_number_def(output, Type1Font::dP, "BlueScale", font, Cff::oBlueScale);
    add_number_def(output, Type1Font::dP, "BlueShift", font, Cff::oBlueShift);
    add_number_def(output, Type1Font::dP, "BlueFuzz", font, Cff::oBlueFuzz);
    if (font->dict_value(Cff::oStdHW, &v))
	output->add_definition(Type1Font::dP, Type1Definition::make_literal("StdHW", String("[") + String(v) + "]", "|-"));
    if (font->dict_value(Cff::oStdVW, &v))
	output->add_definition(Type1Font::dP, Type1Definition::make_literal("StdVW", String("[") + String(v) + "]", "|-"));
    add_delta_def(output, Type1Font::dP, "StemSnapH", font, Cff::oStemSnapH);
    add_delta_def(output, Type1Font::dP, "StemSnapV", font, Cff::oStemSnapV);
    if (font->dict_value(Cff::oForceBold, &v))
	output->add_definition(Type1Font::dP, Type1Definition::make_literal("ForceBold", (v ? "true" : "false"), "def"));
    add_number_def(output, Type1Font::dP, "LanguageGroup", font, Cff::oLanguageGroup);
    add_number_def(output, Type1Font::dP, "ExpansionFactor", font, Cff::oExpansionFactor);
    add_number_def(output, Type1Font::dP, "UniqueID", font, Cff::oUniqueID);
    output->add_definition(Type1Font::dP, Type1Definition::make_literal("MinFeature", "{16 16}", "|-"));
    output->add_definition(Type1Font::dP, Type1Definition::make_literal("password", "5839", "def"));
    output->add_definition(Type1Font::dP, Type1Definition::make_literal("lenIV", "0", "def"));
    output->skeleton_private_end();
    output->skeleton_common_subrs();

    // add glyphs
    MakeType1CharstringInterp maker(5);
    maker.run(font, output, " |-", errh);

    return output;
}

#include <lcdf/vector.cc>
