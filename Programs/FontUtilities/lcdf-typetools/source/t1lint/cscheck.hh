#ifndef CSCHECK_HH
#define CSCHECK_HH
#include <efont/t1interp.hh>
#include <lcdf/point.hh>
#include <lcdf/vector.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <ctype.h>

template <typename T>
class CharstringCheckerErrorHandler : public ErrorVeneer { public:

    CharstringCheckerErrorHandler(ErrorHandler *errh, T *checker)
	: ErrorVeneer(errh), _checker(checker) {
    }

    String decorate(const String &str);

  private:

    T *_checker;

};

class CharstringChecker : public Efont::CharstringInterp { public:

    CharstringChecker();
    CharstringChecker(const Vector<double> &weight_vec);

    bool error(int, int);
    bool callothersubr();
    bool type1_command(int);

    bool check(const Efont::CharstringContext &, ErrorHandler *);

    int ncommand() const {
	return _ncommand;
    }
    int subrno() const {
	return _subrno;
    }

  private:

    ErrorHandler *_errh;
    int _ncommand;
    int _subrno;

    Point _cp;

    bool _started;
    bool _flex;
    bool _cp_exists;

    bool _hstem;
    bool _hstem3;
    bool _vstem;
    bool _vstem3;

    bool _just_flexed;
    bool _counter_controlled;
    int _last_command;

    Vector<double> _h_hstem;
    Vector<double> _h_vstem;

    Vector<double> _h_hstem3;
    Vector<double> _h_vstem3;

    void stem(double, double, const char *);
    void check_stem3(const char *);

    void moveto(double, double, bool cp_exists = true);
    void rmoveto(double, double);
    void rlineto(double, double);
    void rrcurveto(double, double, double, double, double, double);

};

class CharstringSubrChecker : public Efont::CharstringInterp { public:

    CharstringSubrChecker();
    CharstringSubrChecker(const Vector<double> &weight_vec);

    bool error(int, int);
    bool type1_command(int);

    bool check(const Efont::CharstringContext &, ErrorHandler *);

    int ncommand() const {
	return -1;
    }
    int subrno() const {
	return -1;
    }

  private:

    ErrorHandler *_errh;

    bool _returned;

};


template <typename T> String
CharstringCheckerErrorHandler<T>::decorate(const String &str)
{
    StringAccum sa;
    const char *s = skip_anno(str.begin(), str.end());
    while (s < str.end() && isspace((unsigned char) *s))
	++s;
    sa.append(str.begin(), s);
    if (_checker->subrno() >= 0)
	sa << "called from ";
    if (_checker->ncommand() >= 0)
	sa << "command " << (_checker->ncommand() - 1) << ':';
    if (sa)
	sa << ' ';
    if (s + 11 < str.end() && memcmp(s, "charstring ", 11) == 0) {
	bool quote_parity = 0;
	const char *last = s + 11;
	for (const char *x = last; x != str.end(); ++x)
	    if (*x == '\'') {
		sa.append(last, x);
		sa << format(quote_parity ? "%>" : "%<");
		quote_parity = !quote_parity;
		last = x + 1;
	    }
	sa.append(last, str.end());
    } else
	sa.append(s, str.end());
    return ErrorVeneer::decorate(sa.take_string());
}

#endif
