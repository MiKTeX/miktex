#include "exvar.h"

FILE *efp;

int lines,idxcount,acc=0,reject=0;
int prange=1,fsti=0,lorder=0,bcomp=0,force=0,fpage=0,gflg=0,verb=1,debug=0;
int warn=0,scount=0,pattr[PAGE_COMPOSIT_DEPTH]={0};
/* static char roman[]={"ivxlcdm"},Roman[]={"IVXLCDM"}; */

struct index *ind;

char keyword[STYBUFSIZE]={"\\indexentry"};
char arg_open='{',arg_close='}';
char range_open='(',range_close=')';
char level='!',actual='@',encap='|',quote='\"',escape='\\';
char preamble[STYBUFSIZE]={"\\begin{theindex}\n"},postamble[STYBUFSIZE]={"\n\n\\end{theindex}\n"};
char setpage_prefix[STYBUFSIZE]={"\n  \\setcounter{page}{"},setpage_suffix[STYBUFSIZE]={"}\n"};
char group_skip[STYBUFSIZE]={"\n\n  \\indexspace\n"};
char lethead_prefix[STYBUFSIZE]={""},lethead_suffix[STYBUFSIZE]={""};
int lethead_flag=0;
char item_0[][STYBUFSIZE] ={{"\n  \\item "},
                            {"\n    \\subitem "},
                            {"\n      \\subsubitem "},
                            {"\n        \\subsubsubitem "},
                            {"\n          \\subsubsubsubitem "}};
char item_01[][STYBUFSIZE]={{"\n    \\subitem "},
                            {"\n      \\subsubitem "},
                            {"\n        \\subsubsubitem "},
                            {"\n          \\subsubsubsubitem "}};
char item_x[][STYBUFSIZE] ={{"\n    \\subitem "},
                            {"\n      \\subsubitem "},
                            {"\n        \\subsubsubitem "},
                            {"\n          \\subsubsubsubitem "}};
char delim_0[][STYBUFSIZE]={{", "},{", "},{", "},{", "},{", "}};
char delim_n[STYBUFSIZE]={", "},delim_r[STYBUFSIZE]={"--"},delim_t[STYBUFSIZE]={""};
char suffix_2p[STYBUFSIZE]={""},suffix_3p[STYBUFSIZE]={""},suffix_mp[STYBUFSIZE]={""};
char encap_prefix[STYBUFSIZE]={"\\"},encap_infix[STYBUFSIZE]={"{"},encap_suffix[STYBUFSIZE]={"}"};
int line_max=72;
char indent_space[STYBUFSIZE]={"\t\t"};
int indent_length=16;
int priority=0;
char symhead[STYBUFSIZE]={""},numhead[STYBUFSIZE]={""};
char symhead_positive[STYBUFSIZE]={"Symbols"},symhead_negative[STYBUFSIZE]={"symbols"};
char numhead_positive[STYBUFSIZE]={"Numbers"},numhead_negative[STYBUFSIZE]={"numbers"};
int symbol_flag=1;
int letter_head=1;
UChar atama[STYBUFSIZE],hangul_head[STYBUFSIZE],hanzi_head[STYBUFSIZE]={L'\0'},kana_head[STYBUFSIZE]={L'\0'};
UChar devanagari_head[STYBUFSIZE],thai_head[STYBUFSIZE];
char page_compositor[STYBUFSIZE]={"-"},page_precedence[STYBUFSIZE]={"rnaRA"};
char character_order[STYBUFSIZE]={"SNLGCJKHDTah"};
char script_preamble[11][STYBUFSIZE],script_postamble[11][STYBUFSIZE];
char icu_locale[STYBUFSIZE]={"root"},icu_rules[STYBUFSIZE]={""};
int icu_attributes[UCOL_ATTRIBUTE_COUNT];

UCollator * icu_collator;
