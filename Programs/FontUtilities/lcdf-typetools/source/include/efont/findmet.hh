// -*- related-file-name: "../../libefont/findmet.cc" -*-
#ifndef EFONT_FINDMET_HH
#define EFONT_FINDMET_HH
#include <lcdf/hashmap.hh>
#include <lcdf/vector.hh>
#include <lcdf/permstr.hh>
class Filename;
class ErrorHandler;
namespace Efont {
class Metrics;
class AmfmMetrics;
class PsresDatabase;

class MetricsFinder { public:

    MetricsFinder()                     : _next(0), _prev(0) { }
    virtual ~MetricsFinder();

    MetricsFinder *next() const         { return _next; }

    void add_finder(MetricsFinder *);

    Metrics *find_metrics(PermString, ErrorHandler * = 0);
    AmfmMetrics *find_amfm(PermString, ErrorHandler * = 0);

    virtual Metrics *find_metrics_x(PermString, MetricsFinder *, ErrorHandler *);
    virtual AmfmMetrics *find_amfm_x(PermString, MetricsFinder *, ErrorHandler *);

    void record(Metrics *m);
    virtual void record(Metrics *, PermString);
    virtual void record(AmfmMetrics *);

  private:

    MetricsFinder *_next;
    MetricsFinder *_prev;

    MetricsFinder(const MetricsFinder &)                        { }
    MetricsFinder &operator=(const MetricsFinder &)     { return *this; }

  protected:

    Metrics *try_metrics_file(const Filename &, MetricsFinder *, ErrorHandler *);
    AmfmMetrics *try_amfm_file(const Filename &, MetricsFinder *, ErrorHandler *);

};


class CacheMetricsFinder: public MetricsFinder { public:

    CacheMetricsFinder();
    ~CacheMetricsFinder();

    Metrics *find_metrics_x(PermString, MetricsFinder *, ErrorHandler *);
    AmfmMetrics *find_amfm_x(PermString, MetricsFinder *, ErrorHandler *);
    void record(Metrics *, PermString);
    void record(AmfmMetrics *);

    void clear();

  private:

    HashMap<PermString, int> _metrics_map;
    Vector<Metrics *> _metrics;
    HashMap<PermString, int> _amfm_map;
    Vector<AmfmMetrics *> _amfm;

};


class InstanceMetricsFinder: public MetricsFinder { public:

    InstanceMetricsFinder(bool call_mmpfb = true);

    Metrics *find_metrics_x(PermString, MetricsFinder *, ErrorHandler *);

  private:

    bool _call_mmpfb;

    Metrics *find_metrics_instance(PermString, MetricsFinder *, ErrorHandler *);

};


class PsresMetricsFinder: public MetricsFinder { public:

    PsresMetricsFinder(PsresDatabase *);

    Metrics *find_metrics_x(PermString, MetricsFinder *, ErrorHandler *);
    AmfmMetrics *find_amfm_x(PermString, MetricsFinder *, ErrorHandler *);

  private:

    PsresDatabase *_psres;

};


class DirectoryMetricsFinder: public MetricsFinder { public:

    DirectoryMetricsFinder(PermString);

    Metrics *find_metrics_x(PermString, MetricsFinder *, ErrorHandler *);
    AmfmMetrics *find_amfm_x(PermString, MetricsFinder *, ErrorHandler *);

  private:

    PermString _directory;

};

}
#endif
