/*****
 * drawgsave.h
 * John Bowman
 *
 * Output PostScript gsave to picture.
 *****/

#ifndef DRAWGSAVE_H
#define DRAWGSAVE_H

#include "drawelement.h"

namespace camp {

class drawGsave : public drawElement {
public:
  drawGsave() {}
  virtual ~drawGsave() {}

  bool draw(psfile *out) {
    out->gsave();
    return true;
  }
  
  bool write(texfile *out, const bbox&) {
    out->gsave();
    return true;
  }
};

}

GC_DECLARE_PTRFREE(camp::drawGsave);

#endif
