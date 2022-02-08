/*
 * v3dfile.h
 * Header file for v3d export and types
 *
 * Supakorn "Jamie" Rassameemasmuang <jamievlin@outlook.com> and
 * John C. Bowman
 */

#ifndef V3DFILE_H
#define V3DFILE_H

#include <prc/oPRCFile.h>
#include <zlib.h>

#include "common.h"

#ifdef HAVE_RPC_RPC_H

#include "abs3doutfile.h"
#include "xstream.h"
#include "triple.h"
#include "material.h"
#include "glrender.h"
#define transform transform_
#include "v3dtypes.h"
#undef transform
#include "v3dheadertypes.h"

namespace camp
{

class AHeader
{
public:
  v3dheadertypes ty;

  AHeader(v3dheadertypes const& ty) : ty(ty) {}
  virtual ~AHeader() = default;
  virtual uint32_t getWordSize(bool singleprecision) const = 0;
  virtual void writeContent(xdr::oxstream& ox) const = 0;
};

template<typename T, uint32_t realWords, uint32_t floatWords=0>
class SingleObjectHeader : public AHeader
{
public:
  SingleObjectHeader(v3dheadertypes const& ty, T const& ob) : AHeader(ty), obj(ob)
  {
  }
  ~SingleObjectHeader() override = default;

protected:
  uint32_t getWordSize(bool singleprecision) const override
  {
    return (singleprecision ? 1 : 2)*realWords+floatWords;
  }

  void writeContent(xdr::oxstream &ox) const override
  {
    ox << obj;
  }

private:
  T obj;
};

using open_mode=xdr::xios::open_mode;
using TripleHeader=SingleObjectHeader<triple,3>;
using PairHeader=SingleObjectHeader<pair,2>;
using DoubleHeader=SingleObjectHeader<double,1>;
using Uint32Header=SingleObjectHeader<uint32_t,0,1>;
using RGBAHeader=SingleObjectHeader<prc::RGBAColour,0,4>;

const unsigned int v3dVersion=1;

class LightHeader : public AHeader
{
public:
  LightHeader(triple const& direction, prc::RGBAColour const& color);
  ~LightHeader() override=default;

protected:
  [[nodiscard]]
  uint32_t getWordSize(bool singleprecision) const override;
  void writeContent(xdr::oxstream &ox) const override;

private:
  triple direction;
  prc::RGBAColour color;
};

class v3dfile : public abs3Doutfile {
private:
  bool finalized;
public:
  v3dfile(bool singleprecision=false) : abs3Doutfile(singleprecision),
                                        finalized(false) {}
  void writeInit();
  void finalize();

  void addPatch(triple const* controls, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;
  void addStraightPatch(
          triple const* controls, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;
  void addBezierTriangle(
          triple const* control, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;
  void addStraightBezierTriangle(
          triple const* controls, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;

#ifdef HAVE_LIBGLM
  void addMaterial(Material const& mat) override;
#endif

  void addSphere(triple const& center, double radius) override;
  void addHemisphere(triple const& center, double radius, double const& polar, double const& azimuth) override;

  void addCylinder(triple const& center, double radius, double height,
                   double const& polar, const double& azimuth,
                   bool core) override;
  void addDisk(triple const& center, double radius,
               double const& polar, const double& azimuth) override;
  void addTube(const triple *g, double width,
               const triple& Min, const triple& Max, bool core) override;

  void addTriangles(size_t nP, const triple* P, size_t nN,
                            const triple* N, size_t nC, const prc::RGBAColour* C,
                            size_t nI, const uint32_t (*PI)[3],
                            const uint32_t (*NI)[3], const uint32_t (*CI)[3],
                            const triple& Min, const triple& Max) override;

  void addCurve(triple const& z0, triple const& c0, triple const& c1, triple const& z1, triple const& Min,
                triple const& Max) override;

  void addCurve(triple const& z0, triple const& z1, triple const& Min, triple const& Max) override;

  void addPixel(triple const& z0, double width, triple const& Min, triple const& Max) override;

  void precision(int digits) override {}


protected:
#ifdef HAVE_LIBGLM
  void addvec4(glm::vec4 const& vec);
#endif

  void addCenterIndexMat();
  void addIndices(uint32_t const* trip);
  void addTriples(triple const* triples, size_t n);
  void addColors(prc::RGBAColour const* col, size_t nc);

  void addHeaders();
  void addCenters();

  virtual xdr::oxstream& getXDRFile() = 0;
};

class gzv3dfile : public v3dfile {
public:
  gzv3dfile(string const& name, bool singleprecision=false);
  ~gzv3dfile() override;

protected:
  xdr::oxstream& getXDRFile() override;

  [[nodiscard]]
  char const* data() const;

  [[nodiscard]]
  size_t const& length() const;

private:
  xdr::memoxstream memxdrfile;
  string name;
  bool destroyed;
  void close() override;
};

} //namespace camp
#endif

#endif
