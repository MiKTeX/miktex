#ifndef OTFTOTFM_METRICS_HH
#define OTFTOTFM_METRICS_HH
#include <efont/otfgsub.hh>
#include <efont/otfgpos.hh>
#include "setting.hh"
namespace Efont { class CharstringProgram; }
class DvipsEncoding;
class GlyphFilter;

class Metrics { public:

    typedef int Code;
    typedef Efont::OpenType::Glyph Glyph;
    enum { VIRTUAL_GLYPH = 0x10000 };

    typedef Efont::OpenType::Substitution Substitution;
    typedef Efont::OpenType::Positioning Positioning;

    Metrics(const Efont::CharstringProgram *, int nglyphs);
    ~Metrics();

    void check() const;

    Glyph boundary_glyph() const        { return _boundary_glyph; }
    Glyph emptyslot_glyph() const       { return _emptyslot_glyph; }

    String coding_scheme() const                { return _coding_scheme; }
    void set_coding_scheme(const String &s)     { _coding_scheme = s; }

    int design_units() const                    { return _design_units; }
    int units_per_em() const                    { return _units_per_em; }
    void set_design_units(int du)               { _design_units = du; }

    int n_mapped_fonts() const                  { return _mapped_fonts.size();}
    const Efont::CharstringProgram *mapped_font(int i) const { return _mapped_fonts[i]; }
    const String &mapped_font_name(int i) const { return _mapped_font_names[i]; }
    int add_mapped_font(const Efont::CharstringProgram *, const String &);

    inline int encoding_size() const            { return _encoding.size(); }
    inline bool valid_code(Code) const;
    inline bool nonvirtual_code(Code) const;
    PermString code_name(Code) const;
    inline const char *code_str(Code) const;

    inline Glyph glyph(Code code) const;
    inline uint32_t unicode(Code code) const;
    inline Code encoding(Glyph g, Code after) const;
    Code unicode_encoding(uint32_t uni) const;
    Code force_encoding(Glyph g, int lookup_source = -1);
    void encode(Code code, uint32_t uni, Glyph g);
    void encode_virtual(Code, PermString, uint32_t uni, const Vector<Setting> &, bool base_char);

    void add_altselector_code(Code, int altselector_type);
    bool altselectors() const           { return _altselectors.size() > 0; }

    inline bool was_base_glyph(Code) const;
    inline Code base_code(Code) const;
    inline Glyph base_glyph(Code) const;
    bool base_glyphs(Vector<Glyph> &, int size) const;

    void add_ligature(Code in1, Code in2, Code out);
    Code pair_code(Code, Code, int lookup_source = -1);
    void add_kern(Code in1, Code in2, int kern);
    void set_kern(Code in1, Code in2, int kern);
    void add_single_positioning(Code, int pdx, int pdy, int adx);

    enum { CODE_ALL = 0x7FFFFFFF };
    void remove_ligatures(Code in1, Code in2);
    int reencode_right_ligkern(Code old_in2, Code new_in2);

    int apply(const Vector<Substitution>&, bool allow_single, int lookup, const GlyphFilter&, const Vector<PermString>& glyph_names);
    void apply_alternates(const Vector<Substitution>&, int lookup, const GlyphFilter&, const Vector<PermString>& glyph_names);
    int apply(const Vector<Positioning>&);

    void apply_base_encoding(const String &font_name, const DvipsEncoding &, const Vector<int> &mapping);

    void cut_encoding(int size);
    void shrink_encoding(int size, const DvipsEncoding &, ErrorHandler *);
    void make_base(int size);

    bool need_virtual(int size) const;
    bool need_base();
    enum SettingMode { SET_NONE = 0, SET_KEEP = 1, SET_INTERMEDIATE = 3 };
    bool setting(Code, Vector<Setting> &, SettingMode = SET_NONE) const;
    int ligatures(Code in1, Vector<Code> &in2, Vector<Code> &out, Vector<int> &context) const;
    int kerns(Code in1, Vector<Code> &in2, Vector<int> &kern) const;
    int kern(Code in1, Code in2) const;

    void unparse() const;

    struct Ligature {
        Code in2;
        Code out;
        Ligature(Code in2_, Code out_) : in2(in2_), out(out_) { }
    };

    struct Kern {
        Code in2;
        int kern;
        Kern(Code in2_, int kern_) : in2(in2_), kern(kern_) { }
    };

    struct VirtualChar {
        PermString name;
        Vector<Setting> setting;
    };

    struct Ligature3 {
        Code in1;
        Code in2;
        Code out;
        Ligature3(Code in1_, Code in2_, Code out_) : in1(in1_), in2(in2_), out(out_) { }
        String unparse(const Metrics& m) const;
    };

  private:

    struct Char {
        Glyph glyph;
        Code base_code;
        uint32_t unicode;
        Vector<Ligature> ligatures;
        Vector<Kern> kerns;
        VirtualChar *virtual_char;
        int pdx;
        int pdy;
        int adx;
        Code built_in1;
        Code built_in2;
        int lookup_source;
        enum { BUILT = 1, INTERMEDIATE = 2, CONTEXT_ONLY = 4, LIVE = 8,
               BASE_LIVE = 16, BASE_REP = 32, IS_FF = 64 };
        int flags;

        Char()                          : virtual_char(0) { clear(); }
        void clear();
        void swap(Char &);
        bool visible() const            { return glyph != 0; }
        bool visible_base() const       { return glyph != 0 && glyph != VIRTUAL_GLYPH; }
        bool flag(int f) const          { return (flags & f) != 0; }
        inline bool base_glyph() const;
        bool context_setting(Code in1, Code in2) const;
    };

    Vector<Char> _encoding;
    mutable Vector<int> _emap;

    Glyph _boundary_glyph;
    Glyph _emptyslot_glyph;

    Vector<Kern> _altselectors;

    String _coding_scheme;
    int _design_units;
    int _units_per_em;

    bool _liveness_marked : 1;

    Vector<const Efont::CharstringProgram *> _mapped_fonts;
    Vector<String> _mapped_font_names;

    Metrics(const Metrics &);   // does not exist
    Metrics &operator=(const Metrics &); // does not exist

    inline void assign_emap(Glyph, Code);
    Code hard_encoding(Glyph, Code) const;
    bool next_encoding(Vector<Code> &codes, const Vector<Glyph> &glyphs) const;

    Ligature *ligature_obj(Code, Code);
    Kern *kern_obj(Code, Code);
    inline void new_ligature(Code, Code, Code);
    inline void repoint_ligature(Code, Ligature *, Code);

    friend bool operator<(const Ligature3 &, const Ligature3 &);
    void all_ligatures(Vector<Ligature3> &) const;
    void mark_liveness(int size, const Vector<Ligature3> * = 0);
    void reencode(const Vector<Code> &);

    class ChangedContext;
    void apply_ligature(const Vector<Code> &, const Substitution *, int lookup);
    void apply_single(Code cin, const Substitution *s, int lookup,
                ChangedContext &ctx, const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names);
    void apply_simple_context_ligature(const Vector<Code> &codes,
                const Substitution *s, int lookup, ChangedContext &ctx,
                const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names);
    void apply_alternates_single(Code cin, const Substitution *s, int lookup,
                const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names);
    void apply_alternates_ligature(const Vector<Code> &codes,
                const Substitution *s, int lookup,
                const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names);

    void unparse(const Char *) const;

};


inline bool
Metrics::valid_code(Code code) const
{
    return code >= 0 && code < _encoding.size();
}

inline bool
Metrics::nonvirtual_code(Code code) const
{
    return code >= 0 && code < _encoding.size() && !_encoding[code].virtual_char;
}

inline Metrics::Glyph
Metrics::glyph(Code code) const
{
    if (code < 0 || code >= _encoding.size())
        return 0;
    else
        return _encoding[code].glyph;
}

inline uint32_t
Metrics::unicode(Code code) const
{
    if (code < 0 || code >= _encoding.size())
        return 0;
    else
        return _encoding[code].unicode;
}

inline Metrics::Glyph
Metrics::base_glyph(Code code) const
{
    if (code < 0 || code >= _encoding.size() || _encoding[code].base_code < 0)
        return 0;
    else
        return _encoding[code].glyph;
}

inline Metrics::Code
Metrics::base_code(Code code) const
{
    if (code < 0 || code >= _encoding.size())
        return 0;
    else
        return _encoding[code].base_code;
}

inline Metrics::Code
Metrics::encoding(Glyph g, Code after) const
{
    Code c;
    if (g >= 0 && g < _emap.size() && (c = _emap.at_u(g)) >= -1)
        return c < 0 || c >= after ? c : -1;
    else
        return hard_encoding(g, after);
}

inline void
Metrics::assign_emap(Glyph g, Code code)
{
    if (g >= _emap.size())
        _emap.resize(g + 1, -1);
    _emap[g] = (_emap[g] == -1 || _emap[g] == code ? code : -2);
}

inline const char *
Metrics::code_str(Code code) const
{
    return code_name(code).c_str();
}

inline bool
Metrics::Char::base_glyph() const
{
    return glyph == VIRTUAL_GLYPH ? flags & BASE_REP : glyph != 0;
}

inline bool
Metrics::was_base_glyph(Code code) const
{
    if (code < 0 || code >= _encoding.size())
        return 0;
    else
        return _encoding[code].base_glyph();
}

#endif
