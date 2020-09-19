// -*- related-file-name: "../../libefont/t1font.cc" -*-
#ifndef EFONT_T1FONT_HH
#define EFONT_T1FONT_HH
#include <efont/t1cs.hh>
#include <lcdf/hashmap.hh>
class StringAccum;
class ErrorHandler;
namespace Efont {
class Type1Item;
class Type1Definition;
class Type1Encoding;
class Type1IncludedFont;
class Type1Subr;
class Type1Reader;
class Type1Writer;
class MultipleMasterSpace;

class Type1Font : public CharstringProgram { public:

    Type1Font(Type1Reader &);
    ~Type1Font();

    int read(Type1Reader &);

    bool ok() const;

    inline PermString font_name() const;
    void font_matrix(double[6]) const;

    int nitems() const                  { return _items.size(); }
    Type1Item *item(int i) const        { return _items[i]; }
    void add_item(Type1Item *it)        { _items.push_back(it); }
    void add_definition(int dict, Type1Definition *);
    void add_type1_encoding(Type1Encoding *);

    int nsubrs() const                  { return _subrs.size(); }
    Type1Charstring *subr(int) const;

    int nglyphs() const                 { return _glyphs.size(); }
    PermString glyph_name(int) const;
    Type1Charstring *glyph(int) const;
    Type1Charstring *glyph(PermString) const;
    void add_glyph(Type1Subr *);

    Type1Subr *subr_x(int i) const      { return _subrs[i]; }
    bool set_subr(int, const Type1Charstring &, PermString definer = PermString());
    bool remove_subr(int);
    void fill_in_subrs();
    void renumber_subrs(const Vector<int> &); // dangerous!

    Type1Subr *glyph_x(int i) const     { return _glyphs[i]; }

    Type1Encoding *type1_encoding() const { return _encoding; }

    // note: the order is relevant
    enum Dict {
        dFont = 0,              dF = dFont,
        dFontInfo = 1,          dFI = dFontInfo,
        dPrivate = 2,           dP = dPrivate,
        dBlend = 3,             dB = dBlend,
        dBlendFontInfo = dB+dFI, dBFI = dBlendFontInfo,
        dBlendPrivate = dB+dP,  dBP = dBlendPrivate,
        dLast
    };

    Type1Definition *dict(int d, PermString s) const { return _dict[d][s]; }
    Type1Definition *dict(PermString s) const   { return _dict[dF][s]; }
    Type1Definition *p_dict(PermString s) const { return _dict[dP][s]; }
    Type1Definition *b_dict(PermString s) const { return _dict[dB][s]; }
    Type1Definition *bp_dict(PermString s) const { return _dict[dBP][s];}
    Type1Definition *fi_dict(PermString s) const { return _dict[dFI][s];}

    typedef HashMap<PermString, Type1Definition *> DictHashMap;
    inline DictHashMap::const_iterator dict_begin(int dict) const;
    inline DictHashMap::iterator dict_begin(int dict);
    int first_dict_item(int d) const            { return _index[d]; }

    Type1Definition *ensure(Dict, PermString);
    void add_header_comment(const String &);

    MultipleMasterSpace *create_mmspace(ErrorHandler * = 0) const;
    MultipleMasterSpace *mmspace() const;

    void undo_synthetic();

    void set_charstring_definer(PermString d)   { _charstring_definer = d; }

    void write(Type1Writer &);

    // font skeletons
    static Type1Font *skeleton_make(PermString, const String &version = String());
    static Type1Font *skeleton_make_copy(const Type1Font *, PermString, const Vector<double> *xuid_extension);
    void skeleton_comments_end();
    void skeleton_fontinfo_end();
    void skeleton_fontdict_end();
    void skeleton_private_end();
    void skeleton_common_subrs();

    // for experts only
    void set_item(int, Type1Item *);
    inline void set_dict(int dict, PermString, Type1Definition *);
    void uncache_defs();

  private:

    mutable bool _cached_defs;
    bool _built;
    mutable PermString _font_name;

    Vector<Type1Item *> _items;

    HashMap<PermString, Type1Definition *> *_dict;
    int _index[dLast];
    int _dict_deltas[dLast];

    Vector<Type1Subr *> _subrs;
    Vector<Type1Subr *> _glyphs;
    HashMap<PermString, int> _glyph_map;

    PermString _charstring_definer;
    Type1Encoding *_encoding;

    mutable bool _cached_mmspace;
    mutable MultipleMasterSpace *_mmspace;

    Type1IncludedFont *_synthetic_item;

    Type1Font(PermString);
    Type1Font(const Type1Font &);
    Type1Font &operator=(const Type1Font &);

    void read_encoding(Type1Reader &, const char *);
    bool read_synthetic_font(Type1Reader &, const char *, StringAccum &);
    void cache_defs() const;
    void shift_indices(int, int);

    Type1Item *dict_size_item(int) const;
    int get_dict_size(int) const;
    void set_dict_size(int, int);

};


inline Type1Font::DictHashMap::const_iterator Type1Font::dict_begin(int dict) const
{
    return _dict[dict].begin();
}

inline Type1Font::DictHashMap::iterator Type1Font::dict_begin(int dict)
{
    return _dict[dict].begin();
}

inline void Type1Font::set_dict(int dict, PermString name, Type1Definition *t1d)
{
    _dict[dict].insert(name, t1d);
}

inline PermString Type1Font::font_name() const
{
    if (!_cached_defs)
        cache_defs();
    return _font_name;
}

}
#endif
