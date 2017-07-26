#ifndef DELAUNAY_H
#define DELAUNAY_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cfloat>

#include "common.h"

struct ITRIANGLE{
  Int p1, p2, p3;
};

struct IEDGE{
  Int p1, p2;
};

struct XYZ{
  double p[2]; // {x,y}
  Int i;
};

Int Triangulate(Int nv, XYZ pxyz[], ITRIANGLE v[], Int &ntri,
                bool presort=true, bool postsort=true);

#endif


