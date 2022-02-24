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

	#line 10763 "format.w"
	
#include "hibasetypes.h"
#include <string.h>
#include <math.h>
#include "hierror.h"
#include "hiformat.h"
#include "hiput.h"
extern char**hfont_name;

	/*366:*/
uint32_t definition_bits[0x100/32][32]= {
	#line 8067 "format.w"
	{0}};

#define SET_DBIT(N,K) ((N)>0xFF?1:(definition_bits[N/32][K]	|= (1<<((N)&(32-1)))))
#define GET_DBIT(N,K) ((N)>0xFF?1:((definition_bits[N/32][K]>>((N)&(32-1)))&1))
#define DEF(D,K,N) (D).k= K; (D).n= (N);SET_DBIT((D).n,(D).k);\
 DBG(DBGDEF,"Defining %s %d\n",definition_name[(D).k],(D).n);\
 RNG("Definition",(D).n,max_fixed[(D).k]+1,max_ref[(D).k]);
#define REF(K,N) REF_RNG(K,N);if(!GET_DBIT(N,K)) \
 QUIT("Reference %d to %s before definition",(N),definition_name[K])
	/*:366*/	/*370:*/
#define DEF_REF(D,K,M,N)  DEF(D,K,M);\
if ((int)(M)>max_default[K]) QUIT("Defining non default reference %d for %s",M,definition_name[K]); \
if ((int)(N)>max_fixed[K]) QUIT("Defining reference %d for %s by non fixed reference %d",M,definition_name[K],N);
	/*:370*/

extern void hset_entry(Entry*e,uint16_t i,uint32_t size,
uint32_t xsize,char*file_name);

	/*433:*/
#ifdef DEBUG
#define  YYDEBUG 1
extern int yydebug;
#else
#define YYDEBUG 0
#endif
	/*:433*/
extern int yylex(void);

	/*362:*/
void hset_max(Kind k,int n)
{
	#line 7910 "format.w"
	DBG(DBGDEF,"Setting max %s to %d\n",definition_name[k],n);
	RNG("Maximum",n,max_fixed[k]+1,MAX_REF(k));
	if(n>max_ref[k])
	max_ref[k]= n;
	}
	/*:362*/	/*373:*/
void check_param_def(Ref*df)
{
	#line 8216 "format.w"
	if(df->k!=int_kind&&df->k!=dimen_kind&&df->k!=glue_kind)
	QUIT("Kind %s not allowed in parameter list",definition_name[df->k]);
	if(df->n<=max_fixed[df->k]||max_default[df->k]<df->n)
	QUIT("Parameter %d for %s not allowed in parameter list",df->n,definition_name[df->k]);
	}
	/*:373*/	/*432:*/
extern int yylineno;
int yyerror(const char*msg)
{
	#line 9337 "format.w"
	QUIT(" in line %d %s",yylineno,msg);
	return 0;
	}
	/*:432*/



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
  YYSYMBOL_STREAM = 73,                    /* "stream"  */
  YYSYMBOL_STREAMDEF = 74,                 /* "stream (definition)"  */
  YYSYMBOL_FIRST = 75,                     /* "first"  */
  YYSYMBOL_LAST = 76,                      /* "last"  */
  YYSYMBOL_TOP = 77,                       /* "top"  */
  YYSYMBOL_NOREFERENCE = 78,               /* "*"  */
  YYSYMBOL_PAGE = 79,                      /* "page"  */
  YYSYMBOL_RANGE = 80,                     /* "range"  */
  YYSYMBOL_DIRECTORY = 81,                 /* "directory"  */
  YYSYMBOL_SECTION = 82,                   /* "entry"  */
  YYSYMBOL_DEFINITIONS = 83,               /* "definitions"  */
  YYSYMBOL_MAX = 84,                       /* "max"  */
  YYSYMBOL_PARAM = 85,                     /* "param"  */
  YYSYMBOL_FONT = 86,                      /* "font"  */
  YYSYMBOL_CONTENT = 87,                   /* "content"  */
  YYSYMBOL_YYACCEPT = 88,                  /* $accept  */
  YYSYMBOL_glyph = 89,                     /* glyph  */
  YYSYMBOL_content_node = 90,              /* content_node  */
  YYSYMBOL_start = 91,                     /* start  */
  YYSYMBOL_integer = 92,                   /* integer  */
  YYSYMBOL_string = 93,                    /* string  */
  YYSYMBOL_number = 94,                    /* number  */
  YYSYMBOL_dimension = 95,                 /* dimension  */
  YYSYMBOL_xdimen = 96,                    /* xdimen  */
  YYSYMBOL_xdimen_node = 97,               /* xdimen_node  */
  YYSYMBOL_order = 98,                     /* order  */
  YYSYMBOL_stretch = 99,                   /* stretch  */
  YYSYMBOL_penalty = 100,                  /* penalty  */
  YYSYMBOL_rule_dimension = 101,           /* rule_dimension  */
  YYSYMBOL_rule = 102,                     /* rule  */
  YYSYMBOL_rule_node = 103,                /* rule_node  */
  YYSYMBOL_explicit = 104,                 /* explicit  */
  YYSYMBOL_kern = 105,                     /* kern  */
  YYSYMBOL_plus = 106,                     /* plus  */
  YYSYMBOL_minus = 107,                    /* minus  */
  YYSYMBOL_glue = 108,                     /* glue  */
  YYSYMBOL_glue_node = 109,                /* glue_node  */
  YYSYMBOL_position = 110,                 /* position  */
  YYSYMBOL_content_list = 111,             /* content_list  */
  YYSYMBOL_estimate = 112,                 /* estimate  */
  YYSYMBOL_list = 113,                     /* list  */
  YYSYMBOL_114_1 = 114,                    /* $@1  */
  YYSYMBOL_text = 115,                     /* text  */
  YYSYMBOL_txt = 116,                      /* txt  */
  YYSYMBOL_117_2 = 117,                    /* $@2  */
  YYSYMBOL_box_dimen = 118,                /* box_dimen  */
  YYSYMBOL_box_shift = 119,                /* box_shift  */
  YYSYMBOL_box_glue_set = 120,             /* box_glue_set  */
  YYSYMBOL_box = 121,                      /* box  */
  YYSYMBOL_hbox_node = 122,                /* hbox_node  */
  YYSYMBOL_vbox_node = 123,                /* vbox_node  */
  YYSYMBOL_box_flex = 124,                 /* box_flex  */
  YYSYMBOL_xbox = 125,                     /* xbox  */
  YYSYMBOL_box_goal = 126,                 /* box_goal  */
  YYSYMBOL_hpack = 127,                    /* hpack  */
  YYSYMBOL_vpack = 128,                    /* vpack  */
  YYSYMBOL_129_3 = 129,                    /* $@3  */
  YYSYMBOL_vxbox_node = 130,               /* vxbox_node  */
  YYSYMBOL_hxbox_node = 131,               /* hxbox_node  */
  YYSYMBOL_ltype = 132,                    /* ltype  */
  YYSYMBOL_leaders = 133,                  /* leaders  */
  YYSYMBOL_baseline = 134,                 /* baseline  */
  YYSYMBOL_135_4 = 135,                    /* $@4  */
  YYSYMBOL_cc_list = 136,                  /* cc_list  */
  YYSYMBOL_lig_cc = 137,                   /* lig_cc  */
  YYSYMBOL_ref = 138,                      /* ref  */
  YYSYMBOL_ligature = 139,                 /* ligature  */
  YYSYMBOL_140_5 = 140,                    /* $@5  */
  YYSYMBOL_replace_count = 141,            /* replace_count  */
  YYSYMBOL_disc = 142,                     /* disc  */
  YYSYMBOL_disc_node = 143,                /* disc_node  */
  YYSYMBOL_par_dimen = 144,                /* par_dimen  */
  YYSYMBOL_par = 145,                      /* par  */
  YYSYMBOL_146_6 = 146,                    /* $@6  */
  YYSYMBOL_math = 147,                     /* math  */
  YYSYMBOL_on_off = 148,                   /* on_off  */
  YYSYMBOL_span_count = 149,               /* span_count  */
  YYSYMBOL_table = 150,                    /* table  */
  YYSYMBOL_image_aspect = 151,             /* image_aspect  */
  YYSYMBOL_image_width = 152,              /* image_width  */
  YYSYMBOL_image_height = 153,             /* image_height  */
  YYSYMBOL_image_spec = 154,               /* image_spec  */
  YYSYMBOL_image = 155,                    /* image  */
  YYSYMBOL_max_value = 156,                /* max_value  */
  YYSYMBOL_placement = 157,                /* placement  */
  YYSYMBOL_def_node = 158,                 /* def_node  */
  YYSYMBOL_stream_link = 159,              /* stream_link  */
  YYSYMBOL_stream_split = 160,             /* stream_split  */
  YYSYMBOL_stream_info = 161,              /* stream_info  */
  YYSYMBOL_162_7 = 162,                    /* $@7  */
  YYSYMBOL_stream_type = 163,              /* stream_type  */
  YYSYMBOL_stream_def_node = 164,          /* stream_def_node  */
  YYSYMBOL_stream_ins_node = 165,          /* stream_ins_node  */
  YYSYMBOL_stream = 166,                   /* stream  */
  YYSYMBOL_page_priority = 167,            /* page_priority  */
  YYSYMBOL_stream_def_list = 168,          /* stream_def_list  */
  YYSYMBOL_page = 169,                     /* page  */
  YYSYMBOL_170_8 = 170,                    /* $@8  */
  YYSYMBOL_171_9 = 171,                    /* $@9  */
  YYSYMBOL_hint = 172,                     /* hint  */
  YYSYMBOL_directory_section = 173,        /* directory_section  */
  YYSYMBOL_174_10 = 174,                   /* $@10  */
  YYSYMBOL_entry_list = 175,               /* entry_list  */
  YYSYMBOL_entry = 176,                    /* entry  */
  YYSYMBOL_definition_section = 177,       /* definition_section  */
  YYSYMBOL_178_11 = 178,                   /* $@11  */
  YYSYMBOL_definition_list = 179,          /* definition_list  */
  YYSYMBOL_max_definitions = 180,          /* max_definitions  */
  YYSYMBOL_max_list = 181,                 /* max_list  */
  YYSYMBOL_def_list = 182,                 /* def_list  */
  YYSYMBOL_parameters = 183,               /* parameters  */
  YYSYMBOL_empty_param_list = 184,         /* empty_param_list  */
  YYSYMBOL_non_empty_param_list = 185,     /* non_empty_param_list  */
  YYSYMBOL_186_12 = 186,                   /* $@12  */
  YYSYMBOL_font = 187,                     /* font  */
  YYSYMBOL_font_head = 188,                /* font_head  */
  YYSYMBOL_font_param_list = 189,          /* font_param_list  */
  YYSYMBOL_font_param = 190,               /* font_param  */
  YYSYMBOL_fref = 191,                     /* fref  */
  YYSYMBOL_xdimen_ref = 192,               /* xdimen_ref  */
  YYSYMBOL_param_ref = 193,                /* param_ref  */
  YYSYMBOL_stream_ref = 194,               /* stream_ref  */
  YYSYMBOL_content_section = 195,          /* content_section  */
  YYSYMBOL_196_13 = 196                    /* $@13  */
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
#define YYLAST   642

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  109
/* YYNRULES -- Number of rules.  */
#define YYNRULES  274
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  576

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   342


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
      85,    86,    87
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   268,   268,   271,   274,   278,   278,   282,   286,   286,
     292,   294,   296,   298,   301,   304,   308,   311,   314,   317,
     321,   326,   328,   330,   332,   336,   340,   343,   347,   347,
     350,   356,   359,   361,   363,   366,   369,   373,   375,   378,
     380,   383,   386,   390,   396,   399,   400,   401,   404,   407,
     414,   413,   422,   422,   424,   427,   430,   433,   436,   439,
     442,   445,   445,   450,   454,   457,   461,   464,   467,   472,
     476,   479,   482,   482,   484,   487,   490,   494,   497,   500,
     503,   507,   510,   510,   516,   519,   524,   527,   531,   531,
     533,   535,   537,   539,   542,   545,   548,   551,   555,   555,
     564,   569,   569,   572,   575,   578,   581,   581,   588,   592,
     595,   599,   603,   606,   611,   616,   618,   621,   624,   627,
     630,   630,   635,   638,   642,   646,   649,   652,   655,   658,
     661,   664,   667,   670,   674,   678,   680,   683,   687,   691,
     694,   697,   700,   704,   707,   711,   715,   717,   720,   723,
     726,   729,   733,   737,   741,   745,   750,   754,   758,   765,
     767,   769,   771,   774,   779,   784,   794,   796,   799,   802,
     802,   806,   808,   810,   812,   816,   822,   827,   827,   829,
     832,   835,   838,   843,   846,   850,   850,   852,   854,   852,
     861,   864,   868,   870,   870,   873,   873,   874,   879,   879,
     886,   886,   888,   913,   913,   915,   918,   921,   924,   927,
     930,   933,   936,   939,   942,   945,   948,   951,   954,   957,
     960,   963,   969,   972,   975,   978,   981,   984,   987,   990,
     993,   996,   999,  1002,  1005,  1008,  1013,  1016,  1019,  1023,
    1024,  1027,  1031,  1034,  1034,  1042,  1044,  1049,  1049,  1052,
    1055,  1058,  1061,  1064,  1067,  1070,  1073,  1077,  1081,  1084,
    1087,  1093,  1096,  1100,  1104,  1107,  1110,  1113,  1116,  1119,
    1122,  1125,  1129,  1136,  1136
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
  "\"bot\"", "\"mid\"", "\"link\"", "\"outline\"", "\"stream\"",
  "\"stream (definition)\"", "\"first\"", "\"last\"", "\"top\"", "\"*\"",
  "\"page\"", "\"range\"", "\"directory\"", "\"entry\"", "\"definitions\"",
  "\"max\"", "\"param\"", "\"font\"", "\"content\"", "$accept", "glyph",
  "content_node", "start", "integer", "string", "number", "dimension",
  "xdimen", "xdimen_node", "order", "stretch", "penalty", "rule_dimension",
  "rule", "rule_node", "explicit", "kern", "plus", "minus", "glue",
  "glue_node", "position", "content_list", "estimate", "list", "$@1",
  "text", "txt", "$@2", "box_dimen", "box_shift", "box_glue_set", "box",
  "hbox_node", "vbox_node", "box_flex", "xbox", "box_goal", "hpack",
  "vpack", "$@3", "vxbox_node", "hxbox_node", "ltype", "leaders",
  "baseline", "$@4", "cc_list", "lig_cc", "ref", "ligature", "$@5",
  "replace_count", "disc", "disc_node", "par_dimen", "par", "$@6", "math",
  "on_off", "span_count", "table", "image_aspect", "image_width",
  "image_height", "image_spec", "image", "max_value", "placement",
  "def_node", "stream_link", "stream_split", "stream_info", "$@7",
  "stream_type", "stream_def_node", "stream_ins_node", "stream",
  "page_priority", "stream_def_list", "page", "$@8", "$@9", "hint",
  "directory_section", "$@10", "entry_list", "entry", "definition_section",
  "$@11", "definition_list", "max_definitions", "max_list", "def_list",
  "parameters", "empty_param_list", "non_empty_param_list", "$@12", "font",
  "font_head", "font_param_list", "font_param", "fref", "xdimen_ref",
  "param_ref", "stream_ref", "content_section", "$@13", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-242)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      55,    10,   108,   129,   135,  -242,    81,   170,  -242,  -242,
     104,  -242,  -242,   191,  -242,   209,   113,  -242,  -242,   140,
    -242,  -242,  -242,   216,  -242,   227,   220,   238,   177,  -242,
      59,  -242,    -2,  -242,  -242,   568,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,   280,   391,  -242,   252,   255,   255,
     255,   255,   255,   255,   255,   255,   255,   255,   255,   255,
     255,   255,   257,   260,    75,   278,   264,   175,   246,   217,
     164,   164,   256,   164,   256,   164,   110,   217,   255,    70,
     217,    88,    94,   167,    87,   300,   286,   255,   255,  -242,
    -242,   299,   310,   313,   321,   339,   341,   343,   344,   345,
     349,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     342,   115,  -242,   217,   164,   278,   280,   181,   217,   364,
     164,   255,   246,   362,   280,   363,   280,    90,   254,   365,
     366,   367,  -242,  -242,  -242,   370,   371,   377,  -242,  -242,
    -242,  -242,   297,  -242,   181,   379,   382,  -242,   198,   383,
     164,   361,   384,   386,   164,   256,   389,   392,   164,   275,
     393,   256,   394,   315,   397,   400,  -242,   380,   269,   402,
     404,  -242,   406,   407,   408,   409,   411,   415,    94,   418,
     255,  -242,  -242,   419,   255,  -242,  -242,  -242,  -242,   420,
    -242,    94,    94,  -242,   363,   421,   275,   275,   422,  -242,
     423,   513,   427,   430,   164,   434,    94,   436,   277,  -242,
     255,    76,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,   437,   441,   445,   446,   447,   448,   449,
     450,   453,   454,   457,  -242,   458,   463,   464,  -242,   465,
    -242,  -242,   468,   164,   469,   364,  -242,  -242,  -242,   470,
     471,   474,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,   181,  -242,  -242,   255,  -242,   475,  -242,   322,   164,
     455,  -242,  -242,   164,   311,  -242,  -242,  -242,   110,   110,
      94,  -242,   361,  -242,   433,  -242,  -242,   217,  -242,  -242,
    -242,   364,  -242,  -242,   364,  -242,  -242,  -242,   193,  -242,
    -242,  -242,    94,  -242,  -242,    94,  -242,    94,    94,  -242,
      45,   364,    94,    94,    51,   364,    94,  -242,  -242,  -242,
      94,    94,  -242,  -242,  -242,   480,  -242,   304,  -242,  -242,
    -242,   481,   483,    94,    94,  -242,  -242,  -242,  -242,   472,
     485,  -242,    94,    94,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,   487,  -242,
    -242,   486,  -242,   489,  -242,   364,   364,  -242,  -242,  -242,
    -242,   493,  -242,   164,  -242,   197,  -242,   164,  -242,  -242,
     164,   164,    94,  -242,  -242,  -242,  -242,  -242,   455,   110,
     164,   494,   495,   154,  -242,  -242,  -242,   364,  -242,  -242,
     473,  -242,    94,    46,  -242,    94,  -242,    94,  -242,  -242,
     459,  -242,  -242,   364,    94,  -242,  -242,  -242,   340,    94,
      94,  -242,   240,   435,  -242,  -242,  -242,    94,  -242,   164,
    -242,   364,   497,  -242,   364,  -242,   498,   451,  -242,    77,
    -242,  -242,   488,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,    94,    94,  -242,  -242,  -242,   181,  -242,
    -242,  -242,  -242,  -242,   363,  -242,  -242,  -242,   200,  -242,
    -242,  -242,   440,  -242,   250,  -242,  -242,   505,    47,   364,
    -242,   164,  -242,   246,   255,   255,   255,   255,   255,   255,
     255,   255,  -242,  -242,  -242,   275,   107,   506,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,   430,   262,  -242,
    -242,  -242,  -242,  -242,  -242,    47,  -242,    94,  -242,  -242,
     115,   280,   181,   246,   164,   255,   246,   362,    94,  -242,
    -242,  -242,  -242,  -242,   507,   364,   364,   508,   510,   511,
     164,   512,   516,   517,   521,   522,  -242,  -242,   523,   364,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,    94,
    -242,   364,   443,  -242,   255,    86
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     0,     0,     0,     0,     1,     0,     0,   193,   198,
       0,   192,   195,     0,   273,     0,     0,   200,    44,     0,
     194,   196,   203,     0,    45,     0,     0,     0,     4,   199,
       0,   201,     4,   274,    46,     0,    32,    72,    73,    88,
      89,   115,   177,   178,     0,     0,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,    64,     0,    64,     0,     0,     0,     0,    33,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     8,
       9,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,     0,    47,     0,   162,     0,     0,
       0,     0,     6,     5,    26,     0,     0,     0,    10,    11,
      12,    29,     0,    28,     0,     0,     0,    34,     0,     0,
      19,    37,     0,     0,     0,    64,     0,     0,     0,     0,
       0,    64,     0,     0,     0,     0,     4,     0,    90,     0,
       0,    98,     0,     0,   106,     0,   109,     0,   113,     0,
     116,   258,    44,     0,    44,   135,   136,   242,   259,     0,
     137,     0,     0,    44,    47,     0,     0,     0,     0,   139,
       0,    47,     0,     0,   147,     0,     0,     0,     0,   260,
      44,     0,   197,   207,   216,   206,   211,   212,   210,   214,
     215,   208,   209,   213,   221,   158,   218,   219,   220,   217,
     205,   204,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   106,     0,     0,     0,   187,     0,
      48,    44,     0,     0,     0,     0,   160,   161,   159,     0,
       0,     0,     2,     7,     3,    27,   261,   271,    13,    15,
      14,     0,    31,   267,     0,    35,     0,    36,     0,     0,
      39,    42,   264,     0,    66,    70,    71,    65,     0,     0,
       0,    87,    37,    86,     0,    85,    84,     0,    91,    92,
      93,     0,    97,   269,     0,   100,   270,   265,     0,   108,
     110,   266,   112,   114,   120,     0,   124,     0,     0,   134,
      47,   128,     0,     0,    47,   125,     0,    50,    44,   138,
       0,     0,   145,   140,   142,     0,   146,   149,   268,   156,
     157,     0,     0,     0,     0,   176,   172,   173,   174,     0,
       0,   171,     0,     0,   224,   237,   227,   223,   236,   225,
     228,   226,   238,   229,   230,   231,   232,   233,   183,   235,
     239,   241,   234,     0,   222,     0,   245,   163,   190,   191,
      30,     0,   262,    17,    18,     0,    38,     0,    41,    63,
       0,     0,     0,    80,    78,    79,    77,    81,    39,     0,
       0,     0,     0,     0,    94,    95,    96,     0,   103,   104,
       0,   111,     0,    47,   123,     0,   119,     0,   117,   243,
       0,   129,   130,   131,     0,   126,   127,    44,     0,     0,
       0,   141,     0,   151,   164,   182,   179,     0,   181,     0,
     169,     0,     0,   184,     0,   240,     0,     0,   247,     0,
     248,   263,     0,    21,    22,    23,    24,    25,    40,    67,
      68,    69,    74,     0,     0,    82,    43,   272,     0,    99,
     101,   121,   122,   118,    47,   132,   133,    52,    61,    49,
     143,   144,   151,   148,     0,   152,   180,     0,     0,     0,
     165,     0,   246,    33,     0,     0,     0,     0,     0,     0,
       0,     0,    16,    76,    75,     0,     0,     0,    51,    60,
      58,    59,    55,    57,    56,    54,    53,     0,     0,   153,
     154,   150,    20,   167,   166,     0,   170,     0,   188,   257,
       0,     0,     0,    33,     0,     0,    33,     0,     0,   107,
     102,   244,    62,   155,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,   168,     0,     0,
     249,   254,   255,   250,   253,   251,   252,   256,   175,     0,
     185,   189,     0,   186,     0,     0
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -242,  -242,   -83,   -23,   100,   -87,   -86,    -8,   -54,  -233,
    -242,  -137,   -19,   -94,   -96,   229,   -51,    -5,   239,   134,
     -99,  -217,    28,   205,   -95,   -35,  -242,  -242,  -242,  -242,
     127,   -33,  -242,   476,  -147,   242,  -242,   466,  -169,  -242,
    -242,  -242,  -242,  -242,  -242,   425,   426,  -242,  -242,  -242,
     -45,  -103,  -242,  -242,   -97,   174,  -242,  -242,  -242,  -242,
     331,  -242,  -242,  -242,  -242,    63,  -242,  -100,  -242,  -242,
     179,    26,  -242,  -242,  -242,  -242,   -18,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,    82,  -110,  -188,  -242,  -242,
    -242,  -242,  -242,  -166,  -241,   -73,  -242,  -242,  -242
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   131,    34,   194,   134,    91,   142,   150,   151,   350,
     457,   386,   135,   144,   145,    36,   176,   149,   280,   388,
     152,   168,   187,    25,   328,   195,   427,   478,   516,   517,
     155,   159,   392,   156,    37,    38,   399,   162,   290,   160,
     164,   505,    39,    40,   301,   169,   172,   304,   506,   410,
     529,   175,   308,   178,   179,    41,   182,   183,   412,   189,
     190,   203,   198,   337,   433,   485,   206,   207,   110,   259,
      31,   525,   526,   351,   488,   352,    42,    43,   342,   444,
     571,   249,   368,   546,     2,     3,    12,    15,    21,     7,
      13,    23,    17,    27,   371,   252,   191,   323,   474,   254,
     255,   376,   450,   530,   184,   192,   210,    11,    18
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,   200,    35,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   148,   245,   240,
     136,   239,   146,   247,   153,   246,   180,   330,   331,   238,
     251,   170,   173,   174,   177,   181,   188,   248,   375,   253,
     205,   163,   209,   211,   322,   326,    24,   394,   396,   202,
     271,   250,   250,   167,   112,   393,   395,   250,     1,   143,
     235,   201,   154,   154,   278,   154,    62,   154,   234,   171,
     237,    48,   315,   241,   317,    49,   244,   112,    63,   166,
     345,   129,    50,    51,    52,   130,    70,   407,    53,   166,
      32,     4,    70,   199,   275,   112,   167,   166,   147,   494,
     343,   495,   496,   276,   497,   233,   498,   314,     5,   143,
      54,   318,   171,   166,    55,    56,    57,   112,   336,   193,
     335,   132,   284,   133,    58,   523,   193,   415,   292,   417,
     419,   419,     6,   499,   500,   188,   143,   344,    59,   188,
     539,     8,   501,   312,    60,    61,   283,   540,   185,   186,
     287,   346,   347,   348,   405,   437,   321,   325,   464,   256,
     257,   346,   347,   348,     9,   188,   463,   258,   320,   324,
     138,   339,   139,    10,   421,   140,   424,   380,   425,   468,
      35,   138,   112,   139,   196,   197,   140,   138,   349,   139,
     469,    14,   140,   385,    16,    70,    71,    22,   401,   408,
     161,   141,   161,   409,   138,   112,   139,   141,   489,   140,
     453,   232,    19,    20,   274,   236,   454,   455,   456,    28,
      29,   327,    26,   138,   112,   139,    44,   491,   140,   381,
      32,    33,   167,   508,   509,   510,   511,   512,   513,   514,
     515,    45,    46,   181,   181,   373,   138,   482,   139,    47,
     458,   140,   402,   459,   460,   397,   138,   520,   139,   111,
     353,   140,   112,   143,   127,   349,   349,   128,   138,   543,
     139,   137,   527,   140,   147,   389,   475,   411,   403,   370,
     414,   167,   416,   418,   132,   112,   133,   422,   423,    89,
      90,   426,   413,   208,   413,   429,   430,   452,   420,   158,
     324,   385,   420,   212,   385,   385,   204,   112,   436,   438,
     268,   269,   270,   559,   260,   261,   213,   441,   442,   214,
     413,   298,   299,   300,   288,   289,   569,   215,   558,   531,
     532,   533,   534,   535,   536,   537,   538,   185,   186,   383,
     384,   390,   391,    32,   479,   216,   231,   217,    30,   218,
     219,   220,   447,   449,   181,   221,    24,   461,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   166,   204,   250,
     432,   264,   262,   263,   265,   266,   349,   471,   483,   251,
     472,   267,   473,   272,   167,   487,   273,   277,   281,   476,
     282,   279,   465,   285,   480,   481,   286,   291,   293,   294,
     420,   295,   486,    92,   296,    35,   302,    93,   303,   297,
     305,   306,   307,   309,    94,    95,    96,   310,   349,   311,
      97,   167,   313,   316,   319,   329,   332,   333,   503,   504,
     521,   334,   553,    32,   542,   552,   549,   555,   338,   554,
     340,   354,    98,   524,   548,   355,    99,   100,   101,   356,
     357,   358,   359,   360,   361,   477,   102,   362,   363,   103,
     143,   364,   365,   104,   521,   105,   167,   366,   367,   369,
     106,   107,   372,   374,   377,   378,   108,   109,   379,   382,
     524,   400,   550,   528,   431,   434,   387,   435,   439,    28,
     244,   440,   545,   443,    35,   446,   275,   451,   466,   467,
      70,   490,   484,   556,   492,   470,   502,   518,   493,   522,
     541,   547,   560,   557,   561,   562,   563,   574,    64,   250,
     564,   565,   167,   349,   143,   566,   567,   568,   551,   575,
     404,   398,   462,   428,   570,    65,   349,    66,    67,   341,
      68,   165,    69,   406,   242,   519,   243,   157,   572,   448,
     445,   544,   349,   573,    70,    71,   507,    72,    73,    74,
      75,     0,     0,     0,    76,     0,     0,     0,    77,    78,
      79,    80,    81,    64,     0,    82,    83,    84,    85,     0,
       0,     0,     0,     0,    86,     0,    87,    88,     0,     0,
      65,     0,    66,    67,     0,    68,     0,    69,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,     0,     0,     0,    76,
       0,     0,     0,    77,    78,    79,    80,    81,     0,     0,
      82,    83,    84,    85,     0,     0,     0,     0,     0,    86,
       0,    87,    88
};

static const yytype_int16 yycheck[] =
{
      23,    84,    25,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    68,   121,   118,
      65,   117,    67,   123,    69,   122,    80,   196,   197,   116,
     125,    76,    77,    78,    79,    80,    81,   124,   255,   126,
      85,    74,    87,    88,   191,   192,    18,   288,   289,    84,
     144,     6,     6,    76,     7,   288,   289,     6,     3,    67,
     114,    84,    70,    71,   150,    73,    68,    75,   113,    77,
     115,    12,   182,   118,   184,    16,   121,     7,    80,     3,
       4,     6,    23,    24,    25,    10,    41,   304,    29,     3,
       3,    81,    41,     6,   148,     7,   119,     3,    28,    22,
     210,    24,    25,   148,    27,   113,    29,   180,     0,   117,
      51,   184,   120,     3,    55,    56,    57,     7,   204,    32,
     203,     6,   155,     8,    65,    78,    32,   315,   161,   317,
      85,    85,     3,    56,    57,   180,   144,   210,    79,   184,
      33,     6,    65,   178,    85,    86,   154,    40,    60,    61,
     158,    75,    76,    77,   301,   343,   191,   192,   399,    69,
      70,    75,    76,    77,    83,   210,   399,    77,   191,   192,
       6,   206,     8,     3,   321,    11,   323,   271,   325,    25,
     203,     6,     7,     8,    17,    18,    11,     6,   211,     8,
     407,    87,    11,   279,     3,    41,    42,    84,   297,     6,
      73,    26,    75,    10,     6,     7,     8,    26,   441,    11,
      13,   111,     3,     4,    16,   115,    19,    20,    21,     3,
       4,   193,    82,     6,     7,     8,     6,   444,    11,   274,
       3,     4,   255,    33,    34,    35,    36,    37,    38,    39,
      40,     3,     4,   288,   289,   253,     6,     7,     8,    72,
     387,    11,   297,   390,   391,   290,     6,     7,     8,     7,
     232,    11,     7,   271,     7,   288,   289,     7,     6,     7,
       8,     7,   489,    11,    28,   283,   423,   312,   301,   251,
     315,   304,   317,   318,     6,     7,     8,   322,   323,     9,
      10,   326,   315,     7,   317,   330,   331,   383,   321,    43,
     323,   387,   325,     4,   390,   391,     6,     7,   343,   344,
      13,    14,    15,   546,    60,    61,     6,   352,   353,     6,
     343,    52,    53,    54,    49,    50,   559,     6,   545,   495,
     496,   497,   498,   499,   500,   501,   505,    60,    61,    17,
      18,    30,    31,     3,     4,     6,     4,     6,   371,     6,
       6,     6,   375,   376,   399,     6,   328,   392,     6,     6,
       6,     6,     6,     6,     6,     6,     6,     3,     6,     6,
      66,     4,     7,     7,     4,     4,   399,   412,   432,   474,
     415,     4,   417,     4,   407,   439,     4,     4,     4,   424,
       4,    30,   400,     4,   429,   430,     4,     4,     4,    84,
     423,     4,   437,    12,     4,   428,     4,    16,     4,    29,
       4,     4,     4,     4,    23,    24,    25,     6,   441,     4,
      29,   444,     4,     4,     4,     4,     4,     4,   463,   464,
     484,     4,   535,     3,   517,   534,   532,   537,     4,   536,
       4,     4,    51,   488,   531,     4,    55,    56,    57,     4,
       4,     4,     4,     4,     4,   427,    65,     4,     4,    68,
     468,     4,     4,    72,   518,    74,   489,     4,     4,     4,
      79,    80,     4,     4,     4,     4,    85,    86,     4,     4,
     525,    48,   533,   491,     4,     4,    31,     4,    16,     3,
     535,     6,   527,     6,   517,     6,   550,     4,     4,     4,
      41,     4,    67,   538,     6,    32,    18,    67,    57,     4,
       4,   530,     4,     6,     4,     4,     4,    74,     5,     6,
       4,     4,   545,   546,   532,     4,     4,     4,   533,   574,
     301,   292,   398,   328,   569,    22,   559,    24,    25,   208,
      27,    75,    29,   301,   119,   482,   120,    71,   571,   375,
     371,   525,   575,   571,    41,    42,   474,    44,    45,    46,
      47,    -1,    -1,    -1,    51,    -1,    -1,    -1,    55,    56,
      57,    58,    59,     5,    -1,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    73,    74,    -1,    -1,
      22,    -1,    24,    25,    -1,    27,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    -1,    44,    45,    46,    47,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    -1,    71,
      -1,    73,    74
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   172,   173,    81,     0,     3,   177,     6,    83,
       3,   195,   174,   178,    87,   175,     3,   180,   196,     3,
       4,   176,    84,   179,   110,   111,    82,   181,     3,     4,
      91,   158,     3,     4,    90,    91,   103,   122,   123,   130,
     131,   143,   164,   165,     6,     3,     4,    72,    12,    16,
      23,    24,    25,    29,    51,    55,    56,    57,    65,    79,
      85,    86,    68,    80,     5,    22,    24,    25,    27,    29,
      41,    42,    44,    45,    46,    47,    51,    55,    56,    57,
      58,    59,    62,    63,    64,    65,    71,    73,    74,     9,
      10,    93,    12,    16,    23,    24,    25,    29,    51,    55,
      56,    57,    65,    68,    72,    74,    79,    80,    85,    86,
     156,     7,     7,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,     7,     7,     6,
      10,    89,     6,     8,    92,   100,   138,     7,     6,     8,
      11,    26,    94,    95,   101,   102,   138,    28,   104,   105,
      95,    96,   108,   138,    95,   118,   121,   121,    43,   119,
     127,   118,   125,   119,   128,   125,     3,    91,   109,   133,
     138,    95,   134,   138,   138,   139,   104,   138,   141,   142,
      96,   138,   144,   145,   192,    60,    61,   110,   138,   147,
     148,   184,   193,    32,    91,   113,    17,    18,   150,     6,
      90,    91,   113,   149,     6,   138,   154,   155,     7,   138,
     194,   138,     4,     6,     6,     6,     6,     6,     6,     6,
       6,     6,     6,     6,     6,     6,     6,     6,     6,     6,
       6,     4,    92,    95,   138,    96,    92,   138,    93,   102,
     108,   138,   133,   134,   138,   139,   142,   155,    93,   169,
       6,   112,   183,    93,   187,   188,    69,    70,    77,   157,
      60,    61,     7,     7,     4,     4,     4,     4,    13,    14,
      15,   101,     4,     4,    16,    96,   138,     4,    94,    30,
     106,     4,     4,    95,   119,     4,     4,    95,    49,    50,
     126,     4,   119,     4,    84,     4,     4,    29,    52,    53,
      54,   132,     4,     4,   135,     4,     4,     4,   140,     4,
       6,     4,   113,     4,   193,   184,     4,   184,   193,     4,
      91,   113,   122,   185,    91,   113,   122,   110,   112,     4,
     126,   126,     4,     4,     4,    90,    94,   151,     4,   113,
       4,   148,   166,   184,   193,     4,    75,    76,    77,    91,
      97,   161,   163,   110,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,   170,     4,
     110,   182,     4,    95,     4,   109,   189,     4,     4,     4,
     101,   138,     4,    17,    18,    94,    99,    31,   107,    95,
      30,    31,   120,    97,   192,    97,   192,   113,   106,   124,
      48,   108,   138,    91,   103,   122,   123,   109,     6,    10,
     137,   113,   146,    91,   113,   185,   113,   185,   113,    85,
      91,   122,   113,   113,   122,   122,   113,   114,   111,   113,
     113,     4,    66,   152,     4,     4,   113,   185,   113,    16,
       6,   113,   113,     6,   167,   158,     6,    91,   143,    91,
     190,     4,    94,    13,    19,    20,    21,    98,    99,    99,
      99,   113,   107,    97,   192,    95,     4,     4,    25,   109,
      32,   113,   113,   113,   186,   122,   113,   110,   115,     4,
     113,   113,     7,    96,    67,   153,   113,    96,   162,    97,
       4,   109,     6,    57,    22,    24,    25,    27,    29,    56,
      57,    65,    18,   113,   113,   129,   136,   183,    33,    34,
      35,    36,    37,    38,    39,    40,   116,   117,    67,   153,
       7,    96,     4,    78,   138,   159,   160,   109,    95,   138,
     191,   191,   191,   191,   191,   191,   191,   191,   126,    33,
      40,     4,    90,     7,   159,   113,   171,   100,    93,   102,
     104,   105,   108,   139,   142,   155,   113,     6,   109,    97,
       4,     4,     4,     4,     4,     4,     4,     4,     4,    97,
     113,   168,    91,   164,    74,   138
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    90,    91,    92,    92,    89,    93,    93,
      94,    94,    94,    95,    95,    95,    96,    96,    96,    96,
      97,    98,    98,    98,    98,    99,   100,    90,   101,   101,
     102,   103,    90,   104,   104,   105,    90,   106,   106,   107,
     107,   108,    90,   109,   110,   111,   111,   112,   112,   113,
     114,   113,   115,   115,   116,   116,   116,   116,   116,   116,
     116,   117,   116,   118,   119,   119,   120,   120,   120,   121,
     122,   123,    90,    90,   124,   125,   125,   126,   126,   126,
     126,   127,   129,   128,   130,   130,   131,   131,    90,    90,
     132,   132,   132,   132,   133,   133,   133,    90,   135,   134,
      90,   136,   136,   137,   137,   138,   140,   139,    90,   141,
     141,   142,   142,   142,   143,    90,   144,   145,   145,   145,
     146,   145,   145,   145,    90,   147,   147,   147,   147,   147,
     147,   147,   147,   147,    90,   148,   148,   147,    90,   149,
      90,    90,    90,   150,   150,    90,   151,   151,   152,   152,
     153,   153,   154,   154,   154,   154,   155,    90,   156,   157,
     157,   157,   157,    90,    90,   158,   159,   159,   160,   162,
     161,   163,   163,   163,   163,   164,   165,    90,    90,   166,
     166,   166,    90,   167,   167,   168,   168,   170,   171,   169,
      90,    90,   172,   174,   173,   175,   175,   176,   178,   177,
     179,   179,   180,   181,   181,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   158,   158,   158,   158,   158,   158,   158,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   182,
     182,   183,   184,   186,   185,   187,   188,   189,   189,   190,
     190,   190,   190,   190,   190,   190,   190,   191,   192,   193,
     194,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,   109,   196,   195
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
       4,     5,     4,     4,     4,     4,     1,     0,     2,     0,
       2,     0,     4,     5,     5,     6,     2,     4,     2,     1,
       1,     1,     0,     5,     5,     7,     1,     1,     3,     0,
       4,     1,     1,     1,     1,    10,     4,     1,     1,     2,
       3,     2,     5,     0,     1,     0,     2,     0,     0,    10,
       5,     5,     3,     0,     6,     0,     2,     5,     0,     6,
       0,     2,     4,     0,     4,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     5,     5,     5,     5,     5,     5,     5,     5,
       5,     5,     5,     5,     5,     5,     5,     5,     5,     1,
       2,     2,     1,     0,     5,     2,     4,     2,     2,     5,
       5,     5,     5,     5,     5,     5,     5,     1,     1,     1,
       1,     4,     5,     6,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     0,     5
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
#line 268 "parser.y"
                        {
	#line 423 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2028 "parser.c"
    break;

  case 3: /* content_node: start "glyph" glyph ">"  */
#line 271 "parser.y"
                                  {
	#line 424 "format.w"
	hput_tags((yyvsp[-3].u),hput_glyph(&((yyvsp[-1].c))));}
#line 2036 "parser.c"
    break;

  case 4: /* start: "<"  */
#line 274 "parser.y"
           {
	#line 425 "format.w"
	HPUTNODE;(yyval.u)= (uint32_t)(hpos++-hstart);}
#line 2044 "parser.c"
    break;

  case 6: /* integer: UNSIGNED  */
#line 278 "parser.y"
                         {
	#line 946 "format.w"
	RNG("number",(yyvsp[0].u),0,0x7FFFFFFF);}
#line 2052 "parser.c"
    break;

  case 7: /* glyph: CHARCODE REFERENCE  */
#line 282 "parser.y"
                        {
	#line 1087 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2060 "parser.c"
    break;

  case 9: /* string: CHARCODE  */
#line 286 "parser.y"
                         {
	#line 1192 "format.w"
	static char s[2];
	RNG("String element",(yyvsp[0].u),0x20,0x7E);
	s[0]= (yyvsp[0].u);s[1]= 0;(yyval.s)= s;}
#line 2070 "parser.c"
    break;

  case 10: /* number: UNSIGNED  */
#line 292 "parser.y"
               {
	#line 1345 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].u);}
#line 2078 "parser.c"
    break;

  case 11: /* number: SIGNED  */
#line 294 "parser.y"
                                       {
	#line 1345 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].i);}
#line 2086 "parser.c"
    break;

  case 13: /* dimension: number "pt"  */
#line 298 "parser.y"
                   {
	#line 1696 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2094 "parser.c"
    break;

  case 14: /* dimension: number "in"  */
#line 301 "parser.y"
                    {
	#line 1697 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*72.27);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2102 "parser.c"
    break;

  case 15: /* dimension: number "mm"  */
#line 304 "parser.y"
                  {
	#line 1698 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*(72.27/25.4));RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2110 "parser.c"
    break;

  case 16: /* xdimen: dimension number "h" number "v"  */
#line 308 "parser.y"
                                  {
	#line 1776 "format.w"
	(yyval.xd).w= (yyvsp[-4].d);(yyval.xd).h= (yyvsp[-3].f);(yyval.xd).v= (yyvsp[-1].f);}
#line 2118 "parser.c"
    break;

  case 17: /* xdimen: dimension number "h"  */
#line 311 "parser.y"
                           {
	#line 1777 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= (yyvsp[-1].f);(yyval.xd).v= 0.0;}
#line 2126 "parser.c"
    break;

  case 18: /* xdimen: dimension number "v"  */
#line 314 "parser.y"
                           {
	#line 1778 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= 0.0;(yyval.xd).v= (yyvsp[-1].f);}
#line 2134 "parser.c"
    break;

  case 19: /* xdimen: dimension  */
#line 317 "parser.y"
                  {
	#line 1779 "format.w"
	(yyval.xd).w= (yyvsp[0].d);(yyval.xd).h= 0.0;(yyval.xd).v= 0.0;}
#line 2142 "parser.c"
    break;

  case 20: /* xdimen_node: start "xdimen" xdimen ">"  */
#line 321 "parser.y"
                                   {
	#line 1781 "format.w"
	hput_tags((yyvsp[-3].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 2150 "parser.c"
    break;

  case 21: /* order: "pt"  */
#line 326 "parser.y"
        {
	#line 1960 "format.w"
	(yyval.o)= normal_o;}
#line 2158 "parser.c"
    break;

  case 22: /* order: "fil"  */
#line 328 "parser.y"
                            {
	#line 1960 "format.w"
	(yyval.o)= fil_o;}
#line 2166 "parser.c"
    break;

  case 23: /* order: "fill"  */
#line 330 "parser.y"
                             {
	#line 1960 "format.w"
	(yyval.o)= fill_o;}
#line 2174 "parser.c"
    break;

  case 24: /* order: "filll"  */
#line 332 "parser.y"
                              {
	#line 1960 "format.w"
	(yyval.o)= filll_o;}
#line 2182 "parser.c"
    break;

  case 25: /* stretch: number order  */
#line 336 "parser.y"
                    {
	#line 1962 "format.w"
	(yyval.st).f= (yyvsp[-1].f);(yyval.st).o= (yyvsp[0].o);}
#line 2190 "parser.c"
    break;

  case 26: /* penalty: integer  */
#line 340 "parser.y"
               {
	#line 2016 "format.w"
	RNG("Penalty",(yyvsp[0].i),-20000,+20000);(yyval.i)= (yyvsp[0].i);}
#line 2198 "parser.c"
    break;

  case 27: /* content_node: start "penalty" penalty ">"  */
#line 343 "parser.y"
                                      {
	#line 2017 "format.w"
	hput_tags((yyvsp[-3].u),hput_int((yyvsp[-1].i)));}
#line 2206 "parser.c"
    break;

  case 29: /* rule_dimension: "|"  */
#line 347 "parser.y"
                                        {
	#line 2192 "format.w"
	(yyval.d)= RUNNING_DIMEN;}
#line 2214 "parser.c"
    break;

  case 30: /* rule: rule_dimension rule_dimension rule_dimension  */
#line 351 "parser.y"
{
	#line 2194 "format.w"
	(yyval.r).h= (yyvsp[-2].d);(yyval.r).d= (yyvsp[-1].d);(yyval.r).w= (yyvsp[0].d);
	if((yyvsp[0].d)==RUNNING_DIMEN&&((yyvsp[-2].d)==RUNNING_DIMEN||(yyvsp[-1].d)==RUNNING_DIMEN))
	QUIT("Incompatible running dimensions 0x%x 0x%x 0x%x",(yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2224 "parser.c"
    break;

  case 31: /* rule_node: start "rule" rule ">"  */
#line 356 "parser.y"
                             {
	#line 2197 "format.w"
	hput_tags((yyvsp[-3].u),hput_rule(&((yyvsp[-1].r))));}
#line 2232 "parser.c"
    break;

  case 33: /* explicit: %empty  */
#line 361 "parser.y"
         {
	#line 2305 "format.w"
	(yyval.b)= false;}
#line 2240 "parser.c"
    break;

  case 34: /* explicit: "!"  */
#line 363 "parser.y"
                                 {
	#line 2305 "format.w"
	(yyval.b)= true;}
#line 2248 "parser.c"
    break;

  case 35: /* kern: explicit xdimen  */
#line 366 "parser.y"
                    {
	#line 2306 "format.w"
	(yyval.kt).x= (yyvsp[-1].b);(yyval.kt).d= (yyvsp[0].xd);}
#line 2256 "parser.c"
    break;

  case 36: /* content_node: start "kern" kern ">"  */
#line 369 "parser.y"
                                {
	#line 2307 "format.w"
	hput_tags((yyvsp[-3].u),hput_kern(&((yyvsp[-1].kt))));}
#line 2264 "parser.c"
    break;

  case 37: /* plus: %empty  */
#line 373 "parser.y"
     {
	#line 2517 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2272 "parser.c"
    break;

  case 38: /* plus: "plus" stretch  */
#line 375 "parser.y"
                                             {
	#line 2517 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2280 "parser.c"
    break;

  case 39: /* minus: %empty  */
#line 378 "parser.y"
      {
	#line 2518 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2288 "parser.c"
    break;

  case 40: /* minus: "minus" stretch  */
#line 380 "parser.y"
                                              {
	#line 2518 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2296 "parser.c"
    break;

  case 41: /* glue: xdimen plus minus  */
#line 383 "parser.y"
                      {
	#line 2519 "format.w"
	(yyval.g).w= (yyvsp[-2].xd);(yyval.g).p= (yyvsp[-1].st);(yyval.g).m= (yyvsp[0].st);}
#line 2304 "parser.c"
    break;

  case 42: /* content_node: start "glue" glue ">"  */
#line 386 "parser.y"
                                {
	#line 2520 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){HPUT8(zero_skip_no);
	hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}else hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));}
#line 2313 "parser.c"
    break;

  case 43: /* glue_node: start "glue" glue ">"  */
#line 391 "parser.y"
{
	#line 2523 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){hpos--;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));(yyval.b)= true;}}
#line 2322 "parser.c"
    break;

  case 44: /* position: %empty  */
#line 396 "parser.y"
         {
	#line 2799 "format.w"
	(yyval.u)= hpos-hstart;}
#line 2330 "parser.c"
    break;

  case 47: /* estimate: %empty  */
#line 401 "parser.y"
         {
	#line 2802 "format.w"
	hpos+= 2;}
#line 2338 "parser.c"
    break;

  case 48: /* estimate: UNSIGNED  */
#line 404 "parser.y"
                 {
	#line 2803 "format.w"
	hpos+= hsize_bytes((yyvsp[0].u))+1;}
#line 2346 "parser.c"
    break;

  case 49: /* list: start estimate content_list ">"  */
#line 408 "parser.y"
{
	#line 2805 "format.w"
	(yyval.l).k= list_kind;(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2355 "parser.c"
    break;

  case 50: /* $@1: %empty  */
#line 414 "parser.y"
{
	#line 3212 "format.w"
	hpos+= 4;}
#line 2363 "parser.c"
    break;

  case 51: /* list: TXT_START position $@1 text TXT_END  */
#line 418 "parser.y"
{
	#line 3214 "format.w"
	(yyval.l).k= text_kind;(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2372 "parser.c"
    break;

  case 54: /* txt: TXT_CC  */
#line 424 "parser.y"
          {
	#line 3218 "format.w"
	hput_txt_cc((yyvsp[0].u));}
#line 2380 "parser.c"
    break;

  case 55: /* txt: TXT_FONT  */
#line 427 "parser.y"
                 {
	#line 3219 "format.w"
	REF(font_kind,(yyvsp[0].u));hput_txt_font((yyvsp[0].u));}
#line 2388 "parser.c"
    break;

  case 56: /* txt: TXT_GLOBAL  */
#line 430 "parser.y"
                   {
	#line 3220 "format.w"
	REF((yyvsp[0].rf).k,(yyvsp[0].rf).n);hput_txt_global(&((yyvsp[0].rf)));}
#line 2396 "parser.c"
    break;

  case 57: /* txt: TXT_LOCAL  */
#line 433 "parser.y"
                  {
	#line 3221 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,11);hput_txt_local((yyvsp[0].u));}
#line 2404 "parser.c"
    break;

  case 58: /* txt: TXT_FONT_GLUE  */
#line 436 "parser.y"
                      {
	#line 3222 "format.w"
	HPUTX(1);HPUT8(txt_glue);}
#line 2412 "parser.c"
    break;

  case 59: /* txt: TXT_FONT_HYPHEN  */
#line 439 "parser.y"
                        {
	#line 3223 "format.w"
	HPUTX(1);HPUT8(txt_hyphen);}
#line 2420 "parser.c"
    break;

  case 60: /* txt: TXT_IGNORE  */
#line 442 "parser.y"
                   {
	#line 3224 "format.w"
	HPUTX(1);HPUT8(txt_ignore);}
#line 2428 "parser.c"
    break;

  case 61: /* $@2: %empty  */
#line 445 "parser.y"
         {
	#line 3225 "format.w"
	HPUTX(1);HPUT8(txt_node);}
#line 2436 "parser.c"
    break;

  case 63: /* box_dimen: dimension dimension dimension  */
#line 451 "parser.y"
{
	#line 3485 "format.w"
	(yyval.info)= hput_box_dimen((yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2444 "parser.c"
    break;

  case 64: /* box_shift: %empty  */
#line 454 "parser.y"
          {
	#line 3486 "format.w"
	(yyval.info)= b000;}
#line 2452 "parser.c"
    break;

  case 65: /* box_shift: "shifted" dimension  */
#line 457 "parser.y"
                          {
	#line 3487 "format.w"
	(yyval.info)= hput_box_shift((yyvsp[0].d));}
#line 2460 "parser.c"
    break;

  case 66: /* box_glue_set: %empty  */
#line 461 "parser.y"
             {
	#line 3489 "format.w"
	(yyval.info)= b000;}
#line 2468 "parser.c"
    break;

  case 67: /* box_glue_set: "plus" stretch  */
#line 464 "parser.y"
                     {
	#line 3490 "format.w"
	(yyval.info)= hput_box_glue_set(+1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2476 "parser.c"
    break;

  case 68: /* box_glue_set: "minus" stretch  */
#line 467 "parser.y"
                      {
	#line 3491 "format.w"
	(yyval.info)= hput_box_glue_set(-1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2484 "parser.c"
    break;

  case 69: /* box: box_dimen box_shift box_glue_set list  */
#line 472 "parser.y"
                                         {
	#line 3494 "format.w"
	(yyval.info)= (yyvsp[-3].info)	|(yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2492 "parser.c"
    break;

  case 70: /* hbox_node: start "hbox" box ">"  */
#line 476 "parser.y"
                            {
	#line 3496 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hbox_kind,(yyvsp[-1].info)));}
#line 2500 "parser.c"
    break;

  case 71: /* vbox_node: start "vbox" box ">"  */
#line 479 "parser.y"
                            {
	#line 3497 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vbox_kind,(yyvsp[-1].info)));}
#line 2508 "parser.c"
    break;

  case 74: /* box_flex: plus minus  */
#line 484 "parser.y"
                   {
	#line 3678 "format.w"
	hput_stretch(&((yyvsp[-1].st)));hput_stretch(&((yyvsp[0].st)));}
#line 2516 "parser.c"
    break;

  case 75: /* xbox: box_dimen box_shift box_flex xdimen_ref list  */
#line 487 "parser.y"
                                                 {
	#line 3679 "format.w"
	(yyval.info)= (yyvsp[-4].info)	|(yyvsp[-3].info);}
#line 2524 "parser.c"
    break;

  case 76: /* xbox: box_dimen box_shift box_flex xdimen_node list  */
#line 490 "parser.y"
                                                      {
	#line 3680 "format.w"
	(yyval.info)= (yyvsp[-4].info)	|(yyvsp[-3].info)	|b100;}
#line 2532 "parser.c"
    break;

  case 77: /* box_goal: "to" xdimen_ref  */
#line 494 "parser.y"
                      {
	#line 3682 "format.w"
	(yyval.info)= b000;}
#line 2540 "parser.c"
    break;

  case 78: /* box_goal: "add" xdimen_ref  */
#line 497 "parser.y"
                       {
	#line 3683 "format.w"
	(yyval.info)= b001;}
#line 2548 "parser.c"
    break;

  case 79: /* box_goal: "to" xdimen_node  */
#line 500 "parser.y"
                       {
	#line 3684 "format.w"
	(yyval.info)= b100;}
#line 2556 "parser.c"
    break;

  case 80: /* box_goal: "add" xdimen_node  */
#line 503 "parser.y"
                        {
	#line 3685 "format.w"
	(yyval.info)= b101;}
#line 2564 "parser.c"
    break;

  case 81: /* hpack: box_shift box_goal list  */
#line 507 "parser.y"
                             {
	#line 3687 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 2572 "parser.c"
    break;

  case 82: /* $@3: %empty  */
#line 510 "parser.y"
                                   {
	#line 3688 "format.w"
	HPUT32((yyvsp[0].d));}
#line 2580 "parser.c"
    break;

  case 83: /* vpack: box_shift "max" "depth" dimension $@3 box_goal list  */
#line 512 "parser.y"
                                 {
	#line 3688 "format.w"
	(yyval.info)= (yyvsp[-6].info)	|(yyvsp[-1].info);}
#line 2588 "parser.c"
    break;

  case 84: /* vxbox_node: start "vset" xbox ">"  */
#line 516 "parser.y"
                              {
	#line 3690 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vset_kind,(yyvsp[-1].info)));}
#line 2596 "parser.c"
    break;

  case 85: /* vxbox_node: start "vpack" vpack ">"  */
#line 519 "parser.y"
                              {
	#line 3691 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vpack_kind,(yyvsp[-1].info)));}
#line 2604 "parser.c"
    break;

  case 86: /* hxbox_node: start "hset" xbox ">"  */
#line 524 "parser.y"
                              {
	#line 3694 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hset_kind,(yyvsp[-1].info)));}
#line 2612 "parser.c"
    break;

  case 87: /* hxbox_node: start "hpack" hpack ">"  */
#line 527 "parser.y"
                              {
	#line 3695 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hpack_kind,(yyvsp[-1].info)));}
#line 2620 "parser.c"
    break;

  case 90: /* ltype: %empty  */
#line 533 "parser.y"
      {
	#line 3805 "format.w"
	(yyval.info)= 1;}
#line 2628 "parser.c"
    break;

  case 91: /* ltype: "align"  */
#line 535 "parser.y"
                      {
	#line 3805 "format.w"
	(yyval.info)= 1;}
#line 2636 "parser.c"
    break;

  case 92: /* ltype: "center"  */
#line 537 "parser.y"
                       {
	#line 3805 "format.w"
	(yyval.info)= 2;}
#line 2644 "parser.c"
    break;

  case 93: /* ltype: "expand"  */
#line 539 "parser.y"
                       {
	#line 3805 "format.w"
	(yyval.info)= 3;}
#line 2652 "parser.c"
    break;

  case 94: /* leaders: glue_node ltype rule_node  */
#line 542 "parser.y"
                                 {
	#line 3806 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2660 "parser.c"
    break;

  case 95: /* leaders: glue_node ltype hbox_node  */
#line 545 "parser.y"
                                  {
	#line 3807 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2668 "parser.c"
    break;

  case 96: /* leaders: glue_node ltype vbox_node  */
#line 548 "parser.y"
                                  {
	#line 3808 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2676 "parser.c"
    break;

  case 97: /* content_node: start "leaders" leaders ">"  */
#line 551 "parser.y"
                                      {
	#line 3809 "format.w"
	hput_tags((yyvsp[-3].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 2684 "parser.c"
    break;

  case 98: /* $@4: %empty  */
#line 555 "parser.y"
                  {
	#line 3915 "format.w"
	if((yyvsp[0].d)!=0)HPUT32((yyvsp[0].d));}
#line 2692 "parser.c"
    break;

  case 99: /* baseline: dimension $@4 glue_node glue_node  */
#line 558 "parser.y"
                   {
	#line 3916 "format.w"
	(yyval.info)= b000;if((yyvsp[-3].d)!=0)(yyval.info)	|= b001;
	if((yyvsp[-1].b))(yyval.info)	|= b100;
	if((yyvsp[0].b))(yyval.info)	|= b010;
	}
#line 2703 "parser.c"
    break;

  case 100: /* content_node: start "baseline" baseline ">"  */
#line 565 "parser.y"
{
	#line 3921 "format.w"
	if((yyvsp[-1].info)==b000)HPUT8(0);hput_tags((yyvsp[-3].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 2711 "parser.c"
    break;

  case 102: /* cc_list: cc_list TXT_CC  */
#line 569 "parser.y"
                               {
	#line 4004 "format.w"
	hput_utf8((yyvsp[0].u));}
#line 2719 "parser.c"
    break;

  case 103: /* lig_cc: UNSIGNED  */
#line 572 "parser.y"
               {
	#line 4005 "format.w"
	RNG("UTF-8 code",(yyvsp[0].u),0,0x1FFFFF);(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2727 "parser.c"
    break;

  case 104: /* lig_cc: CHARCODE  */
#line 575 "parser.y"
               {
	#line 4006 "format.w"
	(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2735 "parser.c"
    break;

  case 105: /* ref: REFERENCE  */
#line 578 "parser.y"
             {
	#line 4007 "format.w"
	HPUT8((yyvsp[0].u));(yyval.u)= (yyvsp[0].u);}
#line 2743 "parser.c"
    break;

  case 106: /* $@5: %empty  */
#line 581 "parser.y"
            {
	#line 4008 "format.w"
	REF(font_kind,(yyvsp[0].u));}
#line 2751 "parser.c"
    break;

  case 107: /* ligature: ref $@5 lig_cc TXT_START cc_list TXT_END  */
#line 584 "parser.y"
{
	#line 4009 "format.w"
	(yyval.lg).f= (yyvsp[-5].u);(yyval.lg).l.p= (yyvsp[-3].u);(yyval.lg).l.s= (hpos-hstart)-(yyvsp[-3].u);
	RNG("Ligature size",(yyval.lg).l.s,0,255);}
#line 2760 "parser.c"
    break;

  case 108: /* content_node: start "ligature" ligature ">"  */
#line 588 "parser.y"
                                        {
	#line 4011 "format.w"
	hput_tags((yyvsp[-3].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 2768 "parser.c"
    break;

  case 109: /* replace_count: explicit  */
#line 592 "parser.y"
                      {
	#line 4121 "format.w"
	if((yyvsp[0].b)){(yyval.u)= 0x80;HPUT8(0x80);}else (yyval.u)= 0x00;}
#line 2776 "parser.c"
    break;

  case 110: /* replace_count: explicit UNSIGNED  */
#line 595 "parser.y"
                          {
	#line 4122 "format.w"
	RNG("Replace count",(yyvsp[0].u),0,31);
	(yyval.u)= ((yyvsp[0].u))	|(((yyvsp[-1].b))?0x80:0x00);if((yyval.u)!=0)HPUT8((yyval.u));}
#line 2785 "parser.c"
    break;

  case 111: /* disc: replace_count list list  */
#line 599 "parser.y"
                            {
	#line 4124 "format.w"
	(yyval.dc).r= (yyvsp[-2].u);(yyval.dc).p= (yyvsp[-1].l);(yyval.dc).q= (yyvsp[0].l);
	if((yyvsp[0].l).s==0){hpos= hpos-2;if((yyvsp[-1].l).s==0)hpos= hpos-2;}}
#line 2794 "parser.c"
    break;

  case 112: /* disc: replace_count list  */
#line 603 "parser.y"
                           {
	#line 4126 "format.w"
	(yyval.dc).r= (yyvsp[-1].u);(yyval.dc).p= (yyvsp[0].l);if((yyvsp[0].l).s==0)hpos= hpos-2;(yyval.dc).q.s= 0;}
#line 2802 "parser.c"
    break;

  case 113: /* disc: replace_count  */
#line 606 "parser.y"
                      {
	#line 4127 "format.w"
	(yyval.dc).r= (yyvsp[0].u);(yyval.dc).p.s= 0;(yyval.dc).q.s= 0;}
#line 2810 "parser.c"
    break;

  case 114: /* disc_node: start "disc" disc ">"  */
#line 612 "parser.y"
{
	#line 4131 "format.w"
	hput_tags((yyvsp[-3].u),hput_disc(&((yyvsp[-1].dc))));}
#line 2818 "parser.c"
    break;

  case 116: /* par_dimen: xdimen  */
#line 618 "parser.y"
                {
	#line 4285 "format.w"
	hput_xdimen_node(&((yyvsp[0].xd)));}
#line 2826 "parser.c"
    break;

  case 117: /* par: xdimen_ref param_ref list  */
#line 621 "parser.y"
                             {
	#line 4286 "format.w"
	(yyval.info)= b000;}
#line 2834 "parser.c"
    break;

  case 118: /* par: xdimen_ref empty_param_list non_empty_param_list list  */
#line 624 "parser.y"
                                                              {
	#line 4287 "format.w"
	(yyval.info)= b010;}
#line 2842 "parser.c"
    break;

  case 119: /* par: xdimen_ref empty_param_list list  */
#line 627 "parser.y"
                                         {
	#line 4288 "format.w"
	(yyval.info)= b010;}
#line 2850 "parser.c"
    break;

  case 120: /* $@6: %empty  */
#line 630 "parser.y"
                         {
	#line 4289 "format.w"
	hput_xdimen_node(&((yyvsp[-1].xd)));}
#line 2858 "parser.c"
    break;

  case 121: /* par: xdimen param_ref $@6 list  */
#line 632 "parser.y"
                                     {
	#line 4289 "format.w"
	(yyval.info)= b100;}
#line 2866 "parser.c"
    break;

  case 122: /* par: par_dimen empty_param_list non_empty_param_list list  */
#line 635 "parser.y"
                                                             {
	#line 4290 "format.w"
	(yyval.info)= b110;}
#line 2874 "parser.c"
    break;

  case 123: /* par: par_dimen empty_param_list list  */
#line 638 "parser.y"
                                        {
	#line 4291 "format.w"
	(yyval.info)= b110;}
#line 2882 "parser.c"
    break;

  case 124: /* content_node: start "par" par ">"  */
#line 642 "parser.y"
                              {
	#line 4293 "format.w"
	hput_tags((yyvsp[-3].u),TAG(par_kind,(yyvsp[-1].info)));}
#line 2890 "parser.c"
    break;

  case 125: /* math: param_ref list  */
#line 646 "parser.y"
                   {
	#line 4359 "format.w"
	(yyval.info)= b000;}
#line 2898 "parser.c"
    break;

  case 126: /* math: param_ref list hbox_node  */
#line 649 "parser.y"
                                 {
	#line 4360 "format.w"
	(yyval.info)= b001;}
#line 2906 "parser.c"
    break;

  case 127: /* math: param_ref hbox_node list  */
#line 652 "parser.y"
                                 {
	#line 4361 "format.w"
	(yyval.info)= b010;}
#line 2914 "parser.c"
    break;

  case 128: /* math: empty_param_list list  */
#line 655 "parser.y"
                              {
	#line 4362 "format.w"
	(yyval.info)= b100;}
#line 2922 "parser.c"
    break;

  case 129: /* math: empty_param_list list hbox_node  */
#line 658 "parser.y"
                                        {
	#line 4363 "format.w"
	(yyval.info)= b101;}
#line 2930 "parser.c"
    break;

  case 130: /* math: empty_param_list hbox_node list  */
#line 661 "parser.y"
                                        {
	#line 4364 "format.w"
	(yyval.info)= b110;}
#line 2938 "parser.c"
    break;

  case 131: /* math: empty_param_list non_empty_param_list list  */
#line 664 "parser.y"
                                                   {
	#line 4365 "format.w"
	(yyval.info)= b100;}
#line 2946 "parser.c"
    break;

  case 132: /* math: empty_param_list non_empty_param_list list hbox_node  */
#line 667 "parser.y"
                                                             {
	#line 4366 "format.w"
	(yyval.info)= b101;}
#line 2954 "parser.c"
    break;

  case 133: /* math: empty_param_list non_empty_param_list hbox_node list  */
#line 670 "parser.y"
                                                             {
	#line 4367 "format.w"
	(yyval.info)= b110;}
#line 2962 "parser.c"
    break;

  case 134: /* content_node: start "math" math ">"  */
#line 674 "parser.y"
                                {
	#line 4369 "format.w"
	hput_tags((yyvsp[-3].u),TAG(math_kind,(yyvsp[-1].info)));}
#line 2970 "parser.c"
    break;

  case 135: /* on_off: "on"  */
#line 678 "parser.y"
         {
	#line 4419 "format.w"
	(yyval.i)= 1;}
#line 2978 "parser.c"
    break;

  case 136: /* on_off: "off"  */
#line 680 "parser.y"
                    {
	#line 4419 "format.w"
	(yyval.i)= 0;}
#line 2986 "parser.c"
    break;

  case 137: /* math: on_off  */
#line 683 "parser.y"
           {
	#line 4420 "format.w"
	(yyval.info)= b011	|((yyvsp[0].i)<<2);}
#line 2994 "parser.c"
    break;

  case 138: /* content_node: start "adjust" list ">"  */
#line 687 "parser.y"
                                  {
	#line 4451 "format.w"
	hput_tags((yyvsp[-3].u),TAG(adjust_kind,1));}
#line 3002 "parser.c"
    break;

  case 139: /* span_count: UNSIGNED  */
#line 691 "parser.y"
                   {
	#line 4550 "format.w"
	(yyval.info)= hput_span_count((yyvsp[0].u));}
#line 3010 "parser.c"
    break;

  case 140: /* content_node: start "item" content_node ">"  */
#line 694 "parser.y"
                                        {
	#line 4551 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,1));}
#line 3018 "parser.c"
    break;

  case 141: /* content_node: start "item" span_count content_node ">"  */
#line 697 "parser.y"
                                                   {
	#line 4552 "format.w"
	hput_tags((yyvsp[-4].u),TAG(item_kind,(yyvsp[-2].info)));}
#line 3026 "parser.c"
    break;

  case 142: /* content_node: start "item" list ">"  */
#line 700 "parser.y"
                                {
	#line 4553 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,b000));}
#line 3034 "parser.c"
    break;

  case 143: /* table: "h" box_goal list list  */
#line 704 "parser.y"
                          {
	#line 4555 "format.w"
	(yyval.info)= (yyvsp[-2].info);}
#line 3042 "parser.c"
    break;

  case 144: /* table: "v" box_goal list list  */
#line 707 "parser.y"
                          {
	#line 4556 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|b010;}
#line 3050 "parser.c"
    break;

  case 145: /* content_node: start "table" table ">"  */
#line 711 "parser.y"
                                  {
	#line 4558 "format.w"
	hput_tags((yyvsp[-3].u),TAG(table_kind,(yyvsp[-1].info)));}
#line 3058 "parser.c"
    break;

  case 146: /* image_aspect: number  */
#line 715 "parser.y"
                   {
	#line 4692 "format.w"
	(yyval.f)= (yyvsp[0].f);}
#line 3066 "parser.c"
    break;

  case 147: /* image_aspect: %empty  */
#line 717 "parser.y"
                         {
	#line 4692 "format.w"
	(yyval.f)= 0.0;}
#line 3074 "parser.c"
    break;

  case 148: /* image_width: "width" xdimen  */
#line 720 "parser.y"
                        {
	#line 4693 "format.w"
	(yyval.xd)= (yyvsp[0].xd);}
#line 3082 "parser.c"
    break;

  case 149: /* image_width: %empty  */
#line 723 "parser.y"
         {
	#line 4694 "format.w"
	(yyval.xd)= xdimen_defaults[zero_xdimen_no];}
#line 3090 "parser.c"
    break;

  case 150: /* image_height: "height" xdimen  */
#line 726 "parser.y"
                          {
	#line 4695 "format.w"
	(yyval.xd)= (yyvsp[0].xd);}
#line 3098 "parser.c"
    break;

  case 151: /* image_height: %empty  */
#line 729 "parser.y"
         {
	#line 4696 "format.w"
	(yyval.xd)= xdimen_defaults[zero_xdimen_no];}
#line 3106 "parser.c"
    break;

  case 152: /* image_spec: UNSIGNED image_aspect image_width image_height  */
#line 734 "parser.y"
{
	#line 4699 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-3].u),(yyvsp[-2].f),0,&((yyvsp[-1].xd)),0,&((yyvsp[0].xd)));}
#line 3114 "parser.c"
    break;

  case 153: /* image_spec: UNSIGNED image_aspect "width" REFERENCE image_height  */
#line 738 "parser.y"
{
	#line 4701 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-4].u),(yyvsp[-3].f),(yyvsp[-1].u),NULL,0,&((yyvsp[0].xd)));}
#line 3122 "parser.c"
    break;

  case 154: /* image_spec: UNSIGNED image_aspect image_width "height" REFERENCE  */
#line 742 "parser.y"
{
	#line 4703 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-4].u),(yyvsp[-3].f),0,&((yyvsp[-2].xd)),(yyvsp[0].u),NULL);}
#line 3130 "parser.c"
    break;

  case 155: /* image_spec: UNSIGNED image_aspect "width" REFERENCE "height" REFERENCE  */
#line 746 "parser.y"
{
	#line 4705 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-5].u),(yyvsp[-4].f),(yyvsp[-2].u),NULL,(yyvsp[0].u),NULL);}
#line 3138 "parser.c"
    break;

  case 156: /* image: image_spec list  */
#line 750 "parser.y"
                     {
	#line 4707 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 3146 "parser.c"
    break;

  case 157: /* content_node: start "image" image ">"  */
#line 754 "parser.y"
                                  {
	#line 4709 "format.w"
	hput_tags((yyvsp[-3].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3154 "parser.c"
    break;

  case 158: /* max_value: "outline" UNSIGNED  */
#line 758 "parser.y"
                          {
	#line 5309 "format.w"
	max_outline= (yyvsp[0].u);
	RNG("max outline",max_outline,0,0xFFFF);
	DBG(DBGDEF	|DBGLABEL,"Setting max outline to %d\n",max_outline);
	}
#line 3165 "parser.c"
    break;

  case 159: /* placement: "top"  */
#line 765 "parser.y"
             {
	#line 5401 "format.w"
	(yyval.i)= LABEL_TOP;}
#line 3173 "parser.c"
    break;

  case 160: /* placement: "bot"  */
#line 767 "parser.y"
                            {
	#line 5401 "format.w"
	(yyval.i)= LABEL_BOT;}
#line 3181 "parser.c"
    break;

  case 161: /* placement: "mid"  */
#line 769 "parser.y"
                            {
	#line 5401 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3189 "parser.c"
    break;

  case 162: /* placement: %empty  */
#line 771 "parser.y"
                         {
	#line 5401 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3197 "parser.c"
    break;

  case 163: /* content_node: "<" "label" REFERENCE placement ">"  */
#line 775 "parser.y"
{
	#line 5403 "format.w"
	hset_label((yyvsp[-2].u),(yyvsp[-1].i));}
#line 3205 "parser.c"
    break;

  case 164: /* content_node: start "link" REFERENCE on_off ">"  */
#line 780 "parser.y"
{
	#line 5661 "format.w"
	hput_tags((yyvsp[-4].u),hput_link((yyvsp[-2].u),(yyvsp[-1].i)));}
#line 3213 "parser.c"
    break;

  case 165: /* def_node: "<" "outline" REFERENCE integer position list ">"  */
#line 784 "parser.y"
                                                          {
	#line 5791 "format.w"
	
	static int outline_no= -1;
	(yyval.rf).k= outline_kind;(yyval.rf).n= (yyvsp[-4].u);
	if((yyvsp[-1].l).s==0)QUIT("Outline with empty title in line %d",yylineno);
	outline_no++;
	hset_outline(outline_no,(yyvsp[-4].u),(yyvsp[-3].i),(yyvsp[-2].u));
	}
#line 3227 "parser.c"
    break;

  case 166: /* stream_link: ref  */
#line 794 "parser.y"
               {
	#line 6206 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3235 "parser.c"
    break;

  case 167: /* stream_link: "*"  */
#line 796 "parser.y"
                                                    {
	#line 6206 "format.w"
	HPUT8(255);}
#line 3243 "parser.c"
    break;

  case 168: /* stream_split: stream_link stream_link UNSIGNED  */
#line 799 "parser.y"
                                             {
	#line 6207 "format.w"
	RNG("split ratio",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3251 "parser.c"
    break;

  case 169: /* $@7: %empty  */
#line 802 "parser.y"
                                {
	#line 6208 "format.w"
	RNG("magnification factor",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3259 "parser.c"
    break;

  case 171: /* stream_type: stream_info  */
#line 806 "parser.y"
                       {
	#line 6210 "format.w"
	(yyval.info)= 0;}
#line 3267 "parser.c"
    break;

  case 172: /* stream_type: "first"  */
#line 808 "parser.y"
                      {
	#line 6210 "format.w"
	(yyval.info)= 1;}
#line 3275 "parser.c"
    break;

  case 173: /* stream_type: "last"  */
#line 810 "parser.y"
                     {
	#line 6210 "format.w"
	(yyval.info)= 2;}
#line 3283 "parser.c"
    break;

  case 174: /* stream_type: "top"  */
#line 812 "parser.y"
                    {
	#line 6210 "format.w"
	(yyval.info)= 3;}
#line 3291 "parser.c"
    break;

  case 175: /* stream_def_node: start "stream (definition)" ref stream_type list xdimen_node glue_node list glue_node ">"  */
#line 818 "parser.y"
{
	#line 6214 "format.w"
	DEF((yyval.rf),stream_kind,(yyvsp[-7].u));hput_tags((yyvsp[-9].u),TAG(stream_kind,(yyvsp[-6].info)	|b100));}
#line 3299 "parser.c"
    break;

  case 176: /* stream_ins_node: start "stream (definition)" ref ">"  */
#line 823 "parser.y"
{
	#line 6217 "format.w"
	RNG("Stream insertion",(yyvsp[-1].u),0,max_ref[stream_kind]);hput_tags((yyvsp[-3].u),TAG(stream_kind,b100));}
#line 3307 "parser.c"
    break;

  case 179: /* stream: empty_param_list list  */
#line 829 "parser.y"
                            {
	#line 6312 "format.w"
	(yyval.info)= b010;}
#line 3315 "parser.c"
    break;

  case 180: /* stream: empty_param_list non_empty_param_list list  */
#line 832 "parser.y"
                                                   {
	#line 6313 "format.w"
	(yyval.info)= b010;}
#line 3323 "parser.c"
    break;

  case 181: /* stream: param_ref list  */
#line 835 "parser.y"
                       {
	#line 6314 "format.w"
	(yyval.info)= b000;}
#line 3331 "parser.c"
    break;

  case 182: /* content_node: start "stream" stream_ref stream ">"  */
#line 839 "parser.y"
{
	#line 6316 "format.w"
	hput_tags((yyvsp[-4].u),TAG(stream_kind,(yyvsp[-1].info)));}
#line 3339 "parser.c"
    break;

  case 183: /* page_priority: %empty  */
#line 843 "parser.y"
              {
	#line 6419 "format.w"
	HPUT8(1);}
#line 3347 "parser.c"
    break;

  case 184: /* page_priority: UNSIGNED  */
#line 846 "parser.y"
                 {
	#line 6420 "format.w"
	RNG("page priority",(yyvsp[0].u),0,255);HPUT8((yyvsp[0].u));}
#line 3355 "parser.c"
    break;

  case 187: /* $@8: %empty  */
#line 852 "parser.y"
           {
	#line 6424 "format.w"
	hput_string((yyvsp[0].s));}
#line 3363 "parser.c"
    break;

  case 188: /* $@9: %empty  */
#line 854 "parser.y"
                                                          {
	#line 6424 "format.w"
	HPUT32((yyvsp[0].d));}
#line 3371 "parser.c"
    break;

  case 190: /* content_node: "<" "range" REFERENCE "on" ">"  */
#line 861 "parser.y"
                                         {
	#line 6536 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),true);}
#line 3379 "parser.c"
    break;

  case 191: /* content_node: "<" "range" REFERENCE "off" ">"  */
#line 864 "parser.y"
                                      {
	#line 6537 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),false);}
#line 3387 "parser.c"
    break;

  case 193: /* $@10: %empty  */
#line 870 "parser.y"
                                          {
	#line 7228 "format.w"
	new_directory((yyvsp[0].u)+1);new_output_buffers();}
#line 3395 "parser.c"
    break;

  case 197: /* entry: "<" "entry" UNSIGNED string ">"  */
#line 875 "parser.y"
{
	#line 7231 "format.w"
	RNG("Section number",(yyvsp[-2].u),3,max_section_no);hset_entry(&(dir[(yyvsp[-2].u)]),(yyvsp[-2].u),0,0,(yyvsp[-1].s));}
#line 3403 "parser.c"
    break;

  case 198: /* $@11: %empty  */
#line 879 "parser.y"
                                    {
	#line 7766 "format.w"
	hput_definitions_start();}
#line 3411 "parser.c"
    break;

  case 199: /* definition_section: "<" "definitions" $@11 max_definitions definition_list ">"  */
#line 883 "parser.y"
   {
	#line 7768 "format.w"
	hput_definitions_end();}
#line 3419 "parser.c"
    break;

  case 202: /* max_definitions: "<" "max" max_list ">"  */
#line 889 "parser.y"
{
	#line 7884 "format.w"
		/*253:*/
	if(max_ref[label_kind]>=0)
	ALLOCATE(labels,max_ref[label_kind]+1,Label);
		/*:253*/	/*274:*/
	if(max_outline>=0)
	ALLOCATE(outlines,max_outline+1,Outline);
		/*:274*/	/*301:*/
	ALLOCATE(page_on,max_ref[page_kind]+1,int);
	ALLOCATE(range_pos,2*(max_ref[range_kind]+1),RangePos);
		/*:301*/	/*367:*/
	definition_bits[0][int_kind]= (1<<(MAX_INT_DEFAULT+1))-1;
	definition_bits[0][dimen_kind]= (1<<(MAX_DIMEN_DEFAULT+1))-1;
	definition_bits[0][xdimen_kind]= (1<<(MAX_XDIMEN_DEFAULT+1))-1;
	definition_bits[0][glue_kind]= (1<<(MAX_GLUE_DEFAULT+1))-1;
	definition_bits[0][baseline_kind]= (1<<(MAX_BASELINE_DEFAULT+1))-1;
	definition_bits[0][page_kind]= (1<<(MAX_PAGE_DEFAULT+1))-1;
	definition_bits[0][stream_kind]= (1<<(MAX_STREAM_DEFAULT+1))-1;
	definition_bits[0][range_kind]= (1<<(MAX_RANGE_DEFAULT+1))-1;
		/*:367*/	/*382:*/
	ALLOCATE(hfont_name,max_ref[font_kind]+1,char*);
		/*:382*/hput_max_definitions();}
#line 3447 "parser.c"
    break;

  case 205: /* max_value: "font" UNSIGNED  */
#line 915 "parser.y"
                       {
	#line 7888 "format.w"
	hset_max(font_kind,(yyvsp[0].u));}
#line 3455 "parser.c"
    break;

  case 206: /* max_value: "int" UNSIGNED  */
#line 918 "parser.y"
                         {
	#line 7889 "format.w"
	hset_max(int_kind,(yyvsp[0].u));}
#line 3463 "parser.c"
    break;

  case 207: /* max_value: "dimen" UNSIGNED  */
#line 921 "parser.y"
                       {
	#line 7890 "format.w"
	hset_max(dimen_kind,(yyvsp[0].u));}
#line 3471 "parser.c"
    break;

  case 208: /* max_value: "ligature" UNSIGNED  */
#line 924 "parser.y"
                          {
	#line 7891 "format.w"
	hset_max(ligature_kind,(yyvsp[0].u));}
#line 3479 "parser.c"
    break;

  case 209: /* max_value: "disc" UNSIGNED  */
#line 927 "parser.y"
                      {
	#line 7892 "format.w"
	hset_max(disc_kind,(yyvsp[0].u));}
#line 3487 "parser.c"
    break;

  case 210: /* max_value: "glue" UNSIGNED  */
#line 930 "parser.y"
                      {
	#line 7893 "format.w"
	hset_max(glue_kind,(yyvsp[0].u));}
#line 3495 "parser.c"
    break;

  case 211: /* max_value: "language" UNSIGNED  */
#line 933 "parser.y"
                          {
	#line 7894 "format.w"
	hset_max(language_kind,(yyvsp[0].u));}
#line 3503 "parser.c"
    break;

  case 212: /* max_value: "rule" UNSIGNED  */
#line 936 "parser.y"
                      {
	#line 7895 "format.w"
	hset_max(rule_kind,(yyvsp[0].u));}
#line 3511 "parser.c"
    break;

  case 213: /* max_value: "image" UNSIGNED  */
#line 939 "parser.y"
                       {
	#line 7896 "format.w"
	hset_max(image_kind,(yyvsp[0].u));}
#line 3519 "parser.c"
    break;

  case 214: /* max_value: "leaders" UNSIGNED  */
#line 942 "parser.y"
                         {
	#line 7897 "format.w"
	hset_max(leaders_kind,(yyvsp[0].u));}
#line 3527 "parser.c"
    break;

  case 215: /* max_value: "baseline" UNSIGNED  */
#line 945 "parser.y"
                          {
	#line 7898 "format.w"
	hset_max(baseline_kind,(yyvsp[0].u));}
#line 3535 "parser.c"
    break;

  case 216: /* max_value: "xdimen" UNSIGNED  */
#line 948 "parser.y"
                        {
	#line 7899 "format.w"
	hset_max(xdimen_kind,(yyvsp[0].u));}
#line 3543 "parser.c"
    break;

  case 217: /* max_value: "param" UNSIGNED  */
#line 951 "parser.y"
                       {
	#line 7900 "format.w"
	hset_max(param_kind,(yyvsp[0].u));}
#line 3551 "parser.c"
    break;

  case 218: /* max_value: "stream (definition)" UNSIGNED  */
#line 954 "parser.y"
                           {
	#line 7901 "format.w"
	hset_max(stream_kind,(yyvsp[0].u));}
#line 3559 "parser.c"
    break;

  case 219: /* max_value: "page" UNSIGNED  */
#line 957 "parser.y"
                      {
	#line 7902 "format.w"
	hset_max(page_kind,(yyvsp[0].u));}
#line 3567 "parser.c"
    break;

  case 220: /* max_value: "range" UNSIGNED  */
#line 960 "parser.y"
                       {
	#line 7903 "format.w"
	hset_max(range_kind,(yyvsp[0].u));}
#line 3575 "parser.c"
    break;

  case 221: /* max_value: "label" UNSIGNED  */
#line 963 "parser.y"
                       {
	#line 7904 "format.w"
	hset_max(label_kind,(yyvsp[0].u));}
#line 3583 "parser.c"
    break;

  case 222: /* def_node: start "font" ref font ">"  */
#line 969 "parser.y"
                       {
	#line 8101 "format.w"
	DEF((yyval.rf),font_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),(yyvsp[-1].info));}
#line 3591 "parser.c"
    break;

  case 223: /* def_node: start "int" ref integer ">"  */
#line 972 "parser.y"
                                      {
	#line 8102 "format.w"
	DEF((yyval.rf),int_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3599 "parser.c"
    break;

  case 224: /* def_node: start "dimen" ref dimension ">"  */
#line 975 "parser.y"
                                      {
	#line 8103 "format.w"
	DEF((yyval.rf),dimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_dimen((yyvsp[-1].d)));}
#line 3607 "parser.c"
    break;

  case 225: /* def_node: start "language" ref string ">"  */
#line 978 "parser.y"
                                      {
	#line 8104 "format.w"
	DEF((yyval.rf),language_kind,(yyvsp[-2].u));hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3615 "parser.c"
    break;

  case 226: /* def_node: start "glue" ref glue ">"  */
#line 981 "parser.y"
                                {
	#line 8105 "format.w"
	DEF((yyval.rf),glue_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3623 "parser.c"
    break;

  case 227: /* def_node: start "xdimen" ref xdimen ">"  */
#line 984 "parser.y"
                                    {
	#line 8106 "format.w"
	DEF((yyval.rf),xdimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 3631 "parser.c"
    break;

  case 228: /* def_node: start "rule" ref rule ">"  */
#line 987 "parser.y"
                                {
	#line 8107 "format.w"
	DEF((yyval.rf),rule_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3639 "parser.c"
    break;

  case 229: /* def_node: start "leaders" ref leaders ">"  */
#line 990 "parser.y"
                                      {
	#line 8108 "format.w"
	DEF((yyval.rf),leaders_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 3647 "parser.c"
    break;

  case 230: /* def_node: start "baseline" ref baseline ">"  */
#line 993 "parser.y"
                                        {
	#line 8109 "format.w"
	DEF((yyval.rf),baseline_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 3655 "parser.c"
    break;

  case 231: /* def_node: start "ligature" ref ligature ">"  */
#line 996 "parser.y"
                                        {
	#line 8110 "format.w"
	DEF((yyval.rf),ligature_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3663 "parser.c"
    break;

  case 232: /* def_node: start "disc" ref disc ">"  */
#line 999 "parser.y"
                                {
	#line 8111 "format.w"
	DEF((yyval.rf),disc_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3671 "parser.c"
    break;

  case 233: /* def_node: start "image" ref image ">"  */
#line 1002 "parser.y"
                                  {
	#line 8112 "format.w"
	DEF((yyval.rf),image_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3679 "parser.c"
    break;

  case 234: /* def_node: start "param" ref parameters ">"  */
#line 1005 "parser.y"
                                       {
	#line 8113 "format.w"
	DEF((yyval.rf),param_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_list((yyvsp[-4].u)+2,&((yyvsp[-1].l))));}
#line 3687 "parser.c"
    break;

  case 235: /* def_node: start "page" ref page ">"  */
#line 1008 "parser.y"
                                {
	#line 8114 "format.w"
	DEF((yyval.rf),page_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(page_kind,0));}
#line 3695 "parser.c"
    break;

  case 236: /* def_node: start "int" ref ref ">"  */
#line 1013 "parser.y"
                         {
	#line 8133 "format.w"
	DEF_REF((yyval.rf),int_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(int_kind,0));}
#line 3703 "parser.c"
    break;

  case 237: /* def_node: start "dimen" ref ref ">"  */
#line 1016 "parser.y"
                                {
	#line 8134 "format.w"
	DEF_REF((yyval.rf),dimen_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(dimen_kind,0));}
#line 3711 "parser.c"
    break;

  case 238: /* def_node: start "glue" ref ref ">"  */
#line 1019 "parser.y"
                               {
	#line 8135 "format.w"
	DEF_REF((yyval.rf),glue_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(glue_kind,0));}
#line 3719 "parser.c"
    break;

  case 240: /* def_list: def_list def_node  */
#line 1024 "parser.y"
                          {
	#line 8249 "format.w"
	check_param_def(&((yyvsp[0].rf)));}
#line 3727 "parser.c"
    break;

  case 241: /* parameters: estimate def_list  */
#line 1027 "parser.y"
                            {
	#line 8250 "format.w"
	(yyval.l).p= (yyvsp[0].u);(yyval.l).k= param_kind;(yyval.l).s= (hpos-hstart)-(yyvsp[0].u);}
#line 3735 "parser.c"
    break;

  case 242: /* empty_param_list: position  */
#line 1031 "parser.y"
                         {
	#line 8271 "format.w"
	HPUTX(2);hpos++;hput_tags((yyvsp[0].u),TAG(param_kind,1));}
#line 3743 "parser.c"
    break;

  case 243: /* $@12: %empty  */
#line 1034 "parser.y"
                                {
	#line 8272 "format.w"
	hpos= hpos-2;}
#line 3751 "parser.c"
    break;

  case 244: /* non_empty_param_list: start "param" $@12 parameters ">"  */
#line 1037 "parser.y"
{
	#line 8273 "format.w"
	hput_tags((yyvsp[-4].u)-2,hput_list((yyvsp[-4].u)-1,&((yyvsp[-1].l))));}
#line 3759 "parser.c"
    break;

  case 246: /* font_head: string dimension UNSIGNED UNSIGNED  */
#line 1045 "parser.y"
{
	#line 8415 "format.w"
	uint8_t f= (yyvsp[-4].u);SET_DBIT(f,font_kind);hfont_name[f]= strdup((yyvsp[-3].s));(yyval.info)= hput_font_head(f,hfont_name[f],(yyvsp[-2].d),(yyvsp[-1].u),(yyvsp[0].u));}
#line 3767 "parser.c"
    break;

  case 249: /* font_param: start "penalty" fref penalty ">"  */
#line 1052 "parser.y"
                              {
	#line 8420 "format.w"
	hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3775 "parser.c"
    break;

  case 250: /* font_param: start "kern" fref kern ">"  */
#line 1055 "parser.y"
                                 {
	#line 8421 "format.w"
	hput_tags((yyvsp[-4].u),hput_kern(&((yyvsp[-1].kt))));}
#line 3783 "parser.c"
    break;

  case 251: /* font_param: start "ligature" fref ligature ">"  */
#line 1058 "parser.y"
                                         {
	#line 8422 "format.w"
	hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3791 "parser.c"
    break;

  case 252: /* font_param: start "disc" fref disc ">"  */
#line 1061 "parser.y"
                                 {
	#line 8423 "format.w"
	hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3799 "parser.c"
    break;

  case 253: /* font_param: start "glue" fref glue ">"  */
#line 1064 "parser.y"
                                 {
	#line 8424 "format.w"
	hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3807 "parser.c"
    break;

  case 254: /* font_param: start "language" fref string ">"  */
#line 1067 "parser.y"
                                       {
	#line 8425 "format.w"
	hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3815 "parser.c"
    break;

  case 255: /* font_param: start "rule" fref rule ">"  */
#line 1070 "parser.y"
                                 {
	#line 8426 "format.w"
	hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3823 "parser.c"
    break;

  case 256: /* font_param: start "image" fref image ">"  */
#line 1073 "parser.y"
                                   {
	#line 8427 "format.w"
	hput_tags((yyvsp[-4].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3831 "parser.c"
    break;

  case 257: /* fref: ref  */
#line 1077 "parser.y"
        {
	#line 8429 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,MAX_FONT_PARAMS);}
#line 3839 "parser.c"
    break;

  case 258: /* xdimen_ref: ref  */
#line 1081 "parser.y"
              {
	#line 8502 "format.w"
	REF(xdimen_kind,(yyvsp[0].u));}
#line 3847 "parser.c"
    break;

  case 259: /* param_ref: ref  */
#line 1084 "parser.y"
             {
	#line 8503 "format.w"
	REF(param_kind,(yyvsp[0].u));}
#line 3855 "parser.c"
    break;

  case 260: /* stream_ref: ref  */
#line 1087 "parser.y"
              {
	#line 8504 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3863 "parser.c"
    break;

  case 261: /* content_node: start "penalty" ref ">"  */
#line 1093 "parser.y"
                     {
	#line 8508 "format.w"
	REF(penalty_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(penalty_kind,0));}
#line 3871 "parser.c"
    break;

  case 262: /* content_node: start "kern" explicit ref ">"  */
#line 1097 "parser.y"
{
	#line 8510 "format.w"
	REF(dimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(kern_kind,((yyvsp[-2].b))?b100:b000));}
#line 3879 "parser.c"
    break;

  case 263: /* content_node: start "kern" explicit "xdimen" ref ">"  */
#line 1101 "parser.y"
{
	#line 8512 "format.w"
	REF(xdimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-5].u),TAG(kern_kind,((yyvsp[-3].b))?b101:b001));}
#line 3887 "parser.c"
    break;

  case 264: /* content_node: start "glue" ref ">"  */
#line 1104 "parser.y"
                           {
	#line 8513 "format.w"
	REF(glue_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}
#line 3895 "parser.c"
    break;

  case 265: /* content_node: start "ligature" ref ">"  */
#line 1107 "parser.y"
                               {
	#line 8514 "format.w"
	REF(ligature_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(ligature_kind,0));}
#line 3903 "parser.c"
    break;

  case 266: /* content_node: start "disc" ref ">"  */
#line 1110 "parser.y"
                           {
	#line 8515 "format.w"
	REF(disc_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(disc_kind,0));}
#line 3911 "parser.c"
    break;

  case 267: /* content_node: start "rule" ref ">"  */
#line 1113 "parser.y"
                           {
	#line 8516 "format.w"
	REF(rule_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(rule_kind,0));}
#line 3919 "parser.c"
    break;

  case 268: /* content_node: start "image" ref ">"  */
#line 1116 "parser.y"
                            {
	#line 8517 "format.w"
	REF(image_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(image_kind,0));}
#line 3927 "parser.c"
    break;

  case 269: /* content_node: start "leaders" ref ">"  */
#line 1119 "parser.y"
                              {
	#line 8518 "format.w"
	REF(leaders_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(leaders_kind,0));}
#line 3935 "parser.c"
    break;

  case 270: /* content_node: start "baseline" ref ">"  */
#line 1122 "parser.y"
                               {
	#line 8519 "format.w"
	REF(baseline_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(baseline_kind,0));}
#line 3943 "parser.c"
    break;

  case 271: /* content_node: start "language" REFERENCE ">"  */
#line 1125 "parser.y"
                                     {
	#line 8520 "format.w"
	REF(language_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),hput_language((yyvsp[-1].u)));}
#line 3951 "parser.c"
    break;

  case 272: /* glue_node: start "glue" ref ">"  */
#line 1129 "parser.y"
                            {
	#line 8522 "format.w"
	REF(glue_kind,(yyvsp[-1].u));
	if((yyvsp[-1].u)==zero_skip_no){hpos= hpos-2;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),TAG(glue_kind,0));(yyval.b)= true;}}
#line 3961 "parser.c"
    break;

  case 273: /* $@13: %empty  */
#line 1136 "parser.y"
                             {
	#line 8953 "format.w"
	hput_content_start();}
#line 3969 "parser.c"
    break;

  case 274: /* content_section: "<" "content" $@13 content_list ">"  */
#line 1139 "parser.y"
{
	#line 8954 "format.w"
	hput_content_end();hput_range_defs();hput_label_defs();}
#line 3977 "parser.c"
    break;


#line 3981 "parser.c"

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

#line 1143 "parser.y"

	/*:520*/
