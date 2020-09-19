/*****
 * drawimage.h
 * John Bowman
 *
 * Stores a image that has been added to a picture.
 *****/

#ifndef DRAWIMAGE_H
#define DRAWIMAGE_H

#include "drawelement.h"
#include "array.h"

namespace camp {

class drawImage : public drawElement {
protected:
  transform t;
  bool antialias;
public:
  drawImage(const transform& t, bool antialias, const string& key="")
    : drawElement(key), t(t), antialias(antialias) {}
  
  virtual ~drawImage() {}

  void bounds(bbox& b, iopipestream&, boxvector&, bboxlist&) {
    b += t*pair(0,0);
    b += t*pair(1,1);
  }

  bool svg() {return true;}
  bool svgpng() {return true;}
};

class drawPaletteImage : public drawImage {
  vm::array image;
  vm::array palette;
public:
  drawPaletteImage(const vm::array& image, const vm::array& palette,
                   const transform& t, bool antialias, const string& key="")
    : drawImage(t,antialias,key), image(image), palette(palette) {}
  
  virtual ~drawPaletteImage() {}

  bool draw(psfile *out) {
    out->gsave();
    out->concat(t);
    out->image(image,palette,antialias);
    
    out->grestore();
    
    return true;
  }

  drawElement *transformed(const transform& T) {
    return new drawPaletteImage(image,palette,T*t,antialias,KEY);
  }
};

class drawNoPaletteImage : public drawImage {
  vm::array image;
public:
  drawNoPaletteImage(const vm::array& image, const transform& t,
                     bool antialias, const string& key="")
    : drawImage(t,antialias,key), image(image) {}
  
  virtual ~drawNoPaletteImage() {}

  bool draw(psfile *out) {
    out->gsave();
    out->concat(t);
    out->image(image,antialias);
    out->grestore();
    return true;
  }

  drawElement *transformed(const transform& T) {
    return new drawNoPaletteImage(image,T*t,antialias,KEY);
  }
};

class drawFunctionImage : public drawImage {
  vm::stack *Stack;
  vm::callable *f;
  Int width, height;
public:
  drawFunctionImage(vm::stack *Stack, vm::callable *f, Int width, Int height,
                    const transform& t, bool antialias, const string& key="")
    : drawImage(t,antialias,key), Stack(Stack), f(f),
      width(width), height(height) {}
  
  virtual ~drawFunctionImage() {}

  bool draw(psfile *out) {
    out->gsave();
    out->concat(t);
    out->image(Stack,f,width,height,antialias);
    out->grestore();
    return true;
  }

  drawElement *transformed(const transform& T) {
    return new drawFunctionImage(Stack,f,width,height,T*t,antialias,KEY);
  }
};

class drawRawImage : public drawImage {
  unsigned char *raw; // For internal use; not buffered, may be overwritten.
  size_t width,height;
public:
  drawRawImage(unsigned char *raw, size_t width, size_t height,
               const transform& t, bool antialias, const string& key="")
    : drawImage(t,antialias,key), raw(raw), width(width), height(height) {}
  
  virtual ~drawRawImage() {}

  bool draw(psfile *out) {
    out->gsave();
    out->concat(t);
    out->rawimage(raw,width,height,antialias);
    out->grestore();
    return true;
  }
};

}

#endif
