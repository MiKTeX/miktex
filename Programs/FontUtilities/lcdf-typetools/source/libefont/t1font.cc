// -*- related-file-name: "../include/efont/t1font.hh" -*-

/* t1font.{cc,hh} -- Type 1 font
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
#include <efont/t1font.hh>
#include <efont/t1item.hh>
#include <efont/t1rw.hh>
#include <efont/t1mm.hh>
#include <lcdf/error.hh>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
namespace Efont {

static PermString::Initializer initializer;
static PermString lenIV_str = "lenIV";
static PermString FontInfo_str = "FontInfo";

Type1Font::Type1Font(PermString name)
    : CharstringProgram(1000),
      _cached_defs(false), _built(true), _font_name(name), _glyph_map(-1),
      _encoding(0), _cached_mmspace(0), _mmspace(0), _synthetic_item(0)
{
    _dict = new HashMap<PermString, Type1Definition *>[dLast];
    for (int i = 0; i < dLast; i++) {
        _index[i] = -1;
        _dict_deltas[i] = 0;
        _dict[i].set_default_value((Type1Definition *)0);
    }
}

Type1Font::Type1Font(Type1Reader &reader)
    : CharstringProgram(1000),
      _cached_defs(false), _built(false), _glyph_map(-1), _encoding(0),
      _cached_mmspace(0), _mmspace(0), _synthetic_item(0)
{
    _dict = new HashMap<PermString, Type1Definition *>[dLast];
    for (int i = 0; i < dLast; i++) {
        _index[i] = -1;
        _dict_deltas[i] = 0;
        _dict[i].set_default_value((Type1Definition *)0);
    }
    read(reader);
}

Type1Font::~Type1Font()
{
    delete[] _dict;
    for (int i = 0; i < _items.size(); i++)
        delete _items[i];
    delete _mmspace;
    for (int i = 0; i < _subrs.size(); i++)
        delete _subrs[i];
    if (!_synthetic_item)
        for (int i = 0; i < _glyphs.size(); i++)
            delete _glyphs[i];
}

void
Type1Font::set_item(int i, Type1Item *it)
{
    delete _items[i];
    _items[i] = it;
}

static const char * const dict_starters[] = {
    "0 dict begin", "/FontInfo 0 dict dup begin", "dup /Private 0 dict dup begin", "/Blend 0 dict dup begin", 0, 0
};

void
Type1Font::add_definition(int dict, Type1Definition *t1d)
{
    if (_index[dict] < 0) {
        if (_built && dict_starters[dict])
            add_item(new Type1CopyItem(dict_starters[dict]));
        _index[dict] = _items.size();
    }
    add_item(t1d);
    set_dict(dict, t1d->name(), t1d);
}

void
Type1Font::add_type1_encoding(Type1Encoding *e)
{
    if (_encoding) {
        for (Type1Item** t1i = _items.begin(); t1i < _items.end(); t1i++)
            if (*t1i == _encoding) {
                delete _encoding;
                *t1i = _encoding = e;
                return;
            }
    }
    _encoding = e;
    add_item(e);
}

void
Type1Font::add_glyph(Type1Subr *s)
{
    int &g = _glyph_map.find_force(s->name(), _glyphs.size());
    if (g == _glyphs.size())
        _glyphs.push_back(s);
    else {
        delete _glyphs[g];
        _glyphs[g] = s;
    }
}

int
Type1Font::read(Type1Reader &reader)
{
    Dict cur_dict = dFont;
    int eexec_state = 0;
    bool have_subrs = false;
    bool have_charstrings = false;
    int lenIV = 4;
    Type1SubrGroupItem *cur_group = 0;
    int cur_group_count = 0;

    StringAccum accum;
    while (reader.next_line(accum)) {

        // check for NULL STRING
        int x_length = accum.length();
        if (!x_length)
            continue;
        const char *x = accum.c_str(); // ensure we don't run off the string

        // check for CHARSTRINGS
        if (reader.was_charstring()) {
            Type1Subr *fcs = Type1Subr::make(x, x_length, reader.charstring_start(), reader.charstring_length(), lenIV);

            if (fcs->is_subr()) {
                if (fcs->subrno() >= _subrs.size())
                    _subrs.resize(fcs->subrno() + 30, (Type1Subr *)0);
                if (_subrs[fcs->subrno()]) // hybrid font program
                    delete _subrs[fcs->subrno()];
                _subrs[fcs->subrno()] = fcs;
                if (!have_subrs && _items.size()) {
                    if (Type1CopyItem *item = _items.back()->cast_copy()) {
                        cur_group = new Type1SubrGroupItem
                            (this, true, item->value());
                        cur_group_count = 0;
                        _items.back() = cur_group;
                        delete item;
                    }
                    have_subrs = true;
                }

            } else {
                add_glyph(fcs);
                if (!have_charstrings && _items.size()) {
                    if (Type1CopyItem *item = _items.back()->cast_copy()) {
                        cur_group = new Type1SubrGroupItem
                            (this, false, item->value());
                        cur_group_count = 0;
                        _items.back() = cur_group;
                        delete item;
                    }
                    have_charstrings = true;
                }
            }

            accum.clear();
            continue;
        }

        // check for COMMENTS
        if (x[0] == '%') {
            add_item(new Type1CopyItem(accum.take_string()));
            continue;
        }

        // check for CHARSTRING START
        // 5/29/1999: beware of charstring start-like things that don't have
        // `readstring' in them!
        if (!_charstring_definer
            && strstr(x, "string currentfile") != 0
            && strstr(x, "readstring") != 0) {
            const char *sb = x;
            while (*sb && *sb != '/')
                sb++;
            const char *se = sb + 1;
            while (*sb && *se && *se != ' ' && *se != '{')
                se++;
            if (*sb && *se) {
                _charstring_definer = permprintf(" %*s ", se - sb - 1, sb + 1);
                reader.set_charstring_definer(_charstring_definer);
                add_item(new Type1CopyItem(accum.take_string()));
                continue;
            }
        }

        // check for ENCODING
        if (!_encoding && strncmp(x, "/Encoding ", 10) == 0) {
            read_encoding(reader, x + 10);
            accum.clear();
            continue;
        }

        // check for a DEFINITION
        if (x[0] == '/') {
          definition_succeed:
            Type1Definition *t1d = Type1Definition::make(accum, &reader);
            if (!t1d)
                goto definition_fail;
            if (t1d->name() == lenIV_str)
                t1d->value_int(lenIV);
            add_definition(cur_dict, t1d);
            accum.clear();
            continue;
        } else if (x[0] == ' ') {
            const char *y;
            for (y = x; y[0] == ' '; y++)
                ;
            if (y[0] == '/')
                goto definition_succeed;
        }

      definition_fail:

        // check for ZEROS special case
        if (eexec_state == 2) {
            // In eexec_state 2 (right after turning off eexec), the opening
            // part of the string will have some 0 bytes followed by '0's.
            // Change the 0 bytes into textual '0's.
            int zeros = 0;
            while (x[zeros] == 0 && x_length > 0)
                zeros++, x_length--;
            add_item(new Type1CopyItem(String::make_fill('0', zeros * 2 + x_length)));
            eexec_state = 3;
            accum.clear();
            continue;
        }

        // check for MODIFIED FONT
        if (eexec_state == 1 && strstr(x, "FontDirectory") != 0
            && read_synthetic_font(reader, x, accum)) {
            accum.clear();
            continue;
        }

        // check for END-OF-CHARSTRING-GROUP TEXT
        if (cur_group) {
            if (cur_group_count == 0
                || ((strstr(x, "end") != 0 || strstr(x, "put") != 0)
                    && strchr(x, '/') == 0)) {
                cur_group->add_end_text(x);
                cur_group_count++;
                accum.clear();
                continue;
            }
            cur_group = 0;
        }

        // add COPY ITEM
        String s = accum.take_string();
        add_item(new Type1CopyItem(s));
        x = s.data();

        if (eexec_state == 0 && strncmp(x, "currentfile eexec", 17) == 0 && (isspace((unsigned char) x[17]) || !x[17])) {
            // allow arbitrary whitespace after "currentfile eexec".
            // note: strlen("currentfile eexec") == 17
            for (x += 17; isspace((unsigned char) *x); x++)
                /* nada */;
            reader.switch_eexec(true, (unsigned char *)x, (s.data() + s.length()) - x);
            set_item(nitems() - 1, new Type1EexecItem(true));
            eexec_state = 1;
        } else if (eexec_state == 1 && strstr(x, "currentfile closefile") != 0) {
            reader.switch_eexec(false, 0, 0);
            add_item(new Type1EexecItem(false));
            eexec_state = 2;
        } else if (strstr(x, "begin") != 0) {
            // 30.Sep.2002: NuevaMM's BlendFontInfo dict starts with a simple
            // "/FontInfo ... begin" inside a "/Blend ... begin".
            Dict was_dict = cur_dict;
            if (strstr(x, "/Private") != 0)
                cur_dict = dPrivate;
            else if (strstr(x, "/FontInfo") != 0)
                cur_dict = dFontInfo;
            else
                cur_dict = dFont;
            if (strstr(x, "/Blend") != 0)
                cur_dict = (Dict)(cur_dict + dBlend);
            else if (was_dict == dBlend && cur_dict == dFontInfo)
                cur_dict = (Dict)(cur_dict + dBlend);
        } else if (cur_dict == dFontInfo && strstr(x, "end") != 0)
            cur_dict = dFont;
    }

    // set dictionary deltas
    for (int i = dFI; i < dLast; i++)
        _dict_deltas[i] = get_dict_size(i) - _dict[i].size();
    // borrow glyphs and glyph map from _synthetic_item
    if (!_glyphs.size() && _synthetic_item) {
        _glyphs = _synthetic_item->included_font()->_glyphs;
        _glyph_map = _synthetic_item->included_font()->_glyph_map;
    }

    return (ok() ? 0 : -1);
}

bool
Type1Font::ok() const
{
    return font_name() && _glyphs.size() > 0;
}

static char *skip_comment_space(char *s)
{
    while (1) {
        if (isspace((unsigned char) *s))
            ++s;
        else if (*s == '%') {
            for (++s; *s != '\r' && *s != '\n' && *s != '\0'; ++s)
                /* nada */;
        } else
            return s;
    }
}

void
Type1Font::read_encoding(Type1Reader &reader, const char *first_line)
{
    while (isspace((unsigned char) *first_line))
        first_line++;
    if (strncmp(first_line, "StandardEncoding", 16) == 0) {
        add_type1_encoding(Type1Encoding::standard_encoding());
        return;
    }

    add_type1_encoding(new Type1Encoding);

    bool got_any = false;
    StringAccum accum;

    while (reader.next_line(accum)) {

        // check for NULL STRING
        if (!accum.length())
            continue;
        accum.append('\0');             // ensure we don't run off the string
        char *pos = accum.data();

        // skip to first `dup' token
        if (!got_any) {
            if (!(pos = strstr(pos, "dup"))) {
                pos = accum.data();
                goto check_done;
            }
        }

        // parse as many `dup INDEX */CHARNAME put' as there are in the line
        while (1) {
            // skip spaces, look for `dup '
            while (isspace((unsigned char) pos[0]))
                ++pos;
            if (pos[0] == '%')
                pos = skip_comment_space(pos);
            if (pos[0] != 'd' || pos[1] != 'u' || pos[2] != 'p' || !isspace((unsigned char) pos[3]))
                break;

            // look for `INDEX */'
            char *scan;
            int char_value = strtol(pos + 4, &scan, 10);
            if (scan[0] == '#' && char_value > 0 && char_value < 37
                && isalnum((unsigned char) scan[1]))
                char_value = strtol(scan + 1, &scan, char_value);
            while (isspace((unsigned char) scan[0]))
                scan++;
            if (char_value < 0 || char_value >= 256 || scan[0] != '/')
                break;

            // look for `CHARNAME put'
            scan++;
            char *name_pos = scan;
            while (!isspace((unsigned char) scan[0]) && scan[0] != '\0')
                ++scan;
            char *name_end = scan;
            while (isspace((unsigned char) scan[0]))
                ++scan;
            if (scan[0] != 'p' || scan[1] != 'u' || scan[2] != 't')
                break;

            _encoding->put(char_value, PermString(name_pos, name_end - name_pos));
            got_any = true;
            pos = scan + 3;
        }

      check_done:
        // check for end of encoding section
        // if not over, add COPY ITEM for leftovers we didn't parse
        if ((strstr(pos, "readonly") != 0 || strstr(pos, "def") != 0)
            && (got_any || strstr(pos, "for") == 0)) {
            _encoding->set_definer(String(pos));
            return;
        } else if (got_any && *pos)
            add_item(new Type1CopyItem(String(pos)));

        accum.clear();
    }
}

static bool
read_synthetic_string(Type1Reader &reader, StringAccum &wrong_accum,
                      const char *format, int *value)
{
    StringAccum accum;
    if (!reader.next_line(accum))
        return false;
    wrong_accum << accum;
    accum.append('\0');         // ensure we don't run off the string
    int n = 0;
    if (value)
        sscanf(accum.data(), format, value, &n);
    else
        sscanf(accum.data(), format, &n);
    return (n != 0 && (isspace((unsigned char) accum[n]) || accum[n] == '\0'));
}

bool
Type1Font::read_synthetic_font(Type1Reader &reader, const char *first_line,
                               StringAccum &wrong_accum)
{
    // read font name
    PermString font_name;
    {
        char *x = new char[strlen(first_line) + 1];
        int n = 0;
        sscanf(first_line, "FontDirectory /%[^] \t\r\n[{}/] known {%n", x, &n);
        if (n && (isspace((unsigned char) first_line[n]) || first_line[n] == 0))
            font_name = x;
        delete[] x;
        if (!font_name)
            return false;
    }

    // check UniqueID
    int unique_id;
    {
        StringAccum accum;
        if (!reader.next_line(accum))
            return false;
        wrong_accum << accum;
        accum.c_str();          // ensure we don't run off the string
        const char *y = accum.data();
        if (*y != '/' || strncmp(y + 1, font_name.c_str(), font_name.length()) != 0)
            return false;
        int n = 0;
        sscanf(y + font_name.length() + 1, " findfont%n", &n);
        y = strstr(y, "/UniqueID get ");
        if (n == 0 || y == 0)
            return false;
        n = 0;
        sscanf(y + 14, "%d%n", &unique_id, &n);
        if (n == 0)
            return false;
    }

    // check lines that say how much text
    int multiplier;
    if (!read_synthetic_string(reader, wrong_accum, "save userdict /fbufstr %d string put%n", &multiplier))
        return false;

    int multiplicand;
    if (!read_synthetic_string(reader, wrong_accum, "%d {currentfile fbufstr readstring { pop } { clear currentfile%n", &multiplicand))
        return false;

    if (!read_synthetic_string(reader, wrong_accum, "closefile /fontdownload /unexpectedEOF /.error cvx exec } ifelse } repeat%n", 0))
        return false;

    int extra;
    if (!read_synthetic_string(reader, wrong_accum, "currentfile %d string readstring { pop } { clear currentfile%n", &extra))
        return false;

    if (!read_synthetic_string(reader, wrong_accum, "closefile /fontdownload /unexpectedEOF /.error cvx exec } ifelse%n", 0))
        return false;

    if (!read_synthetic_string(reader, wrong_accum, "restore } if } if%n", 0))
        return false;

    Type1SubsetReader subreader(&reader, multiplier*multiplicand + extra);
    Type1Font *synthetic = new Type1Font(subreader);
    if (!synthetic->ok())
        delete synthetic;
    else {
        _synthetic_item = new Type1IncludedFont(synthetic, unique_id);
        add_item(_synthetic_item);
    }
    return true;
}


void
Type1Font::undo_synthetic()
{
    // A synthetic font doesn't share arbitrary code with its base font; it
    // shares just the CharStrings dictionary, according to Adobe Type 1 Font
    // Format. We take advantage of this.

    if (!_synthetic_item)
        return;

    int mod_ii;
    for (mod_ii = nitems() - 1; mod_ii >= 0; mod_ii--)
        if (_items[mod_ii] == _synthetic_item)
            break;
    if (mod_ii < 0)
        return;

    // remove synthetic item and the reference to the included font
    _items[mod_ii] = new Type1NullItem;
    if (Type1CopyItem *copy = _items[mod_ii+1]->cast_copy())
        if (copy->value().find_left("findfont") >= 0) {
            delete copy;
            _items[mod_ii+1] = new Type1NullItem;
        }

    Type1Font *f = _synthetic_item->included_font();
    // its glyphs are already stored in our _glyphs array

    // copy SubrGroupItem from `f' into `this'
    Type1SubrGroupItem *oth_subrs = 0, *oth_glyphs = 0;
    for (int i = 0; i < f->nitems(); i++)
        if (Type1SubrGroupItem *subr_grp = f->_items[i]->cast_subr_group()) {
            if (subr_grp->is_subrs())
                oth_subrs = subr_grp;
            else
                oth_glyphs = subr_grp;
        }

    assert(oth_glyphs != 0);

    for (int i = nitems() - 1; i >= 0; i--)
        if (Type1SubrGroupItem *subr_grp = _items[i]->cast_subr_group()) {
            assert(subr_grp->is_subrs());
            if (oth_subrs)
                subr_grp->set_end_text(oth_subrs->end_text());
            shift_indices(i + 1, 1);
            Type1SubrGroupItem *nsubr = new Type1SubrGroupItem(*oth_glyphs, this);
            _items[i + 1] = nsubr;
            break;
        }

    // delete included font
    f->_glyphs.clear();         // don't delete glyphs; we've stolen them
    delete _synthetic_item;
    _synthetic_item = 0;
}


Type1Charstring *
Type1Font::subr(int e) const
{
    if (e >= 0 && e < _subrs.size() && _subrs[e])
        return &_subrs[e]->t1cs();
    else
        return 0;
}

PermString
Type1Font::glyph_name(int i) const
{
    if (i >= 0 && i < _glyphs.size() && _glyphs[i])
        return _glyphs[i]->name();
    else
        return PermString();
}

Type1Charstring *
Type1Font::glyph(int i) const
{
    if (i >= 0 && i < _glyphs.size() && _glyphs[i])
        return &_glyphs[i]->t1cs();
    else
        return 0;
}

Type1Charstring *
Type1Font::glyph(PermString name) const
{
    int i = _glyph_map[name];
    if (i >= 0)
        return &_glyphs[i]->t1cs();
    else
        return 0;
}


bool
Type1Font::set_subr(int e, const Type1Charstring &t1cs, PermString definer)
{
    if (e < 0)
        return false;
    if (e >= _subrs.size())
        _subrs.resize(e + 1, (Type1Subr *)0);

    if (!definer) {
        Type1Subr *pattern_subr = _subrs[e];
        for (int i = 0; i < _subrs.size() && !pattern_subr; i++)
            pattern_subr = _subrs[i];
        if (!pattern_subr)
            return false;
        definer = pattern_subr->definer();
    }

    delete _subrs[e];
    _subrs[e] = Type1Subr::make_subr(e, t1cs, definer);
    return true;
}

bool
Type1Font::remove_subr(int e)
{
    if (e < 0 || e >= _subrs.size())
        return false;
    delete _subrs[e];
    _subrs[e] = 0;
    return true;
}

void
Type1Font::fill_in_subrs()
{
    while (_subrs.size() && _subrs.back() == 0)
        _subrs.pop_back();
    for (int i = 0; i < _subrs.size(); i++)
        if (!_subrs[i])
            set_subr(i, Type1Charstring(String::make_stable("\013", 1)));
}

void
Type1Font::renumber_subrs(const Vector<int> &renumbering)
{
    Vector<Type1Subr *> old_subrs;
    old_subrs.swap(_subrs);
    for (int i = 0; i < old_subrs.size() && i < renumbering.size(); i++) {
        int r = renumbering[i];
        Type1Subr *s = old_subrs[i];
        if (r >= 0 && s)
            set_subr(r, s->t1cs(), s->definer());
        else
            delete s;
    }
    for (int i = renumbering.size(); i < old_subrs.size(); i++)
        delete old_subrs[i];
}


void
Type1Font::shift_indices(int move_index, int delta)
{
    if (delta > 0) {
        _items.resize(_items.size() + delta, (Type1Item *)0);
        memmove(&_items[move_index + delta], &_items[move_index],
                sizeof(Type1Item *) * (_items.size() - move_index - delta));

        for (int i = dFont; i < dLast; i++)
            if (_index[i] > move_index)
                _index[i] += delta;

    } else {
        memmove(&_items[move_index], &_items[move_index - delta],
                sizeof(Type1Item *) * (_items.size() - (move_index - delta)));
        _items.resize(_items.size() + delta);

        for (int i = dFont; i < dLast; i++)
            if (_index[i] >= move_index) {
                if (_index[i] < move_index - delta)
                    _index[i] = move_index;
                else
                    _index[i] += delta;
            }
    }
}

Type1Definition *
Type1Font::ensure(Dict dict, PermString name)
{
    assert(_index[dict] >= 0);
    Type1Definition *def = _dict[dict][name];
    if (!def) {
        def = new Type1Definition(name, 0, "def");
        int move_index = _index[dict];
        shift_indices(move_index, 1);
        _items[move_index] = def;
        set_dict(dict, name, def);
    }
    return def;
}

void
Type1Font::add_header_comment(const String &comment)
{
    int i;
    for (i = 0; i < _items.size(); i++) {
        Type1CopyItem *copy = _items[i]->cast_copy();
        if (!copy || copy->value()[0] != '%')
            break;
    }
    shift_indices(i, 1);
    _items[i] = new Type1CopyItem(comment);
}


Type1Item *
Type1Font::dict_size_item(int d) const
{
    switch (d) {

      case dF:
        if (_built && _index[d] > 0)
            return _items[_index[d] - 1];
        break;

      case dFI: case dP: case dB:
        if (_index[d] > 0)
            return _items[_index[d] - 1];
        break;

      case dBFI:
        if (Type1Item *t1i = b_dict("FontInfo"))
            return t1i;
        else if (_index[dBFI] > 0)
            return _items[_index[dBFI] - 1];
        break;

      case dBP:
        if (Type1Item *t1i = b_dict("Private"))
            return t1i;
        else if (_index[dBP] > 0)
            return _items[_index[dBP] - 1];
        break;

    }
    return 0;
}

int
Type1Font::get_dict_size(int d) const
{
    Type1Item *item = dict_size_item(d);
    if (!item)
        /* nada */;
    else if (Type1Definition *t1d = item->cast_definition()) {
        int num;
        if (strstr(t1d->definer().c_str(), "dict") && t1d->value_int(num))
            return num;
    } else if (Type1CopyItem *copy = item->cast_copy()) {
        String value = copy->value();
        int pos = value.find_left(" dict");
        if (pos >= 1 && isdigit((unsigned char) value[pos - 1])) {
            while (pos >= 1 && isdigit((unsigned char) value[pos - 1]))
                pos--;
            return strtol(value.data() + pos, 0, 10);
        }
    }
    return -1;
}

void
Type1Font::set_dict_size(int d, int size)
{
    Type1Item *item = dict_size_item(d);
    if (!item)
        return;
    if (Type1Definition *t1d = item->cast_definition()) {
        int num;
        if (strstr(t1d->definer().c_str(), "dict") && t1d->value_int(num))
            t1d->set_int(size);
    } else if (Type1CopyItem *copy = item->cast_copy()) {
        String value = copy->value();
        int pos = value.find_left(" dict");
        if (pos >= 1 && isdigit((unsigned char) value[pos - 1])) {
            int numpos = pos - 1;
            while (numpos >= 1 && isdigit((unsigned char) value[numpos - 1]))
                numpos--;
            StringAccum accum;
            accum << value.substring(0, numpos) << size << value.substring(pos);
            copy->set_value(accum.take_string());
        }
    }
}

void
Type1Font::write(Type1Writer &w)
{
    Type1Definition *lenIV_def = p_dict("lenIV");
    int lenIV = 4;
    if (lenIV_def)
        lenIV_def->value_int(lenIV);
    w.set_charstring_start(_charstring_definer);
    w.set_lenIV(lenIV);

    // change dict sizes
    for (int i = dF; i < dLast; i++)
        set_dict_size(i, _dict[i].size() + _dict_deltas[i]);
    // XXX what if dict had nothing, but now has something?

    for (int i = 0; i < _items.size(); i++)
        _items[i]->gen(w);

    w.flush();
}

void
Type1Font::cache_defs() const
{
    Type1Definition *t1d = dict("FontName");
    if (t1d)
        t1d->value_name(_font_name);
    _cached_defs = true;
}

MultipleMasterSpace *
Type1Font::mmspace() const
{
    if (!_cached_mmspace)
        create_mmspace();
    return _mmspace;
}

MultipleMasterSpace *
Type1Font::create_mmspace(ErrorHandler *errh) const
{
    if (_cached_mmspace)
        return _mmspace;
    _cached_mmspace = 1;

    Type1Definition *t1d;

    Vector< Vector<double> > master_positions;
    t1d = fi_dict("BlendDesignPositions");
    if (!t1d || !t1d->value_numvec_vec(master_positions))
        return 0;

    int nmasters = master_positions.size();
    if (nmasters <= 0) {
        errh->error("bad BlendDesignPositions");
        return 0;
    }
    int naxes = master_positions[0].size();
    _mmspace = new MultipleMasterSpace(font_name(), naxes, nmasters);
    _mmspace->set_master_positions(master_positions);

    Vector< Vector<double> > normalize_in, normalize_out;
    t1d = fi_dict("BlendDesignMap");
    if (t1d && t1d->value_normalize(normalize_in, normalize_out))
        _mmspace->set_normalize(normalize_in, normalize_out);

    Vector<PermString> axis_types;
    t1d = fi_dict("BlendAxisTypes");
    if (t1d && t1d->value_namevec(axis_types) && axis_types.size() == naxes)
        for (int a = 0; a < naxes; a++)
            _mmspace->set_axis_type(a, axis_types[a]);

    int ndv, cdv;
    Type1Charstring *cs;
    t1d = p_dict("NDV");
    if (t1d && t1d->value_int(ndv) && (cs = subr(ndv)))
        _mmspace->set_ndv(*cs);
    t1d = p_dict("CDV");
    if (t1d && t1d->value_int(cdv) && (cs = subr(cdv)))
        _mmspace->set_cdv(*cs);

    Vector<double> design_vector;
    t1d = dict("DesignVector");
    if (t1d && t1d->value_numvec(design_vector))
        _mmspace->set_design_vector(design_vector);

    Vector<double> weight_vector;
    t1d = dict("WeightVector");
    if (t1d && t1d->value_numvec(weight_vector))
        _mmspace->set_weight_vector(weight_vector);

    if (!_mmspace->check(errh)) {
        delete _mmspace;
        _mmspace = 0;
    }
    return _mmspace;
}

void
Type1Font::uncache_defs()
{
    _cached_defs = false;
}

void
Type1Font::font_matrix(double matrix[6]) const
{
    Vector<double> t1d_matrix;
    Type1Definition *t1d = dict("FontMatrix");
    if (t1d && t1d->value_numvec(t1d_matrix) && t1d_matrix.size() == 6)
        memcpy(&matrix[0], &t1d_matrix[0], sizeof(double) * 6);
    else {
        matrix[0] = matrix[3] = 0.001;
        matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0;
    }
}

}
