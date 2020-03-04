/*****
 * drawelement.h
 * Andy Hammerlindl 2002/06/06
 *
 * Abstract base class of any drawable item in camp.
 *****/

#ifndef DRAWELEMENT_H
#define DRAWELEMENT_H

#include <vector>

#include "common.h"
#include "bbox.h"
#include "bbox3.h"
#include "pen.h"
#include "psfile.h"
#include "texfile.h"
#include "prcfile.h"
#include "jsfile.h"
#include "glrender.h"
#include "arrayop.h"
#include "material.h"

namespace camp {

static const double pixel=1.0; // Adaptive rendering constant.

enum Interaction {EMBEDDED=0,BILLBOARD};

void copyArray4x4C(double*& dest, const vm::array *a);
  
class box {
  pair p[4];
public:
  
  box() {}
  box(const pair& a, const pair& b, const pair& c, const pair& d) {
    p[0]=a; p[1]=b; p[2]=c; p[3]=d;
  }
  
// Returns true if the line a--b intersects box b.
  bool intersect(const pair& a, const pair& b) const
  {
    for(Int i=0; i < 4; ++i) {
      pair A=p[i];
      pair B=p[i < 3 ? i+1 : 0];
      double de=(b.x-a.x)*(A.y-B.y)-(A.x-B.x)*(b.y-a.y);
      if(de != 0.0) {
        de=1.0/de;
        double t=((A.x-a.x)*(A.y-B.y)-(A.x-B.x)*(A.y-a.y))*de;
        double T=((b.x-a.x)*(A.y-a.y)-(A.x-a.x)*(b.y-a.y))*de;
        if(0 <= t && t <= 1 && 0 <= T && T <= 1) return true;
      }
    }
    return false;
  }
  
  pair operator [] (Int i) const {return p[i];}
  
  bool intersect(const box& b) const {
    for(Int i=0; i < 4; ++i) {
      pair A=b[i];
      pair B=b[i < 3 ? i+1 : 0];
      if(intersect(A,B)) return true;
    }
    return false;
  }
  
  double xmax() {
    return max(max(max(p[0].x,p[1].x),p[2].x),p[3].x);
  }
  
  double ymax() {
    return max(max(max(p[0].y,p[1].y),p[2].y),p[3].y);
  }
  
  double xmin() {
    return min(min(min(p[0].x,p[1].x),p[2].x),p[3].x);
  }
  
  double ymin() {
    return min(min(min(p[0].y,p[1].y),p[2].y),p[3].y);
  }
  
};
  
class bbox2 {
public:
  double x,y,X,Y;
  bbox2(size_t n, const triple *v) {
    Bounds(v[0]);
    for(size_t i=1; i < n; ++i)
      bounds(v[i]);
  }
    
  bbox2(const triple& m, const triple& M) {
    Bounds(m);
    bounds(triple(m.getx(),m.gety(),M.getz()));
    bounds(triple(m.getx(),M.gety(),m.getz()));
    bounds(triple(m.getx(),M.gety(),M.getz()));
    bounds(triple(M.getx(),m.gety(),m.getz()));
    bounds(triple(M.getx(),m.gety(),M.getz()));
    bounds(triple(M.getx(),M.gety(),m.getz()));
    bounds(M);
  }
    
  bbox2(const triple& m, const triple& M, const Billboard& BB) {
    Bounds(BB.transform(m));
    bounds(BB.transform(triple(m.getx(),m.gety(),M.getz())));
    bounds(BB.transform(triple(m.getx(),M.gety(),m.getz())));
    bounds(BB.transform(triple(m.getx(),M.gety(),M.getz())));
    bounds(BB.transform(triple(M.getx(),m.gety(),m.getz())));
    bounds(BB.transform(triple(M.getx(),m.gety(),M.getz())));
    bounds(BB.transform(triple(M.getx(),M.gety(),m.getz())));
    bounds(BB.transform(M));
  }
    
// Is 2D bounding box formed by projecting 3d points in vector v offscreen?
  bool offscreen() {
    double eps=1.0e-2;
    double min=-1.0-eps;
    double max=1.0+eps;
    return X < min || x > max || Y < min || y > max;
  }
    
  void Bounds(const triple& v) {
    pair V=Transform2T(gl::dprojView,v);
    x=X=V.getx();
    y=Y=V.gety();
  }
  
  void bounds(const triple& v) {
    pair V=Transform2T(gl::dprojView,v);
    double a=V.getx();
    double b=V.gety();
    if(a < x) x=a;
    else if(a > X) X=a;
    if(b < y) y=b;
    else if(b > Y) Y=b;
  }
};

typedef mem::vector<box> boxvector;
  
typedef mem::list<bbox> bboxlist;
  
typedef mem::map<CONST string,unsigned> groupmap;
typedef mem::vector<groupmap> groupsmap;

class drawElement : public gc
{
public:
  string KEY;
  
  drawElement(const string& key="") : KEY(key == "" ? processData().KEY : key)
                                      {}
  
  virtual ~drawElement() {}
  
  static mem::vector<triple> center;
  static size_t centerIndex;
  static triple lastcenter;
  static size_t lastcenterIndex;
  
  static pen lastpen;  
  static const triple zero;
  
  // Adjust the bbox of the picture based on the addition of this
  // element. The iopipestream is needed for determining label sizes.
  virtual void bounds(bbox&, iopipestream&, boxvector&, bboxlist&) {}
  virtual void bounds(const double*, bbox3&) {}
  virtual void bounds(bbox3& b) { bounds(NULL, b); }

  // Compute bounds on ratio (x,y)/z for 3d picture (not cached).
  virtual void ratio(const double *t, pair &b, double (*m)(double, double),
                     double fuzz, bool &first) {}
  
  virtual void minratio(const double *t, pair &b, double fuzz, bool &first) {
    ratio(t,b,camp::min,fuzz,first);
  }
  
  virtual void maxratio(const double *t,pair &b, double fuzz, bool &first) {
    ratio(t,b,camp::max,fuzz,first);
  }
  
  virtual void ratio(pair &b, double (*m)(double, double), double fuzz,
                     bool &first) {
    ratio(NULL,b,m,fuzz,first);
  }

  virtual void minratio(pair &b, double fuzz, bool &first) {
    minratio(NULL,b,fuzz,first);
  }

  virtual void maxratio(pair &b, double fuzz, bool &first) {
    maxratio(NULL,b,fuzz,first);
  }

  virtual bool islabel() {return false;}

  virtual bool isnewpage() {return false;}
  
  virtual bool islayer() {return false;}

  virtual bool is3D() {return false;}

// Implement element as raw SVG code?
  virtual bool svg() {return false;}
  
// Implement SVG element as png image?
  virtual bool svgpng() {return false;}
  
  virtual bool beginclip() {return false;}
  virtual bool endclip() {return false;}
  
  virtual bool begingroup() {return false;}
  virtual bool begingroup3() {return false;}

  virtual bool endgroup() {return false;}
  virtual bool endgroup3() {return false;}

  virtual const double* transf3() {return NULL;}

  virtual void save(bool b) {}
  
  // Output to a PostScript file
  virtual bool draw(psfile *) {
    return false;
  }

  // Output to a TeX file
  virtual bool write(texfile *, const bbox&) {
    return false;
  }

  // Output to a PRC file
  virtual bool write(prcfile *out, unsigned int *count, double compressionlimit,
                     groupsmap& groups) {
    return false;
  }
  
  // Output to a JS file
  virtual bool write(jsfile *out) {
    return false;
  }
  
  // Used to compute deviation of a surface from a quadrilateral.
  virtual void displacement() {}

  // Render with OpenGL
  virtual void render(double size2, const triple& Min, const triple& Max,
                      double perspective, bool remesh) 
  {}

  virtual void meshinit() {}
  
  size_t centerindex(const triple& center) {
    if(drawElement::center.empty() || center != drawElement::lastcenter) {
      drawElement::lastcenter=center;
      drawElement::center.push_back(center);
      drawElement::lastcenterIndex=drawElement::center.size();
    }
    return drawElement::lastcenterIndex;
  }

  // Transform as part of a picture.
  virtual drawElement *transformed(const transform&) {
    return this;
  }
  
  virtual drawElement *transformed(const double* t) {
    return this;
  }

};

// Hold transform of an object.
class drawElementLC : public virtual drawElement {
public:
  double *T; // Keep track of accumulative picture transform
  
  drawElementLC() : T(NULL) {}
  
  drawElementLC(const double *t) : T(NULL) {
    copyTransform3(T,t);
  }

  drawElementLC(const vm::array& t) : T(NULL) {
    copyArray4x4C(T,&t);
  }

  drawElementLC(const double* t, const drawElementLC *s) : 
    drawElement(s->KEY), T(NULL) {
    multiplyTransform3(T,t,s->T);
  }

  virtual ~drawElementLC() {}

  virtual bool is3D() {return true;}

  virtual const double* transf3() {return T;}

  virtual drawElement* transformed(const double* t) {
    return new drawElementLC(t,this);
  }
};

// Base class for drawElements that involve paths.
class drawPathBase : public virtual drawElement {
protected:
  path p;

  path transpath(const transform& t) const {
    return p.transformed(t);
  }

public:
  drawPathBase() {}
  drawPathBase(path p) : p(p) {}

  virtual ~drawPathBase() {}

  virtual void bounds(bbox& b, iopipestream&, boxvector&, bboxlist&) {
    b += p.bounds();
  }
  
  virtual void writepath(psfile *out,bool) {
    out->write(p);
  }
  
  virtual void writeclippath(psfile *out, bool newpath=true) {
    out->writeclip(p,newpath);
  }
  
  virtual void writeshiftedpath(texfile *out) {
    out->writeshifted(p);
  }
};     

// Base class for drawElements that involve paths and pens.
class drawPathPenBase : public drawPathBase {
protected:
  pen pentype;

  pen transpen(const transform& t) const {
    return camp::transformed(shiftless(t),pentype);
  }

public:
  drawPathPenBase(path p, pen pentype) : 
    drawPathBase(p), pentype(pentype) {}
  
  drawPathPenBase(pen pentype) :
    pentype(pentype) {}
  
  virtual bool empty() {
    return p.empty();
  }
  
  virtual bool cyclic() {
    return p.cyclic();
  }
  
  void strokebounds(bbox& b, const path& p);
    
  virtual void penSave(psfile *out)
  {
    if (!pentype.getTransform().isIdentity())
      out->gsave();
  }
  
  virtual void penTranslate(psfile *out)
  {
    out->translate(shiftpair(pentype.getTransform()));
  }

  virtual void penConcat(psfile *out)
  {
    out->concat(shiftless(pentype.getTransform()));
  }

  virtual void penRestore(psfile *out)
  {
    if (!pentype.getTransform().isIdentity())
      out->grestore();
  }
};
  
// Base class for drawElements that involve superpaths and pens.
class drawSuperPathPenBase : public drawPathPenBase {
protected:
  vm::array P;
  size_t size;
  bbox bpath;

  vm::array transpath(const transform& t) const {
    vm::array *Pt=new vm::array(size);
    for(size_t i=0; i < size; i++)
      (*Pt)[i]=vm::read<path>(P,i).transformed(t);
    return *Pt;
  }
  
public:
  drawSuperPathPenBase(const vm::array& P, pen pentype) :
    drawPathPenBase(pentype), P(P), size(P.size()) {}

  bool empty() {
    for(size_t i=0; i < size; i++) 
      if(vm::read<path>(P,i).size() != 0) return false;
    return true;
  }
  
  bool cyclic() {
    for(size_t i=0; i < size; i++) 
      if(!vm::read<path>(P,i).cyclic()) return false;
    return true;
  }
  
  void bounds(bbox& b, iopipestream&, boxvector&, bboxlist&) {
    for(size_t i=0; i < size; i++)
      bpath += vm::read<path>(P,i).bounds();
    b += bpath;
  }
  
  void strokepath(psfile *out) {
    out->strokepath();
  }
  
  void strokebounds(bbox& b) {
    for(size_t i=0; i < size; i++)
      drawPathPenBase::strokebounds(bpath,vm::read<path>(P,i));
    b += bpath;
  }
  
  void writepath(psfile *out, bool newpath=true) {
    if(size > 0) out->write(vm::read<path>(P,0),newpath);
    for(size_t i=1; i < size; i++)
      out->write(vm::read<path>(P,i),false);
  }
  
  void writeclippath(psfile *out, bool newpath=true) {
    if(size > 0) out->writeclip(vm::read<path>(P,0),newpath);
    for(size_t i=1; i < size; i++)
      out->writeclip(vm::read<path>(P,i),false);
  }
  
  void writeshiftedpath(texfile *out) {
    for(size_t i=0; i < size; i++) 
      out->writeshifted(vm::read<path>(P,i),i == 0);
  }
};
 
#ifdef HAVE_LIBGLM
void setcolors(bool colors,
               const prc::RGBAColour& diffuse,
               const prc::RGBAColour& emissive,
               const prc::RGBAColour& specular, double shininess,
               double metallic, double fresnel0, jsfile *out=NULL);
#endif

  

}

GC_DECLARE_PTRFREE(camp::box);
GC_DECLARE_PTRFREE(camp::drawElement);

#endif
