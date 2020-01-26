// -*- related-file-name: "../../libefont/otfpost.cc" -*-
#ifndef EFONT_OTFPOST_HH
#define EFONT_OTFPOST_HH
#include <efont/otfdata.hh>
#include <lcdf/error.hh>
namespace Efont { namespace OpenType {

class Post { public:

    Post(const String &, ErrorHandler * = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    double italic_angle() const;
    bool is_fixed_pitch() const;
    inline int underline_position() const;
    inline int underline_thickness() const;
    inline uint32_t mem_type42(bool ismax) const;
    int nglyphs() const                 { return _nglyphs; }
    bool glyph_names(Vector<PermString> &gnames) const;

  private:

    Data _str;
    int _error;
    uint32_t _version;
    int _nglyphs;
    Vector<int> _extend_glyph_names;

    enum { HEADER_SIZE = 32, N_MAC_GLYPHS = 258 };
    int parse_header(ErrorHandler *);

};

inline int
Post::underline_position() const
{
    // NB: "This is the suggested distance of the top of the underline from
    // the baseline (negative values indicate below baseline).  The PostScript
    // definition of this FontInfo dictionary key (the y coordinate of the
    // center of the stroke) is not used for historical reasons. The value of
    // the PostScript key may be calculated by subtracting half the
    // underlineThickness from the value of this field." -- OpenType spec
    return ok() ? _str.s16(8) : 0;
}

inline int
Post::underline_thickness() const
{
    return ok() ? _str.s16(10) : 0;
}

inline uint32_t
Post::mem_type42(bool ismax) const
{
    return ok() ? _str.u32(ismax ? 20 : 16) : 0;
}

}}
#endif
