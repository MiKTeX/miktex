// -*- related-file-name: "../../libefont/otf.cc" -*-
#ifndef EFONT_OTF_HH
#define EFONT_OTF_HH
#include <efont/otfdata.hh>
#include <lcdf/hashcode.hh>
class ErrorHandler;
namespace Efont { namespace OpenType {
class Post;
class Name;

typedef int Glyph;                      // 16-bit integer

class Tag {
  public:
    Tag()                               : _tag(0U) { }
    explicit Tag(uint32_t tag)          : _tag(tag) { }
    Tag(const char *name);
    Tag(const String &name);
    // default destructor

    static Tag head_tag()               { return Tag(0x68656164U); }

    typedef bool (Tag::*unspecified_bool_type)() const;
    bool null() const                   { return _tag == 0; }
    operator unspecified_bool_type() const {
        return _tag != 0 ? &Tag::null : 0;
    }
    bool valid() const;

    uint32_t value() const              { return _tag; }

    String text() const;
    static String langsys_text(Tag script, Tag langsys = Tag());

    const uint8_t* table_entry(const uint8_t* table, int n, int entry_size) const;

    const char* script_description() const;
    const char* language_description() const;
    const char* feature_description() const;

  private:
    uint32_t _tag;
};

inline hashcode_t hashcode(const Tag& t) {
    return t.value();
}

class Font {
  public:
    Font(const String& str, ErrorHandler* errh = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }
    bool check_checksums(ErrorHandler* errh = 0) const;
    int error() const                   { return _error; }

    const String& data_string() const   { return _str; }
    const uint8_t* data() const         { return _str.udata(); }
    int length() const                  { return _str.length(); }

    unsigned units_per_em() const       { return _units_per_em; }

    int ntables() const;
    bool has_table(Tag tag) const;
    String table(Tag tag) const;
    uint32_t table_checksum(Tag tag) const;
    Tag table_tag(int i) const;

    static uint32_t checksum(const uint8_t *, const uint8_t *);
    static uint32_t checksum(const String &);
    static Font make(bool truetype, const Vector<Tag>& tags, const Vector<String>& data);

    enum { HEADER_SIZE = 12, TABLE_DIR_ENTRY_SIZE = 16 };

  private:
    String _str;
    int _error;
    unsigned _units_per_em;

    int parse_header(ErrorHandler*);
};

class ScriptList {
  public:
    ScriptList()                { }
    inline ScriptList(const String&, ErrorHandler* = 0);
    int assign(const String&, ErrorHandler* = 0);
    // default destructor

    bool ok() const                     { return _str.length() > 0; }

    int language_systems(Vector<Tag>& scripts, Vector<Tag>& langsys, ErrorHandler* = 0) const;
    int features(Tag script, Tag langsys, int& required_fid, Vector<int>& fids, ErrorHandler* = 0, bool clear_fids = true) const;

  private:
    enum { SCRIPTLIST_HEADERSIZE = 2, SCRIPT_RECSIZE = 6,
           SCRIPT_HEADERSIZE = 4, LANGSYS_RECSIZE = 6,
           LANGSYS_HEADERSIZE = 6, FEATURE_RECSIZE = 2 };

    String _str;

    int check_header(ErrorHandler*);
    int script_offset(Tag) const;
    int check_script(Tag, int, ErrorHandler*) const;
    int langsys_offset(Tag, Tag, ErrorHandler* = 0) const;
};

class FeatureList {
  public:
    FeatureList()               { }
    inline FeatureList(const String&, ErrorHandler* = 0);
    int assign(const String&, ErrorHandler* = 0);
    // default destructor

    bool ok() const                     { return _str.length() > 0; }

    Tag tag(int fid) const;
    String params(int fid, int length, ErrorHandler* = 0, bool old_style_offset = false) const;
    String size_params(int fid, const Name& name, ErrorHandler* = 0) const;
    int lookups(int fid, Vector<int>& results, ErrorHandler* = 0, bool clear_results = true) const;

    int find(Tag, const Vector<int>& fids) const;
    void filter(Vector<int>& fids, const Vector<Tag>& sorted_ftags) const;
    inline void filter(Vector<int>& fids, Tag ftag) const;

    int lookups(const Vector<int>& fids, Vector<int>& results, ErrorHandler* = 0) const;
    int lookups(const Vector<int>& required_fids, const Vector<int>& fids, const Vector<Tag>& sorted_ftags, Vector<int>& results, ErrorHandler* = 0) const;
    int lookups(int required_fid, const Vector<int>& fids, const Vector<Tag>& sorted_ftags, Vector<int>& results, ErrorHandler* = 0) const;
    int lookups(const ScriptList&, Tag script, Tag langsys, const Vector<Tag>& sorted_ftags, Vector<int>& results, ErrorHandler* = 0) const;

  private:
    enum { FEATURELIST_HEADERSIZE = 2, FEATURE_RECSIZE = 6,
           FEATURE_HEADERSIZE = 4, LOOKUPLIST_RECSIZE = 2 };

    String _str;

    int check_header(ErrorHandler*);
    int script_offset(Tag) const;
    int langsys_offset(Tag, Tag, ErrorHandler* = 0) const;
};

class Coverage {
  public:
    Coverage() noexcept;                // empty coverage
    Coverage(Glyph first, Glyph last) noexcept; // range coverage
    Coverage(const Vector<bool> &gmap) noexcept; // used-bytemap coverage
    Coverage(const String &str, ErrorHandler *errh = 0, bool check = true) noexcept;
    // default destructor

    bool ok() const noexcept            { return _str.length() > 0; }
    int size() const noexcept;
    bool has_fast_covers() const noexcept {
        return _str.length() > 0 && _str.data()[1] == T_X_BYTEMAP;
    }

    int coverage_index(Glyph) const noexcept;
    bool covers(Glyph g) const noexcept { return coverage_index(g) >= 0; }

    void unparse(StringAccum&) const noexcept;
    String unparse() const noexcept;

    class iterator { public:
        iterator()                      : _pos(0), _value(0) { }
        // private constructor
        // default destructor

        bool ok() const                 { return _pos < _str.length(); }
        operator bool() const           { return ok(); }

        Glyph operator*() const         { return _value; }
        Glyph value() const             { return _value; }
        int coverage_index() const;

        void operator++(int);
        void operator++()               { (*this)++; }
        bool forward_to(Glyph);

        // XXX should check iterators are of same type
        bool operator<(const iterator& o) { return _value < o._value; }
        bool operator<=(const iterator& o) { return _value <= o._value; }
        bool operator>=(const iterator& o) { return _value >= o._value; }
        bool operator>(const iterator& o) { return _value > o._value; }
        bool operator==(const iterator& o) { return _value == o._value; }
        bool operator!=(const iterator& o) { return _value != o._value; }

      private:
        String _str;
        int _pos;
        Glyph _value;
        friend class Coverage;
        iterator(const String &str, bool is_end);
    };

    iterator begin() const              { return iterator(_str, false); }
    iterator end() const                { return iterator(_str, true); }
    Glyph operator[](int) const noexcept;

    enum { T_LIST = 1, T_RANGES = 2, T_X_BYTEMAP = 3,
           HEADERSIZE = 4, LIST_RECSIZE = 2, RANGES_RECSIZE = 6 };

  private:
    String _str;

    int check(ErrorHandler*);
};

Coverage operator&(const Coverage&, const Coverage&);
bool operator<=(const Coverage&, const Coverage&);

inline bool operator>=(const Coverage& a, const Coverage& b) {
    return b <= a;
}

class GlyphSet {
  public:
    GlyphSet();
    GlyphSet(const GlyphSet&);
    ~GlyphSet();

    inline bool covers(Glyph g) const;
    inline bool operator[](Glyph g) const;
    int change(Glyph, bool);
    void insert(Glyph g)                { change(g, true); }
    void remove(Glyph g)                { change(g, false); }

    GlyphSet& operator=(const GlyphSet&);

  private:
    enum { GLYPHBITS = 16, SHIFT = 8,
           MAXGLYPH = (1 << GLYPHBITS) - 1, UNSHIFT = GLYPHBITS - SHIFT,
           MASK = (1 << UNSHIFT) - 1, VLEN = (1 << SHIFT),
           VULEN = (1 << UNSHIFT) >> 5
    };

    uint32_t* _v[VLEN];
};

class ClassDef {
  public:
    ClassDef(const String&, ErrorHandler* = 0) noexcept;
    // default destructor

    bool ok() const                     { return _str.length() > 0; }
    int nclass() const noexcept;

    int lookup(Glyph) const noexcept;
    int operator[](Glyph g) const noexcept { return lookup(g); }

    void unparse(StringAccum&) const noexcept;
    String unparse() const noexcept;

    class class_iterator {
      public:
        // private constructor
        // default destructor

        bool ok() const                 { return _pos < _str.length(); }
        operator bool() const           { return ok(); }

        Glyph operator*() const         { return *_coviter; }
        Glyph value() const             { return *_coviter; }
        int class_value() const         { return _class; }

        void operator++(int);
        void operator++()               { (*this)++; }

        // XXX should check iterators are of same type
        bool operator<(const class_iterator& o) { return _coviter < o._coviter; }
        bool operator<=(const class_iterator& o) { return _coviter <= o._coviter; }
        bool operator>=(const class_iterator& o) { return _coviter >= o._coviter; }
        bool operator>(const class_iterator& o) { return _coviter > o._coviter; }
        bool operator==(const class_iterator& o) { return _coviter == o._coviter; }
        bool operator!=(const class_iterator& o) { return _coviter != o._coviter; }

      private:
        String _str;
        int _pos;
        int _class;
        Coverage::iterator _coviter;
        friend class ClassDef;
        class_iterator(const String&, int, int, const Coverage::iterator&);
        void increment_class0();
        enum { FIRST_POS = -1, LAST_POS = -2 };
    };

    // XXX does not work correctly for class 0
    class_iterator begin(int c) const   { return class_iterator(_str, 0, c, Coverage::iterator()); }
    class_iterator begin(int c, const Coverage& coverage) const { return class_iterator(_str, 0, c, coverage.begin()); }
    class_iterator end(int c) const     { return class_iterator(_str, _str.length(), c, Coverage::iterator()); }

    enum { T_LIST = 1, T_RANGES = 2,
           LIST_HEADERSIZE = 6, LIST_RECSIZE = 2,
           RANGES_HEADERSIZE = 4, RANGES_RECSIZE = 6 };

  private:
    String _str;

    int check(ErrorHandler*);
};

extern Vector<PermString> debug_glyph_names;


inline bool operator==(Tag t1, uint32_t t2) {
    return t1.value() == t2;
}

inline bool operator!=(Tag t1, uint32_t t2) {
    return t1.value() != t2;
}

inline bool operator<(Tag t1, uint32_t t2) {
    return t1.value() < t2;
}

inline bool operator>(Tag t1, uint32_t t2) {
    return t1.value() > t2;
}

inline bool operator<=(Tag t1, uint32_t t2) {
    return t1.value() <= t2;
}

inline bool operator>=(Tag t1, uint32_t t2) {
    return t1.value() >= t2;
}

inline bool operator==(Tag t1, Tag t2) {
    return t1.value() == t2.value();
}

inline bool operator!=(Tag t1, Tag t2) {
    return t1.value() != t2.value();
}

inline bool operator<(Tag t1, Tag t2) {
    return t1.value() < t2.value();
}

inline ScriptList::ScriptList(const String& str, ErrorHandler* errh) {
    assign(str, errh);
}

inline FeatureList::FeatureList(const String& str, ErrorHandler* errh) {
    assign(str, errh);
}

inline void FeatureList::filter(Vector<int>& fids, Tag ftag) const {
    Vector<Tag> tags;
    tags.push_back(ftag);
    filter(fids, tags);
}

inline bool GlyphSet::covers(Glyph g) const {
    if ((unsigned)g > MAXGLYPH)
        return false;
    else if (const uint32_t* u = _v[g >> SHIFT])
        return (u[(g & MASK) >> 5] & (1 << (g & 0x1F))) != 0;
    else
        return false;
}

inline bool GlyphSet::operator[](Glyph g) const {
    return covers(g);
}

} // namespace Efont::OpenType
} // namespace Efont

#endif
