#ifndef JSFILE_H
#define JSFILE_H

#include <fstream>
#include "common.h"
#include "triple.h"
#include "locate.h"
#include "prcfile.h"
#include "abs3doutfile.h"

namespace camp {

class jsfile : public abs3Doutfile {
  jsofstream out;

public:
  jsfile();
  jsfile(string name);
  ~jsfile();

  void close() override;

  void addCurve(const triple& z0, const triple& c0,
                const triple& c1, const triple& z1,
                const triple& Min, const triple& Max) override;

  void addCurve(const triple& z0, const triple& z1,
                const triple& Min, const triple& Max) override;

  void addPixel(const triple& z0, double width,
                const triple& Min, const triple& Max) override;

  void addTriangles(size_t nP, const triple* P, size_t nN, const triple* N,
                    size_t nC, const prc::RGBAColour* C, size_t nI,
                    const uint32_t (*PI)[3], const uint32_t (*NI)[3],
                    const uint32_t (*CI)[3],
                    const triple& Min, const triple& Max) override;

  void addCylinder(const triple& center, double radius, double height,
                   const double& polar, const double& azimuth,
                   bool core) override;
  void addDisk(const triple& center, double radius,
               const double& polar, const double& azimuth) override;
  void addTube(const triple *g, double width,
               const triple& Min, const triple& Max, bool core) override;

#ifdef HAVE_LIBGLM
  void addMaterial(Material const& mat) override;
#endif

  void addSphere(triple const& center, double radius) override;

  void addHemisphere(triple const& center, double radius, double const& polar, double const& azimuth) override;

  void addPatch(triple const* controls, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;

  void
  addStraightPatch(triple const* controls, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;

  void
  addBezierTriangle(triple const* control, triple const& Min, triple const& Max, prc::RGBAColour const* c) override;

  void addStraightBezierTriangle(triple const* controls, triple const& Min, triple const& Max,
                                 prc::RGBAColour const* c) override;


  void svgtohtml(string name);
  void precision(int digits) override {out.precision(digits);}

protected:
  void copy(string name, bool header=false);
  void header(string name);
  void meta(string name, bool scalable=true);
  void finish(string name);
  void footer(string name);



  void open(string name);
  void comment(string name);

  void addColor(const prc::RGBAColour& c);
  void addIndices(const uint32_t *I);

  void addRawPatch(const triple* controls, size_t n, const triple& Min,
                const triple& Max, const prc::RGBAColour *colors, size_t nc);
  void addSphere(const triple& center, double radius, bool half=false,
                 const double& polar=0.0, const double& azimuth=0.0);

private:
  bool finished;
  string fileName;
};

} //namespace camp

#endif
