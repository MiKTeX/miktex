#ifndef T1REWRIT_HH
#define T1REWRIT_HH
#include <efont/t1interp.hh>
#include <efont/t1font.hh>
#include <lcdf/straccum.hh>

class Type1MMRemover { public:

    Type1MMRemover(Efont::Type1Font *, const Vector<double> &weight_vec, int, ErrorHandler *);
    ~Type1MMRemover();

    Efont::CharstringProgram *program() const	{ return _font; }
    const Vector<double> &weight_vector() const { return _weight_vector; }
    int nmasters() const		{ return _weight_vector.size(); }
    int precision() const			{ return _precision; }

    Efont::Type1Charstring *subr_prefix(int);
    Efont::Type1Charstring *subr_expander(int);

    void run();

  private:

    Efont::Type1Font *_font;
    Vector<double> _weight_vector;
    int _precision;

    int _nsubrs;
    Vector<int> _subr_done;
    Vector<Efont::Type1Charstring *> _subr_prefix;
    Vector<int> _must_expand_subr;
    Vector<int> _hint_replacement_subr;
    bool _expand_all_subrs;

    ErrorHandler *_errh;

};


class Type1SubrRemover { public:

    Type1SubrRemover(Efont::Type1Font *, ErrorHandler *);
    ~Type1SubrRemover();

    Efont::CharstringProgram *program() const	{ return _font; }
    ErrorHandler *errh() const			{ return _errh; }

    int save_count() const			{ return _save_count; }

    bool run(int);

  private:

    Efont::Type1Font *_font;

    int _nsubrs;
    enum { REMOVABLE = -1, DEAD = -2 };
    Vector<int> _renumbering;
    Vector<int> _cost;
    int _save_count;
    int _nonexist_count;

    ErrorHandler *_errh;

};


#endif
