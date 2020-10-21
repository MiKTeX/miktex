/*****
 * drawgrestore.h
 * John Bowman
 *
 * Output PostScript grestore to picture.
 *****/

#ifndef DRAWGRESTORE_H
#define DRAWGRESTORE_H

#include "drawelement.h"

namespace camp {

class drawGrestore : public drawElement {
public:
  drawGrestore() {}
  virtual ~drawGrestore() {}

  bool draw(psfile *out) {
    out->grestore();
    return true;
  }

  bool write(texfile *out, const bbox&) {
    out->grestore();
    return true;
  }
};

}

GC_DECLARE_PTRFREE(camp::drawGrestore);

#endif
