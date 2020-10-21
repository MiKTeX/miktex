/*****
 * drawverbatim.h
 * John Bowman 2003/03/18
 *
 * Add verbatim postscript to picture.
 *****/

#ifndef DRAWVERBATIM_H
#define DRAWVERBATIM_H

#include "drawelement.h"

namespace camp {

enum Language {PostScript,TeX};

class drawVerbatim : public drawElement {
private:
  Language language;
  string text;
  bool userbounds;
  pair min,max;
  bool havebounds;
public:
  drawVerbatim(Language language, const string& text) :
    language(language), text(text), userbounds(false), havebounds(false) {}

  drawVerbatim(Language language, const string& text, pair min,
               pair max) :
    language(language), text(text), userbounds(true), min(min), max(max),
    havebounds(false) {}

  virtual ~drawVerbatim() {}

  void bounds(bbox& b, iopipestream& tex, boxvector&, bboxlist&) {
    if(havebounds) return;
    havebounds=true;
    if(language == TeX)
      tex << text << "%" << newl;
    if(userbounds) {
      b += min;
      b += max;
    }
  }

  bool islabel() {
    return language == TeX;
  }

  bool draw(psfile *out) {
    if(language == PostScript) out->verbatimline(text);
    return true;
  }

  bool write(texfile *out, const bbox&) {
    if(language == TeX) out->verbatimline(stripblanklines(text));
    return true;
  }
};

}

#endif
