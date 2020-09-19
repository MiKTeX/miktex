/* dvipsencoding.{cc,hh} -- store a DVIPS encoding
 *
 * Copyright (c) 2003-2019 Eddie Kohler
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
#include "dvipsencoding.hh"
#include "metrics.hh"
#include "secondary.hh"
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <algorithm>
#include "util.hh"

enum { GLYPHLIST_ALTERNATIVE = 0x40000000,
       GLYPHLIST_USEMAP = GLYPHLIST_ALTERNATIVE,
       U_EMPTYSLOT = 0xD801,
       U_ALTSELECTOR = 0xD802 };
static HashMap<String, uint32_t> glyphlist((uint32_t) -1);
static Vector<uint32_t> glyphmap;
static PermString::Initializer perm_initializer;
PermString DvipsEncoding::dot_notdef(".notdef");

#define NEXT_GLYPH_NAME(gn)     ("/" + (gn))

void
DvipsEncoding::add_glyphlist(String text)
{
    const char *s = text.begin(), *end = text.end();
    while (s != end) {
        // move to first nonblank
        while (s != end && isspace((unsigned char) *s))
            ++s;
        // ignore comments
        if (s != end && *s == '#') {
        skip_to_end_of_line:
            while (s != end && *s != '\n' && *s != '\r')
                ++s;
            continue;
        }
        // parse glyph name
        const char *name_start = s;
        while (s != end && !isspace((unsigned char) *s) && *s != ';')
            ++s;
        if (s == name_start)
            goto skip_to_end_of_line;
        String glyph_name = text.substring(name_start, s).compact();
        int map_pos = glyphmap.size();
        // parse Unicodes
        while (1) {
            while (s != end && (*s == ' ' || *s == '\t'))
                ++s;
            if (s == end || *s == '\n' || *s == '\r' || *s == '#'
                || (map_pos == glyphmap.size() && *s != ';' && *s != ','))
                break;
            if (*s == ';' || *s == ',') {
                ++s;
                while (s != end && (*s == ' ' || *s == '\t'))
                    ++s;
                if (s == end || !isxdigit((unsigned char) *s))
                    goto skip_to_end_of_line;
                if (map_pos != glyphmap.size())
                    glyphmap.push_back(GLYPHLIST_ALTERNATIVE);
            }
            uint32_t u = 0;
            while (s != end && isxdigit((unsigned char) *s)) {
                if (*s >= '0' && *s <= '9')
                    u = (u << 4) + *s - '0';
                else if (*s >= 'A' && *s <= 'F')
                    u = (u << 4) + *s - 'A' + 10;
                else
                    u = (u << 4) + *s - 'a' + 10;
                ++s;
            }
            if (u == 0 || u > 0x10FFFF)
                goto skip_to_end_of_line;
            glyphmap.push_back(u);
            if (s != end && !isspace((unsigned char) *s) && *s != ',' && *s != ';')
                break;
        }
        // store result
        if (map_pos == glyphmap.size() - 1) {
            glyphlist.insert(glyph_name, glyphmap.back());
            glyphmap.pop_back();
        } else {
            glyphlist.insert(glyph_name, map_pos | GLYPHLIST_USEMAP);
            glyphmap.push_back(0);
        }
        goto skip_to_end_of_line;
    }
}

static void
unicode_add_suffix(Vector<uint32_t> &prefix,
                   int prefix_starting_from,
                   const Vector<uint32_t> &suffix)
{
    int prefix_size = prefix.size();
    for (Vector<uint32_t>::const_iterator it = suffix.begin();
         it != suffix.end() && *it != 0;
         ++it)
        if (*it == GLYPHLIST_ALTERNATIVE) {
            prefix.push_back(*it);
            for (int i = prefix_starting_from; i < prefix_size; ++i)
                prefix.push_back(prefix[i]);
        } else
            prefix.push_back(*it);
}

bool
DvipsEncoding::glyphname_unicode(String gn, Vector<uint32_t> &unis)
{
    int unis_first_size = unis.size();

    // drop all characters to the right of the first dot
    String::iterator dot = std::find(gn.begin(), gn.end(), '.');
    if (dot > gn.begin() && dot < gn.end())
        gn = gn.substring(gn.begin(), dot);

    // map the first component, handle later components recursively
    String::iterator underscore = std::find(gn.begin(), gn.end(), '_');
    String component = gn.substring(gn.begin(), underscore);
    int prefix_start = 0;
    Vector<uint32_t> suffix;
    if (String gn_suffix = gn.substring(underscore + 1, gn.end())) {
        if (!glyphname_unicode(gn_suffix, suffix))
            return false;
    }

    // check glyphlist
    int value = glyphlist[component];
    uint32_t uval;
    if (value >= 0 && !(value & GLYPHLIST_USEMAP))
        unis.push_back(value);
    else if (value >= 0) {
        for (int i = (value & ~GLYPHLIST_USEMAP);
             glyphmap[i];
             ++i)
            if (glyphmap[i] == GLYPHLIST_ALTERNATIVE) {
                unicode_add_suffix(unis, prefix_start, suffix);
                unis.push_back(GLYPHLIST_ALTERNATIVE);
                prefix_start = unis.size();
            } else
                unis.push_back(glyphmap[i]);
    } else if (component.length() >= 7
               && (component.length() % 4) == 3
               && (memcmp(component.data(), "uni", 3) == 0
                   // 16.Aug.2008: Some texnansx.enc have incorrect "Uni"
                   // prefix, but we might as well understand it.
                   || memcmp(component.data(), "Uni", 3) == 0)) {
        for (const char *s = component.begin() + 3;
             s < component.end();
             s += 4)
            if (parse_unicode_number(s, s + 4, -1, uval))
                unis.push_back(uval);
            else {
                unis.resize(unis_first_size);
                return false;
            }
    } else if (component.length() >= 5
               && component.length() <= 7
               && component[0] == 'u'
               && parse_unicode_number(component.begin() + 1, component.end(), -1, uval))
        unis.push_back(uval);
    else
        return false;

    unicode_add_suffix(unis, prefix_start, suffix);
    return true;
}


DvipsEncoding::DvipsEncoding()
    : _boundary_char(-1), _altselector_char(-1), _unicoding_map(-1),
      _warn_missing(false)
{
}

void
DvipsEncoding::encode(int e, PermString what)
{
    if (e >= _e.size())
        _e.resize(e + 1, dot_notdef);
    _e[e] = what;
}

int
DvipsEncoding::encoding_of(PermString what, bool encoding_required)
{
    int slot = -1;
    for (int i = 0; i < _e.size(); i++)
        if (_e[i] == what) {
            slot = i;
            goto use_slot;
        } else if (!_e[i] || _e[i] == dot_notdef)
            slot = i;
    if (what == "||")
        return _boundary_char;
    else if (!encoding_required || slot < 0)
        return -1;
  use_slot:
    if (encoding_required) {
        if (slot >= _encoding_required.size())
            _encoding_required.resize(slot + 1, false);
        _encoding_required[slot] = true;
        this->encode(slot, what);
    }
    return slot;
}

static String
tokenize(const String &s, int &pos_in, int &line)
{
    const char *data = s.data();
    int len = s.length();
    int pos = pos_in;
    while (1) {
        // skip whitespace
        while (pos < len && isspace((unsigned char) data[pos])) {
            if (data[pos] == '\n')
                line++;
            else if (data[pos] == '\r' && (pos + 1 == len || data[pos+1] != '\n'))
                line++;
            pos++;
        }

        if (pos >= len) {
            pos_in = len;
            return String();
        } else if (data[pos] == '%') {
            for (pos++; pos < len && data[pos] != '\n' && data[pos] != '\r'; pos++)
                /* nada */;
        } else if (data[pos] == '[' || data[pos] == ']' || data[pos] == '{' || data[pos] == '}') {
            pos_in = pos + 1;
            return s.substring(pos, 1);
        } else if (data[pos] == '(') {
            int first = pos, nest = 0;
            for (pos++; pos < len && (data[pos] != ')' || nest); pos++)
                switch (data[pos]) {
                  case '(': nest++; break;
                  case ')': nest--; break;
                  case '\\':
                    if (pos + 1 < len)
                        pos++;
                    break;
                  case '\n': line++; break;
                  case '\r':
                    if (pos + 1 == len || data[pos+1] != '\n')
                        line++;
                    break;
                }
            pos_in = (pos < len ? pos + 1 : len);
            return s.substring(first, pos_in - first);
        } else {
            int first = pos;
            while (pos < len && data[pos] == '/')
                pos++;
            while (pos < len && data[pos] != '/' && !isspace((unsigned char) data[pos]) && data[pos] != '[' && data[pos] != ']' && data[pos] != '%' && data[pos] != '(' && data[pos] != '{' && data[pos] != '}')
                pos++;
            pos_in = pos;
            return s.substring(first, pos - first);
        }
    }
}


static String
comment_tokenize(const String &s, int &pos_in, int &line)
{
    const char *data = s.data();
    int len = s.length();
    int pos = pos_in;
    while (1) {
        while (pos < len && data[pos] != '%' && data[pos] != '(') {
            if (data[pos] == '\n')
                line++;
            else if (data[pos] == '\r' && (pos + 1 == len || data[pos+1] != '\n'))
                line++;
            pos++;
        }

        if (pos >= len) {
            pos_in = len;
            return String();
        } else if (data[pos] == '%') {
            for (pos++; pos < len && (data[pos] == ' ' || data[pos] == '\t'); pos++)
                /* nada */;
            int first = pos;
            for (; pos < len && data[pos] != '\n' && data[pos] != '\r'; pos++)
                /* nada */;
            pos_in = pos;
            if (pos > first)
                return s.substring(first, pos - first);
        } else {
            int nest = 0;
            for (pos++; pos < len && (data[pos] != ')' || nest); pos++)
                switch (data[pos]) {
                  case '(': nest++; break;
                  case ')': nest--; break;
                  case '\\':
                    if (pos + 1 < len)
                        pos++;
                    break;
                  case '\n': line++; break;
                  case '\r':
                    if (pos + 1 == len || data[pos+1] != '\n')
                        line++;
                    break;
                }
        }
    }
}

static bool
retokenize_isword(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '.' || c == '_';
}


static struct { const char *s; int v; } ligkern_ops[] = {
    { "=:", DvipsEncoding::JL_LIG }, { "|=:", DvipsEncoding::JL_CLIG },
    { "|=:>", DvipsEncoding::JL_CLIG_S }, { "=:|", DvipsEncoding::JL_LIGC },
    { "=:|>", DvipsEncoding::JL_LIGC_S }, { "|=:|", DvipsEncoding::JL_CLIGC },
    { "|=:>", DvipsEncoding::JL_CLIGC_S }, { "|=:|>>", DvipsEncoding::JL_CLIGC_SS },
    { "{}", DvipsEncoding::JT_KERN }, { "{K}", DvipsEncoding::JT_KERN },
    { "{L}", DvipsEncoding::JT_LIG }, { "{LK}", DvipsEncoding::JT_NOLIGKERN },
    { "{KL}", DvipsEncoding::JT_NOLIGKERN }, { "{k}", DvipsEncoding::JT_KERN },
    { "{l}", DvipsEncoding::JT_LIG }, { "{lk}", DvipsEncoding::JT_NOLIGKERN },
    { "{kl}", DvipsEncoding::JT_NOLIGKERN },
    // some encodings have @{@} instead of {}
    { "@{@}", DvipsEncoding::JT_KERN },
    { 0, 0 }
};

static int
find_ligkern_op(const String &s)
{
    for (int i = 0; ligkern_ops[i].s; i++)
        if (ligkern_ops[i].s == s)
            return ligkern_ops[i].v;
    return 0;
}

inline bool
operator==(const DvipsEncoding::Ligature& l1, const DvipsEncoding::Ligature& l2)
{
    return l1.c1 == l2.c1 && l1.c2 == l2.c2;
}

void
DvipsEncoding::add_ligkern(const Ligature &l, int override)
{
    Ligature *old = std::find(_lig.begin(), _lig.end(), l);
    if (old == _lig.end())
        _lig.push_back(l);
    else {
        if ((l.join & JT_KERN) && (override > 0 || !(old->join & JT_KERN))) {
            old->join |= JT_KERN;
            old->k = l.k;
        }
        if ((l.join & JT_LIG) && (override > 0 || !(old->join & JT_LIG))) {
            old->join = (old->join & JT_KERN) | (l.join & JT_LIGALL);
            old->d = l.d;
        }
    }
}

int
DvipsEncoding::parse_ligkern_words(Vector<String> &v, int override, ErrorHandler *errh)
{
    _file_had_ligkern = true;
    int op;
    long l;
    char *endptr;
    if (v.size() == 3) {
        // empty string fails
        if (!v[0])
            return -1;
        // boundary char setting
        if (v[0] == "||" && v[1] == "=") {
            char *endptr;
            if (override > 0 || _boundary_char < 0)
                _boundary_char = strtol(v[2].c_str(), &endptr, 10);
            if (*endptr == 0 && _boundary_char < _e.size())
                return 0;
            else
                return errh->error("parse error in boundary character assignment");
        }
        // altselector char setting
        if (v[0] == "^^" && v[1] == "=") {
            char *endptr;
            if (override > 0 || _altselector_char < 0)
                _altselector_char = strtol(v[2].c_str(), &endptr, 10);
            if (*endptr == 0 && _altselector_char < _e.size())
                return 0;
            else
                return errh->error("parse error in altselector character assignment");
        }
        // encoding
        l = strtol(v[0].c_str(), &endptr, 0);
        if (endptr == v[0].end() && v[1] == "=") {
            if (l >= 0 && l < 256) {
                if (override > 0 || !_e[l])
                    encode(l, v[2]);
                return 0;
            } else
                return errh->error("encoding value %<%d%> out of range", l);
        }

        // kern operation
        if (v[1].length() >= 3 && v[1][0] == '{' && v[1].back() == '}') {
            String middle = v[1].substring(1, v[1].length() - 2);
            l = strtol(middle.c_str(), &endptr, 0);
            if (endptr == middle.end()) {
                op = JT_KERN;
                goto found_kernop;
            }
        }
        op = find_ligkern_op(v[1]);
        if (!op || (op & JT_ADDLIG))
            return -1;
      found_kernop:
        int av = (v[0] == "*" ? J_ALL : encoding_of(v[0]));
        if (av < 0)
            return errh->warning("bad %<%s%> (%<%s%> has no encoding)", v[1].c_str(), v[0].c_str());
        int bv = (v[2] == "*" ? J_ALL : encoding_of(v[2]));
        if (bv < 0)
            return errh->warning("bad %<%s%> (%<%s%> has no encoding)", v[1].c_str(), v[2].c_str());
        if ((op & JT_KERN) && l && (av == J_ALL || bv == J_ALL))
            return errh->warning("%<%s %s %s%> illegal, only {0} works with *", v[0].c_str(), v[1].c_str(), v[2].c_str());
        Ligature lig = { av, bv, op, static_cast<int>(l), 0 };
        add_ligkern(lig, override);
        return 0;

    } else if (v.size() == 4 && ((op = find_ligkern_op(v[2])) & JT_ADDLIG)) {
        int av = encoding_of(v[0], override > 0);
        if (av < 0)
            return (override > 0 ? errh->warning("bad ligature (%<%s%> has no encoding)", v[0].c_str()) : -1);
        int bv = encoding_of(v[1], override > 0);
        if (bv < 0)
            return (override > 0 ? errh->warning("bad ligature (%<%s%> has no encoding)", v[1].c_str()) : -1);
        int cv = encoding_of(v[3], override > 0);
        if (cv < 0)
            return (override > 0 ? errh->warning("bad ligature (%<%s%> has no encoding)", v[3].c_str()) : -1);
        Ligature lig = { av, bv, op, 0, cv };
        add_ligkern(lig, override);
        return 0;

    } else
        return -EPARSE;
}

int
DvipsEncoding::parse_position_words(Vector<String> &v, int override, ErrorHandler *errh)
{
    if (v.size() != 4)
        return -EPARSE;

    int c = encoding_of(v[0]);
    if (c < 0)
        return (override > 0 ? errh->warning("bad positioning (%<%s%> has no encoding)", v[0].c_str()) : -1);

    char *endptr;
    int pdx, pdy, adx;
    if (!v[1] || !v[2] || !v[3]
        || (pdx = strtol(v[1].c_str(), &endptr, 10), *endptr)
        || (pdy = strtol(v[2].c_str(), &endptr, 10), *endptr)
        || (adx = strtol(v[3].c_str(), &endptr, 10), *endptr))
        return errh->error("parse error in POSITION");

    Ligature l = { c, pdx, pdy, adx, 0 };
    Ligature *old = std::find(_pos.begin(), _pos.end(), l);
    if (old == _pos.end())
        _pos.push_back(l);
    else if (override > 0)
        *old = l;
    return 0;
}

int
DvipsEncoding::parse_unicoding_words(Vector<String> &v, int override, ErrorHandler *errh)
{
    int av;
    if (v.size() < 2 || (v[1] != "=" && v[1] != "=:" && v[1] != ":="))
        return -EPARSE;
    else if (v[0] == "||" || (av = encoding_of(v[0])) < 0)
        return errh->warning("bad UNICODING (%<%s%> has no encoding)", v[0].c_str());

    int original_size = _unicoding.size();

    if (v.size() == 2 || (v.size() == 3 && v[2] == dot_notdef))
        /* no warnings to delete a glyph */;
    else {
        for (int i = 2; i < v.size(); i++) {
            if (_unicoding.size() != original_size)
                _unicoding.push_back(GLYPHLIST_ALTERNATIVE);
            if (!glyphname_unicode(v[i], _unicoding)) {
                errh->warning("can%,t map %<%s%> to Unicode", v[i].c_str());
                if (i == v.size() - 1 && _unicoding.size() == original_size)
                    errh->warning("target %<%s%> will be deleted from encoding", v[0].c_str());
                else if (_unicoding.size() != original_size)
                    _unicoding.pop_back();
            }
        }
    }

    _unicoding.push_back(0);
    if (override > 0 || _unicoding_map[v[0]] < 0)
        _unicoding_map.insert(v[0], original_size);
    return 0;
}

const DvipsEncoding::WordType DvipsEncoding::word_types[] = {
    { "LIGKERN", &DvipsEncoding::parse_ligkern_words },
    { "POSITION", &DvipsEncoding::parse_position_words },
    { "UNICODING", &DvipsEncoding::parse_unicoding_words }
};

void
DvipsEncoding::parse_word_group(Vector<String> &words, int override, int wt, ErrorHandler *errh)
{
    if (words.size() > 0) {
        int (DvipsEncoding::*method)(Vector<String> &, int, ErrorHandler *) = word_types[wt].parsefunc;
        if ((this->*method)(words, override, errh) == -EPARSE) {
            Vector<String> rewords;
            for (String *sp = words.begin(); sp != words.end(); sp++) {
                const char *s = sp->begin(), *ends = sp->end();
                while (s != ends) {
                    const char *word = s;
                    if (*s == '{') {
                        for (s++; s != ends && *s != '}'; s++)
                            /* nada */;
                        if (s != ends)
                            s++;
                    } else {
                        bool x = retokenize_isword(*s);
                        for (s++; s != ends && x == retokenize_isword(*s); s++)
                            /* nada */;
                    }
                    rewords.push_back(sp->substring(word, s));
                }
            }
            if ((this->*method)(rewords, override, errh) == -EPARSE)
                errh->error("parse error in %s", word_types[wt].name);

        }
        words.clear();
    }
}

int
DvipsEncoding::parse_words(const String &s, int override, int wt, ErrorHandler *errh)
{
    Vector<String> words;
    const char *data = s.data();
    const char *end = s.end();
    while (data < end) {
        while (data < end && isspace((unsigned char) *data))
            data++;
        const char *first = data;
        while (data < end && !isspace((unsigned char) *data) && *data != ';')
            data++;
        if (data == first) {
            data++;             // step past semicolon (or harmlessly past EOS)
            parse_word_group(words, override, wt, errh);
        } else
            words.push_back(s.substring(first, data));
    }
    parse_word_group(words, override, wt, errh);
    return 0;
}

String
DvipsEncoding::landmark(int line) const
{
    StringAccum sa;
    sa << _printable_filename << ':' << line;
    return sa.take_string();
}

static String
trim_space(const String &s, int pos)
{
    while (pos < s.length() && isspace((unsigned char) s[pos]))
        pos++;
    int epos = s.length();
    for (int x = 0; x < 2; x++) {
        while (epos > pos && isspace((unsigned char) s[epos - 1]))
            epos--;
        if (epos == pos || s[epos - 1] != ';')
            break;
        epos--;
    }
    return s.substring(pos, epos - pos);
}

int
DvipsEncoding::parse(String filename, bool ignore_ligkern, bool ignore_other, ErrorHandler *errh)
{
    int before = errh->nerrors();
    String s = read_file(filename, errh);
    if (errh->nerrors() != before)
        return -1;
    _filename = filename;
    _printable_filename = printable_filename(filename);
    _file_had_ligkern = false;
    int pos = 0, line = 1;

    // parse text
    String token = tokenize(s, pos, line);
    if (!token || token[0] != '/')
        return errh->lerror(landmark(line), "parse error, expected name");
    _name = token.substring(1);
    _initial_comment = s.substring(0, pos - token.length());

    if (tokenize(s, pos, line) != "[")
        return errh->lerror(landmark(line), "parse error, expected [");

    while ((token = tokenize(s, pos, line)) && token[0] == '/')
        _e.push_back(token.substring(1));

    _final_text = token + s.substring(pos);

    // now parse comments
    pos = 0, line = 1;
    Vector<String> words;
    LandmarkErrorHandler lerrh(errh, "");
    while ((token = comment_tokenize(s, pos, line)))
        if (token.length() >= 8
            && memcmp(token.data(), "LIGKERN", 7) == 0
            && isspace((unsigned char) token[7])
            && !ignore_ligkern) {
            lerrh.set_landmark(landmark(line));
            parse_words(token.substring(8), 1, WT_LIGKERN, &lerrh);

        } else if (token.length() >= 9
                   && memcmp(token.data(), "LIGKERNX", 8) == 0
                   && isspace((unsigned char) token[8])
                   && !ignore_ligkern) {
            lerrh.set_landmark(landmark(line));
            parse_words(token.substring(9), 1, WT_LIGKERN, &lerrh);

        } else if (token.length() >= 10
                   && memcmp(token.data(), "UNICODING", 9) == 0
                   && isspace((unsigned char) token[9])
                   && !ignore_other) {
            lerrh.set_landmark(landmark(line));
            parse_words(token.substring(10), 1, WT_UNICODING, &lerrh);

        } else if (token.length() >= 9
                   && memcmp(token.data(), "POSITION", 8) == 0
                   && isspace((unsigned char) token[8])
                   && !ignore_other) {
            lerrh.set_landmark(landmark(line));
            parse_words(token.substring(9), 1, WT_POSITION, &lerrh);

        } else if (token.length() >= 13
                   && memcmp(token.data(), "CODINGSCHEME", 12) == 0
                   && isspace((unsigned char) token[12])
                   && !ignore_other) {
            _coding_scheme = trim_space(token, 13);
            if (_coding_scheme.length() > 39)
                lerrh.lwarning(landmark(line), "only first 39 chars of CODINGSCHEME are significant");
            if (std::find(_coding_scheme.begin(), _coding_scheme.end(), '(') < _coding_scheme.end()
                || std::find(_coding_scheme.begin(), _coding_scheme.end(), ')') < _coding_scheme.end()) {
                lerrh.lerror(landmark(line), "CODINGSCHEME cannot contain parentheses");
                _coding_scheme = String();
            }

        } else if (token.length() >= 11
                   && memcmp(token.data(), "WARNMISSING", 11) == 0
                   && (token.length() == 11 || isspace((unsigned char) token[11]))
                   && !ignore_other) {
            String value = trim_space(token, 11);
            if (value == "1" || value == "yes" || value == "true" || !value)
                _warn_missing = true;
            else if (value == "0" || value == "no" || value == "false")
                _warn_missing = false;
            else
                lerrh.lerror(landmark(line), "WARNMISSING command not understood");
        }

    return 0;
}

int
DvipsEncoding::parse_ligkern(const String &ligkern_text, int override, ErrorHandler *errh)
{
    return parse_words(ligkern_text, override, WT_LIGKERN, errh);
}

int
DvipsEncoding::parse_position(const String &position_text, int override, ErrorHandler *errh)
{
    return parse_words(position_text, override, WT_POSITION, errh);
}

int
DvipsEncoding::parse_unicoding(const String &unicoding_text, int override, ErrorHandler *errh)
{
    return parse_words(unicoding_text, override, WT_UNICODING, errh);
}

void
DvipsEncoding::bad_codepoint(int code, Metrics &metrics, HashMap<PermString, int> &unencoded)
{
    for (int i = 0; i < _lig.size(); i++) {
        Ligature &l = _lig[i];
        if (l.c1 == code || l.c2 == code)
            l.join = 0;
        else if ((l.join & JT_ADDLIG) && l.d == code)
            l.join &= ~JT_LIGALL;
    }

    if (_warn_missing) {
        Vector<uint32_t> garbage;
        bool unicodes_explicit = x_unicodes(_e[code], garbage);
        if (!unicodes_explicit || garbage.size() > 0) {
            Vector<Setting> v;
            v.push_back(Setting(Setting::RULE, 500, 500));
            v.push_back(Setting(Setting::SPECIAL, String("Warning: missing glyph '") + _e[code] + "'"));
            metrics.encode_virtual(code, _e[code], 0, v, true);
            unencoded.insert(_e[code], 1);
        }
    }
}

static inline Efont::OpenType::Glyph
map_uni(uint32_t uni, const Efont::OpenType::Cmap &cmap, const Metrics &m)
{
    if (uni == U_EMPTYSLOT)
        return m.emptyslot_glyph();
    else
        return cmap.map_uni(uni);
}

bool
DvipsEncoding::x_unicodes(PermString chname, Vector<uint32_t> &unicodes) const
{
    int i = _unicoding_map[chname];
    if (i >= 0) {
        for (; _unicoding[i] > 0; i++)
            unicodes.push_back(_unicoding[i]);
        return true;
    } else {
        glyphname_unicode(chname, unicodes);
        return false;
    }
}


void
DvipsEncoding::make_metrics(Metrics &metrics, const FontInfo &finfo, Secondary *secondary, bool literal, ErrorHandler *errh)
{
    // first pass: without secondaries
    for (int code = 0; code < _e.size(); code++) {
        PermString chname = _e[code];

        // common case: skip .notdef
        if (chname == dot_notdef)
            continue;

        // find first single Unicode glyph supported by the font
        Efont::OpenType::Glyph glyph = 0;
        uint32_t glyph_uni = 0;
        {
            Vector<uint32_t> unicodes;
            (void) x_unicodes(chname, unicodes);
            Vector<uint32_t>::iterator u = unicodes.begin();
            while (u != unicodes.end() && glyph <= 0) {
                uint32_t this_uni = u[0];
                ++u;
                if (u != unicodes.end()) {
                    if (*u != GLYPHLIST_ALTERNATIVE)
                        break;
                    ++u;
                }

                glyph = map_uni(this_uni, *finfo.cmap, metrics);
                if (glyph_uni == 0 || glyph > 0)
                    glyph_uni = this_uni;
            }
        }

        // find named glyph, if any
        Efont::OpenType::Glyph named_glyph = finfo.glyphid(chname);
#if 0
        // 2.May.2008: ff, fi, fl, ffi, and ffl might map to f_f, f_i, f_l,
        // f_f_i, and f_f_l
        if (!named_glyph && chname.length() > 0 && chname.length() <= 3
            && chname[0] == 'f') {
            if (chname.equals("ff", 2))
                named_glyph = finfo.glyphid("f_f");
            else if (chname.equals("fi", 2))
                named_glyph = finfo.glyphid("f_i");
            else if (chname.equals("fl", 2))
                named_glyph = finfo.glyphid("f_l");
            else if (chname.equals("ffi", 2))
                named_glyph = finfo.glyphid("f_f_i");
            else if (chname.equals("ffl", 2))
                named_glyph = finfo.glyphid("f_f_l");
        }
#endif
        // do not use a Unicode-mapped glyph if literal
        if (literal)
            glyph = named_glyph;

        // If we found a glyph, maybe use its named_glyph variant.
        if (glyph > 0 && named_glyph > 0
            && std::find(chname.begin(), chname.end(), '.') < chname.end())
            glyph = named_glyph;

        // assign slot
        if (glyph > 0)
            metrics.encode(code, glyph_uni, glyph);
    }

    // second pass: with secondaries
    for (int code = 0; code < _e.size(); code++) {
        // skip already-encoded characters and .notdef
        if (literal || metrics.glyph(code) > 0 || _e[code] == dot_notdef)
            continue;

        PermString chname = _e[code];

        // find all Unicodes
        Vector<uint32_t> unicodes;
        bool unicodes_explicit = x_unicodes(chname, unicodes);

        // find named glyph, if any
        Efont::OpenType::Glyph named_glyph = finfo.glyphid(chname);

        // 1. We were not able to find the glyph using Unicode.
        // 2. There might be a named_glyph.
        // May need to try secondaries later.  Store this slot.
        // Try secondaries, if there's explicit unicoding or no named_glyph.
        if (unicodes_explicit || named_glyph <= 0)
            for (uint32_t *u = unicodes.begin(); u != unicodes.end(); ) {
                uint32_t *endu = u + 1;
                while (endu != unicodes.end() && *endu != GLYPHLIST_ALTERNATIVE)
                    ++endu;
                if (secondary->encode_uni(code, chname, u, endu, metrics, errh))
                    goto encoded;
                u = (endu == unicodes.end() ? endu : endu + 1);
            }

        // 1. We were not able to find the glyph using Unicode or secondaries.
        // 2. There might be a named_glyph.
        // Use named glyph, if any.  Special case for "UNICODING foo =: ;",
        // which should turn off the character (even if a named_glyph exists),
        // UNLESS the glyph was explicitly requested.
        if (named_glyph > 0
            && (!unicodes_explicit
                || unicodes.size() > 0
                || (_encoding_required.size() > code && _encoding_required[code]))) {
            uint32_t uni = 0;
            if (unicodes.size() == 1 || (unicodes.size() > 0 && unicodes[1] == GLYPHLIST_ALTERNATIVE))
                uni = unicodes[0];
            metrics.encode(code, uni, named_glyph);
        }

      encoded:
        /* all set */;
    }

    // add altselector
    if (_altselector_char >= 0 && _altselector_char < _e.size()) {
        metrics.add_altselector_code(_altselector_char, 0);
        if (metrics.glyph(_altselector_char) <= 0 && !literal)
            (void) secondary->encode_uni(_altselector_char, "<altselector>", U_ALTSELECTOR, metrics, errh);
    }

    // final pass: complain
    HashMap<PermString, int> unencoded_map;
    for (int code = 0; code < _e.size(); code++)
        if (_e[code] != dot_notdef && metrics.glyph(code) <= 0)
            bad_codepoint(code, metrics, unencoded_map);
    Vector<String> unencoded;
    for (HashMap<PermString, int>::iterator it = unencoded_map.begin(); it; ++it)
        unencoded.push_back(it.key());

    if (unencoded.size() == 1) {
        errh->warning("%<%s%> glyph not found in font", unencoded[0].c_str());
        errh->message("(This glyph will appear as a blot and cause warnings if used.)");
    } else if (unencoded.size() > 1) {
        std::sort(unencoded.begin(), unencoded.end());
        StringAccum sa;
        for (const String* a = unencoded.begin(); a < unencoded.end(); a++)
            sa << *a << ' ';
        sa.pop_back();
        sa.append_break_lines(sa.take_string(), 68, "  ");
        sa.pop_back();
        errh->warning("%d glyphs not found in font:", unencoded.size());
        errh->message("%s\n(These glyphs will appear as blots and cause warnings if used.)", sa.c_str());
    }

    metrics.set_coding_scheme(_coding_scheme);
}

void
DvipsEncoding::make_base_mappings(Vector<int> &mappings, const FontInfo &finfo)
{
    mappings.clear();
    for (int code = 0; code < _e.size(); code++) {
        PermString chname = _e[code];

        // common case: skip .notdef
        if (chname == dot_notdef)
            continue;

        // find named glyph
        Efont::OpenType::Glyph named_glyph = finfo.glyphid(chname);
        if (named_glyph > 0) {
            if (mappings.size() <= named_glyph)
                mappings.resize(named_glyph + 1, -1);
            mappings[named_glyph] = code;
        }
    }
}


void
DvipsEncoding::apply_ligkern_lig(Metrics &metrics, ErrorHandler *errh) const
{
    assert((int)J_ALL == (int)Metrics::CODE_ALL);
    for (const Ligature *l = _lig.begin(); l < _lig.end(); l++) {
        if (l->c1 < 0 || l->c2 < 0 || l->join < 0 || !(l->join & JT_LIG))
            continue;
        metrics.remove_ligatures(l->c1, l->c2);
        if (!(l->join & JT_ADDLIG))
            /* nada */;
        else if ((l->join & JT_LIGALL) == JL_LIG)
            metrics.add_ligature(l->c1, l->c2, l->d);
        else if ((l->join & JT_LIGALL) == JL_LIGC)
            metrics.add_ligature(l->c1, l->c2, metrics.pair_code(l->d, l->c2));
        else if ((l->join & JT_LIGALL) == JL_CLIG)
            metrics.add_ligature(l->c1, l->c2, metrics.pair_code(l->c1, l->d));
        else {
            static int complex_join_warning = 0;
            if (!complex_join_warning) {
                errh->warning("complex LIGKERN ligature removed (I only support %<=:%>, %<=:|%>, and %<|=:%>)");
                complex_join_warning = 1;
            }
        }
    }
}

void
DvipsEncoding::apply_ligkern_kern(Metrics &metrics, ErrorHandler *) const
{
    assert((int)J_ALL == (int)Metrics::CODE_ALL);
    for (const Ligature *l = _lig.begin(); l < _lig.end(); l++)
        if (l->c1 >= 0 && l->c2 >= 0 && (l->join & JT_KERN))
            metrics.set_kern(l->c1, l->c2, l->k);
}

void
DvipsEncoding::apply_position(Metrics &metrics, ErrorHandler *) const
{
    for (const Ligature *l = _pos.begin(); l < _pos.end(); l++)
        if (l->c1 >= 0)
            metrics.add_single_positioning(l->c1, l->c2, l->join, l->k);
}
