/*****
 * bezierpatch.h
 * Authors: John C. Bowman and Jesse Frohlich
 *
 * Render Bezier patches and triangles.
 *****/

#ifndef BEZIERPATCH_H
#define BEZIERPATCH_H

#include "drawelement.h"

namespace camp {

#ifdef HAVE_GL

struct BezierPatch
{
  vertexBuffer data;

  bool transparent;
  bool color;
  double epsilon;
  double Epsilon;
  double res2;
  double Res2; // Reduced resolution for Bezier triangles flatness test.
  typedef GLuint (vertexBuffer::*vertexFunction)(const triple &v,
                                                 const triple& n);
  vertexFunction pvertex;
  bool Onscreen;

  void init(double res);
    
  triple normal(triple left3, triple left2, triple left1, triple middle,
                triple right1, triple right2, triple right3) {
    triple lp=3.0*(left1-middle);
    triple rp=3.0*(right1-middle);

    triple n=cross(rp,lp);
    if(abs2(n) > epsilon)
      return n;

    triple lpp=bezierPP(middle,left1,left2);
    triple rpp=bezierPP(middle,right1,right2);

    n=cross(rpp,lp)+cross(rp,lpp);
    if(abs2(n) > epsilon)
      return n;

    triple lppp=bezierPPP(middle,left1,left2,left3);
    triple rppp=bezierPPP(middle,right1,right2,right3);

    n=cross(rpp,lpp)+cross(rppp,lp)+cross(rp,lppp);
    if(abs2(n) > epsilon)
      return n;

    n=cross(rppp,lpp)+cross(rpp,lppp);
    if(abs2(n) > epsilon)
      return n;

    return cross(rppp,lppp);
  }

  // Return the differential of the Bezier curve p0,p1,p2,p3 at 0
  triple differential(triple p0, triple p1, triple p2, triple p3) {
    triple p=p1-p0;
    if(abs2(p) > epsilon)
      return p;
    
    p=bezierPP(p0,p1,p2);
    if(abs2(p) > epsilon)
      return p;
    
    return bezierPPP(p0,p1,p2,p3);
  }

  // Determine the flatness of a Bezier patch.
  pair Distance(const triple *p) {
    triple p0=p[0];
    triple p3=p[3];
    triple p12=p[12];
    triple p15=p[15];

    // Check the horizontal flatness.
    double h=Flatness(p0,p12,p3,p15);
    // Check straightness of the horizontal edges and interior control curves.
    h=max(h,Straightness(p0,p[4],p[8],p12));
    h=max(h,Straightness(p[1],p[5],p[9],p[13]));
    h=max(h,Straightness(p[2],p[6],p[10],p[14]));
    h=max(h,Straightness(p3,p[7],p[11],p15));

    // Check the vertical flatness.
    double v=Flatness(p0,p3,p12,p15);
    // Check straightness of the vertical edges and interior control curves.
    v=max(v,Straightness(p0,p[1],p[2],p3));
    v=max(v,Straightness(p[4],p[5],p[6],p[7]));
    v=max(v,Straightness(p[8],p[9],p[10],p[11]));
    v=max(v,Straightness(p12,p[13],p[14],p15));
    
    return pair(h,v);
  }
  
  struct Split3 {
    triple m0,m2,m3,m4,m5;
    Split3() {}
    Split3(triple z0, triple c0, triple c1, triple z1) {
      m0=0.5*(z0+c0);
      triple m1=0.5*(c0+c1);
      m2=0.5*(c1+z1);
      m3=0.5*(m0+m1);
      m4=0.5*(m1+m2);
      m5=0.5*(m3+m4);
    }
  };
  
  // Approximate bounds by bounding box of control polyhedron.
  bool offscreen(size_t n, const triple *v) {
    if(bbox2(n,v).offscreen()) {
      Onscreen=false;
      return true;
    }
    return false;
  }

  virtual void render(const triple *p, bool straight, GLfloat *c0=NULL);
  void render(const triple *p,
              GLuint I0, GLuint I1, GLuint I2, GLuint I3,
              triple P0, triple P1, triple P2, triple P3,
              bool flat0, bool flat1, bool flat2, bool flat3,
              GLfloat *C0=NULL, GLfloat *C1=NULL, GLfloat *C2=NULL,
              GLfloat *C3=NULL);
  
  void append() {
    if(transparent)
      transparentData.Append(data);
    else {
      if(color)
        colorData.Append(data);
      else
        materialData.append(data);
    }
  }
  
  void queue(const triple *g, bool straight, double ratio, bool Transparent,
             GLfloat *colors=NULL) {
    data.clear();
    Onscreen=true;
    transparent=Transparent;
    color=colors;
    init(pixel*ratio);
    render(g,straight,colors);
  }
  
};

struct BezierTriangle : public BezierPatch {
public:
  BezierTriangle() : BezierPatch() {}
  
  double Distance(const triple *p) {
    triple p0=p[0];
    triple p6=p[6];
    triple p9=p[9];
    
    // Check how far the internal point is from the centroid of the vertices.
    double d=abs2((p0+p6+p9)*third-p[4]);

    // Determine how straight the edges are.
    d=max(d,Straightness(p0,p[1],p[3],p6));
    d=max(d,Straightness(p0,p[2],p[5],p9));
    return max(d,Straightness(p6,p[7],p[8],p9));
  }
  
  void render(const triple *p, bool straight, GLfloat *c0=NULL);
  void render(const triple *p,
              GLuint I0, GLuint I1, GLuint I2,
              triple P0, triple P1, triple P2,
              bool flat0, bool flat1, bool flat2,
              GLfloat *C0=NULL, GLfloat *C1=NULL, GLfloat *C2=NULL);
};

struct Triangles : public BezierPatch {
public:
  Triangles() : BezierPatch() {}

  void queue(size_t nP, const triple* P, size_t nN, const triple* N,
             size_t nC, const prc::RGBAColour* C, size_t nI,
             const uint32_t (*PI)[3], const uint32_t (*NI)[3],
             const uint32_t (*CI)[3], bool transparent);
  
  void append() {
    if(transparent)
      transparentData.Append(data);
    else
      triangleData.Append(data);
  }

};

extern void sortTriangles();

#endif

} //namespace camp

#endif
