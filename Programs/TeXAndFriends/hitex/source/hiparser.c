/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "parser.y"

	#line 11139 "format.w"
	
#include "hibasetypes.h"
#include <string.h>
#include <math.h>
#include "hierror.h"
#include "hiformat.h"
#include "hiput.h"
extern char**hfont_name;

	/*375:*/
uint32_t definition_bits[0x100/32][32]= {
	#line 8341 "format.w"
	{0}};

#define SET_DBIT(N,K) ((N)>0xFF?1:(definition_bits[N/32][K]	|= (1<<((N)&(32-1)))))
#define GET_DBIT(N,K) ((N)>0xFF?1:((definition_bits[N/32][K]>>((N)&(32-1)))&1))
#define DEF(D,K,N) (D).k= K; (D).n= (N);SET_DBIT((D).n,(D).k);\
 DBG(DBGDEF,"Defining %s %d\n",definition_name[(D).k],(D).n);\
 RNG("Definition",(D).n,max_fixed[(D).k]+1,max_ref[(D).k]);
#define REF(K,N) REF_RNG(K,N);if(!GET_DBIT(N,K)) \
 QUIT("Reference %d to %s before definition",(N),definition_name[K])
	/*:375*/	/*379:*/
#define DEF_REF(D,K,M,N)  DEF(D,K,M);\
if ((int)(M)>max_default[K]) QUIT("Defining non default reference %d for %s",M,definition_name[K]); \
if ((int)(N)>max_fixed[K]) QUIT("Defining reference %d for %s by non fixed reference %d",M,definition_name[K],N);
	/*:379*/

extern void hset_entry(Entry*e,uint16_t i,uint32_t size,
uint32_t xsize,char*file_name);

	/*445:*/
#ifdef DEBUG
#define  YYDEBUG 1
extern int yydebug;
#else
#define YYDEBUG 0
#endif
	/*:445*/
extern int yylex(void);

	/*371:*/
void hset_max(Kind k,int n)
{
	#line 8183 "format.w"
	DBG(DBGDEF,"Setting max %s to %d\n",definition_name[k],n);
	RNG("Maximum",n,max_fixed[k]+1,MAX_REF(k));
	if(n>max_ref[k])
	max_ref[k]= n;
	}
	/*:371*/	/*382:*/
void check_param_def(Ref*df)
{
	#line 8494 "format.w"
	if(df->k!=int_kind&&df->k!=dimen_kind&&df->k!=glue_kind)
	QUIT("Kind %s not allowed in parameter list",definition_name[df->k]);
	if(df->n<=max_fixed[df->k]||max_default[df->k]<df->n)
	QUIT("Parameter %d for %s not allowed in parameter list",df->n,definition_name[df->k]);
	}
	/*:382*/	/*444:*/
extern int yylineno;
int yyerror(const char*msg)
{
	#line 9634 "format.w"
	QUIT(" in line %d %s",yylineno,msg);
	return 0;
	}
	/*:444*/



#line 144 "parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "hiparser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_START = 3,                      /* "<"  */
  YYSYMBOL_END = 4,                        /* ">"  */
  YYSYMBOL_GLYPH = 5,                      /* "glyph"  */
  YYSYMBOL_UNSIGNED = 6,                   /* UNSIGNED  */
  YYSYMBOL_REFERENCE = 7,                  /* REFERENCE  */
  YYSYMBOL_SIGNED = 8,                     /* SIGNED  */
  YYSYMBOL_STRING = 9,                     /* STRING  */
  YYSYMBOL_CHARCODE = 10,                  /* CHARCODE  */
  YYSYMBOL_FPNUM = 11,                     /* FPNUM  */
  YYSYMBOL_DIMEN = 12,                     /* "dimen"  */
  YYSYMBOL_PT = 13,                        /* "pt"  */
  YYSYMBOL_MM = 14,                        /* "mm"  */
  YYSYMBOL_INCH = 15,                      /* "in"  */
  YYSYMBOL_XDIMEN = 16,                    /* "xdimen"  */
  YYSYMBOL_H = 17,                         /* "h"  */
  YYSYMBOL_V = 18,                         /* "v"  */
  YYSYMBOL_FIL = 19,                       /* "fil"  */
  YYSYMBOL_FILL = 20,                      /* "fill"  */
  YYSYMBOL_FILLL = 21,                     /* "filll"  */
  YYSYMBOL_PENALTY = 22,                   /* "penalty"  */
  YYSYMBOL_INTEGER = 23,                   /* "int"  */
  YYSYMBOL_LANGUAGE = 24,                  /* "language"  */
  YYSYMBOL_RULE = 25,                      /* "rule"  */
  YYSYMBOL_RUNNING = 26,                   /* "|"  */
  YYSYMBOL_KERN = 27,                      /* "kern"  */
  YYSYMBOL_EXPLICIT = 28,                  /* "!"  */
  YYSYMBOL_GLUE = 29,                      /* "glue"  */
  YYSYMBOL_PLUS = 30,                      /* "plus"  */
  YYSYMBOL_MINUS = 31,                     /* "minus"  */
  YYSYMBOL_TXT_START = 32,                 /* TXT_START  */
  YYSYMBOL_TXT_END = 33,                   /* TXT_END  */
  YYSYMBOL_TXT_IGNORE = 34,                /* TXT_IGNORE  */
  YYSYMBOL_TXT_FONT_GLUE = 35,             /* TXT_FONT_GLUE  */
  YYSYMBOL_TXT_FONT_HYPHEN = 36,           /* TXT_FONT_HYPHEN  */
  YYSYMBOL_TXT_FONT = 37,                  /* TXT_FONT  */
  YYSYMBOL_TXT_LOCAL = 38,                 /* TXT_LOCAL  */
  YYSYMBOL_TXT_GLOBAL = 39,                /* TXT_GLOBAL  */
  YYSYMBOL_TXT_CC = 40,                    /* TXT_CC  */
  YYSYMBOL_HBOX = 41,                      /* "hbox"  */
  YYSYMBOL_VBOX = 42,                      /* "vbox"  */
  YYSYMBOL_SHIFTED = 43,                   /* "shifted"  */
  YYSYMBOL_HPACK = 44,                     /* "hpack"  */
  YYSYMBOL_HSET = 45,                      /* "hset"  */
  YYSYMBOL_VPACK = 46,                     /* "vpack"  */
  YYSYMBOL_VSET = 47,                      /* "vset"  */
  YYSYMBOL_DEPTH = 48,                     /* "depth"  */
  YYSYMBOL_ADD = 49,                       /* "add"  */
  YYSYMBOL_TO = 50,                        /* "to"  */
  YYSYMBOL_LEADERS = 51,                   /* "leaders"  */
  YYSYMBOL_ALIGN = 52,                     /* "align"  */
  YYSYMBOL_CENTER = 53,                    /* "center"  */
  YYSYMBOL_EXPAND = 54,                    /* "expand"  */
  YYSYMBOL_BASELINE = 55,                  /* "baseline"  */
  YYSYMBOL_LIGATURE = 56,                  /* "ligature"  */
  YYSYMBOL_DISC = 57,                      /* "disc"  */
  YYSYMBOL_PAR = 58,                       /* "par"  */
  YYSYMBOL_MATH = 59,                      /* "math"  */
  YYSYMBOL_ON = 60,                        /* "on"  */
  YYSYMBOL_OFF = 61,                       /* "off"  */
  YYSYMBOL_ADJUST = 62,                    /* "adjust"  */
  YYSYMBOL_TABLE = 63,                     /* "table"  */
  YYSYMBOL_ITEM = 64,                      /* "item"  */
  YYSYMBOL_IMAGE = 65,                     /* "image"  */
  YYSYMBOL_WIDTH = 66,                     /* "width"  */
  YYSYMBOL_HEIGHT = 67,                    /* "height"  */
  YYSYMBOL_LABEL = 68,                     /* "label"  */
  YYSYMBOL_BOT = 69,                       /* "bot"  */
  YYSYMBOL_MID = 70,                       /* "mid"  */
  YYSYMBOL_LINK = 71,                      /* "link"  */
  YYSYMBOL_OUTLINE = 72,                   /* "outline"  */
  YYSYMBOL_UNKNOWN = 73,                   /* "unknown"  */
  YYSYMBOL_STREAM = 74,                    /* "stream"  */
  YYSYMBOL_STREAMDEF = 75,                 /* "stream (definition)"  */
  YYSYMBOL_FIRST = 76,                     /* "first"  */
  YYSYMBOL_LAST = 77,                      /* "last"  */
  YYSYMBOL_TOP = 78,                       /* "top"  */
  YYSYMBOL_NOREFERENCE = 79,               /* "*"  */
  YYSYMBOL_PAGE = 80,                      /* "page"  */
  YYSYMBOL_RANGE = 81,                     /* "range"  */
  YYSYMBOL_DIRECTORY = 82,                 /* "directory"  */
  YYSYMBOL_SECTION = 83,                   /* "entry"  */
  YYSYMBOL_DEFINITIONS = 84,               /* "definitions"  */
  YYSYMBOL_MAX = 85,                       /* "max"  */
  YYSYMBOL_PARAM = 86,                     /* "param"  */
  YYSYMBOL_FONT = 87,                      /* "font"  */
  YYSYMBOL_CONTENT = 88,                   /* "content"  */
  YYSYMBOL_YYACCEPT = 89,                  /* $accept  */
  YYSYMBOL_glyph = 90,                     /* glyph  */
  YYSYMBOL_content_node = 91,              /* content_node  */
  YYSYMBOL_start = 92,                     /* start  */
  YYSYMBOL_integer = 93,                   /* integer  */
  YYSYMBOL_string = 94,                    /* string  */
  YYSYMBOL_number = 95,                    /* number  */
  YYSYMBOL_dimension = 96,                 /* dimension  */
  YYSYMBOL_xdimen = 97,                    /* xdimen  */
  YYSYMBOL_xdimen_node = 98,               /* xdimen_node  */
  YYSYMBOL_order = 99,                     /* order  */
  YYSYMBOL_stretch = 100,                  /* stretch  */
  YYSYMBOL_penalty = 101,                  /* penalty  */
  YYSYMBOL_rule_dimension = 102,           /* rule_dimension  */
  YYSYMBOL_rule = 103,                     /* rule  */
  YYSYMBOL_rule_node = 104,                /* rule_node  */
  YYSYMBOL_explicit = 105,                 /* explicit  */
  YYSYMBOL_kern = 106,                     /* kern  */
  YYSYMBOL_plus = 107,                     /* plus  */
  YYSYMBOL_minus = 108,                    /* minus  */
  YYSYMBOL_glue = 109,                     /* glue  */
  YYSYMBOL_glue_node = 110,                /* glue_node  */
  YYSYMBOL_position = 111,                 /* position  */
  YYSYMBOL_content_list = 112,             /* content_list  */
  YYSYMBOL_estimate = 113,                 /* estimate  */
  YYSYMBOL_list = 114,                     /* list  */
  YYSYMBOL_115_1 = 115,                    /* $@1  */
  YYSYMBOL_text = 116,                     /* text  */
  YYSYMBOL_txt = 117,                      /* txt  */
  YYSYMBOL_118_2 = 118,                    /* $@2  */
  YYSYMBOL_box_dimen = 119,                /* box_dimen  */
  YYSYMBOL_box_shift = 120,                /* box_shift  */
  YYSYMBOL_box_glue_set = 121,             /* box_glue_set  */
  YYSYMBOL_box = 122,                      /* box  */
  YYSYMBOL_hbox_node = 123,                /* hbox_node  */
  YYSYMBOL_vbox_node = 124,                /* vbox_node  */
  YYSYMBOL_box_flex = 125,                 /* box_flex  */
  YYSYMBOL_box_options = 126,              /* box_options  */
  YYSYMBOL_hxbox_node = 127,               /* hxbox_node  */
  YYSYMBOL_vbox_dimen = 128,               /* vbox_dimen  */
  YYSYMBOL_vxbox_node = 129,               /* vxbox_node  */
  YYSYMBOL_box_goal = 130,                 /* box_goal  */
  YYSYMBOL_hpack = 131,                    /* hpack  */
  YYSYMBOL_max_depth = 132,                /* max_depth  */
  YYSYMBOL_vpack = 133,                    /* vpack  */
  YYSYMBOL_134_3 = 134,                    /* $@3  */
  YYSYMBOL_135_4 = 135,                    /* $@4  */
  YYSYMBOL_ltype = 136,                    /* ltype  */
  YYSYMBOL_leaders = 137,                  /* leaders  */
  YYSYMBOL_baseline = 138,                 /* baseline  */
  YYSYMBOL_139_5 = 139,                    /* $@5  */
  YYSYMBOL_cc_list = 140,                  /* cc_list  */
  YYSYMBOL_lig_cc = 141,                   /* lig_cc  */
  YYSYMBOL_ref = 142,                      /* ref  */
  YYSYMBOL_ligature = 143,                 /* ligature  */
  YYSYMBOL_144_6 = 144,                    /* $@6  */
  YYSYMBOL_replace_count = 145,            /* replace_count  */
  YYSYMBOL_disc = 146,                     /* disc  */
  YYSYMBOL_disc_node = 147,                /* disc_node  */
  YYSYMBOL_par_dimen = 148,                /* par_dimen  */
  YYSYMBOL_par = 149,                      /* par  */
  YYSYMBOL_150_7 = 150,                    /* $@7  */
  YYSYMBOL_math = 151,                     /* math  */
  YYSYMBOL_on_off = 152,                   /* on_off  */
  YYSYMBOL_span_count = 153,               /* span_count  */
  YYSYMBOL_table = 154,                    /* table  */
  YYSYMBOL_image_aspect = 155,             /* image_aspect  */
  YYSYMBOL_image_width = 156,              /* image_width  */
  YYSYMBOL_image_height = 157,             /* image_height  */
  YYSYMBOL_image_spec = 158,               /* image_spec  */
  YYSYMBOL_image = 159,                    /* image  */
  YYSYMBOL_max_value = 160,                /* max_value  */
  YYSYMBOL_placement = 161,                /* placement  */
  YYSYMBOL_def_node = 162,                 /* def_node  */
  YYSYMBOL_unknown_bytes = 163,            /* unknown_bytes  */
  YYSYMBOL_unknown_node = 164,             /* unknown_node  */
  YYSYMBOL_unknown_nodes = 165,            /* unknown_nodes  */
  YYSYMBOL_stream_link = 166,              /* stream_link  */
  YYSYMBOL_stream_split = 167,             /* stream_split  */
  YYSYMBOL_stream_info = 168,              /* stream_info  */
  YYSYMBOL_169_8 = 169,                    /* $@8  */
  YYSYMBOL_stream_type = 170,              /* stream_type  */
  YYSYMBOL_stream_def_node = 171,          /* stream_def_node  */
  YYSYMBOL_stream_ins_node = 172,          /* stream_ins_node  */
  YYSYMBOL_stream = 173,                   /* stream  */
  YYSYMBOL_page_priority = 174,            /* page_priority  */
  YYSYMBOL_stream_def_list = 175,          /* stream_def_list  */
  YYSYMBOL_page = 176,                     /* page  */
  YYSYMBOL_177_9 = 177,                    /* $@9  */
  YYSYMBOL_178_10 = 178,                   /* $@10  */
  YYSYMBOL_hint = 179,                     /* hint  */
  YYSYMBOL_directory_section = 180,        /* directory_section  */
  YYSYMBOL_181_11 = 181,                   /* $@11  */
  YYSYMBOL_entry_list = 182,               /* entry_list  */
  YYSYMBOL_entry = 183,                    /* entry  */
  YYSYMBOL_definition_section = 184,       /* definition_section  */
  YYSYMBOL_185_12 = 185,                   /* $@12  */
  YYSYMBOL_definition_list = 186,          /* definition_list  */
  YYSYMBOL_max_definitions = 187,          /* max_definitions  */
  YYSYMBOL_max_list = 188,                 /* max_list  */
  YYSYMBOL_def_list = 189,                 /* def_list  */
  YYSYMBOL_parameters = 190,               /* parameters  */
  YYSYMBOL_named_param_list = 191,         /* named_param_list  */
  YYSYMBOL_param_list = 192,               /* param_list  */
  YYSYMBOL_font = 193,                     /* font  */
  YYSYMBOL_font_head = 194,                /* font_head  */
  YYSYMBOL_font_param_list = 195,          /* font_param_list  */
  YYSYMBOL_font_param = 196,               /* font_param  */
  YYSYMBOL_fref = 197,                     /* fref  */
  YYSYMBOL_xdimen_ref = 198,               /* xdimen_ref  */
  YYSYMBOL_param_ref = 199,                /* param_ref  */
  YYSYMBOL_stream_ref = 200,               /* stream_ref  */
  YYSYMBOL_content_section = 201,          /* content_section  */
  YYSYMBOL_202_13 = 202                    /* $@13  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   720

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  114
/* YYNRULES -- Number of rules.  */
#define YYNRULES  285
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  596

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   343


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   274,   274,   277,   280,   284,   284,   288,   292,   292,
     298,   300,   302,   304,   307,   310,   314,   317,   320,   323,
     327,   332,   334,   336,   338,   342,   346,   349,   353,   353,
     356,   362,   365,   367,   369,   372,   375,   379,   381,   384,
     386,   389,   392,   396,   402,   405,   406,   407,   410,   413,
     420,   419,   428,   428,   430,   433,   436,   439,   442,   445,
     448,   451,   451,   456,   460,   463,   467,   470,   473,   478,
     482,   485,   488,   488,   490,   494,   497,   501,   505,   506,
     511,   515,   518,   521,   524,   528,   532,   536,   538,   542,
     542,   547,   547,   553,   557,   557,   559,   561,   563,   565,
     568,   571,   574,   577,   581,   581,   590,   595,   595,   598,
     601,   604,   607,   607,   614,   618,   621,   625,   629,   632,
     637,   642,   644,   647,   650,   653,   653,   658,   662,   666,
     669,   672,   675,   678,   681,   685,   689,   691,   694,   698,
     702,   705,   708,   711,   715,   718,   722,   726,   728,   731,
     734,   737,   740,   744,   748,   752,   756,   761,   765,   769,
     776,   778,   780,   782,   785,   790,   795,   805,   808,   812,
     815,   817,   820,   820,   820,   820,   821,   823,   827,   829,
     832,   835,   835,   839,   841,   843,   845,   849,   855,   860,
     860,   862,   865,   868,   873,   876,   880,   880,   882,   884,
     882,   891,   894,   898,   900,   900,   903,   903,   904,   909,
     909,   916,   916,   918,   945,   945,   947,   950,   953,   956,
     959,   962,   965,   968,   971,   974,   977,   980,   983,   986,
     989,   992,   995,  1001,  1004,  1007,  1010,  1013,  1016,  1019,
    1022,  1025,  1028,  1031,  1034,  1037,  1040,  1045,  1048,  1051,
    1055,  1056,  1059,  1063,  1067,  1067,  1073,  1075,  1080,  1080,
    1083,  1086,  1089,  1092,  1095,  1098,  1101,  1104,  1108,  1112,
    1115,  1118,  1124,  1127,  1131,  1135,  1138,  1141,  1144,  1147,
    1150,  1153,  1156,  1160,  1167,  1167
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "\"<\"", "\">\"",
  "\"glyph\"", "UNSIGNED", "REFERENCE", "SIGNED", "STRING", "CHARCODE",
  "FPNUM", "\"dimen\"", "\"pt\"", "\"mm\"", "\"in\"", "\"xdimen\"",
  "\"h\"", "\"v\"", "\"fil\"", "\"fill\"", "\"filll\"", "\"penalty\"",
  "\"int\"", "\"language\"", "\"rule\"", "\"|\"", "\"kern\"", "\"!\"",
  "\"glue\"", "\"plus\"", "\"minus\"", "TXT_START", "TXT_END",
  "TXT_IGNORE", "TXT_FONT_GLUE", "TXT_FONT_HYPHEN", "TXT_FONT",
  "TXT_LOCAL", "TXT_GLOBAL", "TXT_CC", "\"hbox\"", "\"vbox\"",
  "\"shifted\"", "\"hpack\"", "\"hset\"", "\"vpack\"", "\"vset\"",
  "\"depth\"", "\"add\"", "\"to\"", "\"leaders\"", "\"align\"",
  "\"center\"", "\"expand\"", "\"baseline\"", "\"ligature\"", "\"disc\"",
  "\"par\"", "\"math\"", "\"on\"", "\"off\"", "\"adjust\"", "\"table\"",
  "\"item\"", "\"image\"", "\"width\"", "\"height\"", "\"label\"",
  "\"bot\"", "\"mid\"", "\"link\"", "\"outline\"", "\"unknown\"",
  "\"stream\"", "\"stream (definition)\"", "\"first\"", "\"last\"",
  "\"top\"", "\"*\"", "\"page\"", "\"range\"", "\"directory\"",
  "\"entry\"", "\"definitions\"", "\"max\"", "\"param\"", "\"font\"",
  "\"content\"", "$accept", "glyph", "content_node", "start", "integer",
  "string", "number", "dimension", "xdimen", "xdimen_node", "order",
  "stretch", "penalty", "rule_dimension", "rule", "rule_node", "explicit",
  "kern", "plus", "minus", "glue", "glue_node", "position", "content_list",
  "estimate", "list", "$@1", "text", "txt", "$@2", "box_dimen",
  "box_shift", "box_glue_set", "box", "hbox_node", "vbox_node", "box_flex",
  "box_options", "hxbox_node", "vbox_dimen", "vxbox_node", "box_goal",
  "hpack", "max_depth", "vpack", "$@3", "$@4", "ltype", "leaders",
  "baseline", "$@5", "cc_list", "lig_cc", "ref", "ligature", "$@6",
  "replace_count", "disc", "disc_node", "par_dimen", "par", "$@7", "math",
  "on_off", "span_count", "table", "image_aspect", "image_width",
  "image_height", "image_spec", "image", "max_value", "placement",
  "def_node", "unknown_bytes", "unknown_node", "unknown_nodes",
  "stream_link", "stream_split", "stream_info", "$@8", "stream_type",
  "stream_def_node", "stream_ins_node", "stream", "page_priority",
  "stream_def_list", "page", "$@9", "$@10", "hint", "directory_section",
  "$@11", "entry_list", "entry", "definition_section", "$@12",
  "definition_list", "max_definitions", "max_list", "def_list",
  "parameters", "named_param_list", "param_list", "font", "font_head",
  "font_param_list", "font_param", "fref", "xdimen_ref", "param_ref",
  "stream_ref", "content_section", "$@13", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-275)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     101,    25,   127,   133,   163,  -275,    94,   139,  -275,  -275,
      93,  -275,  -275,   214,  -275,    47,   151,  -275,  -275,   165,
    -275,  -275,  -275,   114,  -275,   159,   249,   259,   178,  -275,
     174,  -275,     9,  -275,  -275,   645,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,   271,   320,  -275,   252,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   260,
     269,   269,   269,   275,   282,   103,   198,   287,   108,   250,
     213,   261,   261,   268,   261,    59,    57,   219,   213,   269,
      60,   213,    90,    78,   270,    63,   295,   316,   325,   269,
     269,  -275,  -275,   336,   352,   353,   354,   355,   356,   358,
     359,   360,   364,   366,   368,   372,   373,   374,   376,   377,
     378,   384,   338,   183,  -275,   213,   261,   198,   271,   153,
     213,   394,   261,   269,   250,   392,   393,   271,   396,   271,
     131,   244,   398,   401,   399,  -275,  -275,  -275,   412,   413,
     414,  -275,  -275,  -275,  -275,   220,  -275,   153,   415,   416,
    -275,   177,   420,   261,   397,   422,   424,   261,   268,   427,
     428,   261,   267,   430,   268,   351,   389,  -275,   434,   261,
    -275,   268,  -275,   411,   192,   437,   438,  -275,   439,   440,
     441,   442,   443,   444,    78,   446,   269,  -275,   394,   447,
     219,  -275,  -275,    34,  -275,   448,  -275,  -275,    78,    78,
    -275,   396,   452,   267,   267,   455,  -275,   456,   590,   457,
     459,   261,   462,    78,   463,   265,  -275,  -275,   219,    70,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,   465,   466,   467,   468,   470,   471,   472,   473,   474,
     475,   476,  -275,   478,   479,   480,   210,  -275,   481,  -275,
    -275,   482,   261,   485,   394,  -275,  -275,  -275,   487,   488,
     489,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
     153,  -275,  -275,   269,  -275,   490,  -275,   329,   261,   432,
    -275,  -275,   261,   304,  -275,  -275,  -275,   219,   219,    78,
    -275,   397,   491,  -275,   261,   268,  -275,   261,   492,   213,
    -275,  -275,  -275,   394,  -275,  -275,   394,  -275,  -275,  -275,
     217,  -275,  -275,  -275,    78,  -275,  -275,    78,  -275,    78,
      78,   396,   493,  -275,    58,   394,    78,   394,    78,  -275,
    -275,  -275,    78,    78,  -275,  -275,  -275,   494,  -275,   381,
    -275,  -275,  -275,   496,   495,   499,    78,    78,  -275,  -275,
    -275,  -275,   497,   500,  -275,    78,    78,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,   501,   502,  -275,  -275,   504,  -275,   503,  -275,
     394,   394,  -275,  -275,  -275,  -275,   506,  -275,   261,  -275,
     136,  -275,   261,  -275,  -275,   261,   261,    78,  -275,  -275,
    -275,  -275,  -275,   432,   219,  -275,   268,  -275,   267,   261,
    -275,   507,   510,    61,  -275,  -275,  -275,   394,  -275,  -275,
     483,  -275,    78,  -275,  -275,  -275,   513,  -275,   477,  -275,
    -275,  -275,  -275,  -275,   334,    78,    78,  -275,   302,   453,
    -275,  -275,    97,  -275,  -275,  -275,   261,  -275,   394,   515,
    -275,  -275,   394,  -275,   516,   464,  -275,   186,  -275,  -275,
     505,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,    78,    78,   267,    78,  -275,  -275,  -275,   153,  -275,
    -275,  -275,  -275,  -275,   317,  -275,  -275,  -275,   458,  -275,
     313,  -275,  -275,  -275,   535,  -275,  -275,  -275,  -275,   522,
      64,   394,  -275,   261,  -275,   250,   269,   269,   269,   269,
     269,   269,   269,   269,  -275,  -275,  -275,    78,  -275,   120,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,   459,
     322,  -275,  -275,  -275,  -275,  -275,  -275,    64,  -275,    78,
    -275,  -275,   183,   271,   153,   250,   261,   269,   250,   392,
    -275,  -275,  -275,  -275,  -275,   521,   394,   394,   524,   525,
     527,   261,   528,   529,   530,   531,   532,  -275,   533,   394,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,    78,
    -275,   394,   486,  -275,   269,    45
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     0,     0,     0,     0,     1,     0,     0,   204,   209,
       0,   203,   206,     0,   284,     0,     0,   211,    44,     0,
     205,   207,   214,     0,    45,     0,     0,     0,     4,   210,
       0,   212,     4,   285,    46,     0,    32,    72,    73,    95,
      94,   121,   189,   190,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,     0,     0,    64,     0,    87,     0,     0,     0,     0,
      33,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     8,     9,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   111,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    33,     0,     0,     0,    47,     0,
     163,     0,     0,     0,     0,     6,     5,    26,     0,     0,
       0,    10,    11,    12,    29,     0,    28,     0,     0,     0,
      34,     0,     0,    19,    37,     0,     0,     0,    64,     0,
       0,     0,     0,     0,    64,    87,     0,    89,     0,     0,
      78,    64,     4,     0,    96,     0,     0,   104,     0,     0,
     112,     0,   115,     0,   119,     0,   122,   269,     0,     0,
       0,   136,   137,    47,   270,     0,   138,   254,     0,     0,
      44,    47,     0,     0,     0,     0,   140,     0,    47,     0,
       0,   148,     0,     0,     0,     0,   170,   271,     0,     0,
     208,   218,   227,   217,   222,   223,   221,   225,   226,   219,
     220,   224,   232,   159,   229,   230,   231,   228,   216,   215,
      44,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   112,     0,     0,     0,     0,   198,     0,    48,
      44,     0,     0,     0,     0,   161,   162,   160,     0,     0,
       0,     2,     7,     3,    27,   272,   282,    13,    15,    14,
       0,    31,   278,     0,    35,     0,    36,     0,     0,    39,
      42,   275,     0,    66,    70,    71,    65,     0,     0,     0,
      86,    37,     0,    91,     0,    64,    93,     0,     0,     0,
      97,    98,    99,     0,   103,   280,     0,   106,   281,   276,
       0,   114,   116,   277,   118,   120,   125,     0,   128,     0,
       0,    47,     0,   135,    47,   132,     0,   129,     0,    50,
      44,   139,     0,     0,   146,   141,   143,     0,   147,   150,
     279,   157,   158,     0,   176,     0,     0,     0,   188,   184,
     185,   186,     0,     0,   183,     0,     0,   235,   248,   238,
     234,   247,   236,   239,   237,   249,   240,   241,   242,   243,
     244,   167,     0,   194,   246,   250,   252,   245,     0,   233,
       0,   256,   164,   201,   202,    30,     0,   273,    17,    18,
       0,    38,     0,    41,    63,     0,     0,     0,    84,    82,
      83,    81,    85,    39,     0,    77,    64,    88,     0,     0,
      80,     0,     0,     0,   100,   101,   102,     0,   109,   110,
       0,   117,     0,   127,   124,   123,     0,   255,     0,   133,
     134,   130,   131,    44,     0,     0,     0,   142,     0,   152,
     165,   171,     0,   193,   191,   192,     0,   181,     0,     0,
     168,   195,     0,   251,     0,     0,   258,     0,   259,   274,
       0,    21,    22,    23,    24,    25,    40,    67,    68,    69,
      74,     0,     0,     0,     0,    79,    43,   283,     0,   105,
     107,   126,   253,    52,    61,    49,   144,   145,   152,   149,
       0,   153,   169,   172,    47,   173,   174,   177,   175,     0,
       0,     0,   166,     0,   257,    33,     0,     0,     0,     0,
       0,     0,     0,     0,    16,    76,    75,     0,    90,     0,
      51,    60,    58,    59,    55,    57,    56,    54,    53,     0,
       0,   154,   155,   151,    20,   179,   178,     0,   182,     0,
     199,   268,     0,     0,     0,    33,     0,     0,    33,     0,
      92,   113,   108,    62,   156,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,     0,     0,
     260,   265,   266,   261,   264,   262,   263,   267,   187,     0,
     196,   200,     0,   197,     0,     0
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -275,  -275,   -58,   -23,   115,   -98,  -106,    11,   -67,  -240,
    -275,  -274,   -14,  -109,   -97,   226,   -68,   -13,   245,   132,
    -102,  -225,  -148,   207,   -82,   -59,  -275,  -275,  -275,  -275,
     182,   -20,  -275,   498,  -143,   235,  -275,   379,  -275,  -275,
    -275,  -161,  -275,   387,  -275,  -275,  -275,  -275,   433,   431,
    -275,  -275,  -275,   -45,  -104,  -275,  -275,  -100,   168,  -275,
    -275,  -275,  -275,   340,  -275,  -275,  -275,  -275,    65,  -275,
     -95,  -275,  -275,   180,  -275,  -275,  -275,    20,  -275,  -275,
    -275,  -275,   -22,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -165,   119,   -94,  -275,  -275,  -275,  -275,  -108,  -237,
    -110,  -275,  -275,  -275
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   134,    34,   201,   137,    93,   145,   153,   154,   363,
     475,   401,   138,   147,   148,    36,   182,   152,   289,   403,
     155,   174,    24,    25,   340,   202,   443,   494,   538,   539,
     158,   301,   407,   159,    37,    38,   414,   302,    39,   171,
      40,   299,   163,   167,   168,   305,   416,   313,   175,   178,
     316,   529,   430,   551,   181,   320,   184,   185,    41,   188,
     189,   432,   195,   196,   210,   205,   349,   449,   501,   213,
     214,   112,   268,    31,   354,   507,   452,   547,   548,   364,
     510,   365,    42,    43,   355,   462,   591,   258,   383,   567,
       2,     3,    12,    15,    21,     7,    13,    23,    17,    27,
     386,   261,   197,   198,   263,   264,   391,   468,   552,   190,
     199,   218,    11,    18
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,   151,    35,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   186,   127,   128,   129,   248,   253,
     246,   139,   247,   149,   254,   156,   209,   207,   332,   257,
     255,   262,   176,   179,   180,   183,   187,   194,   280,   390,
     259,   212,   342,   343,   217,   219,   260,   287,   172,   243,
      19,    20,   339,   162,   173,   336,   338,   408,   410,   193,
     409,   411,   208,   141,   259,   142,    32,   114,   143,   206,
     242,   114,   245,   172,   358,   249,   326,    63,   252,   146,
     330,   172,   157,   157,   284,   157,   488,   157,   150,   177,
      64,   427,   366,   172,   327,   200,   329,   114,   173,    71,
      32,   502,    71,    72,     1,   348,   285,     4,   357,   132,
     200,   260,   385,   133,   141,   114,   142,    28,    29,   143,
     331,   359,   360,   361,   356,   324,   241,     5,   476,   200,
     146,   477,   478,   177,   144,   169,     6,   165,   293,   335,
     337,   194,    10,   545,   166,   194,   359,   360,   361,   471,
     191,   192,   347,   561,   351,   472,   473,   474,   146,   141,
     562,   142,    32,    33,   143,   193,   436,   193,   292,     8,
     425,   395,   296,   194,   481,   334,   334,   482,     9,   144,
     307,    14,   400,   141,   114,   142,    48,    35,   143,   135,
      49,   136,   439,   283,   441,   193,   362,    50,    51,    52,
     265,   266,   489,    53,   135,   114,   136,   421,   516,   267,
     517,   518,   505,   519,   381,   520,   382,    16,   511,   141,
     114,   142,   172,   428,   143,    54,   114,   429,   240,    55,
      56,    57,   244,   277,   278,   279,    22,   513,   396,    58,
     412,   173,   521,   522,   310,   311,   312,    59,    26,   260,
      47,   523,   187,   187,    60,    44,   164,   484,   170,   113,
      61,    62,    45,    46,   422,   431,   126,   141,   433,   142,
     434,   435,   143,   388,   362,   362,   114,   440,   150,   442,
      91,    92,   130,   445,   446,   418,   549,   203,   204,   131,
     423,   146,   470,   173,   140,   493,   400,   454,   455,   400,
     400,   211,   114,   404,   269,   270,   458,   459,   141,   498,
     142,   161,   438,   143,   438,   417,   297,   298,   419,   141,
     542,   142,   527,   215,   143,   191,   192,   579,   141,   564,
     142,   216,    94,   143,   405,   406,    95,    32,   495,   589,
     220,   578,   239,    96,    97,    98,   398,   399,   479,    99,
     530,   531,   532,   533,   534,   535,   536,   537,   221,   222,
     223,   224,   225,    30,   226,   227,   228,   465,   467,   187,
     229,   100,   230,   491,   231,   101,   102,   103,   232,   233,
     234,   499,   235,   236,   237,   104,   496,   497,   105,   509,
     238,   362,   106,   506,   503,   107,   483,   172,   211,   256,
     108,   109,   259,   273,   173,   271,   110,   111,   272,   553,
     554,   555,   556,   557,   558,   559,   274,   275,   276,   281,
     282,    35,   525,   526,   286,   528,   290,   288,   291,   504,
     485,   294,   295,   543,   300,   362,   166,   304,   306,   173,
     309,   314,   315,   317,   318,   319,   321,   448,   323,   322,
     325,   328,   333,   574,   573,   569,   341,   570,   575,   344,
     345,   346,    32,   402,   576,   546,   350,   352,   560,   367,
     368,   369,   370,   543,   371,   372,   373,   374,   375,   376,
     377,   563,   378,   379,   380,   384,   387,   571,   173,   389,
     566,   392,   393,   394,   397,   415,   420,   437,   447,   146,
     450,   451,   546,   453,   284,   460,   457,    28,   461,   464,
     469,   486,   252,   456,   487,   490,    35,   492,    71,   512,
     500,   515,   514,   524,   550,   540,   544,   577,   580,   581,
     590,   582,   583,   584,   585,   586,   587,   588,   568,   424,
      65,   259,   572,   173,   362,   480,   413,   444,   426,   595,
     308,   456,   303,   251,   250,   353,   362,    66,   466,    67,
      68,   594,    69,   541,    70,   146,   463,   565,   592,   593,
     160,   508,   362,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,     0,     0,     0,    77,     0,     0,     0,
      78,    79,    80,    81,    82,    65,   259,    83,    84,    85,
      86,     0,     0,     0,     0,     0,    87,     0,    88,    89,
      90,     0,    66,     0,    67,    68,     0,    69,     0,    70,
       0,   331,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    71,    72,     0,    73,    74,    75,    76,     0,     0,
       0,    77,     0,     0,     0,    78,    79,    80,    81,    82,
      65,     0,    83,    84,    85,    86,     0,     0,     0,     0,
       0,    87,     0,    88,    89,    90,     0,    66,     0,    67,
      68,     0,    69,     0,    70,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,     0,     0,     0,    77,     0,     0,     0,
      78,    79,    80,    81,    82,     0,     0,    83,    84,    85,
      86,     0,     0,     0,     0,     0,    87,     0,    88,    89,
      90
};

static const yytype_int16 yycheck[] =
{
      23,    69,    25,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    81,    60,    61,    62,   120,   123,
     118,    66,   119,    68,   124,    70,    85,    85,   193,   127,
     125,   129,    77,    78,    79,    80,    81,    82,   147,   264,
       6,    86,   203,   204,    89,    90,   128,   153,     3,   116,
       3,     4,   200,    73,    77,   198,   199,   297,   298,    82,
     297,   298,    85,     6,     6,     8,     3,     7,    11,     6,
     115,     7,   117,     3,     4,   120,   186,    68,   123,    68,
     190,     3,    71,    72,   151,    74,    25,    76,    28,    78,
      81,   316,   240,     3,   188,    32,   190,     7,   121,    41,
       3,     4,    41,    42,     3,   211,   151,    82,   218,     6,
      32,   193,   260,    10,     6,     7,     8,     3,     4,    11,
      86,    76,    77,    78,   218,   184,   115,     0,   402,    32,
     119,   405,   406,   122,    26,    78,     3,    78,   158,   198,
     199,   186,     3,    79,    85,   190,    76,    77,    78,    13,
      60,    61,   210,    33,   213,    19,    20,    21,   147,     6,
      40,     8,     3,     4,    11,   188,   331,   190,   157,     6,
     313,   280,   161,   218,   414,   198,   199,   414,    84,    26,
     169,    88,   288,     6,     7,     8,    12,   210,    11,     6,
      16,     8,   335,    16,   337,   218,   219,    23,    24,    25,
      69,    70,   427,    29,     6,     7,     8,   309,    22,    78,
      24,    25,   452,    27,     4,    29,     6,     3,   458,     6,
       7,     8,     3,     6,    11,    51,     7,    10,   113,    55,
      56,    57,   117,    13,    14,    15,    85,   462,   283,    65,
     299,   264,    56,    57,    52,    53,    54,    73,    83,   331,
      72,    65,   297,   298,    80,     6,    74,   418,    76,     7,
      86,    87,     3,     4,   309,   324,     6,     6,   327,     8,
     329,   330,    11,   262,   297,   298,     7,   336,    28,   338,
       9,    10,     7,   342,   343,   305,   511,    17,    18,     7,
     313,   280,   398,   316,     7,   443,   402,   356,   357,   405,
     406,     6,     7,   292,    60,    61,   365,   366,     6,     7,
       8,    43,   335,    11,   337,   304,    49,    50,   307,     6,
       7,     8,   483,     7,    11,    60,    61,   567,     6,     7,
       8,     6,    12,    11,    30,    31,    16,     3,     4,   579,
       4,   566,     4,    23,    24,    25,    17,    18,   407,    29,
      33,    34,    35,    36,    37,    38,    39,    40,     6,     6,
       6,     6,     6,   386,     6,     6,     6,   390,   391,   414,
       6,    51,     6,   432,     6,    55,    56,    57,     6,     6,
       6,   448,     6,     6,     6,    65,   445,   446,    68,   456,
       6,   414,    72,   452,   452,    75,   416,     3,     6,     6,
      80,    81,     6,     4,   427,     7,    86,    87,     7,   517,
     518,   519,   520,   521,   522,   523,     4,     4,     4,     4,
       4,   444,   481,   482,     4,   484,     4,    30,     4,   452,
     419,     4,     4,   500,     4,   458,    85,    48,     4,   462,
      29,     4,     4,     4,     4,     4,     4,    66,     4,     6,
       4,     4,     4,   557,   556,   553,     4,   554,   558,     4,
       4,     4,     3,    31,   559,   510,     4,     4,   527,     4,
       4,     4,     4,   540,     4,     4,     4,     4,     4,     4,
       4,   539,     4,     4,     4,     4,     4,   555,   511,     4,
     549,     4,     4,     4,     4,     4,     4,     4,     4,   488,
       4,     6,   547,     4,   571,     4,     6,     3,     6,     6,
       4,     4,   557,    16,     4,    32,   539,     4,    41,     4,
      67,    57,     6,    18,   513,    67,     4,     6,     4,     4,
     589,     4,     4,     4,     4,     4,     4,     4,   552,   313,
       5,     6,   555,   566,   567,   413,   301,   340,   313,   594,
     171,    16,   165,   122,   121,   215,   579,    22,   390,    24,
      25,    75,    27,   498,    29,   554,   386,   547,   591,   591,
      72,   452,   595,    -1,    -1,    -1,    41,    42,    -1,    44,
      45,    46,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,     5,     6,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    74,
      75,    -1,    22,    -1,    24,    25,    -1,    27,    -1,    29,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    -1,    44,    45,    46,    47,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
       5,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    73,    74,    75,    -1,    22,    -1,    24,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    -1,    44,
      45,    46,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    74,
      75
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   179,   180,    82,     0,     3,   184,     6,    84,
       3,   201,   181,   185,    88,   182,     3,   187,   202,     3,
       4,   183,    85,   186,   111,   112,    83,   188,     3,     4,
      92,   162,     3,     4,    91,    92,   104,   123,   124,   127,
     129,   147,   171,   172,     6,     3,     4,    72,    12,    16,
      23,    24,    25,    29,    51,    55,    56,    57,    65,    73,
      80,    86,    87,    68,    81,     5,    22,    24,    25,    27,
      29,    41,    42,    44,    45,    46,    47,    51,    55,    56,
      57,    58,    59,    62,    63,    64,    65,    71,    73,    74,
      75,     9,    10,    94,    12,    16,    23,    24,    25,    29,
      51,    55,    56,    57,    65,    68,    72,    75,    80,    81,
      86,    87,   160,     7,     7,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,     6,   142,   142,   142,
       7,     7,     6,    10,    90,     6,     8,    93,   101,   142,
       7,     6,     8,    11,    26,    95,    96,   102,   103,   142,
      28,   105,   106,    96,    97,   109,   142,    96,   119,   122,
     122,    43,   120,   131,   119,    78,    85,   132,   133,    78,
     119,   128,     3,    92,   110,   137,   142,    96,   138,   142,
     142,   143,   105,   142,   145,   146,    97,   142,   148,   149,
     198,    60,    61,    92,   142,   151,   152,   191,   192,   199,
      32,    92,   114,    17,    18,   154,     6,    91,    92,   114,
     153,     6,   142,   158,   159,     7,     6,   142,   200,   142,
       4,     6,     6,     6,     6,     6,     6,     6,     6,     6,
       6,     6,     6,     6,     6,     6,     6,     6,     6,     4,
      93,    96,   142,    97,    93,   142,    94,   103,   109,   142,
     137,   138,   142,   143,   146,   159,     6,    94,   176,     6,
     113,   190,    94,   193,   194,    69,    70,    78,   161,    60,
      61,     7,     7,     4,     4,     4,     4,    13,    14,    15,
     102,     4,     4,    16,    97,   142,     4,    95,    30,   107,
       4,     4,    96,   120,     4,     4,    96,    49,    50,   130,
       4,   120,   126,   132,    48,   134,     4,    96,   126,    29,
      52,    53,    54,   136,     4,     4,   139,     4,     4,     4,
     144,     4,     6,     4,   114,     4,   199,   192,     4,   192,
     199,    86,   190,     4,    92,   114,   123,   114,   123,   111,
     113,     4,   130,   130,     4,     4,     4,    91,    95,   155,
       4,   114,     4,   152,   163,   173,   192,   199,     4,    76,
      77,    78,    92,    98,   168,   170,   111,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     6,   177,     4,   111,   189,     4,    96,     4,
     110,   195,     4,     4,     4,   102,   142,     4,    17,    18,
      95,   100,    31,   108,    96,    30,    31,   121,    98,   198,
      98,   198,   114,   107,   125,     4,   135,    96,   120,    96,
       4,   109,   142,    92,   104,   123,   124,   110,     6,    10,
     141,   114,   150,   114,   114,   114,   190,     4,    92,   123,
     114,   123,   114,   115,   112,   114,   114,     4,    66,   156,
       4,     6,   165,     4,   114,   114,    16,     6,   114,   114,
       4,     6,   174,   162,     6,    92,   147,    92,   196,     4,
      95,    13,    19,    20,    21,    99,   100,   100,   100,   114,
     108,    98,   198,   120,   130,    96,     4,     4,    25,   110,
      32,   114,     4,   111,   116,     4,   114,   114,     7,    97,
      67,   157,     4,    91,    92,    98,   114,   164,   191,    97,
     169,    98,     4,   110,     6,    57,    22,    24,    25,    27,
      29,    56,    57,    65,    18,   114,   114,   130,   114,   140,
      33,    34,    35,    36,    37,    38,    39,    40,   117,   118,
      67,   157,     7,    97,     4,    79,   142,   166,   167,   110,
      96,   142,   197,   197,   197,   197,   197,   197,   197,   197,
     114,    33,    40,    91,     7,   166,   114,   178,   101,    94,
     103,   105,   106,   109,   143,   146,   159,     6,   110,    98,
       4,     4,     4,     4,     4,     4,     4,     4,     4,    98,
     114,   175,    92,   171,    75,   142
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    91,    92,    93,    93,    90,    94,    94,
      95,    95,    95,    96,    96,    96,    97,    97,    97,    97,
      98,    99,    99,    99,    99,   100,   101,    91,   102,   102,
     103,   104,    91,   105,   105,   106,    91,   107,   107,   108,
     108,   109,    91,   110,   111,   112,   112,   113,   113,   114,
     115,   114,   116,   116,   117,   117,   117,   117,   117,   117,
     117,   118,   117,   119,   120,   120,   121,   121,   121,   122,
     123,   124,    91,    91,   125,   126,   126,   127,   128,   128,
     129,   130,   130,   130,   130,   131,   127,   132,   132,   134,
     133,   135,   133,   129,    91,    91,   136,   136,   136,   136,
     137,   137,   137,    91,   139,   138,    91,   140,   140,   141,
     141,   142,   144,   143,    91,   145,   145,   146,   146,   146,
     147,    91,   148,   149,   149,   150,   149,   149,    91,   151,
     151,   151,   151,   151,   151,    91,   152,   152,   151,    91,
     153,    91,    91,    91,   154,   154,    91,   155,   155,   156,
     156,   157,   157,   158,   158,   158,   158,   159,    91,   160,
     161,   161,   161,   161,    91,    91,   162,   162,   162,    91,
     163,   163,   164,   164,   164,   164,   165,   165,   166,   166,
     167,   169,   168,   170,   170,   170,   170,   171,   172,    91,
      91,   173,   173,    91,   174,   174,   175,   175,   177,   178,
     176,    91,    91,   179,   181,   180,   182,   182,   183,   185,
     184,   186,   186,   187,   188,   188,   160,   160,   160,   160,
     160,   160,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   160,   160,   162,   162,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     189,   189,   190,   191,   192,   192,   193,   194,   195,   195,
     196,   196,   196,   196,   196,   196,   196,   196,   197,   198,
     199,   200,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,   110,   202,   201
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     4,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     2,     2,     2,     5,     3,     3,     1,
       4,     1,     1,     1,     1,     2,     1,     4,     1,     1,
       3,     4,     1,     0,     1,     2,     4,     0,     2,     0,
       2,     3,     4,     4,     0,     1,     2,     0,     1,     4,
       0,     5,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     0,     2,     3,     0,     2,     0,     2,     2,     4,
       4,     4,     1,     1,     2,     4,     4,     5,     1,     4,
       5,     2,     2,     2,     2,     3,     4,     0,     3,     0,
       5,     0,     6,     4,     1,     1,     0,     1,     1,     1,
       3,     3,     3,     4,     0,     4,     4,     0,     2,     1,
       1,     1,     0,     6,     4,     1,     2,     3,     2,     1,
       4,     1,     1,     3,     3,     0,     4,     3,     4,     2,
       3,     3,     2,     3,     3,     4,     1,     1,     1,     4,
       1,     4,     5,     4,     4,     4,     4,     1,     0,     2,
       0,     2,     0,     4,     5,     5,     6,     2,     4,     2,
       1,     1,     1,     0,     5,     5,     7,     5,     6,     6,
       0,     2,     1,     1,     1,     1,     0,     2,     1,     1,
       3,     0,     4,     1,     1,     1,     1,    10,     4,     1,
       1,     2,     2,     5,     0,     1,     0,     2,     0,     0,
      10,     5,     5,     3,     0,     6,     0,     2,     5,     0,
       6,     0,     2,     4,     0,     4,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     5,     5,     5,     5,     5,     5,     5,
       5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
       1,     2,     2,     4,     1,     3,     2,     4,     2,     2,
       5,     5,     5,     5,     5,     5,     5,     5,     1,     1,
       1,     1,     4,     5,     6,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     0,     5
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* glyph: UNSIGNED REFERENCE  */
#line 274 "parser.y"
                        {
	#line 421 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2063 "parser.c"
    break;

  case 3: /* content_node: start "glyph" glyph ">"  */
#line 277 "parser.y"
                                  {
	#line 422 "format.w"
	hput_tags((yyvsp[-3].u),hput_glyph(&((yyvsp[-1].c))));}
#line 2071 "parser.c"
    break;

  case 4: /* start: "<"  */
#line 280 "parser.y"
           {
	#line 423 "format.w"
	HPUTNODE;(yyval.u)= (uint32_t)(hpos++-hstart);}
#line 2079 "parser.c"
    break;

  case 6: /* integer: UNSIGNED  */
#line 284 "parser.y"
                         {
	#line 951 "format.w"
	RNG("number",(yyvsp[0].u),0,0x7FFFFFFF);}
#line 2087 "parser.c"
    break;

  case 7: /* glyph: CHARCODE REFERENCE  */
#line 288 "parser.y"
                        {
	#line 1093 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2095 "parser.c"
    break;

  case 9: /* string: CHARCODE  */
#line 292 "parser.y"
                         {
	#line 1198 "format.w"
	static char s[2];
	RNG("String element",(yyvsp[0].u),0x20,0x7E);
	s[0]= (yyvsp[0].u);s[1]= 0;(yyval.s)= s;}
#line 2105 "parser.c"
    break;

  case 10: /* number: UNSIGNED  */
#line 298 "parser.y"
               {
	#line 1351 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].u);}
#line 2113 "parser.c"
    break;

  case 11: /* number: SIGNED  */
#line 300 "parser.y"
                                       {
	#line 1351 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].i);}
#line 2121 "parser.c"
    break;

  case 13: /* dimension: number "pt"  */
#line 304 "parser.y"
                   {
	#line 1705 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2129 "parser.c"
    break;

  case 14: /* dimension: number "in"  */
#line 307 "parser.y"
                    {
	#line 1706 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*72.27);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2137 "parser.c"
    break;

  case 15: /* dimension: number "mm"  */
#line 310 "parser.y"
                  {
	#line 1707 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*(72.27/25.4));RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2145 "parser.c"
    break;

  case 16: /* xdimen: dimension number "h" number "v"  */
#line 314 "parser.y"
                                  {
	#line 1785 "format.w"
	(yyval.xd).w= (yyvsp[-4].d);(yyval.xd).h= (yyvsp[-3].f);(yyval.xd).v= (yyvsp[-1].f);}
#line 2153 "parser.c"
    break;

  case 17: /* xdimen: dimension number "h"  */
#line 317 "parser.y"
                           {
	#line 1786 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= (yyvsp[-1].f);(yyval.xd).v= 0.0;}
#line 2161 "parser.c"
    break;

  case 18: /* xdimen: dimension number "v"  */
#line 320 "parser.y"
                           {
	#line 1787 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= 0.0;(yyval.xd).v= (yyvsp[-1].f);}
#line 2169 "parser.c"
    break;

  case 19: /* xdimen: dimension  */
#line 323 "parser.y"
                  {
	#line 1788 "format.w"
	(yyval.xd).w= (yyvsp[0].d);(yyval.xd).h= 0.0;(yyval.xd).v= 0.0;}
#line 2177 "parser.c"
    break;

  case 20: /* xdimen_node: start "xdimen" xdimen ">"  */
#line 327 "parser.y"
                                   {
	#line 1790 "format.w"
	hput_tags((yyvsp[-3].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 2185 "parser.c"
    break;

  case 21: /* order: "pt"  */
#line 332 "parser.y"
        {
	#line 1969 "format.w"
	(yyval.o)= normal_o;}
#line 2193 "parser.c"
    break;

  case 22: /* order: "fil"  */
#line 334 "parser.y"
                            {
	#line 1969 "format.w"
	(yyval.o)= fil_o;}
#line 2201 "parser.c"
    break;

  case 23: /* order: "fill"  */
#line 336 "parser.y"
                             {
	#line 1969 "format.w"
	(yyval.o)= fill_o;}
#line 2209 "parser.c"
    break;

  case 24: /* order: "filll"  */
#line 338 "parser.y"
                              {
	#line 1969 "format.w"
	(yyval.o)= filll_o;}
#line 2217 "parser.c"
    break;

  case 25: /* stretch: number order  */
#line 342 "parser.y"
                    {
	#line 1971 "format.w"
	(yyval.st).f= (yyvsp[-1].f);(yyval.st).o= (yyvsp[0].o);}
#line 2225 "parser.c"
    break;

  case 26: /* penalty: integer  */
#line 346 "parser.y"
               {
	#line 2025 "format.w"
	RNG("Penalty",(yyvsp[0].i),-20000,+20000);(yyval.i)= (yyvsp[0].i);}
#line 2233 "parser.c"
    break;

  case 27: /* content_node: start "penalty" penalty ">"  */
#line 349 "parser.y"
                                      {
	#line 2026 "format.w"
	hput_tags((yyvsp[-3].u),hput_int((yyvsp[-1].i)));}
#line 2241 "parser.c"
    break;

  case 29: /* rule_dimension: "|"  */
#line 353 "parser.y"
                                        {
	#line 2203 "format.w"
	(yyval.d)= RUNNING_DIMEN;}
#line 2249 "parser.c"
    break;

  case 30: /* rule: rule_dimension rule_dimension rule_dimension  */
#line 357 "parser.y"
{
	#line 2205 "format.w"
	(yyval.r).h= (yyvsp[-2].d);(yyval.r).d= (yyvsp[-1].d);(yyval.r).w= (yyvsp[0].d);
	if((yyvsp[0].d)==RUNNING_DIMEN&&((yyvsp[-2].d)==RUNNING_DIMEN||(yyvsp[-1].d)==RUNNING_DIMEN))
	QUIT("Incompatible running dimensions 0x%x 0x%x 0x%x",(yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2259 "parser.c"
    break;

  case 31: /* rule_node: start "rule" rule ">"  */
#line 362 "parser.y"
                             {
	#line 2208 "format.w"
	hput_tags((yyvsp[-3].u),hput_rule(&((yyvsp[-1].r))));}
#line 2267 "parser.c"
    break;

  case 33: /* explicit: %empty  */
#line 367 "parser.y"
         {
	#line 2316 "format.w"
	(yyval.b)= false;}
#line 2275 "parser.c"
    break;

  case 34: /* explicit: "!"  */
#line 369 "parser.y"
                                 {
	#line 2316 "format.w"
	(yyval.b)= true;}
#line 2283 "parser.c"
    break;

  case 35: /* kern: explicit xdimen  */
#line 372 "parser.y"
                    {
	#line 2317 "format.w"
	(yyval.kt).x= (yyvsp[-1].b);(yyval.kt).d= (yyvsp[0].xd);}
#line 2291 "parser.c"
    break;

  case 36: /* content_node: start "kern" kern ">"  */
#line 375 "parser.y"
                                {
	#line 2318 "format.w"
	hput_tags((yyvsp[-3].u),hput_kern(&((yyvsp[-1].kt))));}
#line 2299 "parser.c"
    break;

  case 37: /* plus: %empty  */
#line 379 "parser.y"
     {
	#line 2528 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2307 "parser.c"
    break;

  case 38: /* plus: "plus" stretch  */
#line 381 "parser.y"
                                             {
	#line 2528 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2315 "parser.c"
    break;

  case 39: /* minus: %empty  */
#line 384 "parser.y"
      {
	#line 2529 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2323 "parser.c"
    break;

  case 40: /* minus: "minus" stretch  */
#line 386 "parser.y"
                                              {
	#line 2529 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2331 "parser.c"
    break;

  case 41: /* glue: xdimen plus minus  */
#line 389 "parser.y"
                      {
	#line 2530 "format.w"
	(yyval.g).w= (yyvsp[-2].xd);(yyval.g).p= (yyvsp[-1].st);(yyval.g).m= (yyvsp[0].st);}
#line 2339 "parser.c"
    break;

  case 42: /* content_node: start "glue" glue ">"  */
#line 392 "parser.y"
                                {
	#line 2531 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){HPUT8(zero_skip_no);
	hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}else hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));}
#line 2348 "parser.c"
    break;

  case 43: /* glue_node: start "glue" glue ">"  */
#line 397 "parser.y"
{
	#line 2534 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){hpos--;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));(yyval.b)= true;}}
#line 2357 "parser.c"
    break;

  case 44: /* position: %empty  */
#line 402 "parser.y"
         {
	#line 2809 "format.w"
	(yyval.u)= hpos-hstart;}
#line 2365 "parser.c"
    break;

  case 47: /* estimate: %empty  */
#line 407 "parser.y"
         {
	#line 2812 "format.w"
	hpos+= 2;}
#line 2373 "parser.c"
    break;

  case 48: /* estimate: UNSIGNED  */
#line 410 "parser.y"
                 {
	#line 2813 "format.w"
	hpos+= hsize_bytes((yyvsp[0].u))+1;}
#line 2381 "parser.c"
    break;

  case 49: /* list: start estimate content_list ">"  */
#line 414 "parser.y"
{
	#line 2815 "format.w"
	(yyval.l).t= TAG(list_kind,b010);(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2390 "parser.c"
    break;

  case 50: /* $@1: %empty  */
#line 420 "parser.y"
{
	#line 3229 "format.w"
	hpos+= 4;}
#line 2398 "parser.c"
    break;

  case 51: /* list: TXT_START position $@1 text TXT_END  */
#line 424 "parser.y"
{
	#line 3231 "format.w"
	(yyval.l).t= TAG(list_kind,b110);(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2407 "parser.c"
    break;

  case 54: /* txt: TXT_CC  */
#line 430 "parser.y"
          {
	#line 3235 "format.w"
	hput_txt_cc((yyvsp[0].u));}
#line 2415 "parser.c"
    break;

  case 55: /* txt: TXT_FONT  */
#line 433 "parser.y"
                 {
	#line 3236 "format.w"
	REF(font_kind,(yyvsp[0].u));hput_txt_font((yyvsp[0].u));}
#line 2423 "parser.c"
    break;

  case 56: /* txt: TXT_GLOBAL  */
#line 436 "parser.y"
                   {
	#line 3237 "format.w"
	REF((yyvsp[0].rf).k,(yyvsp[0].rf).n);hput_txt_global(&((yyvsp[0].rf)));}
#line 2431 "parser.c"
    break;

  case 57: /* txt: TXT_LOCAL  */
#line 439 "parser.y"
                  {
	#line 3238 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,11);hput_txt_local((yyvsp[0].u));}
#line 2439 "parser.c"
    break;

  case 58: /* txt: TXT_FONT_GLUE  */
#line 442 "parser.y"
                      {
	#line 3239 "format.w"
	HPUTX(1);HPUT8(txt_glue);}
#line 2447 "parser.c"
    break;

  case 59: /* txt: TXT_FONT_HYPHEN  */
#line 445 "parser.y"
                        {
	#line 3240 "format.w"
	HPUTX(1);HPUT8(txt_hyphen);}
#line 2455 "parser.c"
    break;

  case 60: /* txt: TXT_IGNORE  */
#line 448 "parser.y"
                   {
	#line 3241 "format.w"
	HPUTX(1);HPUT8(txt_ignore);}
#line 2463 "parser.c"
    break;

  case 61: /* $@2: %empty  */
#line 451 "parser.y"
         {
	#line 3242 "format.w"
	HPUTX(1);HPUT8(txt_node);}
#line 2471 "parser.c"
    break;

  case 63: /* box_dimen: dimension dimension dimension  */
#line 457 "parser.y"
{
	#line 3498 "format.w"
	(yyval.info)= hput_box_dimen((yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2479 "parser.c"
    break;

  case 64: /* box_shift: %empty  */
#line 460 "parser.y"
          {
	#line 3499 "format.w"
	(yyval.info)= b000;}
#line 2487 "parser.c"
    break;

  case 65: /* box_shift: "shifted" dimension  */
#line 463 "parser.y"
                          {
	#line 3500 "format.w"
	(yyval.info)= hput_box_shift((yyvsp[0].d));}
#line 2495 "parser.c"
    break;

  case 66: /* box_glue_set: %empty  */
#line 467 "parser.y"
             {
	#line 3502 "format.w"
	(yyval.info)= b000;}
#line 2503 "parser.c"
    break;

  case 67: /* box_glue_set: "plus" stretch  */
#line 470 "parser.y"
                     {
	#line 3503 "format.w"
	(yyval.info)= hput_box_glue_set(+1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2511 "parser.c"
    break;

  case 68: /* box_glue_set: "minus" stretch  */
#line 473 "parser.y"
                      {
	#line 3504 "format.w"
	(yyval.info)= hput_box_glue_set(-1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2519 "parser.c"
    break;

  case 69: /* box: box_dimen box_shift box_glue_set list  */
#line 478 "parser.y"
                                         {
	#line 3507 "format.w"
	(yyval.info)= (yyvsp[-3].info)	|(yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2527 "parser.c"
    break;

  case 70: /* hbox_node: start "hbox" box ">"  */
#line 482 "parser.y"
                            {
	#line 3509 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hbox_kind,(yyvsp[-1].info)));}
#line 2535 "parser.c"
    break;

  case 71: /* vbox_node: start "vbox" box ">"  */
#line 485 "parser.y"
                            {
	#line 3510 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vbox_kind,(yyvsp[-1].info)));}
#line 2543 "parser.c"
    break;

  case 74: /* box_flex: plus minus  */
#line 490 "parser.y"
                   {
	#line 3706 "format.w"
	hput_stretch(&((yyvsp[-1].st)));hput_stretch(&((yyvsp[0].st)));}
#line 2551 "parser.c"
    break;

  case 75: /* box_options: box_shift box_flex xdimen_ref list  */
#line 494 "parser.y"
                                              {
	#line 3708 "format.w"
	(yyval.info)= (yyvsp[-3].info);}
#line 2559 "parser.c"
    break;

  case 76: /* box_options: box_shift box_flex xdimen_node list  */
#line 497 "parser.y"
                                            {
	#line 3709 "format.w"
	(yyval.info)= (yyvsp[-3].info)	|b100;}
#line 2567 "parser.c"
    break;

  case 77: /* hxbox_node: start "hset" box_dimen box_options ">"  */
#line 501 "parser.y"
                                               {
	#line 3711 "format.w"
	hput_tags((yyvsp[-4].u),TAG(hset_kind,(yyvsp[-2].info)	|(yyvsp[-1].info)));}
#line 2575 "parser.c"
    break;

  case 79: /* vbox_dimen: "top" dimension dimension dimension  */
#line 507 "parser.y"
{
	#line 3715 "format.w"
	(yyval.info)= hput_box_dimen((yyvsp[-2].d),(yyvsp[-1].d)^0x40000000,(yyvsp[0].d));}
#line 2583 "parser.c"
    break;

  case 80: /* vxbox_node: start "vset" vbox_dimen box_options ">"  */
#line 511 "parser.y"
                                                {
	#line 3717 "format.w"
	hput_tags((yyvsp[-4].u),TAG(vset_kind,(yyvsp[-2].info)	|(yyvsp[-1].info)));}
#line 2591 "parser.c"
    break;

  case 81: /* box_goal: "to" xdimen_ref  */
#line 515 "parser.y"
                      {
	#line 3719 "format.w"
	(yyval.info)= b000;}
#line 2599 "parser.c"
    break;

  case 82: /* box_goal: "add" xdimen_ref  */
#line 518 "parser.y"
                       {
	#line 3720 "format.w"
	(yyval.info)= b001;}
#line 2607 "parser.c"
    break;

  case 83: /* box_goal: "to" xdimen_node  */
#line 521 "parser.y"
                       {
	#line 3721 "format.w"
	(yyval.info)= b100;}
#line 2615 "parser.c"
    break;

  case 84: /* box_goal: "add" xdimen_node  */
#line 524 "parser.y"
                        {
	#line 3722 "format.w"
	(yyval.info)= b101;}
#line 2623 "parser.c"
    break;

  case 85: /* hpack: box_shift box_goal list  */
#line 528 "parser.y"
                             {
	#line 3724 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 2631 "parser.c"
    break;

  case 86: /* hxbox_node: start "hpack" hpack ">"  */
#line 532 "parser.y"
                                {
	#line 3726 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hpack_kind,(yyvsp[-1].info)));}
#line 2639 "parser.c"
    break;

  case 87: /* max_depth: %empty  */
#line 536 "parser.y"
          {
	#line 3728 "format.w"
	(yyval.d)= MAX_DIMEN;}
#line 2647 "parser.c"
    break;

  case 88: /* max_depth: "max" "depth" dimension  */
#line 538 "parser.y"
                                            {
	#line 3728 "format.w"
	(yyval.d)= (yyvsp[0].d);}
#line 2655 "parser.c"
    break;

  case 89: /* $@3: %empty  */
#line 542 "parser.y"
               {
	#line 3730 "format.w"
	HPUT32((yyvsp[0].d));}
#line 2663 "parser.c"
    break;

  case 90: /* vpack: max_depth $@3 box_shift box_goal list  */
#line 544 "parser.y"
                                           {
	#line 3730 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2671 "parser.c"
    break;

  case 91: /* $@4: %empty  */
#line 547 "parser.y"
                      {
	#line 3731 "format.w"
	HPUT32((yyvsp[0].d)^0x40000000);}
#line 2679 "parser.c"
    break;

  case 92: /* vpack: "top" max_depth $@4 box_shift box_goal list  */
#line 549 "parser.y"
                                                      {
	#line 3731 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2687 "parser.c"
    break;

  case 93: /* vxbox_node: start "vpack" vpack ">"  */
#line 553 "parser.y"
                                {
	#line 3733 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vpack_kind,(yyvsp[-1].info)));}
#line 2695 "parser.c"
    break;

  case 96: /* ltype: %empty  */
#line 559 "parser.y"
      {
	#line 3847 "format.w"
	(yyval.info)= 1;}
#line 2703 "parser.c"
    break;

  case 97: /* ltype: "align"  */
#line 561 "parser.y"
                      {
	#line 3847 "format.w"
	(yyval.info)= 1;}
#line 2711 "parser.c"
    break;

  case 98: /* ltype: "center"  */
#line 563 "parser.y"
                       {
	#line 3847 "format.w"
	(yyval.info)= 2;}
#line 2719 "parser.c"
    break;

  case 99: /* ltype: "expand"  */
#line 565 "parser.y"
                       {
	#line 3847 "format.w"
	(yyval.info)= 3;}
#line 2727 "parser.c"
    break;

  case 100: /* leaders: glue_node ltype rule_node  */
#line 568 "parser.y"
                                 {
	#line 3848 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2735 "parser.c"
    break;

  case 101: /* leaders: glue_node ltype hbox_node  */
#line 571 "parser.y"
                                  {
	#line 3849 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2743 "parser.c"
    break;

  case 102: /* leaders: glue_node ltype vbox_node  */
#line 574 "parser.y"
                                  {
	#line 3850 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2751 "parser.c"
    break;

  case 103: /* content_node: start "leaders" leaders ">"  */
#line 577 "parser.y"
                                      {
	#line 3851 "format.w"
	hput_tags((yyvsp[-3].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 2759 "parser.c"
    break;

  case 104: /* $@5: %empty  */
#line 581 "parser.y"
                  {
	#line 3957 "format.w"
	if((yyvsp[0].d)!=0)HPUT32((yyvsp[0].d));}
#line 2767 "parser.c"
    break;

  case 105: /* baseline: dimension $@5 glue_node glue_node  */
#line 584 "parser.y"
                   {
	#line 3958 "format.w"
	(yyval.info)= b000;if((yyvsp[-3].d)!=0)(yyval.info)	|= b001;
	if((yyvsp[-1].b))(yyval.info)	|= b100;
	if((yyvsp[0].b))(yyval.info)	|= b010;
	}
#line 2778 "parser.c"
    break;

  case 106: /* content_node: start "baseline" baseline ">"  */
#line 591 "parser.y"
{
	#line 3963 "format.w"
	if((yyvsp[-1].info)==b000)HPUT8(0);hput_tags((yyvsp[-3].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 2786 "parser.c"
    break;

  case 108: /* cc_list: cc_list TXT_CC  */
#line 595 "parser.y"
                               {
	#line 4046 "format.w"
	hput_utf8((yyvsp[0].u));}
#line 2794 "parser.c"
    break;

  case 109: /* lig_cc: UNSIGNED  */
#line 598 "parser.y"
               {
	#line 4047 "format.w"
	RNG("UTF-8 code",(yyvsp[0].u),0,0x1FFFFF);(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2802 "parser.c"
    break;

  case 110: /* lig_cc: CHARCODE  */
#line 601 "parser.y"
               {
	#line 4048 "format.w"
	(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2810 "parser.c"
    break;

  case 111: /* ref: REFERENCE  */
#line 604 "parser.y"
             {
	#line 4049 "format.w"
	HPUT8((yyvsp[0].u));(yyval.u)= (yyvsp[0].u);}
#line 2818 "parser.c"
    break;

  case 112: /* $@6: %empty  */
#line 607 "parser.y"
            {
	#line 4050 "format.w"
	REF(font_kind,(yyvsp[0].u));}
#line 2826 "parser.c"
    break;

  case 113: /* ligature: ref $@6 lig_cc TXT_START cc_list TXT_END  */
#line 610 "parser.y"
{
	#line 4051 "format.w"
	(yyval.lg).f= (yyvsp[-5].u);(yyval.lg).l.p= (yyvsp[-3].u);(yyval.lg).l.s= (hpos-hstart)-(yyvsp[-3].u);
	RNG("Ligature size",(yyval.lg).l.s,0,255);}
#line 2835 "parser.c"
    break;

  case 114: /* content_node: start "ligature" ligature ">"  */
#line 614 "parser.y"
                                        {
	#line 4053 "format.w"
	hput_tags((yyvsp[-3].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 2843 "parser.c"
    break;

  case 115: /* replace_count: explicit  */
#line 618 "parser.y"
                      {
	#line 4163 "format.w"
	if((yyvsp[0].b)){(yyval.u)= 0x80;HPUT8(0x80);}else (yyval.u)= 0x00;}
#line 2851 "parser.c"
    break;

  case 116: /* replace_count: explicit UNSIGNED  */
#line 621 "parser.y"
                          {
	#line 4164 "format.w"
	RNG("Replace count",(yyvsp[0].u),0,31);
	(yyval.u)= ((yyvsp[0].u))	|(((yyvsp[-1].b))?0x80:0x00);if((yyval.u)!=0)HPUT8((yyval.u));}
#line 2860 "parser.c"
    break;

  case 117: /* disc: replace_count list list  */
#line 625 "parser.y"
                            {
	#line 4166 "format.w"
	(yyval.dc).r= (yyvsp[-2].u);(yyval.dc).p= (yyvsp[-1].l);(yyval.dc).q= (yyvsp[0].l);
	if((yyvsp[0].l).s==0){hpos= hpos-3;if((yyvsp[-1].l).s==0)hpos= hpos-3;}}
#line 2869 "parser.c"
    break;

  case 118: /* disc: replace_count list  */
#line 629 "parser.y"
                           {
	#line 4168 "format.w"
	(yyval.dc).r= (yyvsp[-1].u);(yyval.dc).p= (yyvsp[0].l);if((yyvsp[0].l).s==0)hpos= hpos-3;(yyval.dc).q.s= 0;}
#line 2877 "parser.c"
    break;

  case 119: /* disc: replace_count  */
#line 632 "parser.y"
                      {
	#line 4169 "format.w"
	(yyval.dc).r= (yyvsp[0].u);(yyval.dc).p.s= 0;(yyval.dc).q.s= 0;}
#line 2885 "parser.c"
    break;

  case 120: /* disc_node: start "disc" disc ">"  */
#line 638 "parser.y"
{
	#line 4173 "format.w"
	hput_tags((yyvsp[-3].u),hput_disc(&((yyvsp[-1].dc))));}
#line 2893 "parser.c"
    break;

  case 122: /* par_dimen: xdimen  */
#line 644 "parser.y"
                {
	#line 4325 "format.w"
	hput_xdimen_node(&((yyvsp[0].xd)));}
#line 2901 "parser.c"
    break;

  case 123: /* par: xdimen_ref param_ref list  */
#line 647 "parser.y"
                             {
	#line 4326 "format.w"
	(yyval.info)= b000;}
#line 2909 "parser.c"
    break;

  case 124: /* par: xdimen_ref param_list list  */
#line 650 "parser.y"
                                   {
	#line 4327 "format.w"
	(yyval.info)= b010;}
#line 2917 "parser.c"
    break;

  case 125: /* $@7: %empty  */
#line 653 "parser.y"
                         {
	#line 4328 "format.w"
	hput_xdimen_node(&((yyvsp[-1].xd)));}
#line 2925 "parser.c"
    break;

  case 126: /* par: xdimen param_ref $@7 list  */
#line 655 "parser.y"
                                     {
	#line 4328 "format.w"
	(yyval.info)= b100;}
#line 2933 "parser.c"
    break;

  case 127: /* par: par_dimen param_list list  */
#line 658 "parser.y"
                                  {
	#line 4329 "format.w"
	(yyval.info)= b110;}
#line 2941 "parser.c"
    break;

  case 128: /* content_node: start "par" par ">"  */
#line 662 "parser.y"
                              {
	#line 4331 "format.w"
	hput_tags((yyvsp[-3].u),TAG(par_kind,(yyvsp[-1].info)));}
#line 2949 "parser.c"
    break;

  case 129: /* math: param_ref list  */
#line 666 "parser.y"
                   {
	#line 4397 "format.w"
	(yyval.info)= b000;}
#line 2957 "parser.c"
    break;

  case 130: /* math: param_ref list hbox_node  */
#line 669 "parser.y"
                                 {
	#line 4398 "format.w"
	(yyval.info)= b001;}
#line 2965 "parser.c"
    break;

  case 131: /* math: param_ref hbox_node list  */
#line 672 "parser.y"
                                 {
	#line 4399 "format.w"
	(yyval.info)= b010;}
#line 2973 "parser.c"
    break;

  case 132: /* math: param_list list  */
#line 675 "parser.y"
                        {
	#line 4400 "format.w"
	(yyval.info)= b100;}
#line 2981 "parser.c"
    break;

  case 133: /* math: param_list list hbox_node  */
#line 678 "parser.y"
                                  {
	#line 4401 "format.w"
	(yyval.info)= b101;}
#line 2989 "parser.c"
    break;

  case 134: /* math: param_list hbox_node list  */
#line 681 "parser.y"
                                  {
	#line 4402 "format.w"
	(yyval.info)= b110;}
#line 2997 "parser.c"
    break;

  case 135: /* content_node: start "math" math ">"  */
#line 685 "parser.y"
                                {
	#line 4404 "format.w"
	hput_tags((yyvsp[-3].u),TAG(math_kind,(yyvsp[-1].info)));}
#line 3005 "parser.c"
    break;

  case 136: /* on_off: "on"  */
#line 689 "parser.y"
         {
	#line 4454 "format.w"
	(yyval.i)= 1;}
#line 3013 "parser.c"
    break;

  case 137: /* on_off: "off"  */
#line 691 "parser.y"
                    {
	#line 4454 "format.w"
	(yyval.i)= 0;}
#line 3021 "parser.c"
    break;

  case 138: /* math: on_off  */
#line 694 "parser.y"
           {
	#line 4455 "format.w"
	(yyval.info)= b011	|((yyvsp[0].i)<<2);}
#line 3029 "parser.c"
    break;

  case 139: /* content_node: start "adjust" list ">"  */
#line 698 "parser.y"
                                  {
	#line 4486 "format.w"
	hput_tags((yyvsp[-3].u),TAG(adjust_kind,1));}
#line 3037 "parser.c"
    break;

  case 140: /* span_count: UNSIGNED  */
#line 702 "parser.y"
                   {
	#line 4585 "format.w"
	(yyval.info)= hput_span_count((yyvsp[0].u));}
#line 3045 "parser.c"
    break;

  case 141: /* content_node: start "item" content_node ">"  */
#line 705 "parser.y"
                                        {
	#line 4586 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,1));}
#line 3053 "parser.c"
    break;

  case 142: /* content_node: start "item" span_count content_node ">"  */
#line 708 "parser.y"
                                                   {
	#line 4587 "format.w"
	hput_tags((yyvsp[-4].u),TAG(item_kind,(yyvsp[-2].info)));}
#line 3061 "parser.c"
    break;

  case 143: /* content_node: start "item" list ">"  */
#line 711 "parser.y"
                                {
	#line 4588 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,b000));}
#line 3069 "parser.c"
    break;

  case 144: /* table: "h" box_goal list list  */
#line 715 "parser.y"
                          {
	#line 4590 "format.w"
	(yyval.info)= (yyvsp[-2].info);}
#line 3077 "parser.c"
    break;

  case 145: /* table: "v" box_goal list list  */
#line 718 "parser.y"
                          {
	#line 4591 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|b010;}
#line 3085 "parser.c"
    break;

  case 146: /* content_node: start "table" table ">"  */
#line 722 "parser.y"
                                  {
	#line 4593 "format.w"
	hput_tags((yyvsp[-3].u),TAG(table_kind,(yyvsp[-1].info)));}
#line 3093 "parser.c"
    break;

  case 147: /* image_aspect: number  */
#line 726 "parser.y"
                   {
	#line 4731 "format.w"
	(yyval.f)= (yyvsp[0].f);}
#line 3101 "parser.c"
    break;

  case 148: /* image_aspect: %empty  */
#line 728 "parser.y"
                         {
	#line 4731 "format.w"
	(yyval.f)= 0.0;}
#line 3109 "parser.c"
    break;

  case 149: /* image_width: "width" xdimen  */
#line 731 "parser.y"
                        {
	#line 4732 "format.w"
	(yyval.xd)= (yyvsp[0].xd);}
#line 3117 "parser.c"
    break;

  case 150: /* image_width: %empty  */
#line 734 "parser.y"
         {
	#line 4733 "format.w"
	(yyval.xd)= xdimen_defaults[zero_xdimen_no];}
#line 3125 "parser.c"
    break;

  case 151: /* image_height: "height" xdimen  */
#line 737 "parser.y"
                          {
	#line 4734 "format.w"
	(yyval.xd)= (yyvsp[0].xd);}
#line 3133 "parser.c"
    break;

  case 152: /* image_height: %empty  */
#line 740 "parser.y"
         {
	#line 4735 "format.w"
	(yyval.xd)= xdimen_defaults[zero_xdimen_no];}
#line 3141 "parser.c"
    break;

  case 153: /* image_spec: UNSIGNED image_aspect image_width image_height  */
#line 745 "parser.y"
{
	#line 4738 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-3].u),(yyvsp[-2].f),0,&((yyvsp[-1].xd)),0,&((yyvsp[0].xd)));}
#line 3149 "parser.c"
    break;

  case 154: /* image_spec: UNSIGNED image_aspect "width" REFERENCE image_height  */
#line 749 "parser.y"
{
	#line 4740 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-4].u),(yyvsp[-3].f),(yyvsp[-1].u),NULL,0,&((yyvsp[0].xd)));}
#line 3157 "parser.c"
    break;

  case 155: /* image_spec: UNSIGNED image_aspect image_width "height" REFERENCE  */
#line 753 "parser.y"
{
	#line 4742 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-4].u),(yyvsp[-3].f),0,&((yyvsp[-2].xd)),(yyvsp[0].u),NULL);}
#line 3165 "parser.c"
    break;

  case 156: /* image_spec: UNSIGNED image_aspect "width" REFERENCE "height" REFERENCE  */
#line 757 "parser.y"
{
	#line 4744 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-5].u),(yyvsp[-4].f),(yyvsp[-2].u),NULL,(yyvsp[0].u),NULL);}
#line 3173 "parser.c"
    break;

  case 157: /* image: image_spec list  */
#line 761 "parser.y"
                     {
	#line 4746 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 3181 "parser.c"
    break;

  case 158: /* content_node: start "image" image ">"  */
#line 765 "parser.y"
                                  {
	#line 4748 "format.w"
	hput_tags((yyvsp[-3].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3189 "parser.c"
    break;

  case 159: /* max_value: "outline" UNSIGNED  */
#line 769 "parser.y"
                          {
	#line 5373 "format.w"
	max_outline= (yyvsp[0].u);
	RNG("max outline",max_outline,0,0xFFFF);
	DBG(DBGDEF	|DBGLABEL,"Setting max outline to %d\n",max_outline);
	}
#line 3200 "parser.c"
    break;

  case 160: /* placement: "top"  */
#line 776 "parser.y"
             {
	#line 5465 "format.w"
	(yyval.i)= LABEL_TOP;}
#line 3208 "parser.c"
    break;

  case 161: /* placement: "bot"  */
#line 778 "parser.y"
                            {
	#line 5465 "format.w"
	(yyval.i)= LABEL_BOT;}
#line 3216 "parser.c"
    break;

  case 162: /* placement: "mid"  */
#line 780 "parser.y"
                            {
	#line 5465 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3224 "parser.c"
    break;

  case 163: /* placement: %empty  */
#line 782 "parser.y"
                         {
	#line 5465 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3232 "parser.c"
    break;

  case 164: /* content_node: "<" "label" REFERENCE placement ">"  */
#line 786 "parser.y"
{
	#line 5467 "format.w"
	hset_label((yyvsp[-2].u),(yyvsp[-1].i));}
#line 3240 "parser.c"
    break;

  case 165: /* content_node: start "link" REFERENCE on_off ">"  */
#line 791 "parser.y"
{
	#line 5725 "format.w"
	hput_tags((yyvsp[-4].u),hput_link((yyvsp[-2].u),(yyvsp[-1].i)));}
#line 3248 "parser.c"
    break;

  case 166: /* def_node: "<" "outline" REFERENCE integer position list ">"  */
#line 795 "parser.y"
                                                          {
	#line 5855 "format.w"
	
	static int outline_no= -1;
	(yyval.rf).k= outline_kind;(yyval.rf).n= (yyvsp[-4].u);
	if((yyvsp[-1].l).s==0)QUIT("Outline with empty title in line %d",yylineno);
	outline_no++;
	hset_outline(outline_no,(yyvsp[-4].u),(yyvsp[-3].i),(yyvsp[-2].u));
	}
#line 3262 "parser.c"
    break;

  case 167: /* def_node: start "unknown" UNSIGNED UNSIGNED ">"  */
#line 805 "parser.y"
                                            {
	#line 6013 "format.w"
	hput_tags((yyvsp[-4].u),hput_unknown_def((yyvsp[-2].u),(yyvsp[-1].u),0));}
#line 3270 "parser.c"
    break;

  case 168: /* def_node: start "unknown" UNSIGNED UNSIGNED UNSIGNED ">"  */
#line 808 "parser.y"
                                                     {
	#line 6014 "format.w"
	hput_tags((yyvsp[-5].u),hput_unknown_def((yyvsp[-3].u),(yyvsp[-2].u),(yyvsp[-1].u)));}
#line 3278 "parser.c"
    break;

  case 169: /* content_node: start "unknown" UNSIGNED unknown_bytes unknown_nodes ">"  */
#line 812 "parser.y"
                                                                   {
	#line 6026 "format.w"
	hput_tags((yyvsp[-5].u),hput_unknown((yyvsp[-5].u),(yyvsp[-3].u),(yyvsp[-2].u),(yyvsp[-1].u)));}
#line 3286 "parser.c"
    break;

  case 170: /* unknown_bytes: %empty  */
#line 815 "parser.y"
              {
	#line 6027 "format.w"
	(yyval.u)= 0;}
#line 3294 "parser.c"
    break;

  case 171: /* unknown_bytes: unknown_bytes UNSIGNED  */
#line 817 "parser.y"
                                       {
	#line 6027 "format.w"
	RNG("byte",(yyvsp[0].u),0,0xFF);HPUT8((yyvsp[0].u));(yyval.u)= (yyvsp[-1].u)+1;}
#line 3302 "parser.c"
    break;

  case 176: /* unknown_nodes: %empty  */
#line 821 "parser.y"
              {
	#line 6029 "format.w"
	(yyval.u)= 0;}
#line 3310 "parser.c"
    break;

  case 177: /* unknown_nodes: unknown_nodes unknown_node  */
#line 823 "parser.y"
                                           {
	#line 6029 "format.w"
	RNG("unknown subnodes",(yyvsp[-1].u),0,3);(yyval.u)= (yyvsp[-1].u)+1;}
#line 3318 "parser.c"
    break;

  case 178: /* stream_link: ref  */
#line 827 "parser.y"
               {
	#line 6464 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3326 "parser.c"
    break;

  case 179: /* stream_link: "*"  */
#line 829 "parser.y"
                                                    {
	#line 6464 "format.w"
	HPUT8(255);}
#line 3334 "parser.c"
    break;

  case 180: /* stream_split: stream_link stream_link UNSIGNED  */
#line 832 "parser.y"
                                             {
	#line 6465 "format.w"
	RNG("split ratio",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3342 "parser.c"
    break;

  case 181: /* $@8: %empty  */
#line 835 "parser.y"
                                {
	#line 6466 "format.w"
	RNG("magnification factor",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3350 "parser.c"
    break;

  case 183: /* stream_type: stream_info  */
#line 839 "parser.y"
                       {
	#line 6468 "format.w"
	(yyval.info)= 0;}
#line 3358 "parser.c"
    break;

  case 184: /* stream_type: "first"  */
#line 841 "parser.y"
                      {
	#line 6468 "format.w"
	(yyval.info)= 1;}
#line 3366 "parser.c"
    break;

  case 185: /* stream_type: "last"  */
#line 843 "parser.y"
                     {
	#line 6468 "format.w"
	(yyval.info)= 2;}
#line 3374 "parser.c"
    break;

  case 186: /* stream_type: "top"  */
#line 845 "parser.y"
                    {
	#line 6468 "format.w"
	(yyval.info)= 3;}
#line 3382 "parser.c"
    break;

  case 187: /* stream_def_node: start "stream (definition)" ref stream_type list xdimen_node glue_node list glue_node ">"  */
#line 851 "parser.y"
{
	#line 6472 "format.w"
	DEF((yyval.rf),stream_kind,(yyvsp[-7].u));hput_tags((yyvsp[-9].u),TAG(stream_kind,(yyvsp[-6].info)	|b100));}
#line 3390 "parser.c"
    break;

  case 188: /* stream_ins_node: start "stream (definition)" ref ">"  */
#line 856 "parser.y"
{
	#line 6475 "format.w"
	RNG("Stream insertion",(yyvsp[-1].u),0,max_ref[stream_kind]);hput_tags((yyvsp[-3].u),TAG(stream_kind,b100));}
#line 3398 "parser.c"
    break;

  case 191: /* stream: param_list list  */
#line 862 "parser.y"
                      {
	#line 6570 "format.w"
	(yyval.info)= b010;}
#line 3406 "parser.c"
    break;

  case 192: /* stream: param_ref list  */
#line 865 "parser.y"
                       {
	#line 6571 "format.w"
	(yyval.info)= b000;}
#line 3414 "parser.c"
    break;

  case 193: /* content_node: start "stream" stream_ref stream ">"  */
#line 869 "parser.y"
{
	#line 6573 "format.w"
	hput_tags((yyvsp[-4].u),TAG(stream_kind,(yyvsp[-1].info)));}
#line 3422 "parser.c"
    break;

  case 194: /* page_priority: %empty  */
#line 873 "parser.y"
              {
	#line 6676 "format.w"
	HPUT8(1);}
#line 3430 "parser.c"
    break;

  case 195: /* page_priority: UNSIGNED  */
#line 876 "parser.y"
                 {
	#line 6677 "format.w"
	RNG("page priority",(yyvsp[0].u),0,255);HPUT8((yyvsp[0].u));}
#line 3438 "parser.c"
    break;

  case 198: /* $@9: %empty  */
#line 882 "parser.y"
           {
	#line 6681 "format.w"
	hput_string((yyvsp[0].s));}
#line 3446 "parser.c"
    break;

  case 199: /* $@10: %empty  */
#line 884 "parser.y"
                                                          {
	#line 6681 "format.w"
	HPUT32((yyvsp[0].d));}
#line 3454 "parser.c"
    break;

  case 201: /* content_node: "<" "range" REFERENCE "on" ">"  */
#line 891 "parser.y"
                                         {
	#line 6794 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),true);}
#line 3462 "parser.c"
    break;

  case 202: /* content_node: "<" "range" REFERENCE "off" ">"  */
#line 894 "parser.y"
                                      {
	#line 6795 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),false);}
#line 3470 "parser.c"
    break;

  case 204: /* $@11: %empty  */
#line 900 "parser.y"
                                          {
	#line 7493 "format.w"
	new_directory((yyvsp[0].u)+1);new_output_buffers();}
#line 3478 "parser.c"
    break;

  case 208: /* entry: "<" "entry" UNSIGNED string ">"  */
#line 905 "parser.y"
{
	#line 7496 "format.w"
	RNG("Section number",(yyvsp[-2].u),3,max_section_no);hset_entry(&(dir[(yyvsp[-2].u)]),(yyvsp[-2].u),0,0,(yyvsp[-1].s));}
#line 3486 "parser.c"
    break;

  case 209: /* $@12: %empty  */
#line 909 "parser.y"
                                    {
	#line 8039 "format.w"
	hput_definitions_start();}
#line 3494 "parser.c"
    break;

  case 210: /* definition_section: "<" "definitions" $@12 max_definitions definition_list ">"  */
#line 913 "parser.y"
   {
	#line 8041 "format.w"
	hput_definitions_end();}
#line 3502 "parser.c"
    break;

  case 213: /* max_definitions: "<" "max" max_list ">"  */
#line 919 "parser.y"
{
	#line 8157 "format.w"
		/*253:*/
	if(max_ref[label_kind]>=0)
	ALLOCATE(labels,max_ref[label_kind]+1,Label);
		/*:253*/	/*274:*/
	if(max_outline>=0)
	ALLOCATE(outlines,max_outline+1,Outline);
		/*:274*/	/*310:*/
	ALLOCATE(page_on,max_ref[page_kind]+1,int);
	ALLOCATE(range_pos,2*(max_ref[range_kind]+1),RangePos);
		/*:310*/	/*376:*/
	definition_bits[0][list_kind]= (1<<(MAX_LIST_DEFAULT+1))-1;
	definition_bits[0][param_kind]= (1<<(MAX_LIST_DEFAULT+1))-1;
	definition_bits[0][int_kind]= (1<<(MAX_INT_DEFAULT+1))-1;
	definition_bits[0][dimen_kind]= (1<<(MAX_DIMEN_DEFAULT+1))-1;
	definition_bits[0][xdimen_kind]= (1<<(MAX_XDIMEN_DEFAULT+1))-1;
	definition_bits[0][glue_kind]= (1<<(MAX_GLUE_DEFAULT+1))-1;
	definition_bits[0][baseline_kind]= (1<<(MAX_BASELINE_DEFAULT+1))-1;
	definition_bits[0][page_kind]= (1<<(MAX_PAGE_DEFAULT+1))-1;
	definition_bits[0][stream_kind]= (1<<(MAX_STREAM_DEFAULT+1))-1;
	definition_bits[0][range_kind]= (1<<(MAX_RANGE_DEFAULT+1))-1;
		/*:376*/	/*391:*/
	ALLOCATE(hfont_name,max_ref[font_kind]+1,char*);
		/*:391*/hput_max_definitions();}
#line 3532 "parser.c"
    break;

  case 216: /* max_value: "font" UNSIGNED  */
#line 947 "parser.y"
                       {
	#line 8161 "format.w"
	hset_max(font_kind,(yyvsp[0].u));}
#line 3540 "parser.c"
    break;

  case 217: /* max_value: "int" UNSIGNED  */
#line 950 "parser.y"
                         {
	#line 8162 "format.w"
	hset_max(int_kind,(yyvsp[0].u));}
#line 3548 "parser.c"
    break;

  case 218: /* max_value: "dimen" UNSIGNED  */
#line 953 "parser.y"
                       {
	#line 8163 "format.w"
	hset_max(dimen_kind,(yyvsp[0].u));}
#line 3556 "parser.c"
    break;

  case 219: /* max_value: "ligature" UNSIGNED  */
#line 956 "parser.y"
                          {
	#line 8164 "format.w"
	hset_max(ligature_kind,(yyvsp[0].u));}
#line 3564 "parser.c"
    break;

  case 220: /* max_value: "disc" UNSIGNED  */
#line 959 "parser.y"
                      {
	#line 8165 "format.w"
	hset_max(disc_kind,(yyvsp[0].u));}
#line 3572 "parser.c"
    break;

  case 221: /* max_value: "glue" UNSIGNED  */
#line 962 "parser.y"
                      {
	#line 8166 "format.w"
	hset_max(glue_kind,(yyvsp[0].u));}
#line 3580 "parser.c"
    break;

  case 222: /* max_value: "language" UNSIGNED  */
#line 965 "parser.y"
                          {
	#line 8167 "format.w"
	hset_max(language_kind,(yyvsp[0].u));}
#line 3588 "parser.c"
    break;

  case 223: /* max_value: "rule" UNSIGNED  */
#line 968 "parser.y"
                      {
	#line 8168 "format.w"
	hset_max(rule_kind,(yyvsp[0].u));}
#line 3596 "parser.c"
    break;

  case 224: /* max_value: "image" UNSIGNED  */
#line 971 "parser.y"
                       {
	#line 8169 "format.w"
	hset_max(image_kind,(yyvsp[0].u));}
#line 3604 "parser.c"
    break;

  case 225: /* max_value: "leaders" UNSIGNED  */
#line 974 "parser.y"
                         {
	#line 8170 "format.w"
	hset_max(leaders_kind,(yyvsp[0].u));}
#line 3612 "parser.c"
    break;

  case 226: /* max_value: "baseline" UNSIGNED  */
#line 977 "parser.y"
                          {
	#line 8171 "format.w"
	hset_max(baseline_kind,(yyvsp[0].u));}
#line 3620 "parser.c"
    break;

  case 227: /* max_value: "xdimen" UNSIGNED  */
#line 980 "parser.y"
                        {
	#line 8172 "format.w"
	hset_max(xdimen_kind,(yyvsp[0].u));}
#line 3628 "parser.c"
    break;

  case 228: /* max_value: "param" UNSIGNED  */
#line 983 "parser.y"
                       {
	#line 8173 "format.w"
	hset_max(param_kind,(yyvsp[0].u));}
#line 3636 "parser.c"
    break;

  case 229: /* max_value: "stream (definition)" UNSIGNED  */
#line 986 "parser.y"
                           {
	#line 8174 "format.w"
	hset_max(stream_kind,(yyvsp[0].u));}
#line 3644 "parser.c"
    break;

  case 230: /* max_value: "page" UNSIGNED  */
#line 989 "parser.y"
                      {
	#line 8175 "format.w"
	hset_max(page_kind,(yyvsp[0].u));}
#line 3652 "parser.c"
    break;

  case 231: /* max_value: "range" UNSIGNED  */
#line 992 "parser.y"
                       {
	#line 8176 "format.w"
	hset_max(range_kind,(yyvsp[0].u));}
#line 3660 "parser.c"
    break;

  case 232: /* max_value: "label" UNSIGNED  */
#line 995 "parser.y"
                       {
	#line 8177 "format.w"
	hset_max(label_kind,(yyvsp[0].u));}
#line 3668 "parser.c"
    break;

  case 233: /* def_node: start "font" ref font ">"  */
#line 1001 "parser.y"
                       {
	#line 8377 "format.w"
	DEF((yyval.rf),font_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),(yyvsp[-1].info));}
#line 3676 "parser.c"
    break;

  case 234: /* def_node: start "int" ref integer ">"  */
#line 1004 "parser.y"
                                      {
	#line 8378 "format.w"
	DEF((yyval.rf),int_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3684 "parser.c"
    break;

  case 235: /* def_node: start "dimen" ref dimension ">"  */
#line 1007 "parser.y"
                                      {
	#line 8379 "format.w"
	DEF((yyval.rf),dimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_dimen((yyvsp[-1].d)));}
#line 3692 "parser.c"
    break;

  case 236: /* def_node: start "language" ref string ">"  */
#line 1010 "parser.y"
                                      {
	#line 8380 "format.w"
	DEF((yyval.rf),language_kind,(yyvsp[-2].u));hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3700 "parser.c"
    break;

  case 237: /* def_node: start "glue" ref glue ">"  */
#line 1013 "parser.y"
                                {
	#line 8381 "format.w"
	DEF((yyval.rf),glue_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3708 "parser.c"
    break;

  case 238: /* def_node: start "xdimen" ref xdimen ">"  */
#line 1016 "parser.y"
                                    {
	#line 8382 "format.w"
	DEF((yyval.rf),xdimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 3716 "parser.c"
    break;

  case 239: /* def_node: start "rule" ref rule ">"  */
#line 1019 "parser.y"
                                {
	#line 8383 "format.w"
	DEF((yyval.rf),rule_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3724 "parser.c"
    break;

  case 240: /* def_node: start "leaders" ref leaders ">"  */
#line 1022 "parser.y"
                                      {
	#line 8384 "format.w"
	DEF((yyval.rf),leaders_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 3732 "parser.c"
    break;

  case 241: /* def_node: start "baseline" ref baseline ">"  */
#line 1025 "parser.y"
                                        {
	#line 8385 "format.w"
	DEF((yyval.rf),baseline_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 3740 "parser.c"
    break;

  case 242: /* def_node: start "ligature" ref ligature ">"  */
#line 1028 "parser.y"
                                        {
	#line 8386 "format.w"
	DEF((yyval.rf),ligature_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3748 "parser.c"
    break;

  case 243: /* def_node: start "disc" ref disc ">"  */
#line 1031 "parser.y"
                                {
	#line 8387 "format.w"
	DEF((yyval.rf),disc_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3756 "parser.c"
    break;

  case 244: /* def_node: start "image" ref image ">"  */
#line 1034 "parser.y"
                                  {
	#line 8388 "format.w"
	DEF((yyval.rf),image_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3764 "parser.c"
    break;

  case 245: /* def_node: start "param" ref parameters ">"  */
#line 1037 "parser.y"
                                       {
	#line 8389 "format.w"
	DEF((yyval.rf),param_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_list((yyvsp[-4].u)+2,&((yyvsp[-1].l))));}
#line 3772 "parser.c"
    break;

  case 246: /* def_node: start "page" ref page ">"  */
#line 1040 "parser.y"
                                {
	#line 8390 "format.w"
	DEF((yyval.rf),page_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(page_kind,0));}
#line 3780 "parser.c"
    break;

  case 247: /* def_node: start "int" ref ref ">"  */
#line 1045 "parser.y"
                         {
	#line 8409 "format.w"
	DEF_REF((yyval.rf),int_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(int_kind,0));}
#line 3788 "parser.c"
    break;

  case 248: /* def_node: start "dimen" ref ref ">"  */
#line 1048 "parser.y"
                                {
	#line 8410 "format.w"
	DEF_REF((yyval.rf),dimen_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(dimen_kind,0));}
#line 3796 "parser.c"
    break;

  case 249: /* def_node: start "glue" ref ref ">"  */
#line 1051 "parser.y"
                               {
	#line 8411 "format.w"
	DEF_REF((yyval.rf),glue_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(glue_kind,0));}
#line 3804 "parser.c"
    break;

  case 251: /* def_list: def_list def_node  */
#line 1056 "parser.y"
                          {
	#line 8527 "format.w"
	check_param_def(&((yyvsp[0].rf)));}
#line 3812 "parser.c"
    break;

  case 252: /* parameters: estimate def_list  */
#line 1059 "parser.y"
                            {
	#line 8528 "format.w"
	(yyval.l).p= (yyvsp[0].u);(yyval.l).t= TAG(param_kind,b001);(yyval.l).s= (hpos-hstart)-(yyvsp[0].u);}
#line 3820 "parser.c"
    break;

  case 253: /* named_param_list: start "param" parameters ">"  */
#line 1064 "parser.y"
{
	#line 8541 "format.w"
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyvsp[-1].l))));}
#line 3828 "parser.c"
    break;

  case 255: /* param_list: start parameters ">"  */
#line 1068 "parser.y"
{
	#line 8543 "format.w"
	hput_tags((yyvsp[-2].u),hput_list((yyvsp[-2].u)+1,&((yyvsp[-1].l))));}
#line 3836 "parser.c"
    break;

  case 257: /* font_head: string dimension UNSIGNED UNSIGNED  */
#line 1076 "parser.y"
{
	#line 8689 "format.w"
	uint8_t f= (yyvsp[-4].u);SET_DBIT(f,font_kind);hfont_name[f]= strdup((yyvsp[-3].s));(yyval.info)= hput_font_head(f,hfont_name[f],(yyvsp[-2].d),(yyvsp[-1].u),(yyvsp[0].u));}
#line 3844 "parser.c"
    break;

  case 260: /* font_param: start "penalty" fref penalty ">"  */
#line 1083 "parser.y"
                              {
	#line 8694 "format.w"
	hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3852 "parser.c"
    break;

  case 261: /* font_param: start "kern" fref kern ">"  */
#line 1086 "parser.y"
                                 {
	#line 8695 "format.w"
	hput_tags((yyvsp[-4].u),hput_kern(&((yyvsp[-1].kt))));}
#line 3860 "parser.c"
    break;

  case 262: /* font_param: start "ligature" fref ligature ">"  */
#line 1089 "parser.y"
                                         {
	#line 8696 "format.w"
	hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3868 "parser.c"
    break;

  case 263: /* font_param: start "disc" fref disc ">"  */
#line 1092 "parser.y"
                                 {
	#line 8697 "format.w"
	hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3876 "parser.c"
    break;

  case 264: /* font_param: start "glue" fref glue ">"  */
#line 1095 "parser.y"
                                 {
	#line 8698 "format.w"
	hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3884 "parser.c"
    break;

  case 265: /* font_param: start "language" fref string ">"  */
#line 1098 "parser.y"
                                       {
	#line 8699 "format.w"
	hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3892 "parser.c"
    break;

  case 266: /* font_param: start "rule" fref rule ">"  */
#line 1101 "parser.y"
                                 {
	#line 8700 "format.w"
	hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3900 "parser.c"
    break;

  case 267: /* font_param: start "image" fref image ">"  */
#line 1104 "parser.y"
                                   {
	#line 8701 "format.w"
	hput_tags((yyvsp[-4].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3908 "parser.c"
    break;

  case 268: /* fref: ref  */
#line 1108 "parser.y"
        {
	#line 8703 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,MAX_FONT_PARAMS);}
#line 3916 "parser.c"
    break;

  case 269: /* xdimen_ref: ref  */
#line 1112 "parser.y"
              {
	#line 8780 "format.w"
	REF(xdimen_kind,(yyvsp[0].u));}
#line 3924 "parser.c"
    break;

  case 270: /* param_ref: ref  */
#line 1115 "parser.y"
             {
	#line 8781 "format.w"
	REF(param_kind,(yyvsp[0].u));}
#line 3932 "parser.c"
    break;

  case 271: /* stream_ref: ref  */
#line 1118 "parser.y"
              {
	#line 8782 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3940 "parser.c"
    break;

  case 272: /* content_node: start "penalty" ref ">"  */
#line 1124 "parser.y"
                     {
	#line 8786 "format.w"
	REF(penalty_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(penalty_kind,0));}
#line 3948 "parser.c"
    break;

  case 273: /* content_node: start "kern" explicit ref ">"  */
#line 1128 "parser.y"
{
	#line 8788 "format.w"
	REF(dimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(kern_kind,((yyvsp[-2].b))?b100:b000));}
#line 3956 "parser.c"
    break;

  case 274: /* content_node: start "kern" explicit "xdimen" ref ">"  */
#line 1132 "parser.y"
{
	#line 8790 "format.w"
	REF(xdimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-5].u),TAG(kern_kind,((yyvsp[-3].b))?b101:b001));}
#line 3964 "parser.c"
    break;

  case 275: /* content_node: start "glue" ref ">"  */
#line 1135 "parser.y"
                           {
	#line 8791 "format.w"
	REF(glue_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}
#line 3972 "parser.c"
    break;

  case 276: /* content_node: start "ligature" ref ">"  */
#line 1138 "parser.y"
                               {
	#line 8792 "format.w"
	REF(ligature_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(ligature_kind,0));}
#line 3980 "parser.c"
    break;

  case 277: /* content_node: start "disc" ref ">"  */
#line 1141 "parser.y"
                           {
	#line 8793 "format.w"
	REF(disc_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(disc_kind,0));}
#line 3988 "parser.c"
    break;

  case 278: /* content_node: start "rule" ref ">"  */
#line 1144 "parser.y"
                           {
	#line 8794 "format.w"
	REF(rule_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(rule_kind,0));}
#line 3996 "parser.c"
    break;

  case 279: /* content_node: start "image" ref ">"  */
#line 1147 "parser.y"
                            {
	#line 8795 "format.w"
	REF(image_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(image_kind,0));}
#line 4004 "parser.c"
    break;

  case 280: /* content_node: start "leaders" ref ">"  */
#line 1150 "parser.y"
                              {
	#line 8796 "format.w"
	REF(leaders_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(leaders_kind,0));}
#line 4012 "parser.c"
    break;

  case 281: /* content_node: start "baseline" ref ">"  */
#line 1153 "parser.y"
                               {
	#line 8797 "format.w"
	REF(baseline_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(baseline_kind,0));}
#line 4020 "parser.c"
    break;

  case 282: /* content_node: start "language" REFERENCE ">"  */
#line 1156 "parser.y"
                                     {
	#line 8798 "format.w"
	REF(language_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),hput_language((yyvsp[-1].u)));}
#line 4028 "parser.c"
    break;

  case 283: /* glue_node: start "glue" ref ">"  */
#line 1160 "parser.y"
                            {
	#line 8800 "format.w"
	REF(glue_kind,(yyvsp[-1].u));
	if((yyvsp[-1].u)==zero_skip_no){hpos= hpos-2;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),TAG(glue_kind,0));(yyval.b)= true;}}
#line 4038 "parser.c"
    break;

  case 284: /* $@13: %empty  */
#line 1167 "parser.y"
                             {
	#line 9247 "format.w"
	hput_content_start();}
#line 4046 "parser.c"
    break;

  case 285: /* content_section: "<" "content" $@13 content_list ">"  */
#line 1170 "parser.y"
{
	#line 9248 "format.w"
	hput_content_end();hput_range_defs();hput_label_defs();}
#line 4054 "parser.c"
    break;


#line 4058 "parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1174 "parser.y"

	/*:534*/
