// Robust version of Gilles Dumoulin's C++ port of Paul Bourke's
// triangulation code available from
// http://astronomy.swin.edu.au/~pbourke/papers/triangulate
// Used with permission of Paul Bourke.
// Segmentation fault and numerical robustness improvements by John C. Bowman

#include <cassert>
#include "Delaunay.h"
#include "predicates.h"

inline double max(double a, double b)
{
  return (a > b) ? a : b;
}

int XYZCompare(const void *v1, const void *v2) 
{
  double x1=((XYZ*)v1)->p[0];
  double x2=((XYZ*)v2)->p[0];
  if(x1 < x2)
    return(-1);
  else if(x1 > x2)
    return(1);
  else
    return(0);
}

inline double hypot2(double *x)
{
  return x[0]*x[0]+x[1]*x[1];
}

///////////////////////////////////////////////////////////////////////////////
// Triangulate():
//   Triangulation subroutine
//   Takes as input NV vertices in array pxyz
//   Returned is a list of ntri triangular faces in the array v
//   These triangles are arranged in a consistent clockwise order.
//   The triangle array v should be allocated to 4 * nv
//   The vertex array pxyz must be big enough to hold 3 additional points.
//   By default, the array pxyz is automatically presorted and postsorted.
///////////////////////////////////////////////////////////////////////////////

Int Triangulate(Int nv, XYZ pxyz[], ITRIANGLE v[], Int &ntri,
                bool presort, bool postsort)
{
  Int emax = 200;

  if(presort) qsort(pxyz,nv,sizeof(XYZ),XYZCompare);
  else postsort=false;
  
/* Allocate memory for the completeness list, flag for each triangle */
  Int trimax = 4 * nv;
  Int *complete = new Int[trimax];
/* Allocate memory for the edge list */
  IEDGE *edges = new IEDGE[emax];
/*
  Find the maximum and minimum vertex bounds.
  This is to allow calculation of the bounding triangle
*/
  double xmin = pxyz[0].p[0];
  double ymin = pxyz[0].p[1];
  double xmax = xmin;
  double ymax = ymin;
  for(Int i = 1; i < nv; i++) {
    XYZ *pxyzi=pxyz+i;
    double x=pxyzi->p[0];
    double y=pxyzi->p[1];
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
  }
  double dx = xmax - xmin;
  double dy = ymax - ymin;
/*
  Set up the supertriangle.
  This is a triangle which encompasses all the sample points.
  The supertriangle coordinates are added to the end of the
  vertex list. The supertriangle is the first triangle in
  the triangle list.
*/
  static const double margin=0.01;
  double xmargin=margin*dx;
  double ymargin=margin*dy;
  pxyz[nv+0].p[0] = xmin-xmargin;
  pxyz[nv+0].p[1] = ymin-ymargin;
  pxyz[nv+1].p[0] = xmin-xmargin;
  pxyz[nv+1].p[1] = ymax+ymargin+dx;
  pxyz[nv+2].p[0] = xmax+xmargin+dy;
  pxyz[nv+2].p[1] = ymin-ymargin;
  v->p1 = nv;
  v->p2 = nv+1;
  v->p3 = nv+2;
  complete[0] = false;
  ntri = 1;
/*
  Include each point one at a time into the existing mesh
*/
  for(Int i = 0; i < nv; i++) {
    Int nedge = 0;
    double *d=pxyz[i].p;
/*
  Set up the edge buffer.
  If the point d lies inside the circumcircle then the
  three edges of that triangle are added to the edge buffer
  and that triangle is removed.
*/
    for(Int j = 0; j < ntri; j++) {
      if(complete[j])
        continue;
      ITRIANGLE *vj=v+j;

      double *a=pxyz[vj->p1].p;
      double *b=pxyz[vj->p2].p;
      double *c=pxyz[vj->p3].p;
      
      if(incircle(a,b,c,d) <= 0.0) { // Point d is inside or on circumcircle
/* Check that we haven't exceeded the edge list size */
        if(nedge + 3 >= emax) {
          emax += 100;
          IEDGE *p_EdgeTemp = new IEDGE[emax];
          for (Int i = 0; i < nedge; i++) {
            p_EdgeTemp[i] = edges[i];   
          }
          delete[] edges;
          edges = p_EdgeTemp;
        }
        ITRIANGLE *vj=v+j;
        Int p1=vj->p1;
        Int p2=vj->p2;
        Int p3=vj->p3;
        edges[nedge].p1 = p1;
        edges[nedge].p2 = p2;
        edges[++nedge].p1 = p2;
        edges[nedge].p2 = p3;
        edges[++nedge].p1 = p3;
        edges[nedge].p2 = p1;
        ++nedge;
        ntri--;
        v[j] = v[ntri];
        complete[j] = complete[ntri];
        j--;
      } else {
        double A=hypot2(a);
        double B=hypot2(b);
        double C=hypot2(c);
        double a0=orient2d(a,b,c);
        // Is d[0] > xc+r for circumcircle abc of radius r about (xc,yc)?
        if(d[0]*a0 < 0.5*orient2d(A,a[1],B,b[1],C,c[1]))
          complete[j]=
            incircle(a[0]*a0,a[1]*a0,b[0]*a0,b[1]*a0,c[0]*a0,c[1]*a0,
                     d[0]*a0,0.5*orient2d(a[0],A,b[0],B,c[0],C)) > 0.0;
      }
    }
/*
  Tag multiple edges
  Note: if all triangles are specified anticlockwise then all
  interior edges are opposite pointing in direction.
*/
    for(Int j = 0; j < nedge - 1; j++) {
      for(Int k = j + 1; k < nedge; k++) {
        if((edges[j].p1 == edges[k].p2) && (edges[j].p2 == edges[k].p1)) {
          edges[j].p1 = -1;
          edges[j].p2 = -1;
          edges[k].p1 = -1;
          edges[k].p2 = -1;
        }
      }
    }
/*
  Form new triangles for the current point
  Skipping over any tagged edges.
  All edges are arranged in clockwise order.
*/
    for(Int j = 0; j < nedge; j++) {
      if(edges[j].p1 < 0 || edges[j].p2 < 0)
        continue;
      v[ntri].p1 = edges[j].p1;
      v[ntri].p2 = edges[j].p2;
      v[ntri].p3 = i;
      complete[ntri] = false;
      ntri++;
      assert(ntri < trimax);
    }
  }
/*
  Remove triangles with supertriangle vertices
  These are triangles which have a vertex number greater than nv
*/
  for(Int i = 0; i < ntri; i++) {
    ITRIANGLE *vi=v+i;
    if(vi->p1 >= nv || vi->p2 >= nv || vi->p3 >= nv) {
      ntri--;
      *vi = v[ntri];
      i--;
    }
  }
  delete[] edges;
  delete[] complete;

  if(postsort) { 
    for(Int i = 0; i < ntri; i++) {
      ITRIANGLE *vi=v+i;
      vi->p1=pxyz[vi->p1].i;
      vi->p2=pxyz[vi->p2].i;
      vi->p3=pxyz[vi->p3].i;
    }
  }

  return 0;
} 
