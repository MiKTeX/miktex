#ifndef OTFTOTFM_UNIPROP_HH
#define OTFTOTFM_UNIPROP_HH
#include <efont/otf.hh>

class UnicodeProperty { public:

    enum {
        P_C = 000, P_Cn = 000, P_Co = 001, P_Cs = 002, P_Cf = 003, P_Cc = 004,
        P_Z = 010, P_Zs = 010, P_Zl = 011, P_Zp = 012,
        P_M = 020, P_Mn = 020, P_Mc = 021, P_Me = 022,
        P_L = 030, P_Lo = 030, P_Lu = 031, P_Ll = 032, P_Lt = 033, P_Lm = 034,
        P_N = 040, P_No = 040, P_Nd = 041, P_Nl = 042,
        P_P = 050, P_Po = 050, P_Pc = 051, P_Pd = 052, P_Ps = 053, P_Pe = 054, P_Pi = 055, P_Pf = 056,
        P_S = 060, P_So = 060, P_Sm = 061, P_Sc = 062, P_Sk = 063,
        P_TMASK = 0370,
        P_MAX = 0377
    };

    static int property(uint32_t uni);
    static const char* property_name(int p);
    static bool parse_property(const String&, int& prop, int& prop_mask);

  private:

    enum { P_Lul = 070 };
    static const unsigned char property_pages[];
    static const unsigned int property_offsets[];
    static const int nproperty_offsets;

    static inline const unsigned int* find_offset(uint32_t uni);

};

#endif
