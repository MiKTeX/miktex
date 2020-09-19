/*****
 * drawpath.h
 * Andy Hammerlindl 2002/06/06
 *
 * Stores a path that has been added to a picture.
 *****/

#ifndef DRAWPATH_H
#define DRAWPATH_H

#include "drawelement.h"
#include "path.h"

namespace camp {

class drawPath : public drawPathPenBase {
public:
  drawPath(path src, pen pentype, const string& key="") : 
    drawElement(key), drawPathPenBase(src,pentype) {}
  
  virtual ~drawPath() {}

  void bounds(bbox& b, iopipestream&, boxvector&, bboxlist&) {
    strokebounds(b,p);
  }

  bool svg() {return true;}
  
  bool draw(psfile *out);

  drawElement *transformed(const transform& t);
};

pen adjustdash(pen& p, double arclength, bool cyclic);
}

#endif
