#ifndef EFONT_AFMW_HH
#define EFONT_AFMW_HH
#include <efont/afm.hh>
#include <stdio.h>
namespace Efont {

class AfmWriter { public:

    static void write(Metrics *, FILE *);

  private:

    Metrics *_m;
    AfmMetricsXt *_afm_xt;
    FILE *_f;

    void write_prologue() const;
    void write_char_metric_data(GlyphIndex, int) const;
    void write_kerns() const;
    void write();

    double fd(int i) const              { return _m->fd(i); }

    AfmWriter(Metrics *, FILE *);

};

}
#endif
