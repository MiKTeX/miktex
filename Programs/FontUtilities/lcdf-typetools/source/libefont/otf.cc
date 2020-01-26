// -*- related-file-name: "../include/efont/otf.hh" -*-

/* otf.{cc,hh} -- OpenType font basics
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
#include <efont/otf.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <efont/otfdata.hh>     // for ntohl()
#include <efont/otfname.hh>
#include <efont/ttfhead.hh>

namespace Efont { namespace OpenType {

Vector<PermString> debug_glyph_names;

Font::Font(const String& s, ErrorHandler* errh)
    : _str(s), _units_per_em(0) {
    _str.align(4);
    _error = parse_header(errh ? errh : ErrorHandler::silent_handler());
}

int
Font::parse_header(ErrorHandler *errh)
{
    // HEADER FORMAT:
    // Fixed    sfnt version
    // USHORT   numTables
    // USHORT   searchRange
    // USHORT   entrySelector
    // USHORT   rangeShift
    int len = length();
    const uint8_t *data = this->data();
    if (HEADER_SIZE > len)
        return errh->error("OTF file corrupted (too small)"), -EFAULT;
    if ((data[0] != 'O' || data[1] != 'T' || data[2] != 'T' || data[3] != 'O')
        && (data[0] != '\000' || data[1] != '\001'))
        return errh->error("not an OpenType font (bad magic number)"), -ERANGE;
    int ntables = Data::u16_aligned(data + 4);
    if (ntables == 0)
        return errh->error("OTF contains no tables"), -EINVAL;
    if (HEADER_SIZE + TABLE_DIR_ENTRY_SIZE * ntables > len)
        return errh->error("OTF table directory out of range"), -EFAULT;

    // TABLE DIRECTORY ENTRY FORMAT:
    // ULONG    tag
    // ULONG    checksum
    // ULONG    offset
    // ULONG    length
    uint32_t last_tag = 0U;
    for (int i = 0; i < ntables; i++) {
        int loc = HEADER_SIZE + TABLE_DIR_ENTRY_SIZE * i;
        uint32_t tag = Data::u32_aligned(data + loc);
        uint32_t offset = Data::u32_aligned(data + loc + 8);
        uint32_t length = Data::u32_aligned(data + loc + 12);
        if (tag <= last_tag)
            return errh->error("tags out of order"), -EINVAL;
        if (offset + length > (uint32_t) len)
            return errh->error("OTF data for %<%s%> out of range", Tag(tag).text().c_str()), -EFAULT;
        if (Tag::head_tag() == tag) {
            Head head(_str.substring(offset, length));
            _units_per_em = head.units_per_em();
        }
        last_tag = tag;
    }

    return 0;
}

bool
Font::check_checksums(ErrorHandler *errh) const
{
    if (error() < 0)
        return false;
    int nt = ntables();
    bool ok = true;
    for (int i = 0; i < nt; i++) {
        const uint8_t *entry = data() + HEADER_SIZE + TABLE_DIR_ENTRY_SIZE * i;
        String tbl = _str.substring(Data::u32_aligned(entry + 8),
                                    Data::u32_aligned(entry + 12));
        uint32_t sum = checksum(tbl);
        if (Data::u32_aligned(entry) == 0x68656164      // 'head'
            && tbl.length() >= 12)
            sum -= Data::u32_aligned(tbl.udata() + 8);
        if (sum != Data::u32_aligned(entry + 4)) {
            if (errh)
                errh->error("table %<%s%> checksum error: %x vs. %x", Tag(Data::u32_aligned(entry)).text().c_str(), sum, Data::u32_aligned(entry + 4));
            ok = false;
        }
    }
    return ok;
}

int
Font::ntables() const
{
    if (error() < 0)
        return 0;
    else
        return Data::u16_aligned(data() + 4);
}

String
Font::table(Tag tag) const
{
    if (error() < 0)
        return String();
    const uint8_t *entry = tag.table_entry(data() + HEADER_SIZE, Data::u16_aligned(data() + 4), TABLE_DIR_ENTRY_SIZE);
    if (entry)
        return _str.substring(Data::u32_aligned(entry + 8), Data::u32_aligned(entry + 12));
    else
        return String();
}

bool
Font::has_table(Tag tag) const
{
    const uint8_t *entry = 0;
    if (error() >= 0)
        entry = tag.table_entry(data() + HEADER_SIZE, Data::u16_aligned(data() + 4), TABLE_DIR_ENTRY_SIZE);
    return entry != 0;
}

uint32_t
Font::table_checksum(Tag tag) const
{
    if (error() < 0)
        return 0;
    const uint8_t *entry = tag.table_entry(data() + HEADER_SIZE, Data::u16_aligned(data() + 4), TABLE_DIR_ENTRY_SIZE);
    if (entry)
        return Data::u32_aligned(entry + 4);
    else
        return 0;
}

Tag
Font::table_tag(int i) const
{
    if (error() < 0 || i < 0 || i >= ntables())
        return Tag();
    else
        return Tag(Data::u32_aligned(data() + HEADER_SIZE + TABLE_DIR_ENTRY_SIZE * i));
}

uint32_t
Font::checksum(const uint8_t *begin, const uint8_t *end)
{
    uint32_t sum = 0;
    if (reinterpret_cast<uintptr_t>(begin) % 4)
        for (; begin + 3 < end; begin += 4)
            sum += Data::u32(begin);
    else
        for (; begin + 3 < end; begin += 4)
            sum += Data::u32_aligned(begin);
    uint32_t leftover = 0;
    for (int i = 0; i < 4; i++)
        leftover = (leftover << 8) + (begin < end ? *begin++ : 0);
    return sum + leftover;
}

uint32_t
Font::checksum(const String &s)
{
    return checksum(reinterpret_cast<const uint8_t *>(s.begin()), reinterpret_cast<const uint8_t *>(s.end()));
}

namespace {
class TagCompar { public:
    TagCompar(const Vector<Tag> &tags) : _tags(tags) { }
    bool operator()(int a, int b) { return _tags[a] < _tags[b]; }
    const Vector<Tag> &_tags;
};
}

Font
Font::make(bool truetype, const Vector<Tag>& tags, const Vector<String>& data)
{
    StringAccum sa;

    // create offset table
    {
        union {
            uint8_t c[HEADER_SIZE];
            uint16_t s[HEADER_SIZE / 2];
            uint32_t l[HEADER_SIZE / 4];
        } hdr;
        hdr.l[0] = (truetype ? htonl(0x00010000) : htonl(0x4F54544F));
        hdr.s[2] = htons(tags.size());
        int entrySelector;
        for (entrySelector = 0; (2 << entrySelector) <= tags.size(); entrySelector++)
            /* nada */;
        hdr.s[3] = htons((1 << entrySelector) * 16);
        hdr.s[4] = htons(entrySelector);
        hdr.s[5] = htons((tags.size() - (1 << entrySelector)) * 16);
        sa.append(&hdr.c[0], HEADER_SIZE);
    }

    // sort tags
    Vector<int> permut;
    for (int i = 0; i < tags.size(); i++)
        permut.push_back(i);
    std::sort(permut.begin(), permut.end(), TagCompar(tags));

    // table listing
    uint32_t offset = HEADER_SIZE + TABLE_DIR_ENTRY_SIZE * tags.size();
    for (int *tp = permut.begin(); tp < permut.end(); tp++) {
        union {
            uint8_t c[TABLE_DIR_ENTRY_SIZE];
            uint32_t l[TABLE_DIR_ENTRY_SIZE / 4];
        } tdir;
        tdir.l[0] = htonl(tags[*tp].value());

        // discount current checksum adjustment in head table
        uint32_t sum = checksum(data[*tp]);
        if (tags[*tp] == Tag("head") && data[*tp].length() >= 12)
            sum -= Data::u32(data[*tp].udata() + 8);
        tdir.l[1] = htonl(sum);

        tdir.l[2] = htonl(offset);
        tdir.l[3] = htonl(data[*tp].length());
        sa.append(&tdir.c[0], TABLE_DIR_ENTRY_SIZE);
        offset += (data[*tp].length() + 3) & ~3;
    }

    // actual tables
    for (int *tp = permut.begin(); tp < permut.end(); tp++) {
        sa << data[*tp];
        while (sa.length() % 4)
            sa << '\0';
    }

    // fix 'head' table
    for (int i = 0; i < tags.size(); i++) {
        unsigned char *thdr = sa.udata() + HEADER_SIZE + TABLE_DIR_ENTRY_SIZE * i;
        if (Data::u32(thdr) == 0x68656164 && Data::u32(thdr + 12) >= 12) {
            uint32_t offset = Data::u32(thdr + 8);
            char *head = sa.data() + offset;
            memset(head + 8, '\0', 4);
            uint32_t allsum = checksum(reinterpret_cast<uint8_t *>(sa.data()), reinterpret_cast<uint8_t *>(sa.data() + sa.length()));
            uint32_t *adj = reinterpret_cast<uint32_t *>(head + 8);
            *adj = htonl(0xB1B0AFBA - allsum);
        }
    }

    return Font(sa.take_string());
}


/**************************
 * Tag                    *
 *                        *
 **************************/

Tag::Tag(const char *s)
    : _tag(0)
{
    if (!s)
        s = "";
    for (int i = 0; i < 4; i++)
        if (*s == 0)
            _tag = (_tag << 8) | 0x20;
        else if (*s < 32 || *s > 126) { // don't care if s is signed
            _tag = 0;
            return;
        } else
            _tag = (_tag << 8) | *s++;
    if (*s)
        _tag = 0;
}

Tag::Tag(const String &s)
    : _tag(0)
{
    if (s.length() <= 4) {
        const char *ss = s.data();
        for (int i = 0; i < s.length(); i++, ss++)
            if (*ss < 32 || *ss > 126) {
                _tag = 0;
                return;
            } else
                _tag = (_tag << 8) | *ss;
        for (int i = s.length(); i < 4; i++)
            _tag = (_tag << 8) | 0x20;
    }
}

bool
Tag::valid() const
{
    uint32_t tag = _tag;
    for (int i = 0; i < 4; i++, tag >>= 8)
        if ((tag & 255) < 32 || (tag & 255) > 126)
            return false;
    return true;
}

String
Tag::text() const
{
    StringAccum sa;
    uint32_t tag = _tag;
    for (int i = 0; i < 4; i++, tag = (tag << 8) | 0x20)
        if (tag != 0x20202020) {
            uint8_t c = (tag >> 24) & 255;
            if (c < 32 || c > 126)
                sa.snprintf(6, "\\%03o", c);
            else
                sa << c;
        }
    return sa.take_string();
}

String
Tag::langsys_text(Tag script, Tag langsys)
{
    if (!langsys.null())
        return script.text() + "." + langsys.text();
    else
        return script.text();
}

const uint8_t *
Tag::table_entry(const uint8_t *table, int n, int entry_size) const
{
    assert(((uintptr_t)table & 1) == 0);
    int l = 0, r = n;
    while (l < r) {
        int m = l + (r - l) / 2;
        const uint8_t *entry = table + m * entry_size;
        uint32_t m_tag = Data::u32_aligned16(entry);
        if (_tag < m_tag)
            r = m;
        else if (_tag == m_tag)
            return entry;
        else
            l = m + 1;
    }
    return 0;
}



/**************************
 * ScriptList             *
 *                        *
 **************************/

int
ScriptList::assign(const String &str, ErrorHandler *errh)
{
    _str = str;
    _str.align(4);
    int result = check_header(errh ? errh : ErrorHandler::silent_handler());
    if (result < 0)
        _str = String();
    return result;
}

int
ScriptList::check_header(ErrorHandler *errh)
{
    // HEADER FORMAT:
    // USHORT   scriptCount
    // 6bytes   scriptRecord[]
    int scriptCount;
    if (_str.length() < SCRIPTLIST_HEADERSIZE
        || (scriptCount = Data::u16_aligned(_str.udata()),
            _str.length() < SCRIPTLIST_HEADERSIZE + scriptCount*SCRIPT_RECSIZE))
        return errh->error("OTF ScriptList too short");

    // XXX check that scripts are sorted

    return 0;
}

int
ScriptList::script_offset(Tag script) const
{
    if (_str.length() == 0)
        return -1;
    const uint8_t *data = _str.udata();
    if (const uint8_t *entry = script.table_entry(data + SCRIPTLIST_HEADERSIZE, Data::u16_aligned(data), SCRIPT_RECSIZE))
        return Data::u16_aligned(entry + 4);
    else
        return 0;
}

int
ScriptList::check_script(Tag tag, int script_off, ErrorHandler *errh) const
{
    const uint8_t *data = _str.udata();
    int langSysCount;
    if (_str.length() < script_off + SCRIPT_HEADERSIZE
        || (langSysCount = Data::u16_aligned(data + script_off + 2),
            (_str.length() < script_off + SCRIPT_HEADERSIZE + langSysCount*LANGSYS_RECSIZE)))
        return (errh ? errh->error("OTF Script table for %<%s%> out of range", tag.text().c_str()) : -1);
    // XXX check that langsys are sorted
    return 0;
}

int
ScriptList::langsys_offset(Tag script, Tag langsys, ErrorHandler *errh) const
{
    int script_off = script_offset(script);
    if (script_off == 0) {
        script = Tag("DFLT");
        script_off = script_offset(script);
    }
    if (script_off <= 0)
        return script_off;

    // check script bounds
    if (check_script(script, script_off, errh) < 0)
        return -1;

    // search script table
    const uint8_t *data = _str.udata();
    int langSysCount = Data::u16_aligned(data + script_off + 2);
    if (const uint8_t *entry = langsys.table_entry(data + script_off + SCRIPT_HEADERSIZE, langSysCount, LANGSYS_RECSIZE))
        return script_off + Data::u16_aligned(entry + 4);

    // return default
    int defaultLangSys = Data::u16_aligned(data + script_off);
    if (defaultLangSys != 0)
        return script_off + defaultLangSys;
    else
        return 0;
}

int
ScriptList::language_systems(Vector<Tag> &script, Vector<Tag> &langsys, ErrorHandler *errh) const
{
    script.clear();
    langsys.clear();

    const uint8_t *data = _str.udata();
    int nscripts = Data::u16_aligned(data);
    for (int i = 0; i < nscripts; i++) {
        Tag script_tag(Data::u32_aligned16(data + SCRIPTLIST_HEADERSIZE + i*SCRIPT_RECSIZE));
        int script_off = Data::u16_aligned(data + SCRIPTLIST_HEADERSIZE + i*SCRIPT_RECSIZE + 4);
        if (check_script(script_tag, script_off, errh) < 0)
            return -1;
        const uint8_t *script_table = data + script_off;
        if (Data::u16_aligned(script_table) != 0) // default LangSys
            script.push_back(script_tag), langsys.push_back(Tag());
        int nlangsys = Data::u16_aligned(script_table + 2);
        for (int j = 0; j < nlangsys; j++) {
            Tag langsys_tag(Data::u32_aligned16(script_table + SCRIPT_HEADERSIZE + j*LANGSYS_RECSIZE));
            script.push_back(script_tag), langsys.push_back(langsys_tag);
        }
    }

    return 0;
}

int
ScriptList::features(Tag script, Tag langsys, int &required_fid, Vector<int> &fids, ErrorHandler *errh, bool clear_fids) const
{
    required_fid = -1;
    if (clear_fids)
        fids.clear();

    int offset = langsys_offset(script, langsys);
    if (offset <= 0)
        return offset;

    // check langsys bounds
    const uint8_t *data = _str.udata();
    int featureCount;
    if (_str.length() < offset + LANGSYS_HEADERSIZE
        || (featureCount = Data::u16_aligned(data + offset + 4),
            (_str.length() < offset + LANGSYS_HEADERSIZE + featureCount*FEATURE_RECSIZE)))
        return (errh ? errh->error("OTF LangSys table for %<%s/%s%> out of range", script.text().c_str(), langsys.text().c_str()) : -1);

    // search langsys table
    int f = Data::u16_aligned(data + offset + 2);
    if (f != 0xFFFF)
        required_fid = f;
    data += offset + 6;
    for (int i = 0; i < featureCount; i++, data += FEATURE_RECSIZE)
        fids.push_back(Data::u16_aligned(data));

    return 0;
}



/**************************
 * FeatureList            *
 *                        *
 **************************/

int
FeatureList::assign(const String &str, ErrorHandler *errh)
{
    _str = str;
    _str.align(2);
    int result = check_header(errh ? errh : ErrorHandler::silent_handler());
    if (result < 0)
        _str = String();
    return result;
}

int
FeatureList::check_header(ErrorHandler *errh)
{
    int featureCount;
    if (_str.length() < FEATURELIST_HEADERSIZE
        || (featureCount = Data::u16_aligned(_str.udata()),
            _str.length() < FEATURELIST_HEADERSIZE + featureCount*FEATURE_RECSIZE))
        return errh->error("OTF FeatureList too short");
    return 0;
}

Tag
FeatureList::tag(int fid) const
{
    if (_str.length() == 0)
        return Tag();
    const uint8_t *data = _str.udata();
    int nfeatures = Data::u16_aligned(data);
    if (fid >= 0 && fid < nfeatures)
        return Tag(Data::u32_aligned16(data + FEATURELIST_HEADERSIZE + fid*FEATURE_RECSIZE));
    else
        return Tag();
}

String
FeatureList::params(int fid, int length, ErrorHandler *errh, bool old_style_offset) const
{
    if (_str.length() == 0 || length < 0)
        return String();
    if (errh == 0)
        errh = ErrorHandler::silent_handler();

    const uint8_t *data = _str.udata();
    int len = _str.length();
    int nfeatures = Data::u16_aligned(data);
    if (fid < 0 || fid >= nfeatures)
        return errh->error("OTF feature ID %<%d%> out of range", fid), String();
    int foff = Data::u16_aligned(data + FEATURELIST_HEADERSIZE + fid*FEATURE_RECSIZE + 4);
    if (len < foff + FEATURE_HEADERSIZE)
        return errh->error("OTF LookupList for feature ID %<%d%> too short", fid), String();
    int poff = Data::u16_aligned(data + foff);
    if (poff == 0)
        return String();
    if (!old_style_offset)
        poff += foff;
    if (len < poff + length)
        return errh->error("OTF feature parameters for feature ID %<%d%> out of range", fid), String();
    else
        return _str.substring(poff, length);
}

String
FeatureList::size_params(int fid, const Name &name, ErrorHandler *errh) const
{
    // implement 'size' checks from Read Roberts
    for (int i = 0; i < 2; i++) {
        String s = params(fid, 10, errh, i != 0);
        const uint8_t *data = s.udata();
        // errh->message("trying %d %d %d %d %d\n", Data::u16_aligned(data), Data::u16_aligned(data + 2), Data::u16_aligned(data + 4), Data::u16_aligned(data + 6), Data::u16_aligned(data + 8));
        if (Data::u16_aligned(data) == 0)               // design size == 0
            continue;
        if (Data::u16_aligned(data + 2) == 0            // subfamily ID == 0
            && Data::u16_aligned(data + 6) == 0         // range start == 0
            && Data::u16_aligned(data + 8) == 0         // range end == 0
            && Data::u16_aligned(data + 4) == 0)        // menu name ID == 0
            return s;
        if (Data::u16_aligned(data + 6) >= Data::u16_aligned(data + 8) // range start >= range end
            || Data::u16_aligned(data + 4) < 256        // menu name ID < 256
            || Data::u16_aligned(data + 4) > 32767      // menu name ID > 32767
            || !name.english_name(Data::u16_aligned(data + 4))) // menu name ID is a name ID defined by the font
            continue;
        if (Data::u16_aligned(data) + 1 >= Data::u16_aligned(data + 6) // design size >= range start (with 1 dp grace)
            && Data::u16_aligned(data) <= Data::u16_aligned(data + 8) + 1) // design size <= range end (with 1 dp grace)
            return s;
        else if (i == 1                         // old-style feature
                 && Data::u16_aligned(data + 8) <= 1440 // range end <= 144 point
                 && Data::u16_aligned(data) <= 1440) {  // design size <= 144 point
            // some old fonts define a bogus feature with design size
            // not in range (John Owens, Read Roberts)
            if (errh)
                errh->warning("invalid 'size' feature: design size not in range");
            return s;
        }
    }
    if (errh)
        errh->error("no valid 'size' feature data in the 'size' feature");
    return String();
}

int
FeatureList::find(Tag tag, const Vector<int> &fids) const
{
    if (fids.size() == 0 || _str.length() == 0)
        return -1;

    const uint8_t *data = _str.udata();
    int nfeatures = Data::u16_aligned(data);
    for (const int *fidp = fids.begin(); fidp != fids.end(); fidp++)
        if (*fidp >= 0 && *fidp < nfeatures) {
            uint32_t ftag = Data::u32_aligned16(data + FEATURELIST_HEADERSIZE + (*fidp)*FEATURE_RECSIZE);
            if (ftag == tag.value())
                return *fidp;
        }

    return -1;
}

void
FeatureList::filter(Vector<int> &fids, const Vector<Tag> &sorted_ftags) const
{
    if (_str.length() == 0)
        fids.clear();
    else {
        std::sort(fids.begin(), fids.end()); // sort fids

        int i = 0, j = 0;
        while (i < fids.size() && fids[i] < 0)
            fids[i++] = 0x7FFFFFFF;

        // XXX check that feature list is in alphabetical order

        const uint8_t *data = _str.udata();
        int nfeatures = Data::u16_aligned(data);
        while (i < fids.size() && j < sorted_ftags.size() && fids[i] < nfeatures) {
            uint32_t ftag = Data::u32_aligned16(data + FEATURELIST_HEADERSIZE + fids[i]*FEATURE_RECSIZE);
            if (ftag < sorted_ftags[j].value()) { // not an interesting feature
                // replace featureID with a large number, remove later
                fids[i] = 0x7FFFFFFF;
                i++;
            } else if (ftag == sorted_ftags[j].value()) // interesting feature
                i++;
            else                // an interesting feature is not available
                j++;
        }

        fids.resize(i);         // remove remaining uninteresting features
        std::sort(fids.begin(), fids.end()); // resort, to move bad ones last
        while (fids.size() && fids.back() == 0x7FFFFFFF)
            fids.pop_back();
    }
}

int
FeatureList::lookups(int fid, Vector<int> &results, ErrorHandler *errh, bool clear_results) const
{
    if (clear_results)
        results.clear();
    if (_str.length() == 0)
        return -1;
    if (errh == 0)
        errh = ErrorHandler::silent_handler();

    const uint8_t *data = _str.udata();
    int len = _str.length();
    int nfeatures = Data::u16_aligned(data);
    if (fid < 0 || fid >= nfeatures)
        return errh->error("OTF feature ID %<%d%> out of range", fid);
    int foff = Data::u16_aligned(data + FEATURELIST_HEADERSIZE + fid*FEATURE_RECSIZE + 4);
    int lookupCount;
    if (len < foff + FEATURE_HEADERSIZE
        || (lookupCount = Data::u16_aligned(data + foff + 2),
            len < foff + FEATURE_HEADERSIZE + lookupCount*LOOKUPLIST_RECSIZE))
        return errh->error("OTF LookupList for feature ID %<%d%> too short", fid);
    const uint8_t *ldata = data + foff + FEATURE_HEADERSIZE;
    for (int j = 0; j < lookupCount; j++, ldata += LOOKUPLIST_RECSIZE)
        results.push_back(Data::u16_aligned(ldata));

    return 0;
}

int
FeatureList::lookups(const Vector<int> &fids, Vector<int> &results, ErrorHandler *errh) const
{
    results.clear();
    if (_str.length() == 0)
        return -1;

    for (int i = 0; i < fids.size(); i++)
        if (lookups(fids[i], results, errh, false) < 0)
            return -1;

    // sort results and remove duplicates
    std::sort(results.begin(), results.end());
    int *unique_result = std::unique(results.begin(), results.end());
    results.resize(unique_result - results.begin());
    return 0;
}

int
FeatureList::lookups(const Vector<int> &required_fids, const Vector<int> &fids, const Vector<Tag> &sorted_ftags, Vector<int> &results, ErrorHandler *errh) const
{
    Vector<int> fidsx(fids);
    filter(fidsx, sorted_ftags);
    for (int i = 0; i < required_fids.size(); i++)
        fidsx.push_back(required_fids[i]);
    return lookups(fidsx, results, errh);
}

int
FeatureList::lookups(int required_fid, const Vector<int> &fids, const Vector<Tag> &sorted_ftags, Vector<int> &results, ErrorHandler *errh) const
{
    Vector<int> fidsx(fids);
    filter(fidsx, sorted_ftags);
    if (required_fid >= 0)
        fidsx.push_back(required_fid);
    return lookups(fidsx, results, errh);
}

int
FeatureList::lookups(const ScriptList &script_list, Tag script, Tag langsys, const Vector<Tag> &sorted_ftags, Vector<int> &results, ErrorHandler *errh) const
{
    int required_fid;
    Vector<int> fids;
    int result = script_list.features(script, langsys, required_fid, fids, errh);
    if (result >= 0) {
        filter(fids, sorted_ftags);
        if (required_fid >= 0)
            fids.push_back(required_fid);
        result = lookups(fids, results, errh);
    }
    return result;
}


/**************************
 * Coverage               *
 *                        *
 **************************/

Coverage::Coverage() noexcept
{
}

Coverage::Coverage(Glyph first, Glyph last) noexcept
{
    if (first <= last) {
        _str = String("\000\002\000\001\000\000\000\000\000\000", 10);
        uint8_t *data = _str.mutable_udata();
        data[4] = first >> 8;
        data[5] = first & 255;
        data[6] = last >> 8;
        data[7] = last & 255;
        _str.align(2);
    }
}

Coverage::Coverage(const Vector<bool> &gmap) noexcept
{
    int end = gmap.size();
    while (end > 0 && !gmap[end - 1])
        --end;
    if (end > 0) {
        _str = String::make_uninitialized(8 + end);
        _str.align(4);
        uint8_t *data = _str.mutable_udata();
        memset(data, 0, 8 + end);
        data[1] = 3;

        uint32_t n = 0;
        const bool *it = gmap.begin();
        data += 8;
        for (int i = 0; i < end; ++i, ++it, ++data)
            if (*it) {
                *data = 1;
                ++n;
            }

        n = htonl(n);
        memcpy(_str.mutable_udata() + 4, &n, 4);
    }
}

Coverage::Coverage(const String &str, ErrorHandler *errh, bool do_check) noexcept
    : _str(str)
{
    _str.align(2);
    if (do_check) {
        if (check(errh ? errh : ErrorHandler::silent_handler()) < 0)
            _str = String();
    } else {                    // check()'s shorten-string side effect
        const uint8_t *data = _str.udata();
        int count = Data::u16_aligned(data + 2);
        if (data[1] == T_LIST)
            _str = _str.substring(0, HEADERSIZE + count*LIST_RECSIZE);
        else
            _str = _str.substring(0, HEADERSIZE + count*RANGES_RECSIZE);
    }
}

int
Coverage::check(ErrorHandler *errh)
    // side effect: shorten string to cover coverage
{
    // HEADER FORMAT:
    // USHORT   coverageFormat
    // USHORT   glyphCount
    const uint8_t *data = _str.udata();
    if (_str.length() < HEADERSIZE)
        return errh->error("OTF coverage table too small");
    int coverageFormat = Data::u16_aligned(data);
    int count = Data::u16_aligned(data + 2);

    int len;
    switch (coverageFormat) {
      case T_LIST:
        len = HEADERSIZE + count*LIST_RECSIZE;
        if (_str.length() < len)
            return errh->error("OTF coverage table too short (format 1)");
        // XXX don't check sorting
        break;
      case T_RANGES:
        len = HEADERSIZE + count*RANGES_RECSIZE;
        if (_str.length() < len)
            return errh->error("OTF coverage table too short (format 2)");
        // XXX don't check sorting
        // XXX don't check startCoverageIndexes
        break;
      default:
        return errh->error("OTF coverage table has unknown format %d", coverageFormat);
    }

    _str = _str.substring(0, len);
    return 0;
}

int
Coverage::size() const noexcept
{
    if (_str.length() == 0)
        return -1;
    const uint8_t *data = _str.udata();
    if (data[1] == T_LIST)
        return (_str.length() - HEADERSIZE) / LIST_RECSIZE;
    else if (data[1] == T_RANGES) {
        data += _str.length() - RANGES_RECSIZE;
        return Data::u16_aligned(data + 4) + Data::u16_aligned(data + 2) - Data::u16_aligned(data) + 1;
    } else if (data[1] == T_X_BYTEMAP)
        return Data::u32_aligned(data + 4);
    else
        return -1;
}

int
Coverage::coverage_index(Glyph g) const noexcept
{
    if (_str.length() == 0)
        return -1;

    const uint8_t *data = _str.udata();
    int count = Data::u16_aligned(data + 2);
    if (data[1] == T_LIST) {
        int l = 0, r = count;
        data += HEADERSIZE;
        while (l < r) {
            int m = l + (r - l) / 2;
            int mval = Data::u16_aligned(data + m * LIST_RECSIZE);
            if (g < mval)
                r = m;
            else if (g == mval)
                return m;
            else
                l = m + 1;
        }
        return -1;
    } else if (data[1] == T_RANGES) {
        int l = 0, r = count;
        data += HEADERSIZE;
        while (l < r) {
            int m = l + (r - l) / 2;
            const uint8_t *rec = data + m * RANGES_RECSIZE;
            if (g < Data::u16_aligned(rec))
                r = m;
            else if (g <= Data::u16_aligned(rec + 2))
                return Data::u16_aligned(rec + 4) + g - Data::u16_aligned(rec);
            else
                l = m + 1;
        }
        return -1;
    } else if (data[1] == T_X_BYTEMAP) {
        if (g >= 0 && g < _str.length() - 8 && data[8 + g])
            return g;
        else
            return -1;
    } else
        return -1;
}

Glyph
Coverage::operator[](int cindex) const noexcept
{
    if (_str.length() == 0 || cindex < 0)
        return 0;

    const uint8_t *data = _str.udata();
    int count = Data::u16_aligned(data + 2);
    if (data[1] == T_LIST)
        return (cindex < count ? Data::u16_aligned(data + cindex * LIST_RECSIZE) : 0);
    else if (data[1] == T_RANGES) {
        int l = 0, r = count;
        data += HEADERSIZE;
        while (l < r) {
            int m = l + (r - l) / 2;
            const uint8_t *rec = data + m * RANGES_RECSIZE;
            int start_cindex = Data::u16_aligned(rec + 4);
            if (cindex < start_cindex)
                r = m;
            else if (cindex < start_cindex + Data::u16_aligned(rec + 2) - Data::u16_aligned(rec))
                return Data::u16_aligned(rec) + cindex - start_cindex;
            else
                l = m + 1;
        }
        return 0;
    } else
        return 0;
}

void
Coverage::unparse(StringAccum &sa) const noexcept
{
    const uint8_t *data = _str.udata();
    if (_str.length() == 0)
        sa << "@*#!";
    else if (data[1] == T_LIST) {
        int count = Data::u16_aligned(data + 2);
        for (int i = 0; i < count; i++) {
            if (i) sa << ',';
            sa << Data::u16_aligned(data + HEADERSIZE + i*LIST_RECSIZE);
        }
    } else {
        for (int pos = HEADERSIZE; pos < _str.length(); pos += RANGES_RECSIZE) {
            Glyph start = Data::u16_aligned(data + pos);
            Glyph end = Data::u16_aligned(data + pos + 2);
            if (pos > HEADERSIZE) sa << ',';
            sa << start;
            if (end != start) sa << '.' << '.' << end;
        }
    }
}

String
Coverage::unparse() const noexcept
{
    StringAccum sa;
    unparse(sa);
    return sa.take_string();
}

Coverage
operator&(const Coverage &a, const Coverage &b)
{
    StringAccum sa;
    sa << '\000' << '\001' << '\000' << '\000';

    if (b.has_fast_covers()) {
        for (Coverage::iterator ai = a.begin(); ai; ++ai)
            if (b.covers(*ai)) {
                uint16_t x = *ai;
                sa << (char)(x >> 8) << (char)(x & 0xFF);
            }

    } else {
        Coverage::iterator ai = a.begin(), bi = b.begin();
        while (ai && bi) {
            if (*ai < *bi)
                ai.forward_to(*bi);
            else if (*ai == *bi) {
                uint16_t x = *ai;
                sa << (char)(x >> 8) << (char)(x & 0xFF);
                ai++, bi++;
            } else
                bi.forward_to(*ai);
        }
    }

    int n = (sa.length() - 4) / 2;
    sa[2] = (n >> 8);
    sa[3] = (n & 0xFF);
    return Coverage(sa.take_string(), 0, false);
}

bool
operator<=(const Coverage &a, const Coverage &b)
{
    Coverage::iterator ai = a.begin(), bi = b.begin();
    while (ai && bi) {
        if (*ai != *bi && !bi.forward_to(*ai))
            return false;
        ai++, bi++;
    }
    return bi || !ai;
}


/************************
 * Coverage::iterator   *
 *                      *
 ************************/

Coverage::iterator::iterator(const String &str, bool is_end)
    : _str(str), _value(0)
{
    // XXX assume _str has been checked
    if (!_str.length()) {
        _pos = 0;
        return;
    }

    // shrink _str to fit the coverage table
    const uint8_t *data = _str.udata();
    int n = Data::u16_aligned(data + 2);
    switch (Data::u16_aligned(data)) {
    case T_LIST:
        _str = _str.substring(0, HEADERSIZE + n*LIST_RECSIZE);
    normal_pos_setting:
        _pos = is_end ? _str.length() : HEADERSIZE;
        _value = _pos >= _str.length() ? 0 : Data::u16_aligned(data + _pos);
        break;
    case T_RANGES:
        _str = _str.substring(0, HEADERSIZE + n*RANGES_RECSIZE);
        goto normal_pos_setting;
    case T_X_BYTEMAP:
        for (_pos = 8; _pos < _str.length() && !data[_pos]; ++_pos)
            /* do nothing */;
        _value = _pos >= _str.length() ? 0 : _pos - 8;
        break;
    default:
        _str = String();
        _pos = 0;
        _value = 0;
        break;
    }
}

int
Coverage::iterator::coverage_index() const
{
    const uint8_t *data = _str.udata();
    assert(_pos < _str.length());
    if (data[1] == T_LIST)
        return (_pos - HEADERSIZE) / LIST_RECSIZE;
    else if (data[1] == T_RANGES)
        return Data::u16_aligned(data + _pos + 4) + _value - Data::u16_aligned(data + _pos);
    else
        return _pos - 8;
}

void
Coverage::iterator::operator++(int)
{
    const uint8_t *data = _str.udata();
    int len = _str.length();
    if (_pos >= len
        || (data[1] == T_RANGES && ++_value <= Data::u16_aligned(data + _pos + 2)))
        return;
    switch (data[1]) {
    case T_LIST:
        _pos += LIST_RECSIZE;
    normal_pos_setting:
        _value = _pos >= len ? 0 : Data::u16_aligned(data + _pos);
        break;
    case T_RANGES:
        _pos += RANGES_RECSIZE;
        goto normal_pos_setting;
    case T_X_BYTEMAP:
        for (++_pos; _pos < len && !data[_pos]; ++_pos)
            /* do nothing */;
        _value = _pos >= len ? 0 : _pos - 8;
        break;
    }
}

bool
Coverage::iterator::forward_to(Glyph find)
{
    // XXX really should check that this works

    if (find <= _value)
        return find == _value;
    else if (_pos >= _str.length())
        return false;

    const uint8_t *data = _str.udata();
    if (data[1] == T_LIST) {
        // check for "common" case: next element
        _pos += LIST_RECSIZE;
        if (_pos >= _str.length())
            return false;
        else if (find <= Data::u16_aligned(data + _pos)) {
            _value = Data::u16_aligned(data + _pos);
            return find == _value;
        }

        // otherwise, binary search over remaining area
        int l = ((_pos - HEADERSIZE) / LIST_RECSIZE) + 1;
        int r = (_str.length() - HEADERSIZE) / LIST_RECSIZE;
        data += HEADERSIZE;
        while (l < r) {
            int m = l + (r - l) / 2;
            Glyph g = Data::u16_aligned(data + m * LIST_RECSIZE);
            if (find < g)
                r = m;
            else if (find == g)
                l = r = m;
            else
                l = m + 1;
        }
        _pos = HEADERSIZE + l * LIST_RECSIZE;
        _value = (_pos >= _str.length() ? 0 : Data::u16_aligned(data - HEADERSIZE + _pos));

    } else if (data[1] == T_RANGES) {
        // check for "common" case: this or next element
        if (find <= Data::u16_aligned(data + _pos + 2)) {
            assert(find >= Data::u16_aligned(data + _pos));
            _value = find;
            return true;
        }
        _pos += RANGES_RECSIZE;
        if (_pos >= _str.length())
            return false;
        else if (find <= Data::u16_aligned(data + _pos + 2)) {
            _value = (find >= Data::u16_aligned(data + _pos) ? find : Data::u16_aligned(data + _pos));
            return find == _value;
        }

        // otherwise, binary search over remaining area
        int l = ((_pos - HEADERSIZE) / RANGES_RECSIZE) + 1;
        int r = (_str.length() - HEADERSIZE) / RANGES_RECSIZE;
        data += HEADERSIZE;
        while (l < r) {
            int m = l + (r - l) / 2;
            if (find < Data::u16_aligned(data + m * RANGES_RECSIZE))
                r = m;
            else if (find <= Data::u16_aligned(data + m * RANGES_RECSIZE + 2)) {
                _pos = HEADERSIZE + m * RANGES_RECSIZE;
                _value = find;
                return true;
            } else
                l = m + 1;
        }
        _pos = HEADERSIZE + l * LIST_RECSIZE;
        _value = (_pos >= _str.length() ? 0 : Data::u16_aligned(data - HEADERSIZE + _pos));

    } else if (data[1] == T_X_BYTEMAP) {
        _pos = 8 + find;
        while (_pos < _str.length() && !data[_pos])
            ++_pos;
        _pos = _pos >= _str.length() ? _str.length() : _pos;
        _value = _pos >= _str.length() ? 0 : _pos - 8;
    }

    return find == _value;
}


/**************************
 * GlyphSet               *
 *                        *
 **************************/

GlyphSet::GlyphSet()
{
    memset(_v, 0, sizeof(_v));
}

GlyphSet::GlyphSet(const GlyphSet &o)
{
    for (int i = 0; i < VLEN; i++)
        if (o._v[i]) {
            _v[i] = new uint32_t[VULEN];
            memcpy(_v[i], o._v[i], sizeof(uint32_t) * VULEN);
        } else
            _v[i] = 0;
}

GlyphSet::~GlyphSet()
{
    for (int i = 0; i < VLEN; i++)
        delete[] _v[i];
}

int
GlyphSet::change(Glyph g, bool value)
{
    if ((unsigned)g > MAXGLYPH)
        return -1;
    uint32_t *&u = _v[g >> SHIFT];
    if (!u) {
        u = new uint32_t[VULEN];
        memset(u, 0, sizeof(uint32_t) * VULEN);
    }
    uint32_t mask = (1 << (g & 0x1F));
    if (value)
        u[(g & MASK) >> 5] |= mask;
    else
        u[(g & MASK) >> 5] &= ~mask;
    return 0;
}

GlyphSet &
GlyphSet::operator=(const GlyphSet &o)
{
    if (&o != this) {
        for (int i = 0; i < VLEN; i++)
            if (o._v[i]) {
                if (!_v[i])
                    _v[i] = new uint32_t[VULEN];
                memcpy(_v[i], o._v[i], sizeof(uint32_t) * VULEN);
            } else if (_v[i])
                memset(_v[i], 0, sizeof(uint32_t) * VULEN);
    }
    return *this;
}


/**************************
 * ClassDef               *
 *                        *
 **************************/

ClassDef::ClassDef(const String &str, ErrorHandler *errh) noexcept
    : _str(str)
{
    _str.align(2);
    if (check(errh ? errh : ErrorHandler::silent_handler()) < 0)
        _str = String();
}

int
ClassDef::check(ErrorHandler *errh)
{
    // HEADER FORMAT:
    // USHORT   coverageFormat
    // USHORT   glyphCount
    const uint8_t *data = _str.udata();
    if (_str.length() < 6)      // NB: prevents empty format-2 tables
        return errh->error("OTF class def table too small");
    int classFormat = Data::u16_aligned(data);

    int len;
    if (classFormat == T_LIST) {
        int count = Data::u16_aligned(data + 4);
        len = LIST_HEADERSIZE + count*LIST_RECSIZE;
        // XXX don't check sorting
    } else if (classFormat == T_RANGES) {
        int count = Data::u16_aligned(data + 2);
        len = RANGES_HEADERSIZE + count*RANGES_RECSIZE;
        // XXX don't check sorting
    } else
        return errh->error("OTF class def table has unknown format %d", classFormat);

    if (len > _str.length())
        return errh->error("OTF class def table too short");
    else {
        _str = _str.substring(0, len);
        return 0;
    }
}

int
ClassDef::lookup(Glyph g) const noexcept
{
    if (_str.length() == 0)
        return -1;

    const uint8_t *data = _str.udata();
    int coverageFormat = Data::u16_aligned(data);

    if (coverageFormat == T_LIST) {
        Glyph start = Data::u16_aligned(data + 2);
        int count = Data::u16_aligned(data + 4);
        if (g < start || g >= start + count)
            return 0;
        else
            return Data::u16_aligned(data + LIST_HEADERSIZE + (g - start) * LIST_RECSIZE);
    } else if (coverageFormat == T_RANGES) {
        int l = 0, r = Data::u16_aligned(data + 2);
        data += RANGES_HEADERSIZE;
        while (l < r) {
            int m = l + (r - l) / 2;
            const uint8_t *rec = data + m * RANGES_RECSIZE;
            if (g < Data::u16_aligned(rec))
                r = m;
            else if (g <= Data::u16_aligned(rec + 2))
                return Data::u16_aligned(rec + 4);
            else
                l = m + 1;
        }
        return 0;
    } else
        return 0;
}

void
ClassDef::unparse(StringAccum &sa) const noexcept
{
    const uint8_t *data = _str.udata();
    if (_str.length() == 0)
        sa << "@*#!";
    else if (data[1] == T_LIST) {
        Glyph start = Data::u16_aligned(data + 2);
        int count = Data::u16_aligned(data + 4);
        for (int i = 0; i < count; i++) {
            if (i) sa << ',';
            sa << start + i << '=' << Data::u16_aligned(data + LIST_HEADERSIZE + i*LIST_RECSIZE);
        }
    } else {
        for (int pos = RANGES_HEADERSIZE; pos < _str.length(); pos += RANGES_RECSIZE) {
            Glyph start = Data::u16_aligned(data + pos);
            Glyph end = Data::u16_aligned(data + pos + 2);
            if (pos > RANGES_HEADERSIZE) sa << ',';
            sa << start;
            if (end != start) sa << '.' << '.' << end;
            sa << '=' << Data::u16_aligned(data + pos + 4);
        }
    }
}

String
ClassDef::unparse() const noexcept
{
    StringAccum sa;
    unparse(sa);
    return sa.take_string();
}


/******************************
 * ClassDef::class_iterator   *
 *                            *
 ******************************/

ClassDef::class_iterator::class_iterator(const String &str, int pos, int the_class, const Coverage::iterator &coviter)
    : _str(str), _pos(pos), _class(the_class), _coviter(coviter)
{
    // XXX assume _str has been checked

    // cannot handle the_class == 0 when coviter doesn't exist
    if (_class == 0 && !_coviter)
        throw Error("cannot iterate over ClassDef class 0");

    // shrink _str to fit the coverage table, and create a fake coverage
    // iterator if necessary
    const uint8_t *data = _str.udata();
    if (_str.length()) {
        switch (Data::u16_aligned(data)) {
          case T_LIST: {
              Glyph start = Data::u16_aligned(data + 2);
              int nglyphs = Data::u16_aligned(data + 4);
              _str = _str.substring(0, LIST_HEADERSIZE + nglyphs*LIST_RECSIZE);
              if (!_coviter)
                  _coviter = Coverage(start, start + nglyphs - 1).begin();
              if (_class)
                  _coviter.forward_to(start);
              break;
          }
          case T_RANGES: {
              Glyph start = Data::u16_aligned(data + RANGES_HEADERSIZE);
              int nranges = Data::u16_aligned(data + 2);
              _str = _str.substring(0, RANGES_HEADERSIZE + nranges*RANGES_RECSIZE);
              if (!_coviter) {
                  Glyph end = Data::u16_aligned(data + RANGES_HEADERSIZE + 2 + (nranges - 1)*RANGES_RECSIZE);
                  _coviter = Coverage(start, end).begin();
              }
              if (_class)
                  _coviter.forward_to(start);
              break;
          }
          default:
            _str = String();
            break;
        }
    }

    // move to the first relevant glyph
    if (_pos >= _str.length())
        _pos = _str.length();
    else {
        _pos = 0;
        (*this)++;
    }
}

void
ClassDef::class_iterator::increment_class0()
{
    const uint8_t *data = _str.udata();
    int len = _str.length();
    bool is_list = (data[1] == T_LIST);

    if (_pos != 0)
        _coviter++;
    else
        _pos = FIRST_POS;

    if (_pos == FIRST_POS && _coviter) {
        if (*_coviter < Data::u16_aligned(data + (is_list ? 2 : RANGES_HEADERSIZE)))
            return;
        _pos = (is_list ? LIST_HEADERSIZE : RANGES_HEADERSIZE);
    }

    while (_pos > 0 && _pos < len && _coviter) {
        Glyph g = *_coviter;
        if (is_list) {
            _pos = LIST_HEADERSIZE + LIST_RECSIZE*(g - Data::u16_aligned(data + 2));
            if (_pos >= len)
                break;
            else if (Data::u16_aligned(data + _pos) == 0) // _class == 0
                return;
            _coviter++;
        } else {
            if (g < Data::u16_aligned(data + _pos)) // in a zero range
                return;
            else if (g > Data::u16_aligned(data + _pos + 2))
                _pos += RANGES_RECSIZE;
            else if (Data::u16_aligned(data + _pos + 4) == 0) // _class == 0
                return;
            else
                _coviter.forward_to(Data::u16_aligned(data + _pos + 2) + 1);
        }
    }

    if (_coviter)
        _pos = LAST_POS;
    else
        _pos = len;
}

void
ClassDef::class_iterator::operator++(int)
{
    if (_class == 0) {
        increment_class0();
        return;
    }

    const uint8_t *data = _str.udata();
    int len = _str.length();
    bool is_list = (data[1] == T_LIST);

    if (_pos != 0)
        _coviter++;
    else
        _pos = (is_list ? LIST_HEADERSIZE : RANGES_HEADERSIZE);

    while (_pos < len && _coviter) {
        Glyph g = *_coviter;
        if (is_list) {
            _pos = LIST_HEADERSIZE + LIST_RECSIZE*(g - Data::u16_aligned(data + 2));
            if (_pos >= len || Data::u16_aligned(data + _pos) == _class)
                return;
            _coviter++;
        } else {
            while (_pos < len && (g > Data::u16_aligned(data + _pos + 2)
                                  || Data::u16_aligned(data + _pos + 4) != _class))
                _pos += RANGES_RECSIZE;
            // now, _pos >= len, or g <= rec.end && class == rec.class
            if (_pos >= len || g >= Data::u16_aligned(data + _pos))
                return;
            _coviter.forward_to(Data::u16_aligned(data + _pos));
        }
    }

    _pos = len;
}


}}


// template instantiations
#include <lcdf/vector.cc>
