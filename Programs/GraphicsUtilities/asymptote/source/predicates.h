#ifndef PREDICATES_H
#define PREDICATES_H

double orient2d(double* pa, double* pb, double* pc);
double orient2d(double ax, double ay, double bx, double by, double cx,
                double cy);
double orient2dadapt(double *pa, double *pb, double *pc, double detsum);
double orient3d(double* pa, double* pb, double* pc, double* pd);
double incircle(double* pa, double* pb, double* pc, double* pd);
double incircle(double ax, double ay, double bx, double by, double cx,
                double cy, double dx, double dy);
double insphere(double* pa, double* pb, double* pc, double* pd, double* pe);

extern const double resulterrbound,ccwerrboundA,ccwerrboundB,ccwerrboundC,
  o3derrboundA,o3derrboundB,o3derrboundC,iccerrboundA,iccerrboundB,
  iccerrboundC,isperrboundA,isperrboundB,isperrboundC;

#endif
