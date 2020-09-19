// -*- related-file-name: "../../libefont/otfos2.cc" -*-
#ifndef EFONT_OTFOS2_HH
#define EFONT_OTFOS2_HH
#include <efont/otf.hh>
#include <efont/otfdata.hh>
#include <lcdf/error.hh>
namespace Efont { namespace OpenType {

class Os2 { public:

    Os2(const Data &, ErrorHandler * = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    enum Offsets { O_AVGCHARWIDTH = 2, O_SUBSCRIPTXSIZE = 10,
                   O_SUBSCRIPTYSIZE = 12, O_SUBSCRIPTXOFFSET = 14,
                   O_SUBSCRIPTYOFFSET = 16, O_SUPERSCRIPTXSIZE = 18,
                   O_SUPERSCRIPTYSIZE = 20, O_SUPERSCRIPTXOFFSET = 22,
                   O_SUPERSCRIPTYOFFSET = 24, O_STRIKEOUTSIZE = 26,
                   O_STRIKEOUTPOSITION = 28, O_VENDORID = 58,
                   O_TYPOASCENDER = 68, O_TYPODESCENDER = 70,
                   O_TYPOLINEGAP = 72, O_XHEIGHT = 86, O_CAPHEIGHT = 88,
                   O_LOWEROPTICALPOINTSIZE = 96, O_UPPEROPTICALPOINTSIZE = 98 };
    enum { HEADER_SIZE = 2 };

    inline int16_t typo_ascender() const;
    inline int16_t typo_descender() const;
    inline int16_t typo_line_gap() const;
    inline int16_t x_height() const;
    inline int16_t cap_height() const;
    inline double lower_optical_point_size() const;
    inline double upper_optical_point_size() const;
    inline bool has_optical_point_size() const noexcept;
    inline String vendor_id() const noexcept;

  private:

    Data _data;
    int _error;

    int parse_header(ErrorHandler *);

};


inline int16_t Os2::typo_ascender() const
{
    return _data.s16(O_TYPOASCENDER);
}

inline int16_t Os2::typo_descender() const
{
    return _data.s16(O_TYPODESCENDER);
}

inline int16_t Os2::typo_line_gap() const
{
    return _data.s16(O_TYPOLINEGAP);
}

inline int16_t Os2::x_height() const
{
    return _data.s16(O_XHEIGHT);
}

inline int16_t Os2::cap_height() const
{
    return _data.s16(O_CAPHEIGHT);
}

inline double Os2::lower_optical_point_size() const
{
    return _data.u16(O_LOWEROPTICALPOINTSIZE) / 20.;
}

inline double Os2::upper_optical_point_size() const
{
    return _data.u16(O_UPPEROPTICALPOINTSIZE) / 20.;
}

inline bool Os2::has_optical_point_size() const noexcept
{
    return _data.length() >= O_UPPEROPTICALPOINTSIZE + 2;
}

inline String Os2::vendor_id() const noexcept
{
    return _data.substring(O_VENDORID, 4).string();
}

}}
#endif
