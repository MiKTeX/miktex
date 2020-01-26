#ifndef MYFONT_HH
#define MYFONT_HH
#include <efont/t1font.hh>
namespace Efont { class MultipleMasterSpace; }
class ErrorHandler;

class MyFont : public Efont::Type1Font { public:

    MyFont(Efont::Type1Reader &);
    ~MyFont();

    bool set_design_vector(Efont::MultipleMasterSpace *, const Vector<double> &, ErrorHandler * = 0);

    void interpolate_dicts(bool force_integers, ErrorHandler *);
    void interpolate_charstrings(int precision, ErrorHandler * = 0);

  private:

    typedef Vector<double> NumVector;

    int _nmasters;
    Vector<double> _weight_vector;

    void interpolate_dict_int(PermString, Dict, ErrorHandler *);
    void interpolate_dict_num(PermString, Dict, bool round_integer = false);
    void interpolate_dict_numvec(PermString, Dict, int round_mode = 0, bool executable = false);
    void kill_def(Efont::Type1Definition *, int which_dict = -1);

};

#endif
