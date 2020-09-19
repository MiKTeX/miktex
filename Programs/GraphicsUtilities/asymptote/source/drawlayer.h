/*****
 * drawlayer.h
 * John Bowman
 *
 * Start a new postscript/TeX layer in picture.
 *****/

#ifndef DRAWLAYER_H
#define DRAWLAYER_H

#include "drawelement.h"

namespace camp {

class drawLayer : public drawElement {
public:
  drawLayer() {}

  virtual ~drawLayer() {}

  bool islayer() {return true;}
};

class drawNewPage : public drawLayer {
public:
  drawNewPage() {}

  virtual ~drawNewPage() {}

  bool islabel() {return true;}
  bool isnewpage() {return true;}
  
  bool write(texfile *out, const bbox&) {
    out->verbatimline(settings::latex(out->texengine) ? "\\newpage" : 
                      settings::context(out->texengine) ? "}\\page\\hbox{%" :
                      "\\eject");
    return true;
  }
};

}

GC_DECLARE_PTRFREE(camp::drawLayer);

#endif
