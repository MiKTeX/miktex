// -*- related-file-name: "../../libefont/otfgsub.cc" -*-
#ifndef EFONT_OTFGSUB_HH
#define EFONT_OTFGSUB_HH
#include <efont/otf.hh>
#include <efont/otfdata.hh>
namespace Efont { namespace OpenType {
class GsubLookup;
class Substitution;

class Gsub { public:

    Gsub(const Data &, const Font *, ErrorHandler * = 0);
    // default destructor

    const ScriptList &script_list() const { return _script_list; }
    const FeatureList &feature_list() const { return _feature_list; }
    bool chaincontext_reverse_backtrack() const { return _chaincontext_reverse_backtrack; }

    int nlookups() const;
    GsubLookup lookup(unsigned) const;

    enum { HEADERSIZE = 10 };

  private:

    ScriptList _script_list;
    FeatureList _feature_list;
    Data _lookup_list;
    bool _chaincontext_reverse_backtrack;

};

class GsubLookup { public:
    GsubLookup(const Data &);
    int type() const                    { return _type; }
    uint16_t flags() const              { return _d.u16(2); }
    void mark_out_glyphs(const Gsub &gsub, Vector<bool> &gmap) const;
    bool unparse_automatics(const Gsub &gsub, Vector<Substitution> &subs, const Coverage &limit) const;
    bool apply(const Glyph *, int pos, int n, Substitution &) const;
    enum {
        HEADERSIZE = 6, RECSIZE = 2,
        L_SINGLE = 1, L_MULTIPLE = 2, L_ALTERNATE = 3, L_LIGATURE = 4,
        L_CONTEXT = 5, L_CHAIN = 6, L_EXTENSION = 7, L_REVCHAIN = 8
    };
  private:
    Data _d;
    int _type;
    Data subtable(int i) const;
};

class GsubSingle { public:
    GsubSingle(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    Glyph map(Glyph) const;
    void mark_out_glyphs(Vector<bool> &gmap) const;
    void unparse(Vector<Substitution> &subs, const Coverage &limit) const;
    bool apply(const Glyph *, int pos, int n, Substitution &) const;
    enum { HEADERSIZE = 6, FORMAT2_RECSIZE = 2 };
  private:
    Data _d;
};

class GsubMultiple { public:
    GsubMultiple(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    bool map(Glyph, Vector<Glyph> &) const;
    void mark_out_glyphs(Vector<bool> &gmap) const;
    void unparse(Vector<Substitution> &, bool alternate = false) const;
    bool apply(const Glyph *, int pos, int n, Substitution &, bool alternate = false) const;
    enum { HEADERSIZE = 6, RECSIZE = 2,
           SEQ_HEADERSIZE = 2, SEQ_RECSIZE = 2 };
  private:
    Data _d;
};

class GsubLigature { public:
    GsubLigature(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    bool map(const Vector<Glyph> &, Glyph &, int &) const;
    void mark_out_glyphs(Vector<bool> &gmap) const;
    void unparse(Vector<Substitution> &) const;
    bool apply(const Glyph *, int pos, int n, Substitution &) const;
    enum { HEADERSIZE = 6, RECSIZE = 2,
           SET_HEADERSIZE = 2, SET_RECSIZE = 2,
           LIG_HEADERSIZE = 4, LIG_RECSIZE = 2 };
  private:
    Data _d;
};

class GsubContext { public:
    GsubContext(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    void mark_out_glyphs(const Gsub &gsub, Vector<bool> &gmap) const;
    bool unparse(const Gsub &gsub, Vector<Substitution> &out_subs, const Coverage &limit) const;
    enum { F3_HSIZE = 6, SUBRECSIZE = 4 };
  private:
    Data _d;
    static void subruleset_mark_out_glyphs(const Data &data, int nsub, int subtab_offset, const Gsub &gsub, Vector<bool> &gmap);
    static bool f1_unparse(const Data& data,
                           int nsub, int subtab_offset,
                           const Gsub& gsub, Vector<Substitution>& outsubs,
                           Substitution prototype_sub);
    static bool f3_unparse(const Data &data,
                           int nglyph, int glyphtab_offset, const Coverage &limit,
                           int nsub, int subtab_offset,
                           const Gsub &gsub, Vector<Substitution> &outsubs,
                           const Substitution &prototype_sub);
    friend class GsubChainContext;
};

class GsubChainContext { public:
    GsubChainContext(const Data &);
    // default destructor
    Coverage coverage() const noexcept;
    void mark_out_glyphs(const Gsub &gsub, Vector<bool> &gmap) const;
    bool unparse(const Gsub &gsub, Vector<Substitution> &subs, const Coverage &limit) const;
    enum { F1_HEADERSIZE = 6, F1_RECSIZE = 2,
           F1_SRS_HSIZE = 2, F1_SRS_RSIZE = 2,
           F3_HSIZE = 4, F3_INPUT_HSIZE = 2, F3_LOOKAHEAD_HSIZE = 2, F3_SUBST_HSIZE = 2 };
  private:
    Data _d;
    bool f1_unparse(const Gsub &gsub, Vector<Substitution> &subs, const Coverage &limit) const;
    bool f3_unparse(const Gsub &gsub, Vector<Substitution> &subs, const Coverage &limit) const;
};

class Substitution { public:

    Substitution();
    Substitution(const Substitution &);

    // single substitution
    Substitution(Glyph in, Glyph out);

    // multiple substitution
    Substitution(Glyph in, const Vector<Glyph> &out, bool is_alternate = false);

    // ligature substitution
    Substitution(Glyph in1, Glyph in2, Glyph out);
    Substitution(const Vector<Glyph> &in, Glyph out);
    Substitution(int nin, const Glyph *in, Glyph out);

    // space
    Substitution(int nleft, int nin, int nout, int nright);

    ~Substitution();

    Substitution &operator=(const Substitution &);

    bool context_in(const Coverage &) const;
    bool context_in(const GlyphSet &) const;

    // types
    inline operator bool() const;
    bool is_noop() const;
    inline bool is_single() const;
    inline bool is_multiple() const;
    inline bool is_alternate() const;
    inline bool is_ligature() const;
    inline bool is_simple_context() const;
    inline bool is_single_lcontext() const;
    inline bool is_single_rcontext() const;
    inline bool is_lcontext() const;
    inline bool is_rcontext() const;

    // extract data
    inline Glyph left_glyph() const;
    inline int left_nglyphs() const;
    inline Glyph *left_glyphptr();
    inline const Glyph *left_glyphptr() const;

    inline Glyph in_glyph() const;
    inline Glyph in_glyph(int pos) const;
    inline bool in_glyphs(Vector<Glyph> &) const;
    inline int in_nglyphs() const;
    inline Glyph *in_glyphptr();
    inline const Glyph *in_glyphptr() const;

    inline bool in_matches(int pos, Glyph) const;

    inline Glyph out_glyph() const;
    inline Glyph out_glyph(int pos) const;
    inline bool out_glyphs(Vector<Glyph> &) const;
    inline Glyph *out_glyphptr();
    inline const Glyph *out_glyphptr() const;

    inline int out_nglyphs() const;
    inline Glyph right_glyph() const;
    inline Glyph *right_glyphptr();
    inline const Glyph *right_glyphptr() const;

    bool all_in_glyphs(Vector<Glyph> &gs) const;
    bool all_out_glyphs(Vector<Glyph> &gs) const;

    // alter
    void add_outer_left(Glyph);
    void remove_outer_left();
    Substitution in_out_append_glyph(Glyph) const;
    bool out_alter(const Substitution &, int) noexcept;
    void add_outer_right(Glyph);
    void remove_outer_right();

    void unparse(StringAccum &, const Vector<PermString> * = &debug_glyph_names) const;
    String unparse(const Vector<PermString> * = &debug_glyph_names) const;

  private:

    enum { T_NONE = 0, T_GLYPH, T_GLYPHS, T_COVERAGE };
    typedef union {
        Glyph gid;
        Glyph *gids;    // first entry is a count
        Coverage *coverage;
    } Substitute;

    Substitute _left;
    Substitute _in;
    Substitute _out;
    Substitute _right;

    uint8_t _left_is;
    uint8_t _in_is;
    uint8_t _out_is;
    uint8_t _right_is;

    bool _alternate : 1;

    static void clear(Substitute &, uint8_t &);
    static void assign_space(Substitute &, uint8_t &, int);
    static void assign(Substitute &, uint8_t &, Glyph);
    static void assign(Substitute &, uint8_t &, int, const Glyph *);
    static void assign(Substitute &, uint8_t &, const Coverage &);
    static void assign(Substitute &, uint8_t &, const Substitute &, uint8_t);
    static void assign_append(Substitute &, uint8_t &, const Substitute &, uint8_t, const Substitute &, uint8_t);
    static void assign_append(Substitute &, uint8_t &, const Substitute &, uint8_t, Glyph);
    static bool substitute_in(const Substitute &, uint8_t, const Coverage &);
    static bool substitute_in(const Substitute &, uint8_t, const GlyphSet &);

    static Glyph extract_glyph(const Substitute &, uint8_t) noexcept;
    static Glyph extract_glyph(const Substitute &, int which, uint8_t) noexcept;
    static bool extract_glyphs(const Substitute &, uint8_t, Vector<Glyph> &, bool coverage_ok) noexcept;
    static Glyph *extract_glyphptr(const Substitute &, uint8_t) noexcept;
    static int extract_nglyphs(const Substitute &, uint8_t, bool coverage_ok) noexcept;
    static bool matches(const Substitute &, uint8_t, int pos, Glyph) noexcept;

    static void unparse_glyphids(StringAccum &, const Substitute &, uint8_t, const Vector<PermString> *) noexcept;

};

inline Substitution::Substitution()
    : _left_is(T_NONE), _in_is(T_NONE), _out_is(T_NONE), _right_is(T_NONE)
{
}

/* Single 1: u16 format, offset coverage, u16 glyphdelta
   Single 2: u16 format, offset coverage, u16 count, glyph subst[]
   Multiple 1: u16 format, offset coverage, u16 count, offset sequence[];
     sequence is: u16 count, glyph subst[]
   Alternate 1: u16 format, offset coverage, u16 count, offset alternates[];
     alternate is: u16 count, glyph alts[]
   Ligature 1: u16 format, offset coverage, u16 count, offset sets[];
     set is: u16 count, offset ligatures[];
     ligature is: glyph result, u16 count, glyph components[]
*/

inline Substitution::operator bool() const
{
    return !(_left_is == T_NONE && _in_is == T_NONE && _out_is == T_NONE && _right_is == T_NONE);
}

inline bool Substitution::is_single() const
{
    return _left_is == T_NONE && _in_is == T_GLYPH && _out_is == T_GLYPH && _right_is == T_NONE;
}

inline bool Substitution::is_multiple() const
{
    return _left_is == T_NONE && _in_is == T_GLYPH && _out_is == T_GLYPHS && _right_is == T_NONE && !_alternate;
}

inline bool Substitution::is_alternate() const
{
    return _left_is == T_NONE && _in_is == T_GLYPH && _out_is == T_GLYPHS && _right_is == T_NONE && _alternate;
}

inline bool Substitution::is_ligature() const
{
    return _left_is == T_NONE && _in_is == T_GLYPHS && _out_is == T_GLYPH && _right_is == T_NONE;
}

inline bool Substitution::is_simple_context() const
{
    return _left_is != T_COVERAGE && (_in_is == T_GLYPH || _in_is == T_GLYPHS) && (_out_is == T_GLYPH || _out_is == T_GLYPHS) && _right_is != T_COVERAGE;
}

inline bool Substitution::is_single_lcontext() const
{
    return _left_is == T_GLYPH && _in_is == T_GLYPH && _out_is == T_GLYPH && _right_is == T_NONE;
}

inline bool Substitution::is_single_rcontext() const
{
    return _left_is == T_NONE && _in_is == T_GLYPH && _out_is == T_GLYPH && _right_is == T_GLYPH;
}

inline Glyph Substitution::left_glyph() const
{
    return extract_glyph(_left, _left_is);
}

inline int Substitution::left_nglyphs() const
{
    return extract_nglyphs(_left, _left_is, false);
}

inline Glyph Substitution::in_glyph() const
{
    return extract_glyph(_in, _in_is);
}

inline Glyph Substitution::in_glyph(int which) const
{
    return extract_glyph(_in, which, _in_is);
}

inline bool Substitution::in_glyphs(Vector<Glyph> &v) const
{
    return extract_glyphs(_in, _in_is, v, true);
}

inline int Substitution::in_nglyphs() const
{
    return extract_nglyphs(_in, _in_is, true);
}

inline bool Substitution::in_matches(int pos, Glyph g) const
{
    return matches(_in, _in_is, pos, g);
}

inline Glyph Substitution::out_glyph() const
{
    return extract_glyph(_out, _out_is);
}

inline Glyph Substitution::out_glyph(int which) const
{
    return extract_glyph(_out, which, _out_is);
}

inline bool Substitution::out_glyphs(Vector<Glyph> &v) const
{
    return extract_glyphs(_out, _out_is, v, false);
}

inline int Substitution::out_nglyphs() const
{
    return extract_nglyphs(_out, _out_is, false);
}

inline Glyph Substitution::right_glyph() const
{
    return extract_glyph(_right, _right_is);
}

inline const Glyph *Substitution::left_glyphptr() const
{
    return extract_glyphptr(_left, _left_is);
}

inline Glyph *Substitution::left_glyphptr()
{
    return extract_glyphptr(_left, _left_is);
}

inline const Glyph *Substitution::in_glyphptr() const
{
    return extract_glyphptr(_in, _in_is);
}

inline Glyph *Substitution::in_glyphptr()
{
    return extract_glyphptr(_in, _in_is);
}

inline const Glyph *Substitution::out_glyphptr() const
{
    return extract_glyphptr(_out, _out_is);
}

inline Glyph *Substitution::out_glyphptr()
{
    return extract_glyphptr(_out, _out_is);
}

inline const Glyph *Substitution::right_glyphptr() const
{
    return extract_glyphptr(_right, _right_is);
}

inline Glyph *Substitution::right_glyphptr()
{
    return extract_glyphptr(_right, _right_is);
}

inline StringAccum &operator<<(StringAccum &sa, const Substitution &sub)
{
    sub.unparse(sa);
    return sa;
}

}}
#endif
