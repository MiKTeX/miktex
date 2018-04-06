#ifndef PREDICATES_H
#define PREDICATES_H

double orient2d(const double* pa, const double* pb, const double* pc);
double orient2d(double ax, double ay, double bx, double by, double cx,
                double cy);
double orient2dadapt(const double *pa, const double *pb, const double *pc,
                     double detsum);
double orient3d(const double *pa, const double *pb, const double *pc,
                const double *pd);
double incircle(const double *pa, const double *pb, const double *pc,
                const double *pd);
double incircle(double ax, double ay, double bx, double by, double cx,
                double cy, double dx, double dy);
double insphere(const double *pa, const double *pb, const double *pc,
                const double *pd, const double *pe);

extern const double resulterrbound,ccwerrboundA,ccwerrboundB,ccwerrboundC,
  o3derrboundA,o3derrboundB,o3derrboundC,iccerrboundA,iccerrboundB,
  iccerrboundC,isperrboundA,isperrboundB,isperrboundC;

#endif
