// -*- related-file-name: "../../libefont/cff.cc" -*-
#ifndef EFONT_CFF_HH
#define EFONT_CFF_HH
#include <lcdf/hashmap.hh>
#include <efont/t1cs.hh>
class ErrorHandler;
namespace Efont {
class Type1Encoding;

class Cff { public:

    class Dict;
    class IndexIterator;
    class Charset;
    class FDSelect;
    class FontParent;
    class Font;
    class CIDFont;
    class ChildFont;

    explicit Cff(const String& str, unsigned units_per_em,
                 ErrorHandler* errh = 0);
    ~Cff();

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    const String &data_string() const   { return _data_string; }
    const uint8_t *data() const         { return _data; }
    int length() const                  { return _len; }

    int nfonts() const                  { return _name_index.size(); }
    PermString font_name(int i) const   { return _name_index[i]; }
    FontParent* font(PermString = PermString(), ErrorHandler* = 0);

    enum { NSTANDARD_STRINGS = 391, MAX_SID = 64999 };
    int max_sid() const                 { return NSTANDARD_STRINGS - 1 + _strings.size(); }
    int sid(PermString);
    String sid_string(int sid) const;
    PermString sid_permstring(int sid) const;

    int ngsubrs() const                 { return _gsubrs_index.nitems(); }
    Charstring *gsubr(int i);

    unsigned units_per_em() const       { return _units_per_em; }

    enum DictOperator {
        oVersion = 0, oNotice = 1, oFullName = 2, oFamilyName = 3,
        oWeight = 4, oFontBBox = 5, oBlueValues = 6, oOtherBlues = 7,
        oFamilyBlues = 8, oFamilyOtherBlues = 9, oStdHW = 10, oStdVW = 11,
        oUniqueID = 13, oXUID = 14, oCharset = 15, oEncoding = 16,
        oCharStrings = 17, oPrivate = 18, oSubrs = 19, oDefaultWidthX = 20,
        oNominalWidthX = 21,
        oCopyright = 32 + 0, oIsFixedPitch = 32 + 1, oItalicAngle = 32 + 2,
        oUnderlinePosition = 32 + 3, oUnderlineThickness = 32 + 4,
        oPaintType = 32 + 5, oCharstringType = 32 + 6, oFontMatrix = 32 + 7,
        oStrokeWidth = 32 + 8, oBlueScale = 32 + 9, oBlueShift = 32 + 10,
        oBlueFuzz = 32 + 11, oStemSnapH = 32 + 12, oStemSnapV = 32 + 13,
        oForceBold = 32 + 14, oLanguageGroup = 32 + 17,
        oExpansionFactor = 32 + 18, oInitialRandomSeed = 32 + 19,
        oSyntheticBase = 32 + 20, oPostScript = 32 + 21,
        oBaseFontName = 32 + 22, oBaseFontBlend = 32 + 23,
        oROS = 32 + 30, oCIDFontVersion = 32 + 31, oCIDFontRevision = 32 + 32,
        oCIDFontType = 32 + 33, oCIDCount = 32 + 34, oUIDBase = 32 + 35,
        oFDArray = 32 + 36, oFDSelect = 32 + 37, oFontName = 32 + 38,
        oLastOperator = oFontName
    };

    enum DictType {
        tNone = 0, tSID, tFontNumber, tBoolean, tNumber, tOffset, tLocalOffset,
        tArray, tArray2, tArray3, tArray4, tArray5, tArray6, tPrivateType,
        tTypeMask = 0x7F, tPrivate = 0x80, tP = tPrivate
    };

    static const char * const operator_names[];
    static const int operator_types[];


    class IndexIterator { public:

        IndexIterator()         : _offset(0), _last_offset(0), _offsize(-1) { }
        IndexIterator(const uint8_t *, int, int, ErrorHandler * = 0, const char *index_name = "INDEX");

        int error() const       { return (_offsize < 0 ? _offsize : 0); }

        typedef bool (IndexIterator::*unspecified_bool_type)() const;
        bool live() const       { return _offset < _last_offset; }
        operator unspecified_bool_type() const {
            return live() ? &IndexIterator::live : 0;
        }
        int nitems() const;

        inline const uint8_t *operator*() const;
        inline const uint8_t *operator[](int) const;
        const uint8_t *index_end() const;

        void operator++()       { _offset += _offsize; }
        void operator++(int)    { ++(*this); }

      private:

        const uint8_t *_contents;
        const uint8_t *_offset;
        const uint8_t *_last_offset;
        int _offsize;

        inline uint32_t offset_at(const uint8_t *) const;

    };

  private:

    String _data_string;
    const uint8_t *_data;
    int _len;

    int _error;

    Vector<PermString> _name_index;

    IndexIterator _top_dict_index;

    IndexIterator _strings_index;
    mutable Vector<PermString> _strings;
    mutable HashMap<PermString, int> _strings_map;

    IndexIterator _gsubrs_index;
    Vector<Charstring*> _gsubrs_cs;
    Vector<FontParent*> _fonts;

    unsigned _units_per_em;

    int parse_header(ErrorHandler *);

    enum { HEADER_SIZE = 4 };

};


class Cff::Dict { public:

    Dict();
    Dict(Cff *, int pos, int dict_len, ErrorHandler * = 0, const char *dict_name = "DICT");
    int assign(Cff *, int pos, int dict_len, ErrorHandler * = 0, const char *dict_name = "DICT");

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    int check(bool is_private, ErrorHandler * = 0, const char *dict_name = "DICT") const;

    bool has(DictOperator) const;
    inline bool has_first(DictOperator) const;
    bool xvalue(DictOperator, Vector<double> &) const;
    bool xvalue(DictOperator, int *) const;
    bool xvalue(DictOperator, double *) const;
    bool value(DictOperator, Vector<double> &) const;
    bool value(DictOperator, int *) const;
    bool value(DictOperator, double *) const;

    void unparse(ErrorHandler *, const char *) const;

  private:

    Cff *_cff;
    int _pos;
    Vector<int> _operators;
    Vector<int> _pointers;
    Vector<double> _operands;
    int _error;

};

class Cff::Charset { public:

    Charset()                           : _error(-1) { }
    Charset(const Cff *, int pos, int nglyphs, int max_sid, ErrorHandler * = 0);
    void assign(const Cff *, int pos, int nglyphs, int max_sid, ErrorHandler * = 0);

    int error() const                   { return _error; }

    int nglyphs() const                 { return _sids.size(); }
    int nsids() const                   { return _gids.size(); }

    inline int gid_to_sid(int gid) const;
    inline int sid_to_gid(int sid) const;

  private:

    Vector<int> _sids;
    Vector<int> _gids;
    int _error;

    void assign(const int *, int, int);
    int parse(const Cff *, int pos, int nglyphs, int max_sid, ErrorHandler *);

};

class Cff::FDSelect { public:

    FDSelect()                          : _fds(0), _my_fds(false), _nglyphs(0), _error(-1) { }
    ~FDSelect();
    void assign(const Cff *, int pos, int nglyphs, ErrorHandler * = 0);

    int error() const                   { return _error; }

    int nglyphs() const                 { return _nglyphs; }
    inline int gid_to_fd(int gid) const;

  private:

    const uint8_t *_fds;
    bool _my_fds;
    int _nglyphs;
    int _error;

    FDSelect(const FDSelect &);
    FDSelect &operator=(const FDSelect &);
    int parse(const Cff *, int pos, int nglyphs, ErrorHandler *);

};


class Cff::FontParent : public CharstringProgram { public:

    FontParent(Cff* cff);

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    int ngsubrs_x() const               { return _cff->ngsubrs(); }
    int ngsubrs() const                 { return ngsubrs_x(); }
    Charstring *gsubr(int) const;
    int gsubr_bias() const;

  private:

    Cff* _cff;
    int _charstring_type;
    int _error;
    int _font_index;

    FontParent(const FontParent &);
    FontParent &operator=(const FontParent &);

    Charstring *charstring(const IndexIterator &, int) const;

    friend class Cff;
    friend class Cff::Font;
    friend class Cff::CIDFont;
    friend class Cff::ChildFont;

};

class Cff::CIDFont : public Cff::FontParent { public:

    CIDFont(Cff* cff, PermString, const Dict &, ErrorHandler *);
    ~CIDFont();

    PermString font_name() const        { return _font_name; }
    void font_matrix(double[6]) const;

    int nglyphs() const                 { return _charstrings_index.nitems(); }
    PermString glyph_name(int) const;
    void glyph_names(Vector<PermString> &) const;
    Charstring *glyph(int) const;
    Charstring *glyph(PermString) const;
    int glyphid(PermString) const;
    const CharstringProgram *child_program(int) const;

    bool dict_has(DictOperator) const;
    String dict_string(DictOperator) const;
    bool dict_value(DictOperator, double *) const;
    bool dict_xvalue(DictOperator, double, double *) const;
    bool dict_value(DictOperator, Vector<double> &) const;

  private:

    PermString _font_name;

    Dict _top_dict;
    Dict _private_dict;

    Cff::Charset _charset;

    IndexIterator _charstrings_index;
    mutable Vector<Charstring *> _charstrings_cs;

    Vector<ChildFont *> _child_fonts;
    Cff::FDSelect _fdselect;

    const Dict &dict_of(DictOperator) const;

};

class Cff::ChildFont : public Cff::FontParent { public:

    ChildFont(Cff* cff, Cff::CIDFont *, int charstring_type, const Dict &, ErrorHandler * = 0);
    ~ChildFont();

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    PermString font_name() const        { return _parent->font_name(); }
    void font_matrix(double m[6]) const { _parent->font_matrix(m); }

    inline bool cid() const;

    int nsubrs_x() const                { return _subrs_index.nitems(); }
    int nsubrs() const                  { return nsubrs_x(); }
    Charstring *subr(int) const;
    int subr_bias() const;

    int nglyphs() const                 { return _parent->nglyphs(); }
    PermString glyph_name(int gi) const { return _parent->glyph_name(gi); }
    void glyph_names(Vector<PermString> &v) const { _parent->glyph_names(v); }
    Charstring *glyph(int gi) const     { return _parent->glyph(gi); }
    Charstring *glyph(PermString n) const { return _parent->glyph(n); }

    double global_width_x(bool) const;

  private:

    Cff::CIDFont *_parent;

    Dict _top_dict;
    Dict _private_dict;

    IndexIterator _subrs_index;
    mutable Vector<Charstring *> _subrs_cs;

    double _default_width_x;
    double _nominal_width_x;

    ChildFont(const ChildFont &); // does not exist
    ChildFont &operator=(const ChildFont &); // does not exist

    Charstring *charstring(const IndexIterator &, int) const;

    friend class Cff::Font;

};

class Cff::Font : public Cff::ChildFont { public:

    Font(Cff* cff, PermString, const Dict &, ErrorHandler *);
    ~Font();

    PermString font_name() const        { return _font_name; }
    void font_matrix(double[6]) const;

    int nglyphs() const                 { return _charstrings_index.nitems(); }
    PermString glyph_name(int) const;
    void glyph_names(Vector<PermString> &) const;
    Charstring *glyph(int) const;
    Charstring *glyph(PermString) const;
    int glyphid(PermString) const;

    Type1Encoding *type1_encoding() const;
    Type1Encoding *type1_encoding_copy() const;

    bool dict_has(DictOperator) const;
    String dict_string(DictOperator) const;
    inline bool dict_value(DictOperator, double *) const;
    inline bool dict_value(DictOperator, Vector<double> &) const;
    const Dict &top_dict() const        { return _top_dict; }

  private:

    PermString _font_name;

    Cff::Charset _charset;

    IndexIterator _charstrings_index;
    mutable Vector<Charstring *> _charstrings_cs;

    int _encoding_pos;
    int _encoding[256];
    mutable Type1Encoding *_t1encoding;

    int parse_encoding(int pos, ErrorHandler *);
    int assign_standard_encoding(const int *standard_encoding);

    inline const Dict &dict_of(DictOperator) const;

};


inline uint32_t Cff::IndexIterator::offset_at(const uint8_t *x) const
{
    switch (_offsize) {
      case 0:
        return 0;
      case 1:
        return x[0];
      case 2:
        return (x[0] << 8) | x[1];
      case 3:
        return (x[0] << 16) | (x[1] << 8) | x[2];
      default:
        return (x[0] << 24) | (x[1] << 16) | (x[2] << 8) | x[3];
    }
}

inline const uint8_t *Cff::IndexIterator::operator*() const
{
    assert(live());
    return _contents + offset_at(_offset);
}

inline const uint8_t *Cff::IndexIterator::operator[](int which) const
{
    assert(live() && _offset + which * _offsize <= _last_offset);
    return _contents + offset_at(_offset + which * _offsize);
}

inline int Cff::Charset::gid_to_sid(int gid) const
{
    if (gid >= 0 && gid < _sids.size())
        return _sids[gid];
    else
        return -1;
}

inline int Cff::Charset::sid_to_gid(int sid) const
{
    if (sid >= 0 && sid < _gids.size())
        return _gids[sid];
    else
        return -1;
}

inline int Cff::FDSelect::gid_to_fd(int gid) const
{
    if (gid >= 0 && gid < _nglyphs)
        return _fds[gid];
    else
        return -1;
}

inline bool Cff::Dict::has_first(DictOperator op) const
{
    return _operators.size() && _operators[0] == op;
}

inline const Cff::Dict &Cff::Font::dict_of(DictOperator op) const
{
    return (op >= 0 && op <= oLastOperator && (operator_types[op] & tP) ? _private_dict : _top_dict);
}

inline bool Cff::Font::dict_value(DictOperator op, double *val) const
{
    return dict_of(op).value(op, val);
}

inline bool Cff::Font::dict_value(DictOperator op, Vector<double> &val) const
{
    return dict_of(op).value(op, val);
}

}
#endif
