// -*- related-file-name: "../../libefont/pairop.cc" -*-
#ifndef PAIROP_HH
#define PAIROP_HH
#include <efont/encoding.hh> /* get GlyphIndex */
namespace Efont {

enum PairOpDefs {

    opNoop              = -1,
    opLigature          = -2,

    opLigSimple         = 0,
    opLigSaveLeft               = 1,
    opLigSaveRight      = 2,
    opLigPast1          = 4,
    opLigPast2          = 8,

    opAllblock          = 0

};


typedef int PairOpIndex;


class PairOp { public:

    PairOp()                    { }
    inline PairOp(GlyphIndex l, GlyphIndex r, int v, PairOpIndex nl);
    inline PairOp(GlyphIndex l, GlyphIndex r, GlyphIndex x, int kind, PairOpIndex nl);

    void noopify()              { _val = opNoop; }
    void set_value(int val)     { _val = val; }

    inline void set_next(PairOpIndex);

    bool is_lig() const         { return _val <= opLigature; }
    bool is_kern() const        { return _val >= 0; }
    bool is_noop() const        { return _val == opNoop; }
    int lig_kind() const        { assert(is_lig()); return -_val+opLigature; }

    GlyphIndex left() const     { return _left; }
    GlyphIndex right() const    { return _right; }
    GlyphIndex result() const   { assert(is_lig()); return _result; }
    int value() const           { return _val; }
    PairOpIndex next_left() const { return _next_left; }

  private:

    GlyphIndex _left;
    GlyphIndex _right;
    GlyphIndex _result;
    int _val;

    PairOpIndex _next_left;

};


class PairProgram { public:

    PairProgram()                               : _reversed(false) { }
    PairProgram(const PairProgram &);

    void reserve_glyphs(int);

    PairOpIndex find_left(GlyphIndex gi) const  { return _left_map[gi]; }
    PairOpIndex find(GlyphIndex leftgi, GlyphIndex rightgi) const;

    int op_count() const                        { return _op.size(); }
    const PairOp &op(PairOpIndex i) const       { return _op[i]; }

    // Return true if it's a duplicate.
    bool add_kern(GlyphIndex, GlyphIndex, int);
    bool add_lig(GlyphIndex, GlyphIndex, GlyphIndex, int kind = opLigSimple);

    void set_reversed(bool r)                   { _reversed = r; }
    void unreverse();
    void optimize();

    void print() const;

  private:

    bool _reversed;
    Vector<PairOpIndex> _left_map;
    Vector<PairOp> _op;

    inline const char *print_name(GlyphIndex) const;

    PairProgram &operator=(const PairProgram &) { assert(0); return *this; }

};


inline PairOp::PairOp(GlyphIndex l, GlyphIndex r, int v, PairOpIndex nl)
    : _left(l), _right(r), _result(), _val(v), _next_left(nl)
{
}

inline PairOp::PairOp(GlyphIndex l, GlyphIndex r, GlyphIndex x, int kind,
                      PairOpIndex nl)
    : _left(l), _right(r), _result(x), _val(opLigature - kind), _next_left(nl)
{
}

inline void PairOp::set_next(PairOpIndex nl)
{
    _next_left = nl;
}

}
#endif
