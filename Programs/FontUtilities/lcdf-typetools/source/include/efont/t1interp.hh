// -*- related-file-name: "../../libefont/t1interp.cc" -*-
#ifndef EFONT_T1INTERP_HH
#define EFONT_T1INTERP_HH
#include <efont/t1cs.hh>
#include <lcdf/point.hh>
#include <stdio.h>
namespace Efont {

class CharstringInterp { public:

    CharstringInterp();
    CharstringInterp(const Vector<double> &weight_vec);
    virtual ~CharstringInterp()                 { }

    int error() const                           { return _error; }
    int error_data() const                      { return _error_data; }
    static String error_string(int error, int error_data);
    inline String error_string() const;

    bool careful() const                        { return _careful; }
    void set_careful(bool c)                    { _careful = c; }

    bool done() const                           { return _done; }
    void set_done()                             { _done = true; }

    int size() const                            { return _sp; }
    double &at(unsigned i)                      { return _s[i]; }
    double &top(unsigned i = 0)                 { return _s[_sp - i - 1]; }
    double pop(unsigned n = 1)                  { _sp -= n; return _s[_sp]; }
    inline void push(double);
    void clear()                                { _sp = 0; }

    int ps_size() const                         { return _ps_sp; }
    double ps_at(unsigned i) const              { return _ps_s[i]; }
    double ps_pop()                             { return _ps_s[--_ps_sp]; }
    inline void ps_push(double);
    void ps_clear()                             { _ps_sp = 0; }

    int subr_depth() const                      { return _subr_depth; }

    inline double &vec(Vector<double> *, int);
    const Vector<double> &weight_vector() const { return _weight_vector; }
    Vector<double> *scratch_vector()            { return &_scratch_vector; }

    const CharstringProgram *program() const    { return _program; }
    inline Charstring *get_subr(int) const;
    inline Charstring *get_gsubr(int) const;
    inline Charstring *get_xsubr(bool g, int) const;
    inline Charstring *get_glyph(PermString) const;

    bool interpret(const CharstringProgram *, const Charstring *);
    inline bool interpret(const CharstringContext &);

    //virtual void init(const CharstringProgram *);
    bool error(int c)                           { return error(c, 0); }
    virtual bool error(int, int);
    virtual bool number(double);

    bool arith_command(int);
    bool vector_command(int);
    bool blend_command();
    bool callsubr_command();
    bool callgsubr_command();
    inline bool callxsubr_command(bool g);
    bool mm_command(int, int);
    bool itc_command(int, int);

    const Point &left_sidebearing() const       { return _lsb; }
    const Point &currentpoint() const           { return _cp; }
    void set_state_path()                       { _state = S_PATH; }

    virtual bool callothersubr_command(int, int);
    virtual bool type1_command(int);
    virtual bool type2_command(int, const uint8_t *, int *);

    virtual void act_sidebearing(int cmd, const Point &lsb);
    virtual void act_width(int cmd, const Point &width);
    virtual void act_default_width(int cmd);
    virtual void act_nominal_width_delta(int cmd, double delta);
    virtual void act_seac(int cmd, double asb, double adx, double ady, int bchar, int achar);

    virtual void act_line(int cmd, const Point &p0, const Point &p1);
    virtual void act_curve(int cmd, const Point &p0, const Point &p1, const Point &p2, const Point &p3);
    virtual void act_closepath(int cmd);

    virtual void act_flex(int cmd, const Point &p0, const Point &p1, const Point &p2, const Point &p3_4, const Point &p5, const Point &p6, const Point &p7, double flex_depth);

    virtual void act_hstem(int cmd, double y, double dy);
    virtual void act_vstem(int cmd, double x, double dx);
    virtual void act_hstem3(int cmd, double y0, double dy0, double y1, double dy1, double y2, double dy2);
    virtual void act_vstem3(int cmd, double x0, double dx0, double x1, double dx1, double x2, double dx2);
    virtual void act_hintmask(int cmd, const uint8_t *data, int nhints);

    typedef Charstring Cs;

    enum Errors {
        errOK           = 0,
        errInternal     = -1,
        errRunoff       = -2,
        errUnimplemented = -3,
        errOverflow     = -4,
        errUnderflow    = -5,
        errVector       = -6,
        errValue        = -7,
        errSubr         = -8,
        errGlyph        = -9,
        errCurrentPoint = -10,
        errFlex         = -11,
        errMultipleMaster = -12,
        errOpenStroke   = -13,
        errLateSidebearing = -14,
        errOthersubr    = -15,
        errOrdering     = -16,
        errHintmask     = -17,
        errSubrDepth    = -18,
        errLastError    = -18
    };

    enum { STACK_SIZE = 48, PS_STACK_SIZE = 24, MAX_SUBR_DEPTH = 10,
           SCRATCH_SIZE = 32 };

  private:

    int _error;
    int _error_data;
    bool _done;
    bool _careful;

    double _s[STACK_SIZE];
    int _sp;
    double _ps_s[PS_STACK_SIZE];
    int _ps_sp;

    int _subr_depth;

    Vector<double> _weight_vector;
    Vector<double> _scratch_vector;

    Point _lsb;
    Point _cp;
    Point _seac_origin;

    const CharstringProgram *_program;

    enum State {
        S_INITIAL, S_SEAC, S_SBW, S_HSTEM, S_VSTEM, S_HINTMASK, S_IPATH, S_PATH
    };
    State _state;
    bool _flex;

    // for processing Type 2 charstrings
    int _t2nhints;

    static double double_for_error;

    void initialize();

    inline void ensure_weight_vector();
    void fetch_weight_vector();

    bool roll_command();
    int type2_handle_width(int, bool);

    inline void actp_rmoveto(int, double, double);
    inline void actp_rlineto(int, double, double);
    void actp_rrcurveto(int, double, double, double, double, double, double);
    void actp_rrflex(int, double, double, double, double, double, double, double, double, double, double, double, double, double);

};


inline String CharstringInterp::error_string() const
{
    return error_string(_error, _error_data);
}

inline void CharstringInterp::push(double d)
{
    if (_sp < STACK_SIZE)
        _s[_sp++] = d;
    else
        error(errOverflow);
}

inline void CharstringInterp::ps_push(double d)
{
    if (_ps_sp < PS_STACK_SIZE)
        _ps_s[_ps_sp++] = d;
    else
        error(errOverflow);
}

inline double &CharstringInterp::vec(Vector<double> *v, int i)
{
    if (i < 0 || i >= v->size()) {
        error(errVector);
        return double_for_error;
    } else
        return v->at_u(i);
}

inline Charstring *CharstringInterp::get_subr(int n) const
{
    return _program ? _program->subr(n) : 0;
}

inline Charstring *CharstringInterp::get_gsubr(int n) const
{
    return _program ? _program->gsubr(n) : 0;
}

inline Charstring *CharstringInterp::get_xsubr(bool g, int n) const
{
    return _program ? _program->xsubr(g, n) : 0;
}

inline Charstring *CharstringInterp::get_glyph(PermString n) const
{
    return _program ? _program->glyph(n) : 0;
}

inline void CharstringInterp::ensure_weight_vector()
{
    if (!_weight_vector.size())
        fetch_weight_vector();
}

inline bool CharstringInterp::callxsubr_command(bool g)
{
    return (g ? callgsubr_command() : callsubr_command());
}

inline bool CharstringInterp::interpret(const CharstringContext &g)
{
    return interpret(g.program, g.cs);
}

}
#endif
