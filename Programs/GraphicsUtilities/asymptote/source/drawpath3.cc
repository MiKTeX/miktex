/*****
 * drawpath3.cc
 *
 * Stores a path3 that has been added to a picture.
 *****/

#include "drawpath3.h"
#include "drawsurface.h"
#include "material.h"

#ifdef HAVE_LIBGLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

namespace camp {

using vm::array;
using namespace prc;

bool drawPath3::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  if(straight) {
    triple controls[]={g.point((Int) 0),g.point((Int) 1)};
    out->addLine(2,controls,diffuse);
  } else {
    triple controls[]={g.point((Int) 0),g.postcontrol((Int) 0),
                       g.precontrol((Int) 1),g.point((Int) 1)};
    out->addBezierCurve(4,controls,diffuse);
  }

  return true;
}

bool drawPath3::write(abs3Doutfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;

  if(billboard) {
    meshinit();
    drawElement::centerIndex=centerIndex;
  } else drawElement::centerIndex=0;

  setcolors(diffuse,emissive,specular,shininess,metallic,fresnel0,out);

  if(straight)
    out->addCurve(g.point((Int) 0),g.point((Int) 1),Min,Max);
  else
    out->addCurve(g.point((Int) 0),g.postcontrol((Int) 0),
                  g.precontrol((Int) 1),g.point((Int) 1),Min,Max);
#endif
  return true;
}

void drawPath3::render(double size2, const triple& b, const triple& B,
                       double perspective, bool remesh)
{
#ifdef HAVE_GL
  if(invisible) return;

  setcolors(diffuse,emissive,specular,shininess,metallic,fresnel0);

  setMaterial(material1Data,drawMaterial1);

  bool offscreen;
  if(billboard) {
    drawElement::centerIndex=centerIndex;
    BB.init(center);
    offscreen=bbox2(Min,Max,BB).offscreen();
  } else
    offscreen=bbox2(Min,Max).offscreen();

  if(offscreen) { // Fully offscreen
    R.Onscreen=false;
    R.data.clear();
    R.notRendered();
    return;
  }

  triple controls[]={g.point((Int) 0),g.postcontrol((Int) 0),
                     g.precontrol((Int) 1),g.point((Int) 1)};
  triple *Controls;
  triple Controls0[4];
  if(billboard) {
    Controls=Controls0;
    for(size_t i=0; i < 4; i++)
      Controls[i]=BB.transform(controls[i]);
  } else {
    Controls=controls;
    if(!remesh && R.Onscreen) { // Fully onscreen; no need to re-render
      R.append();
      return;
    }
  }

  double s=perspective ? Min.getz()*perspective : 1.0; // Move to glrender

  const pair size3(s*(B.getx()-b.getx()),s*(B.gety()-b.gety()));

  R.queue(controls,straight,size3.length()/size2);
#endif
}

drawElement *drawPath3::transformed(const double* t)
{
  return new drawPath3(t,this);
}

bool drawNurbsPath3::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  out->addCurve(degree,n,controls,knots,color,weights);

  return true;
}

// Approximate bounds by bounding box of control polyhedron.
void drawNurbsPath3::bounds(const double* t, bbox3& b)
{
  double x,y,z;
  double X,Y,Z;

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

drawElement *drawNurbsPath3::transformed(const double* t)
{
  return new drawNurbsPath3(t,this);
}

void drawNurbsPath3::ratio(const double* t, pair &b, double (*m)(double, double),
                           double, bool &first)
{
  triple* Controls;
  if(t == NULL) Controls=controls;
  else {
    Controls=new triple[n];
    for(size_t i=0; i < n; i++)
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

void drawNurbsPath3::displacement()
{
#ifdef HAVE_GL
  size_t nknots=degree+n+1;
  if(Controls == NULL) {
    Controls=new(UseGC)  GLfloat[(weights ? 4 : 3)*n];
    Knots=new(UseGC) GLfloat[nknots];
  }
  if(weights)
    for(size_t i=0; i < n; ++i)
      store(Controls+4*i,controls[i],weights[i]);
  else
    for(size_t i=0; i < n; ++i)
      store(Controls+3*i,controls[i]);

  for(size_t i=0; i < nknots; ++i)
    Knots[i]=knots[i];
#endif
}

void drawNurbsPath3::render(double, const triple&, const triple&,
                            double, bool remesh)
{
#ifdef HAVE_GL
  if(invisible) return;

// TODO: implement NURBS renderer
#endif
}

bool drawPixel::write(prcfile *out, unsigned int *, double, groupsmap&)
{
  if(invisible)
    return true;

  out->addPoint(v,color,width);

  return true;
}

bool drawPixel::write(abs3Doutfile *out)
{
#ifdef HAVE_LIBGLM
  if(invisible)
    return true;

  RGBAColour Black(0.0,0.0,0.0,color.A);
  setcolors(color,color,Black,1.0,0.0,0.04,out);

  out->addPixel(v,width,Min,Max);
#endif
  return true;
}

void drawPixel::render(double size2, const triple& b, const triple& B,
                       double perspective, bool remesh)
{
#ifdef HAVE_GL
  if(invisible) return;

  RGBAColour Black(0.0,0.0,0.0,color.A);
  setcolors(color,color,Black,1.0,0.0,0.04);

  setMaterial(material0Data,drawMaterial0);

  if(bbox2(Min,Max).offscreen()) { // Fully offscreen
    R.data.clear();
    return;
  }

  R.queue(v,width);
#endif
}

drawElement *drawPixel::transformed(const double* t)
{
  return new drawPixel(t*v,p,width,KEY);
}

} //namespace camp
