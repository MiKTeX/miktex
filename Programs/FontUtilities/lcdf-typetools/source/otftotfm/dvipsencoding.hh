#ifndef OTFTOTFM_DVIPSENCODING_HH
#define OTFTOTFM_DVIPSENCODING_HH
#include <efont/otfcmap.hh>
#include <efont/cff.hh>
#include <lcdf/hashmap.hh>
class Metrics;
class Secondary;
class FontInfo;

class DvipsEncoding { public:

    DvipsEncoding();

    static void add_glyphlist(String);

    operator bool() const                       { return _e.size() > 0; }
    const String &name() const                  { return _name; }
    const String &filename() const              { return _filename; }
    int boundary_char() const                   { return _boundary_char; }
    const String &coding_scheme() const         { return _coding_scheme; }
    void set_coding_scheme(const String &s)     { _coding_scheme = s; }
    void set_warn_missing(bool wm)              { _warn_missing = wm; }

    void encode(int, PermString);
    inline int encoding_of(PermString) const;
    int encoding_of(PermString, bool encode);
    inline bool encoded(int e) const;
    inline PermString encoding(int e) const;
    int encoding_size() const                   { return _e.size(); }

    int parse(String filename, bool ignore_ligkern, bool ignore_other, ErrorHandler *);
    int parse_ligkern(const String &ligkern_text, int override, ErrorHandler *);
    int parse_position(const String &ligkern_text, int override, ErrorHandler *);
    int parse_unicoding(const String &unicoding_text, int override, ErrorHandler *);

    bool file_had_ligkern() const               { return _file_had_ligkern; }

    // also modifies 'this':
    void make_metrics(Metrics &, const FontInfo &, Secondary *, bool literal, ErrorHandler *);
    void make_base_mappings(Vector<int> &mappings, const FontInfo &);

    void apply_ligkern_lig(Metrics &, ErrorHandler *) const;
    void apply_ligkern_kern(Metrics &, ErrorHandler *) const;
    void apply_position(Metrics &, ErrorHandler *) const;

    enum { JT_KERN = 32, JT_LIG = 64, JT_ADDLIG = 128, JT_LIGALL = 199,
           JL_LIG = JT_LIG | JT_ADDLIG, JL_CLIG = JL_LIG | 1,
           JL_CLIG_S = JL_LIG | 2, JL_LIGC = JL_LIG | 3,
           JL_LIGC_S = JL_LIG | 4, JL_CLIGC = JL_LIG | 5,
           JL_CLIGC_S = JL_LIG | 6, JL_CLIGC_SS = JL_LIG | 7,
           JT_NOLIGKERN = JT_KERN | JT_LIG,
           J_ALL = 0x7FFFFFFF }; // also see nokern_names in dvipsencoding.cc

  private:

    struct Ligature {
        int c1, c2, join, k, d;
    };

    Vector<PermString> _e;
    Vector<bool> _encoding_required;
    int _boundary_char;
    int _altselector_char;

    Vector<Ligature> _lig;
    Vector<Ligature> _pos;
    HashMap<PermString, int> _unicoding_map;
    Vector<uint32_t> _unicoding;

    mutable Vector<uint32_t> _unicodes;

    String _name;
    String _filename;
    String _printable_filename;
    String _coding_scheme;
    String _initial_comment;
    String _final_text;
    bool _file_had_ligkern;
    bool _warn_missing;

    struct WordType {
        const char *name;
        int (DvipsEncoding::*parsefunc)(Vector<String>&, int, ErrorHandler*);
    };
    static const WordType word_types[];
    enum { WT_LIGKERN = 0, WT_POSITION, WT_UNICODING };

    void add_ligkern(const Ligature &, int override);
    enum { EPARSE = 90000 };
    int parse_ligkern_words(Vector<String> &, int override, ErrorHandler *);
    int parse_position_words(Vector<String> &, int override, ErrorHandler *);
    int parse_unicoding_words(Vector<String> &, int override, ErrorHandler *);
    void parse_word_group(Vector<String> &, int override, int wt, ErrorHandler *);
    int parse_words(const String &, int override, int wt, ErrorHandler *);
    void bad_codepoint(int, Metrics &, HashMap<PermString, int> &bad_unicodes);
    bool x_unicodes(PermString chname, Vector<uint32_t> &unicodes) const;
    String landmark(int line) const;

    static PermString dot_notdef;

    static bool glyphname_unicode(String, Vector<uint32_t> &);

    friend inline bool operator==(const Ligature&, const Ligature&);

};

inline bool
DvipsEncoding::encoded(int e) const
{
    return e >= 0 && e < _e.size() && _e[e] != dot_notdef;
}

inline PermString
DvipsEncoding::encoding(int e) const
{
    if (encoded(e))
        return _e[e];
    else
        return PermString();
}

inline int
DvipsEncoding::encoding_of(PermString what) const
{
    return const_cast<DvipsEncoding *>(this)->encoding_of(what, false);
}

#endif
