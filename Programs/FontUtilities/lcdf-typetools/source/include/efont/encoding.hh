// -*- related-file-name: "../../libefont/encoding.cc" -*-
#ifndef EFONT_ENCODING_HH
#define EFONT_ENCODING_HH
#include <lcdf/vector.hh>
namespace Efont {
typedef int GlyphIndex;

class Encoding8 {

  Vector<GlyphIndex> _codes;
  Vector<int> _code_map;

 public:

  Encoding8()                           : _code_map(256, -1) { }

  void reserve_glyphs(int);

  int code(GlyphIndex gi) const         { return _codes[gi]; }
  GlyphIndex find_code(int c) const     { return _code_map[c]; }

  void set_code(GlyphIndex gi, int c)   { _codes[gi] = c; _code_map[c] = gi; }

};

}
#endif
