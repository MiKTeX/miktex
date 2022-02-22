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

	#line 10316 "format.w"
	
#include "hibasetypes.h"
#include <string.h>
#include <math.h>
#include "hierror.h"
#include "hiformat.h"
#include "hiput.h"
extern char**hfont_name;

	/*358:*/
uint32_t definition_bits[0x100/32][32]= {
	#line 7628 "format.w"
	{0}};

#define SET_DBIT(N,K) ((N)>0xFF?1:(definition_bits[N/32][K]	|= (1<<((N)&(32-1)))))
#define GET_DBIT(N,K) ((N)>0xFF?1:((definition_bits[N/32][K]>>((N)&(32-1)))&1))
#define DEF(D,K,N) (D).k= K; (D).n= (N);SET_DBIT((D).n,(D).k);\
 DBG(DBGDEF,"Defining %s %d\n",definition_name[(D).k],(D).n);\
 RNG("Definition",(D).n,max_fixed[(D).k]+1,max_ref[(D).k]);
#define REF(K,N) REF_RNG(K,N);if(!GET_DBIT(N,K)) \
 QUIT("Reference %d to %s before definition",(N),definition_name[K])
	/*:358*/	/*362:*/
#define DEF_REF(D,K,M,N)  DEF(D,K,M);\
if ((int)(M)>max_default[K]) QUIT("Defining non default reference %d for %s",M,definition_name[K]); \
if ((int)(N)>max_fixed[K]) QUIT("Defining reference %d for %s by non fixed reference %d",M,definition_name[K],N);
	/*:362*/

extern void hset_entry(Entry*e,uint16_t i,uint32_t size,
uint32_t xsize,char*file_name);

	/*425:*/
#ifdef DEBUG
#define  YYDEBUG 1
extern int yydebug;
#else
#define YYDEBUG 0
#endif
	/*:425*/
extern int yylex(void);

	/*354:*/
void hset_max(Kind k,int n)
{
	#line 7471 "format.w"
	DBG(DBGDEF,"Setting max %s to %d\n",definition_name[k],n);
	RNG("Maximum",n,max_fixed[k]+1,MAX_REF(k));
	if(n>max_ref[k])
	max_ref[k]= n;
	}
	/*:354*/	/*365:*/
void check_param_def(Ref*df)
{
	#line 7777 "format.w"
	if(df->k!=int_kind&&df->k!=dimen_kind&&df->k!=glue_kind)
	QUIT("Kind %s not allowed in parameter list",definition_name[df->k]);
	if(df->n<=max_fixed[df->k]||max_default[df->k]<df->n)
	QUIT("Parameter %d for %s not allowed in parameter list",df->n,definition_name[df->k]);
	}
	/*:365*/	/*424:*/
extern int yylineno;
int yyerror(const char*msg)
{
	#line 8902 "format.w"
	QUIT(" in line %d %s",yylineno,msg);
	return 0;
	}
	/*:424*/



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
  YYSYMBOL_LABEL = 66,                     /* "label"  */
  YYSYMBOL_BOT = 67,                       /* "bot"  */
  YYSYMBOL_MID = 68,                       /* "mid"  */
  YYSYMBOL_LINK = 69,                      /* "link"  */
  YYSYMBOL_OUTLINE = 70,                   /* "outline"  */
  YYSYMBOL_STREAM = 71,                    /* "stream"  */
  YYSYMBOL_STREAMDEF = 72,                 /* "stream (definition)"  */
  YYSYMBOL_FIRST = 73,                     /* "first"  */
  YYSYMBOL_LAST = 74,                      /* "last"  */
  YYSYMBOL_TOP = 75,                       /* "top"  */
  YYSYMBOL_NOREFERENCE = 76,               /* "*"  */
  YYSYMBOL_PAGE = 77,                      /* "page"  */
  YYSYMBOL_RANGE = 78,                     /* "range"  */
  YYSYMBOL_DIRECTORY = 79,                 /* "directory"  */
  YYSYMBOL_SECTION = 80,                   /* "entry"  */
  YYSYMBOL_DEFINITIONS = 81,               /* "definitions"  */
  YYSYMBOL_MAX = 82,                       /* "max"  */
  YYSYMBOL_PARAM = 83,                     /* "param"  */
  YYSYMBOL_FONT = 84,                      /* "font"  */
  YYSYMBOL_CONTENT = 85,                   /* "content"  */
  YYSYMBOL_YYACCEPT = 86,                  /* $accept  */
  YYSYMBOL_glyph = 87,                     /* glyph  */
  YYSYMBOL_content_node = 88,              /* content_node  */
  YYSYMBOL_start = 89,                     /* start  */
  YYSYMBOL_integer = 90,                   /* integer  */
  YYSYMBOL_string = 91,                    /* string  */
  YYSYMBOL_number = 92,                    /* number  */
  YYSYMBOL_dimension = 93,                 /* dimension  */
  YYSYMBOL_xdimen = 94,                    /* xdimen  */
  YYSYMBOL_xdimen_node = 95,               /* xdimen_node  */
  YYSYMBOL_order = 96,                     /* order  */
  YYSYMBOL_stretch = 97,                   /* stretch  */
  YYSYMBOL_penalty = 98,                   /* penalty  */
  YYSYMBOL_rule_dimension = 99,            /* rule_dimension  */
  YYSYMBOL_rule = 100,                     /* rule  */
  YYSYMBOL_rule_node = 101,                /* rule_node  */
  YYSYMBOL_explicit = 102,                 /* explicit  */
  YYSYMBOL_kern = 103,                     /* kern  */
  YYSYMBOL_plus = 104,                     /* plus  */
  YYSYMBOL_minus = 105,                    /* minus  */
  YYSYMBOL_glue = 106,                     /* glue  */
  YYSYMBOL_glue_node = 107,                /* glue_node  */
  YYSYMBOL_position = 108,                 /* position  */
  YYSYMBOL_content_list = 109,             /* content_list  */
  YYSYMBOL_estimate = 110,                 /* estimate  */
  YYSYMBOL_list = 111,                     /* list  */
  YYSYMBOL_112_1 = 112,                    /* $@1  */
  YYSYMBOL_text = 113,                     /* text  */
  YYSYMBOL_txt = 114,                      /* txt  */
  YYSYMBOL_115_2 = 115,                    /* $@2  */
  YYSYMBOL_box_dimen = 116,                /* box_dimen  */
  YYSYMBOL_box_shift = 117,                /* box_shift  */
  YYSYMBOL_box_glue_set = 118,             /* box_glue_set  */
  YYSYMBOL_box = 119,                      /* box  */
  YYSYMBOL_hbox_node = 120,                /* hbox_node  */
  YYSYMBOL_vbox_node = 121,                /* vbox_node  */
  YYSYMBOL_box_flex = 122,                 /* box_flex  */
  YYSYMBOL_xbox = 123,                     /* xbox  */
  YYSYMBOL_box_goal = 124,                 /* box_goal  */
  YYSYMBOL_hpack = 125,                    /* hpack  */
  YYSYMBOL_vpack = 126,                    /* vpack  */
  YYSYMBOL_127_3 = 127,                    /* $@3  */
  YYSYMBOL_vxbox_node = 128,               /* vxbox_node  */
  YYSYMBOL_hxbox_node = 129,               /* hxbox_node  */
  YYSYMBOL_ltype = 130,                    /* ltype  */
  YYSYMBOL_leaders = 131,                  /* leaders  */
  YYSYMBOL_baseline = 132,                 /* baseline  */
  YYSYMBOL_133_4 = 133,                    /* $@4  */
  YYSYMBOL_cc_list = 134,                  /* cc_list  */
  YYSYMBOL_lig_cc = 135,                   /* lig_cc  */
  YYSYMBOL_ref = 136,                      /* ref  */
  YYSYMBOL_ligature = 137,                 /* ligature  */
  YYSYMBOL_138_5 = 138,                    /* $@5  */
  YYSYMBOL_replace_count = 139,            /* replace_count  */
  YYSYMBOL_disc = 140,                     /* disc  */
  YYSYMBOL_disc_node = 141,                /* disc_node  */
  YYSYMBOL_par_dimen = 142,                /* par_dimen  */
  YYSYMBOL_par = 143,                      /* par  */
  YYSYMBOL_144_6 = 144,                    /* $@6  */
  YYSYMBOL_math = 145,                     /* math  */
  YYSYMBOL_on_off = 146,                   /* on_off  */
  YYSYMBOL_span_count = 147,               /* span_count  */
  YYSYMBOL_table = 148,                    /* table  */
  YYSYMBOL_image_dimen = 149,              /* image_dimen  */
  YYSYMBOL_image = 150,                    /* image  */
  YYSYMBOL_max_value = 151,                /* max_value  */
  YYSYMBOL_placement = 152,                /* placement  */
  YYSYMBOL_def_node = 153,                 /* def_node  */
  YYSYMBOL_stream_link = 154,              /* stream_link  */
  YYSYMBOL_stream_split = 155,             /* stream_split  */
  YYSYMBOL_stream_info = 156,              /* stream_info  */
  YYSYMBOL_157_7 = 157,                    /* $@7  */
  YYSYMBOL_stream_type = 158,              /* stream_type  */
  YYSYMBOL_stream_def_node = 159,          /* stream_def_node  */
  YYSYMBOL_stream_ins_node = 160,          /* stream_ins_node  */
  YYSYMBOL_stream = 161,                   /* stream  */
  YYSYMBOL_page_priority = 162,            /* page_priority  */
  YYSYMBOL_stream_def_list = 163,          /* stream_def_list  */
  YYSYMBOL_page = 164,                     /* page  */
  YYSYMBOL_165_8 = 165,                    /* $@8  */
  YYSYMBOL_166_9 = 166,                    /* $@9  */
  YYSYMBOL_hint = 167,                     /* hint  */
  YYSYMBOL_directory_section = 168,        /* directory_section  */
  YYSYMBOL_169_10 = 169,                   /* $@10  */
  YYSYMBOL_entry_list = 170,               /* entry_list  */
  YYSYMBOL_entry = 171,                    /* entry  */
  YYSYMBOL_definition_section = 172,       /* definition_section  */
  YYSYMBOL_173_11 = 173,                   /* $@11  */
  YYSYMBOL_definition_list = 174,          /* definition_list  */
  YYSYMBOL_max_definitions = 175,          /* max_definitions  */
  YYSYMBOL_max_list = 176,                 /* max_list  */
  YYSYMBOL_def_list = 177,                 /* def_list  */
  YYSYMBOL_parameters = 178,               /* parameters  */
  YYSYMBOL_empty_param_list = 179,         /* empty_param_list  */
  YYSYMBOL_non_empty_param_list = 180,     /* non_empty_param_list  */
  YYSYMBOL_181_12 = 181,                   /* $@12  */
  YYSYMBOL_font = 182,                     /* font  */
  YYSYMBOL_font_head = 183,                /* font_head  */
  YYSYMBOL_font_param_list = 184,          /* font_param_list  */
  YYSYMBOL_font_param = 185,               /* font_param  */
  YYSYMBOL_fref = 186,                     /* fref  */
  YYSYMBOL_xdimen_ref = 187,               /* xdimen_ref  */
  YYSYMBOL_param_ref = 188,                /* param_ref  */
  YYSYMBOL_stream_ref = 189,               /* stream_ref  */
  YYSYMBOL_content_section = 190,          /* content_section  */
  YYSYMBOL_191_13 = 191                    /* $@13  */
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
#define YYLAST   657

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  106
/* YYNRULES -- Number of rules.  */
#define YYNRULES  266
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  566

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   340


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
      85
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   264,   264,   267,   270,   274,   274,   278,   282,   282,
     288,   290,   292,   294,   297,   300,   304,   307,   310,   313,
     319,   324,   326,   328,   330,   334,   338,   341,   345,   345,
     348,   354,   357,   359,   361,   364,   367,   371,   373,   376,
     378,   381,   384,   388,   394,   397,   398,   399,   402,   405,
     412,   411,   420,   420,   422,   425,   428,   431,   434,   437,
     440,   443,   443,   448,   452,   455,   459,   462,   465,   470,
     474,   477,   480,   480,   482,   485,   488,   492,   495,   498,
     501,   505,   508,   508,   514,   517,   522,   525,   529,   529,
     531,   533,   535,   537,   540,   543,   546,   549,   553,   553,
     562,   567,   567,   570,   573,   576,   579,   579,   586,   590,
     593,   597,   601,   604,   609,   614,   616,   619,   622,   625,
     628,   628,   633,   636,   640,   644,   647,   650,   653,   656,
     659,   662,   665,   668,   672,   676,   678,   681,   685,   689,
     692,   695,   698,   702,   705,   709,   713,   715,   718,   721,
     725,   732,   734,   736,   738,   741,   746,   751,   761,   763,
     766,   769,   769,   773,   775,   777,   779,   783,   789,   794,
     794,   796,   799,   802,   805,   810,   813,   817,   817,   819,
     821,   819,   828,   831,   835,   837,   837,   840,   840,   841,
     846,   846,   853,   853,   855,   880,   880,   882,   885,   888,
     891,   894,   897,   900,   903,   906,   909,   912,   915,   918,
     921,   924,   927,   930,   936,   939,   942,   945,   948,   951,
     954,   957,   960,   963,   966,   969,   972,   975,   980,   983,
     986,   990,   991,   994,   998,  1001,  1001,  1009,  1011,  1016,
    1016,  1019,  1022,  1025,  1028,  1031,  1034,  1037,  1040,  1044,
    1048,  1051,  1054,  1060,  1063,  1067,  1071,  1074,  1077,  1080,
    1083,  1086,  1089,  1092,  1096,  1103,  1103
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
  "\"item\"", "\"image\"", "\"label\"", "\"bot\"", "\"mid\"", "\"link\"",
  "\"outline\"", "\"stream\"", "\"stream (definition)\"", "\"first\"",
  "\"last\"", "\"top\"", "\"*\"", "\"page\"", "\"range\"", "\"directory\"",
  "\"entry\"", "\"definitions\"", "\"max\"", "\"param\"", "\"font\"",
  "\"content\"", "$accept", "glyph", "content_node", "start", "integer",
  "string", "number", "dimension", "xdimen", "xdimen_node", "order",
  "stretch", "penalty", "rule_dimension", "rule", "rule_node", "explicit",
  "kern", "plus", "minus", "glue", "glue_node", "position", "content_list",
  "estimate", "list", "$@1", "text", "txt", "$@2", "box_dimen",
  "box_shift", "box_glue_set", "box", "hbox_node", "vbox_node", "box_flex",
  "xbox", "box_goal", "hpack", "vpack", "$@3", "vxbox_node", "hxbox_node",
  "ltype", "leaders", "baseline", "$@4", "cc_list", "lig_cc", "ref",
  "ligature", "$@5", "replace_count", "disc", "disc_node", "par_dimen",
  "par", "$@6", "math", "on_off", "span_count", "table", "image_dimen",
  "image", "max_value", "placement", "def_node", "stream_link",
  "stream_split", "stream_info", "$@7", "stream_type", "stream_def_node",
  "stream_ins_node", "stream", "page_priority", "stream_def_list", "page",
  "$@8", "$@9", "hint", "directory_section", "$@10", "entry_list", "entry",
  "definition_section", "$@11", "definition_list", "max_definitions",
  "max_list", "def_list", "parameters", "empty_param_list",
  "non_empty_param_list", "$@12", "font", "font_head", "font_param_list",
  "font_param", "fref", "xdimen_ref", "param_ref", "stream_ref",
  "content_section", "$@13", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-326)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      48,    -4,   110,   113,   107,  -326,    52,   137,  -326,  -326,
      74,  -326,  -326,   153,  -326,   211,    85,  -326,  -326,    94,
    -326,  -326,  -326,   275,  -326,   278,   190,   280,   159,  -326,
     313,  -326,    31,  -326,  -326,   585,  -326,  -326,  -326,  -326,
    -326,  -326,  -326,  -326,   223,   467,  -326,   203,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   240,   248,   116,   219,   253,   140,   210,   229,
     182,   182,   220,   182,   220,   182,   121,   229,   234,    71,
     229,    41,    66,   271,    79,   301,   261,   234,   234,  -326,
    -326,   268,   270,   279,   284,   286,   288,   294,   297,   315,
     325,   326,   327,   328,   333,   334,   335,   337,   338,   339,
     343,   242,  -326,   229,   182,   219,   223,   152,   229,   345,
     182,   234,   210,   348,   223,   350,   223,    82,   251,   323,
     342,   353,  -326,  -326,  -326,   355,   357,   358,  -326,  -326,
    -326,  -326,   231,  -326,   152,   359,   361,  -326,   165,   362,
     182,   341,   372,   373,   182,   220,   375,   377,   182,   264,
     379,   220,   380,   305,   384,   385,  -326,   363,   200,   387,
     389,  -326,   390,   391,   395,   396,   398,   397,    66,   401,
     234,  -326,  -326,   404,   234,  -326,  -326,  -326,  -326,   405,
    -326,    66,    66,  -326,   350,   406,   264,   264,   407,  -326,
     408,   530,   409,   399,   182,   410,   411,   255,  -326,   234,
      80,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,  -326,   413,   414,   416,   417,   418,   419,   421,   422,
     423,   427,   429,  -326,   430,   432,   433,  -326,   435,  -326,
    -326,   436,   182,   437,   345,  -326,  -326,  -326,   438,   440,
     441,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
     152,  -326,  -326,   234,  -326,   442,  -326,   302,   182,   420,
    -326,  -326,   182,   293,  -326,  -326,  -326,   121,   121,    66,
    -326,   341,  -326,   400,  -326,  -326,   229,  -326,  -326,  -326,
     345,  -326,  -326,   345,  -326,  -326,  -326,   176,  -326,  -326,
    -326,    66,  -326,  -326,    66,  -326,    66,    66,  -326,    38,
     345,    66,    66,    53,   345,    66,  -326,  -326,  -326,    66,
      66,  -326,  -326,  -326,   445,   182,   341,  -326,  -326,   446,
     448,    66,    66,  -326,  -326,  -326,  -326,   439,   450,  -326,
      66,    66,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,  -326,  -326,  -326,  -326,  -326,   452,  -326,  -326,   451,
    -326,   453,  -326,   345,   345,  -326,  -326,  -326,  -326,   456,
    -326,   182,  -326,   164,  -326,   182,  -326,  -326,   182,   182,
      66,  -326,  -326,  -326,  -326,  -326,   420,   121,   182,   458,
     460,    49,  -326,  -326,  -326,   345,  -326,  -326,   434,  -326,
      66,    35,  -326,    66,  -326,    66,  -326,  -326,   424,  -326,
    -326,   345,    66,  -326,  -326,  -326,   324,    66,    66,  -326,
    -326,   420,  -326,  -326,  -326,    66,  -326,   182,  -326,   345,
     463,  -326,   345,  -326,   462,   412,  -326,   105,  -326,  -326,
     454,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,    66,    66,  -326,  -326,  -326,   152,  -326,  -326,  -326,
    -326,  -326,   350,  -326,  -326,  -326,   184,  -326,  -326,  -326,
    -326,  -326,   466,    39,   345,  -326,   182,  -326,   210,   234,
     234,   234,   234,   234,   234,   234,   234,  -326,  -326,  -326,
     264,    72,   471,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,  -326,   399,  -326,  -326,  -326,    39,  -326,    66,  -326,
    -326,   242,   223,   152,   210,   182,   234,   210,   348,    66,
    -326,  -326,  -326,  -326,   470,   345,   345,   473,   476,   478,
     182,   480,   481,   482,   483,   484,  -326,  -326,   489,   345,
    -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,    66,
    -326,   345,   425,  -326,   234,    69
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     0,     0,     0,     0,     1,     0,     0,   185,   190,
       0,   184,   187,     0,   265,     0,     0,   192,    44,     0,
     186,   188,   195,     0,    45,     0,     0,     0,     4,   191,
       0,   193,     4,   266,    46,     0,    32,    72,    73,    88,
      89,   115,   169,   170,     0,     0,   194,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,    64,     0,    64,     0,     0,     0,     0,    33,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     8,
       9,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,     0,    47,     0,   154,     0,     0,
       0,     0,     6,     5,    26,     0,     0,     0,    10,    11,
      12,    29,     0,    28,     0,     0,     0,    34,     0,     0,
      19,    37,     0,     0,     0,    64,     0,     0,     0,     0,
       0,    64,     0,     0,     0,     0,     4,     0,    90,     0,
       0,    98,     0,     0,   106,     0,   109,     0,   113,     0,
     116,   250,    44,     0,    44,   135,   136,   234,   251,     0,
     137,     0,     0,    44,    47,     0,     0,     0,     0,   139,
       0,    47,     0,     0,   147,     0,     0,     0,   252,    44,
       0,   189,   199,   208,   198,   203,   204,   202,   206,   207,
     200,   201,   205,   213,   150,   210,   211,   212,   209,   197,
     196,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   106,     0,     0,     0,   179,     0,    48,
      44,     0,     0,     0,     0,   152,   153,   151,     0,     0,
       0,     2,     7,     3,    27,   253,   263,    13,    15,    14,
       0,    31,   259,     0,    35,     0,    36,     0,     0,    39,
      42,   256,     0,    66,    70,    71,    65,     0,     0,     0,
      87,    37,    86,     0,    85,    84,     0,    91,    92,    93,
       0,    97,   261,     0,   100,   262,   257,     0,   108,   110,
     258,   112,   114,   120,     0,   124,     0,     0,   134,    47,
     128,     0,     0,    47,   125,     0,    50,    44,   138,     0,
       0,   145,   140,   142,     0,     0,    37,   260,   149,     0,
       0,     0,     0,   168,   164,   165,   166,     0,     0,   163,
       0,     0,   216,   229,   219,   215,   228,   217,   220,   218,
     230,   221,   222,   223,   224,   225,   175,   227,   231,   233,
     226,     0,   214,     0,   237,   155,   182,   183,    30,     0,
     254,    17,    18,     0,    38,     0,    41,    63,     0,     0,
       0,    80,    78,    79,    77,    81,    39,     0,     0,     0,
       0,     0,    94,    95,    96,     0,   103,   104,     0,   111,
       0,    47,   123,     0,   119,     0,   117,   235,     0,   129,
     130,   131,     0,   126,   127,    44,     0,     0,     0,   141,
     146,    39,   156,   174,   171,     0,   173,     0,   161,     0,
       0,   176,     0,   232,     0,     0,   239,     0,   240,   255,
       0,    21,    22,    23,    24,    25,    40,    67,    68,    69,
      74,     0,     0,    82,    43,   264,     0,    99,   101,   121,
     122,   118,    47,   132,   133,    52,    61,    49,   143,   144,
     148,   172,     0,     0,     0,   157,     0,   238,    33,     0,
       0,     0,     0,     0,     0,     0,     0,    16,    76,    75,
       0,     0,     0,    51,    60,    58,    59,    55,    57,    56,
      54,    53,     0,    20,   159,   158,     0,   162,     0,   180,
     249,     0,     0,     0,    33,     0,     0,    33,     0,     0,
     107,   102,   236,    62,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,   160,     0,     0,
     241,   246,   247,   242,   245,   243,   244,   248,   167,     0,
     177,   181,     0,   178,     0,     0
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -326,  -326,   -65,   -23,    97,   -79,   -83,   -13,   -62,  -232,
    -326,  -191,   -27,  -118,   -94,   195,   -67,   -26,  -228,  -325,
     -97,  -226,    -1,   172,   -86,   -55,  -326,  -326,  -326,  -326,
     198,   -36,  -326,   431,  -111,   201,  -326,   428,  -147,  -326,
    -326,  -326,  -326,  -326,  -326,   381,   386,  -326,  -326,  -326,
     -45,   -96,  -326,  -326,   -95,   132,  -326,  -326,  -326,  -326,
     300,  -326,  -326,  -326,   -93,  -326,  -326,   139,    -7,  -326,
    -326,  -326,  -326,   -50,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,  -326,
    -326,  -326,    42,   -89,  -227,  -326,  -326,  -326,  -326,  -326,
    -290,  -222,   -92,  -326,  -326,  -326
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   131,    34,   194,   134,    91,   142,   150,   151,   348,
     455,   384,   135,   144,   145,    36,   176,   149,   279,   386,
     152,   168,   187,    25,   327,   195,   425,   476,   511,   512,
     155,   159,   390,   156,    37,    38,   397,   162,   289,   160,
     164,   500,    39,    40,   300,   169,   172,   303,   501,   408,
     520,   175,   307,   178,   179,    41,   182,   183,   410,   189,
     190,   203,   198,   336,   206,   110,   258,    31,   516,   517,
     349,   483,   350,    42,    43,   340,   442,   561,   248,   366,
     536,     2,     3,    12,    15,    21,     7,    13,    23,    17,
      27,   369,   251,   191,   322,   472,   253,   254,   374,   448,
     521,   184,   192,   209,    11,    18
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,   148,    35,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    24,   180,   200,
     136,   239,   146,   238,   153,   244,   270,   245,   373,   202,
     246,   170,   173,   174,   177,   181,   188,   237,   163,   250,
     205,   249,   208,   210,   249,   247,   112,   252,   112,   329,
     330,     1,   234,   167,   143,   391,   393,   154,   154,   249,
     154,   201,   154,   396,   171,   392,   394,   277,   233,   166,
     236,   460,   166,   240,   466,     4,   243,   405,   112,    70,
     321,   325,    32,   166,   343,   199,   274,   413,   313,   415,
      70,    71,   317,   314,    70,   316,   167,    62,   193,   147,
     232,   185,   186,   275,   143,   530,   480,   171,   431,    63,
       5,   193,   531,     8,   435,   514,     6,   342,   417,   283,
     341,   417,   129,   311,   166,   291,   130,   489,   112,   490,
     491,   143,   492,     9,   493,   188,   320,   324,   334,   188,
      10,   282,   344,   345,   346,   286,   138,   112,   139,   255,
     256,   140,   378,   344,   345,   346,    16,   257,   138,    14,
     139,   494,   495,   140,   188,   461,   141,    22,   319,   323,
     496,   138,   112,   139,    26,   462,   140,   451,   141,   467,
      35,   273,   406,   452,   453,   454,   407,   347,   138,   403,
     139,   335,   326,   140,   456,   383,    44,   457,   458,   399,
     522,   523,   524,   525,   526,   527,   528,   484,   231,   419,
     111,   422,   235,   423,    19,    20,   486,   503,   504,   505,
     506,   507,   508,   509,   510,   132,   112,   133,   379,    47,
     351,   167,    89,    90,   395,   138,   112,   139,   147,   371,
     140,   112,   181,   181,   267,   268,   269,   127,   132,   368,
     133,   400,   297,   298,   299,   128,   409,   143,   518,   412,
     137,   414,   416,   158,   347,   347,   420,   421,   207,   387,
     424,   161,   211,   161,   427,   428,   212,   401,    28,    29,
     167,    32,    33,    45,    46,   213,   434,   436,   196,   197,
     214,   411,   215,   411,   216,   439,   440,   418,   450,   323,
     217,   418,   383,   218,   549,   383,   383,   204,   112,   548,
     473,   259,   260,   287,   288,   185,   186,   559,   411,   381,
     382,   219,   430,   388,   389,    48,    24,    32,   477,    49,
     261,   220,   221,   222,   223,   459,    50,    51,    52,   224,
     225,   226,    53,   227,   228,   229,    30,   230,   166,   262,
     445,   447,   181,   529,   204,   469,   249,   263,   470,   264,
     471,   265,   266,   271,    54,   272,   276,   474,    55,    56,
      57,   278,   478,   479,   347,   482,   280,   281,    58,   284,
     481,   285,   167,   290,   292,   463,   250,   293,   294,   295,
      59,   301,   296,   302,   304,   305,    60,    61,   418,   306,
     308,   310,    32,    35,   309,   312,   498,   499,   315,   318,
     328,   331,   332,   333,   337,   338,   347,   352,   353,   167,
     354,   355,   356,   357,   475,   358,   359,   360,   542,   539,
     543,   361,   544,   362,   363,   545,   364,   365,   515,   367,
     370,   372,   375,   538,   376,   377,   380,   533,   398,   429,
     432,   385,   433,   143,    28,   437,   438,   540,   441,   444,
     449,   167,   464,   535,   465,    70,   468,   485,   487,   488,
     513,   515,   497,   519,   546,   532,   547,   550,   274,    92,
     551,   243,   552,    93,   553,   554,   555,   556,   557,    35,
      94,    95,    96,   558,   537,   402,    97,   564,   541,   426,
     241,   404,   157,   165,   560,   446,   242,   339,   443,   534,
     143,   563,   167,   347,   502,     0,     0,     0,    98,   565,
       0,     0,    99,   100,   101,     0,   347,     0,     0,     0,
       0,     0,   102,   103,     0,    64,   249,   104,   562,   105,
       0,     0,   347,     0,   106,   107,     0,     0,     0,     0,
     108,   109,    65,     0,    66,    67,     0,    68,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,     0,     0,
       0,    76,     0,     0,     0,    77,    78,    79,    80,    81,
      64,     0,    82,    83,    84,    85,     0,     0,     0,    86,
       0,    87,    88,     0,     0,     0,     0,    65,     0,    66,
      67,     0,    68,     0,    69,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,     0,     0,     0,    76,     0,     0,     0,
      77,    78,    79,    80,    81,     0,     0,    82,    83,    84,
      85,     0,     0,     0,    86,     0,    87,    88
};

static const yytype_int16 yycheck[] =
{
      23,    68,    25,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    18,    80,    84,
      65,   118,    67,   117,    69,   121,   144,   122,   254,    84,
     123,    76,    77,    78,    79,    80,    81,   116,    74,   125,
      85,     6,    87,    88,     6,   124,     7,   126,     7,   196,
     197,     3,   114,    76,    67,   287,   288,    70,    71,     6,
      73,    84,    75,   291,    77,   287,   288,   150,   113,     3,
     115,   396,     3,   118,    25,    79,   121,   303,     7,    41,
     191,   192,     3,     3,     4,     6,   148,   314,   180,   316,
      41,    42,   184,   182,    41,   184,   119,    66,    32,    28,
     113,    60,    61,   148,   117,    33,   431,   120,   336,    78,
       0,    32,    40,     6,   341,    76,     3,   209,    83,   155,
     209,    83,     6,   178,     3,   161,    10,    22,     7,    24,
      25,   144,    27,    81,    29,   180,   191,   192,   203,   184,
       3,   154,    73,    74,    75,   158,     6,     7,     8,    67,
      68,    11,   270,    73,    74,    75,     3,    75,     6,    85,
       8,    56,    57,    11,   209,   397,    26,    82,   191,   192,
      65,     6,     7,     8,    80,   397,    11,    13,    26,   405,
     203,    16,     6,    19,    20,    21,    10,   210,     6,   300,
       8,   204,   193,    11,   385,   278,     6,   388,   389,   296,
     490,   491,   492,   493,   494,   495,   496,   439,   111,   320,
       7,   322,   115,   324,     3,     4,   442,    33,    34,    35,
      36,    37,    38,    39,    40,     6,     7,     8,   273,    70,
     231,   254,     9,    10,   289,     6,     7,     8,    28,   252,
      11,     7,   287,   288,    13,    14,    15,     7,     6,   250,
       8,   296,    52,    53,    54,     7,   311,   270,   484,   314,
       7,   316,   317,    43,   287,   288,   321,   322,     7,   282,
     325,    73,     4,    75,   329,   330,     6,   300,     3,     4,
     303,     3,     4,     3,     4,     6,   341,   342,    17,    18,
       6,   314,     6,   316,     6,   350,   351,   320,   381,   322,
       6,   324,   385,     6,   536,   388,   389,     6,     7,   535,
     421,    60,    61,    49,    50,    60,    61,   549,   341,    17,
      18,     6,   335,    30,    31,    12,   327,     3,     4,    16,
       7,     6,     6,     6,     6,   390,    23,    24,    25,     6,
       6,     6,    29,     6,     6,     6,   369,     4,     3,     7,
     373,   374,   397,   500,     6,   410,     6,     4,   413,     4,
     415,     4,     4,     4,    51,     4,     4,   422,    55,    56,
      57,    30,   427,   428,   397,   437,     4,     4,    65,     4,
     435,     4,   405,     4,     4,   398,   472,    82,     4,     4,
      77,     4,    29,     4,     4,     4,    83,    84,   421,     4,
       4,     4,     3,   426,     6,     4,   461,   462,     4,     4,
       4,     4,     4,     4,     4,     4,   439,     4,     4,   442,
       4,     4,     4,     4,   425,     4,     4,     4,   525,   523,
     526,     4,   527,     4,     4,   528,     4,     4,   483,     4,
       4,     4,     4,   522,     4,     4,     4,   512,    48,     4,
       4,    31,     4,   466,     3,    16,     6,   524,     6,     6,
       4,   484,     4,   518,     4,    41,    32,     4,     6,    57,
       4,   516,    18,   486,   529,     4,     6,     4,   540,    12,
       4,   526,     4,    16,     4,     4,     4,     4,     4,   512,
      23,    24,    25,     4,   521,   300,    29,    72,   524,   327,
     119,   300,    71,    75,   559,   373,   120,   207,   369,   516,
     523,   561,   535,   536,   472,    -1,    -1,    -1,    51,   564,
      -1,    -1,    55,    56,    57,    -1,   549,    -1,    -1,    -1,
      -1,    -1,    65,    66,    -1,     5,     6,    70,   561,    72,
      -1,    -1,   565,    -1,    77,    78,    -1,    -1,    -1,    -1,
      83,    84,    22,    -1,    24,    25,    -1,    27,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    -1,    44,    45,    46,    47,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
       5,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      -1,    71,    72,    -1,    -1,    -1,    -1,    22,    -1,    24,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    -1,    44,
      45,    46,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    -1,    71,    72
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   167,   168,    79,     0,     3,   172,     6,    81,
       3,   190,   169,   173,    85,   170,     3,   175,   191,     3,
       4,   171,    82,   174,   108,   109,    80,   176,     3,     4,
      89,   153,     3,     4,    88,    89,   101,   120,   121,   128,
     129,   141,   159,   160,     6,     3,     4,    70,    12,    16,
      23,    24,    25,    29,    51,    55,    56,    57,    65,    77,
      83,    84,    66,    78,     5,    22,    24,    25,    27,    29,
      41,    42,    44,    45,    46,    47,    51,    55,    56,    57,
      58,    59,    62,    63,    64,    65,    69,    71,    72,     9,
      10,    91,    12,    16,    23,    24,    25,    29,    51,    55,
      56,    57,    65,    66,    70,    72,    77,    78,    83,    84,
     151,     7,     7,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,     7,     7,     6,
      10,    87,     6,     8,    90,    98,   136,     7,     6,     8,
      11,    26,    92,    93,    99,   100,   136,    28,   102,   103,
      93,    94,   106,   136,    93,   116,   119,   119,    43,   117,
     125,   116,   123,   117,   126,   123,     3,    89,   107,   131,
     136,    93,   132,   136,   136,   137,   102,   136,   139,   140,
      94,   136,   142,   143,   187,    60,    61,   108,   136,   145,
     146,   179,   188,    32,    89,   111,    17,    18,   148,     6,
      88,    89,   111,   147,     6,   136,   150,     7,   136,   189,
     136,     4,     6,     6,     6,     6,     6,     6,     6,     6,
       6,     6,     6,     6,     6,     6,     6,     6,     6,     6,
       4,    90,    93,   136,    94,    90,   136,    91,   100,   106,
     136,   131,   132,   136,   137,   140,   150,    91,   164,     6,
     110,   178,    91,   182,   183,    67,    68,    75,   152,    60,
      61,     7,     7,     4,     4,     4,     4,    13,    14,    15,
      99,     4,     4,    16,    94,   136,     4,    92,    30,   104,
       4,     4,    93,   117,     4,     4,    93,    49,    50,   124,
       4,   117,     4,    82,     4,     4,    29,    52,    53,    54,
     130,     4,     4,   133,     4,     4,     4,   138,     4,     6,
       4,   111,     4,   188,   179,     4,   179,   188,     4,    89,
     111,   120,   180,    89,   111,   120,   108,   110,     4,   124,
     124,     4,     4,     4,    88,    93,   149,     4,     4,   146,
     161,   179,   188,     4,    73,    74,    75,    89,    95,   156,
     158,   108,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,   165,     4,   108,   177,
       4,    93,     4,   107,   184,     4,     4,     4,    99,   136,
       4,    17,    18,    92,    97,    31,   105,    93,    30,    31,
     118,    95,   187,    95,   187,   111,   104,   122,    48,   106,
     136,    89,   101,   120,   121,   107,     6,    10,   135,   111,
     144,    89,   111,   180,   111,   180,   111,    83,    89,   120,
     111,   111,   120,   120,   111,   112,   109,   111,   111,     4,
      93,   104,     4,     4,   111,   180,   111,    16,     6,   111,
     111,     6,   162,   153,     6,    89,   141,    89,   185,     4,
      92,    13,    19,    20,    21,    96,    97,    97,    97,   111,
     105,    95,   187,    93,     4,     4,    25,   107,    32,   111,
     111,   111,   181,   120,   111,   108,   113,     4,   111,   111,
     105,   111,    94,   157,    95,     4,   107,     6,    57,    22,
      24,    25,    27,    29,    56,    57,    65,    18,   111,   111,
     127,   134,   178,    33,    34,    35,    36,    37,    38,    39,
      40,   114,   115,     4,    76,   136,   154,   155,   107,    93,
     136,   186,   186,   186,   186,   186,   186,   186,   186,   124,
      33,    40,     4,    88,   154,   111,   166,    98,    91,   100,
     102,   103,   106,   137,   140,   150,   111,     6,   107,    95,
       4,     4,     4,     4,     4,     4,     4,     4,     4,    95,
     111,   163,    89,   159,    72,   136
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    86,    87,    88,    89,    90,    90,    87,    91,    91,
      92,    92,    92,    93,    93,    93,    94,    94,    94,    94,
      95,    96,    96,    96,    96,    97,    98,    88,    99,    99,
     100,   101,    88,   102,   102,   103,    88,   104,   104,   105,
     105,   106,    88,   107,   108,   109,   109,   110,   110,   111,
     112,   111,   113,   113,   114,   114,   114,   114,   114,   114,
     114,   115,   114,   116,   117,   117,   118,   118,   118,   119,
     120,   121,    88,    88,   122,   123,   123,   124,   124,   124,
     124,   125,   127,   126,   128,   128,   129,   129,    88,    88,
     130,   130,   130,   130,   131,   131,   131,    88,   133,   132,
      88,   134,   134,   135,   135,   136,   138,   137,    88,   139,
     139,   140,   140,   140,   141,    88,   142,   143,   143,   143,
     144,   143,   143,   143,    88,   145,   145,   145,   145,   145,
     145,   145,   145,   145,    88,   146,   146,   145,    88,   147,
      88,    88,    88,   148,   148,    88,   149,   149,   150,    88,
     151,   152,   152,   152,   152,    88,    88,   153,   154,   154,
     155,   157,   156,   158,   158,   158,   158,   159,   160,    88,
      88,   161,   161,   161,    88,   162,   162,   163,   163,   165,
     166,   164,    88,    88,   167,   169,   168,   170,   170,   171,
     173,   172,   174,   174,   175,   176,   176,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   177,   177,   178,   179,   181,   180,   182,   183,   184,
     184,   185,   185,   185,   185,   185,   185,   185,   185,   186,
     187,   188,   189,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,   107,   191,   190
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
       4,     4,     1,     1,     2,     5,     5,     2,     2,     2,
       2,     3,     0,     7,     4,     4,     4,     4,     1,     1,
       0,     1,     1,     1,     3,     3,     3,     4,     0,     4,
       4,     0,     2,     1,     1,     1,     0,     6,     4,     1,
       2,     3,     2,     1,     4,     1,     1,     3,     4,     3,
       0,     4,     4,     3,     4,     2,     3,     3,     2,     3,
       3,     3,     4,     4,     4,     1,     1,     1,     4,     1,
       4,     5,     4,     4,     4,     4,     2,     0,     4,     4,
       2,     1,     1,     1,     0,     5,     5,     7,     1,     1,
       3,     0,     4,     1,     1,     1,     1,    10,     4,     1,
       1,     2,     3,     2,     5,     0,     1,     0,     2,     0,
       0,    10,     5,     5,     3,     0,     6,     0,     2,     5,
       0,     6,     0,     2,     4,     0,     4,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     5,     5,     5,     5,     5,     5,
       5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
       5,     1,     2,     2,     1,     0,     5,     2,     4,     2,
       2,     5,     5,     5,     5,     5,     5,     5,     5,     1,
       1,     1,     1,     4,     5,     6,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     0,     5
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
#line 264 "parser.y"
                        {
	#line 418 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2018 "parser.c"
    break;

  case 3: /* content_node: start "glyph" glyph ">"  */
#line 267 "parser.y"
                                  {
	#line 419 "format.w"
	hput_tags((yyvsp[-3].u),hput_glyph(&((yyvsp[-1].c))));}
#line 2026 "parser.c"
    break;

  case 4: /* start: "<"  */
#line 270 "parser.y"
           {
	#line 420 "format.w"
	HPUTNODE;(yyval.u)= (uint32_t)(hpos++-hstart);}
#line 2034 "parser.c"
    break;

  case 6: /* integer: UNSIGNED  */
#line 274 "parser.y"
                         {
	#line 941 "format.w"
	RNG("number",(yyvsp[0].u),0,0x7FFFFFFF);}
#line 2042 "parser.c"
    break;

  case 7: /* glyph: CHARCODE REFERENCE  */
#line 278 "parser.y"
                        {
	#line 1082 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2050 "parser.c"
    break;

  case 9: /* string: CHARCODE  */
#line 282 "parser.y"
                         {
	#line 1187 "format.w"
	static char s[2];
	RNG("String element",(yyvsp[0].u),0x20,0x7E);
	s[0]= (yyvsp[0].u);s[1]= 0;(yyval.s)= s;}
#line 2060 "parser.c"
    break;

  case 10: /* number: UNSIGNED  */
#line 288 "parser.y"
               {
	#line 1339 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].u);}
#line 2068 "parser.c"
    break;

  case 11: /* number: SIGNED  */
#line 290 "parser.y"
                                       {
	#line 1339 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].i);}
#line 2076 "parser.c"
    break;

  case 13: /* dimension: number "pt"  */
#line 294 "parser.y"
                   {
	#line 1682 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2084 "parser.c"
    break;

  case 14: /* dimension: number "in"  */
#line 297 "parser.y"
                    {
	#line 1683 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*72.27);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2092 "parser.c"
    break;

  case 15: /* dimension: number "mm"  */
#line 300 "parser.y"
                  {
	#line 1684 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*(72.27/25.4));RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2100 "parser.c"
    break;

  case 16: /* xdimen: dimension number "h" number "v"  */
#line 304 "parser.y"
                                  {
	#line 1762 "format.w"
	(yyval.xd).w= (yyvsp[-4].d);(yyval.xd).h= (yyvsp[-3].f);(yyval.xd).v= (yyvsp[-1].f);}
#line 2108 "parser.c"
    break;

  case 17: /* xdimen: dimension number "h"  */
#line 307 "parser.y"
                           {
	#line 1763 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= (yyvsp[-1].f);(yyval.xd).v= 0.0;}
#line 2116 "parser.c"
    break;

  case 18: /* xdimen: dimension number "v"  */
#line 310 "parser.y"
                           {
	#line 1764 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= 0.0;(yyval.xd).v= (yyvsp[-1].f);}
#line 2124 "parser.c"
    break;

  case 19: /* xdimen: dimension  */
#line 313 "parser.y"
                  {
	#line 1765 "format.w"
	(yyval.xd).w= (yyvsp[0].d);(yyval.xd).h= 0.0;(yyval.xd).v= 0.0;}
#line 2132 "parser.c"
    break;

  case 20: /* xdimen_node: start "xdimen" xdimen ">"  */
#line 319 "parser.y"
                                   {
	#line 1769 "format.w"
	hput_tags((yyvsp[-3].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 2140 "parser.c"
    break;

  case 21: /* order: "pt"  */
#line 324 "parser.y"
        {
	#line 1947 "format.w"
	(yyval.o)= normal_o;}
#line 2148 "parser.c"
    break;

  case 22: /* order: "fil"  */
#line 326 "parser.y"
                            {
	#line 1947 "format.w"
	(yyval.o)= fil_o;}
#line 2156 "parser.c"
    break;

  case 23: /* order: "fill"  */
#line 328 "parser.y"
                             {
	#line 1947 "format.w"
	(yyval.o)= fill_o;}
#line 2164 "parser.c"
    break;

  case 24: /* order: "filll"  */
#line 330 "parser.y"
                              {
	#line 1947 "format.w"
	(yyval.o)= filll_o;}
#line 2172 "parser.c"
    break;

  case 25: /* stretch: number order  */
#line 334 "parser.y"
                    {
	#line 1949 "format.w"
	(yyval.st).f= (yyvsp[-1].f);(yyval.st).o= (yyvsp[0].o);}
#line 2180 "parser.c"
    break;

  case 26: /* penalty: integer  */
#line 338 "parser.y"
               {
	#line 2003 "format.w"
	RNG("Penalty",(yyvsp[0].i),-20000,+20000);(yyval.i)= (yyvsp[0].i);}
#line 2188 "parser.c"
    break;

  case 27: /* content_node: start "penalty" penalty ">"  */
#line 341 "parser.y"
                                      {
	#line 2004 "format.w"
	hput_tags((yyvsp[-3].u),hput_int((yyvsp[-1].i)));}
#line 2196 "parser.c"
    break;

  case 29: /* rule_dimension: "|"  */
#line 345 "parser.y"
                                        {
	#line 2179 "format.w"
	(yyval.d)= RUNNING_DIMEN;}
#line 2204 "parser.c"
    break;

  case 30: /* rule: rule_dimension rule_dimension rule_dimension  */
#line 349 "parser.y"
{
	#line 2181 "format.w"
	(yyval.r).h= (yyvsp[-2].d);(yyval.r).d= (yyvsp[-1].d);(yyval.r).w= (yyvsp[0].d);
	if((yyvsp[0].d)==RUNNING_DIMEN&&((yyvsp[-2].d)==RUNNING_DIMEN||(yyvsp[-1].d)==RUNNING_DIMEN))
	QUIT("Incompatible running dimensions 0x%x 0x%x 0x%x",(yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2214 "parser.c"
    break;

  case 31: /* rule_node: start "rule" rule ">"  */
#line 354 "parser.y"
                             {
	#line 2184 "format.w"
	hput_tags((yyvsp[-3].u),hput_rule(&((yyvsp[-1].r))));}
#line 2222 "parser.c"
    break;

  case 33: /* explicit: %empty  */
#line 359 "parser.y"
         {
	#line 2292 "format.w"
	(yyval.b)= false;}
#line 2230 "parser.c"
    break;

  case 34: /* explicit: "!"  */
#line 361 "parser.y"
                                 {
	#line 2292 "format.w"
	(yyval.b)= true;}
#line 2238 "parser.c"
    break;

  case 35: /* kern: explicit xdimen  */
#line 364 "parser.y"
                    {
	#line 2293 "format.w"
	(yyval.kt).x= (yyvsp[-1].b);(yyval.kt).d= (yyvsp[0].xd);}
#line 2246 "parser.c"
    break;

  case 36: /* content_node: start "kern" kern ">"  */
#line 367 "parser.y"
                                {
	#line 2294 "format.w"
	hput_tags((yyvsp[-3].u),hput_kern(&((yyvsp[-1].kt))));}
#line 2254 "parser.c"
    break;

  case 37: /* plus: %empty  */
#line 371 "parser.y"
     {
	#line 2504 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2262 "parser.c"
    break;

  case 38: /* plus: "plus" stretch  */
#line 373 "parser.y"
                                             {
	#line 2504 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2270 "parser.c"
    break;

  case 39: /* minus: %empty  */
#line 376 "parser.y"
      {
	#line 2505 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2278 "parser.c"
    break;

  case 40: /* minus: "minus" stretch  */
#line 378 "parser.y"
                                              {
	#line 2505 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2286 "parser.c"
    break;

  case 41: /* glue: xdimen plus minus  */
#line 381 "parser.y"
                      {
	#line 2506 "format.w"
	(yyval.g).w= (yyvsp[-2].xd);(yyval.g).p= (yyvsp[-1].st);(yyval.g).m= (yyvsp[0].st);}
#line 2294 "parser.c"
    break;

  case 42: /* content_node: start "glue" glue ">"  */
#line 384 "parser.y"
                                {
	#line 2507 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){HPUT8(zero_skip_no);
	hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}else hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));}
#line 2303 "parser.c"
    break;

  case 43: /* glue_node: start "glue" glue ">"  */
#line 389 "parser.y"
{
	#line 2510 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){hpos--;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));(yyval.b)= true;}}
#line 2312 "parser.c"
    break;

  case 44: /* position: %empty  */
#line 394 "parser.y"
         {
	#line 2786 "format.w"
	(yyval.u)= hpos-hstart;}
#line 2320 "parser.c"
    break;

  case 47: /* estimate: %empty  */
#line 399 "parser.y"
         {
	#line 2789 "format.w"
	hpos+= 2;}
#line 2328 "parser.c"
    break;

  case 48: /* estimate: UNSIGNED  */
#line 402 "parser.y"
                 {
	#line 2790 "format.w"
	hpos+= hsize_bytes((yyvsp[0].u))+1;}
#line 2336 "parser.c"
    break;

  case 49: /* list: start estimate content_list ">"  */
#line 406 "parser.y"
{
	#line 2792 "format.w"
	(yyval.l).k= list_kind;(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2345 "parser.c"
    break;

  case 50: /* $@1: %empty  */
#line 412 "parser.y"
{
	#line 3199 "format.w"
	hpos+= 4;}
#line 2353 "parser.c"
    break;

  case 51: /* list: TXT_START position $@1 text TXT_END  */
#line 416 "parser.y"
{
	#line 3201 "format.w"
	(yyval.l).k= text_kind;(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2362 "parser.c"
    break;

  case 54: /* txt: TXT_CC  */
#line 422 "parser.y"
          {
	#line 3205 "format.w"
	hput_txt_cc((yyvsp[0].u));}
#line 2370 "parser.c"
    break;

  case 55: /* txt: TXT_FONT  */
#line 425 "parser.y"
                 {
	#line 3206 "format.w"
	REF(font_kind,(yyvsp[0].u));hput_txt_font((yyvsp[0].u));}
#line 2378 "parser.c"
    break;

  case 56: /* txt: TXT_GLOBAL  */
#line 428 "parser.y"
                   {
	#line 3207 "format.w"
	REF((yyvsp[0].rf).k,(yyvsp[0].rf).n);hput_txt_global(&((yyvsp[0].rf)));}
#line 2386 "parser.c"
    break;

  case 57: /* txt: TXT_LOCAL  */
#line 431 "parser.y"
                  {
	#line 3208 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,11);hput_txt_local((yyvsp[0].u));}
#line 2394 "parser.c"
    break;

  case 58: /* txt: TXT_FONT_GLUE  */
#line 434 "parser.y"
                      {
	#line 3209 "format.w"
	HPUTX(1);HPUT8(txt_glue);}
#line 2402 "parser.c"
    break;

  case 59: /* txt: TXT_FONT_HYPHEN  */
#line 437 "parser.y"
                        {
	#line 3210 "format.w"
	HPUTX(1);HPUT8(txt_hyphen);}
#line 2410 "parser.c"
    break;

  case 60: /* txt: TXT_IGNORE  */
#line 440 "parser.y"
                   {
	#line 3211 "format.w"
	HPUTX(1);HPUT8(txt_ignore);}
#line 2418 "parser.c"
    break;

  case 61: /* $@2: %empty  */
#line 443 "parser.y"
         {
	#line 3212 "format.w"
	HPUTX(1);HPUT8(txt_node);}
#line 2426 "parser.c"
    break;

  case 63: /* box_dimen: dimension dimension dimension  */
#line 449 "parser.y"
{
	#line 3472 "format.w"
	(yyval.info)= hput_box_dimen((yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2434 "parser.c"
    break;

  case 64: /* box_shift: %empty  */
#line 452 "parser.y"
          {
	#line 3473 "format.w"
	(yyval.info)= b000;}
#line 2442 "parser.c"
    break;

  case 65: /* box_shift: "shifted" dimension  */
#line 455 "parser.y"
                          {
	#line 3474 "format.w"
	(yyval.info)= hput_box_shift((yyvsp[0].d));}
#line 2450 "parser.c"
    break;

  case 66: /* box_glue_set: %empty  */
#line 459 "parser.y"
             {
	#line 3476 "format.w"
	(yyval.info)= b000;}
#line 2458 "parser.c"
    break;

  case 67: /* box_glue_set: "plus" stretch  */
#line 462 "parser.y"
                     {
	#line 3477 "format.w"
	(yyval.info)= hput_box_glue_set(+1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2466 "parser.c"
    break;

  case 68: /* box_glue_set: "minus" stretch  */
#line 465 "parser.y"
                      {
	#line 3478 "format.w"
	(yyval.info)= hput_box_glue_set(-1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2474 "parser.c"
    break;

  case 69: /* box: box_dimen box_shift box_glue_set list  */
#line 470 "parser.y"
                                         {
	#line 3481 "format.w"
	(yyval.info)= (yyvsp[-3].info)	|(yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2482 "parser.c"
    break;

  case 70: /* hbox_node: start "hbox" box ">"  */
#line 474 "parser.y"
                            {
	#line 3483 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hbox_kind,(yyvsp[-1].info)));}
#line 2490 "parser.c"
    break;

  case 71: /* vbox_node: start "vbox" box ">"  */
#line 477 "parser.y"
                            {
	#line 3484 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vbox_kind,(yyvsp[-1].info)));}
#line 2498 "parser.c"
    break;

  case 74: /* box_flex: plus minus  */
#line 482 "parser.y"
                   {
	#line 3665 "format.w"
	hput_stretch(&((yyvsp[-1].st)));hput_stretch(&((yyvsp[0].st)));}
#line 2506 "parser.c"
    break;

  case 75: /* xbox: box_dimen box_shift box_flex xdimen_ref list  */
#line 485 "parser.y"
                                                 {
	#line 3666 "format.w"
	(yyval.info)= (yyvsp[-4].info)	|(yyvsp[-3].info);}
#line 2514 "parser.c"
    break;

  case 76: /* xbox: box_dimen box_shift box_flex xdimen_node list  */
#line 488 "parser.y"
                                                      {
	#line 3667 "format.w"
	(yyval.info)= (yyvsp[-4].info)	|(yyvsp[-3].info)	|b100;}
#line 2522 "parser.c"
    break;

  case 77: /* box_goal: "to" xdimen_ref  */
#line 492 "parser.y"
                      {
	#line 3669 "format.w"
	(yyval.info)= b000;}
#line 2530 "parser.c"
    break;

  case 78: /* box_goal: "add" xdimen_ref  */
#line 495 "parser.y"
                       {
	#line 3670 "format.w"
	(yyval.info)= b001;}
#line 2538 "parser.c"
    break;

  case 79: /* box_goal: "to" xdimen_node  */
#line 498 "parser.y"
                       {
	#line 3671 "format.w"
	(yyval.info)= b100;}
#line 2546 "parser.c"
    break;

  case 80: /* box_goal: "add" xdimen_node  */
#line 501 "parser.y"
                        {
	#line 3672 "format.w"
	(yyval.info)= b101;}
#line 2554 "parser.c"
    break;

  case 81: /* hpack: box_shift box_goal list  */
#line 505 "parser.y"
                             {
	#line 3674 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 2562 "parser.c"
    break;

  case 82: /* $@3: %empty  */
#line 508 "parser.y"
                                   {
	#line 3675 "format.w"
	HPUT32((yyvsp[0].d));}
#line 2570 "parser.c"
    break;

  case 83: /* vpack: box_shift "max" "depth" dimension $@3 box_goal list  */
#line 510 "parser.y"
                                 {
	#line 3675 "format.w"
	(yyval.info)= (yyvsp[-6].info)	|(yyvsp[-1].info);}
#line 2578 "parser.c"
    break;

  case 84: /* vxbox_node: start "vset" xbox ">"  */
#line 514 "parser.y"
                              {
	#line 3677 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vset_kind,(yyvsp[-1].info)));}
#line 2586 "parser.c"
    break;

  case 85: /* vxbox_node: start "vpack" vpack ">"  */
#line 517 "parser.y"
                              {
	#line 3678 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vpack_kind,(yyvsp[-1].info)));}
#line 2594 "parser.c"
    break;

  case 86: /* hxbox_node: start "hset" xbox ">"  */
#line 522 "parser.y"
                              {
	#line 3681 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hset_kind,(yyvsp[-1].info)));}
#line 2602 "parser.c"
    break;

  case 87: /* hxbox_node: start "hpack" hpack ">"  */
#line 525 "parser.y"
                              {
	#line 3682 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hpack_kind,(yyvsp[-1].info)));}
#line 2610 "parser.c"
    break;

  case 90: /* ltype: %empty  */
#line 531 "parser.y"
      {
	#line 3792 "format.w"
	(yyval.info)= 1;}
#line 2618 "parser.c"
    break;

  case 91: /* ltype: "align"  */
#line 533 "parser.y"
                      {
	#line 3792 "format.w"
	(yyval.info)= 1;}
#line 2626 "parser.c"
    break;

  case 92: /* ltype: "center"  */
#line 535 "parser.y"
                       {
	#line 3792 "format.w"
	(yyval.info)= 2;}
#line 2634 "parser.c"
    break;

  case 93: /* ltype: "expand"  */
#line 537 "parser.y"
                       {
	#line 3792 "format.w"
	(yyval.info)= 3;}
#line 2642 "parser.c"
    break;

  case 94: /* leaders: glue_node ltype rule_node  */
#line 540 "parser.y"
                                 {
	#line 3793 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2650 "parser.c"
    break;

  case 95: /* leaders: glue_node ltype hbox_node  */
#line 543 "parser.y"
                                  {
	#line 3794 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2658 "parser.c"
    break;

  case 96: /* leaders: glue_node ltype vbox_node  */
#line 546 "parser.y"
                                  {
	#line 3795 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2666 "parser.c"
    break;

  case 97: /* content_node: start "leaders" leaders ">"  */
#line 549 "parser.y"
                                      {
	#line 3796 "format.w"
	hput_tags((yyvsp[-3].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 2674 "parser.c"
    break;

  case 98: /* $@4: %empty  */
#line 553 "parser.y"
                  {
	#line 3902 "format.w"
	if((yyvsp[0].d)!=0)HPUT32((yyvsp[0].d));}
#line 2682 "parser.c"
    break;

  case 99: /* baseline: dimension $@4 glue_node glue_node  */
#line 556 "parser.y"
                   {
	#line 3903 "format.w"
	(yyval.info)= b000;if((yyvsp[-3].d)!=0)(yyval.info)	|= b001;
	if((yyvsp[-1].b))(yyval.info)	|= b100;
	if((yyvsp[0].b))(yyval.info)	|= b010;
	}
#line 2693 "parser.c"
    break;

  case 100: /* content_node: start "baseline" baseline ">"  */
#line 563 "parser.y"
{
	#line 3908 "format.w"
	if((yyvsp[-1].info)==b000)HPUT8(0);hput_tags((yyvsp[-3].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 2701 "parser.c"
    break;

  case 102: /* cc_list: cc_list TXT_CC  */
#line 567 "parser.y"
                               {
	#line 3991 "format.w"
	hput_utf8((yyvsp[0].u));}
#line 2709 "parser.c"
    break;

  case 103: /* lig_cc: UNSIGNED  */
#line 570 "parser.y"
               {
	#line 3992 "format.w"
	RNG("UTF-8 code",(yyvsp[0].u),0,0x1FFFFF);(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2717 "parser.c"
    break;

  case 104: /* lig_cc: CHARCODE  */
#line 573 "parser.y"
               {
	#line 3993 "format.w"
	(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2725 "parser.c"
    break;

  case 105: /* ref: REFERENCE  */
#line 576 "parser.y"
             {
	#line 3994 "format.w"
	HPUT8((yyvsp[0].u));(yyval.u)= (yyvsp[0].u);}
#line 2733 "parser.c"
    break;

  case 106: /* $@5: %empty  */
#line 579 "parser.y"
            {
	#line 3995 "format.w"
	REF(font_kind,(yyvsp[0].u));}
#line 2741 "parser.c"
    break;

  case 107: /* ligature: ref $@5 lig_cc TXT_START cc_list TXT_END  */
#line 582 "parser.y"
{
	#line 3996 "format.w"
	(yyval.lg).f= (yyvsp[-5].u);(yyval.lg).l.p= (yyvsp[-3].u);(yyval.lg).l.s= (hpos-hstart)-(yyvsp[-3].u);
	RNG("Ligature size",(yyval.lg).l.s,0,255);}
#line 2750 "parser.c"
    break;

  case 108: /* content_node: start "ligature" ligature ">"  */
#line 586 "parser.y"
                                        {
	#line 3998 "format.w"
	hput_tags((yyvsp[-3].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 2758 "parser.c"
    break;

  case 109: /* replace_count: explicit  */
#line 590 "parser.y"
                      {
	#line 4108 "format.w"
	if((yyvsp[0].b)){(yyval.u)= 0x80;HPUT8(0x80);}else (yyval.u)= 0x00;}
#line 2766 "parser.c"
    break;

  case 110: /* replace_count: explicit UNSIGNED  */
#line 593 "parser.y"
                          {
	#line 4109 "format.w"
	RNG("Replace count",(yyvsp[0].u),0,31);
	(yyval.u)= ((yyvsp[0].u))	|(((yyvsp[-1].b))?0x80:0x00);if((yyval.u)!=0)HPUT8((yyval.u));}
#line 2775 "parser.c"
    break;

  case 111: /* disc: replace_count list list  */
#line 597 "parser.y"
                            {
	#line 4111 "format.w"
	(yyval.dc).r= (yyvsp[-2].u);(yyval.dc).p= (yyvsp[-1].l);(yyval.dc).q= (yyvsp[0].l);
	if((yyvsp[0].l).s==0){hpos= hpos-2;if((yyvsp[-1].l).s==0)hpos= hpos-2;}}
#line 2784 "parser.c"
    break;

  case 112: /* disc: replace_count list  */
#line 601 "parser.y"
                           {
	#line 4113 "format.w"
	(yyval.dc).r= (yyvsp[-1].u);(yyval.dc).p= (yyvsp[0].l);if((yyvsp[0].l).s==0)hpos= hpos-2;(yyval.dc).q.s= 0;}
#line 2792 "parser.c"
    break;

  case 113: /* disc: replace_count  */
#line 604 "parser.y"
                      {
	#line 4114 "format.w"
	(yyval.dc).r= (yyvsp[0].u);(yyval.dc).p.s= 0;(yyval.dc).q.s= 0;}
#line 2800 "parser.c"
    break;

  case 114: /* disc_node: start "disc" disc ">"  */
#line 610 "parser.y"
{
	#line 4118 "format.w"
	hput_tags((yyvsp[-3].u),hput_disc(&((yyvsp[-1].dc))));}
#line 2808 "parser.c"
    break;

  case 116: /* par_dimen: xdimen  */
#line 616 "parser.y"
                {
	#line 4270 "format.w"
	hput_xdimen_node(&((yyvsp[0].xd)));}
#line 2816 "parser.c"
    break;

  case 117: /* par: xdimen_ref param_ref list  */
#line 619 "parser.y"
                             {
	#line 4271 "format.w"
	(yyval.info)= b000;}
#line 2824 "parser.c"
    break;

  case 118: /* par: xdimen_ref empty_param_list non_empty_param_list list  */
#line 622 "parser.y"
                                                              {
	#line 4272 "format.w"
	(yyval.info)= b010;}
#line 2832 "parser.c"
    break;

  case 119: /* par: xdimen_ref empty_param_list list  */
#line 625 "parser.y"
                                         {
	#line 4273 "format.w"
	(yyval.info)= b010;}
#line 2840 "parser.c"
    break;

  case 120: /* $@6: %empty  */
#line 628 "parser.y"
                         {
	#line 4274 "format.w"
	hput_xdimen_node(&((yyvsp[-1].xd)));}
#line 2848 "parser.c"
    break;

  case 121: /* par: xdimen param_ref $@6 list  */
#line 630 "parser.y"
                                     {
	#line 4274 "format.w"
	(yyval.info)= b100;}
#line 2856 "parser.c"
    break;

  case 122: /* par: par_dimen empty_param_list non_empty_param_list list  */
#line 633 "parser.y"
                                                             {
	#line 4275 "format.w"
	(yyval.info)= b110;}
#line 2864 "parser.c"
    break;

  case 123: /* par: par_dimen empty_param_list list  */
#line 636 "parser.y"
                                        {
	#line 4276 "format.w"
	(yyval.info)= b110;}
#line 2872 "parser.c"
    break;

  case 124: /* content_node: start "par" par ">"  */
#line 640 "parser.y"
                              {
	#line 4278 "format.w"
	hput_tags((yyvsp[-3].u),TAG(par_kind,(yyvsp[-1].info)));}
#line 2880 "parser.c"
    break;

  case 125: /* math: param_ref list  */
#line 644 "parser.y"
                   {
	#line 4344 "format.w"
	(yyval.info)= b000;}
#line 2888 "parser.c"
    break;

  case 126: /* math: param_ref list hbox_node  */
#line 647 "parser.y"
                                 {
	#line 4345 "format.w"
	(yyval.info)= b001;}
#line 2896 "parser.c"
    break;

  case 127: /* math: param_ref hbox_node list  */
#line 650 "parser.y"
                                 {
	#line 4346 "format.w"
	(yyval.info)= b010;}
#line 2904 "parser.c"
    break;

  case 128: /* math: empty_param_list list  */
#line 653 "parser.y"
                              {
	#line 4347 "format.w"
	(yyval.info)= b100;}
#line 2912 "parser.c"
    break;

  case 129: /* math: empty_param_list list hbox_node  */
#line 656 "parser.y"
                                        {
	#line 4348 "format.w"
	(yyval.info)= b101;}
#line 2920 "parser.c"
    break;

  case 130: /* math: empty_param_list hbox_node list  */
#line 659 "parser.y"
                                        {
	#line 4349 "format.w"
	(yyval.info)= b110;}
#line 2928 "parser.c"
    break;

  case 131: /* math: empty_param_list non_empty_param_list list  */
#line 662 "parser.y"
                                                   {
	#line 4350 "format.w"
	(yyval.info)= b100;}
#line 2936 "parser.c"
    break;

  case 132: /* math: empty_param_list non_empty_param_list list hbox_node  */
#line 665 "parser.y"
                                                             {
	#line 4351 "format.w"
	(yyval.info)= b101;}
#line 2944 "parser.c"
    break;

  case 133: /* math: empty_param_list non_empty_param_list hbox_node list  */
#line 668 "parser.y"
                                                             {
	#line 4352 "format.w"
	(yyval.info)= b110;}
#line 2952 "parser.c"
    break;

  case 134: /* content_node: start "math" math ">"  */
#line 672 "parser.y"
                                {
	#line 4354 "format.w"
	hput_tags((yyvsp[-3].u),TAG(math_kind,(yyvsp[-1].info)));}
#line 2960 "parser.c"
    break;

  case 135: /* on_off: "on"  */
#line 676 "parser.y"
         {
	#line 4404 "format.w"
	(yyval.i)= 1;}
#line 2968 "parser.c"
    break;

  case 136: /* on_off: "off"  */
#line 678 "parser.y"
                    {
	#line 4404 "format.w"
	(yyval.i)= 0;}
#line 2976 "parser.c"
    break;

  case 137: /* math: on_off  */
#line 681 "parser.y"
           {
	#line 4405 "format.w"
	(yyval.info)= b011	|((yyvsp[0].i)<<2);}
#line 2984 "parser.c"
    break;

  case 138: /* content_node: start "adjust" list ">"  */
#line 685 "parser.y"
                                  {
	#line 4436 "format.w"
	hput_tags((yyvsp[-3].u),TAG(adjust_kind,1));}
#line 2992 "parser.c"
    break;

  case 139: /* span_count: UNSIGNED  */
#line 689 "parser.y"
                   {
	#line 4535 "format.w"
	(yyval.info)= hput_span_count((yyvsp[0].u));}
#line 3000 "parser.c"
    break;

  case 140: /* content_node: start "item" content_node ">"  */
#line 692 "parser.y"
                                        {
	#line 4536 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,1));}
#line 3008 "parser.c"
    break;

  case 141: /* content_node: start "item" span_count content_node ">"  */
#line 695 "parser.y"
                                                   {
	#line 4537 "format.w"
	hput_tags((yyvsp[-4].u),TAG(item_kind,(yyvsp[-2].info)));}
#line 3016 "parser.c"
    break;

  case 142: /* content_node: start "item" list ">"  */
#line 698 "parser.y"
                                {
	#line 4538 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,b000));}
#line 3024 "parser.c"
    break;

  case 143: /* table: "h" box_goal list list  */
#line 702 "parser.y"
                          {
	#line 4540 "format.w"
	(yyval.info)= (yyvsp[-2].info);}
#line 3032 "parser.c"
    break;

  case 144: /* table: "v" box_goal list list  */
#line 705 "parser.y"
                          {
	#line 4541 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|b010;}
#line 3040 "parser.c"
    break;

  case 145: /* content_node: start "table" table ">"  */
#line 709 "parser.y"
                                  {
	#line 4543 "format.w"
	hput_tags((yyvsp[-3].u),TAG(table_kind,(yyvsp[-1].info)));}
#line 3048 "parser.c"
    break;

  case 146: /* image_dimen: dimension dimension  */
#line 713 "parser.y"
                               {
	#line 4631 "format.w"
	(yyval.x).w= (yyvsp[-1].d);(yyval.x).h= (yyvsp[0].d);}
#line 3056 "parser.c"
    break;

  case 147: /* image_dimen: %empty  */
#line 715 "parser.y"
                                 {
	#line 4631 "format.w"
	(yyval.x).w= (yyval.x).h= 0;}
#line 3064 "parser.c"
    break;

  case 148: /* image: UNSIGNED image_dimen plus minus  */
#line 718 "parser.y"
                                     {
	#line 4632 "format.w"
	(yyval.x).w= (yyvsp[-2].x).w;(yyval.x).h= (yyvsp[-2].x).h;(yyval.x).p= (yyvsp[-1].st);(yyval.x).m= (yyvsp[0].st);RNG("Section number",(yyvsp[-3].u),3,max_section_no);(yyval.x).n= (yyvsp[-3].u);}
#line 3072 "parser.c"
    break;

  case 149: /* content_node: start "image" image ">"  */
#line 721 "parser.y"
                                  {
	#line 4633 "format.w"
	hput_tags((yyvsp[-3].u),hput_image(&((yyvsp[-1].x))));}
#line 3080 "parser.c"
    break;

  case 150: /* max_value: "outline" UNSIGNED  */
#line 725 "parser.y"
                          {
	#line 4870 "format.w"
	max_outline= (yyvsp[0].u);
	RNG("max outline",max_outline,0,0xFFFF);
	DBG(DBGDEF	|DBGLABEL,"Setting max outline to %d\n",max_outline);
	}
#line 3091 "parser.c"
    break;

  case 151: /* placement: "top"  */
#line 732 "parser.y"
             {
	#line 4962 "format.w"
	(yyval.i)= LABEL_TOP;}
#line 3099 "parser.c"
    break;

  case 152: /* placement: "bot"  */
#line 734 "parser.y"
                            {
	#line 4962 "format.w"
	(yyval.i)= LABEL_BOT;}
#line 3107 "parser.c"
    break;

  case 153: /* placement: "mid"  */
#line 736 "parser.y"
                            {
	#line 4962 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3115 "parser.c"
    break;

  case 154: /* placement: %empty  */
#line 738 "parser.y"
                         {
	#line 4962 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3123 "parser.c"
    break;

  case 155: /* content_node: "<" "label" REFERENCE placement ">"  */
#line 742 "parser.y"
{
	#line 4964 "format.w"
	hset_label((yyvsp[-2].u),(yyvsp[-1].i));}
#line 3131 "parser.c"
    break;

  case 156: /* content_node: start "link" REFERENCE on_off ">"  */
#line 747 "parser.y"
{
	#line 5222 "format.w"
	hput_tags((yyvsp[-4].u),hput_link((yyvsp[-2].u),(yyvsp[-1].i)));}
#line 3139 "parser.c"
    break;

  case 157: /* def_node: "<" "outline" REFERENCE integer position list ">"  */
#line 751 "parser.y"
                                                          {
	#line 5352 "format.w"
	
	static int outline_no= -1;
	(yyval.rf).k= outline_kind;(yyval.rf).n= (yyvsp[-4].u);
	if((yyvsp[-1].l).s==0)QUIT("Outline with empty title in line %d",yylineno);
	outline_no++;
	hset_outline(outline_no,(yyvsp[-4].u),(yyvsp[-3].i),(yyvsp[-2].u));
	}
#line 3153 "parser.c"
    break;

  case 158: /* stream_link: ref  */
#line 761 "parser.y"
               {
	#line 5767 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3161 "parser.c"
    break;

  case 159: /* stream_link: "*"  */
#line 763 "parser.y"
                                                    {
	#line 5767 "format.w"
	HPUT8(255);}
#line 3169 "parser.c"
    break;

  case 160: /* stream_split: stream_link stream_link UNSIGNED  */
#line 766 "parser.y"
                                             {
	#line 5768 "format.w"
	RNG("split ratio",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3177 "parser.c"
    break;

  case 161: /* $@7: %empty  */
#line 769 "parser.y"
                                {
	#line 5769 "format.w"
	RNG("magnification factor",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3185 "parser.c"
    break;

  case 163: /* stream_type: stream_info  */
#line 773 "parser.y"
                       {
	#line 5771 "format.w"
	(yyval.info)= 0;}
#line 3193 "parser.c"
    break;

  case 164: /* stream_type: "first"  */
#line 775 "parser.y"
                      {
	#line 5771 "format.w"
	(yyval.info)= 1;}
#line 3201 "parser.c"
    break;

  case 165: /* stream_type: "last"  */
#line 777 "parser.y"
                     {
	#line 5771 "format.w"
	(yyval.info)= 2;}
#line 3209 "parser.c"
    break;

  case 166: /* stream_type: "top"  */
#line 779 "parser.y"
                    {
	#line 5771 "format.w"
	(yyval.info)= 3;}
#line 3217 "parser.c"
    break;

  case 167: /* stream_def_node: start "stream (definition)" ref stream_type list xdimen_node glue_node list glue_node ">"  */
#line 785 "parser.y"
{
	#line 5775 "format.w"
	DEF((yyval.rf),stream_kind,(yyvsp[-7].u));hput_tags((yyvsp[-9].u),TAG(stream_kind,(yyvsp[-6].info)	|b100));}
#line 3225 "parser.c"
    break;

  case 168: /* stream_ins_node: start "stream (definition)" ref ">"  */
#line 790 "parser.y"
{
	#line 5778 "format.w"
	RNG("Stream insertion",(yyvsp[-1].u),0,max_ref[stream_kind]);hput_tags((yyvsp[-3].u),TAG(stream_kind,b100));}
#line 3233 "parser.c"
    break;

  case 171: /* stream: empty_param_list list  */
#line 796 "parser.y"
                            {
	#line 5873 "format.w"
	(yyval.info)= b010;}
#line 3241 "parser.c"
    break;

  case 172: /* stream: empty_param_list non_empty_param_list list  */
#line 799 "parser.y"
                                                   {
	#line 5874 "format.w"
	(yyval.info)= b010;}
#line 3249 "parser.c"
    break;

  case 173: /* stream: param_ref list  */
#line 802 "parser.y"
                       {
	#line 5875 "format.w"
	(yyval.info)= b000;}
#line 3257 "parser.c"
    break;

  case 174: /* content_node: start "stream" stream_ref stream ">"  */
#line 806 "parser.y"
{
	#line 5877 "format.w"
	hput_tags((yyvsp[-4].u),TAG(stream_kind,(yyvsp[-1].info)));}
#line 3265 "parser.c"
    break;

  case 175: /* page_priority: %empty  */
#line 810 "parser.y"
              {
	#line 5980 "format.w"
	HPUT8(1);}
#line 3273 "parser.c"
    break;

  case 176: /* page_priority: UNSIGNED  */
#line 813 "parser.y"
                 {
	#line 5981 "format.w"
	RNG("page priority",(yyvsp[0].u),0,255);HPUT8((yyvsp[0].u));}
#line 3281 "parser.c"
    break;

  case 179: /* $@8: %empty  */
#line 819 "parser.y"
           {
	#line 5985 "format.w"
	hput_string((yyvsp[0].s));}
#line 3289 "parser.c"
    break;

  case 180: /* $@9: %empty  */
#line 821 "parser.y"
                                                          {
	#line 5985 "format.w"
	HPUT32((yyvsp[0].d));}
#line 3297 "parser.c"
    break;

  case 182: /* content_node: "<" "range" REFERENCE "on" ">"  */
#line 828 "parser.y"
                                         {
	#line 6097 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),true);}
#line 3305 "parser.c"
    break;

  case 183: /* content_node: "<" "range" REFERENCE "off" ">"  */
#line 831 "parser.y"
                                      {
	#line 6098 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),false);}
#line 3313 "parser.c"
    break;

  case 185: /* $@10: %empty  */
#line 837 "parser.y"
                                          {
	#line 6789 "format.w"
	new_directory((yyvsp[0].u)+1);new_output_buffers();}
#line 3321 "parser.c"
    break;

  case 189: /* entry: "<" "entry" UNSIGNED string ">"  */
#line 842 "parser.y"
{
	#line 6792 "format.w"
	RNG("Section number",(yyvsp[-2].u),3,max_section_no);hset_entry(&(dir[(yyvsp[-2].u)]),(yyvsp[-2].u),0,0,(yyvsp[-1].s));}
#line 3329 "parser.c"
    break;

  case 190: /* $@11: %empty  */
#line 846 "parser.y"
                                    {
	#line 7327 "format.w"
	hput_definitions_start();}
#line 3337 "parser.c"
    break;

  case 191: /* definition_section: "<" "definitions" $@11 max_definitions definition_list ">"  */
#line 850 "parser.y"
   {
	#line 7329 "format.w"
	hput_definitions_end();}
#line 3345 "parser.c"
    break;

  case 194: /* max_definitions: "<" "max" max_list ">"  */
#line 856 "parser.y"
{
	#line 7445 "format.w"
		/*245:*/
	if(max_ref[label_kind]>=0)
	ALLOCATE(labels,max_ref[label_kind]+1,Label);
		/*:245*/	/*266:*/
	if(max_outline>=0)
	ALLOCATE(outlines,max_outline+1,Outline);
		/*:266*/	/*293:*/
	ALLOCATE(page_on,max_ref[page_kind]+1,int);
	ALLOCATE(range_pos,2*(max_ref[range_kind]+1),RangePos);
		/*:293*/	/*359:*/
	definition_bits[0][int_kind]= (1<<(MAX_INT_DEFAULT+1))-1;
	definition_bits[0][dimen_kind]= (1<<(MAX_DIMEN_DEFAULT+1))-1;
	definition_bits[0][xdimen_kind]= (1<<(MAX_XDIMEN_DEFAULT+1))-1;
	definition_bits[0][glue_kind]= (1<<(MAX_GLUE_DEFAULT+1))-1;
	definition_bits[0][baseline_kind]= (1<<(MAX_BASELINE_DEFAULT+1))-1;
	definition_bits[0][page_kind]= (1<<(MAX_PAGE_DEFAULT+1))-1;
	definition_bits[0][stream_kind]= (1<<(MAX_STREAM_DEFAULT+1))-1;
	definition_bits[0][range_kind]= (1<<(MAX_RANGE_DEFAULT+1))-1;
		/*:359*/	/*374:*/
	ALLOCATE(hfont_name,max_ref[font_kind]+1,char*);
		/*:374*/hput_max_definitions();}
#line 3373 "parser.c"
    break;

  case 197: /* max_value: "font" UNSIGNED  */
#line 882 "parser.y"
                       {
	#line 7449 "format.w"
	hset_max(font_kind,(yyvsp[0].u));}
#line 3381 "parser.c"
    break;

  case 198: /* max_value: "int" UNSIGNED  */
#line 885 "parser.y"
                         {
	#line 7450 "format.w"
	hset_max(int_kind,(yyvsp[0].u));}
#line 3389 "parser.c"
    break;

  case 199: /* max_value: "dimen" UNSIGNED  */
#line 888 "parser.y"
                       {
	#line 7451 "format.w"
	hset_max(dimen_kind,(yyvsp[0].u));}
#line 3397 "parser.c"
    break;

  case 200: /* max_value: "ligature" UNSIGNED  */
#line 891 "parser.y"
                          {
	#line 7452 "format.w"
	hset_max(ligature_kind,(yyvsp[0].u));}
#line 3405 "parser.c"
    break;

  case 201: /* max_value: "disc" UNSIGNED  */
#line 894 "parser.y"
                      {
	#line 7453 "format.w"
	hset_max(disc_kind,(yyvsp[0].u));}
#line 3413 "parser.c"
    break;

  case 202: /* max_value: "glue" UNSIGNED  */
#line 897 "parser.y"
                      {
	#line 7454 "format.w"
	hset_max(glue_kind,(yyvsp[0].u));}
#line 3421 "parser.c"
    break;

  case 203: /* max_value: "language" UNSIGNED  */
#line 900 "parser.y"
                          {
	#line 7455 "format.w"
	hset_max(language_kind,(yyvsp[0].u));}
#line 3429 "parser.c"
    break;

  case 204: /* max_value: "rule" UNSIGNED  */
#line 903 "parser.y"
                      {
	#line 7456 "format.w"
	hset_max(rule_kind,(yyvsp[0].u));}
#line 3437 "parser.c"
    break;

  case 205: /* max_value: "image" UNSIGNED  */
#line 906 "parser.y"
                       {
	#line 7457 "format.w"
	hset_max(image_kind,(yyvsp[0].u));}
#line 3445 "parser.c"
    break;

  case 206: /* max_value: "leaders" UNSIGNED  */
#line 909 "parser.y"
                         {
	#line 7458 "format.w"
	hset_max(leaders_kind,(yyvsp[0].u));}
#line 3453 "parser.c"
    break;

  case 207: /* max_value: "baseline" UNSIGNED  */
#line 912 "parser.y"
                          {
	#line 7459 "format.w"
	hset_max(baseline_kind,(yyvsp[0].u));}
#line 3461 "parser.c"
    break;

  case 208: /* max_value: "xdimen" UNSIGNED  */
#line 915 "parser.y"
                        {
	#line 7460 "format.w"
	hset_max(xdimen_kind,(yyvsp[0].u));}
#line 3469 "parser.c"
    break;

  case 209: /* max_value: "param" UNSIGNED  */
#line 918 "parser.y"
                       {
	#line 7461 "format.w"
	hset_max(param_kind,(yyvsp[0].u));}
#line 3477 "parser.c"
    break;

  case 210: /* max_value: "stream (definition)" UNSIGNED  */
#line 921 "parser.y"
                           {
	#line 7462 "format.w"
	hset_max(stream_kind,(yyvsp[0].u));}
#line 3485 "parser.c"
    break;

  case 211: /* max_value: "page" UNSIGNED  */
#line 924 "parser.y"
                      {
	#line 7463 "format.w"
	hset_max(page_kind,(yyvsp[0].u));}
#line 3493 "parser.c"
    break;

  case 212: /* max_value: "range" UNSIGNED  */
#line 927 "parser.y"
                       {
	#line 7464 "format.w"
	hset_max(range_kind,(yyvsp[0].u));}
#line 3501 "parser.c"
    break;

  case 213: /* max_value: "label" UNSIGNED  */
#line 930 "parser.y"
                       {
	#line 7465 "format.w"
	hset_max(label_kind,(yyvsp[0].u));}
#line 3509 "parser.c"
    break;

  case 214: /* def_node: start "font" ref font ">"  */
#line 936 "parser.y"
                       {
	#line 7662 "format.w"
	DEF((yyval.rf),font_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),(yyvsp[-1].info));}
#line 3517 "parser.c"
    break;

  case 215: /* def_node: start "int" ref integer ">"  */
#line 939 "parser.y"
                                      {
	#line 7663 "format.w"
	DEF((yyval.rf),int_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3525 "parser.c"
    break;

  case 216: /* def_node: start "dimen" ref dimension ">"  */
#line 942 "parser.y"
                                      {
	#line 7664 "format.w"
	DEF((yyval.rf),dimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_dimen((yyvsp[-1].d)));}
#line 3533 "parser.c"
    break;

  case 217: /* def_node: start "language" ref string ">"  */
#line 945 "parser.y"
                                      {
	#line 7665 "format.w"
	DEF((yyval.rf),language_kind,(yyvsp[-2].u));hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3541 "parser.c"
    break;

  case 218: /* def_node: start "glue" ref glue ">"  */
#line 948 "parser.y"
                                {
	#line 7666 "format.w"
	DEF((yyval.rf),glue_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3549 "parser.c"
    break;

  case 219: /* def_node: start "xdimen" ref xdimen ">"  */
#line 951 "parser.y"
                                    {
	#line 7667 "format.w"
	DEF((yyval.rf),xdimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 3557 "parser.c"
    break;

  case 220: /* def_node: start "rule" ref rule ">"  */
#line 954 "parser.y"
                                {
	#line 7668 "format.w"
	DEF((yyval.rf),rule_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3565 "parser.c"
    break;

  case 221: /* def_node: start "leaders" ref leaders ">"  */
#line 957 "parser.y"
                                      {
	#line 7669 "format.w"
	DEF((yyval.rf),leaders_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 3573 "parser.c"
    break;

  case 222: /* def_node: start "baseline" ref baseline ">"  */
#line 960 "parser.y"
                                        {
	#line 7670 "format.w"
	DEF((yyval.rf),baseline_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 3581 "parser.c"
    break;

  case 223: /* def_node: start "ligature" ref ligature ">"  */
#line 963 "parser.y"
                                        {
	#line 7671 "format.w"
	DEF((yyval.rf),ligature_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3589 "parser.c"
    break;

  case 224: /* def_node: start "disc" ref disc ">"  */
#line 966 "parser.y"
                                {
	#line 7672 "format.w"
	DEF((yyval.rf),disc_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3597 "parser.c"
    break;

  case 225: /* def_node: start "image" ref image ">"  */
#line 969 "parser.y"
                                  {
	#line 7673 "format.w"
	DEF((yyval.rf),image_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_image(&((yyvsp[-1].x))));}
#line 3605 "parser.c"
    break;

  case 226: /* def_node: start "param" ref parameters ">"  */
#line 972 "parser.y"
                                       {
	#line 7674 "format.w"
	DEF((yyval.rf),param_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_list((yyvsp[-4].u)+2,&((yyvsp[-1].l))));}
#line 3613 "parser.c"
    break;

  case 227: /* def_node: start "page" ref page ">"  */
#line 975 "parser.y"
                                {
	#line 7675 "format.w"
	DEF((yyval.rf),page_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(page_kind,0));}
#line 3621 "parser.c"
    break;

  case 228: /* def_node: start "int" ref ref ">"  */
#line 980 "parser.y"
                         {
	#line 7694 "format.w"
	DEF_REF((yyval.rf),int_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(int_kind,0));}
#line 3629 "parser.c"
    break;

  case 229: /* def_node: start "dimen" ref ref ">"  */
#line 983 "parser.y"
                                {
	#line 7695 "format.w"
	DEF_REF((yyval.rf),dimen_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(dimen_kind,0));}
#line 3637 "parser.c"
    break;

  case 230: /* def_node: start "glue" ref ref ">"  */
#line 986 "parser.y"
                               {
	#line 7696 "format.w"
	DEF_REF((yyval.rf),glue_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(glue_kind,0));}
#line 3645 "parser.c"
    break;

  case 232: /* def_list: def_list def_node  */
#line 991 "parser.y"
                          {
	#line 7810 "format.w"
	check_param_def(&((yyvsp[0].rf)));}
#line 3653 "parser.c"
    break;

  case 233: /* parameters: estimate def_list  */
#line 994 "parser.y"
                            {
	#line 7811 "format.w"
	(yyval.l).p= (yyvsp[0].u);(yyval.l).k= param_kind;(yyval.l).s= (hpos-hstart)-(yyvsp[0].u);}
#line 3661 "parser.c"
    break;

  case 234: /* empty_param_list: position  */
#line 998 "parser.y"
                         {
	#line 7832 "format.w"
	HPUTX(2);hpos++;hput_tags((yyvsp[0].u),TAG(param_kind,1));}
#line 3669 "parser.c"
    break;

  case 235: /* $@12: %empty  */
#line 1001 "parser.y"
                                {
	#line 7833 "format.w"
	hpos= hpos-2;}
#line 3677 "parser.c"
    break;

  case 236: /* non_empty_param_list: start "param" $@12 parameters ">"  */
#line 1004 "parser.y"
{
	#line 7834 "format.w"
	hput_tags((yyvsp[-4].u)-2,hput_list((yyvsp[-4].u)-1,&((yyvsp[-1].l))));}
#line 3685 "parser.c"
    break;

  case 238: /* font_head: string dimension UNSIGNED UNSIGNED  */
#line 1012 "parser.y"
{
	#line 7976 "format.w"
	uint8_t f= (yyvsp[-4].u);SET_DBIT(f,font_kind);hfont_name[f]= strdup((yyvsp[-3].s));(yyval.info)= hput_font_head(f,hfont_name[f],(yyvsp[-2].d),(yyvsp[-1].u),(yyvsp[0].u));}
#line 3693 "parser.c"
    break;

  case 241: /* font_param: start "penalty" fref penalty ">"  */
#line 1019 "parser.y"
                              {
	#line 7981 "format.w"
	hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3701 "parser.c"
    break;

  case 242: /* font_param: start "kern" fref kern ">"  */
#line 1022 "parser.y"
                                 {
	#line 7982 "format.w"
	hput_tags((yyvsp[-4].u),hput_kern(&((yyvsp[-1].kt))));}
#line 3709 "parser.c"
    break;

  case 243: /* font_param: start "ligature" fref ligature ">"  */
#line 1025 "parser.y"
                                         {
	#line 7983 "format.w"
	hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3717 "parser.c"
    break;

  case 244: /* font_param: start "disc" fref disc ">"  */
#line 1028 "parser.y"
                                 {
	#line 7984 "format.w"
	hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3725 "parser.c"
    break;

  case 245: /* font_param: start "glue" fref glue ">"  */
#line 1031 "parser.y"
                                 {
	#line 7985 "format.w"
	hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3733 "parser.c"
    break;

  case 246: /* font_param: start "language" fref string ">"  */
#line 1034 "parser.y"
                                       {
	#line 7986 "format.w"
	hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3741 "parser.c"
    break;

  case 247: /* font_param: start "rule" fref rule ">"  */
#line 1037 "parser.y"
                                 {
	#line 7987 "format.w"
	hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3749 "parser.c"
    break;

  case 248: /* font_param: start "image" fref image ">"  */
#line 1040 "parser.y"
                                   {
	#line 7988 "format.w"
	hput_tags((yyvsp[-4].u),hput_image(&((yyvsp[-1].x))));}
#line 3757 "parser.c"
    break;

  case 249: /* fref: ref  */
#line 1044 "parser.y"
        {
	#line 7990 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,MAX_FONT_PARAMS);}
#line 3765 "parser.c"
    break;

  case 250: /* xdimen_ref: ref  */
#line 1048 "parser.y"
              {
	#line 8063 "format.w"
	REF(xdimen_kind,(yyvsp[0].u));}
#line 3773 "parser.c"
    break;

  case 251: /* param_ref: ref  */
#line 1051 "parser.y"
             {
	#line 8064 "format.w"
	REF(param_kind,(yyvsp[0].u));}
#line 3781 "parser.c"
    break;

  case 252: /* stream_ref: ref  */
#line 1054 "parser.y"
              {
	#line 8065 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3789 "parser.c"
    break;

  case 253: /* content_node: start "penalty" ref ">"  */
#line 1060 "parser.y"
                     {
	#line 8069 "format.w"
	REF(penalty_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(penalty_kind,0));}
#line 3797 "parser.c"
    break;

  case 254: /* content_node: start "kern" explicit ref ">"  */
#line 1064 "parser.y"
{
	#line 8071 "format.w"
	REF(dimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(kern_kind,((yyvsp[-2].b))?b100:b000));}
#line 3805 "parser.c"
    break;

  case 255: /* content_node: start "kern" explicit "xdimen" ref ">"  */
#line 1068 "parser.y"
{
	#line 8073 "format.w"
	REF(xdimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-5].u),TAG(kern_kind,((yyvsp[-3].b))?b101:b001));}
#line 3813 "parser.c"
    break;

  case 256: /* content_node: start "glue" ref ">"  */
#line 1071 "parser.y"
                           {
	#line 8074 "format.w"
	REF(glue_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}
#line 3821 "parser.c"
    break;

  case 257: /* content_node: start "ligature" ref ">"  */
#line 1074 "parser.y"
                               {
	#line 8075 "format.w"
	REF(ligature_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(ligature_kind,0));}
#line 3829 "parser.c"
    break;

  case 258: /* content_node: start "disc" ref ">"  */
#line 1077 "parser.y"
                           {
	#line 8076 "format.w"
	REF(disc_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(disc_kind,0));}
#line 3837 "parser.c"
    break;

  case 259: /* content_node: start "rule" ref ">"  */
#line 1080 "parser.y"
                           {
	#line 8077 "format.w"
	REF(rule_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(rule_kind,0));}
#line 3845 "parser.c"
    break;

  case 260: /* content_node: start "image" ref ">"  */
#line 1083 "parser.y"
                            {
	#line 8078 "format.w"
	REF(image_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(image_kind,0));}
#line 3853 "parser.c"
    break;

  case 261: /* content_node: start "leaders" ref ">"  */
#line 1086 "parser.y"
                              {
	#line 8079 "format.w"
	REF(leaders_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(leaders_kind,0));}
#line 3861 "parser.c"
    break;

  case 262: /* content_node: start "baseline" ref ">"  */
#line 1089 "parser.y"
                               {
	#line 8080 "format.w"
	REF(baseline_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(baseline_kind,0));}
#line 3869 "parser.c"
    break;

  case 263: /* content_node: start "language" REFERENCE ">"  */
#line 1092 "parser.y"
                                     {
	#line 8081 "format.w"
	REF(language_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),hput_language((yyvsp[-1].u)));}
#line 3877 "parser.c"
    break;

  case 264: /* glue_node: start "glue" ref ">"  */
#line 1096 "parser.y"
                            {
	#line 8083 "format.w"
	REF(glue_kind,(yyvsp[-1].u));
	if((yyvsp[-1].u)==zero_skip_no){hpos= hpos-2;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),TAG(glue_kind,0));(yyval.b)= true;}}
#line 3887 "parser.c"
    break;

  case 265: /* $@13: %empty  */
#line 1103 "parser.y"
                             {
	#line 8514 "format.w"
	hput_content_start();}
#line 3895 "parser.c"
    break;

  case 266: /* content_section: "<" "content" $@13 content_list ">"  */
#line 1106 "parser.y"
{
	#line 8515 "format.w"
	hput_content_end();hput_range_defs();hput_label_defs();}
#line 3903 "parser.c"
    break;


#line 3907 "parser.c"

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

#line 1110 "parser.y"

	/*:512*/
