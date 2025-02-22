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
  YYSYMBOL_decdec = 85,                    /* decdec  */
  YYSYMBOL_decdeclist = 86,                /* decdeclist  */
  YYSYMBOL_typeparam = 87,                 /* typeparam  */
  YYSYMBOL_typeparamlist = 88,             /* typeparamlist  */
  YYSYMBOL_idpair = 89,                    /* idpair  */
  YYSYMBOL_idpairlist = 90,                /* idpairlist  */
  YYSYMBOL_strid = 91,                     /* strid  */
  YYSYMBOL_stridpair = 92,                 /* stridpair  */
  YYSYMBOL_stridpairlist = 93,             /* stridpairlist  */
  YYSYMBOL_vardec = 94,                    /* vardec  */
  YYSYMBOL_barevardec = 95,                /* barevardec  */
  YYSYMBOL_type = 96,                      /* type  */
  YYSYMBOL_celltype = 97,                  /* celltype  */
  YYSYMBOL_dims = 98,                      /* dims  */
  YYSYMBOL_dimexps = 99,                   /* dimexps  */
  YYSYMBOL_decidlist = 100,                /* decidlist  */
  YYSYMBOL_decid = 101,                    /* decid  */
  YYSYMBOL_decidstart = 102,               /* decidstart  */
  YYSYMBOL_varinit = 103,                  /* varinit  */
  YYSYMBOL_block = 104,                    /* block  */
  YYSYMBOL_arrayinit = 105,                /* arrayinit  */
  YYSYMBOL_basearrayinit = 106,            /* basearrayinit  */
  YYSYMBOL_varinits = 107,                 /* varinits  */
  YYSYMBOL_formals = 108,                  /* formals  */
  YYSYMBOL_explicitornot = 109,            /* explicitornot  */
  YYSYMBOL_formal = 110,                   /* formal  */
  YYSYMBOL_fundec = 111,                   /* fundec  */
  YYSYMBOL_typedec = 112,                  /* typedec  */
  YYSYMBOL_slice = 113,                    /* slice  */
  YYSYMBOL_value = 114,                    /* value  */
  YYSYMBOL_argument = 115,                 /* argument  */
  YYSYMBOL_arglist = 116,                  /* arglist  */
  YYSYMBOL_tuple = 117,                    /* tuple  */
  YYSYMBOL_exp = 118,                      /* exp  */
  YYSYMBOL_join = 119,                     /* join  */
  YYSYMBOL_dir = 120,                      /* dir  */
  YYSYMBOL_basicjoin = 121,                /* basicjoin  */
  YYSYMBOL_tension = 122,                  /* tension  */
  YYSYMBOL_controls = 123,                 /* controls  */
  YYSYMBOL_stm = 124,                      /* stm  */
  YYSYMBOL_stmexp = 125,                   /* stmexp  */
  YYSYMBOL_blockstm = 126,                 /* blockstm  */
  YYSYMBOL_forinit = 127,                  /* forinit  */
  YYSYMBOL_fortest = 128,                  /* fortest  */
  YYSYMBOL_forupdate = 129,                /* forupdate  */
  YYSYMBOL_stmexplist = 130                /* stmexplist  */
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
#define YYLAST   1984

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  216
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  427

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
     257,   259,   264,   268,   270,   276,   282,   286,   290,   292,
     297,   301,   303,   308,   310,   314,   315,   320,   321,   326,
     328,   332,   333,   338,   342,   346,   347,   351,   355,   356,
     360,   361,   366,   367,   372,   373,   378,   379,   380,   382,
     387,   388,   392,   397,   398,   400,   402,   407,   408,   409,
     413,   415,   420,   421,   422,   424,   429,   430,   434,   436,
     438,   441,   444,   450,   452,   457,   458,   463,   466,   470,
     476,   477,   478,   479,   483,   484,   486,   487,   489,   490,
     493,   497,   498,   500,   502,   504,   508,   509,   513,   514,
     516,   518,   524,   525,   529,   530,   531,   532,   534,   535,
     537,   539,   541,   543,   544,   545,   546,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   562,   563,   564,   566,   568,   570,   572,   574,
     576,   581,   583,   588,   590,   591,   592,   594,   600,   602,
     605,   607,   608,   615,   616,   618,   621,   624,   630,   631,
     632,   635,   641,   642,   644,   646,   647,   651,   653,   656,
     659,   665,   666,   671,   672,   673,   674,   676,   678,   680,
     682,   684,   686,   687,   688,   689,   693,   697,   701,   702,
     703,   707,   708,   712,   713,   717,   718
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
  "decdec", "decdeclist", "typeparam", "typeparamlist", "idpair",
  "idpairlist", "strid", "stridpair", "stridpairlist", "vardec",
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

#define YYPACT_NINF (-312)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-58)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -312,    65,   450,  -312,  -312,   903,   903,   903,   903,  -312,
     903,  -312,   903,  -312,    34,    32,    14,    38,    18,   134,
     199,    15,   200,    32,   137,   171,   594,   172,   180,   181,
     721,  -312,   217,  -312,  -312,  -312,    30,  -312,   522,  -312,
    -312,   196,   258,  -312,  -312,  -312,  -312,   150,  1558,  -312,
     246,  -312,   224,   274,   274,   274,   274,  1944,   183,   314,
      12,  1159,    94,  -312,    21,  -312,   139,   127,   266,    36,
     261,   283,   287,    -2,     1,    50,  -312,   294,   301,    39,
    -312,   344,   316,   307,   153,   903,   903,   293,   903,  -312,
    -312,  -312,   974,   274,   596,   345,   666,   315,  -312,  -312,
    -312,  -312,  -312,   216,   317,  -312,   330,   668,   362,   723,
     903,   147,  -312,  -312,   217,  -312,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   903,   903,   903,
     903,   903,   903,   903,   903,   903,   903,   793,   903,  -312,
     326,  -312,   723,  -312,  -312,   217,   184,  -312,   903,  -312,
     903,   365,    35,    34,  -312,  -312,   366,    35,  -312,  -312,
    -312,    57,    35,    67,   378,  -312,   366,   323,   250,    32,
      19,   795,   136,   325,  1285,  1325,   331,  -312,   371,  -312,
     329,   333,  -312,   348,  -312,   905,  -312,   161,  1558,  -312,
     903,  -312,   336,  1011,   337,    95,   315,   344,   848,  -312,
     162,  -312,   340,  1048,  1558,   903,   850,   383,   386,   297,
    1776,  1804,  1832,  1860,  1888,  1888,  1916,  1916,  1916,  1916,
     803,   286,   286,   274,   274,   274,   274,   274,  1944,  1558,
    1521,   903,  1201,   297,   326,  -312,  -312,   903,  1558,  1558,
    -312,   358,  -312,    52,  -312,   391,  -312,  -312,  -312,   176,
     276,   392,   349,  -312,   232,   295,   350,   253,  -312,   296,
     104,   141,   294,   327,  -312,   173,    32,  -312,  1085,   304,
     112,  -312,   795,   315,   594,   594,   903,     7,   903,   903,
     903,  -312,  -312,   905,   905,  1558,  -312,   903,  -312,  -312,
     294,   220,  -312,  -312,  -312,  1558,  -312,  -312,  -312,  1593,
     903,  1630,  -312,  -312,   903,  1445,  -312,   903,  -312,  -312,
      32,    29,    35,   352,   366,   354,   402,  -312,   403,  -312,
     359,  -312,  -312,   360,  -312,   238,   119,  -312,  -312,  -312,
     294,   327,   327,   408,  -312,  -312,  -312,   848,  -312,     9,
     367,   294,   241,  1122,   355,  -312,  1365,   903,  1558,   369,
    -312,  1558,  -312,  -312,  1558,  -312,   294,   903,  1667,   903,
    1740,  -312,  1243,  -312,   410,  -312,  -312,  -312,  -312,  -312,
    -312,  -312,   403,  -312,  -312,   373,   248,  -312,  -312,  -312,
      33,   381,   382,  -312,   848,   848,  -312,   294,  -312,   594,
     374,  1405,   903,  -312,  1704,   903,  1704,  -312,   903,   376,
     255,  -312,   377,   393,   848,  -312,   398,  -312,  -312,  -312,
    -312,   594,   404,   333,  1704,  1483,  -312,  -312,  -312,   848,
    -312,  -312,  -312,   594,  -312,  -312,  -312
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     7,     0,     0,     0,     0,     9,
       0,     5,     0,   193,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   115,   126,   127,    15,    14,   124,     4,     0,    10,
      18,     0,     0,    55,   207,    19,    20,   125,   206,    11,
       0,   194,   124,   169,   168,   131,   132,   133,     0,   124,
       0,     0,    47,    48,     0,    51,     0,     0,     0,     0,
       0,     0,     0,     7,     0,     0,     3,     0,     0,    57,
      96,     0,     0,    57,   154,     0,     0,     0,   208,   202,
     203,   204,     0,   128,     0,     0,     0,    56,    17,    16,
      12,    13,    53,    66,    54,    62,    64,     0,     0,     0,
       0,   182,   185,   173,   170,   186,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   167,
     174,   195,     0,    72,     6,   114,     0,   165,     0,   113,
       0,     0,     0,     0,    21,    24,     0,     0,    27,    28,
      29,     0,     0,     0,     0,    95,     0,     0,    66,     0,
      87,     0,   157,   155,     0,     0,     0,   210,     0,   215,
       0,   209,   205,     7,   109,     0,   118,     0,   116,     8,
     100,    58,     0,     0,     0,    87,    67,     0,     0,   111,
       0,   104,     0,     0,   171,     0,     0,     0,     0,   153,
     148,   147,   151,   150,   145,   146,   141,   142,   143,   144,
     149,   134,   135,   136,   137,   138,   139,   140,   152,   164,
       0,     0,     0,   166,   175,   176,   129,     0,   123,   122,
      50,     7,    38,     0,    37,     0,    52,    40,    41,     0,
       0,    43,     0,    45,     0,     0,     0,     0,   172,     0,
      87,     0,     0,    87,    86,     0,     0,    82,     0,     0,
      87,   158,     0,   156,     0,     0,     0,    66,   211,     0,
       0,   119,   110,     0,     0,   102,   107,   101,   105,    59,
      68,     0,    63,    65,    71,    70,   112,   108,   106,   191,
       0,   187,   183,   184,     0,     0,   179,     0,   177,   130,
       0,     0,     0,     0,     0,     0,     0,    23,     0,    22,
       0,    26,    25,     0,    68,     0,    87,    97,   159,    83,
       0,    87,    87,    88,    60,    73,    77,     0,    80,     0,
      78,     0,     0,     0,   196,   198,     0,     0,   212,     0,
     216,   117,   120,   121,   103,    93,    69,     0,   189,     0,
     163,   178,     0,    36,     0,    33,    39,    31,    42,    34,
      44,    46,     0,    30,    69,     0,     0,   161,    84,    85,
      66,    89,     0,    75,     0,    79,   160,     0,    61,     0,
       0,     0,   213,    94,   192,     0,   188,   180,     0,     0,
       0,    98,     0,    91,     0,    74,     0,    81,   162,   197,
     199,     0,     0,   214,   190,     0,    32,    35,    99,     0,
      90,    76,   201,     0,   181,    92,   200
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -312,  -312,   372,  -312,     8,   396,  -312,   417,   144,  -106,
     145,   298,   142,  -155,    -4,    -3,  -312,   440,   375,    -6,
     439,   -25,  -312,  -312,   268,  -311,  -250,   389,   299,  -312,
    -312,  -167,  -312,  -233,  -312,  -312,   361,  -312,  -176,   370,
    -312,    -5,  -312,   -98,   328,  -312,  -312,   -22,   -82,  -189,
    -312,  -312,  -312,    76
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    58,    52,    37,    38,    39,   242,   243,
     248,   249,   253,   254,    64,    65,    66,    40,    41,    42,
      43,   196,   173,   104,   105,   106,   293,    44,   294,   339,
     340,   265,   266,   267,    45,    46,   192,    47,   186,   187,
      60,    48,   138,   139,   140,   207,   208,    49,    50,    51,
     180,   349,   412,   181
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      53,    54,    55,    56,    87,    57,   179,    61,   257,   281,
      36,    97,    69,    70,    75,    81,   101,    62,     4,   338,
      59,    73,   381,    67,   151,    92,    74,    93,   291,    79,
     329,    83,   364,   -57,   146,     4,   168,    62,   241,   151,
     -47,    71,   235,   347,     9,    95,    36,     9,   -47,   260,
     383,   250,   161,   167,    97,   147,   255,   148,   384,   172,
     251,     9,   262,   152,     9,     3,    36,    78,   263,   403,
     251,    68,    94,   328,    95,   260,    96,   365,   157,   167,
     174,   175,   178,    95,   252,   167,    63,   382,   264,   188,
      63,   193,   162,   325,   256,   311,    36,   312,   378,   379,
     163,   355,   188,   342,   203,   204,    63,   352,   353,   209,
      72,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   232,   233,   406,   407,   308,   193,   290,   -49,
     236,   377,   -49,   238,   263,   239,   244,   324,   273,   245,
     246,   244,   386,   263,   420,   341,   244,   205,   206,   376,
      79,   263,   375,   261,   264,    79,   268,   393,   263,   425,
      79,    95,    36,   264,    76,   155,   269,    79,   270,    85,
     188,   264,   194,   326,   153,   285,     4,   154,   264,   327,
       5,     6,   107,   295,   108,   170,   109,   350,   408,   171,
     299,   301,    77,    82,   282,   296,   283,   283,     7,     8,
     284,   284,     9,    86,    88,    10,   330,   400,   331,   313,
       4,   314,   332,    11,   143,    12,   305,   237,    89,    90,
     194,    13,   309,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,   102,    25,    26,    27,    28,    29,
      30,    31,   344,   345,    32,    33,    34,    35,   195,    12,
     333,   103,   167,   356,   295,   331,    94,   343,    95,   332,
     142,   346,    19,   348,    79,   351,    23,   318,   188,   188,
     319,   374,   354,   331,   387,    31,   331,   332,    32,    33,
     332,   402,   260,   331,   141,   358,   167,   332,   318,   360,
     318,   322,   362,   417,   363,   133,   244,     4,   156,   158,
     179,     5,     6,   129,   130,   131,   132,   133,    79,   315,
      79,   312,   127,   128,   129,   130,   131,   132,   133,     7,
       8,   159,   295,     9,    11,   160,    10,   137,   320,   323,
     312,   314,   391,   166,   269,   335,    12,   168,   189,   336,
     169,    95,   394,   337,   396,   176,    94,   145,    95,    19,
      96,   194,   197,    23,   198,   201,   137,   409,   240,   247,
     191,   272,    31,   276,   277,    32,    33,   278,   279,   295,
     295,     4,   280,   286,   289,     5,     6,   297,   302,   422,
     414,   303,   310,   415,   151,   316,   264,   317,   321,   295,
     367,   426,   369,     7,     8,   370,   251,     9,   373,   372,
      10,   380,   385,   399,   295,   404,   389,   392,    11,   258,
      12,   401,   410,   405,   416,   418,    13,   419,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,   421,
      25,    26,    27,    28,    29,    30,    31,   423,   164,    32,
      33,    34,    35,     4,   144,   100,   366,     5,     6,   368,
     371,    80,    84,   177,   259,   292,   165,   234,   413,     0,
     202,   271,     0,     0,     0,     7,     8,   200,     0,     9,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
      11,     0,    12,     0,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,    29,    30,    31,     0,
       0,    32,    33,    34,    35,     4,     0,     0,     0,     5,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     0,
       0,     9,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,    11,     0,    12,     0,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,    29,    30,
      31,     0,     0,    32,    33,    98,    99,     4,     0,   183,
       0,     5,     6,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     7,     8,     9,     0,     9,    10,     0,    10,     0,
       0,     0,     0,     0,    11,     0,    12,     0,    12,   184,
       0,     0,    13,     0,     0,   185,     0,     0,     0,    19,
       0,    19,     0,    23,    24,    23,    25,    26,    27,    28,
      29,    30,    31,     0,    31,    32,    33,    32,    33,     4,
       0,   183,     0,     5,     6,     5,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     7,     8,     7,     8,     9,     0,     9,    10,     0,
      10,     0,   190,     0,     0,     0,     0,     0,    12,     0,
      12,   199,     0,   191,     0,     0,     0,   185,     0,     0,
       0,    19,     0,    19,     4,    23,     4,    23,     5,     6,
       5,     6,     0,     0,    31,     0,    31,    32,    33,    32,
      33,     0,     0,     0,     0,     0,     7,     8,     7,     8,
       9,     0,     9,    10,     0,    10,     0,     0,     0,   190,
       0,     0,     0,    12,     0,    12,     0,     0,     0,    91,
       0,     0,     0,     0,     0,     0,    19,     0,    19,     0,
      23,     0,    23,     0,     0,     0,     0,     0,     0,    31,
       0,    31,    32,    33,    32,    33,     4,     0,     4,     0,
       5,     6,     5,     6,     0,     0,   231,     0,   111,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     7,     8,
       7,     8,     9,     0,     9,    10,     0,    10,   127,   128,
     129,   130,   131,   132,   133,    12,     0,    12,     0,     0,
       0,     0,   191,   137,     0,     0,     0,     0,    19,     0,
      19,     4,    23,     4,    23,     5,     6,     5,     6,     0,
       0,    31,   300,    31,    32,    33,    32,    33,     0,     0,
       0,     0,     0,     7,     8,     7,     8,     9,     0,     9,
      10,     0,    10,     0,     0,     0,     0,     0,   269,     0,
      12,     0,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,    19,     4,    23,   183,    23,
       5,     6,     5,     6,     0,     0,    31,     0,    31,    32,
      33,    32,    33,     0,     0,     0,     0,     0,     7,     8,
       7,     8,     9,     0,     9,    10,     0,    10,     0,     0,
       0,     0,     0,     0,     0,    12,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
      19,     0,    23,     0,    23,     0,     0,     0,     0,     0,
       0,    31,     0,    31,    32,    33,    32,    33,   110,   111,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,   110,   111,   112,   113,   114,
     115,     0,   182,     0,     0,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,     0,   135,   136,   287,     0,     0,
       0,   137,   110,   111,   112,   113,   114,   115,   288,     0,
       0,     0,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,     0,   135,   136,   287,     0,     0,     0,   137,   110,
     111,   112,   113,   114,   115,   298,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   135,
     136,     0,     0,     0,     0,   137,   110,   111,   112,   113,
     114,   115,   334,     0,     0,     0,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,     0,   135,   136,     0,     0,
       0,     0,   137,   110,   111,   112,   113,   114,   115,   388,
       0,     0,     0,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,     0,   135,   136,     0,     0,     0,     0,   137,
       0,     0,   149,     0,   150,   110,   111,   112,   113,   114,
     115,     0,     0,     0,     0,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,     0,   135,   136,     0,     0,     0,
       0,   137,   306,     0,     0,     0,   307,   110,   111,   112,
     113,   114,   115,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,     0,   135,   136,     0,
       0,     0,     0,   137,   397,     0,     0,     0,   398,   110,
     111,   112,   113,   114,   115,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   135,
     136,     0,     0,     0,     0,   137,     0,     0,   274,   110,
     111,   112,   113,   114,   115,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   135,
     136,     0,     0,     0,     0,   137,     0,     0,   275,   110,
     111,   112,   113,   114,   115,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   135,
     136,     0,     0,     0,     0,   137,     0,     0,   390,   110,
     111,   112,   113,   114,   115,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   135,
     136,     0,     0,     0,     0,   137,     0,     0,   411,   110,
     111,   112,   113,   114,   115,     0,     0,     0,     0,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   135,
     136,     0,     0,     0,     0,   137,   361,   110,   111,   112,
     113,   114,   115,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,     0,   135,   136,     0,
       0,     0,     0,   137,   424,   110,   111,   112,   113,   114,
     115,     0,     0,     0,     0,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,     0,   135,   136,   304,     0,     0,
       0,   137,   110,   111,   112,   113,   114,   115,     0,     0,
       0,     0,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,     0,   135,   136,     0,     0,     0,   110,   137,   112,
     113,   114,   115,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,     0,   135,   136,     0,
       0,     0,   357,   137,   110,     0,   112,   113,   114,   115,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,   135,   136,     0,     0,     0,   359,
     137,   110,     0,   112,   113,   114,   115,     0,     0,     0,
       0,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
       0,   135,   136,     0,     0,     0,   395,   137,   110,     0,
     112,   113,   114,   115,     0,     0,     0,     0,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,     0,   135,   136,
       0,     0,     0,     0,   137,   111,   112,   113,   114,   115,
       0,     0,     0,     0,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,     0,   136,     0,     0,     0,     0,
     137,   111,   112,   113,   114,   115,     0,     0,     0,     0,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   111,
     112,   113,   114,   115,     0,     0,   137,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   111,   112,   113,
     114,   115,     0,     0,   137,     0,     0,     0,     0,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   111,   112,   113,   114,   115,
       0,     0,   137,     0,     0,     0,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   111,   112,   113,   114,   115,     0,     0,
     137,     0,     0,     0,     0,     0,     0,     0,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   111,   112,   113,   114,   115,     0,     0,   137,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   111,
     112,   113,   114,   115,     0,     0,   137,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   126,   127,
     128,   129,   130,   131,   132,   133,     0,     0,     0,     0,
       0,     0,     0,     0,   137
};

static const yytype_int16 yycheck[] =
{
       5,     6,     7,     8,    26,    10,    88,    12,   163,   185,
       2,    36,    16,    16,    18,    21,    38,     3,     3,   269,
      12,     3,   333,    15,     3,    30,    18,    32,   195,    21,
     263,    23,     3,     3,    59,     3,     3,     3,     3,     3,
      42,     3,   140,    36,    29,    44,    38,    29,    50,    42,
      41,   157,    51,    46,    79,    43,   162,    45,    49,    84,
       3,    29,    43,    42,    29,     0,    58,    52,    49,   380,
       3,    57,    42,   262,    44,    42,    46,    48,    42,    46,
      85,    86,    88,    44,    27,    46,    72,   337,    69,    94,
      72,    96,    42,   260,    27,    43,    88,    45,   331,   332,
      50,   290,   107,   270,   109,   110,    72,   283,   284,   114,
      72,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   384,   385,   234,   142,    43,    45,
     145,   330,    48,   148,    49,   150,   152,    43,   173,   153,
     153,   157,   341,    49,   404,    43,   162,    10,    11,   326,
     152,    49,    43,   169,    69,   157,   171,   356,    49,   419,
     162,    44,   164,    69,    40,    48,    40,   169,    42,    42,
     185,    69,    46,    42,    45,   190,     3,    48,    69,    48,
       7,     8,    42,   198,    44,    42,    46,   279,   387,    46,
     205,   206,     3,     3,    43,    43,    45,    45,    25,    26,
      49,    49,    29,    42,    42,    32,    43,   372,    45,    43,
       3,    45,    49,    40,    41,    42,   231,    43,    48,    48,
      46,    48,   237,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    48,    62,    63,    64,    65,    66,
      67,    68,   274,   275,    71,    72,    73,    74,    42,    42,
     266,     3,    46,    43,   269,    45,    42,   272,    44,    49,
      46,   276,    55,   278,   266,   280,    59,    45,   283,   284,
      48,    43,   287,    45,    43,    68,    45,    49,    71,    72,
      49,    43,    42,    45,    48,   300,    46,    49,    45,   304,
      45,    48,   307,    48,   310,    31,   312,     3,    42,    48,
     392,     7,     8,    27,    28,    29,    30,    31,   310,    43,
     312,    45,    25,    26,    27,    28,    29,    30,    31,    25,
      26,    48,   337,    29,    40,    48,    32,    40,    43,    43,
      45,    45,   347,    42,    40,    41,    42,     3,     3,    45,
      34,    44,   357,    49,   359,    62,    42,    43,    44,    55,
      46,    46,    45,    59,    34,     3,    40,   389,     3,     3,
      47,    46,    68,    42,     3,    71,    72,    48,    45,   384,
     385,     3,    34,    47,    47,     7,     8,    47,     5,   411,
     395,     5,    34,   398,     3,     3,    69,    48,    48,   404,
      48,   423,    48,    25,    26,     3,     3,    29,    48,    50,
      32,     3,    45,     3,   419,    34,    61,    48,    40,    41,
      42,    48,    48,    41,    48,    48,    48,    34,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    41,
      62,    63,    64,    65,    66,    67,    68,    43,    76,    71,
      72,    73,    74,     3,    58,    38,   312,     7,     8,   314,
     318,    21,    23,    88,   166,   197,    77,   139,   392,    -1,
     109,   172,    -1,    -1,    -1,    25,    26,   107,    -1,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    66,    67,    68,    -1,
      -1,    71,    72,    73,    74,     3,    -1,    -1,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
      68,    -1,    -1,    71,    72,    73,    74,     3,    -1,     3,
      -1,     7,     8,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    25,    26,    29,    -1,    29,    32,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    42,    -1,    42,    43,
      -1,    -1,    48,    -1,    -1,    49,    -1,    -1,    -1,    55,
      -1,    55,    -1,    59,    60,    59,    62,    63,    64,    65,
      66,    67,    68,    -1,    68,    71,    72,    71,    72,     3,
      -1,     3,    -1,     7,     8,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    26,    25,    26,    29,    -1,    29,    32,    -1,
      32,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      42,    43,    -1,    47,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    55,    -1,    55,     3,    59,     3,    59,     7,     8,
       7,     8,    -1,    -1,    68,    -1,    68,    71,    72,    71,
      72,    -1,    -1,    -1,    -1,    -1,    25,    26,    25,    26,
      29,    -1,    29,    32,    -1,    32,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    42,    -1,    42,    -1,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    55,    -1,
      59,    -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    68,    71,    72,    71,    72,     3,    -1,     3,    -1,
       7,     8,     7,     8,    -1,    -1,    13,    -1,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    25,    26,
      25,    26,    29,    -1,    29,    32,    -1,    32,    25,    26,
      27,    28,    29,    30,    31,    42,    -1,    42,    -1,    -1,
      -1,    -1,    47,    40,    -1,    -1,    -1,    -1,    55,    -1,
      55,     3,    59,     3,    59,     7,     8,     7,     8,    -1,
      -1,    68,    12,    68,    71,    72,    71,    72,    -1,    -1,
      -1,    -1,    -1,    25,    26,    25,    26,    29,    -1,    29,
      32,    -1,    32,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      42,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    55,     3,    59,     3,    59,
       7,     8,     7,     8,    -1,    -1,    68,    -1,    68,    71,
      72,    71,    72,    -1,    -1,    -1,    -1,    -1,    25,    26,
      25,    26,    29,    -1,    29,    32,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    42,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      55,    -1,    59,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    68,    71,    72,    71,    72,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,     4,     5,     6,     7,     8,
       9,    -1,    48,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      -1,    40,     4,     5,     6,     7,     8,     9,    47,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    36,    -1,    -1,    -1,    40,     4,
       5,     6,     7,     8,     9,    47,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,     4,     5,     6,     7,
       8,     9,    47,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,     4,     5,     6,     7,     8,     9,    47,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    43,    -1,    45,     4,     5,     6,     7,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,
      -1,    40,    41,    -1,    -1,    -1,    45,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    -1,    -1,    40,    41,    -1,    -1,    -1,    45,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    43,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    41,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    -1,    -1,    40,    41,     4,     5,     6,     7,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    36,    -1,    -1,
      -1,    40,     4,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    -1,    -1,    -1,     4,    40,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    -1,    39,    40,     4,    -1,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,    39,
      40,     4,    -1,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    39,    40,     4,    -1,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      40,     5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,     5,
       6,     7,     8,     9,    -1,    -1,    40,    -1,    -1,    -1,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,     5,     6,     7,
       8,     9,    -1,    -1,    40,    -1,    -1,    -1,    -1,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     5,     6,     7,     8,     9,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,     5,     6,     7,     8,     9,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     5,     6,     7,     8,     9,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    25,    26,    27,    28,    29,    30,    31,    32,     5,
       6,     7,     8,     9,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    78,    79,     0,     3,     7,     8,    25,    26,    29,
      32,    40,    42,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    62,    63,    64,    65,    66,
      67,    68,    71,    72,    73,    74,    81,    82,    83,    84,
      94,    95,    96,    97,   104,   111,   112,   114,   118,   124,
     125,   126,    81,   118,   118,   118,   118,   118,    80,    81,
     117,   118,     3,    72,    91,    92,    93,    81,    57,    91,
      92,     3,    72,     3,    81,    91,    40,     3,    52,    81,
      94,    96,     3,    81,    97,    42,    42,   124,    42,    48,
      48,    48,   118,   118,    42,    44,    46,    98,    73,    74,
      84,   124,    48,     3,   100,   101,   102,    42,    44,    46,
       4,     5,     6,     7,     8,     9,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    34,    35,    40,   119,   120,
     121,    48,    46,    41,    82,    43,    98,    43,    45,    43,
      45,     3,    42,    45,    48,    48,    42,    42,    48,    48,
      48,    51,    42,    50,    79,   104,    42,    46,     3,    34,
      42,    46,    98,    99,   118,   118,    62,    95,    96,   125,
     127,   130,    48,     3,    43,    49,   115,   116,   118,     3,
      36,    47,   113,   118,    46,    42,    98,    45,    34,    43,
     116,     3,   113,   118,   118,    10,    11,   122,   123,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,    13,   118,   118,   121,   120,   118,    43,   118,   118,
       3,     3,    85,    86,    96,    91,    92,     3,    87,    88,
      86,     3,    27,    89,    90,    86,    27,    90,    41,    88,
      42,    96,    43,    49,    69,   108,   109,   110,   118,    40,
      42,   105,    46,    98,    43,    43,    42,     3,    48,    45,
      34,   115,    43,    45,    49,   118,    47,    36,    47,    47,
      43,   108,   101,   103,   105,   118,    43,    47,    47,   118,
      12,   118,     5,     5,    36,   118,    41,    45,   120,   118,
      34,    43,    45,    43,    45,    43,     3,    48,    45,    48,
      43,    48,    48,    43,    43,   108,    42,    48,   126,   110,
      43,    45,    49,    96,    47,    41,    45,    49,   103,   106,
     107,    43,   108,   118,   124,   124,   118,    36,   118,   128,
     125,   118,   115,   115,   118,   126,    43,    39,   118,    39,
     118,    41,   118,    96,     3,    48,    85,    48,    87,    48,
       3,    89,    50,    48,    43,    43,   108,   126,   110,   110,
       3,   102,   103,    41,    49,    45,   126,    43,    47,    61,
      43,   118,    48,   126,   118,    39,   118,    41,    45,     3,
      90,    48,    43,   102,    34,    41,   103,   103,   126,   124,
      48,    43,   129,   130,   118,   118,    48,    48,    48,    34,
     103,    41,   124,    43,    41,   103,   124
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    78,    79,    79,    80,    80,    81,    81,    81,
      82,    82,    82,    82,    83,    83,    83,    83,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    85,    85,    86,    86,
      87,    88,    88,    89,    89,    90,    90,    91,    91,    92,
      92,    93,    93,    94,    95,    96,    96,    97,    98,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   102,   102,
     103,   103,   104,   105,   105,   105,   105,   106,   106,   106,
     107,   107,   108,   108,   108,   108,   109,   109,   110,   110,
     110,   110,   110,   111,   111,   112,   112,   112,   112,   112,
     113,   113,   113,   113,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   115,   115,   116,   116,
     116,   116,   117,   117,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   119,   119,   119,   119,   119,   120,   120,
     120,   120,   121,   121,   121,   121,   121,   122,   122,   122,
     122,   123,   123,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   125,   126,   127,   127,
     127,   128,   128,   129,   129,   130,   130
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     0,     2,     1,     3,     1,
       1,     1,     2,     2,     1,     1,     2,     2,     1,     1,
       1,     3,     5,     5,     3,     5,     5,     3,     3,     3,
       6,     6,     8,     6,     6,     8,     3,     1,     1,     3,
       1,     1,     3,     1,     3,     1,     3,     1,     1,     1,
       3,     1,     3,     2,     2,     1,     2,     1,     2,     3,
       3,     4,     1,     3,     1,     3,     1,     2,     3,     4,
       1,     1,     3,     2,     4,     3,     5,     1,     1,     2,
       1,     3,     1,     2,     3,     3,     1,     0,     2,     3,
       5,     4,     6,     5,     6,     3,     2,     5,     7,     8,
       1,     2,     2,     3,     3,     4,     4,     4,     4,     3,
       4,     3,     4,     3,     3,     1,     1,     3,     1,     2,
       3,     3,     3,     3,     1,     1,     1,     1,     2,     4,
       5,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     4,     3,     4,     5,
       6,     6,     7,     5,     3,     3,     3,     2,     2,     2,
       2,     3,     4,     1,     1,     2,     2,     3,     4,     3,
       5,     7,     1,     3,     3,     1,     1,     2,     4,     3,
       5,     2,     4,     1,     1,     2,     5,     7,     5,     7,
       9,     8,     2,     2,     2,     3,     1,     1,     0,     1,
       1,     0,     1,     0,     1,     1,     3
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
#line 1851 "camp.tab.cc"
    break;

  case 3: /* fileblock: %empty  */
#line 199 "camp.y"
                   { (yyval.b) = new file(lexerPos(), false); }
#line 1857 "camp.tab.cc"
    break;

  case 4: /* fileblock: fileblock runnable  */
#line 201 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1863 "camp.tab.cc"
    break;

  case 5: /* bareblock: %empty  */
#line 205 "camp.y"
                   { (yyval.b) = new block(lexerPos(), true); }
#line 1869 "camp.tab.cc"
    break;

  case 6: /* bareblock: bareblock runnable  */
#line 207 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1875 "camp.tab.cc"
    break;

  case 7: /* name: ID  */
#line 211 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 1881 "camp.tab.cc"
    break;

  case 8: /* name: name '.' ID  */
#line 212 "camp.y"
                   { (yyval.n) = new qualifiedName((yyvsp[-1].pos), (yyvsp[-2].n), (yyvsp[0].ps).sym); }
#line 1887 "camp.tab.cc"
    break;

  case 9: /* name: '%'  */
#line 213 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos,
                                  symbol::trans("operator answer")); }
#line 1894 "camp.tab.cc"
    break;

  case 10: /* runnable: dec  */
#line 218 "camp.y"
                   { (yyval.run) = (yyvsp[0].d); }
#line 1900 "camp.tab.cc"
    break;

  case 11: /* runnable: stm  */
#line 219 "camp.y"
                   { (yyval.run) = (yyvsp[0].s); }
#line 1906 "camp.tab.cc"
    break;

  case 12: /* runnable: modifiers dec  */
#line 221 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].d)); }
#line 1912 "camp.tab.cc"
    break;

  case 13: /* runnable: modifiers stm  */
#line 223 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].s)); }
#line 1918 "camp.tab.cc"
    break;

  case 14: /* modifiers: MODIFIER  */
#line 227 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].mod).pos); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1924 "camp.tab.cc"
    break;

  case 15: /* modifiers: PERM  */
#line 228 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].perm).pos); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1930 "camp.tab.cc"
    break;

  case 16: /* modifiers: modifiers MODIFIER  */
#line 230 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1936 "camp.tab.cc"
    break;

  case 17: /* modifiers: modifiers PERM  */
#line 232 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1942 "camp.tab.cc"
    break;

  case 18: /* dec: vardec  */
#line 236 "camp.y"
                   { (yyval.d) = (yyvsp[0].vd); }
#line 1948 "camp.tab.cc"
    break;

  case 19: /* dec: fundec  */
#line 237 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1954 "camp.tab.cc"
    break;

  case 20: /* dec: typedec  */
#line 238 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1960 "camp.tab.cc"
    break;

  case 21: /* dec: ACCESS stridpairlist ';'  */
#line 240 "camp.y"
                   { (yyval.d) = new accessdec((yyvsp[-2].pos), (yyvsp[-1].ipl)); }
#line 1966 "camp.tab.cc"
    break;

  case 22: /* dec: FROM name UNRAVEL idpairlist ';'  */
#line 242 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), (yyvsp[-1].ipl)); }
#line 1972 "camp.tab.cc"
    break;

  case 23: /* dec: FROM name UNRAVEL '*' ';'  */
#line 244 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), WILDCARD); }
#line 1978 "camp.tab.cc"
    break;

  case 24: /* dec: UNRAVEL name ';'  */
#line 245 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-2].pos), (yyvsp[-1].n), WILDCARD); }
#line 1984 "camp.tab.cc"
    break;

  case 25: /* dec: FROM strid ACCESS idpairlist ';'  */
#line 247 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, (yyvsp[-1].ipl)); }
#line 1990 "camp.tab.cc"
    break;

  case 26: /* dec: FROM strid ACCESS '*' ';'  */
#line 249 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, WILDCARD); }
#line 1996 "camp.tab.cc"
    break;

  case 27: /* dec: IMPORT stridpair ';'  */
#line 251 "camp.y"
                   { (yyval.d) = new importdec((yyvsp[-2].pos), (yyvsp[-1].ip)); }
#line 2002 "camp.tab.cc"
    break;

  case 28: /* dec: INCLUDE ID ';'  */
#line 252 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].ps).sym); }
#line 2008 "camp.tab.cc"
    break;

  case 29: /* dec: INCLUDE STRING ';'  */
#line 254 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].stre)->getString()); }
#line 2014 "camp.tab.cc"
    break;

  case 30: /* dec: TYPEDEF IMPORT '(' typeparamlist ')' ';'  */
#line 258 "camp.y"
                   { (yyval.d) = new receiveTypedefDec((yyvsp[-5].pos), (yyvsp[-2].tps)); }
#line 2020 "camp.tab.cc"
    break;

  case 31: /* dec: IMPORT TYPEDEF '(' typeparamlist ')' ';'  */
#line 260 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[-5].pos),
                     "Expected 'typedef import(<types>);'");
                   }
#line 2028 "camp.tab.cc"
    break;

  case 32: /* dec: ACCESS strid '(' decdeclist ')' ID ID ';'  */
#line 265 "camp.y"
                   { (yyval.d) = new templateAccessDec(
                        (yyvsp[-7].pos), (yyvsp[-6].ps).sym, (yyvsp[-4].fls), (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym, (yyvsp[-2].ps).pos
                      ); }
#line 2036 "camp.tab.cc"
    break;

  case 33: /* dec: ACCESS strid '(' decdeclist ')' ';'  */
#line 269 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[0].pos), "expected 'as'"); }
#line 2042 "camp.tab.cc"
    break;

  case 34: /* dec: IMPORT strid '(' decdeclist ')' ';'  */
#line 271 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[-5].pos),
                        "Parametrized imports disallowed to reduce naming "
                        "conflicts. Try "
                        "'access <module>(<type parameters>) as <newname>;'."
                     ); }
#line 2052 "camp.tab.cc"
    break;

  case 35: /* dec: FROM strid '(' decdeclist ')' ACCESS idpairlist ';'  */
#line 277 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-7].pos), (yyvsp[-6].ps).sym, (yyvsp[-1].ipl), (yyvsp[-4].fls)); }
#line 2058 "camp.tab.cc"
    break;

  case 36: /* decdec: ID ASSIGN type  */
#line 283 "camp.y"
                   { (yyval.fl) = new formal(
                        (yyvsp[-2].ps).pos, (yyvsp[0].t), new decidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym)
                      ); }
#line 2066 "camp.tab.cc"
    break;

  case 37: /* decdec: type  */
#line 286 "camp.y"
       { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), nullptr); }
#line 2072 "camp.tab.cc"
    break;

  case 38: /* decdeclist: decdec  */
#line 291 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2078 "camp.tab.cc"
    break;

  case 39: /* decdeclist: decdeclist ',' decdec  */
#line 293 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2084 "camp.tab.cc"
    break;

  case 40: /* typeparam: ID  */
#line 297 "camp.y"
       { (yyval.tp) = new typeParam((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2090 "camp.tab.cc"
    break;

  case 41: /* typeparamlist: typeparam  */
#line 302 "camp.y"
                   { (yyval.tps) = new typeParamList((yyvsp[0].tp)->getPos()); (yyval.tps)->add((yyvsp[0].tp)); }
#line 2096 "camp.tab.cc"
    break;

  case 42: /* typeparamlist: typeparamlist ',' typeparam  */
#line 304 "camp.y"
                   { (yyval.tps) = (yyvsp[-2].tps); (yyval.tps)->add((yyvsp[0].tp)); }
#line 2102 "camp.tab.cc"
    break;

  case 43: /* idpair: ID  */
#line 308 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2108 "camp.tab.cc"
    break;

  case 44: /* idpair: ID ID ID  */
#line 310 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2114 "camp.tab.cc"
    break;

  case 45: /* idpairlist: idpair  */
#line 314 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2120 "camp.tab.cc"
    break;

  case 46: /* idpairlist: idpairlist ',' idpair  */
#line 316 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2126 "camp.tab.cc"
    break;

  case 47: /* strid: ID  */
#line 320 "camp.y"
                   { (yyval.ps) = (yyvsp[0].ps); }
#line 2132 "camp.tab.cc"
    break;

  case 48: /* strid: STRING  */
#line 321 "camp.y"
                   { (yyval.ps).pos = (yyvsp[0].stre)->getPos();
                     (yyval.ps).sym = symbol::literalTrans((yyvsp[0].stre)->getString()); }
#line 2139 "camp.tab.cc"
    break;

  case 49: /* stridpair: ID  */
#line 326 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2145 "camp.tab.cc"
    break;

  case 50: /* stridpair: strid ID ID  */
#line 328 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2151 "camp.tab.cc"
    break;

  case 51: /* stridpairlist: stridpair  */
#line 332 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2157 "camp.tab.cc"
    break;

  case 52: /* stridpairlist: stridpairlist ',' stridpair  */
#line 334 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2163 "camp.tab.cc"
    break;

  case 53: /* vardec: barevardec ';'  */
#line 338 "camp.y"
                   { (yyval.vd) = (yyvsp[-1].vd); }
#line 2169 "camp.tab.cc"
    break;

  case 54: /* barevardec: type decidlist  */
#line 342 "camp.y"
                   { (yyval.vd) = new vardec((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dil)); }
#line 2175 "camp.tab.cc"
    break;

  case 55: /* type: celltype  */
#line 346 "camp.y"
                   { (yyval.t) = (yyvsp[0].t); }
#line 2181 "camp.tab.cc"
    break;

  case 56: /* type: name dims  */
#line 347 "camp.y"
                   { (yyval.t) = new arrayTy((yyvsp[-1].n), (yyvsp[0].dim)); }
#line 2187 "camp.tab.cc"
    break;

  case 57: /* celltype: name  */
#line 351 "camp.y"
                   { (yyval.t) = new nameTy((yyvsp[0].n)); }
#line 2193 "camp.tab.cc"
    break;

  case 58: /* dims: '[' ']'  */
#line 355 "camp.y"
                   { (yyval.dim) = new dimensions((yyvsp[-1].pos)); }
#line 2199 "camp.tab.cc"
    break;

  case 59: /* dims: dims '[' ']'  */
#line 356 "camp.y"
                   { (yyval.dim) = (yyvsp[-2].dim); (yyval.dim)->increase(); }
#line 2205 "camp.tab.cc"
    break;

  case 60: /* dimexps: '[' exp ']'  */
#line 360 "camp.y"
                   { (yyval.elist) = new explist((yyvsp[-2].pos)); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2211 "camp.tab.cc"
    break;

  case 61: /* dimexps: dimexps '[' exp ']'  */
#line 362 "camp.y"
                   { (yyval.elist) = (yyvsp[-3].elist); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2217 "camp.tab.cc"
    break;

  case 62: /* decidlist: decid  */
#line 366 "camp.y"
                   { (yyval.dil) = new decidlist((yyvsp[0].di)->getPos()); (yyval.dil)->add((yyvsp[0].di)); }
#line 2223 "camp.tab.cc"
    break;

  case 63: /* decidlist: decidlist ',' decid  */
#line 368 "camp.y"
                   { (yyval.dil) = (yyvsp[-2].dil); (yyval.dil)->add((yyvsp[0].di)); }
#line 2229 "camp.tab.cc"
    break;

  case 64: /* decid: decidstart  */
#line 372 "camp.y"
                   { (yyval.di) = new decid((yyvsp[0].dis)->getPos(), (yyvsp[0].dis)); }
#line 2235 "camp.tab.cc"
    break;

  case 65: /* decid: decidstart ASSIGN varinit  */
#line 374 "camp.y"
                   { (yyval.di) = new decid((yyvsp[-2].dis)->getPos(), (yyvsp[-2].dis), (yyvsp[0].vi)); }
#line 2241 "camp.tab.cc"
    break;

  case 66: /* decidstart: ID  */
#line 378 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2247 "camp.tab.cc"
    break;

  case 67: /* decidstart: ID dims  */
#line 379 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym, (yyvsp[0].dim)); }
#line 2253 "camp.tab.cc"
    break;

  case 68: /* decidstart: ID '(' ')'  */
#line 380 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, 0,
                                            new formals((yyvsp[-1].pos))); }
#line 2260 "camp.tab.cc"
    break;

  case 69: /* decidstart: ID '(' formals ')'  */
#line 383 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym, 0, (yyvsp[-1].fls)); }
#line 2266 "camp.tab.cc"
    break;

  case 70: /* varinit: exp  */
#line 387 "camp.y"
                   { (yyval.vi) = (yyvsp[0].e); }
#line 2272 "camp.tab.cc"
    break;

  case 71: /* varinit: arrayinit  */
#line 388 "camp.y"
                   { (yyval.vi) = (yyvsp[0].ai); }
#line 2278 "camp.tab.cc"
    break;

  case 72: /* block: '{' bareblock '}'  */
#line 393 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); }
#line 2284 "camp.tab.cc"
    break;

  case 73: /* arrayinit: '{' '}'  */
#line 397 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-1].pos)); }
#line 2290 "camp.tab.cc"
    break;

  case 74: /* arrayinit: '{' ELLIPSIS varinit '}'  */
#line 399 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-3].pos)); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2296 "camp.tab.cc"
    break;

  case 75: /* arrayinit: '{' basearrayinit '}'  */
#line 401 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2302 "camp.tab.cc"
    break;

  case 76: /* arrayinit: '{' basearrayinit ELLIPSIS varinit '}'  */
#line 403 "camp.y"
                   { (yyval.ai) = (yyvsp[-3].ai); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2308 "camp.tab.cc"
    break;

  case 77: /* basearrayinit: ','  */
#line 407 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].pos)); }
#line 2314 "camp.tab.cc"
    break;

  case 78: /* basearrayinit: varinits  */
#line 408 "camp.y"
                   { (yyval.ai) = (yyvsp[0].ai); }
#line 2320 "camp.tab.cc"
    break;

  case 79: /* basearrayinit: varinits ','  */
#line 409 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2326 "camp.tab.cc"
    break;

  case 80: /* varinits: varinit  */
#line 413 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].vi)->getPos());
		     (yyval.ai)->add((yyvsp[0].vi));}
#line 2333 "camp.tab.cc"
    break;

  case 81: /* varinits: varinits ',' varinit  */
#line 416 "camp.y"
                   { (yyval.ai) = (yyvsp[-2].ai); (yyval.ai)->add((yyvsp[0].vi)); }
#line 2339 "camp.tab.cc"
    break;

  case 82: /* formals: formal  */
#line 420 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2345 "camp.tab.cc"
    break;

  case 83: /* formals: ELLIPSIS formal  */
#line 421 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[-1].pos)); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2351 "camp.tab.cc"
    break;

  case 84: /* formals: formals ',' formal  */
#line 423 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2357 "camp.tab.cc"
    break;

  case 85: /* formals: formals ELLIPSIS formal  */
#line 425 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2363 "camp.tab.cc"
    break;

  case 86: /* explicitornot: EXPLICIT  */
#line 429 "camp.y"
                   { (yyval.boo) = true; }
#line 2369 "camp.tab.cc"
    break;

  case 87: /* explicitornot: %empty  */
#line 430 "camp.y"
                   { (yyval.boo) = false; }
#line 2375 "camp.tab.cc"
    break;

  case 88: /* formal: explicitornot type  */
#line 435 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), 0, 0, (yyvsp[-1].boo), 0); }
#line 2381 "camp.tab.cc"
    break;

  case 89: /* formal: explicitornot type decidstart  */
#line 437 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dis), 0, (yyvsp[-2].boo), 0); }
#line 2387 "camp.tab.cc"
    break;

  case 90: /* formal: explicitornot type decidstart ASSIGN varinit  */
#line 439 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-3].t)->getPos(), (yyvsp[-3].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-4].boo), 0); }
#line 2393 "camp.tab.cc"
    break;

  case 91: /* formal: explicitornot type ID decidstart  */
#line 442 "camp.y"
                   { bool k = checkKeyword((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-2].t)->getPos(), (yyvsp[-2].t), (yyvsp[0].dis), 0, (yyvsp[-3].boo), k); }
#line 2400 "camp.tab.cc"
    break;

  case 92: /* formal: explicitornot type ID decidstart ASSIGN varinit  */
#line 445 "camp.y"
                   { bool k = checkKeyword((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-5].boo), k); }
#line 2407 "camp.tab.cc"
    break;

  case 93: /* fundec: type ID '(' ')' blockstm  */
#line 451 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-2].pos), (yyvsp[-4].t), (yyvsp[-3].ps).sym, new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2413 "camp.tab.cc"
    break;

  case 94: /* fundec: type ID '(' formals ')' blockstm  */
#line 453 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-3].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2419 "camp.tab.cc"
    break;

  case 95: /* typedec: STRUCT ID block  */
#line 457 "camp.y"
                   { (yyval.d) = new recorddec((yyvsp[-2].pos), (yyvsp[-1].ps).sym, (yyvsp[0].b)); }
#line 2425 "camp.tab.cc"
    break;

  case 96: /* typedec: TYPEDEF vardec  */
#line 458 "camp.y"
                   { (yyval.d) = new typedec((yyvsp[-1].pos), (yyvsp[0].vd)); }
#line 2431 "camp.tab.cc"
    break;

  case 97: /* typedec: USING ID ASSIGN type ';'  */
#line 464 "camp.y"
                   { decidstart *dis = new decidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.d) = new typedec((yyvsp[-4].pos), dis, (yyvsp[-1].t)); }
#line 2438 "camp.tab.cc"
    break;

  case 98: /* typedec: USING ID ASSIGN type '(' ')' ';'  */
#line 467 "camp.y"
                   { decidstart *dis = new fundecidstart((yyvsp[-5].ps).pos, (yyvsp[-5].ps).sym,
                                                         0, new formals((yyvsp[-2].pos)));
                     (yyval.d) = new typedec((yyvsp[-6].pos), dis, (yyvsp[-3].t)); }
#line 2446 "camp.tab.cc"
    break;

  case 99: /* typedec: USING ID ASSIGN type '(' formals ')' ';'  */
#line 471 "camp.y"
                   { decidstart *dis = new fundecidstart((yyvsp[-6].ps).pos, (yyvsp[-6].ps).sym, 0, (yyvsp[-2].fls));
                     (yyval.d) = new typedec((yyvsp[-7].pos), dis, (yyvsp[-4].t)); }
#line 2453 "camp.tab.cc"
    break;

  case 100: /* slice: ':'  */
#line 476 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), 0, 0); }
#line 2459 "camp.tab.cc"
    break;

  case 101: /* slice: exp ':'  */
#line 477 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), (yyvsp[-1].e), 0); }
#line 2465 "camp.tab.cc"
    break;

  case 102: /* slice: ':' exp  */
#line 478 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), 0, (yyvsp[0].e)); }
#line 2471 "camp.tab.cc"
    break;

  case 103: /* slice: exp ':' exp  */
#line 479 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2477 "camp.tab.cc"
    break;

  case 104: /* value: value '.' ID  */
#line 483 "camp.y"
                   { (yyval.e) = new fieldExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].ps).sym); }
#line 2483 "camp.tab.cc"
    break;

  case 105: /* value: name '[' exp ']'  */
#line 484 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].e)); }
#line 2490 "camp.tab.cc"
    break;

  case 106: /* value: value '[' exp ']'  */
#line 486 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e)); }
#line 2496 "camp.tab.cc"
    break;

  case 107: /* value: name '[' slice ']'  */
#line 487 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].slice)); }
#line 2503 "camp.tab.cc"
    break;

  case 108: /* value: value '[' slice ']'  */
#line 489 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].slice)); }
#line 2509 "camp.tab.cc"
    break;

  case 109: /* value: name '(' ')'  */
#line 490 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos),
                                      new nameExp((yyvsp[-2].n)->getPos(), (yyvsp[-2].n)),
                                      new arglist()); }
#line 2517 "camp.tab.cc"
    break;

  case 110: /* value: name '(' arglist ')'  */
#line 494 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos),
                                      new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)),
                                      (yyvsp[-1].alist)); }
#line 2525 "camp.tab.cc"
    break;

  case 111: /* value: value '(' ')'  */
#line 497 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos), (yyvsp[-2].e), new arglist()); }
#line 2531 "camp.tab.cc"
    break;

  case 112: /* value: value '(' arglist ')'  */
#line 499 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].alist)); }
#line 2537 "camp.tab.cc"
    break;

  case 113: /* value: '(' exp ')'  */
#line 501 "camp.y"
                   { (yyval.e) = (yyvsp[-1].e); }
#line 2543 "camp.tab.cc"
    break;

  case 114: /* value: '(' name ')'  */
#line 503 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[-1].n)->getPos(), (yyvsp[-1].n)); }
#line 2549 "camp.tab.cc"
    break;

  case 115: /* value: THIS_TOK  */
#line 504 "camp.y"
                   { (yyval.e) = new thisExp((yyvsp[0].pos)); }
#line 2555 "camp.tab.cc"
    break;

  case 116: /* argument: exp  */
#line 508 "camp.y"
                   { (yyval.arg).name = symbol::nullsym; (yyval.arg).val=(yyvsp[0].e); }
#line 2561 "camp.tab.cc"
    break;

  case 117: /* argument: ID ASSIGN exp  */
#line 509 "camp.y"
                   { (yyval.arg).name = (yyvsp[-2].ps).sym; (yyval.arg).val=(yyvsp[0].e); }
#line 2567 "camp.tab.cc"
    break;

  case 118: /* arglist: argument  */
#line 513 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2573 "camp.tab.cc"
    break;

  case 119: /* arglist: ELLIPSIS argument  */
#line 515 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2579 "camp.tab.cc"
    break;

  case 120: /* arglist: arglist ',' argument  */
#line 517 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2585 "camp.tab.cc"
    break;

  case 121: /* arglist: arglist ELLIPSIS argument  */
#line 519 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2591 "camp.tab.cc"
    break;

  case 122: /* tuple: exp ',' exp  */
#line 524 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[-2].e)); (yyval.alist)->add((yyvsp[0].e)); }
#line 2597 "camp.tab.cc"
    break;

  case 123: /* tuple: tuple ',' exp  */
#line 525 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].e)); }
#line 2603 "camp.tab.cc"
    break;

  case 124: /* exp: name  */
#line 529 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[0].n)->getPos(), (yyvsp[0].n)); }
#line 2609 "camp.tab.cc"
    break;

  case 125: /* exp: value  */
#line 530 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2615 "camp.tab.cc"
    break;

  case 126: /* exp: LIT  */
#line 531 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2621 "camp.tab.cc"
    break;

  case 127: /* exp: STRING  */
#line 532 "camp.y"
                   { (yyval.e) = (yyvsp[0].stre); }
#line 2627 "camp.tab.cc"
    break;

  case 128: /* exp: LIT exp  */
#line 534 "camp.y"
                   { (yyval.e) = new scaleExp((yyvsp[-1].e)->getPos(), (yyvsp[-1].e), (yyvsp[0].e)); }
#line 2633 "camp.tab.cc"
    break;

  case 129: /* exp: '(' name ')' exp  */
#line 536 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-2].n)->getPos(), new nameTy((yyvsp[-2].n)), (yyvsp[0].e)); }
#line 2639 "camp.tab.cc"
    break;

  case 130: /* exp: '(' name dims ')' exp  */
#line 538 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-3].n)->getPos(), new arrayTy((yyvsp[-3].n), (yyvsp[-2].dim)), (yyvsp[0].e)); }
#line 2645 "camp.tab.cc"
    break;

  case 131: /* exp: '+' exp  */
#line 540 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2651 "camp.tab.cc"
    break;

  case 132: /* exp: '-' exp  */
#line 542 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2657 "camp.tab.cc"
    break;

  case 133: /* exp: OPERATOR exp  */
#line 543 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2663 "camp.tab.cc"
    break;

  case 134: /* exp: exp '+' exp  */
#line 544 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2669 "camp.tab.cc"
    break;

  case 135: /* exp: exp '-' exp  */
#line 545 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2675 "camp.tab.cc"
    break;

  case 136: /* exp: exp '*' exp  */
#line 546 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2681 "camp.tab.cc"
    break;

  case 137: /* exp: exp '/' exp  */
#line 547 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2687 "camp.tab.cc"
    break;

  case 138: /* exp: exp '%' exp  */
#line 548 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2693 "camp.tab.cc"
    break;

  case 139: /* exp: exp '#' exp  */
#line 549 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2699 "camp.tab.cc"
    break;

  case 140: /* exp: exp '^' exp  */
#line 550 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2705 "camp.tab.cc"
    break;

  case 141: /* exp: exp LT exp  */
#line 551 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2711 "camp.tab.cc"
    break;

  case 142: /* exp: exp LE exp  */
#line 552 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2717 "camp.tab.cc"
    break;

  case 143: /* exp: exp GT exp  */
#line 553 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2723 "camp.tab.cc"
    break;

  case 144: /* exp: exp GE exp  */
#line 554 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2729 "camp.tab.cc"
    break;

  case 145: /* exp: exp EQ exp  */
#line 555 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2735 "camp.tab.cc"
    break;

  case 146: /* exp: exp NEQ exp  */
#line 556 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2741 "camp.tab.cc"
    break;

  case 147: /* exp: exp CAND exp  */
#line 557 "camp.y"
                   { (yyval.e) = new andExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2747 "camp.tab.cc"
    break;

  case 148: /* exp: exp COR exp  */
#line 558 "camp.y"
                   { (yyval.e) = new orExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2753 "camp.tab.cc"
    break;

  case 149: /* exp: exp CARETS exp  */
#line 559 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2759 "camp.tab.cc"
    break;

  case 150: /* exp: exp AMPERSAND exp  */
#line 560 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2765 "camp.tab.cc"
    break;

  case 151: /* exp: exp BAR exp  */
#line 561 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2771 "camp.tab.cc"
    break;

  case 152: /* exp: exp OPERATOR exp  */
#line 562 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2777 "camp.tab.cc"
    break;

  case 153: /* exp: exp INCR exp  */
#line 563 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2783 "camp.tab.cc"
    break;

  case 154: /* exp: NEW celltype  */
#line 565 "camp.y"
                   { (yyval.e) = new newRecordExp((yyvsp[-1].pos), (yyvsp[0].t)); }
#line 2789 "camp.tab.cc"
    break;

  case 155: /* exp: NEW celltype dimexps  */
#line 567 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), (yyvsp[0].elist), 0, 0); }
#line 2795 "camp.tab.cc"
    break;

  case 156: /* exp: NEW celltype dimexps dims  */
#line 569 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), (yyvsp[-1].elist), (yyvsp[0].dim), 0); }
#line 2801 "camp.tab.cc"
    break;

  case 157: /* exp: NEW celltype dims  */
#line 571 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), 0, (yyvsp[0].dim), 0); }
#line 2807 "camp.tab.cc"
    break;

  case 158: /* exp: NEW celltype dims arrayinit  */
#line 573 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), 0, (yyvsp[-1].dim), (yyvsp[0].ai)); }
#line 2813 "camp.tab.cc"
    break;

  case 159: /* exp: NEW celltype '(' ')' blockstm  */
#line 575 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-4].pos), (yyvsp[-3].t), new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2819 "camp.tab.cc"
    break;

  case 160: /* exp: NEW celltype dims '(' ')' blockstm  */
#line 577 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos),
                                             new arrayTy((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-3].dim)),
                                             new formals((yyvsp[-2].pos)),
                                             (yyvsp[0].s)); }
#line 2828 "camp.tab.cc"
    break;

  case 161: /* exp: NEW celltype '(' formals ')' blockstm  */
#line 582 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos), (yyvsp[-4].t), (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2834 "camp.tab.cc"
    break;

  case 162: /* exp: NEW celltype dims '(' formals ')' blockstm  */
#line 584 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-6].pos),
                                             new arrayTy((yyvsp[-5].t)->getPos(), (yyvsp[-5].t), (yyvsp[-4].dim)),
                                             (yyvsp[-2].fls),
                                             (yyvsp[0].s)); }
#line 2843 "camp.tab.cc"
    break;

  case 163: /* exp: exp '?' exp ':' exp  */
#line 589 "camp.y"
                   { (yyval.e) = new conditionalExp((yyvsp[-3].pos), (yyvsp[-4].e), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2849 "camp.tab.cc"
    break;

  case 164: /* exp: exp ASSIGN exp  */
#line 590 "camp.y"
                   { (yyval.e) = new assignExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2855 "camp.tab.cc"
    break;

  case 165: /* exp: '(' tuple ')'  */
#line 591 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), new nameExp((yyvsp[-2].pos), SYM_TUPLE), (yyvsp[-1].alist)); }
#line 2861 "camp.tab.cc"
    break;

  case 166: /* exp: exp join exp  */
#line 593 "camp.y"
                   { (yyvsp[-1].j)->pushFront((yyvsp[-2].e)); (yyvsp[-1].j)->pushBack((yyvsp[0].e)); (yyval.e) = (yyvsp[-1].j); }
#line 2867 "camp.tab.cc"
    break;

  case 167: /* exp: exp dir  */
#line 595 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::OUT);
                     joinExp *jexp =
                         new joinExp((yyvsp[0].se)->getPos(), SYM_DOTS);
                     (yyval.e)=jexp;
                     jexp->pushBack((yyvsp[-1].e)); jexp->pushBack((yyvsp[0].se)); }
#line 2877 "camp.tab.cc"
    break;

  case 168: /* exp: INCR exp  */
#line 601 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_PLUS); }
#line 2883 "camp.tab.cc"
    break;

  case 169: /* exp: DASHES exp  */
#line 603 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_MINUS); }
#line 2889 "camp.tab.cc"
    break;

  case 170: /* exp: exp INCR  */
#line 606 "camp.y"
                   { (yyval.e) = new postfixExp((yyvsp[0].ps).pos, (yyvsp[-1].e), SYM_PLUS); }
#line 2895 "camp.tab.cc"
    break;

  case 171: /* exp: exp SELFOP exp  */
#line 607 "camp.y"
                   { (yyval.e) = new selfExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2901 "camp.tab.cc"
    break;

  case 172: /* exp: QUOTE '{' fileblock '}'  */
#line 609 "camp.y"
                   { (yyval.e) = new quoteExp((yyvsp[-3].pos), (yyvsp[-1].b)); }
#line 2907 "camp.tab.cc"
    break;

  case 173: /* join: DASHES  */
#line 615 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos,(yyvsp[0].ps).sym); }
#line 2913 "camp.tab.cc"
    break;

  case 174: /* join: basicjoin  */
#line 617 "camp.y"
                   { (yyval.j) = (yyvsp[0].j); }
#line 2919 "camp.tab.cc"
    break;

  case 175: /* join: dir basicjoin  */
#line 619 "camp.y"
                   { (yyvsp[-1].se)->setSide(camp::OUT);
                     (yyval.j) = (yyvsp[0].j); (yyval.j)->pushFront((yyvsp[-1].se)); }
#line 2926 "camp.tab.cc"
    break;

  case 176: /* join: basicjoin dir  */
#line 622 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2933 "camp.tab.cc"
    break;

  case 177: /* join: dir basicjoin dir  */
#line 625 "camp.y"
                   { (yyvsp[-2].se)->setSide(camp::OUT); (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushFront((yyvsp[-2].se)); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2940 "camp.tab.cc"
    break;

  case 178: /* dir: '{' CURL exp '}'  */
#line 630 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].e)); }
#line 2946 "camp.tab.cc"
    break;

  case 179: /* dir: '{' exp '}'  */
#line 631 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].pos), symbol::opTrans("spec"), (yyvsp[-1].e)); }
#line 2952 "camp.tab.cc"
    break;

  case 180: /* dir: '{' exp ',' exp '}'  */
#line 633 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-4].pos), symbol::opTrans("spec"),
				      new pairExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2959 "camp.tab.cc"
    break;

  case 181: /* dir: '{' exp ',' exp ',' exp '}'  */
#line 636 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-6].pos), symbol::opTrans("spec"),
				      new tripleExp((yyvsp[-4].pos), (yyvsp[-5].e), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2966 "camp.tab.cc"
    break;

  case 182: /* basicjoin: DOTS  */
#line 641 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2972 "camp.tab.cc"
    break;

  case 183: /* basicjoin: DOTS tension DOTS  */
#line 643 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2978 "camp.tab.cc"
    break;

  case 184: /* basicjoin: DOTS controls DOTS  */
#line 645 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2984 "camp.tab.cc"
    break;

  case 185: /* basicjoin: COLONS  */
#line 646 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2990 "camp.tab.cc"
    break;

  case 186: /* basicjoin: LONGDASH  */
#line 647 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2996 "camp.tab.cc"
    break;

  case 187: /* tension: TENSION exp  */
#line 651 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, false)); }
#line 3003 "camp.tab.cc"
    break;

  case 188: /* tension: TENSION exp AND exp  */
#line 654 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, false)); }
#line 3010 "camp.tab.cc"
    break;

  case 189: /* tension: TENSION ATLEAST exp  */
#line 657 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-2].ps).pos, (yyvsp[0].e), (yyvsp[-2].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, true)); }
#line 3017 "camp.tab.cc"
    break;

  case 190: /* tension: TENSION ATLEAST exp AND exp  */
#line 660 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-4].ps).pos, (yyvsp[-2].e), (yyvsp[-4].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, true)); }
#line 3024 "camp.tab.cc"
    break;

  case 191: /* controls: CONTROLS exp  */
#line 665 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 3030 "camp.tab.cc"
    break;

  case 192: /* controls: CONTROLS exp AND exp  */
#line 667 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e)); }
#line 3036 "camp.tab.cc"
    break;

  case 193: /* stm: ';'  */
#line 671 "camp.y"
                   { (yyval.s) = new emptyStm((yyvsp[0].pos)); }
#line 3042 "camp.tab.cc"
    break;

  case 194: /* stm: blockstm  */
#line 672 "camp.y"
                   { (yyval.s) = (yyvsp[0].s); }
#line 3048 "camp.tab.cc"
    break;

  case 195: /* stm: stmexp ';'  */
#line 673 "camp.y"
                   { (yyval.s) = (yyvsp[-1].s); }
#line 3054 "camp.tab.cc"
    break;

  case 196: /* stm: IF '(' exp ')' stm  */
#line 675 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3060 "camp.tab.cc"
    break;

  case 197: /* stm: IF '(' exp ')' stm ELSE stm  */
#line 677 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-6].pos), (yyvsp[-4].e), (yyvsp[-2].s), (yyvsp[0].s)); }
#line 3066 "camp.tab.cc"
    break;

  case 198: /* stm: WHILE '(' exp ')' stm  */
#line 679 "camp.y"
                   { (yyval.s) = new whileStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3072 "camp.tab.cc"
    break;

  case 199: /* stm: DO stm WHILE '(' exp ')' ';'  */
#line 681 "camp.y"
                   { (yyval.s) = new doStm((yyvsp[-6].pos), (yyvsp[-5].s), (yyvsp[-2].e)); }
#line 3078 "camp.tab.cc"
    break;

  case 200: /* stm: FOR '(' forinit ';' fortest ';' forupdate ')' stm  */
#line 683 "camp.y"
                   { (yyval.s) = new forStm((yyvsp[-8].pos), (yyvsp[-6].run), (yyvsp[-4].e), (yyvsp[-2].sel), (yyvsp[0].s)); }
#line 3084 "camp.tab.cc"
    break;

  case 201: /* stm: FOR '(' type ID ':' exp ')' stm  */
#line 685 "camp.y"
                   { (yyval.s) = new extendedForStm((yyvsp[-7].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3090 "camp.tab.cc"
    break;

  case 202: /* stm: BREAK ';'  */
#line 686 "camp.y"
                   { (yyval.s) = new breakStm((yyvsp[-1].pos)); }
#line 3096 "camp.tab.cc"
    break;

  case 203: /* stm: CONTINUE ';'  */
#line 687 "camp.y"
                   { (yyval.s) = new continueStm((yyvsp[-1].pos)); }
#line 3102 "camp.tab.cc"
    break;

  case 204: /* stm: RETURN_ ';'  */
#line 688 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-1].pos)); }
#line 3108 "camp.tab.cc"
    break;

  case 205: /* stm: RETURN_ exp ';'  */
#line 689 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-2].pos), (yyvsp[-1].e)); }
#line 3114 "camp.tab.cc"
    break;

  case 206: /* stmexp: exp  */
#line 693 "camp.y"
                   { (yyval.s) = new expStm((yyvsp[0].e)->getPos(), (yyvsp[0].e)); }
#line 3120 "camp.tab.cc"
    break;

  case 207: /* blockstm: block  */
#line 697 "camp.y"
                   { (yyval.s) = new blockStm((yyvsp[0].b)->getPos(), (yyvsp[0].b)); }
#line 3126 "camp.tab.cc"
    break;

  case 208: /* forinit: %empty  */
#line 701 "camp.y"
                   { (yyval.run) = 0; }
#line 3132 "camp.tab.cc"
    break;

  case 209: /* forinit: stmexplist  */
#line 702 "camp.y"
                   { (yyval.run) = (yyvsp[0].sel); }
#line 3138 "camp.tab.cc"
    break;

  case 210: /* forinit: barevardec  */
#line 703 "camp.y"
                   { (yyval.run) = (yyvsp[0].vd); }
#line 3144 "camp.tab.cc"
    break;

  case 211: /* fortest: %empty  */
#line 707 "camp.y"
                   { (yyval.e) = 0; }
#line 3150 "camp.tab.cc"
    break;

  case 212: /* fortest: exp  */
#line 708 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 3156 "camp.tab.cc"
    break;

  case 213: /* forupdate: %empty  */
#line 712 "camp.y"
                   { (yyval.sel) = 0; }
#line 3162 "camp.tab.cc"
    break;

  case 214: /* forupdate: stmexplist  */
#line 713 "camp.y"
                   { (yyval.sel) = (yyvsp[0].sel); }
#line 3168 "camp.tab.cc"
    break;

  case 215: /* stmexplist: stmexp  */
#line 717 "camp.y"
                   { (yyval.sel) = new stmExpList((yyvsp[0].s)->getPos()); (yyval.sel)->add((yyvsp[0].s)); }
#line 3174 "camp.tab.cc"
    break;

  case 216: /* stmexplist: stmexplist ',' stmexp  */
#line 719 "camp.y"
                   { (yyval.sel) = (yyvsp[-2].sel); (yyval.sel)->add((yyvsp[0].s)); }
#line 3180 "camp.tab.cc"
    break;


#line 3184 "camp.tab.cc"

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

