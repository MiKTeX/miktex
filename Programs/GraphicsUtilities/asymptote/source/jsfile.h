#ifndef JSFILE_H
#define JSFILE_H

#include <fstream>
#include "common.h"
#include "triple.h"
#include "locate.h"
#include "prcfile.h"

namespace camp {

class jsfile {
  jsofstream out;
  
public:  
  jsfile() {}
  ~jsfile();
  
  void open(string name);
  void copy(string name);
  
  void addColor(const prc::RGBAColour& c); 
  void addIndices(const uint32_t *I); 
    
  void addPatch(const triple* controls, size_t n, const triple& Min,
                const triple& Max, const prc::RGBAColour *colors, size_t nc);
  
  void addCurve(const triple& z0, const triple& c0,
                const triple& c1, const triple& z1,
                const triple& Min, const triple& Max);
  
  void addCurve(const triple& z0, const triple& z1,
                const triple& Min, const triple& Max);
  
  void addPixel(const triple& z0, double width,
                const triple& Min, const triple& Max);
  
  void addMaterial(size_t index);
  
  void addTriangles(size_t nP, const triple* P, size_t nN, const triple* N,
                    size_t nC, const prc::RGBAColour* C, size_t nI,
                    const uint32_t (*PI)[3], const uint32_t (*NI)[3],
                    const uint32_t (*CI)[3],
                    const triple& Min, const triple& Max);
};

} //namespace camp

#endif
