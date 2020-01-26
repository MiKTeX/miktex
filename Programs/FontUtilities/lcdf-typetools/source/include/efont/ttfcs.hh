// -*- related-file-name: "../../libefont/ttfcs.cc" -*-
#ifndef EFONT_TTFCS_HH
#define EFONT_TTFCS_HH
#include <efont/t1cs.hh>
#include <efont/otf.hh>
#include <efont/otfdata.hh>
namespace Efont {

class TrueTypeBoundsCharstringProgram : public CharstringProgram { public:

    TrueTypeBoundsCharstringProgram(const OpenType::Font *);
    ~TrueTypeBoundsCharstringProgram();

    void font_matrix(double[6]) const;

    int nglyphs() const;
    Charstring *glyph(int gi) const;
    PermString glyph_name(int gi) const;
    void glyph_names(Vector<PermString> &) const;

  private:

    const OpenType::Font *_otf;
    int _nglyphs;
    int _nhmtx;
    bool _loca_long;
    OpenType::Data _loca;
    OpenType::Data _glyf;
    OpenType::Data _hmtx;
    mutable Vector<Charstring*> _charstrings;
    mutable Vector<PermString> _glyph_names;
    mutable bool _got_glyph_names;
    mutable Vector<uint32_t> _unicodes;
    mutable bool _got_unicodes;

};

}
#endif

