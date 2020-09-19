// -*- related-file-name: "../../libefont/ttfhead.cc" -*-
#ifndef EFONT_TTFHEAD_HH
#define EFONT_TTFHEAD_HH
#include <efont/otf.hh>
#include <efont/otfdata.hh>     // for ntohl()
#include <lcdf/error.hh>
namespace Efont { namespace OpenType {

class Head { public:

    Head(const String &, ErrorHandler * = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    unsigned units_per_em() const;
    unsigned index_to_loc_format() const;

  private:

    Data _d;
    int _error;

    int parse_header(ErrorHandler *);

};


inline unsigned Head::units_per_em() const
{
    return (_error >= 0 ? _d.u16(18) : 0);
}

inline unsigned Head::index_to_loc_format() const
{
    return (_error >= 0 ? _d.u16(50) : 0);
}

}}
#endif
