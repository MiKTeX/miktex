// -*- related-file-name: "../../libefont/amfm.cc" -*-
#ifndef EFONT_AMFM_HH
#define EFONT_AMFM_HH
#include <efont/metrics.hh>
#include <efont/t1mm.hh>
#include <lcdf/hashmap.hh>
class Slurper;
class Filename;
class ErrorHandler;
namespace Efont {
class AfmParser;
class MetricsFinder;
class Type1Charstring;


struct AmfmMaster {

    PermString font_name;
    PermString family;
    PermString full_name;
    PermString version;
    Vector<double> weight_vector;

    bool loaded;
    Metrics *afm;

    AmfmMaster()                        : loaded(0), afm(0) { }

};


struct AmfmPrimaryFont {

    Vector<int> design_vector;
    Vector<PermString> labels;
    PermString name;
    AmfmPrimaryFont *next;

};


class AmfmMetrics { public:

    AmfmMetrics(MetricsFinder *);
    ~AmfmMetrics();

    void use()                          { _uses++; }
    void unuse()                        { if (--_uses == 0) delete this; }

    bool sanity(ErrorHandler *) const;

    double fd(int i) const              { return _fdv[i]; }
    double &fd(int i)                   { return _fdv[i]; }

    PermString font_name() const        { return _font_name; }
    PermString directory() const        { return _directory; }

    int naxes() const                   { return _naxes; }
    int nmasters() const                { return _nmasters; }
    MultipleMasterSpace *mmspace() const { return _mmspace; }

    int primary_label_value(int, PermString) const;

    Metrics *interpolate(const Vector<double> &design,
                         const Vector<double> &weight, ErrorHandler *);

  private:

    MetricsFinder *_finder;
    PermString _directory;

    Vector<double> _fdv;

    PermString _font_name;
    PermString _family;
    PermString _full_name;
    PermString _weight;

    PermString _version;
    PermString _notice;
    Vector<PermString> _opening_comments;

    PermString _encoding_scheme;
    Vector<double> _weight_vector;

    int _nmasters;
    int _naxes;
    AmfmMaster *_masters;
    MultipleMasterSpace *_mmspace;

    AmfmPrimaryFont *_primary_fonts;

    Metrics *_sanity_afm;

    unsigned _uses;

    friend class AmfmReader;

    AmfmMetrics(const AmfmMetrics &)            { assert(0); }
    AmfmMetrics &operator=(const AmfmMetrics &) { assert(0); return *this; }

    AmfmPrimaryFont *find_primary_font(const Vector<double> &design) const;

    Metrics *master(int, ErrorHandler *);

};


class AmfmReader { public:

    static AmfmMetrics *read(const Filename &, MetricsFinder *, ErrorHandler*);
    static AmfmMetrics *read(Slurper &, MetricsFinder *, ErrorHandler *);
    static void add_amcp_file(Slurper &, AmfmMetrics *, ErrorHandler *);

  private:

    typedef Vector<double> NumVector;

    AmfmMetrics *_amfm;
    MetricsFinder *_finder;
    AfmParser &_l;
    MultipleMasterSpace *_mmspace;
    ErrorHandler *_errh;

    double &fd(int i) const             { return _amfm->fd(i); }
    int naxes() const                   { return _amfm->_naxes; }
    int nmasters() const                { return _amfm->_nmasters; }

    void check_mmspace();

    void lwarning(const char *, ...) const;
    void lerror(const char *, ...) const;
    void no_match_warning(const char *context = 0) const;

    bool read_simple_array(Vector<double> &) const;
    void read_positions() const;
    void read_normalize() const;
    void read_axis_types() const;
    void read_axis(int axis) const;
    void read_master(int master) const;
    void read_primary_fonts() const;
    void read_one_primary_font() const;
    void read_conversion_programs() const;
    bool read();

    void read_amcp_file();

    AmfmReader(AfmParser &, AmfmMetrics *, ErrorHandler *);

};

}
#endif
