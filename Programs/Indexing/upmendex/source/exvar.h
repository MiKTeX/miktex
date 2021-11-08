extern FILE *efp;

extern int lines,idxcount,acc,reject;
extern int prange,fsti,lorder,bcomp,force,fpage,gflg,verb,debug;
extern int warn,scount,pattr[];

extern struct index *ind;

#define STYBUFSIZE 2048

extern char keyword[];
extern char arg_open,arg_close;
extern char range_open,range_close;
extern char level,actual,encap,quote,escape;
extern char preamble[],postamble[];
extern char setpage_prefix[],setpage_suffix[];
extern char group_skip[];
extern char lethead_prefix[],lethead_suffix[];
extern int lethead_flag;
extern char item_0[],item_1[],item_2[];
extern char item_01[],item_x1[],item_12[],item_x2[];
extern char delim_0[],delim_1[],delim_2[],delim_n[],delim_r[],delim_t[];
extern char suffix_2p[],suffix_3p[],suffix_mp[];
extern char encap_prefix[],encap_infix[],encap_suffix[];
extern int line_max;
extern char indent_space[];
extern int indent_length;
extern int priority;
extern char symhead[],numhead[];
extern char symhead_positive[],symhead_negative[];
extern char numhead_positive[],numhead_negative[];
extern int symbol_flag;
extern int letter_head;
extern UChar atama[],hangul_head[],hanzi_head[],kana_head[];
extern UChar devanagari_head[],thai_head[];
extern char page_compositor[],page_precedence[];
extern char character_order[];
extern char icu_locale[],icu_rules[];
extern int icu_attributes[];

extern UCollator * icu_collator;
