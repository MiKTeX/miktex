/*****
 * drawsurface.cc
 *
 * Stores a surface that has been added to a picture.
 *****/

#include "drawsurface.h"
#include "drawpath3.h"
#include "arrayop.h"

#include <iostream>
#include <iomanip>
#include <fstream>

#ifdef HAVE_LIBGLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

using namespace prc;
#include "material.h"

namespace camp {

mem::vector<triple> drawElement::center;
size_t drawElement::centerIndex=0;
triple drawElement::lastcenter=0;
size_t drawElement::lastcenterIndex=0;

const triple drawElement::zero;

using vm::array;

#ifdef HAVE_LIBGLM

void storecolor(GLfloat *colors, int i, const vm::array &pens, int j)
{
  pen p=vm::read<camp::pen>(pens,j);
  p.torgb();
  colors[i]=p.red();
  colors[i+1]=p.green();
  colors[i+2]=p.blue();
  colors[i+3]=p.opacity();
}

void storecolor(GLfloat *colors, int i, const RGBAColour& p)
{
  colors[i]=p.R;
  colors[i+1]=p.G;
  colors[i+2]=p.B;
  colors[i+3]=p.A;
}

void setcolors(bool colors,
               const RGBAColour& diffuse,
               const RGBAColour& emissive,
               const RGBAColour& specular, double shininess,
               double metallic, double fresnel0, jsfile *out)
{
  Material m=Material(glm::vec4(diffuse.R,diffuse.G,diffuse.B,diffuse.A),
                      glm::vec4(emissive.R,emissive.G,emissive.B,emissive.A),
                      glm::vec4(specular.R,specular.G,specular.B,specular.A),
                      shininess,metallic,fresnel0);
  
  MaterialMap::iterator p=materialMap.find(m);
  if(p != materialMap.end()) materialIndex=p->second;
  else {
    materialIndex=material.size();
    if(materialIndex >= nmaterials)
      nmaterials=min(Maxmaterials,2*nmaterials);
    material.push_back(m);
    materialMap[m]=materialIndex;
    if(out)
      out->addMaterial(materialIndex);
  }
}

#endif  

void drawBezierPatch::bounds(const double* t, bbox3& b)
{
  double x,y,z;
  double X,Y,Z;

  if(straight) {
    triple Vertices[4];
    if(t == NULL) {
      Vertices[0]=controls[0];
      Vertices[1]=controls[3];
      Vertices[2]=controls[12];
      Vertices[3]=controls[15];
    } else {
      Vertices[0]=t*controls[0];
      Vertices[1]=t*controls[3];
      Vertices[2]=t*controls[12];
      Vertices[3]=t*controls[15];
    }

    boundstriples(x,y,z,X,Y,Z,4,Vertices);
  } else {
    double cx[16];
    double cy[16];
    double cz[16];

    if(t == NULL) {
      for(int i=0; i < 16; ++i) {
        triple v=controls[i];
        cx[i]=v.getx();
        cy[i]=v.gety();
        cz[i]=v.getz();
      }
    } else {
      for(int i=0; i < 16; ++i) {
        triple v=t*controls[i];
        cx[i]=v.getx();
        cy[i]=v.gety();
        cz[i]=v.getz();
      }
    }

    double c0=cx[0];
    double fuzz=Fuzz*run::norm(cx,16);
    x=bound(cx,min,b.empty ? c0 : min(c0,b.left),fuzz,maxdepth);
    X=bound(cx,max,b.empty ? c0 : max(c0,b.right),fuzz,maxdepth);

    c0=cy[0];
    fuzz=Fuzz*run::norm(cy,16);
    y=bound(cy,min,b.empty ? c0 : min(c0,b.bottom),fuzz,maxdepth);
    Y=boundtri(cy,max,b.empty ? c0 : max(c0,b.top),fuzz,maxdepth);

    c0=cz[0];
    fuzz=Fuzz*run::norm(cz,16);
    z=bound(cz,min,b.empty ? c0 : min(c0,b.near),fuzz,maxdepth);
    Z=bound(cz,max,b.empty ? c0 : max(c0,b.far),fuzz,maxdepth);
  }

  b.add(x,y,z);
  b.add(X,Y,Z);

  if(t == NULL) {
    Min=triple(x,y,z);
    Max=triple(X,Y,Z);
  }
}

void drawBezierPatch::ratio(const double* t, pair &b, double (*m)(double, double),
                        double fuzz, bool &first)
{
  triple buf[16];
  triple* Controls;
  if(straight) {
    if(t == NULL) Controls=controls;
    else {
      Controls=buf;
      Controls[0]=t*controls[0];
      Controls[3]=t*controls[3];
      Controls[12]=t*controls[12];
      Controls[15]=t*controls[15];
    }
  
    triple v=Controls[0];
    double x=xratio(v);
    double y=yratio(v);
    if(first) {
      first=false;
      b=pair(x,y);
    } else {
      x=m(b.getx(),x);
      y=m(b.gety(),y);
    }
    v=Controls[3];
    x=m(x,xratio(v));
    y=m(y,yratio(v));
    v=Controls[12];
    x=m(x,xratio(v));
    y=m(y,yratio(v));
    v=Controls[15];
    x=m(x,xratio(v));
    y=m(y,yratio(v));
    b=pair(x,y);
  } else {
    if(t == NULL) Controls=controls;
    else {
      Controls=buf;
      for(unsigned int i=0; i < 16; ++i)
        Controls[i]=t*controls[i];
    }

    if(first) {
      triple v=Controls[0];
      b=pair(xratio(v),yratio(v));
      first=false;
    }
  
    b=pair(bound(Controls,m,xratio,b.getx(),fuzz,maxdepth),
           bound(Controls,m,yratio,b.gety(),fuzz,maxdepth));
  }
}

bool drawBezierPatch::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible || primitive)
    return true;

  RGBAColour Black(0.0,0.0,0.0,diffuse.A);
  PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);

  if(straight) {
    triple vertices[]={controls[0],controls[12],controls[3],controls[15]};
    if(colors) {
      prc::RGBAColour Colors[]={colors[0],colors[1],colors[3],colors[2]};
      out->addQuad(vertices,Colors);
    } else
      out->addRectangle(vertices,m);
  } else
    out->addPatch(controls,m);
                    
  return true;
}

bool drawBezierPatch::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible || primitive)
    return true;

  if(billboard) {
    meshinit();
    drawElement::centerIndex=centerIndex;
  } else drawElement::centerIndex=0;
  
  setcolors(colors,diffuse,emissive,specular,shininess,metallic,fresnel0,out);
  
  if(straight) {
    triple Controls[]={controls[0],controls[12],controls[15],controls[3]};
    out->addPatch(Controls,4,Min,Max,colors,4);
  } else
    out->addPatch(controls,16,Min,Max,colors,4);
                    
#endif  
  return true;
}

void drawBezierPatch::render(double size2, const triple& b, const triple& B,
                             double perspective, bool remesh)
{
#ifdef HAVE_GL
  if(invisible) return; 
  transparent=colors ? colors[0].A+colors[1].A+colors[2].A+colors[3].A < 4.0 :
    diffuse.A < 1.0;
  
  setcolors(colors,diffuse,emissive,specular,shininess,metallic,fresnel0);
  
  if(transparent)
    setMaterial(transparentData,drawTransparent);
  else {
    if(colors)
      setMaterial(colorData,drawColor);
    else
      setMaterial(materialData,drawMaterial);
  }
  
  bool offscreen;
  if(gl::exporting)
    offscreen=false;
  else if(billboard) {
    drawElement::centerIndex=centerIndex;
    BB.init(center);
    offscreen=bbox2(Min,Max,BB).offscreen();
  } else
    offscreen=bbox2(Min,Max).offscreen();

  if(offscreen) { // Fully offscreen
    S.Onscreen=false;
    S.data.clear();
    return;
  }

  triple *Controls;
  triple Controls0[16];
  if(billboard) {
    Controls=Controls0;
    for(size_t i=0; i < 16; i++) {
     Controls[i]=BB.transform(controls[i]);
    }
  } else {
    Controls=controls;
    if(!remesh && S.Onscreen) { // Fully onscreen; no need to re-render
      S.append();
      return;
    }
  }

  double s=perspective ? Min.getz()*perspective : 1.0; // Move to glrender
    
  const pair size3(s*(B.getx()-b.getx()),s*(B.gety()-b.gety()));

  if(gl::outlinemode) {
    setMaterial(material1Data,drawMaterial);
    triple edge0[]={Controls[0],Controls[4],Controls[8],Controls[12]};
    C.queue(edge0,straight,size3.length()/size2);
    triple edge1[]={Controls[12],Controls[13],Controls[14],Controls[15]};
    C.queue(edge1,straight,size3.length()/size2);
    triple edge2[]={Controls[15],Controls[11],Controls[7],Controls[3]};
    C.queue(edge2,straight,size3.length()/size2);
    triple edge3[]={Controls[3],Controls[2],Controls[1],Controls[0]};
    C.queue(edge3,straight,size3.length()/size2);
  } else {
    GLfloat c[16];
    if(colors)
      for(size_t i=0; i < 4; ++i)
        storecolor(c,4*i,colors[i]);

    S.queue(Controls,straight,size3.length()/size2,transparent,
            colors ? c : NULL);
  }
#endif
}

drawElement *drawBezierPatch::transformed(const double* t)
{
  return new drawBezierPatch(t,this);
}
  
void drawBezierTriangle::bounds(const double* t, bbox3& b)
{
  double x,y,z;
  double X,Y,Z;

  if(straight) {
    triple Vertices[3];
    if(t == NULL) {
      Vertices[0]=controls[0];
      Vertices[1]=controls[6];
      Vertices[2]=controls[9];
    } else {
      Vertices[0]=t*controls[0];
      Vertices[1]=t*controls[6];
      Vertices[2]=t*controls[9];
    }
  
    boundstriples(x,y,z,X,Y,Z,3,Vertices);
  } else {  
    double cx[10];
    double cy[10];
    double cz[10];
    
    if(t == NULL) {
      for(unsigned int i=0; i < 10; ++i) {
        triple v=controls[i];
        cx[i]=v.getx();
        cy[i]=v.gety();
        cz[i]=v.getz();
      }
    } else {
      for(unsigned int i=0; i < 10; ++i) {
        triple v=t*controls[i];
        cx[i]=v.getx();
        cy[i]=v.gety();
        cz[i]=v.getz();
      }
    }
    
    double c0=cx[0];
    double fuzz=Fuzz*run::norm(cx,10);
    x=boundtri(cx,min,b.empty ? c0 : min(c0,b.left),fuzz,maxdepth);
    X=boundtri(cx,max,b.empty ? c0 : max(c0,b.right),fuzz,maxdepth);
    
    c0=cy[0];
    fuzz=Fuzz*run::norm(cy,10);
    y=boundtri(cy,min,b.empty ? c0 : min(c0,b.bottom),fuzz,maxdepth);
    Y=boundtri(cy,max,b.empty ? c0 : max(c0,b.top),fuzz,maxdepth);
    
    c0=cz[0];
    fuzz=Fuzz*run::norm(cz,10);
    z=boundtri(cz,min,b.empty ? c0 : min(c0,b.near),fuzz,maxdepth);
    Z=boundtri(cz,max,b.empty ? c0 : max(c0,b.far),fuzz,maxdepth);
  }
    
  b.add(x,y,z);
  b.add(X,Y,Z);

  if(t == NULL) {
    Min=triple(x,y,z);
    Max=triple(X,Y,Z);
  }
}

void drawBezierTriangle::ratio(const double* t, pair &b,
                               double (*m)(double, double), double fuzz,
                               bool &first)
{
  triple buf[10];
  triple* Controls;
  if(straight) {
    if(t == NULL) Controls=controls;
    else {
      Controls=buf;
      Controls[0]=t*controls[0];
      Controls[6]=t*controls[6];
      Controls[9]=t*controls[9];
    }
  
    triple v=Controls[0];
    double x=xratio(v);
    double y=yratio(v);
    if(first) {
      first=false;
      b=pair(x,y);
    } else {
      x=m(b.getx(),x);
      y=m(b.gety(),y);
    }
    v=Controls[6];
    x=m(x,xratio(v));
    y=m(y,yratio(v));
    v=Controls[9];
    x=m(x,xratio(v));
    y=m(y,yratio(v));
    b=pair(x,y);
  } else {
    if(t == NULL) Controls=controls;
    else {
      Controls=buf;
      for(unsigned int i=0; i < 10; ++i)
        Controls[i]=t*controls[i];
    }

    if(first) {
      triple v=Controls[0];
      b=pair(xratio(v),yratio(v));
      first=false;
    }
  
    b=pair(boundtri(Controls,m,xratio,b.getx(),fuzz,maxdepth),
           boundtri(Controls,m,yratio,b.gety(),fuzz,maxdepth));
  }
}

bool drawBezierTriangle::write(prcfile *out, unsigned int *, double, 
                               groupsmap&)
{
  if(invisible || primitive)
    return true;

  RGBAColour Black(0.0,0.0,0.0,diffuse.A);
  PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);
  
  static const double third=1.0/3.0;
  static const double third2=2.0/3.0;
  triple Controls[]={controls[0],controls[0],controls[0],controls[0],
                     controls[1],third2*controls[1]+third*controls[2],
                     third*controls[1]+third2*controls[2],
                     controls[2],controls[3],
                     third*controls[3]+third2*controls[4],
                     third2*controls[4]+third*controls[5],
                     controls[5],controls[6],controls[7],
                     controls[8],controls[9]};
  out->addPatch(Controls,m);
                    
  return true;
}

bool drawBezierTriangle::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible || primitive)
    return true;

  if(billboard) {
    meshinit();
    drawElement::centerIndex=centerIndex;
  } else drawElement::centerIndex=0;
  
  setcolors(colors,diffuse,emissive,specular,shininess,metallic,fresnel0,out);
  
  if(straight) {
    triple Controls[]={controls[0],controls[6],controls[9]};
    out->addPatch(Controls,3,Min,Max,colors,3);
  } else
    out->addPatch(controls,10,Min,Max,colors,3);
                    
#endif  
  return true;
}

void drawBezierTriangle::render(double size2, const triple& b, const triple& B,
                                double perspective, bool remesh)
{
#ifdef HAVE_GL
  if(invisible) return;
  transparent=colors ? colors[0].A+colors[1].A+colors[2].A < 3.0 :
    diffuse.A < 1.0;
  
  setcolors(colors,diffuse,emissive,specular,shininess,metallic,fresnel0);
  
  if(transparent)
    setMaterial(transparentData,drawTransparent);
  else {
    if(colors)
      setMaterial(colorData,drawColor);
    else
      setMaterial(materialData,drawMaterial);
  }

  bool offscreen;
  if(gl::exporting)
    offscreen=false;
  else if(billboard) {
    drawElement::centerIndex=centerIndex;
    BB.init(center);
    offscreen=bbox2(Min,Max,BB).offscreen();
  } else
    offscreen=bbox2(Min,Max).offscreen();
  
  if(offscreen) { // Fully offscreen
    S.Onscreen=false;
    S.data.clear();
    return;
  }

  triple *Controls;
  triple Controls0[10];
  if(billboard) {
    Controls=Controls0;
    for(size_t i=0; i < 10; i++) {
     Controls[i]=BB.transform(controls[i]);
    }
  } else {
    Controls=controls;
    if(!remesh && S.Onscreen) { // Fully onscreen; no need to re-render
      S.append();
      return;
    }
  }

  double s=perspective ? Min.getz()*perspective : 1.0; // Move to glrender
    
  const pair size3(s*(B.getx()-b.getx()),s*(B.gety()-b.gety()));

  if(gl::outlinemode) {
    setMaterial(material1Data,drawMaterial);
    triple edge0[]={Controls[0],Controls[1],Controls[3],Controls[6]};
    C.queue(edge0,straight,size3.length()/size2);
    triple edge1[]={Controls[6],Controls[7],Controls[8],Controls[9]};
    C.queue(edge1,straight,size3.length()/size2);
    triple edge2[]={Controls[9],Controls[5],Controls[2],Controls[0]};
    C.queue(edge2,straight,size3.length()/size2);
  } else {
    GLfloat c[12];
    if(colors)
      for(size_t i=0; i < 3; ++i)
        storecolor(c,4*i,colors[i]);

    S.queue(Controls,straight,size3.length()/size2,transparent,
            colors ? c : NULL);
  }
#endif
}

drawElement *drawBezierTriangle::transformed(const double* t)
{
  return new drawBezierTriangle(t,this);
}
  
bool drawNurbs::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  RGBAColour Black(0.0,0.0,0.0,diffuse.A);
  PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);
  out->addSurface(udegree,vdegree,nu,nv,controls,uknots,vknots,m,weights);
  
  return true;
}

// Approximate bounds by bounding box of control polyhedron.
void drawNurbs::bounds(const double* t, bbox3& b)
{
  double x,y,z;
  double X,Y,Z;
  
  const size_t n=nu*nv;
  triple* Controls;
  if(t == NULL) Controls=controls;
  else {
    Controls=new triple[n];
    for(size_t i=0; i < n; i++)
      Controls[i]=t*controls[i];
  }

  boundstriples(x,y,z,X,Y,Z,n,Controls);
  
  b.add(x,y,z);
  b.add(X,Y,Z);
  
  if(t == NULL) {
    Min=triple(x,y,z);
    Max=triple(X,Y,Z);
  } else delete[] Controls;
}

drawElement *drawNurbs::transformed(const double* t)
{
  return new drawNurbs(t,this);
}

void drawNurbs::ratio(const double *t, pair &b, double (*m)(double, double),
                      double, bool &first)
{
  const size_t n=nu*nv;
  
  triple* Controls;
  if(t == NULL) Controls=controls;
  else {
    Controls=new triple[n];
    for(unsigned int i=0; i < n; ++i)
      Controls[i]=t*controls[i];
  }

  if(first) {
    first=false;
    triple v=Controls[0];
    b=pair(xratio(v),yratio(v));
  }
  
  double x=b.getx();
  double y=b.gety();
  for(size_t i=0; i < n; ++i) {
    triple v=Controls[i];
    x=m(x,xratio(v));
    y=m(y,yratio(v));
  }
  b=pair(x,y);
  
  if(t != NULL)
    delete[] Controls;
}


void drawNurbs::displacement()
{
#ifdef HAVE_GL
  size_t n=nu*nv;
  size_t nuknots=udegree+nu+1;
  size_t nvknots=vdegree+nv+1;
    
  if(Controls == NULL) {
    Controls=new(UseGC)  GLfloat[(weights ? 4 : 3)*n];
    uKnots=new(UseGC) GLfloat[nuknots];
    vKnots=new(UseGC) GLfloat[nvknots];
  }
  
  if(weights)
    for(size_t i=0; i < n; ++i)
      store(Controls+4*i,controls[i],weights[i]);
  else
    for(size_t i=0; i < n; ++i)
      store(Controls+3*i,controls[i]);
  
  for(size_t i=0; i < nuknots; ++i)
    uKnots[i]=uknots[i];
  for(size_t i=0; i < nvknots; ++i)
    vKnots[i]=vknots[i];
#endif  
}

void drawNurbs::render(double size2, const triple& b, const triple& B,
                       double perspective, bool remesh)
{
// TODO: implement NURBS renderer
}

void drawPRC::P(triple& t, double x, double y, double z)
{
  if(T == NULL) {
    t=triple(x,y,z);
    return;
  }

  double f=T[12]*x+T[13]*y+T[14]*z+T[15];
  if(f == 0.0) run::dividebyzero();
  f=1.0/f;
  
  t=triple((T[0]*x+T[1]*y+T[2]*z+T[3])*f,(T[4]*x+T[5]*y+T[6]*z+T[7])*f,
           (T[8]*x+T[9]*y+T[10]*z+T[11])*f);
}

void drawSphere::P(triple& t, double x, double y, double z)
{
  if(half) {
    double temp=z; z=x; x=-temp;
  }
  drawPRC::P(t,x,y,z);
}

bool drawSphere::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  RGBAColour Black(0.0,0.0,0.0,diffuse.A);
  PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);
  
  switch(type) {
    case 0: // PRCsphere
    {
      if(half) 
        out->addHemisphere(1.0,m,NULL,NULL,NULL,1.0,T);
      else
        out->addSphere(1.0,m,NULL,NULL,NULL,1.0,T);
      break;
    }
    case 1: // NURBSsphere
    {
      static double uknot[]={0.0,0.0,1.0/3.0,0.5,1.0,1.0};
      static double vknot[]={0.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0};
      static double Weights[12]={2.0/3.0,2.0/9.0,2.0/9.0,2.0/3.0,
                                 1.0/3.0,1.0/9.0,1.0/9.0,1.0/3.0,
                                 1.0,1.0/3.0,1.0/3.0,1.0};

// NURBS representation of a sphere using 10 distinct control points
// K. Qin, J. Comp. Sci. and Tech. 12, 210-216 (1997).
  
      triple N,S,P1,P2,P3,P4,P5,P6,P7,P8;
  
      P(N,0.0,0.0,1.0);
      P(P1,-2.0,-2.0,1.0);
      P(P2,-2.0,-2.0,-1.0);
      P(S,0.0,0.0,-1.0);
      P(P3,2.0,-2.0,1.0);
      P(P4,2.0,-2.0,-1.0);
      P(P5,2.0,2.0,1.0);
      P(P6,2.0,2.0,-1.0);
      P(P7,-2.0,2.0,1.0);
      P(P8,-2.0,2.0,-1.0);
        
      triple p0[]={N,P1,P2,S,
                   N,P3,P4,S,
                   N,P5,P6,S,
                   N,P7,P8,S,
                   N,P1,P2,S,
                   N,P3,P4,S};
   
      out->addSurface(2,3,3,4,p0,uknot,vknot,m,Weights);
      out->addSurface(2,3,3,4,p0+4,uknot,vknot,m,Weights);
      if(!half) {
        out->addSurface(2,3,3,4,p0+8,uknot,vknot,m,Weights);
        out->addSurface(2,3,3,4,p0+12,uknot,vknot,m,Weights);
      }
      
      break;
    }
    default:
      reportError("Invalid sphere type");
  }
  
  return true;
}

bool drawSphere::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;

  drawElement::centerIndex=0;

  setcolors(false,diffuse,emissive,specular,shininess,metallic,fresnel0,out);

  triple O,E;
  P(E,1.0,0.0,0.0);
  P(O,0.0,0.0,0.0);
  triple X=E-O;
  double r=length(X);

  if(half)
    out->addSphere(O,r,half,X.polar(false),X.azimuth(false));
  else
    out->addSphere(O,r);

#endif  
  return true;
}

bool drawCylinder::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  RGBAColour Black(0.0,0.0,0.0,diffuse.A);
  PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);
  
  out->addCylinder(1.0,1.0,m,NULL,NULL,NULL,1.0,T);
  
  return true;
}
  
bool drawCylinder::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;

  drawElement::centerIndex=0;
  
  setcolors(false,diffuse,emissive,specular,shininess,metallic,fresnel0,out);

  triple E,H,O;
  P(E,1.0,0.0,0.0);
  P(H,0.0,0.0,1.0);
  P(O,0.0,0.0,0.0);
  triple X=E-O;
  triple Z=H-O;
  double r=length(X);
  double h=length(Z);
  
  out->addCylinder(O,r,h,Z.polar(false),Z.azimuth(false),core);

#endif  
  return true;
}
  
bool drawDisk::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  RGBAColour Black(0.0,0.0,0.0,diffuse.A);
  PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);
  
  out->addDisk(1.0,m,NULL,NULL,NULL,1.0,T);
  
  return true;
}
  
bool drawDisk::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;

  drawElement::centerIndex=0;
  
  setcolors(false,diffuse,emissive,specular,shininess,metallic,fresnel0,out);
  
  triple E,H,O;
  P(E,1.0,0.0,0.0);
  P(H,0.0,0.0,1.0);
  P(O,0.0,0.0,0.0);
  triple X=E-O;
  triple Z=H-O;
  double r=length(X);
  
  out->addDisk(O,r,Z.polar(false),Z.azimuth(false));

#endif
  return true;
}
  
bool drawTube::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;

  drawElement::centerIndex=0;

  setcolors(false,diffuse,emissive,specular,shininess,metallic,fresnel0,out);

  bbox3 b;
  b.add(T*m);
  b.add(T*triple(m.getx(),m.gety(),M.getz()));
  b.add(T*triple(m.getx(),M.gety(),m.getz()));
  b.add(T*triple(m.getx(),M.gety(),M.getz()));
  b.add(T*triple(M.getx(),m.gety(),m.getz()));
  b.add(T*triple(M.getx(),m.gety(),M.getz()));
  b.add(T*triple(M.getx(),M.gety(),m.getz()));
  b.add(T*M);

  out->addTube(g,width,b.Min(),b.Max(),core);

#endif
  return true;
}

const string drawBaseTriangles::wrongsize=
  "triangle indices require 3 components";
const string drawBaseTriangles::outofrange="index out of range";

void drawBaseTriangles::bounds(const double* t, bbox3& b)
{
  double x,y,z;
  double X,Y,Z;
  triple* tP;

  if(t == NULL) tP=P;
  else {
    tP=new triple[nP];
    for(size_t i=0; i < nP; i++)
      tP[i]=t*P[i];
  }

  boundstriples(x,y,z,X,Y,Z,nP,tP);

  b.add(x,y,z);
  b.add(X,Y,Z);

  if(t == NULL) {
    Min=triple(x,y,z);
    Max=triple(X,Y,Z);
  } else delete[] tP;
}

void drawBaseTriangles::ratio(const double* t, pair &b,
                              double (*m)(double, double), double fuzz,
                              bool &first)
{
  triple* tP;

  if(t == NULL) tP=P;
  else {
    tP=new triple[nP];
    for(size_t i=0; i < nP; i++)
      tP[i]=t*P[i];
  }

  ratiotriples(b,m,first,nP,tP);
  
  if(t != NULL)
    delete[] tP;
}

bool drawTriangles::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;
  
  if(nC) {
    const RGBAColour white(1,1,1,opacity);
    const RGBAColour black(0,0,0,opacity);
    const PRCmaterial m(black,white,black,specular,opacity,shininess);
    out->addTriangles(nP,P,nI,PI,m,nN,N,NI,0,NULL,NULL,nC,C,CI,0,NULL,NULL,30);
  } else {
    RGBAColour Black(0.0,0.0,0.0,diffuse.A);
    const PRCmaterial m(Black,diffuse,emissive,specular,opacity,shininess);
    out->addTriangles(nP,P,nI,PI,m,nN,N,NI,0,NULL,NULL,0,NULL,NULL,0,NULL,NULL,30);
  }

  return true;
}

bool drawTriangles::write(jsfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;
  
  setcolors(nC,diffuse,emissive,specular,shininess,metallic,fresnel0,out);
  
  out->addTriangles(nP,P,nN,N,nC,C,nI,PI,NI,CI,Min,Max);
#endif 
  return true;
}

void drawTriangles::render(double size2, const triple& b,
                           const triple& B, double perspective,
                           bool remesh)
{
#ifdef HAVE_GL
  if(invisible) return;
  
  transparent=diffuse.A < 1.0;

  if(!gl::exporting && bbox2(Min,Max).offscreen()) { // Fully offscreen
    R.Onscreen=false;
    R.data.clear();
    return;
  }

  setcolors(nC,diffuse,emissive,specular,shininess,metallic,fresnel0);

  if(transparent)
    setMaterial(transparentData,drawTransparent);
  else
    setMaterial(triangleData,drawTriangle);

  if(!remesh && R.Onscreen) { // Fully onscreen; no need to re-render
    R.append();
    return;
  }
    
  R.queue(nP,P,nN,N,nC,C,nI,PI,NI,CI,transparent);
#endif
}

} //namespace camp
