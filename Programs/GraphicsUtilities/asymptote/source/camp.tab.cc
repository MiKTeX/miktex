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
#line 1 "camp.y"

/*****
 * camp.y
 * Andy Hammerlindl 08/12/2002
 *
 * The grammar of the camp language.
 *****/

#include "errormsg.h"
#include "exp.h"
#include "newexp.h"
#include "dec.h"
#include "fundec.h"
#include "stm.h"
#include "modifier.h"
#include "opsymbols.h"

// Avoid error messages with unpatched bison-1.875:
#ifndef __attribute__
#define __attribute__(x)
#endif

// Used when a position needs to be determined and no token is
// available.  Defined in camp.l.
position lexerPos();

bool lexerEOF();

int yylex(void); /* function prototype */

void yyerror(const char *s)
{
  if (!lexerEOF()) {
    em.error(lexerPos());
    em << s;
    em.cont();
  }
}

// Check if the symbol given is "keyword".  Returns true in this case and
// returns false and reports an error otherwise.
bool checkKeyword(position pos, symbol sym)
{
  if (sym != symbol::trans("keyword")) {
    em.error(pos);
    em << "expected 'keyword' here";

    return false;
  }
  return true;
}

namespace absyntax { file *root; }

using namespace absyntax;
using sym::symbol;
using mem::string;

#line 130 "camp.tab.cc"

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

#include "camp.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ID = 3,                         /* ID  */
  YYSYMBOL_SELFOP = 4,                     /* SELFOP  */
  YYSYMBOL_DOTS = 5,                       /* DOTS  */
  YYSYMBOL_COLONS = 6,                     /* COLONS  */
  YYSYMBOL_DASHES = 7,                     /* DASHES  */
  YYSYMBOL_INCR = 8,                       /* INCR  */
  YYSYMBOL_LONGDASH = 9,                   /* LONGDASH  */
  YYSYMBOL_CONTROLS = 10,                  /* CONTROLS  */
  YYSYMBOL_TENSION = 11,                   /* TENSION  */
  YYSYMBOL_ATLEAST = 12,                   /* ATLEAST  */
  YYSYMBOL_CURL = 13,                      /* CURL  */
  YYSYMBOL_COR = 14,                       /* COR  */
  YYSYMBOL_CAND = 15,                      /* CAND  */
  YYSYMBOL_BAR = 16,                       /* BAR  */
  YYSYMBOL_AMPERSAND = 17,                 /* AMPERSAND  */
  YYSYMBOL_EQ = 18,                        /* EQ  */
  YYSYMBOL_NEQ = 19,                       /* NEQ  */
  YYSYMBOL_LT = 20,                        /* LT  */
  YYSYMBOL_LE = 21,                        /* LE  */
  YYSYMBOL_GT = 22,                        /* GT  */
  YYSYMBOL_GE = 23,                        /* GE  */
  YYSYMBOL_CARETS = 24,                    /* CARETS  */
  YYSYMBOL_25_ = 25,                       /* '+'  */
  YYSYMBOL_26_ = 26,                       /* '-'  */
  YYSYMBOL_27_ = 27,                       /* '*'  */
  YYSYMBOL_28_ = 28,                       /* '/'  */
  YYSYMBOL_29_ = 29,                       /* '%'  */
  YYSYMBOL_30_ = 30,                       /* '#'  */
  YYSYMBOL_31_ = 31,                       /* '^'  */
  YYSYMBOL_OPERATOR = 32,                  /* OPERATOR  */
  YYSYMBOL_LOOSE = 33,                     /* LOOSE  */
  YYSYMBOL_ASSIGN = 34,                    /* ASSIGN  */
  YYSYMBOL_35_ = 35,                       /* '?'  */
  YYSYMBOL_36_ = 36,                       /* ':'  */
  YYSYMBOL_DIRTAG = 37,                    /* DIRTAG  */
  YYSYMBOL_JOIN_PREC = 38,                 /* JOIN_PREC  */
  YYSYMBOL_AND = 39,                       /* AND  */
  YYSYMBOL_40_ = 40,                       /* '{'  */
  YYSYMBOL_41_ = 41,                       /* '}'  */
  YYSYMBOL_42_ = 42,                       /* '('  */
  YYSYMBOL_43_ = 43,                       /* ')'  */
  YYSYMBOL_44_ = 44,                       /* '.'  */
  YYSYMBOL_45_ = 45,                       /* ','  */
  YYSYMBOL_46_ = 46,                       /* '['  */
  YYSYMBOL_47_ = 47,                       /* ']'  */
  YYSYMBOL_48_ = 48,                       /* ';'  */
  YYSYMBOL_ELLIPSIS = 49,                  /* ELLIPSIS  */
  YYSYMBOL_ACCESS = 50,                    /* ACCESS  */
  YYSYMBOL_UNRAVEL = 51,                   /* UNRAVEL  */
  YYSYMBOL_IMPORT = 52,                    /* IMPORT  */
  YYSYMBOL_INCLUDE = 53,                   /* INCLUDE  */
  YYSYMBOL_FROM = 54,                      /* FROM  */
  YYSYMBOL_QUOTE = 55,                     /* QUOTE  */
  YYSYMBOL_STRUCT = 56,                    /* STRUCT  */
  YYSYMBOL_TYPEDEF = 57,                   /* TYPEDEF  */
  YYSYMBOL_USING = 58,                     /* USING  */
  YYSYMBOL_NEW = 59,                       /* NEW  */
  YYSYMBOL_IF = 60,                        /* IF  */
  YYSYMBOL_ELSE = 61,                      /* ELSE  */
  YYSYMBOL_WHILE = 62,                     /* WHILE  */
  YYSYMBOL_DO = 63,                        /* DO  */
  YYSYMBOL_FOR = 64,                       /* FOR  */
  YYSYMBOL_BREAK = 65,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 66,                  /* CONTINUE  */
  YYSYMBOL_RETURN_ = 67,                   /* RETURN_  */
  YYSYMBOL_THIS_TOK = 68,                  /* THIS_TOK  */
  YYSYMBOL_EXPLICIT = 69,                  /* EXPLICIT  */
  YYSYMBOL_GARBAGE = 70,                   /* GARBAGE  */
  YYSYMBOL_LIT = 71,                       /* LIT  */
  YYSYMBOL_STRING = 72,                    /* STRING  */
  YYSYMBOL_PERM = 73,                      /* PERM  */
  YYSYMBOL_MODIFIER = 74,                  /* MODIFIER  */
  YYSYMBOL_UNARY = 75,                     /* UNARY  */
  YYSYMBOL_EXP_IN_PARENS_RULE = 76,        /* EXP_IN_PARENS_RULE  */
  YYSYMBOL_YYACCEPT = 77,                  /* $accept  */
  YYSYMBOL_file = 78,                      /* file  */
  YYSYMBOL_fileblock = 79,                 /* fileblock  */
  YYSYMBOL_bareblock = 80,                 /* bareblock  */
  YYSYMBOL_name = 81,                      /* name  */
  YYSYMBOL_runnable = 82,                  /* runnable  */
  YYSYMBOL_modifiers = 83,                 /* modifiers  */
  YYSYMBOL_dec = 84,                       /* dec  */
  YYSYMBOL_optionalcomma = 85,             /* optionalcomma  */
  YYSYMBOL_decdec = 86,                    /* decdec  */
  YYSYMBOL_decdeclist = 87,                /* decdeclist  */
  YYSYMBOL_typeparam = 88,                 /* typeparam  */
  YYSYMBOL_typeparamlist = 89,             /* typeparamlist  */
  YYSYMBOL_idpair = 90,                    /* idpair  */
  YYSYMBOL_idpairlist = 91,                /* idpairlist  */
  YYSYMBOL_strid = 92,                     /* strid  */
  YYSYMBOL_stridpair = 93,                 /* stridpair  */
  YYSYMBOL_stridpairlist = 94,             /* stridpairlist  */
  YYSYMBOL_vardec = 95,                    /* vardec  */
  YYSYMBOL_barevardec = 96,                /* barevardec  */
  YYSYMBOL_type = 97,                      /* type  */
  YYSYMBOL_celltype = 98,                  /* celltype  */
  YYSYMBOL_dims = 99,                      /* dims  */
  YYSYMBOL_dimexps = 100,                  /* dimexps  */
  YYSYMBOL_decidlist = 101,                /* decidlist  */
  YYSYMBOL_decid = 102,                    /* decid  */
  YYSYMBOL_decidstart = 103,               /* decidstart  */
  YYSYMBOL_varinit = 104,                  /* varinit  */
  YYSYMBOL_block = 105,                    /* block  */
  YYSYMBOL_arrayinit = 106,                /* arrayinit  */
  YYSYMBOL_basearrayinit = 107,            /* basearrayinit  */
  YYSYMBOL_varinits = 108,                 /* varinits  */
  YYSYMBOL_formals = 109,                  /* formals  */
  YYSYMBOL_explicitornot = 110,            /* explicitornot  */
  YYSYMBOL_formal = 111,                   /* formal  */
  YYSYMBOL_fundec = 112,                   /* fundec  */
  YYSYMBOL_typedec = 113,                  /* typedec  */
  YYSYMBOL_slice = 114,                    /* slice  */
  YYSYMBOL_value = 115,                    /* value  */
  YYSYMBOL_argument = 116,                 /* argument  */
  YYSYMBOL_arglist = 117,                  /* arglist  */
  YYSYMBOL_tuple = 118,                    /* tuple  */
  YYSYMBOL_exp = 119,                      /* exp  */
  YYSYMBOL_join = 120,                     /* join  */
  YYSYMBOL_dir = 121,                      /* dir  */
  YYSYMBOL_basicjoin = 122,                /* basicjoin  */
  YYSYMBOL_tension = 123,                  /* tension  */
  YYSYMBOL_controls = 124,                 /* controls  */
  YYSYMBOL_stm = 125,                      /* stm  */
  YYSYMBOL_stmexp = 126,                   /* stmexp  */
  YYSYMBOL_blockstm = 127,                 /* blockstm  */
  YYSYMBOL_forinit = 128,                  /* forinit  */
  YYSYMBOL_fortest = 129,                  /* fortest  */
  YYSYMBOL_forupdate = 130,                /* forupdate  */
  YYSYMBOL_stmexplist = 131                /* stmexplist  */
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2005

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  218
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  429

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   313


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
       2,     2,     2,     2,     2,    30,     2,    29,     2,     2,
      42,    43,    27,    25,    45,    26,    44,    28,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    36,    48,
       2,     2,     2,    35,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    46,     2,    47,    31,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    40,     2,    41,     2,     2,     2,     2,
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
      32,    33,    34,    37,    38,    39,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   195,   195,   199,   200,   205,   206,   211,   212,   213,
     218,   219,   220,   222,   227,   228,   229,   231,   236,   237,
     238,   239,   241,   243,   245,   246,   248,   250,   252,   253,
     257,   259,   264,   268,   270,   276,   281,   282,   287,   291,
     295,   297,   302,   306,   308,   313,   315,   319,   320,   325,
     326,   331,   333,   337,   338,   343,   347,   351,   352,   356,
     360,   361,   365,   366,   371,   372,   377,   378,   383,   384,
     385,   387,   392,   393,   397,   402,   403,   405,   407,   412,
     413,   414,   418,   420,   425,   426,   427,   429,   434,   435,
     439,   441,   443,   446,   449,   455,   457,   462,   463,   468,
     471,   475,   481,   482,   483,   484,   488,   489,   491,   492,
     494,   495,   498,   502,   503,   505,   507,   509,   513,   514,
     518,   519,   521,   523,   529,   530,   534,   535,   536,   537,
     539,   540,   542,   544,   546,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,   564,   565,   566,   567,   568,   569,   571,   573,   575,
     577,   579,   581,   586,   588,   593,   595,   596,   597,   599,
     605,   607,   610,   612,   613,   620,   621,   623,   626,   629,
     635,   636,   637,   640,   646,   647,   649,   651,   652,   656,
     658,   661,   664,   670,   671,   676,   677,   678,   679,   681,
     683,   685,   687,   689,   691,   692,   693,   694,   698,   702,
     706,   707,   708,   712,   713,   717,   718,   722,   723
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ID", "SELFOP", "DOTS",
  "COLONS", "DASHES", "INCR", "LONGDASH", "CONTROLS", "TENSION", "ATLEAST",
  "CURL", "COR", "CAND", "BAR", "AMPERSAND", "EQ", "NEQ", "LT", "LE", "GT",
  "GE", "CARETS", "'+'", "'-'", "'*'", "'/'", "'%'", "'#'", "'^'",
  "OPERATOR", "LOOSE", "ASSIGN", "'?'", "':'", "DIRTAG", "JOIN_PREC",
  "AND", "'{'", "'}'", "'('", "')'", "'.'", "','", "'['", "']'", "';'",
  "ELLIPSIS", "ACCESS", "UNRAVEL", "IMPORT", "INCLUDE", "FROM", "QUOTE",
  "STRUCT", "TYPEDEF", "USING", "NEW", "IF", "ELSE", "WHILE", "DO", "FOR",
  "BREAK", "CONTINUE", "RETURN_", "THIS_TOK", "EXPLICIT", "GARBAGE", "LIT",
  "STRING", "PERM", "MODIFIER", "UNARY", "EXP_IN_PARENS_RULE", "$accept",
  "file", "fileblock", "bareblock", "name", "runnable", "modifiers", "dec",
  "optionalcomma", "decdec", "decdeclist", "typeparam", "typeparamlist",
  "idpair", "idpairlist", "strid", "stridpair", "stridpairlist", "vardec",
  "barevardec", "type", "celltype", "dims", "dimexps", "decidlist",
  "decid", "decidstart", "varinit", "block", "arrayinit", "basearrayinit",
  "varinits", "formals", "explicitornot", "formal", "fundec", "typedec",
  "slice", "value", "argument", "arglist", "tuple", "exp", "join", "dir",
  "basicjoin", "tension", "controls", "stm", "stmexp", "blockstm",
  "forinit", "fortest", "forupdate", "stmexplist", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-296)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-60)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -296,    53,   514,  -296,  -296,   916,   916,   916,   916,  -296,
     916,  -296,   916,  -296,    25,    34,    11,    27,    21,    19,
     104,    18,   161,    34,   115,   148,   180,   166,   176,   185,
     739,  -296,   194,  -296,  -296,  -296,    31,  -296,   586,  -296,
    -296,   189,   228,  -296,  -296,  -296,  -296,   177,  1607,  -296,
     202,  -296,   183,   226,   226,   226,   226,  1965,   370,   172,
      26,  1208,   154,  -296,    37,  -296,   193,    91,   221,    45,
     232,   233,   242,    22,    50,    94,  -296,   236,   267,    62,
    -296,   294,   278,   272,   150,   916,   916,   261,   916,  -296,
    -296,  -296,  1023,   226,   658,   328,   673,   287,  -296,  -296,
    -296,  -296,  -296,   165,   290,  -296,   302,   711,   334,   770,
     916,   318,  -296,  -296,   194,  -296,   916,   916,   916,   916,
     916,   916,   916,   916,   916,   916,   916,   916,   916,   916,
     916,   916,   916,   916,   916,   916,   916,   801,   916,  -296,
     298,  -296,   770,  -296,  -296,   194,   167,  -296,   916,  -296,
     916,   336,    38,    25,  -296,  -296,   338,    38,  -296,  -296,
    -296,    71,    38,   159,   442,  -296,   338,   296,   188,    34,
      -4,   829,    12,   299,  1334,  1374,   304,  -296,   344,  -296,
     301,   305,  -296,   317,  -296,   954,  -296,   128,  1607,  -296,
     916,  -296,   308,  1060,   311,   109,   287,   294,   867,  -296,
     146,  -296,   312,  1097,  1607,   916,   905,   355,   356,   426,
    1825,  1853,  1881,  1909,  1937,  1937,   357,   357,   357,   357,
     666,   256,   256,   226,   226,   226,   226,   226,  1965,  1607,
    1570,   916,  1250,   426,   298,  -296,  -296,   916,  1607,  1607,
    -296,   333,  -296,   225,  -296,   365,  -296,  -296,  -296,   234,
     246,   367,   323,  -296,   211,   251,   324,   213,  -296,   255,
     110,   134,   236,   306,  -296,   258,    34,  -296,  1134,   285,
     112,  -296,   829,   287,   180,   180,   916,    15,   916,   916,
     916,  -296,  -296,   954,   325,  1607,  -296,   916,  -296,  -296,
     236,   262,  -296,  -296,  -296,  1607,  -296,  -296,  -296,  1642,
     916,  1679,  -296,  -296,   916,  1494,  -296,   916,  -296,  -296,
      34,    30,    38,   342,   338,   350,   373,  -296,   389,  -296,
     353,  -296,  -296,   352,  -296,   270,   125,  -296,  -296,  -296,
     236,    -7,   358,   401,  -296,  -296,  -296,   867,  -296,   126,
     361,   236,   276,  1171,   347,  -296,  1414,   916,  1607,   366,
    -296,  1607,  -296,   954,  1607,  -296,   236,   916,  1716,   916,
    1789,  -296,  1292,  -296,   406,  -296,  -296,  -296,  -296,  -296,
    -296,  -296,   389,  -296,  -296,   369,   279,  -296,  -296,   306,
      46,   379,   374,  -296,   867,   867,  -296,   236,  -296,   180,
     371,  1454,   916,  -296,  -296,  1753,   916,  1753,  -296,   916,
     383,   224,  -296,   391,  -296,   412,   867,  -296,   399,  -296,
    -296,  -296,  -296,   180,   404,   305,  1753,  1532,  -296,  -296,
    -296,   867,  -296,  -296,  -296,   180,  -296,  -296,  -296
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     7,     0,     0,     0,     0,     9,
       0,     5,     0,   195,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,   128,   129,    15,    14,   126,     4,     0,    10,
      18,     0,     0,    57,   209,    19,    20,   127,   208,    11,
       0,   196,   126,   171,   170,   133,   134,   135,     0,   126,
       0,     0,    49,    50,     0,    53,     0,     0,     0,     0,
       0,     0,     0,     7,     0,     0,     3,     0,     0,    59,
      98,     0,     0,    59,   156,     0,     0,     0,   210,   204,
     205,   206,     0,   130,     0,     0,     0,    58,    17,    16,
      12,    13,    55,    68,    56,    64,    66,     0,     0,     0,
       0,   184,   187,   175,   172,   188,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   169,
     176,   197,     0,    74,     6,   116,     0,   167,     0,   115,
       0,     0,     0,     0,    21,    24,     0,     0,    27,    28,
      29,     0,     0,     0,     0,    97,     0,     0,    68,     0,
      89,     0,   159,   157,     0,     0,     0,   212,     0,   217,
       0,   211,   207,     7,   111,     0,   120,    37,   118,     8,
     102,    60,     0,     0,     0,    89,    69,     0,     0,   113,
      37,   106,     0,     0,   173,     0,     0,     0,     0,   155,
     150,   149,   153,   152,   147,   148,   143,   144,   145,   146,
     151,   136,   137,   138,   139,   140,   141,   142,   154,   166,
       0,     0,     0,   168,   177,   178,   131,     0,   125,   124,
      52,     7,    40,     0,    39,     0,    54,    42,    43,     0,
       0,    45,     0,    47,     0,     0,     0,     0,   174,     0,
      89,     0,     0,    89,    88,    37,     0,    84,     0,     0,
      89,   160,     0,   158,     0,     0,     0,    68,   213,     0,
       0,   121,   112,    36,     0,   104,   109,   103,   107,    61,
      70,    37,    65,    67,    73,    72,   114,   110,   108,   193,
       0,   189,   185,   186,     0,     0,   181,     0,   179,   132,
       0,     0,     0,     0,     0,     0,     0,    23,     0,    22,
       0,    26,    25,     0,    70,    37,    89,    99,   161,    85,
       0,    89,     0,    90,    62,    75,    79,     0,    82,     0,
      80,     0,    37,     0,   198,   200,     0,     0,   214,     0,
     218,   119,   122,     0,   105,    95,    71,     0,   191,     0,
     165,   180,     0,    38,     0,    33,    41,    31,    44,    34,
      46,    48,     0,    30,    71,     0,    37,   163,    86,    89,
      68,    91,     0,    77,     0,    81,   162,     0,    63,     0,
       0,     0,   215,   123,    96,   194,     0,   190,   182,     0,
       0,     0,   100,     0,    87,    93,     0,    76,     0,    83,
     164,   199,   201,     0,     0,   216,   192,     0,    32,    35,
     101,     0,    92,    78,   203,     0,   183,    94,   202
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -296,  -296,   372,  -296,     8,   400,  -296,   421,  -144,   149,
      41,   151,   297,   142,  -155,     1,    -3,  -296,   441,   376,
      -6,   446,   -24,  -296,  -296,   273,  -295,  -237,   395,   303,
    -296,  -296,  -184,  -296,  -241,  -296,  -296,   364,  -296,  -149,
     378,  -296,    -5,  -296,  -131,   337,  -296,  -296,   -20,   -84,
    -246,  -296,  -296,  -296,    85
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    58,    52,    37,    38,    39,   332,   242,
     243,   248,   249,   253,   254,    64,    65,    66,    40,    41,
      42,    43,   196,   173,   104,   105,   106,   293,    44,   294,
     339,   340,   265,   266,   267,    45,    46,   192,    47,   186,
     187,    60,    48,   138,   139,   140,   207,   208,    49,    50,
      51,   180,   349,   414,   181
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      53,    54,    55,    56,   179,    57,    87,    61,   257,   235,
      36,   291,    97,    70,    62,    81,   328,    69,   101,    75,
      59,     4,   329,    67,    73,    92,    74,    93,    62,    79,
      71,    83,   338,   364,   -59,   146,   281,     4,   381,   262,
     151,   241,   -36,   284,   355,   263,    36,     9,   151,   168,
       9,   347,   269,     3,   270,    97,   284,   260,   194,    76,
     172,   167,   264,     9,   -49,   264,    36,     9,    68,   147,
      78,   148,   -49,    94,   251,    95,   325,    96,   365,   152,
     174,   175,   178,    63,   377,   405,   342,   157,   260,   188,
     378,   193,   167,    63,    95,   386,    36,    63,   252,    72,
     382,   161,   188,   308,   203,   204,    95,    77,   167,   209,
     394,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   232,   233,   352,    95,   162,   193,   404,   155,
     236,   410,   376,   238,   163,   239,   244,   408,   409,   273,
     246,   244,   290,   324,   245,   341,   244,    85,   263,   263,
      79,   263,   251,   261,    82,    79,   268,   383,   375,   422,
      79,   282,    36,   283,   263,   384,   326,    79,   264,   264,
     188,   264,   327,     4,   427,   285,   256,     5,     6,   296,
      86,   283,   170,   295,   264,   350,   171,     4,   250,   -51,
     299,   301,   -51,   255,   393,     7,     8,   195,    88,     9,
     237,   167,    10,   194,    94,   145,    95,   401,    96,   107,
      11,   108,    12,   109,    89,    94,   305,    95,    13,   142,
     260,   103,   309,    90,   167,    19,    12,   102,   153,    23,
      24,   154,    25,    26,    27,    28,    29,    30,    31,    19,
     141,    32,    33,    23,   344,   345,   318,   133,   318,   319,
     333,   322,    31,   156,   295,    32,    33,   343,   311,   318,
     312,   346,   419,   348,    79,   351,    11,   313,   188,   314,
     158,   159,   354,   129,   130,   131,   132,   133,     4,   315,
     160,   312,     5,     6,   320,   358,   312,   168,   323,   360,
     314,   330,   362,   331,   363,   356,   244,   331,   179,   166,
       7,     8,   169,   374,     9,   331,    95,    10,    79,   387,
      79,   331,   403,   176,   331,   269,   335,    12,   205,   206,
     336,   189,   295,   194,   337,   197,   198,   201,   137,   240,
      19,   247,   391,   191,    23,   272,   276,   277,   188,   278,
     279,   280,   395,    31,   397,   286,    32,    33,   289,   297,
     302,   303,   111,   112,   113,   114,   115,   310,   151,   411,
     316,   317,   321,     4,   353,   264,   370,     5,     6,   295,
     295,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     367,   416,   251,   424,   417,     7,     8,   137,   369,     9,
     373,   295,    10,   372,   380,   428,   385,   379,   389,   400,
      11,   143,    12,   406,   392,   407,   295,   402,    13,   412,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,   418,    25,    26,    27,    28,    29,    30,    31,   420,
     423,    32,    33,    34,    35,     4,   421,   425,   164,     5,
       6,   127,   128,   129,   130,   131,   132,   133,   144,   100,
     371,   366,    80,   259,   177,   368,   137,     7,     8,    84,
     292,     9,   165,   202,    10,   271,   234,   415,     0,     0,
       0,     0,    11,   258,    12,   200,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,    29,    30,
      31,     0,     0,    32,    33,    34,    35,     4,     0,     0,
       0,     5,     6,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     0,     0,     9,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,    11,     0,    12,     0,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
      29,    30,    31,     0,     0,    32,    33,    34,    35,     4,
       0,     0,     0,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     7,     8,     0,     0,     9,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,    11,     0,    12,     0,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,    29,    30,    31,     0,     0,    32,    33,    98,
      99,   183,     0,     0,     0,     5,     6,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     4,     0,     0,     0,
       5,     6,     0,     7,     8,     0,     0,     9,     0,     0,
      10,   127,   128,   129,   130,   131,   132,   133,     7,     8,
      12,   184,     9,     0,     0,    10,   137,   185,     0,   190,
       0,     0,     0,    19,   183,    12,     0,    23,     5,     6,
     191,     0,     0,     0,     0,     0,    31,     0,    19,    32,
      33,     0,    23,     0,     0,     0,     7,     8,     0,     0,
       9,    31,     4,    10,    32,    33,     5,     6,     0,     0,
       0,     0,     0,    12,   199,     0,     0,     0,     0,     0,
     185,     0,     0,     0,     7,     8,    19,     0,     9,     0,
      23,    10,     0,     4,     0,     0,     0,     5,     6,    31,
       0,    12,    32,    33,     0,     0,     0,    91,     0,     0,
       0,     0,     0,     0,    19,     7,     8,     0,    23,     9,
       0,     0,    10,     0,     4,     0,   190,    31,     5,     6,
      32,    33,    12,     0,   231,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     7,     8,     0,    23,
       9,     0,     4,    10,     0,     0,     5,     6,    31,     0,
       0,    32,    33,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     8,    19,     0,     9,     0,
      23,    10,     0,     0,     0,     0,     0,     0,     0,    31,
       4,    12,    32,    33,     5,     6,   191,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,    23,     0,
       0,     0,     7,     8,     0,     0,     9,    31,     0,    10,
      32,    33,     0,     0,     0,     0,     0,   269,     4,    12,
       0,     0,     5,     6,     0,     0,     0,   300,     0,     4,
       0,     0,    19,     5,     6,     0,    23,     0,     0,     0,
       7,     8,     0,     0,     9,    31,     0,    10,    32,    33,
       0,     7,     8,     0,     0,     9,     0,    12,    10,     0,
       0,     0,     0,     0,     0,     0,     0,   183,    12,     0,
      19,     5,     6,     0,    23,     0,     0,     0,     0,     0,
       0,    19,     0,    31,     0,    23,    32,    33,     0,     7,
       8,     0,     0,     9,    31,     0,    10,    32,    33,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,     0,     0,    23,     0,     0,     0,     0,     0,     0,
       0,     0,    31,     0,     0,    32,    33,   110,   111,   112,
     113,   114,   115,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,     0,   135,   136,     0,
       0,     0,     0,   137,   110,   111,   112,   113,   114,   115,
       0,   182,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,   135,   136,   287,     0,     0,     0,
     137,   110,   111,   112,   113,   114,   115,   288,     0,     0,
       0,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
       0,   135,   136,   287,     0,     0,     0,   137,   110,   111,
     112,   113,   114,   115,   298,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,   110,   111,   112,   113,   114,
     115,   334,     0,     0,     0,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,     0,   135,   136,     0,     0,     0,
       0,   137,   110,   111,   112,   113,   114,   115,   388,     0,
       0,     0,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,     0,   135,   136,     0,     0,     0,     0,   137,     0,
       0,   149,     0,   150,   110,   111,   112,   113,   114,   115,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,   135,   136,     0,     0,     0,     0,
     137,   306,     0,     0,     0,   307,   110,   111,   112,   113,
     114,   115,     0,     0,     0,     0,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,     0,   135,   136,     0,     0,
       0,     0,   137,   398,     0,     0,     0,   399,   110,   111,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,     0,     0,   274,   110,   111,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,     0,     0,   275,   110,   111,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,     0,     0,   390,   110,   111,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,     0,     0,   413,   110,   111,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,   361,   110,   111,   112,   113,
     114,   115,     0,     0,     0,     0,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,     0,   135,   136,     0,     0,
       0,     0,   137,   426,   110,   111,   112,   113,   114,   115,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,   135,   136,   304,     0,     0,     0,
     137,   110,   111,   112,   113,   114,   115,     0,     0,     0,
       0,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
       0,   135,   136,     0,     0,     0,   110,   137,   112,   113,
     114,   115,     0,     0,     0,     0,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,     0,   135,   136,     0,     0,
       0,   357,   137,   110,     0,   112,   113,   114,   115,     0,
       0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,     0,   135,   136,     0,     0,     0,   359,   137,
     110,     0,   112,   113,   114,   115,     0,     0,     0,     0,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,     0,
     135,   136,     0,     0,     0,   396,   137,   110,     0,   112,
     113,   114,   115,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,     0,   135,   136,     0,
       0,     0,     0,   137,   111,   112,   113,   114,   115,     0,
       0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,     0,     0,   136,     0,     0,     0,     0,   137,
     111,   112,   113,   114,   115,     0,     0,     0,     0,     0,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   111,   112,
     113,   114,   115,     0,     0,   137,     0,     0,     0,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   111,   112,   113,   114,
     115,     0,     0,   137,     0,     0,     0,     0,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   111,   112,   113,   114,   115,     0,
       0,   137,     0,     0,     0,     0,     0,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   111,   112,   113,   114,   115,     0,     0,   137,
       0,     0,     0,     0,     0,     0,     0,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     111,   112,   113,   114,   115,     0,     0,   137,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   126,
     127,   128,   129,   130,   131,   132,   133,     0,     0,     0,
       0,     0,     0,     0,     0,   137
};

static const yytype_int16 yycheck[] =
{
       5,     6,     7,     8,    88,    10,    26,    12,   163,   140,
       2,   195,    36,    16,     3,    21,   262,    16,    38,    18,
      12,     3,   263,    15,     3,    30,    18,    32,     3,    21,
       3,    23,   269,     3,     3,    59,   185,     3,   333,    43,
       3,     3,    49,   187,   290,    49,    38,    29,     3,     3,
      29,    36,    40,     0,    42,    79,   200,    42,    46,    40,
      84,    46,    69,    29,    42,    69,    58,    29,    57,    43,
      52,    45,    50,    42,     3,    44,   260,    46,    48,    42,
      85,    86,    88,    72,   330,   380,   270,    42,    42,    94,
     331,    96,    46,    72,    44,   341,    88,    72,    27,    72,
     337,    51,   107,   234,   109,   110,    44,     3,    46,   114,
     356,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   283,    44,    42,   142,   379,    48,
     145,   387,   326,   148,    50,   150,   152,   384,   385,   173,
     153,   157,    43,    43,   153,    43,   162,    42,    49,    49,
     152,    49,     3,   169,     3,   157,   171,    41,    43,   406,
     162,    43,   164,    45,    49,    49,    42,   169,    69,    69,
     185,    69,    48,     3,   421,   190,    27,     7,     8,    43,
      42,    45,    42,   198,    69,   279,    46,     3,   157,    45,
     205,   206,    48,   162,   353,    25,    26,    42,    42,    29,
      43,    46,    32,    46,    42,    43,    44,   372,    46,    42,
      40,    44,    42,    46,    48,    42,   231,    44,    48,    46,
      42,     3,   237,    48,    46,    55,    42,    48,    45,    59,
      60,    48,    62,    63,    64,    65,    66,    67,    68,    55,
      48,    71,    72,    59,   274,   275,    45,    31,    45,    48,
     266,    48,    68,    42,   269,    71,    72,   272,    43,    45,
      45,   276,    48,   278,   266,   280,    40,    43,   283,    45,
      48,    48,   287,    27,    28,    29,    30,    31,     3,    43,
      48,    45,     7,     8,    43,   300,    45,     3,    43,   304,
      45,    43,   307,    45,   310,    43,   312,    45,   392,    42,
      25,    26,    34,    43,    29,    45,    44,    32,   310,    43,
     312,    45,    43,    62,    45,    40,    41,    42,    10,    11,
      45,     3,   337,    46,    49,    45,    34,     3,    40,     3,
      55,     3,   347,    47,    59,    46,    42,     3,   353,    48,
      45,    34,   357,    68,   359,    47,    71,    72,    47,    47,
       5,     5,     5,     6,     7,     8,     9,    34,     3,   389,
       3,    48,    48,     3,    49,    69,     3,     7,     8,   384,
     385,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      48,   396,     3,   413,   399,    25,    26,    40,    48,    29,
      48,   406,    32,    50,     3,   425,    45,    49,    61,     3,
      40,    41,    42,    34,    48,    41,   421,    48,    48,    48,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    48,    62,    63,    64,    65,    66,    67,    68,    48,
      41,    71,    72,    73,    74,     3,    34,    43,    76,     7,
       8,    25,    26,    27,    28,    29,    30,    31,    58,    38,
     318,   312,    21,   166,    88,   314,    40,    25,    26,    23,
     197,    29,    77,   109,    32,   172,   139,   392,    -1,    -1,
      -1,    -1,    40,    41,    42,   107,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
      68,    -1,    -1,    71,    72,    73,    74,     3,    -1,    -1,
      -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    68,    -1,    -1,    71,    72,    73,    74,     3,
      -1,    -1,    -1,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    63,
      64,    65,    66,    67,    68,    -1,    -1,    71,    72,    73,
      74,     3,    -1,    -1,    -1,     7,     8,    -1,    -1,    -1,
      -1,     5,     6,     7,     8,     9,     3,    -1,    -1,    -1,
       7,     8,    -1,    25,    26,    -1,    -1,    29,    -1,    -1,
      32,    25,    26,    27,    28,    29,    30,    31,    25,    26,
      42,    43,    29,    -1,    -1,    32,    40,    49,    -1,    36,
      -1,    -1,    -1,    55,     3,    42,    -1,    59,     7,     8,
      47,    -1,    -1,    -1,    -1,    -1,    68,    -1,    55,    71,
      72,    -1,    59,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    68,     3,    32,    71,    72,     7,     8,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    25,    26,    55,    -1,    29,    -1,
      59,    32,    -1,     3,    -1,    -1,    -1,     7,     8,    68,
      -1,    42,    71,    72,    -1,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    25,    26,    -1,    59,    29,
      -1,    -1,    32,    -1,     3,    -1,    36,    68,     7,     8,
      71,    72,    42,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    25,    26,    -1,    59,
      29,    -1,     3,    32,    -1,    -1,     7,     8,    68,    -1,
      -1,    71,    72,    42,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    26,    55,    -1,    29,    -1,
      59,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
       3,    42,    71,    72,     7,     8,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    59,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    68,    -1,    32,
      71,    72,    -1,    -1,    -1,    -1,    -1,    40,     3,    42,
      -1,    -1,     7,     8,    -1,    -1,    -1,    12,    -1,     3,
      -1,    -1,    55,     7,     8,    -1,    59,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    68,    -1,    32,    71,    72,
      -1,    25,    26,    -1,    -1,    29,    -1,    42,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,    42,    -1,
      55,     7,     8,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    68,    -1,    59,    71,    72,    -1,    25,
      26,    -1,    -1,    29,    68,    -1,    32,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    -1,    -1,    71,    72,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    -1,    -1,    40,     4,     5,     6,     7,     8,     9,
      -1,    48,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    -1,
      40,     4,     5,     6,     7,     8,     9,    47,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    36,    -1,    -1,    -1,    40,     4,     5,
       6,     7,     8,     9,    47,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,     4,     5,     6,     7,     8,
       9,    47,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,
      -1,    40,     4,     5,     6,     7,     8,     9,    47,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    43,    -1,    45,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,
      40,    41,    -1,    -1,    -1,    45,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    41,    -1,    -1,    -1,    45,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,    41,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    41,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    -1,
      40,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,     4,    40,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    39,    40,     4,    -1,     6,     7,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    -1,    -1,    -1,    39,    40,
       4,    -1,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    -1,    -1,    -1,    39,    40,     4,    -1,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    -1,    -1,    40,     5,     6,     7,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,     5,     6,
       7,     8,     9,    -1,    -1,    40,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,     5,     6,     7,     8,
       9,    -1,    -1,    40,    -1,    -1,    -1,    -1,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,     5,     6,     7,     8,     9,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,     5,     6,     7,     8,     9,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       5,     6,     7,     8,     9,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    78,    79,     0,     3,     7,     8,    25,    26,    29,
      32,    40,    42,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    62,    63,    64,    65,    66,
      67,    68,    71,    72,    73,    74,    81,    82,    83,    84,
      95,    96,    97,    98,   105,   112,   113,   115,   119,   125,
     126,   127,    81,   119,   119,   119,   119,   119,    80,    81,
     118,   119,     3,    72,    92,    93,    94,    81,    57,    92,
      93,     3,    72,     3,    81,    92,    40,     3,    52,    81,
      95,    97,     3,    81,    98,    42,    42,   125,    42,    48,
      48,    48,   119,   119,    42,    44,    46,    99,    73,    74,
      84,   125,    48,     3,   101,   102,   103,    42,    44,    46,
       4,     5,     6,     7,     8,     9,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    34,    35,    40,   120,   121,
     122,    48,    46,    41,    82,    43,    99,    43,    45,    43,
      45,     3,    42,    45,    48,    48,    42,    42,    48,    48,
      48,    51,    42,    50,    79,   105,    42,    46,     3,    34,
      42,    46,    99,   100,   119,   119,    62,    96,    97,   126,
     128,   131,    48,     3,    43,    49,   116,   117,   119,     3,
      36,    47,   114,   119,    46,    42,    99,    45,    34,    43,
     117,     3,   114,   119,   119,    10,    11,   123,   124,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,    13,   119,   119,   122,   121,   119,    43,   119,   119,
       3,     3,    86,    87,    97,    92,    93,     3,    88,    89,
      87,     3,    27,    90,    91,    87,    27,    91,    41,    89,
      42,    97,    43,    49,    69,   109,   110,   111,   119,    40,
      42,   106,    46,    99,    43,    43,    42,     3,    48,    45,
      34,   116,    43,    45,    85,   119,    47,    36,    47,    47,
      43,   109,   102,   104,   106,   119,    43,    47,    47,   119,
      12,   119,     5,     5,    36,   119,    41,    45,   121,   119,
      34,    43,    45,    43,    45,    43,     3,    48,    45,    48,
      43,    48,    48,    43,    43,   109,    42,    48,   127,   111,
      43,    45,    85,    97,    47,    41,    45,    49,   104,   107,
     108,    43,   109,   119,   125,   125,   119,    36,   119,   129,
     126,   119,   116,    49,   119,   127,    43,    39,   119,    39,
     119,    41,   119,    97,     3,    48,    86,    48,    88,    48,
       3,    90,    50,    48,    43,    43,   109,   127,   111,    49,
       3,   103,   104,    41,    49,    45,   127,    43,    47,    61,
      43,   119,    48,   116,   127,   119,    39,   119,    41,    45,
       3,    91,    48,    43,   111,   103,    34,    41,   104,   104,
     127,   125,    48,    43,   130,   131,   119,   119,    48,    48,
      48,    34,   104,    41,   125,    43,    41,   104,   125
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    78,    79,    79,    80,    80,    81,    81,    81,
      82,    82,    82,    82,    83,    83,    83,    83,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    85,    85,    86,    86,
      87,    87,    88,    89,    89,    90,    90,    91,    91,    92,
      92,    93,    93,    94,    94,    95,    96,    97,    97,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   103,   103,
     103,   103,   104,   104,   105,   106,   106,   106,   106,   107,
     107,   107,   108,   108,   109,   109,   109,   109,   110,   110,
     111,   111,   111,   111,   111,   112,   112,   113,   113,   113,
     113,   113,   114,   114,   114,   114,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   116,   116,
     117,   117,   117,   117,   118,   118,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   120,   120,   120,   120,   120,
     121,   121,   121,   121,   122,   122,   122,   122,   122,   123,
     123,   123,   123,   124,   124,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   126,   127,
     128,   128,   128,   129,   129,   130,   130,   131,   131
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     0,     2,     1,     3,     1,
       1,     1,     2,     2,     1,     1,     2,     2,     1,     1,
       1,     3,     5,     5,     3,     5,     5,     3,     3,     3,
       6,     6,     8,     6,     6,     8,     1,     0,     3,     1,
       1,     3,     1,     1,     3,     1,     3,     1,     3,     1,
       1,     1,     3,     1,     3,     2,     2,     1,     2,     1,
       2,     3,     3,     4,     1,     3,     1,     3,     1,     2,
       3,     4,     1,     1,     3,     2,     4,     3,     5,     1,
       1,     2,     1,     3,     1,     2,     3,     4,     1,     0,
       2,     3,     5,     4,     6,     5,     6,     3,     2,     5,
       7,     8,     1,     2,     2,     3,     3,     4,     4,     4,
       4,     3,     4,     3,     4,     3,     3,     1,     1,     3,
       1,     2,     3,     4,     3,     3,     1,     1,     1,     1,
       2,     4,     5,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     4,     3,
       4,     5,     6,     6,     7,     5,     3,     3,     3,     2,
       2,     2,     2,     3,     4,     1,     1,     2,     2,     3,
       4,     3,     5,     7,     1,     3,     3,     1,     1,     2,
       4,     3,     5,     2,     4,     1,     1,     2,     5,     7,
       5,     7,     9,     8,     2,     2,     2,     3,     1,     1,
       0,     1,     1,     0,     1,     0,     1,     1,     3
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
  case 2: /* file: fileblock  */
#line 195 "camp.y"
                   { absyntax::root = (yyvsp[0].b); }
#line 1856 "camp.tab.cc"
    break;

  case 3: /* fileblock: %empty  */
#line 199 "camp.y"
                   { (yyval.b) = new file(lexerPos(), false); }
#line 1862 "camp.tab.cc"
    break;

  case 4: /* fileblock: fileblock runnable  */
#line 201 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1868 "camp.tab.cc"
    break;

  case 5: /* bareblock: %empty  */
#line 205 "camp.y"
                   { (yyval.b) = new block(lexerPos(), true); }
#line 1874 "camp.tab.cc"
    break;

  case 6: /* bareblock: bareblock runnable  */
#line 207 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1880 "camp.tab.cc"
    break;

  case 7: /* name: ID  */
#line 211 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 1886 "camp.tab.cc"
    break;

  case 8: /* name: name '.' ID  */
#line 212 "camp.y"
                   { (yyval.n) = new qualifiedName((yyvsp[-1].pos), (yyvsp[-2].n), (yyvsp[0].ps).sym); }
#line 1892 "camp.tab.cc"
    break;

  case 9: /* name: '%'  */
#line 213 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos,
                                  symbol::trans("operator answer")); }
#line 1899 "camp.tab.cc"
    break;

  case 10: /* runnable: dec  */
#line 218 "camp.y"
                   { (yyval.run) = (yyvsp[0].d); }
#line 1905 "camp.tab.cc"
    break;

  case 11: /* runnable: stm  */
#line 219 "camp.y"
                   { (yyval.run) = (yyvsp[0].s); }
#line 1911 "camp.tab.cc"
    break;

  case 12: /* runnable: modifiers dec  */
#line 221 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].d)); }
#line 1917 "camp.tab.cc"
    break;

  case 13: /* runnable: modifiers stm  */
#line 223 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].s)); }
#line 1923 "camp.tab.cc"
    break;

  case 14: /* modifiers: MODIFIER  */
#line 227 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].mod).pos); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1929 "camp.tab.cc"
    break;

  case 15: /* modifiers: PERM  */
#line 228 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].perm).pos); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1935 "camp.tab.cc"
    break;

  case 16: /* modifiers: modifiers MODIFIER  */
#line 230 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1941 "camp.tab.cc"
    break;

  case 17: /* modifiers: modifiers PERM  */
#line 232 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1947 "camp.tab.cc"
    break;

  case 18: /* dec: vardec  */
#line 236 "camp.y"
                   { (yyval.d) = (yyvsp[0].vd); }
#line 1953 "camp.tab.cc"
    break;

  case 19: /* dec: fundec  */
#line 237 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1959 "camp.tab.cc"
    break;

  case 20: /* dec: typedec  */
#line 238 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1965 "camp.tab.cc"
    break;

  case 21: /* dec: ACCESS stridpairlist ';'  */
#line 240 "camp.y"
                   { (yyval.d) = new accessdec((yyvsp[-2].pos), (yyvsp[-1].ipl)); }
#line 1971 "camp.tab.cc"
    break;

  case 22: /* dec: FROM name UNRAVEL idpairlist ';'  */
#line 242 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), (yyvsp[-1].ipl)); }
#line 1977 "camp.tab.cc"
    break;

  case 23: /* dec: FROM name UNRAVEL '*' ';'  */
#line 244 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), WILDCARD); }
#line 1983 "camp.tab.cc"
    break;

  case 24: /* dec: UNRAVEL name ';'  */
#line 245 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-2].pos), (yyvsp[-1].n), WILDCARD); }
#line 1989 "camp.tab.cc"
    break;

  case 25: /* dec: FROM strid ACCESS idpairlist ';'  */
#line 247 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, (yyvsp[-1].ipl)); }
#line 1995 "camp.tab.cc"
    break;

  case 26: /* dec: FROM strid ACCESS '*' ';'  */
#line 249 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, WILDCARD); }
#line 2001 "camp.tab.cc"
    break;

  case 27: /* dec: IMPORT stridpair ';'  */
#line 251 "camp.y"
                   { (yyval.d) = new importdec((yyvsp[-2].pos), (yyvsp[-1].ip)); }
#line 2007 "camp.tab.cc"
    break;

  case 28: /* dec: INCLUDE ID ';'  */
#line 252 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].ps).sym); }
#line 2013 "camp.tab.cc"
    break;

  case 29: /* dec: INCLUDE STRING ';'  */
#line 254 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].stre)->getString()); }
#line 2019 "camp.tab.cc"
    break;

  case 30: /* dec: TYPEDEF IMPORT '(' typeparamlist ')' ';'  */
#line 258 "camp.y"
                   { (yyval.d) = new receiveTypedefDec((yyvsp[-5].pos), (yyvsp[-2].tps)); }
#line 2025 "camp.tab.cc"
    break;

  case 31: /* dec: IMPORT TYPEDEF '(' typeparamlist ')' ';'  */
#line 260 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[-5].pos),
                     "Expected 'typedef import(<types>);'");
                   }
#line 2033 "camp.tab.cc"
    break;

  case 32: /* dec: ACCESS strid '(' decdeclist ')' ID ID ';'  */
#line 265 "camp.y"
                   { (yyval.d) = new templateAccessDec(
                        (yyvsp[-7].pos), (yyvsp[-6].ps).sym, (yyvsp[-4].fls), (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym, (yyvsp[-2].ps).pos
                      ); }
#line 2041 "camp.tab.cc"
    break;

  case 33: /* dec: ACCESS strid '(' decdeclist ')' ';'  */
#line 269 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[0].pos), "expected 'as'"); }
#line 2047 "camp.tab.cc"
    break;

  case 34: /* dec: IMPORT strid '(' decdeclist ')' ';'  */
#line 271 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[-5].pos),
                        "Parametrized imports disallowed to reduce naming "
                        "conflicts. Try "
                        "'access <module>(<type parameters>) as <newname>;'."
                     ); }
#line 2057 "camp.tab.cc"
    break;

  case 35: /* dec: FROM strid '(' decdeclist ')' ACCESS idpairlist ';'  */
#line 277 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-7].pos), (yyvsp[-6].ps).sym, (yyvsp[-1].ipl), (yyvsp[-4].fls)); }
#line 2063 "camp.tab.cc"
    break;

  case 36: /* optionalcomma: ','  */
#line 281 "camp.y"
                   { (yyval.boo) = true; }
#line 2069 "camp.tab.cc"
    break;

  case 37: /* optionalcomma: %empty  */
#line 282 "camp.y"
                   { (yyval.boo) = false; }
#line 2075 "camp.tab.cc"
    break;

  case 38: /* decdec: ID ASSIGN type  */
#line 288 "camp.y"
                   { (yyval.fl) = new formal(
                        (yyvsp[-2].ps).pos, (yyvsp[0].t), new decidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym)
                      ); }
#line 2083 "camp.tab.cc"
    break;

  case 39: /* decdec: type  */
#line 291 "camp.y"
       { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), nullptr); }
#line 2089 "camp.tab.cc"
    break;

  case 40: /* decdeclist: decdec  */
#line 296 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2095 "camp.tab.cc"
    break;

  case 41: /* decdeclist: decdeclist ',' decdec  */
#line 298 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2101 "camp.tab.cc"
    break;

  case 42: /* typeparam: ID  */
#line 302 "camp.y"
       { (yyval.tp) = new typeParam((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2107 "camp.tab.cc"
    break;

  case 43: /* typeparamlist: typeparam  */
#line 307 "camp.y"
                   { (yyval.tps) = new typeParamList((yyvsp[0].tp)->getPos()); (yyval.tps)->add((yyvsp[0].tp)); }
#line 2113 "camp.tab.cc"
    break;

  case 44: /* typeparamlist: typeparamlist ',' typeparam  */
#line 309 "camp.y"
                   { (yyval.tps) = (yyvsp[-2].tps); (yyval.tps)->add((yyvsp[0].tp)); }
#line 2119 "camp.tab.cc"
    break;

  case 45: /* idpair: ID  */
#line 313 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2125 "camp.tab.cc"
    break;

  case 46: /* idpair: ID ID ID  */
#line 315 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2131 "camp.tab.cc"
    break;

  case 47: /* idpairlist: idpair  */
#line 319 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2137 "camp.tab.cc"
    break;

  case 48: /* idpairlist: idpairlist ',' idpair  */
#line 321 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2143 "camp.tab.cc"
    break;

  case 49: /* strid: ID  */
#line 325 "camp.y"
                   { (yyval.ps) = (yyvsp[0].ps); }
#line 2149 "camp.tab.cc"
    break;

  case 50: /* strid: STRING  */
#line 326 "camp.y"
                   { (yyval.ps).pos = (yyvsp[0].stre)->getPos();
                     (yyval.ps).sym = symbol::literalTrans((yyvsp[0].stre)->getString()); }
#line 2156 "camp.tab.cc"
    break;

  case 51: /* stridpair: ID  */
#line 331 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2162 "camp.tab.cc"
    break;

  case 52: /* stridpair: strid ID ID  */
#line 333 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2168 "camp.tab.cc"
    break;

  case 53: /* stridpairlist: stridpair  */
#line 337 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2174 "camp.tab.cc"
    break;

  case 54: /* stridpairlist: stridpairlist ',' stridpair  */
#line 339 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2180 "camp.tab.cc"
    break;

  case 55: /* vardec: barevardec ';'  */
#line 343 "camp.y"
                   { (yyval.vd) = (yyvsp[-1].vd); }
#line 2186 "camp.tab.cc"
    break;

  case 56: /* barevardec: type decidlist  */
#line 347 "camp.y"
                   { (yyval.vd) = new vardec((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dil)); }
#line 2192 "camp.tab.cc"
    break;

  case 57: /* type: celltype  */
#line 351 "camp.y"
                   { (yyval.t) = (yyvsp[0].t); }
#line 2198 "camp.tab.cc"
    break;

  case 58: /* type: name dims  */
#line 352 "camp.y"
                   { (yyval.t) = new arrayTy((yyvsp[-1].n), (yyvsp[0].dim)); }
#line 2204 "camp.tab.cc"
    break;

  case 59: /* celltype: name  */
#line 356 "camp.y"
                   { (yyval.t) = new nameTy((yyvsp[0].n)); }
#line 2210 "camp.tab.cc"
    break;

  case 60: /* dims: '[' ']'  */
#line 360 "camp.y"
                   { (yyval.dim) = new dimensions((yyvsp[-1].pos)); }
#line 2216 "camp.tab.cc"
    break;

  case 61: /* dims: dims '[' ']'  */
#line 361 "camp.y"
                   { (yyval.dim) = (yyvsp[-2].dim); (yyval.dim)->increase(); }
#line 2222 "camp.tab.cc"
    break;

  case 62: /* dimexps: '[' exp ']'  */
#line 365 "camp.y"
                   { (yyval.elist) = new explist((yyvsp[-2].pos)); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2228 "camp.tab.cc"
    break;

  case 63: /* dimexps: dimexps '[' exp ']'  */
#line 367 "camp.y"
                   { (yyval.elist) = (yyvsp[-3].elist); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2234 "camp.tab.cc"
    break;

  case 64: /* decidlist: decid  */
#line 371 "camp.y"
                   { (yyval.dil) = new decidlist((yyvsp[0].di)->getPos()); (yyval.dil)->add((yyvsp[0].di)); }
#line 2240 "camp.tab.cc"
    break;

  case 65: /* decidlist: decidlist ',' decid  */
#line 373 "camp.y"
                   { (yyval.dil) = (yyvsp[-2].dil); (yyval.dil)->add((yyvsp[0].di)); }
#line 2246 "camp.tab.cc"
    break;

  case 66: /* decid: decidstart  */
#line 377 "camp.y"
                   { (yyval.di) = new decid((yyvsp[0].dis)->getPos(), (yyvsp[0].dis)); }
#line 2252 "camp.tab.cc"
    break;

  case 67: /* decid: decidstart ASSIGN varinit  */
#line 379 "camp.y"
                   { (yyval.di) = new decid((yyvsp[-2].dis)->getPos(), (yyvsp[-2].dis), (yyvsp[0].vi)); }
#line 2258 "camp.tab.cc"
    break;

  case 68: /* decidstart: ID  */
#line 383 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2264 "camp.tab.cc"
    break;

  case 69: /* decidstart: ID dims  */
#line 384 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym, (yyvsp[0].dim)); }
#line 2270 "camp.tab.cc"
    break;

  case 70: /* decidstart: ID '(' ')'  */
#line 385 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, 0,
                                            new formals((yyvsp[-1].pos))); }
#line 2277 "camp.tab.cc"
    break;

  case 71: /* decidstart: ID '(' formals ')'  */
#line 388 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym, 0, (yyvsp[-1].fls)); }
#line 2283 "camp.tab.cc"
    break;

  case 72: /* varinit: exp  */
#line 392 "camp.y"
                   { (yyval.vi) = (yyvsp[0].e); }
#line 2289 "camp.tab.cc"
    break;

  case 73: /* varinit: arrayinit  */
#line 393 "camp.y"
                   { (yyval.vi) = (yyvsp[0].ai); }
#line 2295 "camp.tab.cc"
    break;

  case 74: /* block: '{' bareblock '}'  */
#line 398 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); }
#line 2301 "camp.tab.cc"
    break;

  case 75: /* arrayinit: '{' '}'  */
#line 402 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-1].pos)); }
#line 2307 "camp.tab.cc"
    break;

  case 76: /* arrayinit: '{' ELLIPSIS varinit '}'  */
#line 404 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-3].pos)); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2313 "camp.tab.cc"
    break;

  case 77: /* arrayinit: '{' basearrayinit '}'  */
#line 406 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2319 "camp.tab.cc"
    break;

  case 78: /* arrayinit: '{' basearrayinit ELLIPSIS varinit '}'  */
#line 408 "camp.y"
                   { (yyval.ai) = (yyvsp[-3].ai); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2325 "camp.tab.cc"
    break;

  case 79: /* basearrayinit: ','  */
#line 412 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].pos)); }
#line 2331 "camp.tab.cc"
    break;

  case 80: /* basearrayinit: varinits  */
#line 413 "camp.y"
                   { (yyval.ai) = (yyvsp[0].ai); }
#line 2337 "camp.tab.cc"
    break;

  case 81: /* basearrayinit: varinits ','  */
#line 414 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2343 "camp.tab.cc"
    break;

  case 82: /* varinits: varinit  */
#line 418 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].vi)->getPos());
		     (yyval.ai)->add((yyvsp[0].vi));}
#line 2350 "camp.tab.cc"
    break;

  case 83: /* varinits: varinits ',' varinit  */
#line 421 "camp.y"
                   { (yyval.ai) = (yyvsp[-2].ai); (yyval.ai)->add((yyvsp[0].vi)); }
#line 2356 "camp.tab.cc"
    break;

  case 84: /* formals: formal  */
#line 425 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2362 "camp.tab.cc"
    break;

  case 85: /* formals: ELLIPSIS formal  */
#line 426 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[-1].pos)); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2368 "camp.tab.cc"
    break;

  case 86: /* formals: formals ',' formal  */
#line 428 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2374 "camp.tab.cc"
    break;

  case 87: /* formals: formals optionalcomma ELLIPSIS formal  */
#line 430 "camp.y"
                   { (yyval.fls) = (yyvsp[-3].fls); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2380 "camp.tab.cc"
    break;

  case 88: /* explicitornot: EXPLICIT  */
#line 434 "camp.y"
                   { (yyval.boo) = true; }
#line 2386 "camp.tab.cc"
    break;

  case 89: /* explicitornot: %empty  */
#line 435 "camp.y"
                   { (yyval.boo) = false; }
#line 2392 "camp.tab.cc"
    break;

  case 90: /* formal: explicitornot type  */
#line 440 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), 0, 0, (yyvsp[-1].boo), 0); }
#line 2398 "camp.tab.cc"
    break;

  case 91: /* formal: explicitornot type decidstart  */
#line 442 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dis), 0, (yyvsp[-2].boo), 0); }
#line 2404 "camp.tab.cc"
    break;

  case 92: /* formal: explicitornot type decidstart ASSIGN varinit  */
#line 444 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-3].t)->getPos(), (yyvsp[-3].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-4].boo), 0); }
#line 2410 "camp.tab.cc"
    break;

  case 93: /* formal: explicitornot type ID decidstart  */
#line 447 "camp.y"
                   { bool k = checkKeyword((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-2].t)->getPos(), (yyvsp[-2].t), (yyvsp[0].dis), 0, (yyvsp[-3].boo), k); }
#line 2417 "camp.tab.cc"
    break;

  case 94: /* formal: explicitornot type ID decidstart ASSIGN varinit  */
#line 450 "camp.y"
                   { bool k = checkKeyword((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-5].boo), k); }
#line 2424 "camp.tab.cc"
    break;

  case 95: /* fundec: type ID '(' ')' blockstm  */
#line 456 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-2].pos), (yyvsp[-4].t), (yyvsp[-3].ps).sym, new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2430 "camp.tab.cc"
    break;

  case 96: /* fundec: type ID '(' formals ')' blockstm  */
#line 458 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-3].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2436 "camp.tab.cc"
    break;

  case 97: /* typedec: STRUCT ID block  */
#line 462 "camp.y"
                   { (yyval.d) = new recorddec((yyvsp[-2].pos), (yyvsp[-1].ps).sym, (yyvsp[0].b)); }
#line 2442 "camp.tab.cc"
    break;

  case 98: /* typedec: TYPEDEF vardec  */
#line 463 "camp.y"
                   { (yyval.d) = new typedec((yyvsp[-1].pos), (yyvsp[0].vd)); }
#line 2448 "camp.tab.cc"
    break;

  case 99: /* typedec: USING ID ASSIGN type ';'  */
#line 469 "camp.y"
                   { decidstart *dis = new decidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.d) = new typedec((yyvsp[-4].pos), dis, (yyvsp[-1].t)); }
#line 2455 "camp.tab.cc"
    break;

  case 100: /* typedec: USING ID ASSIGN type '(' ')' ';'  */
#line 472 "camp.y"
                   { decidstart *dis = new fundecidstart((yyvsp[-5].ps).pos, (yyvsp[-5].ps).sym,
                                                         0, new formals((yyvsp[-2].pos)));
                     (yyval.d) = new typedec((yyvsp[-6].pos), dis, (yyvsp[-3].t)); }
#line 2463 "camp.tab.cc"
    break;

  case 101: /* typedec: USING ID ASSIGN type '(' formals ')' ';'  */
#line 476 "camp.y"
                   { decidstart *dis = new fundecidstart((yyvsp[-6].ps).pos, (yyvsp[-6].ps).sym, 0, (yyvsp[-2].fls));
                     (yyval.d) = new typedec((yyvsp[-7].pos), dis, (yyvsp[-4].t)); }
#line 2470 "camp.tab.cc"
    break;

  case 102: /* slice: ':'  */
#line 481 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), 0, 0); }
#line 2476 "camp.tab.cc"
    break;

  case 103: /* slice: exp ':'  */
#line 482 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), (yyvsp[-1].e), 0); }
#line 2482 "camp.tab.cc"
    break;

  case 104: /* slice: ':' exp  */
#line 483 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), 0, (yyvsp[0].e)); }
#line 2488 "camp.tab.cc"
    break;

  case 105: /* slice: exp ':' exp  */
#line 484 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2494 "camp.tab.cc"
    break;

  case 106: /* value: value '.' ID  */
#line 488 "camp.y"
                   { (yyval.e) = new fieldExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].ps).sym); }
#line 2500 "camp.tab.cc"
    break;

  case 107: /* value: name '[' exp ']'  */
#line 489 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].e)); }
#line 2507 "camp.tab.cc"
    break;

  case 108: /* value: value '[' exp ']'  */
#line 491 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e)); }
#line 2513 "camp.tab.cc"
    break;

  case 109: /* value: name '[' slice ']'  */
#line 492 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].slice)); }
#line 2520 "camp.tab.cc"
    break;

  case 110: /* value: value '[' slice ']'  */
#line 494 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].slice)); }
#line 2526 "camp.tab.cc"
    break;

  case 111: /* value: name '(' ')'  */
#line 495 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos),
                                      new nameExp((yyvsp[-2].n)->getPos(), (yyvsp[-2].n)),
                                      new arglist()); }
#line 2534 "camp.tab.cc"
    break;

  case 112: /* value: name '(' arglist ')'  */
#line 499 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos),
                                      new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)),
                                      (yyvsp[-1].alist)); }
#line 2542 "camp.tab.cc"
    break;

  case 113: /* value: value '(' ')'  */
#line 502 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos), (yyvsp[-2].e), new arglist()); }
#line 2548 "camp.tab.cc"
    break;

  case 114: /* value: value '(' arglist ')'  */
#line 504 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].alist)); }
#line 2554 "camp.tab.cc"
    break;

  case 115: /* value: '(' exp ')'  */
#line 506 "camp.y"
                   { (yyval.e) = (yyvsp[-1].e); }
#line 2560 "camp.tab.cc"
    break;

  case 116: /* value: '(' name ')'  */
#line 508 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[-1].n)->getPos(), (yyvsp[-1].n)); }
#line 2566 "camp.tab.cc"
    break;

  case 117: /* value: THIS_TOK  */
#line 509 "camp.y"
                   { (yyval.e) = new thisExp((yyvsp[0].pos)); }
#line 2572 "camp.tab.cc"
    break;

  case 118: /* argument: exp  */
#line 513 "camp.y"
                   { (yyval.arg).name = symbol::nullsym; (yyval.arg).val=(yyvsp[0].e); }
#line 2578 "camp.tab.cc"
    break;

  case 119: /* argument: ID ASSIGN exp  */
#line 514 "camp.y"
                   { (yyval.arg).name = (yyvsp[-2].ps).sym; (yyval.arg).val=(yyvsp[0].e); }
#line 2584 "camp.tab.cc"
    break;

  case 120: /* arglist: argument  */
#line 518 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2590 "camp.tab.cc"
    break;

  case 121: /* arglist: ELLIPSIS argument  */
#line 520 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2596 "camp.tab.cc"
    break;

  case 122: /* arglist: arglist ',' argument  */
#line 522 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2602 "camp.tab.cc"
    break;

  case 123: /* arglist: arglist optionalcomma ELLIPSIS argument  */
#line 524 "camp.y"
                   { (yyval.alist) = (yyvsp[-3].alist); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2608 "camp.tab.cc"
    break;

  case 124: /* tuple: exp ',' exp  */
#line 529 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[-2].e)); (yyval.alist)->add((yyvsp[0].e)); }
#line 2614 "camp.tab.cc"
    break;

  case 125: /* tuple: tuple ',' exp  */
#line 530 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].e)); }
#line 2620 "camp.tab.cc"
    break;

  case 126: /* exp: name  */
#line 534 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[0].n)->getPos(), (yyvsp[0].n)); }
#line 2626 "camp.tab.cc"
    break;

  case 127: /* exp: value  */
#line 535 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2632 "camp.tab.cc"
    break;

  case 128: /* exp: LIT  */
#line 536 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2638 "camp.tab.cc"
    break;

  case 129: /* exp: STRING  */
#line 537 "camp.y"
                   { (yyval.e) = (yyvsp[0].stre); }
#line 2644 "camp.tab.cc"
    break;

  case 130: /* exp: LIT exp  */
#line 539 "camp.y"
                   { (yyval.e) = new scaleExp((yyvsp[-1].e)->getPos(), (yyvsp[-1].e), (yyvsp[0].e)); }
#line 2650 "camp.tab.cc"
    break;

  case 131: /* exp: '(' name ')' exp  */
#line 541 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-2].n)->getPos(), new nameTy((yyvsp[-2].n)), (yyvsp[0].e)); }
#line 2656 "camp.tab.cc"
    break;

  case 132: /* exp: '(' name dims ')' exp  */
#line 543 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-3].n)->getPos(), new arrayTy((yyvsp[-3].n), (yyvsp[-2].dim)), (yyvsp[0].e)); }
#line 2662 "camp.tab.cc"
    break;

  case 133: /* exp: '+' exp  */
#line 545 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2668 "camp.tab.cc"
    break;

  case 134: /* exp: '-' exp  */
#line 547 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2674 "camp.tab.cc"
    break;

  case 135: /* exp: OPERATOR exp  */
#line 548 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2680 "camp.tab.cc"
    break;

  case 136: /* exp: exp '+' exp  */
#line 549 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2686 "camp.tab.cc"
    break;

  case 137: /* exp: exp '-' exp  */
#line 550 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2692 "camp.tab.cc"
    break;

  case 138: /* exp: exp '*' exp  */
#line 551 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2698 "camp.tab.cc"
    break;

  case 139: /* exp: exp '/' exp  */
#line 552 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2704 "camp.tab.cc"
    break;

  case 140: /* exp: exp '%' exp  */
#line 553 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2710 "camp.tab.cc"
    break;

  case 141: /* exp: exp '#' exp  */
#line 554 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2716 "camp.tab.cc"
    break;

  case 142: /* exp: exp '^' exp  */
#line 555 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2722 "camp.tab.cc"
    break;

  case 143: /* exp: exp LT exp  */
#line 556 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2728 "camp.tab.cc"
    break;

  case 144: /* exp: exp LE exp  */
#line 557 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2734 "camp.tab.cc"
    break;

  case 145: /* exp: exp GT exp  */
#line 558 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2740 "camp.tab.cc"
    break;

  case 146: /* exp: exp GE exp  */
#line 559 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2746 "camp.tab.cc"
    break;

  case 147: /* exp: exp EQ exp  */
#line 560 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2752 "camp.tab.cc"
    break;

  case 148: /* exp: exp NEQ exp  */
#line 561 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2758 "camp.tab.cc"
    break;

  case 149: /* exp: exp CAND exp  */
#line 562 "camp.y"
                   { (yyval.e) = new andExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2764 "camp.tab.cc"
    break;

  case 150: /* exp: exp COR exp  */
#line 563 "camp.y"
                   { (yyval.e) = new orExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2770 "camp.tab.cc"
    break;

  case 151: /* exp: exp CARETS exp  */
#line 564 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2776 "camp.tab.cc"
    break;

  case 152: /* exp: exp AMPERSAND exp  */
#line 565 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2782 "camp.tab.cc"
    break;

  case 153: /* exp: exp BAR exp  */
#line 566 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2788 "camp.tab.cc"
    break;

  case 154: /* exp: exp OPERATOR exp  */
#line 567 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2794 "camp.tab.cc"
    break;

  case 155: /* exp: exp INCR exp  */
#line 568 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2800 "camp.tab.cc"
    break;

  case 156: /* exp: NEW celltype  */
#line 570 "camp.y"
                   { (yyval.e) = new newRecordExp((yyvsp[-1].pos), (yyvsp[0].t)); }
#line 2806 "camp.tab.cc"
    break;

  case 157: /* exp: NEW celltype dimexps  */
#line 572 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), (yyvsp[0].elist), 0, 0); }
#line 2812 "camp.tab.cc"
    break;

  case 158: /* exp: NEW celltype dimexps dims  */
#line 574 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), (yyvsp[-1].elist), (yyvsp[0].dim), 0); }
#line 2818 "camp.tab.cc"
    break;

  case 159: /* exp: NEW celltype dims  */
#line 576 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), 0, (yyvsp[0].dim), 0); }
#line 2824 "camp.tab.cc"
    break;

  case 160: /* exp: NEW celltype dims arrayinit  */
#line 578 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), 0, (yyvsp[-1].dim), (yyvsp[0].ai)); }
#line 2830 "camp.tab.cc"
    break;

  case 161: /* exp: NEW celltype '(' ')' blockstm  */
#line 580 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-4].pos), (yyvsp[-3].t), new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2836 "camp.tab.cc"
    break;

  case 162: /* exp: NEW celltype dims '(' ')' blockstm  */
#line 582 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos),
                                             new arrayTy((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-3].dim)),
                                             new formals((yyvsp[-2].pos)),
                                             (yyvsp[0].s)); }
#line 2845 "camp.tab.cc"
    break;

  case 163: /* exp: NEW celltype '(' formals ')' blockstm  */
#line 587 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos), (yyvsp[-4].t), (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2851 "camp.tab.cc"
    break;

  case 164: /* exp: NEW celltype dims '(' formals ')' blockstm  */
#line 589 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-6].pos),
                                             new arrayTy((yyvsp[-5].t)->getPos(), (yyvsp[-5].t), (yyvsp[-4].dim)),
                                             (yyvsp[-2].fls),
                                             (yyvsp[0].s)); }
#line 2860 "camp.tab.cc"
    break;

  case 165: /* exp: exp '?' exp ':' exp  */
#line 594 "camp.y"
                   { (yyval.e) = new conditionalExp((yyvsp[-3].pos), (yyvsp[-4].e), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2866 "camp.tab.cc"
    break;

  case 166: /* exp: exp ASSIGN exp  */
#line 595 "camp.y"
                   { (yyval.e) = new assignExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2872 "camp.tab.cc"
    break;

  case 167: /* exp: '(' tuple ')'  */
#line 596 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), new nameExp((yyvsp[-2].pos), SYM_TUPLE), (yyvsp[-1].alist)); }
#line 2878 "camp.tab.cc"
    break;

  case 168: /* exp: exp join exp  */
#line 598 "camp.y"
                   { (yyvsp[-1].j)->pushFront((yyvsp[-2].e)); (yyvsp[-1].j)->pushBack((yyvsp[0].e)); (yyval.e) = (yyvsp[-1].j); }
#line 2884 "camp.tab.cc"
    break;

  case 169: /* exp: exp dir  */
#line 600 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::OUT);
                     joinExp *jexp =
                         new joinExp((yyvsp[0].se)->getPos(), SYM_DOTS);
                     (yyval.e)=jexp;
                     jexp->pushBack((yyvsp[-1].e)); jexp->pushBack((yyvsp[0].se)); }
#line 2894 "camp.tab.cc"
    break;

  case 170: /* exp: INCR exp  */
#line 606 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_PLUS); }
#line 2900 "camp.tab.cc"
    break;

  case 171: /* exp: DASHES exp  */
#line 608 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_MINUS); }
#line 2906 "camp.tab.cc"
    break;

  case 172: /* exp: exp INCR  */
#line 611 "camp.y"
                   { (yyval.e) = new postfixExp((yyvsp[0].ps).pos, (yyvsp[-1].e), SYM_PLUS); }
#line 2912 "camp.tab.cc"
    break;

  case 173: /* exp: exp SELFOP exp  */
#line 612 "camp.y"
                   { (yyval.e) = new selfExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2918 "camp.tab.cc"
    break;

  case 174: /* exp: QUOTE '{' fileblock '}'  */
#line 614 "camp.y"
                   { (yyval.e) = new quoteExp((yyvsp[-3].pos), (yyvsp[-1].b)); }
#line 2924 "camp.tab.cc"
    break;

  case 175: /* join: DASHES  */
#line 620 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos,(yyvsp[0].ps).sym); }
#line 2930 "camp.tab.cc"
    break;

  case 176: /* join: basicjoin  */
#line 622 "camp.y"
                   { (yyval.j) = (yyvsp[0].j); }
#line 2936 "camp.tab.cc"
    break;

  case 177: /* join: dir basicjoin  */
#line 624 "camp.y"
                   { (yyvsp[-1].se)->setSide(camp::OUT);
                     (yyval.j) = (yyvsp[0].j); (yyval.j)->pushFront((yyvsp[-1].se)); }
#line 2943 "camp.tab.cc"
    break;

  case 178: /* join: basicjoin dir  */
#line 627 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2950 "camp.tab.cc"
    break;

  case 179: /* join: dir basicjoin dir  */
#line 630 "camp.y"
                   { (yyvsp[-2].se)->setSide(camp::OUT); (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushFront((yyvsp[-2].se)); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2957 "camp.tab.cc"
    break;

  case 180: /* dir: '{' CURL exp '}'  */
#line 635 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].e)); }
#line 2963 "camp.tab.cc"
    break;

  case 181: /* dir: '{' exp '}'  */
#line 636 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].pos), symbol::opTrans("spec"), (yyvsp[-1].e)); }
#line 2969 "camp.tab.cc"
    break;

  case 182: /* dir: '{' exp ',' exp '}'  */
#line 638 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-4].pos), symbol::opTrans("spec"),
				      new pairExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2976 "camp.tab.cc"
    break;

  case 183: /* dir: '{' exp ',' exp ',' exp '}'  */
#line 641 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-6].pos), symbol::opTrans("spec"),
				      new tripleExp((yyvsp[-4].pos), (yyvsp[-5].e), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2983 "camp.tab.cc"
    break;

  case 184: /* basicjoin: DOTS  */
#line 646 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2989 "camp.tab.cc"
    break;

  case 185: /* basicjoin: DOTS tension DOTS  */
#line 648 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2995 "camp.tab.cc"
    break;

  case 186: /* basicjoin: DOTS controls DOTS  */
#line 650 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 3001 "camp.tab.cc"
    break;

  case 187: /* basicjoin: COLONS  */
#line 651 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 3007 "camp.tab.cc"
    break;

  case 188: /* basicjoin: LONGDASH  */
#line 652 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 3013 "camp.tab.cc"
    break;

  case 189: /* tension: TENSION exp  */
#line 656 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, false)); }
#line 3020 "camp.tab.cc"
    break;

  case 190: /* tension: TENSION exp AND exp  */
#line 659 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, false)); }
#line 3027 "camp.tab.cc"
    break;

  case 191: /* tension: TENSION ATLEAST exp  */
#line 662 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-2].ps).pos, (yyvsp[0].e), (yyvsp[-2].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, true)); }
#line 3034 "camp.tab.cc"
    break;

  case 192: /* tension: TENSION ATLEAST exp AND exp  */
#line 665 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-4].ps).pos, (yyvsp[-2].e), (yyvsp[-4].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, true)); }
#line 3041 "camp.tab.cc"
    break;

  case 193: /* controls: CONTROLS exp  */
#line 670 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 3047 "camp.tab.cc"
    break;

  case 194: /* controls: CONTROLS exp AND exp  */
#line 672 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e)); }
#line 3053 "camp.tab.cc"
    break;

  case 195: /* stm: ';'  */
#line 676 "camp.y"
                   { (yyval.s) = new emptyStm((yyvsp[0].pos)); }
#line 3059 "camp.tab.cc"
    break;

  case 196: /* stm: blockstm  */
#line 677 "camp.y"
                   { (yyval.s) = (yyvsp[0].s); }
#line 3065 "camp.tab.cc"
    break;

  case 197: /* stm: stmexp ';'  */
#line 678 "camp.y"
                   { (yyval.s) = (yyvsp[-1].s); }
#line 3071 "camp.tab.cc"
    break;

  case 198: /* stm: IF '(' exp ')' stm  */
#line 680 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3077 "camp.tab.cc"
    break;

  case 199: /* stm: IF '(' exp ')' stm ELSE stm  */
#line 682 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-6].pos), (yyvsp[-4].e), (yyvsp[-2].s), (yyvsp[0].s)); }
#line 3083 "camp.tab.cc"
    break;

  case 200: /* stm: WHILE '(' exp ')' stm  */
#line 684 "camp.y"
                   { (yyval.s) = new whileStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3089 "camp.tab.cc"
    break;

  case 201: /* stm: DO stm WHILE '(' exp ')' ';'  */
#line 686 "camp.y"
                   { (yyval.s) = new doStm((yyvsp[-6].pos), (yyvsp[-5].s), (yyvsp[-2].e)); }
#line 3095 "camp.tab.cc"
    break;

  case 202: /* stm: FOR '(' forinit ';' fortest ';' forupdate ')' stm  */
#line 688 "camp.y"
                   { (yyval.s) = new forStm((yyvsp[-8].pos), (yyvsp[-6].run), (yyvsp[-4].e), (yyvsp[-2].sel), (yyvsp[0].s)); }
#line 3101 "camp.tab.cc"
    break;

  case 203: /* stm: FOR '(' type ID ':' exp ')' stm  */
#line 690 "camp.y"
                   { (yyval.s) = new extendedForStm((yyvsp[-7].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3107 "camp.tab.cc"
    break;

  case 204: /* stm: BREAK ';'  */
#line 691 "camp.y"
                   { (yyval.s) = new breakStm((yyvsp[-1].pos)); }
#line 3113 "camp.tab.cc"
    break;

  case 205: /* stm: CONTINUE ';'  */
#line 692 "camp.y"
                   { (yyval.s) = new continueStm((yyvsp[-1].pos)); }
#line 3119 "camp.tab.cc"
    break;

  case 206: /* stm: RETURN_ ';'  */
#line 693 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-1].pos)); }
#line 3125 "camp.tab.cc"
    break;

  case 207: /* stm: RETURN_ exp ';'  */
#line 694 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-2].pos), (yyvsp[-1].e)); }
#line 3131 "camp.tab.cc"
    break;

  case 208: /* stmexp: exp  */
#line 698 "camp.y"
                   { (yyval.s) = new expStm((yyvsp[0].e)->getPos(), (yyvsp[0].e)); }
#line 3137 "camp.tab.cc"
    break;

  case 209: /* blockstm: block  */
#line 702 "camp.y"
                   { (yyval.s) = new blockStm((yyvsp[0].b)->getPos(), (yyvsp[0].b)); }
#line 3143 "camp.tab.cc"
    break;

  case 210: /* forinit: %empty  */
#line 706 "camp.y"
                   { (yyval.run) = 0; }
#line 3149 "camp.tab.cc"
    break;

  case 211: /* forinit: stmexplist  */
#line 707 "camp.y"
                   { (yyval.run) = (yyvsp[0].sel); }
#line 3155 "camp.tab.cc"
    break;

  case 212: /* forinit: barevardec  */
#line 708 "camp.y"
                   { (yyval.run) = (yyvsp[0].vd); }
#line 3161 "camp.tab.cc"
    break;

  case 213: /* fortest: %empty  */
#line 712 "camp.y"
                   { (yyval.e) = 0; }
#line 3167 "camp.tab.cc"
    break;

  case 214: /* fortest: exp  */
#line 713 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 3173 "camp.tab.cc"
    break;

  case 215: /* forupdate: %empty  */
#line 717 "camp.y"
                   { (yyval.sel) = 0; }
#line 3179 "camp.tab.cc"
    break;

  case 216: /* forupdate: stmexplist  */
#line 718 "camp.y"
                   { (yyval.sel) = (yyvsp[0].sel); }
#line 3185 "camp.tab.cc"
    break;

  case 217: /* stmexplist: stmexp  */
#line 722 "camp.y"
                   { (yyval.sel) = new stmExpList((yyvsp[0].s)->getPos()); (yyval.sel)->add((yyvsp[0].s)); }
#line 3191 "camp.tab.cc"
    break;

  case 218: /* stmexplist: stmexplist ',' stmexp  */
#line 724 "camp.y"
                   { (yyval.sel) = (yyvsp[-2].sel); (yyval.sel)->add((yyvsp[0].s)); }
#line 3197 "camp.tab.cc"
    break;


#line 3201 "camp.tab.cc"

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
      yyerror (YY_("syntax error"));
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

  return yyresult;
}

