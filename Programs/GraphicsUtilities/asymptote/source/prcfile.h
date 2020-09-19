#ifndef PRCFILE_H
#define PRCFILE_H

#include "memory.h"
#include "pen.h"

inline double X(const camp::triple &v) {return v.getx();}
inline double Y(const camp::triple &v) {return v.gety();}
inline double Z(const camp::triple &v) {return v.getz();}

#include "prc/oPRCFile.h"

namespace camp {

inline prc::RGBAColour rgba(pen p) {
  p.convert();
  p.torgb();
  return prc::RGBAColour(p.red(),p.green(),p.blue(),p.opacity());
}
  
static const double inches=72;
static const double cm=inches/2.54;

class prcfile : public prc::oPRCFile {
public:  
  prcfile(string name) : prc::oPRCFile(name.c_str(),10.0/cm) {} // Use bp.
};

} //namespace camp

#endif
