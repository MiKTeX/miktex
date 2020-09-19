/*****
 * drawfill.h
 * Andy Hammerlindl 2002/06/06
 *
 * Stores a cyclic path that will outline a filled shape in a picture.
 *****/

#ifndef DRAWFILL_H
#define DRAWFILL_H

#include "drawelement.h"
#include "path.h"

namespace camp {

class drawFill : public drawSuperPathPenBase {
protected:  
  bool stroke;
public:
  void noncyclic() {
    reportError("non-cyclic path cannot be filled");
  }
  
  drawFill(const vm::array& src, bool stroke, pen pentype,
           const string& key="") : 
    drawElement(key), drawSuperPathPenBase(src,pentype), stroke(stroke) {
    if(!stroke && !cyclic()) noncyclic();
  }

  bool svg() {return true;}
  
  // dvisvgm doesn't yet support SVG patterns.
  bool svgpng() {return pentype.fillpattern() != "";}
  
  virtual ~drawFill() {}

  virtual bool draw(psfile *out);
  
  virtual void palette(psfile *out) {
    penSave(out);
    penTranslate(out);
  }
  virtual void fill(psfile *out) {
    out->setpen(pentype);
    if(stroke) out->strokepath();
    out->fill(pentype);
    penRestore(out);
  };

  drawElement *transformed(const transform& t);
};
  
class drawShade : public drawFill {
public:
  drawShade(const vm::array& src, bool stroke, pen pentype,
            const string& key="")
    : drawFill(src,stroke,pentype,key) {}

  void bounds(bbox& b, iopipestream& iopipe, boxvector& vbox,
              bboxlist& bboxstack) {
    if(stroke) strokebounds(b);
    else drawSuperPathPenBase::bounds(b,iopipe,vbox,bboxstack);
  }
  
  // Shading in SVG is incomplete and not supported at all by dvisvgm.
  bool svgpng() {return true;}
      
  virtual void beginshade(psfile *out)=0;
  virtual void shade(psfile *out)=0;
  
  bool draw(psfile *out) {
    if(pentype.invisible() || empty()) return true;
  
    palette(out);
    beginshade(out);
    writeclippath(out);
    if(stroke) strokepath(out);
    out->endpsclip(pentype.Fillrule());
    shade(out);
    out->grestore();
    return true;
  }
};
  
class drawLatticeShade : public drawShade {
protected:
  vm::array pens;
  const transform T;
public:  
  drawLatticeShade(const vm::array& src, bool stroke,
                   pen pentype, const vm::array& pens,
                   const camp::transform& T=identity, const string& key="")
    : drawShade(src,stroke,pentype,key), pens(pens), T(T) {}
  
  void palette(psfile *out) {
    out->gsave();
  }
  
  void beginshade(psfile *out) {
    out->beginlatticeshade(pens,bpath);
  }
  
  void shade(psfile *out) {
    bbox b;
    for(size_t i=0; i < size; i++) {
      path p=vm::read<path>(P,i).transformed(inverse(T));
      if(stroke)
        drawPathPenBase::strokebounds(b,p);
      else 
        b += p.bounds();
    }
    out->latticeshade(pens,T*matrix(b.Min(),b.Max()));
  }
  
  drawElement *transformed(const transform& t);
};
  
class drawAxialShade : public drawShade {
protected:
  pair a;
  bool extenda;
  pen penb;
  pair b;
  bool extendb;
  ColorSpace colorspace;
public:  
  drawAxialShade(const vm::array& src, bool stroke,
                 pen pentype, pair a, bool extenda, pen penb, pair b,
                 bool extendb, const string& key="") 
    : drawShade(src,stroke,pentype,key), a(a), extenda(extenda),
      penb(penb), b(b), extendb(extendb) {}
  
  bool svgpng() {return false;}
  
  void palette(psfile *out);
  
  void beginshade(psfile *out) {
    out->begingradientshade(true,colorspace,pentype,a,0,penb,b,0);
  }
  
  void shade(psfile *out) {
    out->gradientshade(true,colorspace,pentype,a,0,extenda,penb,b,0,extendb);
  }
  
  drawElement *transformed(const transform& t);
};
  
class drawRadialShade : public drawAxialShade {
protected:
  double ra;
  double rb;
public:
  drawRadialShade(const vm::array& src, bool stroke,
                  pen pentype, pair a, double ra, bool extenda, pen penb,
                  pair b, double rb, bool extendb, const string& key="")
    : drawAxialShade(src,stroke,pentype,a,extenda,penb,b,
                     extendb,key), ra(ra), rb(rb) {}
  
  bool svgpng() {return ra > 0.0;}
  
  void beginshade(psfile *out) {
    out->begingradientshade(false,colorspace,pentype,a,ra,penb,b,rb);
  }
  
  void shade(psfile *out) {
    out->gradientshade(false,colorspace,pentype,a,ra,extenda,penb,b,rb,extendb);
  }
  
  drawElement *transformed(const transform& t);
};
  
class drawGouraudShade : public drawShade {
protected:
  vm::array pens,vertices,edges;
public:  
  drawGouraudShade(const vm::array& src, bool stroke,
                   pen pentype, const vm::array& pens,
                   const vm::array& vertices, const vm::array& edges,
                   const string& key="")
    : drawElement(key), drawShade(src,stroke,pentype,key), pens(pens),
      vertices(vertices), edges(edges) {}
  
  bool svgpng() {return !settings::getSetting<bool>("svgemulation");}
  
  void palette(psfile *out) {
    out->gsave();
  }
  
  void beginshade(psfile *out) {
    out->begingouraudshade(pens,vertices,edges);
  }
  
  void shade(psfile *out) {
    out->gouraudshade(pentype,pens,vertices,edges);
  }
  
  drawElement *transformed(const transform& t);
};
  
class drawTensorShade : public drawShade {
protected:
  vm::array pens,boundaries,z;
public:  
  drawTensorShade(const vm::array& src, bool stroke,
                  pen pentype, const vm::array& pens,
                  const vm::array& boundaries, const vm::array& z,
                  const string& key="") : 
    drawShade(src,stroke,pentype,key), pens(pens), boundaries(boundaries),
    z(z) {}
  
  bool svgpng() {
    return pens.size() > 1 || !settings::getSetting<bool>("svgemulation");
  }
  
  void palette(psfile *out) {
    out->gsave();
  }
  
  void beginshade(psfile *out) {
    out->begintensorshade(pens,boundaries,z);
  }
  
  void shade(psfile *out) {
    out->tensorshade(pentype,pens,boundaries,z);
  }
  
  drawElement *transformed(const transform& t);
};
  
class drawFunctionShade : public drawFill {
protected:  
  string shader;
public:
  drawFunctionShade(const vm::array& src, bool stroke,
                    pen pentype, const string& shader, const string& key="")
    : drawFill(src,stroke,pentype,key), shader(shader) {
    string texengine=settings::getSetting<string>("tex");
    if(!settings::pdf(texengine))
      reportError("functionshade is not implemented for the '"+texengine+
                  "' tex engine");
  }

  virtual ~drawFunctionShade() {}

  bool draw(psfile *out) {return false;}
  
  bool write(texfile *, const bbox&);
  
  bool islabel() {return true;}
  
  drawElement *transformed(const transform& t);
};
  
}

#endif
