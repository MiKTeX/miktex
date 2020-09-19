// -*- related-file-name: "../../libefont/otfgpos.cc" -*-
#ifndef EFONT_OTFGPOS_HH
#define EFONT_OTFGPOS_HH
#include <efont/otf.hh>
#include <efont/otfdata.hh>
namespace Efont { namespace OpenType {
class GposLookup;
class Positioning;

class Gpos { public:

    Gpos(const Data &, ErrorHandler * = 0);
    // default destructor

    const ScriptList &script_list() const { return _script_list; }
    const FeatureList &feature_list() const { return _feature_list; }

    int nlookups() const;
    GposLookup lookup(unsigned) const;

    enum { HEADERSIZE = 10 };

  private:

    ScriptList _script_list;
    FeatureList _feature_list;
    Data _lookup_list;

};

class GposLookup { public:
    GposLookup(const Data &);
    int type() const                    { return _type; }
    uint16_t flags() const              { return _d.u16(2); }
    bool unparse_automatics(Vector<Positioning> &, ErrorHandler * = 0) const;
    enum {
        HEADERSIZE = 6, RECSIZE = 2,
        L_SINGLE = 1, L_PAIR = 2, L_CURSIVE = 3, L_MARKTOBASE = 4,
        L_MARKTOLIGATURE = 5, L_MARKTOMARK = 6, L_CONTEXT = 7,
        L_CHAIN = 8, L_EXTENSION = 9
    };
  private:
    Data _d;
    int _type;
    Data subtable(int i) const;
};

class GposValue { public:
    static inline int size(uint16_t format);
    static inline int16_t xplacement(uint16_t format, const Data &);
    static inline int16_t yplacement(uint16_t format, const Data &);
    static inline int16_t xadvance(uint16_t format, const Data &);
    static inline int16_t yadvance(uint16_t format, const Data &);
    enum {
        F_XPLACEMENT = 0x0001,
        F_YPLACEMENT = 0x0002,
        F_XADVANCE = 0x0004,
        F_YADVANCE = 0x0008,
        F_XPLACEMENT_DEVICE = 0x0010,
        F_YPLACEMENT_DEVICE = 0x0020,
        F_XADVANCE_DEVICE = 0x0040,
        F_YADVANCE_DEVICE = 0x0080
    };
  private:
    static const int nibble_bitcount_x2[];
};

class GposSingle { public:
    GposSingle(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    void unparse(Vector<Positioning> &) const;
    enum { F2_HEADERSIZE = 8 };
  private:
    Data _d;
};

class GposPair { public:
    GposPair(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    void unparse(Vector<Positioning> &) const;
    enum { F1_HEADERSIZE = 10, F1_RECSIZE = 2,
           PAIRSET_HEADERSIZE = 2, PAIRVALUE_HEADERSIZE = 2,
           F2_HEADERSIZE = 16 };
  private:
    Data _d;
};

struct Position {
    Glyph g;
    int pdx, pdy;               // placement
    int adx, ady;               // advance
    inline Position();
    inline Position(Glyph, uint16_t format, const Data &);
    inline Position(uint16_t format, const Data &);
    inline Position(Glyph, const Position &);
    inline Position(Glyph g, int pdx, int pdy, int adx, int ady);
    bool empty() const          { return pdx == 0 && pdy == 0 && adx == 0 && ady == 0; }
    operator bool() const       { return !empty(); }
    bool h_empty() const        { return pdx == 0 && pdy == 0 && adx == 0; }
    bool placed() const         { return pdx != 0 || pdy != 0; }
    void unparse(StringAccum &, const Vector<PermString> * = 0) const;
    String unparse(const Vector<PermString> * = 0) const;
};

class Positioning { public:

    Positioning();

    // single positioning
    inline Positioning(const Position &);

    // pair positioning
    inline Positioning(const Position &, const Position &);

    bool context_in(const Coverage &) const;
    bool context_in(const GlyphSet &) const;

    // types
    inline operator bool() const;
    inline bool is_single() const;
    inline bool is_pair() const;
    inline bool is_pairkern() const;

    // extract data
    const Position &left() const        { return _left; }
    Glyph left_glyph() const            { return _left.g; }
    const Position &right() const       { return _right; }
    Glyph right_glyph() const           { return _right.g; }
    inline void all_in_glyphs(Vector<Glyph> &gs) const;

    void unparse(StringAccum &, const Vector<PermString> * = 0) const;
    String unparse(const Vector<PermString> * = 0) const;

  private:

    Position _left;
    Position _right;

};

inline int GposValue::size(uint16_t format)
{
    return (nibble_bitcount_x2[format & 15] + nibble_bitcount_x2[(format>>4) & 15]);
}

inline int16_t GposValue::xplacement(uint16_t format, const Data &d)
{
    if (format & F_XPLACEMENT)
        return d.s16(0);
    else
        return 0;
}

inline int16_t GposValue::yplacement(uint16_t format, const Data &d)
{
    if (format & F_YPLACEMENT)
        return d.s16((format & F_XPLACEMENT ? 2 : 0));
    else
        return 0;
}

inline int16_t GposValue::xadvance(uint16_t format, const Data &d)
{
    if (format & F_XADVANCE)
        return d.s16(nibble_bitcount_x2[format & (F_XADVANCE - 1)]);
    else
        return 0;
}

inline int16_t GposValue::yadvance(uint16_t format, const Data &d)
{
    if (format & F_YADVANCE)
        return d.s16(nibble_bitcount_x2[format & (F_YADVANCE - 1)]);
    else
        return 0;
}

inline Position::Position()
    : g(0)
{
}

inline Position::Position(Glyph g_, uint16_t format, const Data &value)
    : g(g_),
      pdx(GposValue::xplacement(format, value)), pdy(GposValue::yplacement(format, value)),
      adx(GposValue::xadvance(format, value)), ady(GposValue::yadvance(format, value))
{
}

inline Position::Position(uint16_t format, const Data &value)
    : g(0),
      pdx(GposValue::xplacement(format, value)), pdy(GposValue::yplacement(format, value)),
      adx(GposValue::xadvance(format, value)), ady(GposValue::yadvance(format, value))
{
}

inline Position::Position(Glyph g_, const Position &p)
    : g(g_), pdx(p.pdx), pdy(p.pdy), adx(p.adx), ady(p.ady)
{
}

inline Position::Position(Glyph g_, int pdx_, int pdy_, int adx_, int ady_)
    : g(g_), pdx(pdx_), pdy(pdy_), adx(adx_), ady(ady_)
{
}

inline Positioning::Positioning(const Position &left)
    : _left(left)
{
}

inline Positioning::Positioning(const Position &left, const Position &right)
    : _left(left), _right(right)
{
}

inline Positioning::operator bool() const
{
    return _left.g != 0;
}

inline bool Positioning::is_single() const
{
    return _left.g != 0 && _right.g == 0;
}

inline bool Positioning::is_pair() const
{
    return _left.g != 0 && _right.g != 0;
}

inline bool Positioning::is_pairkern() const
{
    return _left.g != 0 && !_left.placed() && _right.g != 0 && _right.h_empty();
}

inline void Positioning::all_in_glyphs(Vector<Glyph> &gs) const
{
    gs.clear();
    if (_left.g != 0)
        gs.push_back(_left.g);
    if (_right.g != 0)
        gs.push_back(_right.g);
}

}}
#endif
