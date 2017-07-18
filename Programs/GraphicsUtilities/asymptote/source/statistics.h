#ifndef __statistics_h__
#define __statistics_h__ 1

namespace utils {

class statistics {
  unsigned int N;
  double A;
  double varL;
  double varH;
public:
  statistics() : N(0), A(0.0), varL(0.0), varH(0.0) {} 
  double count() {return N;}
  double mean() {return A;}
  void add(double t) {
    ++N;
    double diff=t-A;
    A += diff/N;
    double v=diff*(t-A);
    if(diff < 0.0)
      varL += v;
    else
      varH += v;
  }
  double stdev(double var, double f) {
    double factor=N > f ? f/(N-f) : 0.0;
    return sqrt(var*factor);
  }
  double stdev() {
    return stdev(varL+varH,1.0);
  }
  double stdevL() {
    return stdev(varL,2.0);
  }
  double stdevH() {
    return stdev(varH,2.0);
  }
  void output(const char *text, unsigned int m) {
    std::cout << text << ":\n" 
              << m << "\t" 
              << A << "\t" 
              << stdevL() << "\t" 
              << stdevH() << std::endl;
  }
};
  
}

#endif
