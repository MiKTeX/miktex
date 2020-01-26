#ifndef OTFTOTFM_GLYPHFILTER_HH
#define OTFTOTFM_GLYPHFILTER_HH
#include <efont/otf.hh>
#include <lcdf/vector.hh>
class Metrics;

class GlyphFilter { public:

    GlyphFilter()                       : _sorted(true) { }

    operator bool() const               { return _patterns.size() != 0; }

    inline bool allow_substitution(Efont::OpenType::Glyph glyph, const Vector<PermString>& glyph_names, uint32_t unicode) const;
    inline bool allow_alternate(Efont::OpenType::Glyph glyph, const Vector<PermString>& glyph_names, uint32_t unicode) const;

    void add_substitution_filter(const String&, bool is_exclude, ErrorHandler*);
    void add_alternate_filter(const String&, bool is_exclude, ErrorHandler*);

    friend bool operator==(const GlyphFilter&, const GlyphFilter&);
    inline bool check_eq(GlyphFilter&); // may alter both GlyphFilters

    GlyphFilter& operator+=(const GlyphFilter&);

    void unparse(StringAccum&) const;

    struct Pattern {
        uint16_t type;
        uint16_t data;
        union {
            struct {
                int mask;
                int value;
            } uniprop;
            struct {
                uint32_t low;
                uint32_t high;
            } unirange;
        } u;
        String pattern;
        Pattern(uint16_t type);
        static int compare(const Pattern&, const Pattern&);
    };

  private:

    enum { T_EXCLUDE = 1, T_NEGATE = 2, T_TYPEMASK = 3,
           T_SRC = 0, T_DST = 4 };
    enum { D_NAME, D_UNIPROP, D_UNIRANGE };

    Vector<Pattern> _patterns;
    bool _sorted;

    bool allow(Efont::OpenType::Glyph glyph, const Vector<PermString>& glyph_names, uint32_t unicode, int ptype) const;
    void add_pattern(const String&, int ptype, ErrorHandler*);
    void sort();

};

inline bool
GlyphFilter::allow_substitution(Efont::OpenType::Glyph glyph, const Vector<PermString>& glyph_names, uint32_t unicode) const
{
    return (!_patterns.size() || allow(glyph, glyph_names, unicode, T_SRC));
}

inline bool
GlyphFilter::allow_alternate(Efont::OpenType::Glyph glyph, const Vector<PermString>& glyph_names, uint32_t unicode) const
{
    return (!_patterns.size() || allow(glyph, glyph_names, unicode, T_DST));
}

inline bool operator==(const GlyphFilter::Pattern& a, const GlyphFilter::Pattern& b)
{
    return a.type == b.type && a.data == b.data && a.u.unirange.low == b.u.unirange.low && a.u.unirange.high == b.u.unirange.high && a.pattern == b.pattern;
}

inline bool operator<(const GlyphFilter::Pattern& a, const GlyphFilter::Pattern& b)
{
    return GlyphFilter::Pattern::compare(a, b) < 0;
}

inline bool operator!=(const GlyphFilter::Pattern& a, const GlyphFilter::Pattern& b)
{
    return !(a == b);
}

bool operator==(const GlyphFilter&, const GlyphFilter&);

inline bool operator!=(const GlyphFilter& a, const GlyphFilter& b)
{
    return !(a == b);
}

inline bool
GlyphFilter::check_eq(GlyphFilter& o)
{
    sort();
    o.sort();
    return *this == o;
}

GlyphFilter operator+(const GlyphFilter&, const GlyphFilter&);

inline StringAccum&
operator<<(StringAccum& sa, const GlyphFilter& gf)
{
    gf.unparse(sa);
    return sa;
}

#endif
