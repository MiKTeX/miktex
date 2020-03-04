/*****
 * drawsurface.h
 *
 * Stores a surface that has been added to a picture.
 *****/

#ifndef DRAWSURFACE_H
#define DRAWSURFACE_H

#include "drawelement.h"
#include "arrayop.h"
#include "path3.h"
#include "beziercurve.h"
#include "bezierpatch.h"

namespace run {
void inverse(double *a, size_t n);
}

const string need3pens="array of 3 pens required";

namespace camp {

#ifdef HAVE_LIBGLM
void storecolor(GLfloat *colors, int i, const vm::array &pens, int j);
#endif  

class drawSurface : public drawElement {
protected:
  triple *controls;
  size_t ncontrols;
  triple center;
  bool straight; // True iff Bezier patch is planar and has straight edges.
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  prc::RGBAColour *colors;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  bool invisible;
  Interaction interaction;
  bool billboard;
  size_t centerIndex;  
  
  triple Min,Max;
  bool primitive;
  
public:
#ifdef HAVE_GL
  BezierCurve C;
  bool transparent;
#endif  
  
  string wrongsize() {
    return (ncontrols == 16 ? "4x4" : "triangular")+
      string(" array of triples and array of 4 pens required");
  }
  
  void init() {
    billboard=interaction == BILLBOARD &&
      !settings::getSetting<bool>("offscreen");
    centerIndex=0;
  }
  
  drawSurface(const vm::array& g, size_t ncontrols, triple center,
              bool straight, const vm::array&p, double opacity,
              double shininess, double metallic, double fresnel0,
              const vm::array &pens, Interaction interaction,
              bool primitive=true, const string& key="") :
    drawElement(key), ncontrols(ncontrols), center(center), straight(straight),
    opacity(opacity), shininess(shininess), metallic(metallic),
    fresnel0(fresnel0), interaction(interaction), primitive(primitive) {
    init();
    if(checkArray(&g) != 4 || checkArray(&p) != 3)
      reportError(wrongsize());
    
    size_t k=0;
    controls=new(UseGC) triple[ncontrols];
    for(unsigned int i=0; i < 4; ++i) {
      vm::array *gi=vm::read<vm::array*>(g,i);
      size_t n=(ncontrols == 16 ? 4 : i+1);
      if(checkArray(gi) != n)
        reportError(wrongsize());
      for(unsigned int j=0; j < n; ++j)
        controls[k++]=vm::read<triple>(gi,j);
    }
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
    
    size_t nodes=(ncontrols == 16 ? 4 : 3);
    size_t size=checkArray(&pens);
    if(size > 0) {
      if(size != nodes) reportError("one vertex pen required per node");
      colors=new(UseGC) prc::RGBAColour[nodes];
      for(size_t i=0; i < nodes; ++i)
      colors[i]=rgba(vm::read<camp::pen>(pens,i));
    } else colors=NULL;
  }
  
  drawSurface(const double* t, const drawSurface *s) :
    drawElement(s->KEY), ncontrols(s->ncontrols), straight(s->straight),
    diffuse(s->diffuse), emissive(s->emissive), specular(s->specular),
    colors(s->colors), opacity(s->opacity), shininess(s->shininess),
    metallic(s->metallic), fresnel0(s->fresnel0), invisible(s->invisible),
    interaction(s->interaction), primitive(s->primitive) {
    init();
    if(s->controls) {
      controls=new(UseGC) triple[ncontrols];
      for(unsigned int i=0; i < ncontrols; ++i)
        controls[i]=t*s->controls[i];
    } else controls=NULL;
  
    center=t*s->center;
  }
  
  virtual ~drawSurface() {}

  bool is3D() {return true;}
};
  
class drawBezierPatch : public drawSurface {
public:  
#ifdef HAVE_GL
  BezierPatch S;
#endif  
  
  drawBezierPatch(const vm::array& g, triple center, bool straight,
              const vm::array&p, double opacity, double shininess,
              double metallic, double fresnel0, const vm::array &pens,
                  Interaction interaction, bool primitive) :
    drawSurface(g,16,center,straight,p,opacity,shininess,metallic,fresnel0,
                pens,interaction,primitive) {}

  drawBezierPatch(const double* t, const drawBezierPatch *s) :
    drawSurface(t,s) {}
  
  void bounds(const double* t, bbox3& b);
  
  void ratio(const double* t, pair &b, double (*m)(double, double),
             double fuzz, bool &first);
  
  void meshinit() {
    if(billboard)
      centerIndex=centerindex(center);
  }
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  bool write(jsfile *out);
  
  void render(double, const triple& b, const triple& B,
              double perspective, bool remesh);
  drawElement *transformed(const double* t);
};
  
class drawBezierTriangle : public drawSurface {
public:
#ifdef HAVE_GL
  BezierTriangle S;
#endif  
  
  drawBezierTriangle(const vm::array& g, triple center, bool straight,
                     const vm::array&p, double opacity, double shininess,
                     double metallic, double fresnel0, const vm::array &pens,
                     Interaction interaction, bool primitive) :
    drawSurface(g,10,center,straight,p,opacity,shininess,metallic,fresnel0,
                pens,interaction,primitive) {}
  
  drawBezierTriangle(const double* t, const drawBezierTriangle *s) :
    drawSurface(t,s) {}
  
  void bounds(const double* t, bbox3& b);
  
  void ratio(const double* t, pair &b, double (*m)(double, double),
             double fuzz, bool &first);
  
  void meshinit() {
    if(billboard)
      centerIndex=centerindex(center);
  }
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  bool write(jsfile *out);
  
  void render(double, const triple& b, const triple& B,
              double perspective, bool remesh);
  drawElement *transformed(const double* t);
};
  
class drawNurbs : public drawElement {
protected:
  size_t udegree,vdegree;
  size_t nu,nv;
  triple *controls;
  double *weights;
  double *uknots, *vknots;
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  triple normal;
  bool invisible;
  
  triple Min,Max;
  
#ifdef HAVE_LIBGLM
  GLfloat *colors;
  GLfloat *Controls;
  GLfloat *uKnots;
  GLfloat *vKnots;
#endif  
  
public:
  drawNurbs(const vm::array& g, const vm::array* uknot, const vm::array* vknot,
            const vm::array* weight, const vm::array&p, double opacity,
            double shininess, double metallic, double fresnel0,
            const vm::array &pens, const string& key="")
    : drawElement(key), opacity(opacity), shininess(shininess),
      metallic(metallic), fresnel0(fresnel0) {
    size_t weightsize=checkArray(weight);
    
    const string wrongsize="Inconsistent NURBS data";
    nu=checkArray(&g);
    
    if(nu == 0 || (weightsize != 0 && weightsize != nu) || checkArray(&p) != 3)
      reportError(wrongsize);
    
    vm::array *g0=vm::read<vm::array*>(g,0);
    nv=checkArray(g0);
    
    size_t n=nu*nv;
    controls=new(UseGC) triple[n];
    
    size_t k=0;
    for(size_t i=0; i < nu; ++i) {
      vm::array *gi=vm::read<vm::array*>(g,i);
      if(checkArray(gi) != nv)  
        reportError(wrongsize);
      for(size_t j=0; j < nv; ++j)
        controls[k++]=vm::read<triple>(gi,j);
    }
      
    if(weightsize > 0) {
      size_t k=0;
      weights=new(UseGC) double[n];
      for(size_t i=0; i < nu; ++i) {
        vm::array *weighti=vm::read<vm::array*>(weight,i);
        if(checkArray(weighti) != nv)  
          reportError(wrongsize);
        for(size_t j=0; j < nv; ++j)
          weights[k++]=vm::read<double>(weighti,j);
      }
    } else weights=NULL;
      
    size_t nuknots=checkArray(uknot);
    size_t nvknots=checkArray(vknot);
    
    if(nuknots <= nu+1 || nuknots > 2*nu || nvknots <= nv+1 || nvknots > 2*nv)
      reportError(wrongsize);

    udegree=nuknots-nu-1;
    vdegree=nvknots-nv-1;
    
    run::copyArrayC(uknots,uknot,0,UseGC);
    run::copyArrayC(vknots,vknot,0,UseGC);
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
    
#ifdef HAVE_LIBGLM
    Controls=NULL;
    int size=checkArray(&pens);
    if(size > 0) {
      colors=new(UseGC) GLfloat[16];
      if(size != 4) reportError(wrongsize);
      storecolor(colors,0,pens,0);
      storecolor(colors,8,pens,1);
      storecolor(colors,12,pens,2);
      storecolor(colors,4,pens,3);
    } else colors=NULL;
#endif  
  }
  
  drawNurbs(const double* t, const drawNurbs *s) :
    drawElement(s->KEY), udegree(s->udegree), vdegree(s->vdegree), nu(s->nu),
    nv(s->nv), weights(s->weights), uknots(s->uknots), vknots(s->vknots),
    diffuse(s->diffuse),
    emissive(s->emissive), specular(s->specular), opacity(s->opacity),
    shininess(s->shininess), invisible(s->invisible) {
    
    const size_t n=nu*nv;
    controls=new(UseGC) triple[n];
    for(unsigned int i=0; i < n; ++i)
      controls[i]=t*s->controls[i];
    
#ifdef HAVE_LIBGLM
    Controls=NULL;
    colors=s->colors;
#endif    
  }
  
  bool is3D() {return true;}
  
  void bounds(const double* t, bbox3& b);
  
  virtual ~drawNurbs() {}

  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  
  void displacement();
  void ratio(const double* t, pair &b, double (*m)(double, double), double,
             bool &first);

  void render(double size2, const triple& b, const triple& B,
              double perspective, bool remesh);
    
  drawElement *transformed(const double* t);
};
  
// Draw a transformed PRC object.
class drawPRC : public drawElementLC {
protected:
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  bool invisible;
public:
  void init(const vm::array&p) {
    if(checkArray(&p) != 3)
      reportError(need3pens);
    
    pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    
    diffuse=rgba(surfacepen);
    emissive=rgba(vm::read<camp::pen>(p,1));
    specular=rgba(vm::read<camp::pen>(p,2));
  }
  
  drawPRC(const vm::array& t, const vm::array&p, double opacity,
          double shininess, double metallic, double fresnel0) :
    drawElementLC(t), opacity(opacity), shininess(shininess),
    metallic(metallic), fresnel0(fresnel0) {
    init(p);
  }
  
  drawPRC(const vm::array&p, double opacity,
          double shininess, double metallic, double fresnel0) :
    drawElementLC(NULL), opacity(opacity), shininess(shininess),
    metallic(metallic), fresnel0(fresnel0) {
    init(p);
  }
  
  drawPRC(const double* t, const drawPRC *s) :
    drawElementLC(t,s), diffuse(s->diffuse),
    emissive(s->emissive), specular(s->specular), opacity(s->opacity),
    shininess(s->shininess), metallic(s->metallic), fresnel0(s->fresnel0),
    invisible(s->invisible) {
  }
  
  virtual void P(triple& t, double x, double y, double z);

  virtual bool write(prcfile *out, unsigned int *, double, groupsmap&) {
    return true;
  }

  virtual bool write(jsfile *out) {return true;}

  virtual void transformedbounds(const double*, bbox3&) {}
  virtual void transformedratio(const double*, pair&,
                                double (*)(double, double), double, bool&) {}

};
  
// Output a unit sphere primitive.
class drawSphere : public drawPRC {
  bool half;
  int type;
public:
  drawSphere(const vm::array& t, bool half, const vm::array&p, double opacity,
             double shininess, double metallic, double fresnel0, int type) :
    drawPRC(t,p,opacity,shininess,metallic,fresnel0), half(half), type(type) {}

  drawSphere(const double* t, const drawSphere *s) :
    drawElement(s->KEY), drawPRC(t,s), half(s->half), type(s->type) {}
    
  void P(triple& t, double x, double y, double z);
  
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  bool write(jsfile *out);
  
  drawElement *transformed(const double* t) {
    return new drawSphere(t,this);
  }
};
  
// Output a unit cylinder primitive.
class drawCylinder : public drawPRC {
  bool core;
public:
  drawCylinder(const vm::array& t, const vm::array&p,
               double opacity, double shininess, double metallic,
               double fresnel0, bool core=false) :
    drawPRC(t,p,opacity,shininess,metallic,fresnel0), core(core) {}

  drawCylinder(const double* t, const drawCylinder *s) :
    drawPRC(t,s), core(s->core) {}
    
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  bool write(jsfile *out);
  
  drawElement *transformed(const double* t) {
    return new drawCylinder(t,this);
  }
};
  
// Draw a unit disk.
class drawDisk : public drawPRC {
public:
  drawDisk(const vm::array& t, const vm::array&p, double opacity,
           double shininess, double metallic, double fresnel0) :
    drawPRC(t,p,opacity,shininess,metallic,fresnel0) {}

  drawDisk(const double* t, const drawDisk *s) :
    drawPRC(t,s) {}
    
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  bool write(jsfile *out);
  
  drawElement *transformed(const double* t) {
    return new drawDisk(t,this);
  }
};
  
// Draw a tube.
class drawTube : public drawPRC {
protected:
  triple *g;
  double width;
  triple m,M;
  bool core;
public:
  drawTube(const vm::array&G, double width, const vm::array&p, double opacity,
           double shininess, double metallic, double fresnel0,
           const triple& m, const triple& M, bool core) :
    drawPRC(p,opacity,shininess,metallic,fresnel0), width(width), m(m), M(M),
    core(core) {
    if(vm::checkArray(&G) != 4)
      reportError("array of 4 triples required");

    g=new(UseGC) triple[4];
    for(size_t i=0; i < 4; ++i)
      g[i]=vm::read<triple>(G,i);
  }
  
  drawTube(const double* t, const drawTube *s) :
    drawElement(s->KEY), drawPRC(t,s), width(s->width), m(s->m), M(s->M),
    core(s->core) {
    g=new(UseGC) triple[4];
    for(size_t i=0; i < 4; ++i)
      g[i]=t*s->g[i];
  }
  
  bool write(jsfile *out);

  drawElement *transformed(const double* t) {
    return new drawTube(t,this);
  }
};

  
class drawBaseTriangles : public drawElement {
protected:
#ifdef HAVE_GL
  Triangles R;
  bool transparent;
#endif  
  
  size_t nP;
  triple* P;
  size_t nN;
  triple* N;
  size_t nI;
  size_t Ni;
  uint32_t (*PI)[3];
  uint32_t (*NI)[3];
  
  triple Min,Max;

  static const string wrongsize;
  static const string outofrange;
    
public:
  drawBaseTriangles(const vm::array& v, const vm::array& vi,
                    const vm::array& n, const vm::array& ni) {
    nP=checkArray(&v);
    P=new(UseGC) triple[nP];
    for(size_t i=0; i < nP; ++i)
      P[i]=vm::read<triple>(v,i);
  
    nI=checkArray(&vi);
    PI=new(UseGC) uint32_t[nI][3];
    for(size_t i=0; i < nI; ++i) {
      vm::array *vii=vm::read<vm::array*>(vi,i);
      if(checkArray(vii) != 3) reportError(wrongsize);
      uint32_t *PIi=PI[i];
      for(size_t j=0; j < 3; ++j) {
        size_t index=unsignedcast(vm::read<Int>(vii,j));
        if(index >= nP) reportError(outofrange);
        PIi[j]=index;
      }
    }
    
    nN=checkArray(&n);
    if(nN) {
      N=new(UseGC) triple[nN];
      for(size_t i=0; i < nN; ++i)
        N[i]=vm::read<triple>(n,i);
    
      Ni=checkArray(&ni);
      if(Ni == 0 && nN == nP)
        NI=PI;
      else {
        if(Ni != nI)
          reportError("Index arrays have different lengths");
        NI=new(UseGC) uint32_t[nI][3];
        for(size_t i=0; i < nI; ++i) {
          vm::array *nii=vm::read<vm::array*>(ni,i);
          if(checkArray(nii) != 3) reportError(wrongsize);
          uint32_t *NIi=NI[i];
          for(size_t j=0; j < 3; ++j) {
            size_t index=unsignedcast(vm::read<Int>(nii,j));
            if(index >= nN) reportError(outofrange);
            NIi[j]=index;
          }
        }
      }
    } else Ni=0;
  }

  drawBaseTriangles(const double* t, const drawBaseTriangles *s) :
    drawElement(s->KEY), nP(s->nP), nN(s->nN), nI(s->nI), Ni(s->Ni) {
    P=new(UseGC) triple[nP];
    for(size_t i=0; i < nP; i++)
      P[i]=t*s->P[i];
    
    PI=new(UseGC) uint32_t[nI][3];
    for(size_t i=0; i < nI; ++i) {
      uint32_t *PIi=PI[i];
      uint32_t *sPIi=s->PI[i];
      for(size_t j=0; j < 3; ++j)
        PIi[j]=sPIi[j];
    }

    if(nN) {
      N=new(UseGC) triple[nN];
      if(t == NULL) {
        for(size_t i=0; i < nN; i++)
          N[i]=s->N[i];
      } else {
        double T[]={t[0],t[1],t[2],
                    t[4],t[5],t[6],
                    t[8],t[9],t[10]};
        run::inverse(T,3);
        for(size_t i=0; i < nN; i++)
          N[i]=unit(Transform3(s->N[i],T));
      }

      if(Ni == 0) {
        NI=PI;
      } else {
        NI=new(UseGC) uint32_t[nI][3];
        for(size_t i=0; i < nI; ++i) {
          uint32_t *NIi=NI[i];
          uint32_t *sNIi=s->NI[i];
          for(size_t j=0; j < 3; ++j)
            NIi[j]=sNIi[j];
        }
      }
    }
  }
    
  bool is3D() {return true;}
  
  void bounds(const double* t, bbox3& b);
  
  void ratio(const double* t, pair &b, double (*m)(double, double),
             double fuzz, bool &first);
  
  virtual ~drawBaseTriangles() {}
  
  drawElement *transformed(const double* t) {
    return new drawBaseTriangles(t,this);
  }
};
  
class drawTriangles : public drawBaseTriangles {
  size_t nC;
  prc::RGBAColour*C;
  uint32_t (*CI)[3];
  size_t Ci;
   
  // Asymptote material data
  prc::RGBAColour diffuse;
  prc::RGBAColour emissive;
  prc::RGBAColour specular;
  double opacity;
  double shininess;
  double metallic;
  double fresnel0;
  bool invisible;
   
public:
  drawTriangles(const vm::array& v, const vm::array& vi,
                const vm::array& n, const vm::array& ni,
                const vm::array&p, double opacity, double shininess,
                double metallic, double fresnel0,
                const vm::array& c, const vm::array& ci) :
    drawBaseTriangles(v,vi,n,ni), opacity(opacity), shininess(shininess),
    metallic(metallic), fresnel0(fresnel0) {

    if(checkArray(&p) != 3)
      reportError(need3pens);
      
    const pen surfacepen=vm::read<camp::pen>(p,0);
    invisible=surfacepen.invisible();
    diffuse=rgba(surfacepen);
    
    nC=checkArray(&c);
    if(nC) {
      C=new(UseGC) prc::RGBAColour[nC];
      for(size_t i=0; i < nC; ++i)
        C[i]=rgba(vm::read<camp::pen>(c,i));
    
      size_t nI=checkArray(&vi);
    
      Ci=checkArray(&ci);
      if(Ci == 0 && nC == nP)
        CI=PI;
      else {
        if(Ci != nI)
          reportError("Index arrays have different lengths");
        CI=new(UseGC) uint32_t[nI][3];
        for(size_t i=0; i < nI; ++i) {
          vm::array *cii=vm::read<vm::array*>(ci,i);
          if(checkArray(cii) != 3) reportError(wrongsize);
          uint32_t *CIi=CI[i];
          for(size_t j=0; j < 3; ++j) {
            size_t index=unsignedcast(vm::read<Int>(cii,j));
            if(index >= nC) reportError(outofrange);
            CIi[j]=index;
          }
        }
      }
    } else {
      emissive=rgba(vm::read<camp::pen>(p,1));
    }
    specular=rgba(vm::read<camp::pen>(p,2));
  }
  
  drawTriangles(const double* t, const drawTriangles *s) :
    drawBaseTriangles(t,s), nC(s->nC),
    diffuse(s->diffuse), emissive(s->emissive),
    specular(s->specular), opacity(s->opacity), shininess(s->shininess), 
    metallic(s->metallic), fresnel0(s->fresnel0), invisible(s->invisible) {
    
    if(nC) {
      C=new(UseGC) prc::RGBAColour[nC];
      for(size_t i=0; i < nC; ++i)
        C[i]=s->C[i];
    
      CI=new(UseGC) uint32_t[nI][3];
      for(size_t i=0; i < nI; ++i) {
        uint32_t *CIi=CI[i];
        uint32_t *sCIi=s->CI[i];
        for(size_t j=0; j < 3; ++j)
          CIi[j]=sCIi[j];
      }
    }
  }
 
  virtual ~drawTriangles() {}
 
  void render(double size2, const triple& b, const triple& B,
              double perspective, bool remesh);
 
  bool write(prcfile *out, unsigned int *, double, groupsmap&);
  bool write(jsfile *out);
 
  drawElement *transformed(const double* t) {
    return new drawTriangles(t,this);
  }
};

}

#endif
