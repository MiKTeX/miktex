/* glyphfilter.{cc,hh} -- define subsets of characters
 *
 * Copyright (c) 2004-2019 Eddie Kohler
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
#ifdef WIN32
# define _USE_MATH_DEFINES
#endif
#include "glyphfilter.hh"
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <ctype.h>
#include <algorithm>
#include "uniprop.hh"
#include "util.hh"

bool
GlyphFilter::allow(Efont::OpenType::Glyph glyph, const Vector<PermString>& glyph_names, uint32_t unicode, int ptype) const
{
    // out-of-range glyphs never match
    if (glyph < 0 || glyph >= glyph_names.size())
        return false;

    String glyph_name = glyph_names[glyph];
    int uniprop = -1;
    bool any_includes = false;
    bool included = false;

    // loop over patterns
    for (const Pattern* p = _patterns.begin(); p < _patterns.end(); p++) {
        // check pattern type
        if ((p->type & ~T_TYPEMASK) != ptype)
            continue;
        // check include/exclude
        if ((p->type & T_EXCLUDE) == 0) {
            if (included)
                continue;
            any_includes = true;
        }
        // check if there's a match
        bool match;
        if (p->data == D_NAME)
            match = glob_match(glyph_name, p->pattern);
        else if (p->data == D_UNIPROP) {
            if (uniprop < 0)
                uniprop = UnicodeProperty::property(unicode);
            match = ((uniprop & p->u.uniprop.mask) == p->u.uniprop.value);
        } else
            match = (unicode >= p->u.unirange.low && unicode <= p->u.unirange.high);
        // act if match
        if (match == ((p->type & T_NEGATE) == 0)) {
            if ((p->type & T_EXCLUDE) == 0)
                included = true;
            else
                return false;
        }
    }

    return !any_includes || included;
}

GlyphFilter::Pattern::Pattern(uint16_t ptype)
    : type(ptype), data(D_NAME)
{
    // make sure that even unused data has a known value, to simplify
    // operator==
    u.unirange.low = u.unirange.high = 0;
}

int
GlyphFilter::Pattern::compare(const GlyphFilter::Pattern& a, const GlyphFilter::Pattern& b)
{
    int cmp = a.type - b.type;
    if (cmp == 0)
        cmp = a.data - b.data;
    if (cmp == 0)
        cmp = (int) (a.u.unirange.low - b.u.unirange.low);
    if (cmp == 0)
        cmp = (int) (a.u.unirange.high - b.u.unirange.high);
    if (cmp == 0)
        cmp = String::compare(a.pattern, b.pattern);
    return cmp;
}

void
GlyphFilter::add_pattern(const String& pattern, int ptype, ErrorHandler* errh)
{
    _sorted = false;

    const char* begin = pattern.begin();
    const char* end = pattern.end();
    while (begin < end && isspace((unsigned char) *begin))
        begin++;
    if (begin >= end)
        errh->error("missing pattern");

    while (begin < end) {
        const char* word = begin;
        while (word < end && !isspace((unsigned char) *word))
            word++;
        bool negated = false;
        if (begin < word && begin[0] == '!')
            negated = true, begin++;

        // actually parse clause
        Pattern p(ptype + (negated ? T_NEGATE : 0));

        // unicode property
        if (begin + 3 <= word && begin[0] == '<' && word[-1] == '>') {
            p.data = D_UNIPROP;
            if (UnicodeProperty::parse_property(pattern.substring(begin + 1, word - 1), p.u.uniprop.value, p.u.uniprop.mask))
                _patterns.push_back(p);
            else if (errh)
                errh->error("unknown Unicode property %<%s%>", pattern.c_str());
            goto next_clause;
        }

        // unicode values
        {
            const char* dash = std::find(begin, word, '-');
            if (parse_unicode_number(begin, dash, 2, p.u.unirange.low)) {
                if (dash == word)
                    p.u.unirange.high = p.u.unirange.low;
                else if (dash == word - 1)
                    p.u.unirange.high = 0xFFFFFFFFU;
                else if (parse_unicode_number(dash + 1, word, (begin[0] == 'U' ? 1 : 0), p.u.unirange.high))
                    /* do nothing */;
                else
                    goto name_pattern; // assume it's a name
                p.data = D_UNIRANGE;
                _patterns.push_back(p);
                goto next_clause;
            }
        }

        // otherwise must be name pattern
    name_pattern:
        p.data = D_NAME;
        p.pattern = pattern.substring(begin, word);
        _patterns.push_back(p);

        // move to next clause
      next_clause:
        for (begin = word; begin < end && isspace((unsigned char) *begin); begin++)
            /* nada */;
    }
}

void
GlyphFilter::add_substitution_filter(const String& s, bool is_exclude, ErrorHandler* errh)
{
    add_pattern(s, is_exclude ? T_SRC + T_EXCLUDE : T_SRC, errh);
}

void
GlyphFilter::add_alternate_filter(const String& s, bool is_exclude, ErrorHandler* errh)
{
    add_pattern(s, is_exclude ? T_DST + T_EXCLUDE : T_DST, errh);
}

GlyphFilter&
GlyphFilter::operator+=(const GlyphFilter& gf)
{
    // be careful about self-addition
    _patterns.reserve(gf._patterns.size());
    const Pattern* end = gf._patterns.end();
    for (const Pattern* p = gf._patterns.begin(); p < end; p++)
        _patterns.push_back(*p);
    return *this;
}

GlyphFilter
operator+(const GlyphFilter& a, const GlyphFilter& b)
{
    if (!b)
        return a;
    if (!a)
        return b;
    GlyphFilter x(a);
    x += b;
    return x;
}

bool
operator==(const GlyphFilter& a, const GlyphFilter& b)
{
    if (&a == &b)
        return true;
    if (a._patterns.size() != b._patterns.size())
        return false;
    const GlyphFilter::Pattern* pa = a._patterns.begin();
    const GlyphFilter::Pattern* pb = b._patterns.begin();
    for (; pa < a._patterns.end(); pa++, pb++)
        if (!(*pa == *pb))
            return false;
    return true;
}

void
GlyphFilter::sort()
{
    if (!_sorted) {
        std::sort(_patterns.begin(), _patterns.end());
        Pattern* true_end = std::unique(_patterns.begin(), _patterns.end());
        _patterns.erase(true_end, _patterns.end());
        _sorted = true;
    }
}

void
GlyphFilter::unparse(StringAccum& sa) const
{
    for (const Pattern* p = _patterns.begin(); p < _patterns.end(); p++) {
        sa << (p->type & T_DST ? 'D' : 'S') << (p->type & T_NEGATE ? "!" : "") << (p->type & T_EXCLUDE ? "X" : "");
        if (p->data == D_NAME)
            sa << '<' << p->pattern << '>';
        else if (p->data == D_UNIPROP)
            sa << "[UNIPROP:" << p->u.uniprop.mask << '=' << p->u.uniprop.value << ']';
        else
            sa.snprintf(20, "[U+%02x-U+%02x]", p->u.unirange.low, p->u.unirange.high);
        sa << ' ';
    }
    if (_patterns.size())
        sa.pop_back();
}
