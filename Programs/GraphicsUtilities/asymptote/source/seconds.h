#ifndef __seconds_h__
#define __seconds_h__ 1

#include <chrono>

#if !defined(_WIN32)
#include <sys/resource.h>
#endif

namespace utils {

#ifdef _WIN32
#include <Windows.h>
#define getpid GetCurrentProcessId
inline double cpuTime() {
  FILETIME a,b,c,d;
  return GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d) != 0 ?
    (double) (d.dwLowDateTime |
              ((unsigned long long)d.dwHighDateTime << 32))*100.0 : 0.0;
}
#else
#include <unistd.h>
#include <time.h>

inline double cpuTime() {
#ifdef CLOCK_PROCESS_CPUTIME_ID
  timespec t;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);
  return 1.0e9*t.tv_sec+t.tv_nsec;
#else
  struct rusage ru;
  if(getrusage(RUSAGE_SELF, &ru))
    return 0;
  return 1.0e9*(ru.ru_utime.tv_sec+ru.ru_stime.tv_sec)
    +1.0e3*(ru.ru_utime.tv_usec+ru.ru_stime.tv_usec);
#endif
}
#endif

class stopWatch {
  std::chrono::time_point<std::chrono::steady_clock> Start;

public:
  void reset() {
    Start=std::chrono::steady_clock::now();
  }

  stopWatch() {
    reset();
  }

  double nanoseconds(bool reset=false) {
    auto Stop=std::chrono::steady_clock::now();
    double ns=std::chrono::duration_cast<std::chrono::nanoseconds>
      (Stop-Start).count();
    if(reset)
      Start=Stop;
    return ns;
  }

  double seconds(bool reset=false) {
    return 1.0e-9*nanoseconds(reset);
  }
};

class cpuTimer {
  double start;
  std::chrono::time_point<std::chrono::steady_clock> Start;

public:
  void reset() {
    start=cpuTime();
    Start=std::chrono::steady_clock::now();
  }

  cpuTimer() {
    reset();
  }

  double nanoseconds(bool reset=false) {
    auto Stop=std::chrono::steady_clock::now();
    double stop=cpuTime();
    double ns=std::min((double) std::chrono::duration_cast<std::chrono::nanoseconds>
                       (Stop-Start).count(),stop-start);
    if(reset) {
      Start=Stop;
      start=stop;
    }
    return ns;
  }

  double seconds(bool reset=false) {
    return 1.0e-9*nanoseconds(reset);
  }
};

}

#endif
