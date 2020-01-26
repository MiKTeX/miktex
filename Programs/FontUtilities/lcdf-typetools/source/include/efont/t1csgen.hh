// -*- related-file-name: "../../libefont/t1csgen.cc" -*-
#ifndef EFONT_T1CSGEN_HH
#define EFONT_T1CSGEN_HH
#include <efont/t1interp.hh>
#include <lcdf/straccum.hh>
namespace Efont {
class Type1Font;

class Type1CharstringGen { public:

    Type1CharstringGen(int precision = 5);

    int precision() const               { return _precision; }

    void clear();
    char *data()                        { return _ncs.data(); }
    const char *data() const            { return _ncs.data(); }
    int length() const                  { return _ncs.length(); }

    void gen_number(double, int kind = 0);
    void gen_command(int);
    void gen_stack(CharstringInterp &, int for_cmd);

    void append_charstring(const String &);

    const Point &current_point(bool real) const { return (real ? _true : _false); }
    void gen_moveto(const Point &, bool closepath, bool always);

    inline String take_string();
    Type1Charstring *output();
    void output(Type1Charstring &);

    static String callsubr_string(int subr);

  private:

    StringAccum _ncs;
    int _precision;
    double _f_precision;

    Point _true;
    Point _false;

    enum State { S_INITIAL, S_GEN };
    State _state;

    void gen_rational(int big_val, int divisor);
    bool gen_stem3_stack(CharstringInterp &interp);

};

class Type1CharstringGenInterp : public CharstringInterp { public:

    Type1CharstringGenInterp(int precision);

    int precision() const               { return _csgen.precision(); }
    void set_direct_hint_replacement(bool dhr)  { _direct_hr = dhr; }
    void set_hint_replacement_storage(Type1Font *);

    int nhints() const                  { return _stem_hstem.size(); }
    double max_flex_height() const      { return _max_flex_height; }
    bool had_flex() const               { return _had_flex; }
    bool had_bad_flex() const           { return _had_bad_flex; }
    bool had_hr() const                 { return _had_hr; }

    const Type1CharstringGen &csgen() const     { return _csgen; }

    void act_width(int, const Point &);
    void act_seac(int, double, double, double, int, int);

    void act_hstem(int, double, double);
    void act_vstem(int, double, double);
    void act_hintmask(int, const unsigned char *, int);

    void act_line(int, const Point &, const Point &);
    void act_curve(int, const Point &, const Point &, const Point &, const Point &);
    void act_closepath(int);
    void act_flex(int, const Point &, const Point &, const Point &, const Point &, const Point &, const Point &, const Point &, double);

    void intermediate_output(Type1Charstring &out);
    void run(const CharstringContext &g, Type1Charstring &out);

  private:

    // output
    Type1CharstringGen _csgen;
    mutable Type1CharstringGen _hint_csgen;

    // current glyph
    Point _width;
    enum State { S_INITIAL, S_OPEN, S_CLOSED, S_SEAC };
    State _state;

    // hints and hint replacement
    Vector<double> _stem_pos;
    Vector<double> _stem_width;
    Vector<int> _stem_hstem;
    String _last_hints;

    bool _in_hr;
    bool _direct_hr;
    int _hr_firstsubr;
    Type1Font *_hr_storage;

    // Flex
    double _max_flex_height;
    bool _had_flex;
    bool _had_bad_flex;
    bool _had_hr;

    inline void gen_number(double, int = 0);
    inline void gen_command(int);
    void gen_sbw(bool hints_follow);
    String gen_hints(const unsigned char *, int) const;
    void swap_stem_hints();

};

inline String Type1CharstringGen::take_string()
{
    String s = _ncs.take_string();
    clear();
    return s;
}

}
#endif
