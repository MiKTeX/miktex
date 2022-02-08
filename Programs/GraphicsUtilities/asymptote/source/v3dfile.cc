/*
 * v3dfile.cc
 * V3D Export class
 *
 * Supakorn "Jamie" Rassameemasmuang <jamievlin@outlook.com> and
 * John C. Bowman
 */

#include "v3dfile.h"

#ifdef HAVE_RPC_RPC_H

#ifdef HAVE_LIBGLM

#include "drawelement.h"
#include "makeUnique.h"

namespace camp
{

using settings::getSetting;

void v3dfile::writeInit()
{
  uint32_t doubleprecision = !singleprecision;
  getXDRFile() << v3dVersion << doubleprecision;
  addHeaders();

  camp::clearCenters();
  camp::clearMaterials();
}

void v3dfile::addHeaders()
{
  getXDRFile() << v3dtypes::header;
  std::vector<std::unique_ptr<AHeader>> headers;

  headers.emplace_back(make_unique<Uint32Header>(v3dheadertypes::canvasWidth, gl::fullWidth));
  headers.emplace_back(make_unique<Uint32Header>(v3dheadertypes::canvasHeight, gl::fullHeight));
  headers.emplace_back(make_unique<Uint32Header>(v3dheadertypes::absolute, getSetting<bool>("absolute")));
  headers.emplace_back(make_unique<TripleHeader>(v3dheadertypes::minBound, triple(gl::Xmin, gl::Ymin, gl::Zmin)));
  headers.emplace_back(make_unique<TripleHeader>(v3dheadertypes::maxBound, triple(gl::Xmax, gl:: Ymax, gl::Zmax)));
  headers.emplace_back(make_unique<Uint32Header>(v3dheadertypes::orthographic, gl::orthographic));
  headers.emplace_back(make_unique<DoubleHeader>(v3dheadertypes::angleOfView, gl::Angle));
  headers.emplace_back(make_unique<DoubleHeader>(v3dheadertypes::initialZoom, gl::Zoom0));
  if(gl::Shift != pair(0.0,0.0))
    headers.emplace_back(make_unique<PairHeader>(v3dheadertypes::viewportShift, gl::Shift*gl::Zoom0));
  headers.emplace_back(make_unique<PairHeader>(v3dheadertypes::viewportMargin, gl::Margin));

  for(size_t i=0; i < gl::nlights; ++i) {
    size_t i4=4*i;
    headers.emplace_back(make_unique<LightHeader>(
                           gl::Lights[i],
                           prc::RGBAColour(gl::Diffuse[i4], gl::Diffuse[i4+1], gl::Diffuse[i4+2], 1.0)
                           ));
  }

  headers.emplace_back(make_unique<RGBAHeader>(
                         v3dheadertypes::background,
                         prc::RGBAColour(gl::Background[0],gl::Background[1],gl::Background[2],gl::Background[3])));

  headers.emplace_back(make_unique<DoubleHeader>(v3dheadertypes::zoomFactor, getSetting<double>("zoomfactor")));
  headers.emplace_back(make_unique<DoubleHeader>(
                         v3dheadertypes::zoomPinchFactor, getSetting<double>("zoomPinchFactor")));
  headers.emplace_back(make_unique<DoubleHeader>(
                         v3dheadertypes::zoomPinchCap, getSetting<double>("zoomPinchCap")));
  headers.emplace_back(make_unique<DoubleHeader>(v3dheadertypes::zoomStep, getSetting<double>("zoomstep")));
  headers.emplace_back(make_unique<DoubleHeader>(
                         v3dheadertypes::shiftHoldDistance, getSetting<double>("shiftHoldDistance")));
  headers.emplace_back(make_unique<DoubleHeader>(
                         v3dheadertypes::shiftWaitTime, getSetting<double>("shiftWaitTime")));
  headers.emplace_back(make_unique<DoubleHeader>(
                         v3dheadertypes::vibrateTime, getSetting<double>("vibrateTime")));

  getXDRFile() << (uint32_t) headers.size();
  for(const auto& header : headers) {
    getXDRFile() << header->ty << header->getWordSize(singleprecision);
    header->writeContent(getXDRFile());
  }
}

void v3dfile::addCenters()
{
  getXDRFile() << v3dtypes::centers;
  size_t nelem=drawElement::centers.size();
  getXDRFile() << (uint32_t) nelem;
  if(nelem > 0)
    addTriples(drawElement::centers.data(), nelem);
}

void v3dfile::addTriples(triple const* triples, size_t n)
{
  for(size_t i=0; i < n; ++i)
    getXDRFile() << triples[i];
}

void v3dfile::addColors(prc::RGBAColour const* col, size_t nc)
{
  for(size_t i=0; i < nc; ++i)
    getXDRFile() << col[i];
}


void v3dfile::addPatch(triple const* controls, triple const& Min,
                       triple const& Max, prc::RGBAColour const* c)
{
  getXDRFile() << (c ? v3dtypes::bezierPatchColor : v3dtypes::bezierPatch);
  addTriples(controls,16);
  addCenterIndexMat();

  if(c)
    addColors(c,4);
}

void v3dfile::addStraightPatch(triple const* controls, triple const& Min,
                               triple const& Max, prc::RGBAColour const* c)
{
  getXDRFile() << (c ? v3dtypes::quadColor : v3dtypes::quad);
  addTriples(controls,4);
  addCenterIndexMat();

  if(c)
    addColors(c,4);
}

void v3dfile::addBezierTriangle(triple const* controls, triple const& Min,
                                triple const& Max, prc::RGBAColour const* c)
{
  getXDRFile() << (c ? v3dtypes::bezierTriangleColor : v3dtypes::bezierTriangle);
  addTriples(controls,10);
  addCenterIndexMat();

  if(c)
    addColors(c,3);
}

void v3dfile::addStraightBezierTriangle(triple const* controls, triple const& Min,
                                        triple const& Max, prc::RGBAColour const* c)
{
  getXDRFile() << (c ? v3dtypes::triangleColor : v3dtypes::triangle);
  addTriples(controls,3);
  addCenterIndexMat();

  if(c)
    addColors(c,3);
}

void v3dfile::addMaterial(Material const& mat)
{
  getXDRFile() << v3dtypes::material;
  addvec4(mat.diffuse);
  addvec4(mat.emissive);
  addvec4(mat.specular);
  glm::vec4 vec=mat.parameters;
  getXDRFile() << static_cast<float>(vec.x) << static_cast<float>(vec.y)
               << static_cast<float>(vec.z);
}

void v3dfile::addCenterIndexMat()
{
  getXDRFile() << (uint32_t) drawElement::centerIndex << (uint32_t) materialIndex;
}

void v3dfile::addvec4(glm::vec4 const& vec)
{
  getXDRFile() << static_cast<float>(vec.x) << static_cast<float>(vec.y)
               << static_cast<float>(vec.z) << static_cast<float>(vec.w);
}

void v3dfile::addHemisphere(triple const& center, double radius, double const& polar, double const& azimuth)
{
  getXDRFile() << v3dtypes::halfSphere << center << radius;
  addCenterIndexMat();
  getXDRFile() << polar << azimuth;
}

void v3dfile::addSphere(triple const& center, double radius)
{
  getXDRFile() << v3dtypes::sphere << center << radius;
  addCenterIndexMat();
}

void
v3dfile::addCylinder(triple const& center, double radius, double height, double const& polar, double const& azimuth,
                     bool core)
{
  getXDRFile() << v3dtypes::cylinder << center << radius << height;
  addCenterIndexMat();
  getXDRFile() << polar << azimuth << core;
}

void v3dfile::addDisk(triple const& center, double radius, double const& polar, double const& azimuth)
{
  getXDRFile() << v3dtypes::disk << center << radius;
  addCenterIndexMat();
  getXDRFile() << polar << azimuth;
}

void v3dfile::addTube(triple const* g, double width, triple const& Min, triple const& Max, bool core)
{
  getXDRFile() << v3dtypes::tube;
  for(int i=0; i < 4; ++i)
    getXDRFile() << g[i];
  getXDRFile() << width;
  addCenterIndexMat();
  getXDRFile() << core;
}

void v3dfile::addTriangles(size_t nP, triple const* P, size_t nN, triple const* N, size_t nC, prc::RGBAColour const* C,
                           size_t nI, uint32_t const (* PI)[3], uint32_t const (* NI)[3], uint32_t const (* CI)[3],
                           triple const& Min, triple const& Max)
{
  getXDRFile() << v3dtypes::triangles;
  getXDRFile() << (uint32_t) nI;

  getXDRFile() << (uint32_t) nP;
  addTriples(P,nP);

  getXDRFile() << (uint32_t) nN;
  addTriples(N,nN);

  bool explicitNI=false;
  for(size_t i=0; i < nI; ++i) {
    const uint32_t *PIi=PI[i];
    const uint32_t *NIi=NI[i];
    if(distinct(NIi,PIi)) {
      explicitNI=true;
      break;
    }
  }
  getXDRFile() << (uint32_t) explicitNI;

  getXDRFile() << (uint32_t) nC;
  bool explicitCI=false;
  if(nC) {
    addColors(C,nC);
    for(size_t i=0; i < nI; ++i) {
      const uint32_t *PIi=PI[i];
      const uint32_t *CIi=CI[i];
      if(distinct(CIi,PIi)) {
        explicitNI=true;
        break;
      }
    }
    getXDRFile() << (uint32_t) explicitCI;
  }

  for(size_t i=0; i < nI; ++i) {
    const uint32_t *PIi=PI[i];
    const uint32_t *NIi=NI[i];
    addIndices(PIi);
    if(explicitNI)
      addIndices(NIi);
    if(nC) {
      const uint32_t *CIi=CI[i];
      if(explicitCI)
        addIndices(CIi);
    }
  }

  addCenterIndexMat();
}

void v3dfile::addIndices(uint32_t const* v)
{
  getXDRFile() << v[0] << v[1] << v[2];
}

void v3dfile::addCurve(triple const& z0, triple const& c0, triple const& c1, triple const& z1, triple const& Min,
                       triple const& Max)
{
  getXDRFile() << v3dtypes::curve << z0 << c0 << c1 << z1;
  addCenterIndexMat();

}

void v3dfile::addCurve(triple const& z0, triple const& z1, triple const& Min, triple const& Max)
{
  getXDRFile() << v3dtypes::line << z0 << z1;
  addCenterIndexMat();
}

void v3dfile::addPixel(triple const& z0, double width, triple const& Min, triple const& Max)
{
  getXDRFile() << v3dtypes::pixel << z0 << width;
  getXDRFile() << (uint32_t) materialIndex;
}

void v3dfile::finalize()
{
  if(!finalized) {
    addCenters();
    finalized=true;
  }
}

// gzv3dfile

xdr::oxstream& gzv3dfile::getXDRFile()
{
  return memxdrfile;
}

gzv3dfile::gzv3dfile(string const& name, bool singleprecision): v3dfile(singleprecision), memxdrfile(singleprecision), name(name), destroyed(false)
{
  writeInit();
}

gzv3dfile::~gzv3dfile()
{
  close();
}

void gzv3dfile::close()
{
  if(!destroyed) {
    finalize();
    memxdrfile.close();
    gzFile file=gzopen(name.c_str(), "wb9");
    gzwrite(file,data(),length());
    gzclose(file);
    if(settings::verbose > 0)
      cout << "Wrote " << name << endl;
    destroyed=true;
  }
}

char const* gzv3dfile::data() const
{
  return memxdrfile.stream();
}

size_t const& gzv3dfile::length() const
{
  return memxdrfile.getLength();
}

uint32_t LightHeader::getWordSize(bool singleprecision) const
{
  return (singleprecision ? 1 : 2)*3+3;
}

void LightHeader::writeContent(xdr::oxstream& ox) const
{
  ox << direction << (float) color.R << (float) color.G << (float) color.B;
}

LightHeader::LightHeader(triple const& direction, prc::RGBAColour const& color) :
  AHeader(v3dheadertypes::light), direction(direction), color(color)
{
}

} //namespace camp

#endif

#endif
