/*****
 * beziercurve.h
 * Author: John C. Bowman
 *
 * Render a Bezier curve.
 *****/

#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include "drawelement.h"

namespace camp {

#ifdef HAVE_GL

extern const double Fuzz;
extern const double Fuzz2;

struct BezierCurve
{
  vertexBuffer data;
  double res,res2;
  bool Onscreen;
  
  void init(double res);
    
  // Approximate bounds by bounding box of control polyhedron.
  bool offscreen(size_t n, const triple *v) {
    if(bbox2(n,v).offscreen()) {
      Onscreen=false;
      return true;
    }
    return false;
  }

  void render(const triple *p, bool straight);
  void render(const triple *p, GLuint I0, GLuint I1);
  
  void append() {
    material1Data.append(data);
  }
  
  void queue(const triple *g, bool straight, double ratio) {
    data.clear();
    Onscreen=true;
    init(pixel*ratio);
    render(g,straight);
  }
  
};

struct Pixel
{
  vertexBuffer data;
  
  void append() {
    material0Data.append0(data);
  }
  
  void queue(const triple& p, double width) {
    data.clear();
    MaterialIndex=materialIndex;
    data.indices.push_back(data.vertex0(p,width));
    append();
  }
  
  void draw();
};

#endif

} //namespace camp

#endif
