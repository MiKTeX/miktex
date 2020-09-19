// -*- related-file-name: "../../libefont/ttfkern.cc" -*-
#ifndef EFONT_TTFKERN_HH
#define EFONT_TTFKERN_HH
#include <efont/otfgpos.hh>
namespace Efont { namespace OpenType {

class KernTable { public:

    KernTable(const Data &, ErrorHandler * = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }

    bool unparse_automatics(Vector<Positioning> &, ErrorHandler * = 0) const;

  private:

    Data _d;
    int _version;
    int _error;

    enum {
        COV_V0_HORIZONTAL = 0x0001, COV_V0_MINIMUM = 0x0002,
        COV_V0_CROSS_STREAM = 0x0004, COV_V0_OVERRIDE = 0x0008,
        COV_V0_FORMAT = 0xFF00, COV_V0_FORMAT0 = 0x0000,
        COV_V1_VERTICAL = 0x8000, COV_V1_CROSS_STREAM = 0x4000,
        COV_V1_VARIATION = 0x2000,
        COV_V1_FORMAT = 0x00FF, COV_V1_FORMAT0 = 0x0000
    };

    inline uint32_t ntables() const {
        return _version == 0 ? _d.u16(2) : _d.u32(4);
    }
    inline uint32_t first_offset() const {
        return _version == 0 ? 4 : 8;
    }
    inline Data subtable(uint32_t &off) const;

};

}}
#endif
