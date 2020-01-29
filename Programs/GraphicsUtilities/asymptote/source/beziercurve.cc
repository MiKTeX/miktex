/*****
 * drawbezierpatch.cc
 * Author: John C. Bowman
 *
 * Render a Bezier curve.
 *****/

#include "bezierpatch.h"
#include "beziercurve.h"

namespace camp {

#ifdef HAVE_GL

void BezierCurve::init(double res)
{
  this->res=res;
  res2=res*res;
  
  MaterialIndex=materialIndex;
}

void BezierCurve::render(const triple *p, bool straight) 
{
  GLuint i0=data.vertex1(p[0]);
  GLuint i3=data.vertex1(p[3]);
    
  if(straight) {
    std::vector<GLuint> &q=data.indices;
    q.push_back(i0);
    q.push_back(i3);
  } else
    render(p,i0,i3);
  append();
}
  
// Use a uniform partition to draw a Bezier patch.
// p is an array of 4 triples representing the control points.
// Ii are the vertices indices.
void BezierCurve::render(const triple *p, GLuint I0, GLuint I1)
{
  triple p0=p[0];
  triple p1=p[1];
  triple p2=p[2];
  triple p3=p[3];
  if(Straightness(p0,p1,p2,p3) < res2) { // Segment is flat
    triple P[]={p0,p3};
    if(!offscreen(2,P)) {
      std::vector<GLuint> &q=data.indices;
      q.push_back(I0);
      q.push_back(I1);
    }
  } else { // Segment is not flat
    if(offscreen(4,p)) return;
    triple m0=0.5*(p0+p1);
    triple m1=0.5*(p1+p2);
    triple m2=0.5*(p2+p3);
    triple m3=0.5*(m0+m1);
    triple m4=0.5*(m1+m2);
    triple m5=0.5*(m3+m4);
      
    triple s0[]={p0,m0,m3,m5};
    triple s1[]={m5,m4,m2,p3};
      
    GLuint i0=data.vertex1(m5);
      
    render(s0,I0,i0);
    render(s1,i0,I1);
  }
}

#endif

} //namespace camp
