#ifndef __statistics_h__
#define __statistics_h__ 1

#include <queue>
#include <cfloat>

namespace utils {

template <class T, class S, class C>
void clearpq(std::priority_queue<T, S, C>& q) {
  struct HackedQueue : private std::priority_queue<T, S, C> {
    static S& Container(std::priority_queue<T, S, C>& q) {
            return q.*&HackedQueue::c;
        }
    };
    HackedQueue::Container(q).clear();
}

class statistics {
  size_t N;
  double A;
  double varL;
  double varH;
  double m,M;
  double Median;
  bool computeMedian;

  // These heap stores are only used when computeMedian=true.
  // Max heap stores the smaller half elements:
  std::priority_queue<double> s;
  // Min heap stores the greater half elements:
  std::priority_queue<double,std::vector<double>,std::greater<double> > g;

public:
  statistics(bool computeMedian=false) : computeMedian(computeMedian) {
    clear();
  }
  void clear() {N=0; A=varL=varH=0.0; m=DBL_MAX; M=-m; clearpq(s); clearpq(g);}
  double count() {return N;}
  double mean() {return A;}
  double max() {return M;}
  double min() {return m;}
  double sum() {return N*A;}
  void add(double t) {
    ++N;
    double diff=t-A;
    A += diff/N;
    double v=diff*(t-A);
    if(diff < 0.0)
      varL += v;
    else
      varH += v;

    if(t < m) m=t;
    if(t > M) M=t;

    if(computeMedian) {
      if(N == 1)
        s.push(Median=t);
      else {
        if(s.size() > g.size()) { // left side heap has more elements

          if(t < Median) {
            g.push(s.top());
            s.pop();
            s.push(t);
          } else
            g.push(t);

          Median=0.5*(s.top()+g.top());
        }

        else if(s.size() == g.size()) { // both heaps are balanced
          if(t < Median) {
            s.push(t);
            Median=(double) s.top();
          } else {
            g.push(t);
            Median=(double) g.top();
          }
        }

        else { // right side heap has more elements
          if(t > Median) {
            s.push(g.top());
            g.pop();
            g.push(t);
          } else
            s.push(t);

          Median=0.5*(s.top()+g.top());
        }
      }
    }
  }

  double stdev(double var, double f) {
    if(N <= f) return DBL_MAX;
    return sqrt(var*f/(N-f));
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
  double stderror() {
    return stdev()/sqrt(N);
  }
  double median() {
    if(!computeMedian) {
      std::cerr << "Constructor requires median=true" << std::endl;
      exit(-1);
    }
    return Median;
  }
  void output(const char *text, size_t m) {
    std::cout << text << ": \n"
              << m << "\t"
              << A << "\t"
              << stdevL() << "\t"
              << stdevH() << std::endl;
  }
};

}

#endif
