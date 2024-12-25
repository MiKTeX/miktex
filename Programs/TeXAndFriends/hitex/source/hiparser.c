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

	#line 11628 "format.w"
	
#include "hibasetypes.h"
#include <string.h>
#include <math.h>
#include "hierror.h"
#include "hiformat.h"
#include "hiput.h"
extern char**hfont_name;

	/*390:*/
uint32_t definition_bits[0x100/32][32]= {
	#line 8757 "format.w"
	{0}};

#define SET_DBIT(N,K) ((N)>0xFF?1:(definition_bits[N/32][K]	|= (1<<((N)&(32-1)))))
#define GET_DBIT(N,K) ((N)>0xFF?1:((definition_bits[N/32][K]>>((N)&(32-1)))&1))
#define DEF(D,K,N) (D).k= K; (D).n= (N);SET_DBIT((D).n,(D).k);\
 DBG(DBGDEF,"Defining %s %d\n",definition_name[(D).k],(D).n);\
 RNG("Definition",(D).n,max_fixed[(D).k]+1,max_ref[(D).k]);
#define REF(K,N) REF_RNG(K,N);if(!GET_DBIT(N,K)) \
 QUIT("Reference %d to %s before definition",(N),definition_name[K])
	/*:390*/	/*394:*/
#define DEF_REF(D,K,M,N)  DEF(D,K,M);\
if ((int)(M)>max_default[K]) QUIT("Defining non default reference %d for %s",M,definition_name[K]); \
if ((int)(N)>max_fixed[K]) QUIT("Defining reference %d for %s by non fixed reference %d",M,definition_name[K],N);
	/*:394*/

extern void hset_entry(Entry*e,uint16_t i,uint32_t size,
uint32_t xsize,char*file_name);

	/*462:*/
#ifdef DEBUG
#define  YYDEBUG 1
extern int yydebug;
#else
#define YYDEBUG 0
#endif
	/*:462*/
extern int yylex(void);

	/*386:*/
void hset_max(Kind k,int n)
{
	#line 8598 "format.w"
	DBG(DBGDEF,"Setting max %s to %d\n",definition_name[k],n);
	RNG("Maximum",n,max_fixed[k]+1,MAX_REF(k));
	if(n>max_ref[k])
	max_ref[k]= n;
	}
	/*:386*/	/*397:*/
void check_param_def(Ref*df)
{
	#line 8918 "format.w"
	if(df->k!=int_kind&&df->k!=dimen_kind&&df->k!=glue_kind)
	QUIT("Kind %s not allowed in parameter list",definition_name[df->k]);
	if(df->n<=max_fixed[df->k]||max_default[df->k]<df->n)
	QUIT("Parameter %d for %s not allowed in parameter list",df->n,definition_name[df->k]);
	}
	/*:397*/	/*461:*/
extern int yylineno;
int yyerror(const char*msg)
{
	#line 10097 "format.w"
	QUIT(" in line %d %s",yylineno,msg);
	return 0;
	}
	/*:461*/



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
  YYSYMBOL_COLOR = 73,                     /* "color"  */
  YYSYMBOL_UNKNOWN = 74,                   /* "unknown"  */
  YYSYMBOL_STREAM = 75,                    /* "stream"  */
  YYSYMBOL_STREAMDEF = 76,                 /* "stream (definition)"  */
  YYSYMBOL_FIRST = 77,                     /* "first"  */
  YYSYMBOL_LAST = 78,                      /* "last"  */
  YYSYMBOL_TOP = 79,                       /* "top"  */
  YYSYMBOL_NOREFERENCE = 80,               /* "*"  */
  YYSYMBOL_PAGE = 81,                      /* "page"  */
  YYSYMBOL_RANGE = 82,                     /* "range"  */
  YYSYMBOL_DIRECTORY = 83,                 /* "directory"  */
  YYSYMBOL_SECTION = 84,                   /* "entry"  */
  YYSYMBOL_DEFINITIONS = 85,               /* "definitions"  */
  YYSYMBOL_MAX = 86,                       /* "max"  */
  YYSYMBOL_PARAM = 87,                     /* "param"  */
  YYSYMBOL_FONT = 88,                      /* "font"  */
  YYSYMBOL_CONTENT = 89,                   /* "content"  */
  YYSYMBOL_YYACCEPT = 90,                  /* $accept  */
  YYSYMBOL_glyph = 91,                     /* glyph  */
  YYSYMBOL_content_node = 92,              /* content_node  */
  YYSYMBOL_start = 93,                     /* start  */
  YYSYMBOL_integer = 94,                   /* integer  */
  YYSYMBOL_string = 95,                    /* string  */
  YYSYMBOL_number = 96,                    /* number  */
  YYSYMBOL_dimension = 97,                 /* dimension  */
  YYSYMBOL_xdimen = 98,                    /* xdimen  */
  YYSYMBOL_xdimen_node = 99,               /* xdimen_node  */
  YYSYMBOL_order = 100,                    /* order  */
  YYSYMBOL_stretch = 101,                  /* stretch  */
  YYSYMBOL_penalty = 102,                  /* penalty  */
  YYSYMBOL_rule_dimension = 103,           /* rule_dimension  */
  YYSYMBOL_rule = 104,                     /* rule  */
  YYSYMBOL_rule_node = 105,                /* rule_node  */
  YYSYMBOL_explicit = 106,                 /* explicit  */
  YYSYMBOL_kern = 107,                     /* kern  */
  YYSYMBOL_plus = 108,                     /* plus  */
  YYSYMBOL_minus = 109,                    /* minus  */
  YYSYMBOL_glue = 110,                     /* glue  */
  YYSYMBOL_glue_node = 111,                /* glue_node  */
  YYSYMBOL_position = 112,                 /* position  */
  YYSYMBOL_content_list = 113,             /* content_list  */
  YYSYMBOL_estimate = 114,                 /* estimate  */
  YYSYMBOL_list = 115,                     /* list  */
  YYSYMBOL_116_1 = 116,                    /* $@1  */
  YYSYMBOL_text = 117,                     /* text  */
  YYSYMBOL_txt = 118,                      /* txt  */
  YYSYMBOL_119_2 = 119,                    /* $@2  */
  YYSYMBOL_box_dimen = 120,                /* box_dimen  */
  YYSYMBOL_box_shift = 121,                /* box_shift  */
  YYSYMBOL_box_glue_set = 122,             /* box_glue_set  */
  YYSYMBOL_box = 123,                      /* box  */
  YYSYMBOL_hbox_node = 124,                /* hbox_node  */
  YYSYMBOL_vbox_node = 125,                /* vbox_node  */
  YYSYMBOL_box_flex = 126,                 /* box_flex  */
  YYSYMBOL_box_options = 127,              /* box_options  */
  YYSYMBOL_hxbox_node = 128,               /* hxbox_node  */
  YYSYMBOL_vbox_dimen = 129,               /* vbox_dimen  */
  YYSYMBOL_vxbox_node = 130,               /* vxbox_node  */
  YYSYMBOL_box_goal = 131,                 /* box_goal  */
  YYSYMBOL_hpack = 132,                    /* hpack  */
  YYSYMBOL_max_depth = 133,                /* max_depth  */
  YYSYMBOL_vpack = 134,                    /* vpack  */
  YYSYMBOL_135_3 = 135,                    /* $@3  */
  YYSYMBOL_136_4 = 136,                    /* $@4  */
  YYSYMBOL_ltype = 137,                    /* ltype  */
  YYSYMBOL_leaders = 138,                  /* leaders  */
  YYSYMBOL_baseline = 139,                 /* baseline  */
  YYSYMBOL_140_5 = 140,                    /* $@5  */
  YYSYMBOL_cc_list = 141,                  /* cc_list  */
  YYSYMBOL_lig_cc = 142,                   /* lig_cc  */
  YYSYMBOL_ref = 143,                      /* ref  */
  YYSYMBOL_ligature = 144,                 /* ligature  */
  YYSYMBOL_145_6 = 145,                    /* $@6  */
  YYSYMBOL_replace_count = 146,            /* replace_count  */
  YYSYMBOL_disc = 147,                     /* disc  */
  YYSYMBOL_disc_node = 148,                /* disc_node  */
  YYSYMBOL_par_dimen = 149,                /* par_dimen  */
  YYSYMBOL_par = 150,                      /* par  */
  YYSYMBOL_151_7 = 151,                    /* $@7  */
  YYSYMBOL_math = 152,                     /* math  */
  YYSYMBOL_on_off = 153,                   /* on_off  */
  YYSYMBOL_span_count = 154,               /* span_count  */
  YYSYMBOL_table = 155,                    /* table  */
  YYSYMBOL_image_aspect = 156,             /* image_aspect  */
  YYSYMBOL_image_width = 157,              /* image_width  */
  YYSYMBOL_image_height = 158,             /* image_height  */
  YYSYMBOL_image_spec = 159,               /* image_spec  */
  YYSYMBOL_image = 160,                    /* image  */
  YYSYMBOL_max_value = 161,                /* max_value  */
  YYSYMBOL_placement = 162,                /* placement  */
  YYSYMBOL_def_node = 163,                 /* def_node  */
  YYSYMBOL_color = 164,                    /* color  */
  YYSYMBOL_color_pair = 165,               /* color_pair  */
  YYSYMBOL_color_unset = 166,              /* color_unset  */
  YYSYMBOL_color_tripple = 167,            /* color_tripple  */
  YYSYMBOL_color_set = 168,                /* color_set  */
  YYSYMBOL_169_8 = 169,                    /* $@8  */
  YYSYMBOL_unknown_bytes = 170,            /* unknown_bytes  */
  YYSYMBOL_unknown_node = 171,             /* unknown_node  */
  YYSYMBOL_unknown_nodes = 172,            /* unknown_nodes  */
  YYSYMBOL_stream_link = 173,              /* stream_link  */
  YYSYMBOL_stream_split = 174,             /* stream_split  */
  YYSYMBOL_stream_info = 175,              /* stream_info  */
  YYSYMBOL_176_9 = 176,                    /* $@9  */
  YYSYMBOL_stream_type = 177,              /* stream_type  */
  YYSYMBOL_stream_def_node = 178,          /* stream_def_node  */
  YYSYMBOL_stream_ins_node = 179,          /* stream_ins_node  */
  YYSYMBOL_stream = 180,                   /* stream  */
  YYSYMBOL_page_priority = 181,            /* page_priority  */
  YYSYMBOL_stream_def_list = 182,          /* stream_def_list  */
  YYSYMBOL_page = 183,                     /* page  */
  YYSYMBOL_184_10 = 184,                   /* $@10  */
  YYSYMBOL_185_11 = 185,                   /* $@11  */
  YYSYMBOL_hint = 186,                     /* hint  */
  YYSYMBOL_directory_section = 187,        /* directory_section  */
  YYSYMBOL_188_12 = 188,                   /* $@12  */
  YYSYMBOL_entry_list = 189,               /* entry_list  */
  YYSYMBOL_entry = 190,                    /* entry  */
  YYSYMBOL_definition_section = 191,       /* definition_section  */
  YYSYMBOL_192_13 = 192,                   /* $@13  */
  YYSYMBOL_definition_list = 193,          /* definition_list  */
  YYSYMBOL_max_definitions = 194,          /* max_definitions  */
  YYSYMBOL_max_list = 195,                 /* max_list  */
  YYSYMBOL_def_list = 196,                 /* def_list  */
  YYSYMBOL_parameters = 197,               /* parameters  */
  YYSYMBOL_named_param_list = 198,         /* named_param_list  */
  YYSYMBOL_param_list = 199,               /* param_list  */
  YYSYMBOL_font = 200,                     /* font  */
  YYSYMBOL_font_head = 201,                /* font_head  */
  YYSYMBOL_font_param_list = 202,          /* font_param_list  */
  YYSYMBOL_font_param = 203,               /* font_param  */
  YYSYMBOL_fref = 204,                     /* fref  */
  YYSYMBOL_xdimen_ref = 205,               /* xdimen_ref  */
  YYSYMBOL_param_ref = 206,                /* param_ref  */
  YYSYMBOL_stream_ref = 207,               /* stream_ref  */
  YYSYMBOL_content_section = 208,          /* content_section  */
  YYSYMBOL_209_14 = 209                    /* $@14  */
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
#define YYLAST   761

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  90
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  120
/* YYNRULES -- Number of rules.  */
#define YYNRULES  302
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  638

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   344


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
      85,    86,    87,    88,    89
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   276,   276,   279,   282,   286,   286,   290,   294,   294,
     300,   302,   304,   306,   309,   312,   316,   319,   322,   325,
     329,   334,   336,   338,   340,   344,   348,   351,   355,   355,
     358,   364,   367,   369,   371,   374,   377,   381,   383,   386,
     388,   391,   394,   398,   404,   407,   408,   409,   412,   415,
     422,   421,   430,   430,   432,   435,   438,   441,   444,   447,
     450,   453,   453,   458,   462,   465,   469,   472,   475,   480,
     484,   487,   490,   490,   492,   496,   499,   503,   507,   508,
     513,   517,   520,   523,   526,   530,   534,   538,   540,   544,
     544,   549,   549,   555,   559,   559,   561,   563,   565,   567,
     570,   573,   576,   579,   583,   583,   592,   597,   597,   600,
     603,   606,   609,   609,   616,   620,   623,   627,   631,   634,
     639,   644,   646,   649,   652,   655,   655,   660,   664,   668,
     671,   674,   677,   680,   683,   687,   691,   693,   696,   700,
     704,   707,   710,   713,   717,   720,   724,   728,   730,   733,
     736,   739,   742,   746,   750,   754,   758,   763,   767,   771,
     778,   780,   782,   784,   787,   792,   796,   801,   811,   818,
     825,   829,   830,   833,   837,   838,   839,   842,   843,   845,
     845,   852,   856,   861,   864,   868,   871,   873,   876,   876,
     876,   876,   877,   879,   883,   885,   888,   891,   891,   895,
     897,   899,   901,   905,   911,   916,   916,   918,   921,   924,
     929,   932,   936,   936,   938,   940,   938,   947,   950,   954,
     956,   956,   959,   959,   960,   965,   965,   972,   972,   974,
    1007,  1007,  1009,  1012,  1015,  1018,  1021,  1024,  1027,  1030,
    1033,  1036,  1039,  1042,  1045,  1048,  1051,  1054,  1057,  1060,
    1066,  1069,  1072,  1075,  1078,  1081,  1084,  1087,  1090,  1093,
    1096,  1099,  1102,  1105,  1110,  1113,  1116,  1120,  1121,  1124,
    1128,  1132,  1132,  1138,  1140,  1145,  1145,  1148,  1151,  1154,
    1157,  1160,  1163,  1166,  1169,  1173,  1177,  1180,  1183,  1189,
    1192,  1196,  1200,  1203,  1206,  1209,  1212,  1215,  1218,  1221,
    1225,  1232,  1232
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
  "\"bot\"", "\"mid\"", "\"link\"", "\"outline\"", "\"color\"",
  "\"unknown\"", "\"stream\"", "\"stream (definition)\"", "\"first\"",
  "\"last\"", "\"top\"", "\"*\"", "\"page\"", "\"range\"", "\"directory\"",
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
  "def_node", "color", "color_pair", "color_unset", "color_tripple",
  "color_set", "$@8", "unknown_bytes", "unknown_node", "unknown_nodes",
  "stream_link", "stream_split", "stream_info", "$@9", "stream_type",
  "stream_def_node", "stream_ins_node", "stream", "page_priority",
  "stream_def_list", "page", "$@10", "$@11", "hint", "directory_section",
  "$@12", "entry_list", "entry", "definition_section", "$@13",
  "definition_list", "max_definitions", "max_list", "def_list",
  "parameters", "named_param_list", "param_list", "font", "font_head",
  "font_param_list", "font_param", "fref", "xdimen_ref", "param_ref",
  "stream_ref", "content_section", "$@14", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-408)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      86,    -5,   111,   141,   144,  -408,    72,   196,  -408,  -408,
     117,  -408,  -408,   207,  -408,   121,   132,  -408,  -408,   145,
    -408,  -408,  -408,   185,  -408,   220,   241,   253,   181,  -408,
     203,  -408,    50,  -408,  -408,   685,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,   261,   396,  -408,   287,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     310,   299,   299,   299,   314,   331,   103,   227,   334,   186,
     319,   244,   166,   166,   324,   166,    54,    80,   146,   244,
     299,    75,   244,    78,    55,   282,    96,   272,   346,    66,
     372,   299,   299,  -408,  -408,   375,   376,   379,   380,   381,
     382,   383,   384,   385,   386,   388,   389,   391,   392,   393,
     394,   395,   397,   400,   401,   398,   157,  -408,   244,   166,
     227,   261,   205,   244,   406,   166,   299,   319,   404,  -408,
     405,   261,   407,   261,    38,   303,   408,   409,   410,  -408,
    -408,  -408,   418,   419,   420,  -408,  -408,  -408,  -408,   337,
    -408,   205,   422,   423,  -408,   214,   424,   166,   403,   426,
     431,   166,   324,   432,   433,   166,   316,   434,   324,   354,
     402,  -408,   437,   166,  -408,   324,  -408,   413,   291,   440,
     441,  -408,   442,   444,   451,   452,   453,   456,    55,   458,
     299,  -408,   406,   459,   146,  -408,  -408,    47,  -408,   461,
    -408,  -408,    55,    55,  -408,   407,   462,   316,   316,   463,
    -408,   467,   630,   471,   454,   166,   475,    55,   476,   308,
     477,   481,  -408,  -408,   146,    58,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,   486,   487,
     488,   490,   491,   493,   494,   495,   496,   498,   499,  -408,
     501,   502,   503,   506,   240,  -408,   507,  -408,  -408,   508,
     166,   509,   406,  -408,  -408,  -408,   510,   511,   512,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,   205,  -408,
    -408,   299,  -408,   513,  -408,   353,   166,   479,  -408,  -408,
     166,   342,  -408,  -408,  -408,   146,   146,    55,  -408,   403,
     514,  -408,   166,   324,  -408,   166,   516,   244,  -408,  -408,
    -408,   406,  -408,  -408,   406,  -408,  -408,  -408,   190,  -408,
    -408,  -408,    55,  -408,  -408,    55,  -408,    55,    55,   407,
     517,  -408,    65,   406,    55,   406,    55,  -408,  -408,  -408,
      55,    55,  -408,  -408,  -408,   518,  -408,   457,  -408,  -408,
    -408,   210,  -408,  -408,   519,   520,    55,    55,  -408,  -408,
    -408,  -408,   515,   521,  -408,    55,    55,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,   525,   506,   528,  -408,   529,   523,  -408,  -408,   531,
    -408,   532,  -408,   406,   406,  -408,  -408,  -408,  -408,   533,
    -408,   166,  -408,   147,  -408,   166,  -408,  -408,   166,   166,
      55,  -408,  -408,  -408,  -408,  -408,   479,   146,  -408,   324,
    -408,   316,   166,  -408,   535,   536,    79,  -408,  -408,  -408,
     406,  -408,  -408,   522,  -408,    55,  -408,  -408,  -408,   537,
    -408,   504,  -408,  -408,  -408,  -408,  -408,   371,    55,    55,
    -408,   258,   468,  -408,   539,  -408,    94,  -408,  -408,  -408,
     166,  -408,   406,   542,   236,   525,  -408,  -408,  -408,  -408,
    -408,   406,  -408,   543,   500,  -408,   216,  -408,  -408,   534,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
      55,    55,   316,    55,  -408,  -408,  -408,   205,  -408,  -408,
    -408,  -408,  -408,   292,  -408,  -408,  -408,   483,  -408,   296,
    -408,  -408,  -408,  -408,   575,  -408,  -408,  -408,  -408,   547,
      43,   406,  -408,   549,  -408,   330,   525,  -408,  -408,   166,
    -408,   319,   299,   299,   299,   299,   299,   299,   299,   299,
    -408,  -408,  -408,    55,  -408,   112,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,   454,   307,  -408,  -408,  -408,
    -408,  -408,  -408,    43,  -408,    55,   550,  -408,   554,   555,
     556,   557,  -408,  -408,  -408,   157,   261,   205,   319,   166,
     299,   319,   404,  -408,  -408,  -408,  -408,  -408,   558,   406,
     559,  -408,  -408,  -408,  -408,   406,   562,   563,   564,   166,
     565,   566,   567,   568,   569,  -408,   570,   283,   406,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,   571,
      55,  -408,  -408,   406,   530,  -408,   299,    64
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     0,     0,     0,     0,     1,     0,     0,   220,   225,
       0,   219,   222,     0,   301,     0,     0,   227,    44,     0,
     221,   223,   230,     0,    45,     0,     0,     0,     4,   226,
       0,   228,     4,   302,    46,     0,    32,    72,    73,    95,
      94,   121,   205,   206,     0,     0,   229,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      33,     0,     0,     0,    64,     0,    87,     0,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   111,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,   179,
       0,     0,    47,     0,   163,     0,     0,     0,     0,     6,
       5,    26,     0,     0,     0,    10,    11,    12,    29,     0,
      28,     0,     0,     0,    34,     0,     0,    19,    37,     0,
       0,     0,    64,     0,     0,     0,     0,     0,    64,    87,
       0,    89,     0,     0,    78,    64,     4,     0,    96,     0,
       0,   104,     0,     0,   112,     0,   115,     0,   119,     0,
     122,   286,     0,     0,     0,   136,   137,    47,   287,     0,
     138,   271,     0,     0,    44,    47,     0,     0,     0,     0,
     140,     0,    47,     0,     0,   148,     0,     0,     0,     0,
       0,     0,   186,   288,     0,     0,   224,   234,   243,   233,
     238,   239,   237,   241,   242,   235,   236,   240,   248,   159,
     249,   245,   246,   247,   244,   232,   231,    44,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   112,
       0,     0,     0,     0,     0,   214,     0,    48,    44,     0,
       0,     0,     0,   161,   162,   160,     0,     0,     0,     2,
       7,     3,    27,   289,   299,    13,    15,    14,     0,    31,
     295,     0,    35,     0,    36,     0,     0,    39,    42,   292,
       0,    66,    70,    71,    65,     0,     0,     0,    86,    37,
       0,    91,     0,    64,    93,     0,     0,     0,    97,    98,
      99,     0,   103,   297,     0,   106,   298,   293,     0,   114,
     116,   294,   118,   120,   125,     0,   128,     0,     0,    47,
       0,   135,    47,   132,     0,   129,     0,    50,    44,   139,
       0,     0,   146,   141,   143,     0,   147,   150,   296,   157,
     158,     0,   182,   181,   192,     0,     0,     0,   204,   200,
     201,   202,     0,     0,   199,     0,     0,   252,   265,   255,
     251,   264,   253,   256,   254,   266,   257,   258,   259,   260,
     261,     0,   173,     0,   183,     0,   210,   263,   267,   269,
     262,     0,   250,     0,   273,   164,   217,   218,    30,     0,
     290,    17,    18,     0,    38,     0,    41,    63,     0,     0,
       0,    84,    82,    83,    81,    85,    39,     0,    77,    64,
      88,     0,     0,    80,     0,     0,     0,   100,   101,   102,
       0,   109,   110,     0,   117,     0,   127,   124,   123,     0,
     272,     0,   133,   134,   130,   131,    44,     0,     0,     0,
     142,     0,   152,   165,     0,   187,     0,   209,   207,   208,
       0,   197,     0,     0,     0,   173,   173,   177,   180,   184,
     211,     0,   268,     0,     0,   275,     0,   276,   291,     0,
      21,    22,    23,    24,    25,    40,    67,    68,    69,    74,
       0,     0,     0,     0,    79,    43,   300,     0,   105,   107,
     126,   270,    52,    61,    49,   144,   145,   152,   149,     0,
     153,   166,   185,   188,    47,   189,   190,   193,   191,     0,
       0,     0,   167,     0,   170,     0,   173,   173,   173,     0,
     274,    33,     0,     0,     0,     0,     0,     0,     0,     0,
      16,    76,    75,     0,    90,     0,    51,    60,    58,    59,
      55,    57,    56,    54,    53,     0,     0,   154,   155,   151,
      20,   195,   194,     0,   198,     0,     0,   172,     0,     0,
       0,     0,   178,   215,   285,     0,     0,     0,    33,     0,
       0,    33,     0,    92,   113,   108,    62,   156,     0,     0,
       0,   171,   176,   175,   174,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   196,     0,     0,     0,   277,
     282,   283,   278,   281,   279,   280,   284,   203,   169,     0,
       0,   168,   212,   216,     0,   213,     0,     0
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -408,  -408,   -83,   -23,    88,  -100,  -123,    -3,   -65,  -263,
    -408,  -233,   -32,  -104,   -99,   242,   -69,   -26,   268,   152,
    -119,  -245,  -163,   231,  -102,   -27,  -408,  -408,  -408,  -408,
     260,   -52,  -408,   538,  -138,   262,  -408,   411,  -408,  -408,
    -408,  -156,  -408,   416,  -408,  -408,  -408,  -408,   464,   465,
    -408,  -408,  -408,   -43,  -101,  -408,  -408,   -98,   184,  -408,
    -408,  -408,  -408,   370,  -408,  -408,  -408,  -408,    77,  -408,
     -96,  -408,  -408,   193,    61,  -407,  -382,   206,  -408,  -408,
    -408,  -408,  -408,    28,  -408,  -408,  -408,  -408,   -28,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,  -408,
    -408,  -408,  -408,  -408,  -408,  -408,  -408,  -152,   142,   -78,
    -408,  -408,  -408,  -408,  -187,  -249,   -94,  -408,  -408,  -408
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   138,    34,   205,   141,    95,   149,   157,   158,   373,
     494,   414,   142,   151,   152,    36,   186,   156,   297,   416,
     159,   178,    24,    25,   348,   206,   456,   513,   564,   565,
     162,   309,   420,   163,    37,    38,   427,   310,    39,   175,
      40,   307,   167,   171,   172,   313,   429,   321,   179,   182,
     324,   555,   443,   584,   185,   328,   188,   189,    41,   192,
     193,   445,   199,   200,   214,   209,   357,   462,   520,   217,
     218,   115,   276,    31,   535,   475,   476,   392,   393,   263,
     364,   527,   466,   573,   574,   374,   530,   375,    42,    43,
     365,   481,   633,   266,   396,   605,     2,     3,    12,    15,
      21,     7,    13,    23,    17,    27,   399,   269,   201,   202,
     271,   272,   404,   487,   585,   194,   203,   224,    11,    18
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      30,   155,    35,   211,   255,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   190,   131,   132,
     133,   253,   166,   254,   143,   260,   153,   403,   160,   261,
     268,   265,   262,   270,   295,   180,   183,   184,   187,   191,
     198,   347,   421,   423,   216,   340,   221,   288,   223,   225,
     117,   350,   351,   267,   250,   177,   422,   424,   176,   213,
     197,   176,   368,   212,   344,   346,   150,   176,   536,   161,
     161,   267,   161,   117,   161,   249,   181,   252,     4,   440,
     256,   176,   117,   259,   376,   117,   145,   204,   146,     1,
     292,   147,   356,   537,   538,   268,   334,    32,   522,    32,
     338,   177,   210,   154,   507,   398,    72,   273,   274,   136,
     301,     5,   293,   137,   335,   248,   337,   275,    64,   150,
      72,    73,   181,   571,    19,    20,   204,   220,   204,   579,
     367,   355,    65,   169,   339,   369,   370,   371,   195,   196,
     170,   369,   370,   371,     6,   594,   366,   198,   150,   176,
       8,   198,   595,   117,   580,   581,   582,     9,   300,   173,
     490,   332,   304,   139,   500,   140,   491,   492,   493,   197,
     315,   197,   145,   413,   146,   343,   345,   147,   501,   342,
     342,   198,   495,   438,   408,   496,   497,   449,    28,    29,
     359,    35,   145,   117,   146,   508,   441,   147,   434,    10,
     442,   197,   372,   525,   247,   452,    14,   454,   251,   531,
      16,   145,   148,   146,   463,    48,   147,   464,    22,    49,
     145,   117,   146,    32,    33,   147,    50,    51,    52,    26,
     291,   148,    53,   139,   117,   140,   539,   268,   542,   533,
     543,   544,   534,   545,   394,   546,   395,    44,   409,   177,
     145,   117,   146,    47,    54,   147,    45,    46,    55,    56,
      57,   431,   191,   191,   145,   517,   146,   401,    58,   147,
      93,    94,   547,   548,   435,   503,    59,    60,   215,   117,
     425,   549,   372,   372,    61,   150,   575,   628,   489,   629,
      62,    63,   413,   512,   116,   413,   413,   417,   436,   207,
     208,   177,   145,   568,   146,   444,   117,   147,   446,   430,
     447,   448,   432,   145,   597,   146,   130,   453,   147,   455,
     451,   134,   451,   458,   459,   556,   557,   558,   559,   560,
     561,   562,   563,   533,   577,   168,   534,   174,   135,   468,
     469,   144,   618,   318,   319,   320,   553,   154,   472,   473,
     285,   286,   287,   219,   616,   630,   586,   587,   588,   589,
     590,   591,   592,   277,   278,   305,   306,   165,   195,   196,
     411,   412,   418,   419,    32,   514,    30,   502,   222,   226,
     484,   486,   227,   523,   191,   228,   229,   230,   231,   232,
     233,   234,   235,   498,   236,   237,   518,   238,   239,   240,
     241,   242,   246,   243,   372,   529,   244,   245,    96,   176,
     215,   264,    97,   267,   281,   279,   280,   177,   510,    98,
      99,   100,   282,   283,   284,   101,   289,   290,   294,   504,
     298,   515,   516,   296,    35,   299,   302,   303,   308,   526,
     170,   314,   317,   524,   322,   323,   325,   102,   326,   372,
     312,   103,   104,   105,   569,   327,   329,    32,   177,   330,
     331,   106,   333,   336,   107,   341,   349,   352,   108,   109,
     611,   353,   110,   551,   552,   354,   554,   111,   112,   358,
     360,   362,   596,   113,   114,   363,   607,   572,   608,   612,
     377,   378,   379,   613,   380,   381,   614,   382,   383,   384,
     385,   569,   386,   387,   150,   388,   389,   390,   177,   391,
     415,   397,   400,   402,   405,   406,   407,   410,   428,   609,
     433,   450,   460,   461,   467,   465,   593,   471,   474,   480,
     572,   470,   478,   479,    28,   519,   583,   488,   483,   505,
     506,   511,    35,   521,   292,    72,   532,   259,   599,   540,
     566,   570,   550,   606,   509,   576,   600,   541,   601,   602,
     603,   604,   610,   437,   615,   617,   619,   620,   621,   622,
     623,   624,   625,   626,   627,   631,   177,   426,   499,   457,
      66,   267,   372,   439,   150,   311,   316,   485,   257,   361,
     258,   470,   482,   637,   567,   372,   578,    67,   477,    68,
      69,   598,    70,   632,    71,   635,   636,     0,   528,     0,
     634,   164,     0,     0,   372,     0,    72,    73,     0,    74,
      75,    76,    77,     0,     0,     0,    78,     0,     0,     0,
      79,    80,    81,    82,    83,    66,   267,    84,    85,    86,
      87,     0,     0,     0,     0,     0,    88,     0,    89,    90,
      91,    92,    67,     0,    68,    69,     0,    70,     0,    71,
       0,     0,   339,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,     0,     0,
       0,    78,     0,     0,     0,    79,    80,    81,    82,    83,
      66,     0,    84,    85,    86,    87,     0,     0,     0,     0,
       0,    88,     0,    89,    90,    91,    92,    67,     0,    68,
      69,     0,    70,     0,    71,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,     0,     0,     0,    78,     0,     0,     0,
      79,    80,    81,    82,    83,     0,     0,    84,    85,    86,
      87,     0,     0,     0,     0,     0,    88,     0,    89,    90,
      91,    92
};

static const yytype_int16 yycheck[] =
{
      23,    70,    25,    86,   123,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    82,    61,    62,
      63,   121,    74,   122,    67,   126,    69,   272,    71,   127,
     132,   131,   128,   133,   157,    78,    79,    80,    81,    82,
      83,   204,   305,   306,    87,   197,    89,   151,    91,    92,
       7,   207,   208,     6,   119,    78,   305,   306,     3,    86,
      83,     3,     4,    86,   202,   203,    69,     3,   475,    72,
      73,     6,    75,     7,    77,   118,    79,   120,    83,   324,
     123,     3,     7,   126,   247,     7,     6,    32,     8,     3,
     155,    11,   215,   475,   476,   197,   190,     3,     4,     3,
     194,   124,     6,    28,    25,   268,    41,    69,    70,     6,
     162,     0,   155,    10,   192,   118,   194,    79,    68,   122,
      41,    42,   125,    80,     3,     4,    32,    61,    32,   536,
     224,   214,    82,    79,    87,    77,    78,    79,    60,    61,
      86,    77,    78,    79,     3,    33,   224,   190,   151,     3,
       6,   194,    40,     7,   536,   537,   538,    85,   161,    79,
      13,   188,   165,     6,   427,     8,    19,    20,    21,   192,
     173,   194,     6,   296,     8,   202,   203,    11,   427,   202,
     203,   224,   415,   321,   288,   418,   419,   339,     3,     4,
     217,   214,     6,     7,     8,   440,     6,    11,   317,     3,
      10,   224,   225,   466,   116,   343,    89,   345,   120,   472,
       3,     6,    26,     8,     4,    12,    11,     7,    86,    16,
       6,     7,     8,     3,     4,    11,    23,    24,    25,    84,
      16,    26,    29,     6,     7,     8,   481,   339,    22,     3,
      24,    25,     6,    27,     4,    29,     6,     6,   291,   272,
       6,     7,     8,    72,    51,    11,     3,     4,    55,    56,
      57,   313,   305,   306,     6,     7,     8,   270,    65,    11,
       9,    10,    56,    57,   317,   431,    73,    74,     6,     7,
     307,    65,   305,   306,    81,   288,   531,     4,   411,     6,
      87,    88,   415,   456,     7,   418,   419,   300,   321,    17,
      18,   324,     6,     7,     8,   332,     7,    11,   335,   312,
     337,   338,   315,     6,     7,     8,     6,   344,    11,   346,
     343,     7,   345,   350,   351,    33,    34,    35,    36,    37,
      38,    39,    40,     3,     4,    75,     6,    77,     7,   366,
     367,     7,   605,    52,    53,    54,   502,    28,   375,   376,
      13,    14,    15,     7,   599,   618,   543,   544,   545,   546,
     547,   548,   549,    60,    61,    49,    50,    43,    60,    61,
      17,    18,    30,    31,     3,     4,   399,   429,     6,     4,
     403,   404,     6,   466,   427,     6,     6,     6,     6,     6,
       6,     6,     6,   420,     6,     6,   461,     6,     6,     6,
       6,     6,     4,     6,   427,   470,     6,     6,    12,     3,
       6,     6,    16,     6,     4,     7,     7,   440,   445,    23,
      24,    25,     4,     4,     4,    29,     4,     4,     4,   432,
       4,   458,   459,    30,   457,     4,     4,     4,     4,   466,
      86,     4,    29,   466,     4,     4,     4,    51,     4,   472,
      48,    55,    56,    57,   519,     4,     4,     3,   481,     6,
       4,    65,     4,     4,    68,     4,     4,     4,    72,    73,
     589,     4,    76,   500,   501,     4,   503,    81,    82,     4,
       4,     4,   565,    87,    88,     4,   586,   530,   587,   590,
       4,     4,     4,   591,     4,     4,   592,     4,     4,     4,
       4,   566,     4,     4,   507,     4,     4,     4,   531,     3,
      31,     4,     4,     4,     4,     4,     4,     4,     4,   588,
       4,     4,     4,    66,     4,     6,   553,     6,     3,     6,
     573,    16,     4,     4,     3,    67,   539,     4,     6,     4,
       4,     4,   565,     4,   609,    41,     4,   590,   575,     6,
      67,     4,    18,   585,    32,     6,     6,    57,     4,     4,
       4,     4,   588,   321,     6,     6,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,   599,   309,   426,   348,
       5,     6,   605,   321,   587,   169,   175,   403,   124,   219,
     125,    16,   399,   636,   517,   618,   535,    22,   392,    24,
      25,   573,    27,   630,    29,   633,    76,    -1,   466,    -1,
     633,    73,    -1,    -1,   637,    -1,    41,    42,    -1,    44,
      45,    46,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,     5,     6,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    74,
      75,    76,    22,    -1,    24,    25,    -1,    27,    -1,    29,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    -1,    44,    45,    46,    47,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    55,    56,    57,    58,    59,
       5,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    73,    74,    75,    76,    22,    -1,    24,
      25,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    -1,    44,
      45,    46,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    74,
      75,    76
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,   186,   187,    83,     0,     3,   191,     6,    85,
       3,   208,   188,   192,    89,   189,     3,   194,   209,     3,
       4,   190,    86,   193,   112,   113,    84,   195,     3,     4,
      93,   163,     3,     4,    92,    93,   105,   124,   125,   128,
     130,   148,   178,   179,     6,     3,     4,    72,    12,    16,
      23,    24,    25,    29,    51,    55,    56,    57,    65,    73,
      74,    81,    87,    88,    68,    82,     5,    22,    24,    25,
      27,    29,    41,    42,    44,    45,    46,    47,    51,    55,
      56,    57,    58,    59,    62,    63,    64,    65,    71,    73,
      74,    75,    76,     9,    10,    95,    12,    16,    23,    24,
      25,    29,    51,    55,    56,    57,    65,    68,    72,    73,
      76,    81,    82,    87,    88,   161,     7,     7,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
       6,   143,   143,   143,     7,     7,     6,    10,    91,     6,
       8,    94,   102,   143,     7,     6,     8,    11,    26,    96,
      97,   103,   104,   143,    28,   106,   107,    97,    98,   110,
     143,    97,   120,   123,   123,    43,   121,   132,   120,    79,
      86,   133,   134,    79,   120,   129,     3,    93,   111,   138,
     143,    97,   139,   143,   143,   144,   106,   143,   146,   147,
      98,   143,   149,   150,   205,    60,    61,    93,   143,   152,
     153,   198,   199,   206,    32,    93,   115,    17,    18,   155,
       6,    92,    93,   115,   154,     6,   143,   159,   160,     7,
      61,   143,     6,   143,   207,   143,     4,     6,     6,     6,
       6,     6,     6,     6,     6,     6,     6,     6,     6,     6,
       6,     6,     6,     6,     6,     6,     4,    94,    97,   143,
      98,    94,   143,    95,   104,   110,   143,   138,   139,   143,
     144,   147,   160,   169,     6,    95,   183,     6,   114,   197,
      95,   200,   201,    69,    70,    79,   162,    60,    61,     7,
       7,     4,     4,     4,     4,    13,    14,    15,   103,     4,
       4,    16,    98,   143,     4,    96,    30,   108,     4,     4,
      97,   121,     4,     4,    97,    49,    50,   131,     4,   121,
     127,   133,    48,   135,     4,    97,   127,    29,    52,    53,
      54,   137,     4,     4,   140,     4,     4,     4,   145,     4,
       6,     4,   115,     4,   206,   199,     4,   199,   206,    87,
     197,     4,    93,   115,   124,   115,   124,   112,   114,     4,
     131,   131,     4,     4,     4,    92,    96,   156,     4,   115,
       4,   153,     4,     4,   170,   180,   199,   206,     4,    77,
      78,    79,    93,    99,   175,   177,   112,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     3,   167,   168,     4,     6,   184,     4,   112,   196,
       4,    97,     4,   111,   202,     4,     4,     4,   103,   143,
       4,    17,    18,    96,   101,    31,   109,    97,    30,    31,
     122,    99,   205,    99,   205,   115,   108,   126,     4,   136,
      97,   121,    97,     4,   110,   143,    93,   105,   124,   125,
     111,     6,    10,   142,   115,   151,   115,   115,   115,   197,
       4,    93,   124,   115,   124,   115,   116,   113,   115,   115,
       4,    66,   157,     4,     7,     6,   172,     4,   115,   115,
      16,     6,   115,   115,     3,   165,   166,   167,     4,     4,
       6,   181,   163,     6,    93,   148,    93,   203,     4,    96,
      13,    19,    20,    21,   100,   101,   101,   101,   115,   109,
      99,   205,   121,   131,    97,     4,     4,    25,   111,    32,
     115,     4,   112,   117,     4,   115,   115,     7,    98,    67,
     158,     4,     4,    92,    93,    99,   115,   171,   198,    98,
     176,    99,     4,     3,     6,   164,   165,   166,   166,   111,
       6,    57,    22,    24,    25,    27,    29,    56,    57,    65,
      18,   115,   115,   131,   115,   141,    33,    34,    35,    36,
      37,    38,    39,    40,   118,   119,    67,   158,     7,    98,
       4,    80,   143,   173,   174,   111,     6,     4,   164,   165,
     166,   166,   166,    97,   143,   204,   204,   204,   204,   204,
     204,   204,   204,   115,    33,    40,    92,     7,   173,   115,
       6,     4,     4,     4,     4,   185,   102,    95,   104,   106,
     107,   110,   144,   147,   160,     6,   111,     6,    99,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     6,
      99,     4,   115,   182,    93,   178,    76,   143
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    90,    91,    92,    93,    94,    94,    91,    95,    95,
      96,    96,    96,    97,    97,    97,    98,    98,    98,    98,
      99,   100,   100,   100,   100,   101,   102,    92,   103,   103,
     104,   105,    92,   106,   106,   107,    92,   108,   108,   109,
     109,   110,    92,   111,   112,   113,   113,   114,   114,   115,
     116,   115,   117,   117,   118,   118,   118,   118,   118,   118,
     118,   119,   118,   120,   121,   121,   122,   122,   122,   123,
     124,   125,    92,    92,   126,   127,   127,   128,   129,   129,
     130,   131,   131,   131,   131,   132,   128,   133,   133,   135,
     134,   136,   134,   130,    92,    92,   137,   137,   137,   137,
     138,   138,   138,    92,   140,   139,    92,   141,   141,   142,
     142,   143,   145,   144,    92,   146,   146,   147,   147,   147,
     148,    92,   149,   150,   150,   151,   150,   150,    92,   152,
     152,   152,   152,   152,   152,    92,   153,   153,   152,    92,
     154,    92,    92,    92,   155,   155,    92,   156,   156,   157,
     157,   158,   158,   159,   159,   159,   159,   160,    92,   161,
     162,   162,   162,   162,    92,    92,    92,   163,   164,   164,
     164,   165,   165,   166,   167,   167,   167,   168,   168,   169,
     163,    92,    92,   163,   163,    92,   170,   170,   171,   171,
     171,   171,   172,   172,   173,   173,   174,   176,   175,   177,
     177,   177,   177,   178,   179,    92,    92,   180,   180,    92,
     181,   181,   182,   182,   184,   185,   183,    92,    92,   186,
     188,   187,   189,   189,   190,   192,   191,   193,   193,   194,
     195,   195,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   196,   196,   197,
     198,   199,   199,   200,   201,   202,   202,   203,   203,   203,
     203,   203,   203,   203,   203,   204,   205,   206,   207,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
     111,   209,   208
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
       1,     1,     1,     0,     5,     5,     6,     7,     6,     5,
       1,     4,     3,     0,     5,     5,     5,     2,     4,     0,
       6,     4,     4,     5,     6,     6,     0,     2,     1,     1,
       1,     1,     0,     2,     1,     1,     3,     0,     4,     1,
       1,     1,     1,    10,     4,     1,     1,     2,     2,     5,
       0,     1,     0,     2,     0,     0,    10,     5,     5,     3,
       0,     6,     0,     2,     5,     0,     6,     0,     2,     4,
       0,     4,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
       5,     5,     5,     5,     5,     5,     5,     1,     2,     2,
       4,     1,     3,     2,     4,     2,     2,     5,     5,     5,
       5,     5,     5,     5,     5,     1,     1,     1,     1,     4,
       5,     6,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     0,     5
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
#line 276 "parser.y"
                        {
	#line 421 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2097 "parser.c"
    break;

  case 3: /* content_node: start "glyph" glyph ">"  */
#line 279 "parser.y"
                                  {
	#line 422 "format.w"
	hput_tags((yyvsp[-3].u),hput_glyph(&((yyvsp[-1].c))));}
#line 2105 "parser.c"
    break;

  case 4: /* start: "<"  */
#line 282 "parser.y"
           {
	#line 423 "format.w"
	HPUTNODE;(yyval.u)= (uint32_t)(hpos++-hstart);}
#line 2113 "parser.c"
    break;

  case 6: /* integer: UNSIGNED  */
#line 286 "parser.y"
                         {
	#line 953 "format.w"
	RNG("number",(yyvsp[0].u),0,0x7FFFFFFF);}
#line 2121 "parser.c"
    break;

  case 7: /* glyph: CHARCODE REFERENCE  */
#line 290 "parser.y"
                        {
	#line 1095 "format.w"
	(yyval.c).c= (yyvsp[-1].u);REF(font_kind,(yyvsp[0].u));(yyval.c).f= (yyvsp[0].u);}
#line 2129 "parser.c"
    break;

  case 9: /* string: CHARCODE  */
#line 294 "parser.y"
                         {
	#line 1200 "format.w"
	static char s[2];
	RNG("String element",(yyvsp[0].u),0x20,0x7E);
	s[0]= (yyvsp[0].u);s[1]= 0;(yyval.s)= s;}
#line 2139 "parser.c"
    break;

  case 10: /* number: UNSIGNED  */
#line 300 "parser.y"
               {
	#line 1353 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].u);}
#line 2147 "parser.c"
    break;

  case 11: /* number: SIGNED  */
#line 302 "parser.y"
                                       {
	#line 1353 "format.w"
	(yyval.f)= (float64_t)(yyvsp[0].i);}
#line 2155 "parser.c"
    break;

  case 13: /* dimension: number "pt"  */
#line 306 "parser.y"
                   {
	#line 1707 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2163 "parser.c"
    break;

  case 14: /* dimension: number "in"  */
#line 309 "parser.y"
                    {
	#line 1708 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*72.27);RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2171 "parser.c"
    break;

  case 15: /* dimension: number "mm"  */
#line 312 "parser.y"
                  {
	#line 1709 "format.w"
	(yyval.d)= ROUND((yyvsp[-1].f)*ONE*(72.27/25.4));RNG("Dimension",(yyval.d),-MAX_DIMEN,MAX_DIMEN);}
#line 2179 "parser.c"
    break;

  case 16: /* xdimen: dimension number "h" number "v"  */
#line 316 "parser.y"
                                  {
	#line 1787 "format.w"
	(yyval.xd).w= (yyvsp[-4].d);(yyval.xd).h= (yyvsp[-3].f);(yyval.xd).v= (yyvsp[-1].f);}
#line 2187 "parser.c"
    break;

  case 17: /* xdimen: dimension number "h"  */
#line 319 "parser.y"
                           {
	#line 1788 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= (yyvsp[-1].f);(yyval.xd).v= 0.0;}
#line 2195 "parser.c"
    break;

  case 18: /* xdimen: dimension number "v"  */
#line 322 "parser.y"
                           {
	#line 1789 "format.w"
	(yyval.xd).w= (yyvsp[-2].d);(yyval.xd).h= 0.0;(yyval.xd).v= (yyvsp[-1].f);}
#line 2203 "parser.c"
    break;

  case 19: /* xdimen: dimension  */
#line 325 "parser.y"
                  {
	#line 1790 "format.w"
	(yyval.xd).w= (yyvsp[0].d);(yyval.xd).h= 0.0;(yyval.xd).v= 0.0;}
#line 2211 "parser.c"
    break;

  case 20: /* xdimen_node: start "xdimen" xdimen ">"  */
#line 329 "parser.y"
                                   {
	#line 1792 "format.w"
	hput_tags((yyvsp[-3].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 2219 "parser.c"
    break;

  case 21: /* order: "pt"  */
#line 334 "parser.y"
        {
	#line 1971 "format.w"
	(yyval.o)= normal_o;}
#line 2227 "parser.c"
    break;

  case 22: /* order: "fil"  */
#line 336 "parser.y"
                            {
	#line 1971 "format.w"
	(yyval.o)= fil_o;}
#line 2235 "parser.c"
    break;

  case 23: /* order: "fill"  */
#line 338 "parser.y"
                             {
	#line 1971 "format.w"
	(yyval.o)= fill_o;}
#line 2243 "parser.c"
    break;

  case 24: /* order: "filll"  */
#line 340 "parser.y"
                              {
	#line 1971 "format.w"
	(yyval.o)= filll_o;}
#line 2251 "parser.c"
    break;

  case 25: /* stretch: number order  */
#line 344 "parser.y"
                    {
	#line 1973 "format.w"
	(yyval.st).f= (yyvsp[-1].f);(yyval.st).o= (yyvsp[0].o);}
#line 2259 "parser.c"
    break;

  case 26: /* penalty: integer  */
#line 348 "parser.y"
               {
	#line 2027 "format.w"
	RNG("Penalty",(yyvsp[0].i),-20000,+20000);(yyval.i)= (yyvsp[0].i);}
#line 2267 "parser.c"
    break;

  case 27: /* content_node: start "penalty" penalty ">"  */
#line 351 "parser.y"
                                      {
	#line 2028 "format.w"
	hput_tags((yyvsp[-3].u),hput_int((yyvsp[-1].i)));}
#line 2275 "parser.c"
    break;

  case 29: /* rule_dimension: "|"  */
#line 355 "parser.y"
                                        {
	#line 2205 "format.w"
	(yyval.d)= RUNNING_DIMEN;}
#line 2283 "parser.c"
    break;

  case 30: /* rule: rule_dimension rule_dimension rule_dimension  */
#line 359 "parser.y"
{
	#line 2207 "format.w"
	(yyval.r).h= (yyvsp[-2].d);(yyval.r).d= (yyvsp[-1].d);(yyval.r).w= (yyvsp[0].d);
	if((yyvsp[0].d)==RUNNING_DIMEN&&((yyvsp[-2].d)==RUNNING_DIMEN||(yyvsp[-1].d)==RUNNING_DIMEN))
	QUIT("Incompatible running dimensions 0x%x 0x%x 0x%x",(yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2293 "parser.c"
    break;

  case 31: /* rule_node: start "rule" rule ">"  */
#line 364 "parser.y"
                             {
	#line 2210 "format.w"
	hput_tags((yyvsp[-3].u),hput_rule(&((yyvsp[-1].r))));}
#line 2301 "parser.c"
    break;

  case 33: /* explicit: %empty  */
#line 369 "parser.y"
         {
	#line 2318 "format.w"
	(yyval.b)= false;}
#line 2309 "parser.c"
    break;

  case 34: /* explicit: "!"  */
#line 371 "parser.y"
                                 {
	#line 2318 "format.w"
	(yyval.b)= true;}
#line 2317 "parser.c"
    break;

  case 35: /* kern: explicit xdimen  */
#line 374 "parser.y"
                    {
	#line 2319 "format.w"
	(yyval.kt).x= (yyvsp[-1].b);(yyval.kt).d= (yyvsp[0].xd);}
#line 2325 "parser.c"
    break;

  case 36: /* content_node: start "kern" kern ">"  */
#line 377 "parser.y"
                                {
	#line 2320 "format.w"
	hput_tags((yyvsp[-3].u),hput_kern(&((yyvsp[-1].kt))));}
#line 2333 "parser.c"
    break;

  case 37: /* plus: %empty  */
#line 381 "parser.y"
     {
	#line 2530 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2341 "parser.c"
    break;

  case 38: /* plus: "plus" stretch  */
#line 383 "parser.y"
                                             {
	#line 2530 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2349 "parser.c"
    break;

  case 39: /* minus: %empty  */
#line 386 "parser.y"
      {
	#line 2531 "format.w"
	(yyval.st).f= 0.0;(yyval.st).o= 0;}
#line 2357 "parser.c"
    break;

  case 40: /* minus: "minus" stretch  */
#line 388 "parser.y"
                                              {
	#line 2531 "format.w"
	(yyval.st)= (yyvsp[0].st);}
#line 2365 "parser.c"
    break;

  case 41: /* glue: xdimen plus minus  */
#line 391 "parser.y"
                      {
	#line 2532 "format.w"
	(yyval.g).w= (yyvsp[-2].xd);(yyval.g).p= (yyvsp[-1].st);(yyval.g).m= (yyvsp[0].st);}
#line 2373 "parser.c"
    break;

  case 42: /* content_node: start "glue" glue ">"  */
#line 394 "parser.y"
                                {
	#line 2533 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){HPUT8(zero_skip_no);
	hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}else hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));}
#line 2382 "parser.c"
    break;

  case 43: /* glue_node: start "glue" glue ">"  */
#line 399 "parser.y"
{
	#line 2536 "format.w"
	if(ZERO_GLUE((yyvsp[-1].g))){hpos--;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),hput_glue(&((yyvsp[-1].g))));(yyval.b)= true;}}
#line 2391 "parser.c"
    break;

  case 44: /* position: %empty  */
#line 404 "parser.y"
         {
	#line 2811 "format.w"
	(yyval.u)= hpos-hstart;}
#line 2399 "parser.c"
    break;

  case 47: /* estimate: %empty  */
#line 409 "parser.y"
         {
	#line 2814 "format.w"
	hpos+= 2;}
#line 2407 "parser.c"
    break;

  case 48: /* estimate: UNSIGNED  */
#line 412 "parser.y"
                 {
	#line 2815 "format.w"
	hpos+= hsize_bytes((yyvsp[0].u))+1;}
#line 2415 "parser.c"
    break;

  case 49: /* list: start estimate content_list ">"  */
#line 416 "parser.y"
{
	#line 2817 "format.w"
	(yyval.l).t= TAG(list_kind,b010);(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2424 "parser.c"
    break;

  case 50: /* $@1: %empty  */
#line 422 "parser.y"
{
	#line 3231 "format.w"
	hpos+= 4;}
#line 2432 "parser.c"
    break;

  case 51: /* list: TXT_START position $@1 text TXT_END  */
#line 426 "parser.y"
{
	#line 3233 "format.w"
	(yyval.l).t= TAG(list_kind,b110);(yyval.l).p= (yyvsp[-1].u);(yyval.l).s= (hpos-hstart)-(yyvsp[-1].u);
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyval.l))));}
#line 2441 "parser.c"
    break;

  case 54: /* txt: TXT_CC  */
#line 432 "parser.y"
          {
	#line 3237 "format.w"
	hput_txt_cc((yyvsp[0].u));}
#line 2449 "parser.c"
    break;

  case 55: /* txt: TXT_FONT  */
#line 435 "parser.y"
                 {
	#line 3238 "format.w"
	REF(font_kind,(yyvsp[0].u));hput_txt_font((yyvsp[0].u));}
#line 2457 "parser.c"
    break;

  case 56: /* txt: TXT_GLOBAL  */
#line 438 "parser.y"
                   {
	#line 3239 "format.w"
	REF((yyvsp[0].rf).k,(yyvsp[0].rf).n);hput_txt_global(&((yyvsp[0].rf)));}
#line 2465 "parser.c"
    break;

  case 57: /* txt: TXT_LOCAL  */
#line 441 "parser.y"
                  {
	#line 3240 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,11);hput_txt_local((yyvsp[0].u));}
#line 2473 "parser.c"
    break;

  case 58: /* txt: TXT_FONT_GLUE  */
#line 444 "parser.y"
                      {
	#line 3241 "format.w"
	HPUTX(1);HPUT8(txt_glue);}
#line 2481 "parser.c"
    break;

  case 59: /* txt: TXT_FONT_HYPHEN  */
#line 447 "parser.y"
                        {
	#line 3242 "format.w"
	HPUTX(1);HPUT8(txt_hyphen);}
#line 2489 "parser.c"
    break;

  case 60: /* txt: TXT_IGNORE  */
#line 450 "parser.y"
                   {
	#line 3243 "format.w"
	HPUTX(1);HPUT8(txt_ignore);}
#line 2497 "parser.c"
    break;

  case 61: /* $@2: %empty  */
#line 453 "parser.y"
         {
	#line 3244 "format.w"
	HPUTX(1);HPUT8(txt_node);}
#line 2505 "parser.c"
    break;

  case 63: /* box_dimen: dimension dimension dimension  */
#line 459 "parser.y"
{
	#line 3500 "format.w"
	(yyval.info)= hput_box_dimen((yyvsp[-2].d),(yyvsp[-1].d),(yyvsp[0].d));}
#line 2513 "parser.c"
    break;

  case 64: /* box_shift: %empty  */
#line 462 "parser.y"
          {
	#line 3501 "format.w"
	(yyval.info)= b000;}
#line 2521 "parser.c"
    break;

  case 65: /* box_shift: "shifted" dimension  */
#line 465 "parser.y"
                          {
	#line 3502 "format.w"
	(yyval.info)= hput_box_shift((yyvsp[0].d));}
#line 2529 "parser.c"
    break;

  case 66: /* box_glue_set: %empty  */
#line 469 "parser.y"
             {
	#line 3504 "format.w"
	(yyval.info)= b000;}
#line 2537 "parser.c"
    break;

  case 67: /* box_glue_set: "plus" stretch  */
#line 472 "parser.y"
                     {
	#line 3505 "format.w"
	(yyval.info)= hput_box_glue_set(+1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2545 "parser.c"
    break;

  case 68: /* box_glue_set: "minus" stretch  */
#line 475 "parser.y"
                      {
	#line 3506 "format.w"
	(yyval.info)= hput_box_glue_set(-1,(yyvsp[0].st).f,(yyvsp[0].st).o);}
#line 2553 "parser.c"
    break;

  case 69: /* box: box_dimen box_shift box_glue_set list  */
#line 480 "parser.y"
                                         {
	#line 3509 "format.w"
	(yyval.info)= (yyvsp[-3].info)	|(yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2561 "parser.c"
    break;

  case 70: /* hbox_node: start "hbox" box ">"  */
#line 484 "parser.y"
                            {
	#line 3511 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hbox_kind,(yyvsp[-1].info)));}
#line 2569 "parser.c"
    break;

  case 71: /* vbox_node: start "vbox" box ">"  */
#line 487 "parser.y"
                            {
	#line 3512 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vbox_kind,(yyvsp[-1].info)));}
#line 2577 "parser.c"
    break;

  case 74: /* box_flex: plus minus  */
#line 492 "parser.y"
                   {
	#line 3708 "format.w"
	hput_stretch(&((yyvsp[-1].st)));hput_stretch(&((yyvsp[0].st)));}
#line 2585 "parser.c"
    break;

  case 75: /* box_options: box_shift box_flex xdimen_ref list  */
#line 496 "parser.y"
                                              {
	#line 3710 "format.w"
	(yyval.info)= (yyvsp[-3].info);}
#line 2593 "parser.c"
    break;

  case 76: /* box_options: box_shift box_flex xdimen_node list  */
#line 499 "parser.y"
                                            {
	#line 3711 "format.w"
	(yyval.info)= (yyvsp[-3].info)	|b100;}
#line 2601 "parser.c"
    break;

  case 77: /* hxbox_node: start "hset" box_dimen box_options ">"  */
#line 503 "parser.y"
                                               {
	#line 3713 "format.w"
	hput_tags((yyvsp[-4].u),TAG(hset_kind,(yyvsp[-2].info)	|(yyvsp[-1].info)));}
#line 2609 "parser.c"
    break;

  case 79: /* vbox_dimen: "top" dimension dimension dimension  */
#line 509 "parser.y"
{
	#line 3717 "format.w"
	(yyval.info)= hput_box_dimen((yyvsp[-2].d),(yyvsp[-1].d)^0x40000000,(yyvsp[0].d));}
#line 2617 "parser.c"
    break;

  case 80: /* vxbox_node: start "vset" vbox_dimen box_options ">"  */
#line 513 "parser.y"
                                                {
	#line 3719 "format.w"
	hput_tags((yyvsp[-4].u),TAG(vset_kind,(yyvsp[-2].info)	|(yyvsp[-1].info)));}
#line 2625 "parser.c"
    break;

  case 81: /* box_goal: "to" xdimen_ref  */
#line 517 "parser.y"
                      {
	#line 3721 "format.w"
	(yyval.info)= b000;}
#line 2633 "parser.c"
    break;

  case 82: /* box_goal: "add" xdimen_ref  */
#line 520 "parser.y"
                       {
	#line 3722 "format.w"
	(yyval.info)= b001;}
#line 2641 "parser.c"
    break;

  case 83: /* box_goal: "to" xdimen_node  */
#line 523 "parser.y"
                       {
	#line 3723 "format.w"
	(yyval.info)= b100;}
#line 2649 "parser.c"
    break;

  case 84: /* box_goal: "add" xdimen_node  */
#line 526 "parser.y"
                        {
	#line 3724 "format.w"
	(yyval.info)= b101;}
#line 2657 "parser.c"
    break;

  case 85: /* hpack: box_shift box_goal list  */
#line 530 "parser.y"
                             {
	#line 3726 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 2665 "parser.c"
    break;

  case 86: /* hxbox_node: start "hpack" hpack ">"  */
#line 534 "parser.y"
                                {
	#line 3728 "format.w"
	hput_tags((yyvsp[-3].u),TAG(hpack_kind,(yyvsp[-1].info)));}
#line 2673 "parser.c"
    break;

  case 87: /* max_depth: %empty  */
#line 538 "parser.y"
          {
	#line 3730 "format.w"
	(yyval.d)= MAX_DIMEN;}
#line 2681 "parser.c"
    break;

  case 88: /* max_depth: "max" "depth" dimension  */
#line 540 "parser.y"
                                            {
	#line 3730 "format.w"
	(yyval.d)= (yyvsp[0].d);}
#line 2689 "parser.c"
    break;

  case 89: /* $@3: %empty  */
#line 544 "parser.y"
               {
	#line 3732 "format.w"
	HPUT32((yyvsp[0].d));}
#line 2697 "parser.c"
    break;

  case 90: /* vpack: max_depth $@3 box_shift box_goal list  */
#line 546 "parser.y"
                                           {
	#line 3732 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2705 "parser.c"
    break;

  case 91: /* $@4: %empty  */
#line 549 "parser.y"
                      {
	#line 3733 "format.w"
	HPUT32((yyvsp[0].d)^0x40000000);}
#line 2713 "parser.c"
    break;

  case 92: /* vpack: "top" max_depth $@4 box_shift box_goal list  */
#line 551 "parser.y"
                                                      {
	#line 3733 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|(yyvsp[-1].info);}
#line 2721 "parser.c"
    break;

  case 93: /* vxbox_node: start "vpack" vpack ">"  */
#line 555 "parser.y"
                                {
	#line 3735 "format.w"
	hput_tags((yyvsp[-3].u),TAG(vpack_kind,(yyvsp[-1].info)));}
#line 2729 "parser.c"
    break;

  case 96: /* ltype: %empty  */
#line 561 "parser.y"
      {
	#line 3849 "format.w"
	(yyval.info)= 1;}
#line 2737 "parser.c"
    break;

  case 97: /* ltype: "align"  */
#line 563 "parser.y"
                      {
	#line 3849 "format.w"
	(yyval.info)= 1;}
#line 2745 "parser.c"
    break;

  case 98: /* ltype: "center"  */
#line 565 "parser.y"
                       {
	#line 3849 "format.w"
	(yyval.info)= 2;}
#line 2753 "parser.c"
    break;

  case 99: /* ltype: "expand"  */
#line 567 "parser.y"
                       {
	#line 3849 "format.w"
	(yyval.info)= 3;}
#line 2761 "parser.c"
    break;

  case 100: /* leaders: glue_node ltype rule_node  */
#line 570 "parser.y"
                                 {
	#line 3850 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2769 "parser.c"
    break;

  case 101: /* leaders: glue_node ltype hbox_node  */
#line 573 "parser.y"
                                  {
	#line 3851 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2777 "parser.c"
    break;

  case 102: /* leaders: glue_node ltype vbox_node  */
#line 576 "parser.y"
                                  {
	#line 3852 "format.w"
	if((yyvsp[-2].b))(yyval.info)= (yyvsp[-1].info)	|b100;else (yyval.info)= (yyvsp[-1].info);}
#line 2785 "parser.c"
    break;

  case 103: /* content_node: start "leaders" leaders ">"  */
#line 579 "parser.y"
                                      {
	#line 3853 "format.w"
	hput_tags((yyvsp[-3].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 2793 "parser.c"
    break;

  case 104: /* $@5: %empty  */
#line 583 "parser.y"
                  {
	#line 3959 "format.w"
	if((yyvsp[0].d)!=0)HPUT32((yyvsp[0].d));}
#line 2801 "parser.c"
    break;

  case 105: /* baseline: dimension $@5 glue_node glue_node  */
#line 586 "parser.y"
                   {
	#line 3960 "format.w"
	(yyval.info)= b000;if((yyvsp[-3].d)!=0)(yyval.info)	|= b001;
	if((yyvsp[-1].b))(yyval.info)	|= b100;
	if((yyvsp[0].b))(yyval.info)	|= b010;
	}
#line 2812 "parser.c"
    break;

  case 106: /* content_node: start "baseline" baseline ">"  */
#line 593 "parser.y"
{
	#line 3965 "format.w"
	if((yyvsp[-1].info)==b000)HPUT8(0);hput_tags((yyvsp[-3].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 2820 "parser.c"
    break;

  case 108: /* cc_list: cc_list TXT_CC  */
#line 597 "parser.y"
                               {
	#line 4048 "format.w"
	hput_utf8((yyvsp[0].u));}
#line 2828 "parser.c"
    break;

  case 109: /* lig_cc: UNSIGNED  */
#line 600 "parser.y"
               {
	#line 4049 "format.w"
	RNG("UTF-8 code",(yyvsp[0].u),0,0x1FFFFF);(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2836 "parser.c"
    break;

  case 110: /* lig_cc: CHARCODE  */
#line 603 "parser.y"
               {
	#line 4050 "format.w"
	(yyval.u)= hpos-hstart;hput_utf8((yyvsp[0].u));}
#line 2844 "parser.c"
    break;

  case 111: /* ref: REFERENCE  */
#line 606 "parser.y"
             {
	#line 4051 "format.w"
	HPUT8((yyvsp[0].u));(yyval.u)= (yyvsp[0].u);}
#line 2852 "parser.c"
    break;

  case 112: /* $@6: %empty  */
#line 609 "parser.y"
            {
	#line 4052 "format.w"
	REF(font_kind,(yyvsp[0].u));}
#line 2860 "parser.c"
    break;

  case 113: /* ligature: ref $@6 lig_cc TXT_START cc_list TXT_END  */
#line 612 "parser.y"
{
	#line 4053 "format.w"
	(yyval.lg).f= (yyvsp[-5].u);(yyval.lg).l.p= (yyvsp[-3].u);(yyval.lg).l.s= (hpos-hstart)-(yyvsp[-3].u);
	RNG("Ligature size",(yyval.lg).l.s,0,255);}
#line 2869 "parser.c"
    break;

  case 114: /* content_node: start "ligature" ligature ">"  */
#line 616 "parser.y"
                                        {
	#line 4055 "format.w"
	hput_tags((yyvsp[-3].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 2877 "parser.c"
    break;

  case 115: /* replace_count: explicit  */
#line 620 "parser.y"
                      {
	#line 4165 "format.w"
	if((yyvsp[0].b)){(yyval.u)= 0x80;HPUT8(0x80);}else (yyval.u)= 0x00;}
#line 2885 "parser.c"
    break;

  case 116: /* replace_count: explicit UNSIGNED  */
#line 623 "parser.y"
                          {
	#line 4166 "format.w"
	RNG("Replace count",(yyvsp[0].u),0,31);
	(yyval.u)= ((yyvsp[0].u))	|(((yyvsp[-1].b))?0x80:0x00);if((yyval.u)!=0)HPUT8((yyval.u));}
#line 2894 "parser.c"
    break;

  case 117: /* disc: replace_count list list  */
#line 627 "parser.y"
                            {
	#line 4168 "format.w"
	(yyval.dc).r= (yyvsp[-2].u);(yyval.dc).p= (yyvsp[-1].l);(yyval.dc).q= (yyvsp[0].l);
	if((yyvsp[0].l).s==0){hpos= hpos-3;if((yyvsp[-1].l).s==0)hpos= hpos-3;}}
#line 2903 "parser.c"
    break;

  case 118: /* disc: replace_count list  */
#line 631 "parser.y"
                           {
	#line 4170 "format.w"
	(yyval.dc).r= (yyvsp[-1].u);(yyval.dc).p= (yyvsp[0].l);if((yyvsp[0].l).s==0)hpos= hpos-3;(yyval.dc).q.s= 0;}
#line 2911 "parser.c"
    break;

  case 119: /* disc: replace_count  */
#line 634 "parser.y"
                      {
	#line 4171 "format.w"
	(yyval.dc).r= (yyvsp[0].u);(yyval.dc).p.s= 0;(yyval.dc).q.s= 0;}
#line 2919 "parser.c"
    break;

  case 120: /* disc_node: start "disc" disc ">"  */
#line 640 "parser.y"
{
	#line 4175 "format.w"
	hput_tags((yyvsp[-3].u),hput_disc(&((yyvsp[-1].dc))));}
#line 2927 "parser.c"
    break;

  case 122: /* par_dimen: xdimen  */
#line 646 "parser.y"
                {
	#line 4327 "format.w"
	hput_xdimen_node(&((yyvsp[0].xd)));}
#line 2935 "parser.c"
    break;

  case 123: /* par: xdimen_ref param_ref list  */
#line 649 "parser.y"
                             {
	#line 4328 "format.w"
	(yyval.info)= b000;}
#line 2943 "parser.c"
    break;

  case 124: /* par: xdimen_ref param_list list  */
#line 652 "parser.y"
                                   {
	#line 4329 "format.w"
	(yyval.info)= b010;}
#line 2951 "parser.c"
    break;

  case 125: /* $@7: %empty  */
#line 655 "parser.y"
                         {
	#line 4330 "format.w"
	hput_xdimen_node(&((yyvsp[-1].xd)));}
#line 2959 "parser.c"
    break;

  case 126: /* par: xdimen param_ref $@7 list  */
#line 657 "parser.y"
                                     {
	#line 4330 "format.w"
	(yyval.info)= b100;}
#line 2967 "parser.c"
    break;

  case 127: /* par: par_dimen param_list list  */
#line 660 "parser.y"
                                  {
	#line 4331 "format.w"
	(yyval.info)= b110;}
#line 2975 "parser.c"
    break;

  case 128: /* content_node: start "par" par ">"  */
#line 664 "parser.y"
                              {
	#line 4333 "format.w"
	hput_tags((yyvsp[-3].u),TAG(par_kind,(yyvsp[-1].info)));}
#line 2983 "parser.c"
    break;

  case 129: /* math: param_ref list  */
#line 668 "parser.y"
                   {
	#line 4399 "format.w"
	(yyval.info)= b000;}
#line 2991 "parser.c"
    break;

  case 130: /* math: param_ref list hbox_node  */
#line 671 "parser.y"
                                 {
	#line 4400 "format.w"
	(yyval.info)= b001;}
#line 2999 "parser.c"
    break;

  case 131: /* math: param_ref hbox_node list  */
#line 674 "parser.y"
                                 {
	#line 4401 "format.w"
	(yyval.info)= b010;}
#line 3007 "parser.c"
    break;

  case 132: /* math: param_list list  */
#line 677 "parser.y"
                        {
	#line 4402 "format.w"
	(yyval.info)= b100;}
#line 3015 "parser.c"
    break;

  case 133: /* math: param_list list hbox_node  */
#line 680 "parser.y"
                                  {
	#line 4403 "format.w"
	(yyval.info)= b101;}
#line 3023 "parser.c"
    break;

  case 134: /* math: param_list hbox_node list  */
#line 683 "parser.y"
                                  {
	#line 4404 "format.w"
	(yyval.info)= b110;}
#line 3031 "parser.c"
    break;

  case 135: /* content_node: start "math" math ">"  */
#line 687 "parser.y"
                                {
	#line 4406 "format.w"
	hput_tags((yyvsp[-3].u),TAG(math_kind,(yyvsp[-1].info)));}
#line 3039 "parser.c"
    break;

  case 136: /* on_off: "on"  */
#line 691 "parser.y"
         {
	#line 4456 "format.w"
	(yyval.i)= 1;}
#line 3047 "parser.c"
    break;

  case 137: /* on_off: "off"  */
#line 693 "parser.y"
                    {
	#line 4456 "format.w"
	(yyval.i)= 0;}
#line 3055 "parser.c"
    break;

  case 138: /* math: on_off  */
#line 696 "parser.y"
           {
	#line 4457 "format.w"
	(yyval.info)= b011	|((yyvsp[0].i)<<2);}
#line 3063 "parser.c"
    break;

  case 139: /* content_node: start "adjust" list ">"  */
#line 700 "parser.y"
                                  {
	#line 4488 "format.w"
	hput_tags((yyvsp[-3].u),TAG(adjust_kind,1));}
#line 3071 "parser.c"
    break;

  case 140: /* span_count: UNSIGNED  */
#line 704 "parser.y"
                   {
	#line 4587 "format.w"
	(yyval.info)= hput_span_count((yyvsp[0].u));}
#line 3079 "parser.c"
    break;

  case 141: /* content_node: start "item" content_node ">"  */
#line 707 "parser.y"
                                        {
	#line 4588 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,1));}
#line 3087 "parser.c"
    break;

  case 142: /* content_node: start "item" span_count content_node ">"  */
#line 710 "parser.y"
                                                   {
	#line 4589 "format.w"
	hput_tags((yyvsp[-4].u),TAG(item_kind,(yyvsp[-2].info)));}
#line 3095 "parser.c"
    break;

  case 143: /* content_node: start "item" list ">"  */
#line 713 "parser.y"
                                {
	#line 4590 "format.w"
	hput_tags((yyvsp[-3].u),TAG(item_kind,b000));}
#line 3103 "parser.c"
    break;

  case 144: /* table: "h" box_goal list list  */
#line 717 "parser.y"
                          {
	#line 4592 "format.w"
	(yyval.info)= (yyvsp[-2].info);}
#line 3111 "parser.c"
    break;

  case 145: /* table: "v" box_goal list list  */
#line 720 "parser.y"
                          {
	#line 4593 "format.w"
	(yyval.info)= (yyvsp[-2].info)	|b010;}
#line 3119 "parser.c"
    break;

  case 146: /* content_node: start "table" table ">"  */
#line 724 "parser.y"
                                  {
	#line 4595 "format.w"
	hput_tags((yyvsp[-3].u),TAG(table_kind,(yyvsp[-1].info)));}
#line 3127 "parser.c"
    break;

  case 147: /* image_aspect: number  */
#line 728 "parser.y"
                   {
	#line 4733 "format.w"
	(yyval.f)= (yyvsp[0].f);}
#line 3135 "parser.c"
    break;

  case 148: /* image_aspect: %empty  */
#line 730 "parser.y"
                         {
	#line 4733 "format.w"
	(yyval.f)= 0.0;}
#line 3143 "parser.c"
    break;

  case 149: /* image_width: "width" xdimen  */
#line 733 "parser.y"
                        {
	#line 4734 "format.w"
	(yyval.xd)= (yyvsp[0].xd);}
#line 3151 "parser.c"
    break;

  case 150: /* image_width: %empty  */
#line 736 "parser.y"
         {
	#line 4735 "format.w"
	(yyval.xd)= xdimen_defaults[zero_xdimen_no];}
#line 3159 "parser.c"
    break;

  case 151: /* image_height: "height" xdimen  */
#line 739 "parser.y"
                          {
	#line 4736 "format.w"
	(yyval.xd)= (yyvsp[0].xd);}
#line 3167 "parser.c"
    break;

  case 152: /* image_height: %empty  */
#line 742 "parser.y"
         {
	#line 4737 "format.w"
	(yyval.xd)= xdimen_defaults[zero_xdimen_no];}
#line 3175 "parser.c"
    break;

  case 153: /* image_spec: UNSIGNED image_aspect image_width image_height  */
#line 747 "parser.y"
{
	#line 4740 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-3].u),(yyvsp[-2].f),0,&((yyvsp[-1].xd)),0,&((yyvsp[0].xd)));}
#line 3183 "parser.c"
    break;

  case 154: /* image_spec: UNSIGNED image_aspect "width" REFERENCE image_height  */
#line 751 "parser.y"
{
	#line 4742 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-4].u),(yyvsp[-3].f),(yyvsp[-1].u),NULL,0,&((yyvsp[0].xd)));}
#line 3191 "parser.c"
    break;

  case 155: /* image_spec: UNSIGNED image_aspect image_width "height" REFERENCE  */
#line 755 "parser.y"
{
	#line 4744 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-4].u),(yyvsp[-3].f),0,&((yyvsp[-2].xd)),(yyvsp[0].u),NULL);}
#line 3199 "parser.c"
    break;

  case 156: /* image_spec: UNSIGNED image_aspect "width" REFERENCE "height" REFERENCE  */
#line 759 "parser.y"
{
	#line 4746 "format.w"
	(yyval.info)= hput_image_spec((yyvsp[-5].u),(yyvsp[-4].f),(yyvsp[-2].u),NULL,(yyvsp[0].u),NULL);}
#line 3207 "parser.c"
    break;

  case 157: /* image: image_spec list  */
#line 763 "parser.y"
                     {
	#line 4748 "format.w"
	(yyval.info)= (yyvsp[-1].info);}
#line 3215 "parser.c"
    break;

  case 158: /* content_node: start "image" image ">"  */
#line 767 "parser.y"
                                  {
	#line 4750 "format.w"
	hput_tags((yyvsp[-3].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3223 "parser.c"
    break;

  case 159: /* max_value: "outline" UNSIGNED  */
#line 771 "parser.y"
                          {
	#line 5383 "format.w"
	max_outline= (yyvsp[0].u);
	RNG("max outline",max_outline,0,0xFFFF);
	DBG(DBGDEF	|DBGLABEL,"Setting max outline to %d\n",max_outline);
	}
#line 3234 "parser.c"
    break;

  case 160: /* placement: "top"  */
#line 778 "parser.y"
             {
	#line 5476 "format.w"
	(yyval.i)= LABEL_TOP;}
#line 3242 "parser.c"
    break;

  case 161: /* placement: "bot"  */
#line 780 "parser.y"
                            {
	#line 5476 "format.w"
	(yyval.i)= LABEL_BOT;}
#line 3250 "parser.c"
    break;

  case 162: /* placement: "mid"  */
#line 782 "parser.y"
                            {
	#line 5476 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3258 "parser.c"
    break;

  case 163: /* placement: %empty  */
#line 784 "parser.y"
                         {
	#line 5476 "format.w"
	(yyval.i)= LABEL_MID;}
#line 3266 "parser.c"
    break;

  case 164: /* content_node: "<" "label" REFERENCE placement ">"  */
#line 788 "parser.y"
{
	#line 5478 "format.w"
	hset_label((yyvsp[-2].u),(yyvsp[-1].i));}
#line 3274 "parser.c"
    break;

  case 165: /* content_node: start "link" REFERENCE on_off ">"  */
#line 793 "parser.y"
{
	#line 5767 "format.w"
	hput_tags((yyvsp[-4].u),hput_link((yyvsp[-2].u),(yyvsp[-1].i)?1:0xFF,(yyvsp[-1].i)));}
#line 3282 "parser.c"
    break;

  case 166: /* content_node: start "link" REFERENCE on_off REFERENCE ">"  */
#line 797 "parser.y"
{
	#line 5769 "format.w"
	hput_tags((yyvsp[-5].u),hput_link((yyvsp[-3].u),(yyvsp[-1].u),(yyvsp[-2].i)));}
#line 3290 "parser.c"
    break;

  case 167: /* def_node: "<" "outline" REFERENCE integer position list ">"  */
#line 801 "parser.y"
                                                          {
	#line 5901 "format.w"
	
	static int outline_no= -1;
	(yyval.rf).k= outline_kind;(yyval.rf).n= (yyvsp[-4].u);
	if((yyvsp[-1].l).s==0)QUIT("Outline with empty title in line %d",yylineno);
	outline_no++;
	hset_outline(outline_no,(yyvsp[-4].u),(yyvsp[-3].i),(yyvsp[-2].u));
	}
#line 3304 "parser.c"
    break;

  case 168: /* color: "<" UNSIGNED UNSIGNED UNSIGNED UNSIGNED ">"  */
#line 812 "parser.y"
{
	#line 6191 "format.w"
	RNG("red",(yyvsp[-4].u),0,0xFF);RNG("green",(yyvsp[-3].u),0,0xFF);
	RNG("blue",(yyvsp[-2].u),0,0xFF);RNG("alpha",(yyvsp[-1].u),0,0xFF);
	colors_n[colors_i++]= ((yyvsp[-4].u)<<24)	|((yyvsp[-3].u)<<16)	|((yyvsp[-2].u)<<8)	|(yyvsp[-1].u);
	}
#line 3315 "parser.c"
    break;

  case 169: /* color: "<" UNSIGNED UNSIGNED UNSIGNED ">"  */
#line 819 "parser.y"
{
	#line 6196 "format.w"
	RNG("red",(yyvsp[-3].u),0,0xFF);RNG("green",(yyvsp[-2].u),0,0xFF);
	RNG("blue",(yyvsp[-1].u),0,0xFF);
	colors_n[colors_i++]= ((yyvsp[-3].u)<<24)	|((yyvsp[-2].u)<<16)	|((yyvsp[-1].u)<<8)	|0xFF;
	}
#line 3326 "parser.c"
    break;

  case 170: /* color: UNSIGNED  */
#line 825 "parser.y"
              {
	#line 6200 "format.w"
	colors_n[colors_i++]= (yyvsp[0].u);}
#line 3334 "parser.c"
    break;

  case 172: /* color_pair: "<" color ">"  */
#line 830 "parser.y"
                        {
	#line 6210 "format.w"
	colors_n[colors_i++]= 0;}
#line 3342 "parser.c"
    break;

  case 173: /* color_unset: %empty  */
#line 833 "parser.y"
            {
	#line 6211 "format.w"
	colors_i+= 2;}
#line 3350 "parser.c"
    break;

  case 179: /* $@8: %empty  */
#line 845 "parser.y"
                        {
	#line 6241 "format.w"
	HPUT8(6);color_init();}
#line 3358 "parser.c"
    break;

  case 180: /* def_node: start "color" ref $@8 color_set ">"  */
#line 848 "parser.y"
{
	#line 6242 "format.w"
	DEF((yyval.rf),color_kind,(yyvsp[-3].u));hput_color_def((yyvsp[-5].u),(yyvsp[-3].u));}
#line 3366 "parser.c"
    break;

  case 181: /* content_node: start "color" ref ">"  */
#line 853 "parser.y"
{
	#line 6296 "format.w"
	REF_RNG(color_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(color_kind,b000));}
#line 3374 "parser.c"
    break;

  case 182: /* content_node: start "color" "off" ">"  */
#line 857 "parser.y"
{
	#line 6298 "format.w"
	HPUT8(0xFF);hput_tags((yyvsp[-3].u),TAG(color_kind,b000));}
#line 3382 "parser.c"
    break;

  case 183: /* def_node: start "unknown" UNSIGNED UNSIGNED ">"  */
#line 861 "parser.y"
                                            {
	#line 6426 "format.w"
	hput_tags((yyvsp[-4].u),hput_unknown_def((yyvsp[-2].u),(yyvsp[-1].u),0));}
#line 3390 "parser.c"
    break;

  case 184: /* def_node: start "unknown" UNSIGNED UNSIGNED UNSIGNED ">"  */
#line 864 "parser.y"
                                                     {
	#line 6427 "format.w"
	hput_tags((yyvsp[-5].u),hput_unknown_def((yyvsp[-3].u),(yyvsp[-2].u),(yyvsp[-1].u)));}
#line 3398 "parser.c"
    break;

  case 185: /* content_node: start "unknown" UNSIGNED unknown_bytes unknown_nodes ">"  */
#line 868 "parser.y"
                                                                   {
	#line 6439 "format.w"
	hput_tags((yyvsp[-5].u),hput_unknown((yyvsp[-5].u),(yyvsp[-3].u),(yyvsp[-2].u),(yyvsp[-1].u)));}
#line 3406 "parser.c"
    break;

  case 186: /* unknown_bytes: %empty  */
#line 871 "parser.y"
              {
	#line 6440 "format.w"
	(yyval.u)= 0;}
#line 3414 "parser.c"
    break;

  case 187: /* unknown_bytes: unknown_bytes UNSIGNED  */
#line 873 "parser.y"
                                       {
	#line 6440 "format.w"
	RNG("byte",(yyvsp[0].u),0,0xFF);HPUT8((yyvsp[0].u));(yyval.u)= (yyvsp[-1].u)+1;}
#line 3422 "parser.c"
    break;

  case 192: /* unknown_nodes: %empty  */
#line 877 "parser.y"
              {
	#line 6442 "format.w"
	(yyval.u)= 0;}
#line 3430 "parser.c"
    break;

  case 193: /* unknown_nodes: unknown_nodes unknown_node  */
#line 879 "parser.y"
                                           {
	#line 6442 "format.w"
	RNG("unknown subnodes",(yyvsp[-1].u),0,3);(yyval.u)= (yyvsp[-1].u)+1;}
#line 3438 "parser.c"
    break;

  case 194: /* stream_link: ref  */
#line 883 "parser.y"
               {
	#line 6877 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 3446 "parser.c"
    break;

  case 195: /* stream_link: "*"  */
#line 885 "parser.y"
                                                    {
	#line 6877 "format.w"
	HPUT8(255);}
#line 3454 "parser.c"
    break;

  case 196: /* stream_split: stream_link stream_link UNSIGNED  */
#line 888 "parser.y"
                                             {
	#line 6878 "format.w"
	RNG("split ratio",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3462 "parser.c"
    break;

  case 197: /* $@9: %empty  */
#line 891 "parser.y"
                                {
	#line 6879 "format.w"
	RNG("magnification factor",(yyvsp[0].u),0,1000);HPUT16((yyvsp[0].u));}
#line 3470 "parser.c"
    break;

  case 199: /* stream_type: stream_info  */
#line 895 "parser.y"
                       {
	#line 6881 "format.w"
	(yyval.info)= 0;}
#line 3478 "parser.c"
    break;

  case 200: /* stream_type: "first"  */
#line 897 "parser.y"
                      {
	#line 6881 "format.w"
	(yyval.info)= 1;}
#line 3486 "parser.c"
    break;

  case 201: /* stream_type: "last"  */
#line 899 "parser.y"
                     {
	#line 6881 "format.w"
	(yyval.info)= 2;}
#line 3494 "parser.c"
    break;

  case 202: /* stream_type: "top"  */
#line 901 "parser.y"
                    {
	#line 6881 "format.w"
	(yyval.info)= 3;}
#line 3502 "parser.c"
    break;

  case 203: /* stream_def_node: start "stream (definition)" ref stream_type list xdimen_node glue_node list glue_node ">"  */
#line 907 "parser.y"
{
	#line 6885 "format.w"
	DEF((yyval.rf),stream_kind,(yyvsp[-7].u));hput_tags((yyvsp[-9].u),TAG(stream_kind,(yyvsp[-6].info)	|b100));}
#line 3510 "parser.c"
    break;

  case 204: /* stream_ins_node: start "stream (definition)" ref ">"  */
#line 912 "parser.y"
{
	#line 6888 "format.w"
	RNG("Stream insertion",(yyvsp[-1].u),0,max_ref[stream_kind]);hput_tags((yyvsp[-3].u),TAG(stream_kind,b100));}
#line 3518 "parser.c"
    break;

  case 207: /* stream: param_list list  */
#line 918 "parser.y"
                      {
	#line 6983 "format.w"
	(yyval.info)= b010;}
#line 3526 "parser.c"
    break;

  case 208: /* stream: param_ref list  */
#line 921 "parser.y"
                       {
	#line 6984 "format.w"
	(yyval.info)= b000;}
#line 3534 "parser.c"
    break;

  case 209: /* content_node: start "stream" stream_ref stream ">"  */
#line 925 "parser.y"
{
	#line 6986 "format.w"
	hput_tags((yyvsp[-4].u),TAG(stream_kind,(yyvsp[-1].info)));}
#line 3542 "parser.c"
    break;

  case 210: /* page_priority: %empty  */
#line 929 "parser.y"
              {
	#line 7089 "format.w"
	HPUT8(1);}
#line 3550 "parser.c"
    break;

  case 211: /* page_priority: UNSIGNED  */
#line 932 "parser.y"
                 {
	#line 7090 "format.w"
	RNG("page priority",(yyvsp[0].u),0,255);HPUT8((yyvsp[0].u));}
#line 3558 "parser.c"
    break;

  case 214: /* $@10: %empty  */
#line 938 "parser.y"
           {
	#line 7094 "format.w"
	hput_string((yyvsp[0].s));}
#line 3566 "parser.c"
    break;

  case 215: /* $@11: %empty  */
#line 940 "parser.y"
                                                          {
	#line 7094 "format.w"
	HPUT32((yyvsp[0].d));}
#line 3574 "parser.c"
    break;

  case 217: /* content_node: "<" "range" REFERENCE "on" ">"  */
#line 947 "parser.y"
                                         {
	#line 7207 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),true);}
#line 3582 "parser.c"
    break;

  case 218: /* content_node: "<" "range" REFERENCE "off" ">"  */
#line 950 "parser.y"
                                      {
	#line 7208 "format.w"
	REF(page_kind,(yyvsp[-2].u));hput_range((yyvsp[-2].u),false);}
#line 3590 "parser.c"
    break;

  case 220: /* $@12: %empty  */
#line 956 "parser.y"
                                          {
	#line 7906 "format.w"
	new_directory((yyvsp[0].u)+1);new_output_buffers();}
#line 3598 "parser.c"
    break;

  case 224: /* entry: "<" "entry" UNSIGNED string ">"  */
#line 961 "parser.y"
{
	#line 7909 "format.w"
	RNG("Section number",(yyvsp[-2].u),3,max_section_no);hset_entry(&(dir[(yyvsp[-2].u)]),(yyvsp[-2].u),0,0,(yyvsp[-1].s));}
#line 3606 "parser.c"
    break;

  case 225: /* $@13: %empty  */
#line 965 "parser.y"
                                    {
	#line 8453 "format.w"
	hput_definitions_start();}
#line 3614 "parser.c"
    break;

  case 226: /* definition_section: "<" "definitions" $@13 max_definitions definition_list ">"  */
#line 969 "parser.y"
   {
	#line 8455 "format.w"
	hput_definitions_end();}
#line 3622 "parser.c"
    break;

  case 229: /* max_definitions: "<" "max" max_list ">"  */
#line 975 "parser.y"
{
	#line 8571 "format.w"
		/*253:*/
	if(max_ref[label_kind]>=0)
	ALLOCATE(labels,max_ref[label_kind]+1,Label);
		/*:253*/	/*274:*/
	if(max_outline>=0)
	ALLOCATE(outlines,max_outline+1,Outline);
		/*:274*/	/*290:*/
	{int i;
	for(i= 0;i<sizeof(ColorSet)/4;i++)
	colors_0[i]= color_defaults[0][i];
	}
		/*:290*/	/*325:*/
	ALLOCATE(page_on,max_ref[page_kind]+1,int);
	ALLOCATE(range_pos,2*(max_ref[range_kind]+1),RangePos);
		/*:325*/	/*391:*/
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
	definition_bits[0][color_kind]= (1<<(MAX_COLOR_DEFAULT+1))-1;
		/*:391*/	/*406:*/
	ALLOCATE(hfont_name,max_ref[font_kind]+1,char*);
		/*:406*/hput_max_definitions();}
#line 3658 "parser.c"
    break;

  case 232: /* max_value: "font" UNSIGNED  */
#line 1009 "parser.y"
                       {
	#line 8575 "format.w"
	hset_max(font_kind,(yyvsp[0].u));}
#line 3666 "parser.c"
    break;

  case 233: /* max_value: "int" UNSIGNED  */
#line 1012 "parser.y"
                         {
	#line 8576 "format.w"
	hset_max(int_kind,(yyvsp[0].u));}
#line 3674 "parser.c"
    break;

  case 234: /* max_value: "dimen" UNSIGNED  */
#line 1015 "parser.y"
                       {
	#line 8577 "format.w"
	hset_max(dimen_kind,(yyvsp[0].u));}
#line 3682 "parser.c"
    break;

  case 235: /* max_value: "ligature" UNSIGNED  */
#line 1018 "parser.y"
                          {
	#line 8578 "format.w"
	hset_max(ligature_kind,(yyvsp[0].u));}
#line 3690 "parser.c"
    break;

  case 236: /* max_value: "disc" UNSIGNED  */
#line 1021 "parser.y"
                      {
	#line 8579 "format.w"
	hset_max(disc_kind,(yyvsp[0].u));}
#line 3698 "parser.c"
    break;

  case 237: /* max_value: "glue" UNSIGNED  */
#line 1024 "parser.y"
                      {
	#line 8580 "format.w"
	hset_max(glue_kind,(yyvsp[0].u));}
#line 3706 "parser.c"
    break;

  case 238: /* max_value: "language" UNSIGNED  */
#line 1027 "parser.y"
                          {
	#line 8581 "format.w"
	hset_max(language_kind,(yyvsp[0].u));}
#line 3714 "parser.c"
    break;

  case 239: /* max_value: "rule" UNSIGNED  */
#line 1030 "parser.y"
                      {
	#line 8582 "format.w"
	hset_max(rule_kind,(yyvsp[0].u));}
#line 3722 "parser.c"
    break;

  case 240: /* max_value: "image" UNSIGNED  */
#line 1033 "parser.y"
                       {
	#line 8583 "format.w"
	hset_max(image_kind,(yyvsp[0].u));}
#line 3730 "parser.c"
    break;

  case 241: /* max_value: "leaders" UNSIGNED  */
#line 1036 "parser.y"
                         {
	#line 8584 "format.w"
	hset_max(leaders_kind,(yyvsp[0].u));}
#line 3738 "parser.c"
    break;

  case 242: /* max_value: "baseline" UNSIGNED  */
#line 1039 "parser.y"
                          {
	#line 8585 "format.w"
	hset_max(baseline_kind,(yyvsp[0].u));}
#line 3746 "parser.c"
    break;

  case 243: /* max_value: "xdimen" UNSIGNED  */
#line 1042 "parser.y"
                        {
	#line 8586 "format.w"
	hset_max(xdimen_kind,(yyvsp[0].u));}
#line 3754 "parser.c"
    break;

  case 244: /* max_value: "param" UNSIGNED  */
#line 1045 "parser.y"
                       {
	#line 8587 "format.w"
	hset_max(param_kind,(yyvsp[0].u));}
#line 3762 "parser.c"
    break;

  case 245: /* max_value: "stream (definition)" UNSIGNED  */
#line 1048 "parser.y"
                           {
	#line 8588 "format.w"
	hset_max(stream_kind,(yyvsp[0].u));}
#line 3770 "parser.c"
    break;

  case 246: /* max_value: "page" UNSIGNED  */
#line 1051 "parser.y"
                      {
	#line 8589 "format.w"
	hset_max(page_kind,(yyvsp[0].u));}
#line 3778 "parser.c"
    break;

  case 247: /* max_value: "range" UNSIGNED  */
#line 1054 "parser.y"
                       {
	#line 8590 "format.w"
	hset_max(range_kind,(yyvsp[0].u));}
#line 3786 "parser.c"
    break;

  case 248: /* max_value: "label" UNSIGNED  */
#line 1057 "parser.y"
                       {
	#line 8591 "format.w"
	hset_max(label_kind,(yyvsp[0].u));}
#line 3794 "parser.c"
    break;

  case 249: /* max_value: "color" UNSIGNED  */
#line 1060 "parser.y"
                       {
	#line 8592 "format.w"
	hset_max(color_kind,(yyvsp[0].u));}
#line 3802 "parser.c"
    break;

  case 250: /* def_node: start "font" ref font ">"  */
#line 1066 "parser.y"
                       {
	#line 8794 "format.w"
	DEF((yyval.rf),font_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),(yyvsp[-1].info));}
#line 3810 "parser.c"
    break;

  case 251: /* def_node: start "int" ref integer ">"  */
#line 1069 "parser.y"
                                      {
	#line 8795 "format.w"
	DEF((yyval.rf),int_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3818 "parser.c"
    break;

  case 252: /* def_node: start "dimen" ref dimension ">"  */
#line 1072 "parser.y"
                                      {
	#line 8796 "format.w"
	DEF((yyval.rf),dimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_dimen((yyvsp[-1].d)));}
#line 3826 "parser.c"
    break;

  case 253: /* def_node: start "language" ref string ">"  */
#line 1075 "parser.y"
                                      {
	#line 8797 "format.w"
	DEF((yyval.rf),language_kind,(yyvsp[-2].u));hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 3834 "parser.c"
    break;

  case 254: /* def_node: start "glue" ref glue ">"  */
#line 1078 "parser.y"
                                {
	#line 8798 "format.w"
	DEF((yyval.rf),glue_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 3842 "parser.c"
    break;

  case 255: /* def_node: start "xdimen" ref xdimen ">"  */
#line 1081 "parser.y"
                                    {
	#line 8799 "format.w"
	DEF((yyval.rf),xdimen_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_xdimen(&((yyvsp[-1].xd))));}
#line 3850 "parser.c"
    break;

  case 256: /* def_node: start "rule" ref rule ">"  */
#line 1084 "parser.y"
                                {
	#line 8800 "format.w"
	DEF((yyval.rf),rule_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 3858 "parser.c"
    break;

  case 257: /* def_node: start "leaders" ref leaders ">"  */
#line 1087 "parser.y"
                                      {
	#line 8801 "format.w"
	DEF((yyval.rf),leaders_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(leaders_kind,(yyvsp[-1].info)));}
#line 3866 "parser.c"
    break;

  case 258: /* def_node: start "baseline" ref baseline ">"  */
#line 1090 "parser.y"
                                        {
	#line 8802 "format.w"
	DEF((yyval.rf),baseline_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(baseline_kind,(yyvsp[-1].info)));}
#line 3874 "parser.c"
    break;

  case 259: /* def_node: start "ligature" ref ligature ">"  */
#line 1093 "parser.y"
                                        {
	#line 8803 "format.w"
	DEF((yyval.rf),ligature_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 3882 "parser.c"
    break;

  case 260: /* def_node: start "disc" ref disc ">"  */
#line 1096 "parser.y"
                                {
	#line 8804 "format.w"
	DEF((yyval.rf),disc_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 3890 "parser.c"
    break;

  case 261: /* def_node: start "image" ref image ">"  */
#line 1099 "parser.y"
                                  {
	#line 8805 "format.w"
	DEF((yyval.rf),image_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 3898 "parser.c"
    break;

  case 262: /* def_node: start "param" ref parameters ">"  */
#line 1102 "parser.y"
                                       {
	#line 8806 "format.w"
	DEF((yyval.rf),param_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),hput_list((yyvsp[-4].u)+2,&((yyvsp[-1].l))));}
#line 3906 "parser.c"
    break;

  case 263: /* def_node: start "page" ref page ">"  */
#line 1105 "parser.y"
                                {
	#line 8807 "format.w"
	DEF((yyval.rf),page_kind,(yyvsp[-2].u));hput_tags((yyvsp[-4].u),TAG(page_kind,0));}
#line 3914 "parser.c"
    break;

  case 264: /* def_node: start "int" ref ref ">"  */
#line 1110 "parser.y"
                         {
	#line 8826 "format.w"
	DEF_REF((yyval.rf),int_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(int_kind,0));}
#line 3922 "parser.c"
    break;

  case 265: /* def_node: start "dimen" ref ref ">"  */
#line 1113 "parser.y"
                                {
	#line 8827 "format.w"
	DEF_REF((yyval.rf),dimen_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(dimen_kind,0));}
#line 3930 "parser.c"
    break;

  case 266: /* def_node: start "glue" ref ref ">"  */
#line 1116 "parser.y"
                               {
	#line 8828 "format.w"
	DEF_REF((yyval.rf),glue_kind,(yyvsp[-2].u),(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(glue_kind,0));}
#line 3938 "parser.c"
    break;

  case 268: /* def_list: def_list def_node  */
#line 1121 "parser.y"
                          {
	#line 8951 "format.w"
	check_param_def(&((yyvsp[0].rf)));}
#line 3946 "parser.c"
    break;

  case 269: /* parameters: estimate def_list  */
#line 1124 "parser.y"
                            {
	#line 8952 "format.w"
	(yyval.l).p= (yyvsp[0].u);(yyval.l).t= TAG(param_kind,b001);(yyval.l).s= (hpos-hstart)-(yyvsp[0].u);}
#line 3954 "parser.c"
    break;

  case 270: /* named_param_list: start "param" parameters ">"  */
#line 1129 "parser.y"
{
	#line 8965 "format.w"
	hput_tags((yyvsp[-3].u),hput_list((yyvsp[-3].u)+1,&((yyvsp[-1].l))));}
#line 3962 "parser.c"
    break;

  case 272: /* param_list: start parameters ">"  */
#line 1133 "parser.y"
{
	#line 8967 "format.w"
	hput_tags((yyvsp[-2].u),hput_list((yyvsp[-2].u)+1,&((yyvsp[-1].l))));}
#line 3970 "parser.c"
    break;

  case 274: /* font_head: string dimension UNSIGNED UNSIGNED  */
#line 1141 "parser.y"
{
	#line 9113 "format.w"
	uint8_t f= (yyvsp[-4].u);SET_DBIT(f,font_kind);hfont_name[f]= strdup((yyvsp[-3].s));(yyval.info)= hput_font_head(f,hfont_name[f],(yyvsp[-2].d),(yyvsp[-1].u),(yyvsp[0].u));}
#line 3978 "parser.c"
    break;

  case 277: /* font_param: start "penalty" fref penalty ">"  */
#line 1148 "parser.y"
                              {
	#line 9118 "format.w"
	hput_tags((yyvsp[-4].u),hput_int((yyvsp[-1].i)));}
#line 3986 "parser.c"
    break;

  case 278: /* font_param: start "kern" fref kern ">"  */
#line 1151 "parser.y"
                                 {
	#line 9119 "format.w"
	hput_tags((yyvsp[-4].u),hput_kern(&((yyvsp[-1].kt))));}
#line 3994 "parser.c"
    break;

  case 279: /* font_param: start "ligature" fref ligature ">"  */
#line 1154 "parser.y"
                                         {
	#line 9120 "format.w"
	hput_tags((yyvsp[-4].u),hput_ligature(&((yyvsp[-1].lg))));}
#line 4002 "parser.c"
    break;

  case 280: /* font_param: start "disc" fref disc ">"  */
#line 1157 "parser.y"
                                 {
	#line 9121 "format.w"
	hput_tags((yyvsp[-4].u),hput_disc(&((yyvsp[-1].dc))));}
#line 4010 "parser.c"
    break;

  case 281: /* font_param: start "glue" fref glue ">"  */
#line 1160 "parser.y"
                                 {
	#line 9122 "format.w"
	hput_tags((yyvsp[-4].u),hput_glue(&((yyvsp[-1].g))));}
#line 4018 "parser.c"
    break;

  case 282: /* font_param: start "language" fref string ">"  */
#line 1163 "parser.y"
                                       {
	#line 9123 "format.w"
	hput_string((yyvsp[-1].s));hput_tags((yyvsp[-4].u),TAG(language_kind,0));}
#line 4026 "parser.c"
    break;

  case 283: /* font_param: start "rule" fref rule ">"  */
#line 1166 "parser.y"
                                 {
	#line 9124 "format.w"
	hput_tags((yyvsp[-4].u),hput_rule(&((yyvsp[-1].r))));}
#line 4034 "parser.c"
    break;

  case 284: /* font_param: start "image" fref image ">"  */
#line 1169 "parser.y"
                                   {
	#line 9125 "format.w"
	hput_tags((yyvsp[-4].u),TAG(image_kind,(yyvsp[-1].info)));}
#line 4042 "parser.c"
    break;

  case 285: /* fref: ref  */
#line 1173 "parser.y"
        {
	#line 9127 "format.w"
	RNG("Font parameter",(yyvsp[0].u),0,MAX_FONT_PARAMS);}
#line 4050 "parser.c"
    break;

  case 286: /* xdimen_ref: ref  */
#line 1177 "parser.y"
              {
	#line 9204 "format.w"
	REF(xdimen_kind,(yyvsp[0].u));}
#line 4058 "parser.c"
    break;

  case 287: /* param_ref: ref  */
#line 1180 "parser.y"
             {
	#line 9205 "format.w"
	REF(param_kind,(yyvsp[0].u));}
#line 4066 "parser.c"
    break;

  case 288: /* stream_ref: ref  */
#line 1183 "parser.y"
              {
	#line 9206 "format.w"
	REF_RNG(stream_kind,(yyvsp[0].u));}
#line 4074 "parser.c"
    break;

  case 289: /* content_node: start "penalty" ref ">"  */
#line 1189 "parser.y"
                     {
	#line 9210 "format.w"
	REF(penalty_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(penalty_kind,0));}
#line 4082 "parser.c"
    break;

  case 290: /* content_node: start "kern" explicit ref ">"  */
#line 1193 "parser.y"
{
	#line 9212 "format.w"
	REF(dimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-4].u),TAG(kern_kind,((yyvsp[-2].b))?b100:b000));}
#line 4090 "parser.c"
    break;

  case 291: /* content_node: start "kern" explicit "xdimen" ref ">"  */
#line 1197 "parser.y"
{
	#line 9214 "format.w"
	REF(xdimen_kind,(yyvsp[-1].u));hput_tags((yyvsp[-5].u),TAG(kern_kind,((yyvsp[-3].b))?b101:b001));}
#line 4098 "parser.c"
    break;

  case 292: /* content_node: start "glue" ref ">"  */
#line 1200 "parser.y"
                           {
	#line 9215 "format.w"
	REF(glue_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(glue_kind,0));}
#line 4106 "parser.c"
    break;

  case 293: /* content_node: start "ligature" ref ">"  */
#line 1203 "parser.y"
                               {
	#line 9216 "format.w"
	REF(ligature_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(ligature_kind,0));}
#line 4114 "parser.c"
    break;

  case 294: /* content_node: start "disc" ref ">"  */
#line 1206 "parser.y"
                           {
	#line 9217 "format.w"
	REF(disc_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(disc_kind,0));}
#line 4122 "parser.c"
    break;

  case 295: /* content_node: start "rule" ref ">"  */
#line 1209 "parser.y"
                           {
	#line 9218 "format.w"
	REF(rule_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(rule_kind,0));}
#line 4130 "parser.c"
    break;

  case 296: /* content_node: start "image" ref ">"  */
#line 1212 "parser.y"
                            {
	#line 9219 "format.w"
	REF(image_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(image_kind,0));}
#line 4138 "parser.c"
    break;

  case 297: /* content_node: start "leaders" ref ">"  */
#line 1215 "parser.y"
                              {
	#line 9220 "format.w"
	REF(leaders_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(leaders_kind,0));}
#line 4146 "parser.c"
    break;

  case 298: /* content_node: start "baseline" ref ">"  */
#line 1218 "parser.y"
                               {
	#line 9221 "format.w"
	REF(baseline_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),TAG(baseline_kind,0));}
#line 4154 "parser.c"
    break;

  case 299: /* content_node: start "language" REFERENCE ">"  */
#line 1221 "parser.y"
                                     {
	#line 9222 "format.w"
	REF(language_kind,(yyvsp[-1].u));hput_tags((yyvsp[-3].u),hput_language((yyvsp[-1].u)));}
#line 4162 "parser.c"
    break;

  case 300: /* glue_node: start "glue" ref ">"  */
#line 1225 "parser.y"
                            {
	#line 9224 "format.w"
	REF(glue_kind,(yyvsp[-1].u));
	if((yyvsp[-1].u)==zero_skip_no){hpos= hpos-2;(yyval.b)= false;}
	else{hput_tags((yyvsp[-3].u),TAG(glue_kind,0));(yyval.b)= true;}}
#line 4172 "parser.c"
    break;

  case 301: /* $@14: %empty  */
#line 1232 "parser.y"
                             {
	#line 9710 "format.w"
	hput_content_start();}
#line 4180 "parser.c"
    break;

  case 302: /* content_section: "<" "content" $@14 content_list ">"  */
#line 1235 "parser.y"
{
	#line 9711 "format.w"
	hput_content_end();hput_range_defs();hput_label_defs();}
#line 4188 "parser.c"
    break;


#line 4192 "parser.c"

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

#line 1239 "parser.y"

	/*:553*/
