/*****
 * drawclipbegin.h
 * John Bowman
 *
 * Begin clip of picture to specified path.
 *****/

#ifndef DRAWCLIPBEGIN_H
#define DRAWCLIPBEGIN_H

#include "drawelement.h"
#include "path.h"
#include "drawpath.h"

namespace camp {

class drawClipBegin : public drawSuperPathPenBase {
  bool gsave;
  bool stroke;
public:
  void noncyclic() {
    reportError("cannot clip to non-cyclic path");
  }
  
  drawClipBegin(const vm::array& src, bool stroke, pen pentype,
                bool gsave=true, const string& key="") :
    drawElement(key), drawSuperPathPenBase(src,pentype), gsave(gsave),
    stroke(stroke) {
    if(!stroke && !cyclic()) noncyclic();
  }

  virtual ~drawClipBegin() {}

  bool beginclip() {return true;}
  
  void bounds(bbox& b, iopipestream& iopipe, boxvector& vbox,
              bboxlist& bboxstack) {
    bboxstack.push_back(b);
    bbox bpath;
    if(stroke) strokebounds(bpath);
    else drawSuperPathPenBase::bounds(bpath,iopipe,vbox,bboxstack);
    bboxstack.push_back(bpath);
  }

  bool begingroup() {return true;}
  
  bool svg() {return true;}
  
  void save(bool b) {
    gsave=b;
  }
  
  bool draw(psfile *out) {
    if(gsave) out->gsave();
    if(empty()) return true;
    out->beginclip();
    writepath(out,false);
    if(stroke) strokepath(out);
    out->endclip(pentype);
    return true;
  }

  bool write(texfile *out, const bbox& bpath) {
    if(gsave) out->gsave();
    if(empty()) return true;
    
    if(out->toplevel()) 
      out->beginpicture(bpath);
      
    out->begingroup();

    out->beginspecial();
    out->beginraw();
    writeshiftedpath(out);
    if(stroke) strokepath(out);
    out->endclip(pentype);
    out->endraw();
    out->endspecial();
    
    return true;
  }
  
  drawElement *transformed(const transform& t)
  {
    return new drawClipBegin(transpath(t),stroke,transpen(t),gsave,KEY);
  }

};

}

#endif
