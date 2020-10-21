#include <cmath>
#include <cassert>
#include <cfloat>

// Compute a numerical approximation to an integral via adaptive Simpson's Rule
// This routine ignores underflow.

const int nest=DBL_MANT_DIG;

typedef struct {
  bool left;                    // left interval?
  double psum, f1t, f2t, f3t, dat, estr;
} TABLE;

bool                            // Returns true iff successful.
simpson(double& integral,       // Approximate value of the integral.
        double (*f)(double),    // Pointer to function to be integrated.
        double a, double b,     // Lower, upper limits of integration.
        double acc,             // Desired relative accuracy of integral.
                                // Try to make |error| <= acc*abs(integral).
        double dxmax)           // Maximum limit on the width of a subinterval
// For periodic functions, dxmax should be
// set to the period or smaller to prevent
// premature convergence of Simpson's rule.
{
  double diff,area,estl,estr,alpha,da,dx,wt,est,fv[5];
  TABLE table[nest],*p,*pstop;
  static const double sixth=1.0/6.0;

  bool success=true;
  p=table;
  pstop=table+nest-1;
  p->left=true;
  p->psum=0.0;
  alpha=a;
  da=b-a;
  fv[0]=(*f)(alpha);
  fv[2]=(*f)(alpha+0.5*da);
  fv[4]=(*f)(alpha+da);
  wt=sixth*da;
  est=wt*(fv[0]+4.0*fv[2]+fv[4]);
  area=est;

  // Have estimate est of integral on (alpha, alpha+da).
  // Bisect and compute estimates on left and right half intervals.
  // integral is the best value for the integral.

  for(;;) {
    dx=0.5*da;
    double arg=alpha+0.5*dx;
    fv[1]=(*f)(arg);
    fv[3]=(*f)(arg+dx);
    wt=sixth*dx;
    estl=wt*(fv[0]+4.0*fv[1]+fv[2]);
    estr=wt*(fv[2]+4.0*fv[3]+fv[4]);
    integral=estl+estr;
    diff=est-integral;
    area -= diff;

    if(p >= pstop) success=false;
    if(!success || (fabs(diff) <= acc*fabs(area) && da <= dxmax)) {
      // Accept approximate integral.
      // If it was a right interval, add results to finish at this level.
      // If it was a left interval, process right interval.

      for(;;) {
        if(p->left == false) { // process right-half interval
          alpha += da;
          p->left=true;
          p->psum=integral;
          fv[0]=p->f1t;
          fv[2]=p->f2t;
          fv[4]=p->f3t;
          da=p->dat;
          est=p->estr;
          break;
        }
        integral += p->psum;
        if(--p <= table) return success;
      }

    } else {
      // Raise level and store information for processing right-half interval.
      ++p;
      da=dx;
      est=estl;
      p->left=false;
      p->f1t=fv[2];
      p->f2t=fv[3];
      p->f3t=fv[4];
      p->dat=dx;
      p->estr=estr;
      fv[4]=fv[2];
      fv[2]=fv[1];
    }
  }
}

// Use adaptive Simpson integration to determine the upper limit of
// integration required to make the definite integral of a continuous
// non-negative function close to a user specified sum.
// This routine ignores underflow.

bool                            // Returns true iff successful.
unsimpson(double integral,      // Given value for the integral.
          double (*f)(double),  // Pointer to function to be integrated.
          double a, double& b,  // Lower, upper limits of integration (a <= b).
                                // The value of b provided on entry is used
                                // as an initial guess; somewhat faster if the
                                // given value is an underestimation.
          double acc,           // Desired relative accuracy of b.
                                // Try to make |integral-area| <= acc*integral.
          double& area,         // Computed integral of f(x) on [a,b].
          double dxmax,         // Maximum limit on the width of a subinterval
                                // For periodic functions, dxmax should be
                                // set to the period or smaller to prevent
                                // premature convergence of Simpson's rule.
          double dxmin=0)       // Lower limit on sampling width.
{
  double diff,estl,estr,alpha,da,dx,wt,est,fv[5];
  double sum,parea,pdiff,b2;
  TABLE table[nest],*p,*pstop;
  static const double sixth=1.0/6.0;

  p=table;
  pstop=table+nest-1;
  p->psum=0.0;
  alpha=a;
  parea=0.0;
  pdiff=0.0;

  for(;;) {
    p->left=true;
    da=b-alpha;
    fv[0]=(*f)(alpha);
    fv[2]=(*f)(alpha+0.5*da);
    fv[4]=(*f)(alpha+da);
    wt=sixth*da;
    est=wt*(fv[0]+4.0*fv[2]+fv[4]);
    area=est;

    // Have estimate est of integral on (alpha, alpha+da).
    // Bisect and compute estimates on left and right half intervals.
    // Sum is better value for integral.

    bool cont=true;
    while(cont) {
      dx=0.5*da;
      double arg=alpha+0.5*dx;
      fv[1]=(*f)(arg);
      fv[3]=(*f)(arg+dx);
      wt=sixth*dx;
      estl=wt*(fv[0]+4.0*fv[1]+fv[2]);
      estr=wt*(fv[2]+4.0*fv[3]+fv[4]);
      sum=estl+estr;
      diff=est-sum;

      assert(sum >= 0.0);
      area=parea+sum;
      b2=alpha+da;
      if(fabs(fabs(integral-area)-fabs(pdiff))+fabs(diff) <= fv[4]*acc*(b2-a)){
        b=b2;
        return true;
      }
      if(fabs(integral-area) > fabs(pdiff+diff)) {
        if(integral <= area) {
          p=table;
          p->left=true;
          p->psum=parea;
        } else {
          if((fabs(diff) <= fv[4]*acc*da || dx <= dxmin) && da <= dxmax) {
            // Accept approximate integral sum.
            // If it was a right interval, add results to finish at this level.
            // If it was a left interval, process right interval.

            pdiff += diff;
            for(;;) {
              if(p->left == false) { // process right-half interval
                parea += sum;
                alpha += da;
                p->left=true;
                p->psum=sum;
                fv[0]=p->f1t;
                fv[2]=p->f2t;
                fv[4]=p->f3t;
                da=p->dat;
                est=p->estr;
                break;
              }
              sum += p->psum;
              parea -= p->psum;
              if(--p <= table) {
                p=table;
                p->psum=parea=sum;
                alpha += da;
                b += b-a;
                cont=false;
                break;
              }
            }
            continue;
          }
        }
      }
      if(p >= pstop) return false;
// Raise level and store information for processing right-half interval.
      ++p;
      da=dx;
      est=estl;
      p->psum=0.0;
      p->left=false;
      p->f1t=fv[2];
      p->f2t=fv[3];
      p->f3t=fv[4];
      p->dat=dx;
      p->estr=estr;
      fv[4]=fv[2];
      fv[2]=fv[1];
    }
  }
}
