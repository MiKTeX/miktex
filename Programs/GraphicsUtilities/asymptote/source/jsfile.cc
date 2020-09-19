#include "jsfile.h"

#include "settings.h"
#include "glrender.h"
#include "drawelement.h"

#ifdef HAVE_LIBGLM

using namespace settings;

namespace camp {

void jsfile::copy(string name) {
  std::ifstream fin(locateFile(name).c_str());
  string s;
  while(getline(fin,s))
    out << s << newl;
}

void jsfile::open(string name) {
  out.open(name);
  out << "<!DOCTYPE html>" << newl << newl;
    
  out << "<!-- Use the following line to embed this file within another web page:" << newl
      << newl
      << "<iframe src=\"" << name
      << "\" width=\"" << gl::fullWidth
      << "\" height=\"" << gl::fullHeight
      << "\" frameborder=\"0\"></iframe>" << newl
      << newl
      << "-->" << newl << newl;

  out.precision(getSetting<Int>("digits"));
  out << "<html lang=\"\">" << newl
      << newl
      << "<head>" << newl
      << "<title>" << stripExt(name) << "</title>" << newl
      << newl
      << "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>" << newl
      << "<meta name=\"viewport\" content=\"user-scalable=no\"/>" << newl
      << newl;
  
  if(getSetting<bool>("offline")) {
    out << "<script>" << newl;
    copy(locateFile(AsyGL));
    out << newl << "</script>" << newl;
  } else {
    out << "<script" << newl << "src=\""
        << getSetting<string>("asygl") << "\">" << newl << "</script>" << newl;
  }
  out << "<script>" << newl;
  out << newl
      << "canvasWidth=" << gl::fullWidth << ";" << newl
      << "canvasHeight=" << gl::fullHeight << ";" << newl
      << "absolute=" << std::boolalpha << getSetting<bool>("absolute") << ";"
      << newl << newl
      <<  "b=[" << gl::xmin << "," << gl::ymin << "," << gl::zmin << "];" 
      << newl
      <<  "B=[" << gl::xmax << "," << gl::ymax << "," << gl::zmax << "];" 
      << newl
      << "orthographic=" << gl::orthographic << ";"
      << newl
      << "angle=" << gl::Angle << ";"
      << newl
      << "Zoom0=" << gl::Zoom0 << ";" << newl
      << "viewportmargin=" << gl::Margin << ";" << newl;
  if(gl::Shift != pair(0.0,0.0))
    out << "viewportshift=" << gl::Shift*gl::Zoom0 << ";" << newl;
  out << "zoomFactor=" << getSetting<double>("zoomfactor") << ";" << newl
      << "zoomPinchFactor=" << getSetting<double>("zoomPinchFactor") << ";"
      << newl
      << "zoomPinchCap=" << getSetting<double>("zoomPinchCap") << ";" << newl
      << "zoomStep=" << getSetting<double>("zoomstep") << ";" << newl
      << "shiftHoldDistance=" << getSetting<double>("shiftHoldDistance") << ";"
      << newl
      << "shiftWaitTime=" << getSetting<double>("shiftWaitTime") << ";"
      << newl
      << "vibrateTime=" << getSetting<double>("vibrateTime") << ";"
      << newl << newl
      << "Lights=[";
  for(size_t i=0; i < gl::nlights; ++i) {
    size_t i4=4*i;
    out << "new Light(" << newl
        << "direction=" << gl::Lights[i] << "," << newl
        << "color=[" << gl::Diffuse[i4] << "," << gl::Diffuse[i4+1] << ","
        << gl::Diffuse[i4+2] << "])," << newl;
  }
  out << "];" << newl << newl;
  out << "Background=[" << gl::Background[0] << "," << gl::Background[1] << ","
      << gl::Background[2] << "," << gl::Background[3] << "];"
      << newl;

  size_t nmaterials=material.size();
  out << "Materials=[";
  for(size_t i=0; i < nmaterials; ++i)
    out << "new Material(" << newl
        << material[i]
        << ")," << newl;
  out << "];" << newl << newl;
}

jsfile::~jsfile() {
  size_t ncenters=drawElement::center.size();
  if(ncenters > 0) {
    out << "Centers=[";
    for(size_t i=0; i < ncenters; ++i)
      out << newl << drawElement::center[i] << ",";
    out << newl << "];" << newl;
  }
  out << "</script>"
      << newl << newl << "</head>"
      << newl << newl << "<body style=\"overflow: hidden;\" onload=\"webGLStart();\">"
      << newl << "<canvas id=\"Asymptote\" width=\""
      << gl::fullWidth << "\" height=\"" <<  gl::fullHeight
      << "\" style=\"border: none;\">"
      << newl << "</canvas>"
      << newl << "</body>"
      << newl << newl << "</html>"
      << newl;
}

void jsfile::addColor(const prc::RGBAColour& c) 
{
  out << "[" << byte(c.R) << "," << byte(c.G) << "," << byte(c.B)
      << "," << byte(c.A) << "]";
}

void jsfile::addIndices(const uint32_t *I) 
{
  out << "[" << I[0] << "," << I[1] << "," << I[2] << "]";
}

bool distinct(const uint32_t *I, const uint32_t *J) 
{
  return I[0] != J[0] || I[1] != J[1] || I[2] != J[2];
}

void jsfile::addPatch(triple const* controls, size_t n,
                      const triple& Min, const triple& Max,
                      const prc::RGBAColour *c, size_t nc)
{
  out << "P.push(new BezierPatch([" << newl;
  size_t last=n-1;
  for(size_t i=0; i < last; ++i)
    out << controls[i] << "," << newl;
  out << controls[last] << newl << "]," 
      << drawElement::centerIndex << "," << materialIndex << ","
      << Min << "," << Max;
  if(c) {
    out << ",[" << newl;
    for(size_t i=0; i < nc; ++i) {
      addColor(c[i]);
      out << "," << newl;
    }
    out << "]";
  }
  out << "));" << newl << newl;
}

void jsfile::addCurve(const triple& z0, const triple& c0,
                      const triple& c1, const triple& z1,
                      const triple& Min, const triple& Max)
{
  out << "P.push(new BezierCurve([" << newl;
  out << z0 << "," << newl
      << c0 << "," << newl
      << c1 << "," << newl
      << z1 << newl << "],"
      << drawElement::centerIndex << "," << materialIndex << ","
      << Min << "," << Max << "));" << newl << newl;
}

void jsfile::addCurve(const triple& z0, const triple& z1,
                      const triple& Min, const triple& Max)
{
  out << "P.push(new BezierCurve([" << newl;
  out << z0 << "," << newl
      << z1 << newl << "],"
      << drawElement::centerIndex << "," << materialIndex << ","
      << Min << "," << Max << "));" << newl << newl;
}

void jsfile::addPixel(const triple& z0, double width,
                      const triple& Min, const triple& Max)
{
  out << "P.push(new Pixel(" << newl;
  out << z0 << "," << width << "," << newl
      << materialIndex << "," << Min << "," << Max << "));" << newl << newl;
}

void jsfile::addMaterial(size_t index)
{
  out << "Materials.push(new Material(" << newl
       << material[index]
      << "));" << newl << newl;
}

void jsfile::addTriangles(size_t nP, const triple* P, size_t nN,
                          const triple* N, size_t nC, const prc::RGBAColour* C,
                          size_t nI, const uint32_t (*PI)[3],
                          const uint32_t (*NI)[3], const uint32_t (*CI)[3],
                          const triple& Min, const triple& Max)
{
  for(size_t i=0; i < nP; ++i)
    out << "Positions.push(" << P[i] << ");" << newl;
  
  for(size_t i=0; i < nN; ++i)
    out << "Normals.push(" << N[i] << ");" << newl;
  
  for(size_t i=0; i < nC; ++i) {
    out << "Colors.push(";
    addColor(C[i]);
    out << ");" << newl;
  }
  
  for(size_t i=0; i < nI; ++i) {
    out << "Indices.push(["; 
    const uint32_t *PIi=PI[i];
    const uint32_t *NIi=NI[i];
    bool keepNI=distinct(NIi,PIi);
    bool keepCI=nC && distinct(CI[i],PIi);
    addIndices(PIi);
    if(keepNI || keepCI) {
      out << ",";
      if(keepNI) addIndices(NIi);
    }
    if(keepCI) {
      out << ",";
      addIndices(CI[i]);
    }
    out << "]);" << newl;
  }
  out << "P.push(new Triangles("
      << materialIndex << "," << newl
      << Min << "," << Max << "));" << newl << newl;
}

void jsfile::addSphere(const triple& center, double radius, bool half,
                       const double& polar, const double& azimuth)
{
  out << "sphere(" << center << "," << radius << ","
      << drawElement::centerIndex << "," << materialIndex;
  if(half)
    out << "," << newl << "[" << polar << "," << azimuth << "]";
  out << ");" << newl << newl;
}

// core signifies whether to also draw a central line for better small-scale
// rendering.
void jsfile::addCylinder(const triple& center, double radius, double height,
                         const double& polar, const double& azimuth,
                         bool core)
{
  out << "cylinder(" << center << "," << radius << "," << height << ","
      << drawElement::centerIndex << "," << materialIndex
      << "," << newl << "[" << polar << "," << azimuth << "]," << core
      << ");" << newl << newl;
}

void jsfile::addDisk(const triple& center, double radius,
                     const double& polar, const double& azimuth)
{
  out << "disk(" << center << "," << radius << ","
      << drawElement::centerIndex << "," << materialIndex
      << "," << newl << "[" << polar << "," << azimuth << "]"
      << ");" << newl << newl;
}

void jsfile::addTube(const triple *g, double width,
                     const triple& Min, const triple& Max, bool core)

{
  out << "tube(["
      << g[0] << "," << newl
      << g[1] << "," << newl
      << g[2] << "," << newl
      << g[3] << newl << "],"
      << width << "," 
      << drawElement::centerIndex << "," << materialIndex << ","
      << Min << "," << Max << "," << core <<");" << newl << newl;
}

}
#endif
