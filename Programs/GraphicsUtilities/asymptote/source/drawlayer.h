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

class drawBBox : public drawLayer {
protected:
  bbox box;
public:
  drawBBox() : box() {}
  drawBBox(const bbox& box) : box(box) {}

  bool islayer() {return false;}
  virtual ~drawBBox() {}

  bool write(texfile *out, const bbox& b) {
    out->BBox(box.empty ? b : box);
    return true;
  }
};

class drawNewPage : public drawBBox {
public:
  drawNewPage() : drawBBox() {}
  drawNewPage(const bbox& box) : drawBBox(box) {}

  virtual ~drawNewPage() {}

  bool islayer() {return true;}
  bool islabel() {return true;}
  bool isnewpage() {return true;}

  bool write(texfile *out, const bbox& b) {
    out->newpage(box.empty ? b : box);
    return true;
  }
};

}

GC_DECLARE_PTRFREE(camp::drawLayer);
GC_DECLARE_PTRFREE(camp::drawBBox);
GC_DECLARE_PTRFREE(camp::drawNewPage);

#endif
