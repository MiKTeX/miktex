#ifndef OTFTOTFM_SETTING_HH
#define OTFTOTFM_SETTING_HH
#include <lcdf/string.hh>

struct Setting {
    enum { NONE, FONT, SHOW, KERN, KERNX, MOVE, RULE, PUSH, POP,
           SPECIAL, DEAD };
    int op;
    int x;
    int y;
    String s;
    Setting(int op_in, int x_in = 0, int y_in = 0)
        : op(op_in), x(x_in), y(y_in) { }
    Setting(int op_in, const String &s_in) : op(op_in), s(s_in) { }
    bool valid_op() const               { return op >= FONT && op <= SPECIAL; }
};

#endif
