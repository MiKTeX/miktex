// -*- related-file-name: "../../libefont/otfcmap.cc" -*-
#ifndef EFONT_OTFCMAP_HH
#define EFONT_OTFCMAP_HH
#include <efont/otf.hh>
#include <lcdf/error.hh>
#include <utility>
namespace Efont { namespace OpenType {

class Cmap { public:

    Cmap(const String &, ErrorHandler * = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    inline Glyph map_uni(uint32_t c) const;
    int map_uni(const Vector<uint32_t> &in, Vector<Glyph> &out) const;
    inline void unmap_all(Vector<std::pair<uint32_t, Glyph> > &ugp) const;

  private:

    String _str;
    int _error;
    int _ntables;
    mutable int _first_unicode_table;
    mutable Vector<int> _table_error;

    enum { HEADER_SIZE = 4, ENCODING_SIZE = 8,
           HIBYTE_SUBHEADERS = 524 };
    enum Format { F_BYTE = 0, F_HIBYTE = 2, F_SEGMENTED = 4, F_TRIMMED = 6,
                  F_HIBYTE32 = 8, F_TRIMMED32 = 10, F_SEGMENTED32 = 12 };
    enum { USE_FIRST_UNICODE_TABLE = -2 };

    int parse_header(ErrorHandler *);
    int first_unicode_table() const     { return _first_unicode_table; }
    int first_table(int platform, int encoding) const;
    int check_table(int t, ErrorHandler * = 0) const;
    Glyph map_table(int t, uint32_t, ErrorHandler * = 0) const;
    void dump_table(int t, Vector<std::pair<uint32_t, Glyph> > &ugp, ErrorHandler * = 0) const;
    inline const uint8_t* table_data(int t) const;

};


inline Glyph Cmap::map_uni(uint32_t c) const {
    return map_table(USE_FIRST_UNICODE_TABLE, c, ErrorHandler::default_handler());
}

inline void Cmap::unmap_all(Vector<std::pair<uint32_t, Glyph> > &ugp) const {
    dump_table(USE_FIRST_UNICODE_TABLE, ugp, ErrorHandler::default_handler());
}

inline const uint8_t* Cmap::table_data(int t) const {
    const uint8_t* data = _str.udata();
    return data + Data::u32_aligned(data + HEADER_SIZE + t * ENCODING_SIZE + 4);
}

}}
#endif
