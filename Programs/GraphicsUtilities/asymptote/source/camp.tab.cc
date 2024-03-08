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

#line 130 "camp.tab.c"

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
  YYSYMBOL_NEW = 58,                       /* NEW  */
  YYSYMBOL_IF = 59,                        /* IF  */
  YYSYMBOL_ELSE = 60,                      /* ELSE  */
  YYSYMBOL_WHILE = 61,                     /* WHILE  */
  YYSYMBOL_DO = 62,                        /* DO  */
  YYSYMBOL_FOR = 63,                       /* FOR  */
  YYSYMBOL_BREAK = 64,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 65,                  /* CONTINUE  */
  YYSYMBOL_RETURN_ = 66,                   /* RETURN_  */
  YYSYMBOL_THIS = 67,                      /* THIS  */
  YYSYMBOL_EXPLICIT = 68,                  /* EXPLICIT  */
  YYSYMBOL_GARBAGE = 69,                   /* GARBAGE  */
  YYSYMBOL_LIT = 70,                       /* LIT  */
  YYSYMBOL_STRING = 71,                    /* STRING  */
  YYSYMBOL_PERM = 72,                      /* PERM  */
  YYSYMBOL_MODIFIER = 73,                  /* MODIFIER  */
  YYSYMBOL_UNARY = 74,                     /* UNARY  */
  YYSYMBOL_EXP_IN_PARENS_RULE = 75,        /* EXP_IN_PARENS_RULE  */
  YYSYMBOL_YYACCEPT = 76,                  /* $accept  */
  YYSYMBOL_file = 77,                      /* file  */
  YYSYMBOL_fileblock = 78,                 /* fileblock  */
  YYSYMBOL_bareblock = 79,                 /* bareblock  */
  YYSYMBOL_name = 80,                      /* name  */
  YYSYMBOL_runnable = 81,                  /* runnable  */
  YYSYMBOL_modifiers = 82,                 /* modifiers  */
  YYSYMBOL_dec = 83,                       /* dec  */
  YYSYMBOL_decdec = 84,                    /* decdec  */
  YYSYMBOL_decdeclist = 85,                /* decdeclist  */
  YYSYMBOL_typeparam = 86,                 /* typeparam  */
  YYSYMBOL_typeparamlist = 87,             /* typeparamlist  */
  YYSYMBOL_idpair = 88,                    /* idpair  */
  YYSYMBOL_idpairlist = 89,                /* idpairlist  */
  YYSYMBOL_strid = 90,                     /* strid  */
  YYSYMBOL_stridpair = 91,                 /* stridpair  */
  YYSYMBOL_stridpairlist = 92,             /* stridpairlist  */
  YYSYMBOL_vardec = 93,                    /* vardec  */
  YYSYMBOL_barevardec = 94,                /* barevardec  */
  YYSYMBOL_type = 95,                      /* type  */
  YYSYMBOL_celltype = 96,                  /* celltype  */
  YYSYMBOL_dims = 97,                      /* dims  */
  YYSYMBOL_dimexps = 98,                   /* dimexps  */
  YYSYMBOL_decidlist = 99,                 /* decidlist  */
  YYSYMBOL_decid = 100,                    /* decid  */
  YYSYMBOL_decidstart = 101,               /* decidstart  */
  YYSYMBOL_varinit = 102,                  /* varinit  */
  YYSYMBOL_block = 103,                    /* block  */
  YYSYMBOL_arrayinit = 104,                /* arrayinit  */
  YYSYMBOL_basearrayinit = 105,            /* basearrayinit  */
  YYSYMBOL_varinits = 106,                 /* varinits  */
  YYSYMBOL_formals = 107,                  /* formals  */
  YYSYMBOL_explicitornot = 108,            /* explicitornot  */
  YYSYMBOL_formal = 109,                   /* formal  */
  YYSYMBOL_fundec = 110,                   /* fundec  */
  YYSYMBOL_typedec = 111,                  /* typedec  */
  YYSYMBOL_slice = 112,                    /* slice  */
  YYSYMBOL_value = 113,                    /* value  */
  YYSYMBOL_argument = 114,                 /* argument  */
  YYSYMBOL_arglist = 115,                  /* arglist  */
  YYSYMBOL_tuple = 116,                    /* tuple  */
  YYSYMBOL_exp = 117,                      /* exp  */
  YYSYMBOL_join = 118,                     /* join  */
  YYSYMBOL_dir = 119,                      /* dir  */
  YYSYMBOL_basicjoin = 120,                /* basicjoin  */
  YYSYMBOL_tension = 121,                  /* tension  */
  YYSYMBOL_controls = 122,                 /* controls  */
  YYSYMBOL_stm = 123,                      /* stm  */
  YYSYMBOL_stmexp = 124,                   /* stmexp  */
  YYSYMBOL_blockstm = 125,                 /* blockstm  */
  YYSYMBOL_forinit = 126,                  /* forinit  */
  YYSYMBOL_fortest = 127,                  /* fortest  */
  YYSYMBOL_forupdate = 128,                /* forupdate  */
  YYSYMBOL_stmexplist = 129                /* stmexplist  */
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
#define YYLAST   1994

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  213
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  416

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   312


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
      73,    74,    75
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
     438,   441,   444,   450,   452,   457,   458,   462,   463,   464,
     465,   469,   470,   472,   473,   475,   476,   479,   483,   484,
     486,   488,   490,   494,   495,   499,   500,   502,   504,   510,
     511,   515,   516,   517,   518,   520,   521,   523,   525,   527,
     529,   530,   531,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   543,   544,   545,   546,   547,   548,
     549,   550,   552,   554,   556,   558,   560,   562,   567,   569,
     574,   576,   577,   578,   580,   586,   588,   591,   593,   594,
     601,   602,   604,   607,   610,   616,   617,   618,   621,   627,
     628,   630,   632,   633,   637,   639,   642,   645,   651,   652,
     657,   658,   659,   660,   662,   664,   666,   668,   670,   672,
     673,   674,   675,   679,   683,   687,   688,   689,   693,   694,
     698,   699,   703,   704
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
  "STRUCT", "TYPEDEF", "NEW", "IF", "ELSE", "WHILE", "DO", "FOR", "BREAK",
  "CONTINUE", "RETURN_", "THIS", "EXPLICIT", "GARBAGE", "LIT", "STRING",
  "PERM", "MODIFIER", "UNARY", "EXP_IN_PARENS_RULE", "$accept", "file",
  "fileblock", "bareblock", "name", "runnable", "modifiers", "dec",
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

#define YYPACT_NINF (-292)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-58)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -292,    43,   500,  -292,  -292,   881,   881,   881,   881,  -292,
     881,  -292,   881,  -292,    20,    31,    15,    24,    17,    21,
      68,    11,    31,    50,    57,   176,    64,   143,   156,   282,
    -292,    27,  -292,  -292,  -292,   129,  -292,   571,  -292,  -292,
     185,   165,  -292,  -292,  -292,  -292,   111,  1560,  -292,   197,
    -292,   175,   183,   183,   183,   183,  1946,   358,   222,   124,
    1161,    45,  -292,    26,  -292,    60,   132,   210,    34,   233,
     238,   245,    -1,     4,    12,  -292,   221,   268,    -2,  -292,
     293,   273,   202,   881,   881,   262,   881,  -292,  -292,  -292,
     976,   183,   642,   316,   669,   281,  -292,  -292,  -292,  -292,
    -292,   213,   283,  -292,   297,   700,   329,   727,   881,   217,
    -292,  -292,    27,  -292,   881,   881,   881,   881,   881,   881,
     881,   881,   881,   881,   881,   881,   881,   881,   881,   881,
     881,   881,   881,   881,   881,   770,   881,  -292,   294,  -292,
     727,  -292,  -292,    27,   160,  -292,   881,  -292,   881,   330,
      35,    20,  -292,  -292,   336,    35,  -292,  -292,  -292,   131,
      35,   147,   429,  -292,   336,   307,   216,     2,   784,   146,
     309,  1287,  1327,   315,  -292,   353,  -292,   310,   317,  -292,
     325,  -292,   908,  -292,   144,  1560,  -292,   881,  -292,   320,
    1013,   321,     7,   281,   293,   827,  -292,   151,  -292,   322,
    1050,  1560,   881,   854,   359,   365,   348,  1778,  1806,  1834,
    1862,  1890,  1890,  1918,  1918,  1918,  1918,  1954,   314,   314,
     183,   183,   183,   183,   183,  1946,  1560,  1523,   881,  1203,
     348,   294,  -292,  -292,   881,  1560,  1560,  -292,   347,  -292,
     239,  -292,   360,  -292,  -292,  -292,   249,   254,   383,   344,
    -292,   225,   258,   345,   231,  -292,   270,     9,   221,   326,
    -292,   166,    31,  -292,  1087,   280,    16,  -292,   784,   281,
     176,   176,   881,   120,   881,   881,   881,  -292,  -292,   908,
     908,  1560,  -292,   881,  -292,  -292,   221,   177,  -292,  -292,
    -292,  1560,  -292,  -292,  -292,  1595,   881,  1632,  -292,  -292,
     881,  1447,  -292,   881,  -292,  -292,    31,    18,    35,   349,
     336,   354,   392,  -292,   393,  -292,   351,  -292,  -292,   356,
    -292,   187,  -292,  -292,   221,   326,   326,   402,  -292,  -292,
    -292,   827,  -292,   110,   362,   221,   208,  1124,   366,  -292,
    1367,   881,  1560,   370,  -292,  1560,  -292,  -292,  1560,  -292,
     221,   881,  1669,   881,  1742,  -292,  1245,  -292,   424,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,   393,  -292,  -292,  -292,
    -292,  -292,   139,   399,   394,  -292,   827,   827,  -292,   221,
    -292,   176,   386,  1407,   881,  -292,  1706,   881,  1706,  -292,
     881,   390,   232,   405,   827,  -292,   400,  -292,  -292,  -292,
    -292,   176,   397,   317,  1706,  1485,  -292,  -292,   827,  -292,
    -292,  -292,   176,  -292,  -292,  -292
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     7,     0,     0,     0,     0,     9,
       0,     5,     0,   190,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     112,   123,   124,    15,    14,   121,     4,     0,    10,    18,
       0,     0,    55,   204,    19,    20,   122,   203,    11,     0,
     191,   121,   166,   165,   128,   129,   130,     0,   121,     0,
       0,    47,    48,     0,    51,     0,     0,     0,     0,     0,
       0,     0,     7,     0,     0,     3,     0,     0,    57,    96,
       0,    57,   151,     0,     0,     0,   205,   199,   200,   201,
       0,   125,     0,     0,     0,    56,    17,    16,    12,    13,
      53,    66,    54,    62,    64,     0,     0,     0,     0,   179,
     182,   170,   167,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   164,   171,   192,
       0,    72,     6,   111,     0,   162,     0,   110,     0,     0,
       0,     0,    21,    24,     0,     0,    27,    28,    29,     0,
       0,     0,     0,    95,     0,     0,    66,    87,     0,   154,
     152,     0,     0,     0,   207,     0,   212,     0,   206,   202,
       7,   106,     0,   115,     0,   113,     8,    97,    58,     0,
       0,     0,    87,    67,     0,     0,   108,     0,   101,     0,
       0,   168,     0,     0,     0,     0,   150,   145,   144,   148,
     147,   142,   143,   138,   139,   140,   141,   146,   131,   132,
     133,   134,   135,   136,   137,   149,   161,     0,     0,     0,
     163,   172,   173,   126,     0,   120,   119,    50,     7,    38,
       0,    37,     0,    52,    40,    41,     0,     0,    43,     0,
      45,     0,     0,     0,     0,   169,     0,    87,     0,    87,
      86,     0,     0,    82,     0,     0,    87,   155,     0,   153,
       0,     0,     0,    66,   208,     0,     0,   116,   107,     0,
       0,    99,   104,    98,   102,    59,    68,     0,    63,    65,
      71,    70,   109,   105,   103,   188,     0,   184,   180,   181,
       0,     0,   176,     0,   174,   127,     0,     0,     0,     0,
       0,     0,     0,    23,     0,    22,     0,    26,    25,     0,
      68,     0,   156,    83,     0,    87,    87,    88,    60,    73,
      77,     0,    80,     0,    78,     0,     0,     0,   193,   195,
       0,     0,   209,     0,   213,   114,   117,   118,   100,    93,
      69,     0,   186,     0,   160,   175,     0,    36,     0,    33,
      39,    31,    42,    34,    44,    46,     0,    30,    69,   158,
      84,    85,    66,    89,     0,    75,     0,    79,   157,     0,
      61,     0,     0,     0,   210,    94,   189,     0,   185,   177,
       0,     0,     0,    91,     0,    74,     0,    81,   159,   194,
     196,     0,     0,   211,   187,     0,    32,    35,     0,    90,
      76,   198,     0,   178,    92,   197
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -292,  -292,   367,  -292,    10,   387,  -292,   406,   137,    52,
     136,   284,   133,  -153,     1,    -3,  -292,   428,   364,    -6,
     430,   -25,  -292,  -292,   257,  -291,  -230,   377,   287,  -292,
    -292,  -183,  -292,  -186,  -292,  -292,   350,  -292,  -143,   355,
    -292,    -5,  -292,  -127,   327,  -292,  -292,   -21,   -80,  -125,
    -292,  -292,  -292,    75
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    57,    51,    36,    37,    38,   239,   240,
     245,   246,   250,   251,    63,    64,    65,    39,    40,    41,
      42,   193,   170,   102,   103,   104,   289,    43,   290,   333,
     334,   261,   262,   263,    44,    45,   189,    46,   183,   184,
      59,    47,   136,   137,   138,   204,   205,    48,    49,    50,
     177,   343,   402,   178
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      52,    53,    54,    55,    85,    56,   176,    60,   254,   287,
      95,   232,    35,    69,     4,    80,    99,    68,    61,    74,
      72,   358,    58,    61,    90,    66,    91,    70,    73,   149,
       4,    78,    81,   144,     4,   332,   373,   149,   238,   277,
       9,   -47,    93,     3,   165,   258,     9,    35,    93,   -47,
     286,   259,   320,    95,   160,   159,   259,   169,   259,   335,
       9,    75,   161,    77,     9,   259,   359,    35,   150,    12,
     260,    76,    67,   323,   321,   260,   155,   260,   171,   172,
     175,   393,    19,   336,   260,    22,    62,   185,    62,   190,
     -49,    62,    83,   -49,    30,    71,    35,    31,    32,    84,
     185,   374,   200,   201,   304,   151,    86,   206,   152,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     229,   230,   -57,   322,   248,   190,   346,   347,   233,   370,
     371,   235,   166,   236,   241,   269,   396,   397,   243,   241,
     248,   375,   242,   105,   241,   106,   341,   107,   249,   376,
      78,   349,   257,   264,   409,    78,   165,   145,   101,   146,
      78,    92,    35,    93,   253,    94,    93,   185,   414,     4,
     153,   257,   281,     5,     6,   165,   265,   278,   266,   279,
     291,    87,   191,   280,   292,   344,   279,   295,   297,   369,
     280,     7,     8,   234,    88,     9,   191,   247,    10,   324,
     378,   325,   252,   392,   131,   326,    11,    92,    12,    93,
     350,   140,   325,   301,    13,   385,   326,   202,   203,   305,
     368,    19,   325,   100,    22,    23,   326,    24,    25,    26,
      27,    28,    29,    30,   167,   139,    31,    32,   168,   338,
     339,   379,   154,   325,   398,   192,   327,   326,   257,   165,
     291,    11,   165,   337,    92,   143,    93,   340,    94,   342,
     314,   345,    78,   315,   185,   185,   314,   314,   348,   318,
     407,   156,   307,     4,   308,     4,   157,     5,     6,     5,
       6,   352,   309,   158,   310,   354,   166,   311,   356,   308,
     357,   316,   241,   308,   176,     7,     8,     7,     8,     9,
     164,     9,    10,   319,    10,   310,    78,    93,    78,   186,
     265,   329,    12,   173,    12,   330,   291,   191,   194,   331,
      89,   195,   198,   237,   135,    19,   383,    19,    22,   244,
      22,   127,   128,   129,   130,   131,   386,    30,   388,    30,
      31,    32,    31,    32,   188,   268,   273,   272,   274,   276,
     399,     4,   275,   149,   298,     5,     6,   282,   285,   293,
     299,   291,   291,   125,   126,   127,   128,   129,   130,   131,
     411,   306,   404,     7,     8,   405,   312,     9,   135,   291,
      10,   415,   313,   317,   260,   364,   248,   361,    11,   141,
      12,   366,   363,   291,   367,   372,    13,   377,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,   384,    24,
      25,    26,    27,    28,    29,    30,   381,   391,    31,    32,
      33,    34,     4,   394,   400,   395,     5,     6,   406,   408,
     412,   410,   162,    98,   142,   360,   362,   365,   256,    79,
     174,   288,    82,   163,     7,     8,   267,   199,     9,   403,
     197,    10,     0,     0,   231,     0,     0,     0,     0,    11,
     255,    12,     0,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,     0,
      24,    25,    26,    27,    28,    29,    30,     0,     0,    31,
      32,    33,    34,     4,     0,     0,     0,     5,     6,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     7,     8,     0,     0,     9,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
      11,     0,    12,     0,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,    25,    26,    27,    28,    29,    30,     0,     0,
      31,    32,    33,    34,     4,     0,     0,     0,     5,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     0,     0,
       9,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,    11,     0,    12,     0,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,    25,    26,    27,    28,    29,    30,     0,
       0,    31,    32,    96,    97,   180,     0,     0,     0,     5,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     0,
       0,     9,     4,     0,    10,     0,     5,     6,     0,     0,
       0,     0,     0,     0,    12,   181,     0,     0,     0,     0,
       0,   182,     0,     0,     7,     8,     0,    19,     9,     0,
      22,    10,     0,   180,     0,   187,     0,     5,     6,    30,
       0,    12,    31,    32,     0,     0,   188,     0,     0,     0,
       0,     0,     0,     0,    19,     7,     8,    22,     0,     9,
       4,     0,    10,     0,     5,     6,    30,     0,     0,    31,
      32,     0,    12,   196,     0,     0,     0,     0,     0,   182,
       0,     0,     7,     8,     0,    19,     9,     0,    22,    10,
       0,     0,     0,   187,     0,     0,     0,    30,     0,    12,
      31,    32,     0,     4,     0,     0,     0,     5,     6,     0,
       0,     0,    19,   228,     0,    22,     0,     4,     0,     0,
       0,     5,     6,     0,    30,     7,     8,    31,    32,     9,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     7,
       8,     0,    12,     9,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,    19,    12,     0,    22,     0,
       4,   188,     0,     0,     5,     6,     0,    30,     0,    19,
      31,    32,    22,     0,     0,     0,     0,     0,     0,     0,
       0,    30,     7,     8,    31,    32,     9,     4,     0,    10,
       0,     5,     6,     0,     0,     0,   296,   265,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     0,    19,     9,     4,    22,    10,     0,     5,     6,
       0,     0,     0,     0,    30,     0,    12,    31,    32,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     0,    19,
       9,   180,    22,    10,     0,     5,     6,     0,     0,     0,
       0,    30,     0,    12,    31,    32,     0,     0,     0,     0,
       0,     0,     0,     7,     8,     0,    19,     9,     0,    22,
      10,     0,     0,     0,     0,     0,     0,     0,    30,     0,
      12,    31,    32,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,    22,     0,     0,     0,
       0,     0,     0,     0,     0,    30,     0,     0,    31,    32,
     108,   109,   110,   111,   112,   113,     0,     0,     0,     0,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,     0,
     133,   134,     0,     0,     0,     0,   135,   108,   109,   110,
     111,   112,   113,     0,   179,     0,     0,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,     0,   133,   134,   283,
       0,     0,     0,   135,   108,   109,   110,   111,   112,   113,
     284,     0,     0,     0,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,     0,   133,   134,   283,     0,     0,     0,
     135,   108,   109,   110,   111,   112,   113,   294,     0,     0,
       0,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,   133,   134,     0,     0,     0,     0,   135,   108,   109,
     110,   111,   112,   113,   328,     0,     0,     0,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,     0,   133,   134,
       0,     0,     0,     0,   135,   108,   109,   110,   111,   112,
     113,   380,     0,     0,     0,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,     0,   133,   134,     0,     0,     0,
       0,   135,     0,     0,   147,     0,   148,   108,   109,   110,
     111,   112,   113,     0,     0,     0,     0,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,     0,   133,   134,     0,
       0,     0,     0,   135,   302,     0,     0,     0,   303,   108,
     109,   110,   111,   112,   113,     0,     0,     0,     0,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,   133,
     134,     0,     0,     0,     0,   135,   389,     0,     0,     0,
     390,   108,   109,   110,   111,   112,   113,     0,     0,     0,
       0,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,   133,   134,     0,     0,     0,     0,   135,     0,     0,
     270,   108,   109,   110,   111,   112,   113,     0,     0,     0,
       0,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,   133,   134,     0,     0,     0,     0,   135,     0,     0,
     271,   108,   109,   110,   111,   112,   113,     0,     0,     0,
       0,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,   133,   134,     0,     0,     0,     0,   135,     0,     0,
     382,   108,   109,   110,   111,   112,   113,     0,     0,     0,
       0,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,   133,   134,     0,     0,     0,     0,   135,     0,     0,
     401,   108,   109,   110,   111,   112,   113,     0,     0,     0,
       0,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,   133,   134,     0,     0,     0,     0,   135,   355,   108,
     109,   110,   111,   112,   113,     0,     0,     0,     0,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,   133,
     134,     0,     0,     0,     0,   135,   413,   108,   109,   110,
     111,   112,   113,     0,     0,     0,     0,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,     0,   133,   134,   300,
       0,     0,     0,   135,   108,   109,   110,   111,   112,   113,
       0,     0,     0,     0,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,     0,   133,   134,     0,     0,     0,   108,
     135,   110,   111,   112,   113,     0,     0,     0,     0,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,   133,
     134,     0,     0,     0,   351,   135,   108,     0,   110,   111,
     112,   113,     0,     0,     0,     0,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,     0,   133,   134,     0,     0,
       0,   353,   135,   108,     0,   110,   111,   112,   113,     0,
       0,     0,     0,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,   133,   134,     0,     0,     0,   387,   135,
     108,     0,   110,   111,   112,   113,     0,     0,     0,     0,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,     0,
     133,   134,     0,     0,     0,     0,   135,   109,   110,   111,
     112,   113,     0,     0,     0,     0,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,     0,     0,   134,     0,     0,
       0,     0,   135,   109,   110,   111,   112,   113,     0,     0,
       0,     0,     0,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   109,   110,   111,   112,   113,     0,     0,   135,     0,
       0,     0,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   109,
     110,   111,   112,   113,     0,     0,   135,     0,     0,     0,
       0,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   109,   110,   111,
     112,   113,     0,     0,   135,     0,     0,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   109,   110,   111,   112,   113,
       0,     0,   135,     0,     0,     0,     0,     0,     0,     0,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   109,   110,   111,   112,   113,     0,     0,
     135,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   109,   110,   111,   112,   113,     0,     0,   135,   109,
     110,   111,   112,   113,     0,     0,     0,     0,     0,     0,
     124,   125,   126,   127,   128,   129,   130,   131,     0,   125,
     126,   127,   128,   129,   130,   131,   135,     0,     0,     0,
       0,     0,     0,     0,   135
};

static const yytype_int16 yycheck[] =
{
       5,     6,     7,     8,    25,    10,    86,    12,   161,   192,
      35,   138,     2,    16,     3,    21,    37,    16,     3,    18,
       3,     3,    12,     3,    29,    15,    31,     3,    18,     3,
       3,    21,    22,    58,     3,   265,   327,     3,     3,   182,
      29,    42,    44,     0,    46,    43,    29,    37,    44,    50,
      43,    49,    43,    78,    42,    51,    49,    82,    49,    43,
      29,    40,    50,    52,    29,    49,    48,    57,    42,    42,
      68,     3,    57,   259,   257,    68,    42,    68,    83,    84,
      86,   372,    55,   266,    68,    58,    71,    92,    71,    94,
      45,    71,    42,    48,    67,    71,    86,    70,    71,    42,
     105,   331,   107,   108,   231,    45,    42,   112,    48,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,     3,   258,     3,   140,   279,   280,   143,   325,
     326,   146,     3,   148,   150,   170,   376,   377,   151,   155,
       3,    41,   151,    42,   160,    44,    36,    46,    27,    49,
     150,   286,    42,   168,   394,   155,    46,    43,     3,    45,
     160,    42,   162,    44,    27,    46,    44,   182,   408,     3,
      48,    42,   187,     7,     8,    46,    40,    43,    42,    45,
     195,    48,    46,    49,    43,   275,    45,   202,   203,   324,
      49,    25,    26,    43,    48,    29,    46,   155,    32,    43,
     335,    45,   160,   366,    31,    49,    40,    42,    42,    44,
      43,    46,    45,   228,    48,   350,    49,    10,    11,   234,
      43,    55,    45,    48,    58,    59,    49,    61,    62,    63,
      64,    65,    66,    67,    42,    48,    70,    71,    46,   270,
     271,    43,    42,    45,   379,    42,   262,    49,    42,    46,
     265,    40,    46,   268,    42,    43,    44,   272,    46,   274,
      45,   276,   262,    48,   279,   280,    45,    45,   283,    48,
      48,    48,    43,     3,    45,     3,    48,     7,     8,     7,
       8,   296,    43,    48,    45,   300,     3,    43,   303,    45,
     306,    43,   308,    45,   384,    25,    26,    25,    26,    29,
      42,    29,    32,    43,    32,    45,   306,    44,   308,     3,
      40,    41,    42,    61,    42,    45,   331,    46,    45,    49,
      48,    34,     3,     3,    40,    55,   341,    55,    58,     3,
      58,    27,    28,    29,    30,    31,   351,    67,   353,    67,
      70,    71,    70,    71,    47,    46,     3,    42,    48,    34,
     381,     3,    45,     3,     5,     7,     8,    47,    47,    47,
       5,   376,   377,    25,    26,    27,    28,    29,    30,    31,
     401,    34,   387,    25,    26,   390,     3,    29,    40,   394,
      32,   412,    48,    48,    68,     3,     3,    48,    40,    41,
      42,    50,    48,   408,    48,     3,    48,    45,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    48,    61,
      62,    63,    64,    65,    66,    67,    60,     3,    70,    71,
      72,    73,     3,    34,    48,    41,     7,     8,    48,    34,
      43,    41,    75,    37,    57,   308,   310,   314,   164,    21,
      86,   194,    22,    76,    25,    26,   169,   107,    29,   384,
     105,    32,    -1,    -1,   137,    -1,    -1,    -1,    -1,    40,
      41,    42,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      61,    62,    63,    64,    65,    66,    67,    -1,    -1,    70,
      71,    72,    73,     3,    -1,    -1,    -1,     7,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      -1,    61,    62,    63,    64,    65,    66,    67,    -1,    -1,
      70,    71,    72,    73,     3,    -1,    -1,    -1,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    61,    62,    63,    64,    65,    66,    67,    -1,
      -1,    70,    71,    72,    73,     3,    -1,    -1,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,     3,    -1,    32,    -1,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    25,    26,    -1,    55,    29,    -1,
      58,    32,    -1,     3,    -1,    36,    -1,     7,     8,    67,
      -1,    42,    70,    71,    -1,    -1,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    25,    26,    58,    -1,    29,
       3,    -1,    32,    -1,     7,     8,    67,    -1,    -1,    70,
      71,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    25,    26,    -1,    55,    29,    -1,    58,    32,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    67,    -1,    42,
      70,    71,    -1,     3,    -1,    -1,    -1,     7,     8,    -1,
      -1,    -1,    55,    13,    -1,    58,    -1,     3,    -1,    -1,
      -1,     7,     8,    -1,    67,    25,    26,    70,    71,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    -1,    42,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    42,    -1,    58,    -1,
       3,    47,    -1,    -1,     7,     8,    -1,    67,    -1,    55,
      70,    71,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    25,    26,    70,    71,    29,     3,    -1,    32,
      -1,     7,     8,    -1,    -1,    -1,    12,    40,    -1,    42,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    -1,    55,    29,     3,    58,    32,    -1,     7,     8,
      -1,    -1,    -1,    -1,    67,    -1,    42,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,    55,
      29,     3,    58,    32,    -1,     7,     8,    -1,    -1,    -1,
      -1,    67,    -1,    42,    70,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    26,    -1,    55,    29,    -1,    58,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      42,    70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    70,    71,
       4,     5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    -1,    -1,    -1,    -1,    40,     4,     5,     6,
       7,     8,     9,    -1,    48,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      -1,    -1,    -1,    40,     4,     5,     6,     7,     8,     9,
      47,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    -1,    -1,    -1,
      40,     4,     5,     6,     7,     8,     9,    47,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,     4,     5,
       6,     7,     8,     9,    47,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    35,
      -1,    -1,    -1,    -1,    40,     4,     5,     6,     7,     8,
       9,    47,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    43,    -1,    45,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    -1,    -1,    40,    41,    -1,    -1,    -1,    45,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    41,    -1,    -1,    -1,
      45,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      43,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      43,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      43,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      43,     4,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,    41,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    41,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,    35,    36,
      -1,    -1,    -1,    40,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,     4,
      40,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    39,    40,     4,    -1,     6,     7,
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
      34,    35,    -1,    -1,    -1,    -1,    40,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     5,     6,     7,     8,     9,    -1,    -1,    40,    -1,
      -1,    -1,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,     5,
       6,     7,     8,     9,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,     5,     6,     7,
       8,     9,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     5,     6,     7,     8,     9,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,     5,     6,     7,     8,     9,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     5,     6,     7,     8,     9,    -1,    -1,    40,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    25,
      26,    27,    28,    29,    30,    31,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    77,    78,     0,     3,     7,     8,    25,    26,    29,
      32,    40,    42,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    61,    62,    63,    64,    65,    66,
      67,    70,    71,    72,    73,    80,    81,    82,    83,    93,
      94,    95,    96,   103,   110,   111,   113,   117,   123,   124,
     125,    80,   117,   117,   117,   117,   117,    79,    80,   116,
     117,     3,    71,    90,    91,    92,    80,    57,    90,    91,
       3,    71,     3,    80,    90,    40,     3,    52,    80,    93,
      95,    80,    96,    42,    42,   123,    42,    48,    48,    48,
     117,   117,    42,    44,    46,    97,    72,    73,    83,   123,
      48,     3,    99,   100,   101,    42,    44,    46,     4,     5,
       6,     7,     8,     9,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    34,    35,    40,   118,   119,   120,    48,
      46,    41,    81,    43,    97,    43,    45,    43,    45,     3,
      42,    45,    48,    48,    42,    42,    48,    48,    48,    51,
      42,    50,    78,   103,    42,    46,     3,    42,    46,    97,
      98,   117,   117,    61,    94,    95,   124,   126,   129,    48,
       3,    43,    49,   114,   115,   117,     3,    36,    47,   112,
     117,    46,    42,    97,    45,    34,    43,   115,     3,   112,
     117,   117,    10,    11,   121,   122,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,    13,   117,
     117,   120,   119,   117,    43,   117,   117,     3,     3,    84,
      85,    95,    90,    91,     3,    86,    87,    85,     3,    27,
      88,    89,    85,    27,    89,    41,    87,    42,    43,    49,
      68,   107,   108,   109,   117,    40,    42,   104,    46,    97,
      43,    43,    42,     3,    48,    45,    34,   114,    43,    45,
      49,   117,    47,    36,    47,    47,    43,   107,   100,   102,
     104,   117,    43,    47,    47,   117,    12,   117,     5,     5,
      36,   117,    41,    45,   119,   117,    34,    43,    45,    43,
      45,    43,     3,    48,    45,    48,    43,    48,    48,    43,
      43,   107,   125,   109,    43,    45,    49,    95,    47,    41,
      45,    49,   102,   105,   106,    43,   107,   117,   123,   123,
     117,    36,   117,   127,   124,   117,   114,   114,   117,   125,
      43,    39,   117,    39,   117,    41,   117,    95,     3,    48,
      84,    48,    86,    48,     3,    88,    50,    48,    43,   125,
     109,   109,     3,   101,   102,    41,    49,    45,   125,    43,
      47,    60,    43,   117,    48,   125,   117,    39,   117,    41,
      45,     3,    89,   101,    34,    41,   102,   102,   125,   123,
      48,    43,   128,   129,   117,   117,    48,    48,    34,   102,
      41,   123,    43,    41,   102,   123
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    76,    77,    78,    78,    79,    79,    80,    80,    80,
      81,    81,    81,    81,    82,    82,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    84,    84,    85,    85,
      86,    87,    87,    88,    88,    89,    89,    90,    90,    91,
      91,    92,    92,    93,    94,    95,    95,    96,    97,    97,
      98,    98,    99,    99,   100,   100,   101,   101,   101,   101,
     102,   102,   103,   104,   104,   104,   104,   105,   105,   105,
     106,   106,   107,   107,   107,   107,   108,   108,   109,   109,
     109,   109,   109,   110,   110,   111,   111,   112,   112,   112,
     112,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   114,   114,   115,   115,   115,   115,   116,
     116,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     118,   118,   118,   118,   118,   119,   119,   119,   119,   120,
     120,   120,   120,   120,   121,   121,   121,   121,   122,   122,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   124,   125,   126,   126,   126,   127,   127,
     128,   128,   129,   129
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
       5,     4,     6,     5,     6,     3,     2,     1,     2,     2,
       3,     3,     4,     4,     4,     4,     3,     4,     3,     4,
       3,     3,     1,     1,     3,     1,     2,     3,     3,     3,
       3,     1,     1,     1,     1,     2,     4,     5,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     4,     3,     4,     5,     6,     6,     7,
       5,     3,     3,     3,     2,     2,     2,     2,     3,     4,
       1,     1,     2,     2,     3,     4,     3,     5,     7,     1,
       3,     3,     1,     1,     2,     4,     3,     5,     2,     4,
       1,     1,     2,     5,     7,     5,     7,     9,     8,     2,
       2,     2,     3,     1,     1,     0,     1,     1,     0,     1,
       0,     1,     1,     3
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
#line 1849 "camp.tab.c"
    break;

  case 3: /* fileblock: %empty  */
#line 199 "camp.y"
                   { (yyval.b) = new file(lexerPos(), false); }
#line 1855 "camp.tab.c"
    break;

  case 4: /* fileblock: fileblock runnable  */
#line 201 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1861 "camp.tab.c"
    break;

  case 5: /* bareblock: %empty  */
#line 205 "camp.y"
                   { (yyval.b) = new block(lexerPos(), true); }
#line 1867 "camp.tab.c"
    break;

  case 6: /* bareblock: bareblock runnable  */
#line 207 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1873 "camp.tab.c"
    break;

  case 7: /* name: ID  */
#line 211 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 1879 "camp.tab.c"
    break;

  case 8: /* name: name '.' ID  */
#line 212 "camp.y"
                   { (yyval.n) = new qualifiedName((yyvsp[-1].pos), (yyvsp[-2].n), (yyvsp[0].ps).sym); }
#line 1885 "camp.tab.c"
    break;

  case 9: /* name: '%'  */
#line 213 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos,
                                  symbol::trans("operator answer")); }
#line 1892 "camp.tab.c"
    break;

  case 10: /* runnable: dec  */
#line 218 "camp.y"
                   { (yyval.run) = (yyvsp[0].d); }
#line 1898 "camp.tab.c"
    break;

  case 11: /* runnable: stm  */
#line 219 "camp.y"
                   { (yyval.run) = (yyvsp[0].s); }
#line 1904 "camp.tab.c"
    break;

  case 12: /* runnable: modifiers dec  */
#line 221 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].d)); }
#line 1910 "camp.tab.c"
    break;

  case 13: /* runnable: modifiers stm  */
#line 223 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].s)); }
#line 1916 "camp.tab.c"
    break;

  case 14: /* modifiers: MODIFIER  */
#line 227 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].mod).pos); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1922 "camp.tab.c"
    break;

  case 15: /* modifiers: PERM  */
#line 228 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].perm).pos); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1928 "camp.tab.c"
    break;

  case 16: /* modifiers: modifiers MODIFIER  */
#line 230 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1934 "camp.tab.c"
    break;

  case 17: /* modifiers: modifiers PERM  */
#line 232 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1940 "camp.tab.c"
    break;

  case 18: /* dec: vardec  */
#line 236 "camp.y"
                   { (yyval.d) = (yyvsp[0].vd); }
#line 1946 "camp.tab.c"
    break;

  case 19: /* dec: fundec  */
#line 237 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1952 "camp.tab.c"
    break;

  case 20: /* dec: typedec  */
#line 238 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1958 "camp.tab.c"
    break;

  case 21: /* dec: ACCESS stridpairlist ';'  */
#line 240 "camp.y"
                   { (yyval.d) = new accessdec((yyvsp[-2].pos), (yyvsp[-1].ipl)); }
#line 1964 "camp.tab.c"
    break;

  case 22: /* dec: FROM name UNRAVEL idpairlist ';'  */
#line 242 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), (yyvsp[-1].ipl)); }
#line 1970 "camp.tab.c"
    break;

  case 23: /* dec: FROM name UNRAVEL '*' ';'  */
#line 244 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), WILDCARD); }
#line 1976 "camp.tab.c"
    break;

  case 24: /* dec: UNRAVEL name ';'  */
#line 245 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-2].pos), (yyvsp[-1].n), WILDCARD); }
#line 1982 "camp.tab.c"
    break;

  case 25: /* dec: FROM strid ACCESS idpairlist ';'  */
#line 247 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, (yyvsp[-1].ipl)); }
#line 1988 "camp.tab.c"
    break;

  case 26: /* dec: FROM strid ACCESS '*' ';'  */
#line 249 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, WILDCARD); }
#line 1994 "camp.tab.c"
    break;

  case 27: /* dec: IMPORT stridpair ';'  */
#line 251 "camp.y"
                   { (yyval.d) = new importdec((yyvsp[-2].pos), (yyvsp[-1].ip)); }
#line 2000 "camp.tab.c"
    break;

  case 28: /* dec: INCLUDE ID ';'  */
#line 252 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].ps).sym); }
#line 2006 "camp.tab.c"
    break;

  case 29: /* dec: INCLUDE STRING ';'  */
#line 254 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].stre)->getString()); }
#line 2012 "camp.tab.c"
    break;

  case 30: /* dec: TYPEDEF IMPORT '(' typeparamlist ')' ';'  */
#line 258 "camp.y"
                   { (yyval.d) = new receiveTypedefDec((yyvsp[-5].pos), (yyvsp[-2].tps)); }
#line 2018 "camp.tab.c"
    break;

  case 31: /* dec: IMPORT TYPEDEF '(' typeparamlist ')' ';'  */
#line 260 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[-5].pos),
                     "Expected 'typedef import(<types>);'");
                   }
#line 2026 "camp.tab.c"
    break;

  case 32: /* dec: ACCESS strid '(' decdeclist ')' ID ID ';'  */
#line 265 "camp.y"
                   { (yyval.d) = new templateAccessDec(
                        (yyvsp[-7].pos), (yyvsp[-6].ps).sym, (yyvsp[-4].fls), (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym, (yyvsp[-2].ps).pos
                      ); }
#line 2034 "camp.tab.c"
    break;

  case 33: /* dec: ACCESS strid '(' decdeclist ')' ';'  */
#line 269 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[0].pos), "expected 'as'"); }
#line 2040 "camp.tab.c"
    break;

  case 34: /* dec: IMPORT strid '(' decdeclist ')' ';'  */
#line 271 "camp.y"
                   { (yyval.d) = new badDec((yyvsp[-5].pos), (yyvsp[-5].pos),
                        "Parametrized imports disallowed to reduce naming "
                        "conflicts. Try "
                        "'access <module>(<type parameters>) as <newname>;'."
                     ); }
#line 2050 "camp.tab.c"
    break;

  case 35: /* dec: FROM strid '(' decdeclist ')' ACCESS idpairlist ';'  */
#line 277 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-7].pos), (yyvsp[-6].ps).sym, (yyvsp[-1].ipl), (yyvsp[-4].fls)); }
#line 2056 "camp.tab.c"
    break;

  case 36: /* decdec: ID ASSIGN type  */
#line 283 "camp.y"
                   { (yyval.fl) = new formal(
                        (yyvsp[-2].ps).pos, (yyvsp[0].t), new decidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym)
                      ); }
#line 2064 "camp.tab.c"
    break;

  case 37: /* decdec: type  */
#line 286 "camp.y"
       { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), nullptr); }
#line 2070 "camp.tab.c"
    break;

  case 38: /* decdeclist: decdec  */
#line 291 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2076 "camp.tab.c"
    break;

  case 39: /* decdeclist: decdeclist ',' decdec  */
#line 293 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2082 "camp.tab.c"
    break;

  case 40: /* typeparam: ID  */
#line 297 "camp.y"
       { (yyval.tp) = new typeParam((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2088 "camp.tab.c"
    break;

  case 41: /* typeparamlist: typeparam  */
#line 302 "camp.y"
                   { (yyval.tps) = new typeParamList((yyvsp[0].tp)->getPos()); (yyval.tps)->add((yyvsp[0].tp)); }
#line 2094 "camp.tab.c"
    break;

  case 42: /* typeparamlist: typeparamlist ',' typeparam  */
#line 304 "camp.y"
                   { (yyval.tps) = (yyvsp[-2].tps); (yyval.tps)->add((yyvsp[0].tp)); }
#line 2100 "camp.tab.c"
    break;

  case 43: /* idpair: ID  */
#line 308 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2106 "camp.tab.c"
    break;

  case 44: /* idpair: ID ID ID  */
#line 310 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2112 "camp.tab.c"
    break;

  case 45: /* idpairlist: idpair  */
#line 314 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2118 "camp.tab.c"
    break;

  case 46: /* idpairlist: idpairlist ',' idpair  */
#line 316 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2124 "camp.tab.c"
    break;

  case 47: /* strid: ID  */
#line 320 "camp.y"
                   { (yyval.ps) = (yyvsp[0].ps); }
#line 2130 "camp.tab.c"
    break;

  case 48: /* strid: STRING  */
#line 321 "camp.y"
                   { (yyval.ps).pos = (yyvsp[0].stre)->getPos();
                     (yyval.ps).sym = symbol::literalTrans((yyvsp[0].stre)->getString()); }
#line 2137 "camp.tab.c"
    break;

  case 49: /* stridpair: ID  */
#line 326 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2143 "camp.tab.c"
    break;

  case 50: /* stridpair: strid ID ID  */
#line 328 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2149 "camp.tab.c"
    break;

  case 51: /* stridpairlist: stridpair  */
#line 332 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2155 "camp.tab.c"
    break;

  case 52: /* stridpairlist: stridpairlist ',' stridpair  */
#line 334 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2161 "camp.tab.c"
    break;

  case 53: /* vardec: barevardec ';'  */
#line 338 "camp.y"
                   { (yyval.vd) = (yyvsp[-1].vd); }
#line 2167 "camp.tab.c"
    break;

  case 54: /* barevardec: type decidlist  */
#line 342 "camp.y"
                   { (yyval.vd) = new vardec((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dil)); }
#line 2173 "camp.tab.c"
    break;

  case 55: /* type: celltype  */
#line 346 "camp.y"
                   { (yyval.t) = (yyvsp[0].t); }
#line 2179 "camp.tab.c"
    break;

  case 56: /* type: name dims  */
#line 347 "camp.y"
                   { (yyval.t) = new arrayTy((yyvsp[-1].n), (yyvsp[0].dim)); }
#line 2185 "camp.tab.c"
    break;

  case 57: /* celltype: name  */
#line 351 "camp.y"
                   { (yyval.t) = new nameTy((yyvsp[0].n)); }
#line 2191 "camp.tab.c"
    break;

  case 58: /* dims: '[' ']'  */
#line 355 "camp.y"
                   { (yyval.dim) = new dimensions((yyvsp[-1].pos)); }
#line 2197 "camp.tab.c"
    break;

  case 59: /* dims: dims '[' ']'  */
#line 356 "camp.y"
                   { (yyval.dim) = (yyvsp[-2].dim); (yyval.dim)->increase(); }
#line 2203 "camp.tab.c"
    break;

  case 60: /* dimexps: '[' exp ']'  */
#line 360 "camp.y"
                   { (yyval.elist) = new explist((yyvsp[-2].pos)); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2209 "camp.tab.c"
    break;

  case 61: /* dimexps: dimexps '[' exp ']'  */
#line 362 "camp.y"
                   { (yyval.elist) = (yyvsp[-3].elist); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2215 "camp.tab.c"
    break;

  case 62: /* decidlist: decid  */
#line 366 "camp.y"
                   { (yyval.dil) = new decidlist((yyvsp[0].di)->getPos()); (yyval.dil)->add((yyvsp[0].di)); }
#line 2221 "camp.tab.c"
    break;

  case 63: /* decidlist: decidlist ',' decid  */
#line 368 "camp.y"
                   { (yyval.dil) = (yyvsp[-2].dil); (yyval.dil)->add((yyvsp[0].di)); }
#line 2227 "camp.tab.c"
    break;

  case 64: /* decid: decidstart  */
#line 372 "camp.y"
                   { (yyval.di) = new decid((yyvsp[0].dis)->getPos(), (yyvsp[0].dis)); }
#line 2233 "camp.tab.c"
    break;

  case 65: /* decid: decidstart ASSIGN varinit  */
#line 374 "camp.y"
                   { (yyval.di) = new decid((yyvsp[-2].dis)->getPos(), (yyvsp[-2].dis), (yyvsp[0].vi)); }
#line 2239 "camp.tab.c"
    break;

  case 66: /* decidstart: ID  */
#line 378 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2245 "camp.tab.c"
    break;

  case 67: /* decidstart: ID dims  */
#line 379 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym, (yyvsp[0].dim)); }
#line 2251 "camp.tab.c"
    break;

  case 68: /* decidstart: ID '(' ')'  */
#line 380 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, 0,
                                            new formals((yyvsp[-1].pos))); }
#line 2258 "camp.tab.c"
    break;

  case 69: /* decidstart: ID '(' formals ')'  */
#line 383 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym, 0, (yyvsp[-1].fls)); }
#line 2264 "camp.tab.c"
    break;

  case 70: /* varinit: exp  */
#line 387 "camp.y"
                   { (yyval.vi) = (yyvsp[0].e); }
#line 2270 "camp.tab.c"
    break;

  case 71: /* varinit: arrayinit  */
#line 388 "camp.y"
                   { (yyval.vi) = (yyvsp[0].ai); }
#line 2276 "camp.tab.c"
    break;

  case 72: /* block: '{' bareblock '}'  */
#line 393 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); }
#line 2282 "camp.tab.c"
    break;

  case 73: /* arrayinit: '{' '}'  */
#line 397 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-1].pos)); }
#line 2288 "camp.tab.c"
    break;

  case 74: /* arrayinit: '{' ELLIPSIS varinit '}'  */
#line 399 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-3].pos)); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2294 "camp.tab.c"
    break;

  case 75: /* arrayinit: '{' basearrayinit '}'  */
#line 401 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2300 "camp.tab.c"
    break;

  case 76: /* arrayinit: '{' basearrayinit ELLIPSIS varinit '}'  */
#line 403 "camp.y"
                   { (yyval.ai) = (yyvsp[-3].ai); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2306 "camp.tab.c"
    break;

  case 77: /* basearrayinit: ','  */
#line 407 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].pos)); }
#line 2312 "camp.tab.c"
    break;

  case 78: /* basearrayinit: varinits  */
#line 408 "camp.y"
                   { (yyval.ai) = (yyvsp[0].ai); }
#line 2318 "camp.tab.c"
    break;

  case 79: /* basearrayinit: varinits ','  */
#line 409 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2324 "camp.tab.c"
    break;

  case 80: /* varinits: varinit  */
#line 413 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].vi)->getPos());
		     (yyval.ai)->add((yyvsp[0].vi));}
#line 2331 "camp.tab.c"
    break;

  case 81: /* varinits: varinits ',' varinit  */
#line 416 "camp.y"
                   { (yyval.ai) = (yyvsp[-2].ai); (yyval.ai)->add((yyvsp[0].vi)); }
#line 2337 "camp.tab.c"
    break;

  case 82: /* formals: formal  */
#line 420 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2343 "camp.tab.c"
    break;

  case 83: /* formals: ELLIPSIS formal  */
#line 421 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[-1].pos)); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2349 "camp.tab.c"
    break;

  case 84: /* formals: formals ',' formal  */
#line 423 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2355 "camp.tab.c"
    break;

  case 85: /* formals: formals ELLIPSIS formal  */
#line 425 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2361 "camp.tab.c"
    break;

  case 86: /* explicitornot: EXPLICIT  */
#line 429 "camp.y"
                   { (yyval.boo) = true; }
#line 2367 "camp.tab.c"
    break;

  case 87: /* explicitornot: %empty  */
#line 430 "camp.y"
                   { (yyval.boo) = false; }
#line 2373 "camp.tab.c"
    break;

  case 88: /* formal: explicitornot type  */
#line 435 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), 0, 0, (yyvsp[-1].boo), 0); }
#line 2379 "camp.tab.c"
    break;

  case 89: /* formal: explicitornot type decidstart  */
#line 437 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dis), 0, (yyvsp[-2].boo), 0); }
#line 2385 "camp.tab.c"
    break;

  case 90: /* formal: explicitornot type decidstart ASSIGN varinit  */
#line 439 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-3].t)->getPos(), (yyvsp[-3].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-4].boo), 0); }
#line 2391 "camp.tab.c"
    break;

  case 91: /* formal: explicitornot type ID decidstart  */
#line 442 "camp.y"
                   { bool k = checkKeyword((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-2].t)->getPos(), (yyvsp[-2].t), (yyvsp[0].dis), 0, (yyvsp[-3].boo), k); }
#line 2398 "camp.tab.c"
    break;

  case 92: /* formal: explicitornot type ID decidstart ASSIGN varinit  */
#line 445 "camp.y"
                   { bool k = checkKeyword((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-5].boo), k); }
#line 2405 "camp.tab.c"
    break;

  case 93: /* fundec: type ID '(' ')' blockstm  */
#line 451 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-2].pos), (yyvsp[-4].t), (yyvsp[-3].ps).sym, new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2411 "camp.tab.c"
    break;

  case 94: /* fundec: type ID '(' formals ')' blockstm  */
#line 453 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-3].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2417 "camp.tab.c"
    break;

  case 95: /* typedec: STRUCT ID block  */
#line 457 "camp.y"
                   { (yyval.d) = new recorddec((yyvsp[-2].pos), (yyvsp[-1].ps).sym, (yyvsp[0].b)); }
#line 2423 "camp.tab.c"
    break;

  case 96: /* typedec: TYPEDEF vardec  */
#line 458 "camp.y"
                   { (yyval.d) = new typedec((yyvsp[-1].pos), (yyvsp[0].vd)); }
#line 2429 "camp.tab.c"
    break;

  case 97: /* slice: ':'  */
#line 462 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), 0, 0); }
#line 2435 "camp.tab.c"
    break;

  case 98: /* slice: exp ':'  */
#line 463 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), (yyvsp[-1].e), 0); }
#line 2441 "camp.tab.c"
    break;

  case 99: /* slice: ':' exp  */
#line 464 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), 0, (yyvsp[0].e)); }
#line 2447 "camp.tab.c"
    break;

  case 100: /* slice: exp ':' exp  */
#line 465 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2453 "camp.tab.c"
    break;

  case 101: /* value: value '.' ID  */
#line 469 "camp.y"
                   { (yyval.e) = new fieldExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].ps).sym); }
#line 2459 "camp.tab.c"
    break;

  case 102: /* value: name '[' exp ']'  */
#line 470 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].e)); }
#line 2466 "camp.tab.c"
    break;

  case 103: /* value: value '[' exp ']'  */
#line 472 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e)); }
#line 2472 "camp.tab.c"
    break;

  case 104: /* value: name '[' slice ']'  */
#line 473 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].slice)); }
#line 2479 "camp.tab.c"
    break;

  case 105: /* value: value '[' slice ']'  */
#line 475 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].slice)); }
#line 2485 "camp.tab.c"
    break;

  case 106: /* value: name '(' ')'  */
#line 476 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos),
                                      new nameExp((yyvsp[-2].n)->getPos(), (yyvsp[-2].n)),
                                      new arglist()); }
#line 2493 "camp.tab.c"
    break;

  case 107: /* value: name '(' arglist ')'  */
#line 480 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos),
                                      new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)),
                                      (yyvsp[-1].alist)); }
#line 2501 "camp.tab.c"
    break;

  case 108: /* value: value '(' ')'  */
#line 483 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos), (yyvsp[-2].e), new arglist()); }
#line 2507 "camp.tab.c"
    break;

  case 109: /* value: value '(' arglist ')'  */
#line 485 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].alist)); }
#line 2513 "camp.tab.c"
    break;

  case 110: /* value: '(' exp ')'  */
#line 487 "camp.y"
                   { (yyval.e) = (yyvsp[-1].e); }
#line 2519 "camp.tab.c"
    break;

  case 111: /* value: '(' name ')'  */
#line 489 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[-1].n)->getPos(), (yyvsp[-1].n)); }
#line 2525 "camp.tab.c"
    break;

  case 112: /* value: THIS  */
#line 490 "camp.y"
                   { (yyval.e) = new thisExp((yyvsp[0].pos)); }
#line 2531 "camp.tab.c"
    break;

  case 113: /* argument: exp  */
#line 494 "camp.y"
                   { (yyval.arg).name = symbol::nullsym; (yyval.arg).val=(yyvsp[0].e); }
#line 2537 "camp.tab.c"
    break;

  case 114: /* argument: ID ASSIGN exp  */
#line 495 "camp.y"
                   { (yyval.arg).name = (yyvsp[-2].ps).sym; (yyval.arg).val=(yyvsp[0].e); }
#line 2543 "camp.tab.c"
    break;

  case 115: /* arglist: argument  */
#line 499 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2549 "camp.tab.c"
    break;

  case 116: /* arglist: ELLIPSIS argument  */
#line 501 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2555 "camp.tab.c"
    break;

  case 117: /* arglist: arglist ',' argument  */
#line 503 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2561 "camp.tab.c"
    break;

  case 118: /* arglist: arglist ELLIPSIS argument  */
#line 505 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2567 "camp.tab.c"
    break;

  case 119: /* tuple: exp ',' exp  */
#line 510 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[-2].e)); (yyval.alist)->add((yyvsp[0].e)); }
#line 2573 "camp.tab.c"
    break;

  case 120: /* tuple: tuple ',' exp  */
#line 511 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].e)); }
#line 2579 "camp.tab.c"
    break;

  case 121: /* exp: name  */
#line 515 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[0].n)->getPos(), (yyvsp[0].n)); }
#line 2585 "camp.tab.c"
    break;

  case 122: /* exp: value  */
#line 516 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2591 "camp.tab.c"
    break;

  case 123: /* exp: LIT  */
#line 517 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2597 "camp.tab.c"
    break;

  case 124: /* exp: STRING  */
#line 518 "camp.y"
                   { (yyval.e) = (yyvsp[0].stre); }
#line 2603 "camp.tab.c"
    break;

  case 125: /* exp: LIT exp  */
#line 520 "camp.y"
                   { (yyval.e) = new scaleExp((yyvsp[-1].e)->getPos(), (yyvsp[-1].e), (yyvsp[0].e)); }
#line 2609 "camp.tab.c"
    break;

  case 126: /* exp: '(' name ')' exp  */
#line 522 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-2].n)->getPos(), new nameTy((yyvsp[-2].n)), (yyvsp[0].e)); }
#line 2615 "camp.tab.c"
    break;

  case 127: /* exp: '(' name dims ')' exp  */
#line 524 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-3].n)->getPos(), new arrayTy((yyvsp[-3].n), (yyvsp[-2].dim)), (yyvsp[0].e)); }
#line 2621 "camp.tab.c"
    break;

  case 128: /* exp: '+' exp  */
#line 526 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2627 "camp.tab.c"
    break;

  case 129: /* exp: '-' exp  */
#line 528 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2633 "camp.tab.c"
    break;

  case 130: /* exp: OPERATOR exp  */
#line 529 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2639 "camp.tab.c"
    break;

  case 131: /* exp: exp '+' exp  */
#line 530 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2645 "camp.tab.c"
    break;

  case 132: /* exp: exp '-' exp  */
#line 531 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2651 "camp.tab.c"
    break;

  case 133: /* exp: exp '*' exp  */
#line 532 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2657 "camp.tab.c"
    break;

  case 134: /* exp: exp '/' exp  */
#line 533 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2663 "camp.tab.c"
    break;

  case 135: /* exp: exp '%' exp  */
#line 534 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2669 "camp.tab.c"
    break;

  case 136: /* exp: exp '#' exp  */
#line 535 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2675 "camp.tab.c"
    break;

  case 137: /* exp: exp '^' exp  */
#line 536 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2681 "camp.tab.c"
    break;

  case 138: /* exp: exp LT exp  */
#line 537 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2687 "camp.tab.c"
    break;

  case 139: /* exp: exp LE exp  */
#line 538 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2693 "camp.tab.c"
    break;

  case 140: /* exp: exp GT exp  */
#line 539 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2699 "camp.tab.c"
    break;

  case 141: /* exp: exp GE exp  */
#line 540 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2705 "camp.tab.c"
    break;

  case 142: /* exp: exp EQ exp  */
#line 541 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2711 "camp.tab.c"
    break;

  case 143: /* exp: exp NEQ exp  */
#line 542 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2717 "camp.tab.c"
    break;

  case 144: /* exp: exp CAND exp  */
#line 543 "camp.y"
                   { (yyval.e) = new andExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2723 "camp.tab.c"
    break;

  case 145: /* exp: exp COR exp  */
#line 544 "camp.y"
                   { (yyval.e) = new orExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2729 "camp.tab.c"
    break;

  case 146: /* exp: exp CARETS exp  */
#line 545 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2735 "camp.tab.c"
    break;

  case 147: /* exp: exp AMPERSAND exp  */
#line 546 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2741 "camp.tab.c"
    break;

  case 148: /* exp: exp BAR exp  */
#line 547 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2747 "camp.tab.c"
    break;

  case 149: /* exp: exp OPERATOR exp  */
#line 548 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2753 "camp.tab.c"
    break;

  case 150: /* exp: exp INCR exp  */
#line 549 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2759 "camp.tab.c"
    break;

  case 151: /* exp: NEW celltype  */
#line 551 "camp.y"
                   { (yyval.e) = new newRecordExp((yyvsp[-1].pos), (yyvsp[0].t)); }
#line 2765 "camp.tab.c"
    break;

  case 152: /* exp: NEW celltype dimexps  */
#line 553 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), (yyvsp[0].elist), 0, 0); }
#line 2771 "camp.tab.c"
    break;

  case 153: /* exp: NEW celltype dimexps dims  */
#line 555 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), (yyvsp[-1].elist), (yyvsp[0].dim), 0); }
#line 2777 "camp.tab.c"
    break;

  case 154: /* exp: NEW celltype dims  */
#line 557 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), 0, (yyvsp[0].dim), 0); }
#line 2783 "camp.tab.c"
    break;

  case 155: /* exp: NEW celltype dims arrayinit  */
#line 559 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), 0, (yyvsp[-1].dim), (yyvsp[0].ai)); }
#line 2789 "camp.tab.c"
    break;

  case 156: /* exp: NEW celltype '(' ')' blockstm  */
#line 561 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-4].pos), (yyvsp[-3].t), new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2795 "camp.tab.c"
    break;

  case 157: /* exp: NEW celltype dims '(' ')' blockstm  */
#line 563 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos),
                                             new arrayTy((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-3].dim)),
                                             new formals((yyvsp[-2].pos)),
                                             (yyvsp[0].s)); }
#line 2804 "camp.tab.c"
    break;

  case 158: /* exp: NEW celltype '(' formals ')' blockstm  */
#line 568 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos), (yyvsp[-4].t), (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2810 "camp.tab.c"
    break;

  case 159: /* exp: NEW celltype dims '(' formals ')' blockstm  */
#line 570 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-6].pos),
                                             new arrayTy((yyvsp[-5].t)->getPos(), (yyvsp[-5].t), (yyvsp[-4].dim)),
                                             (yyvsp[-2].fls),
                                             (yyvsp[0].s)); }
#line 2819 "camp.tab.c"
    break;

  case 160: /* exp: exp '?' exp ':' exp  */
#line 575 "camp.y"
                   { (yyval.e) = new conditionalExp((yyvsp[-3].pos), (yyvsp[-4].e), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2825 "camp.tab.c"
    break;

  case 161: /* exp: exp ASSIGN exp  */
#line 576 "camp.y"
                   { (yyval.e) = new assignExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2831 "camp.tab.c"
    break;

  case 162: /* exp: '(' tuple ')'  */
#line 577 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), new nameExp((yyvsp[-2].pos), SYM_TUPLE), (yyvsp[-1].alist)); }
#line 2837 "camp.tab.c"
    break;

  case 163: /* exp: exp join exp  */
#line 579 "camp.y"
                   { (yyvsp[-1].j)->pushFront((yyvsp[-2].e)); (yyvsp[-1].j)->pushBack((yyvsp[0].e)); (yyval.e) = (yyvsp[-1].j); }
#line 2843 "camp.tab.c"
    break;

  case 164: /* exp: exp dir  */
#line 581 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::OUT);
                     joinExp *jexp =
                         new joinExp((yyvsp[0].se)->getPos(), SYM_DOTS);
                     (yyval.e)=jexp;
                     jexp->pushBack((yyvsp[-1].e)); jexp->pushBack((yyvsp[0].se)); }
#line 2853 "camp.tab.c"
    break;

  case 165: /* exp: INCR exp  */
#line 587 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_PLUS); }
#line 2859 "camp.tab.c"
    break;

  case 166: /* exp: DASHES exp  */
#line 589 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_MINUS); }
#line 2865 "camp.tab.c"
    break;

  case 167: /* exp: exp INCR  */
#line 592 "camp.y"
                   { (yyval.e) = new postfixExp((yyvsp[0].ps).pos, (yyvsp[-1].e), SYM_PLUS); }
#line 2871 "camp.tab.c"
    break;

  case 168: /* exp: exp SELFOP exp  */
#line 593 "camp.y"
                   { (yyval.e) = new selfExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2877 "camp.tab.c"
    break;

  case 169: /* exp: QUOTE '{' fileblock '}'  */
#line 595 "camp.y"
                   { (yyval.e) = new quoteExp((yyvsp[-3].pos), (yyvsp[-1].b)); }
#line 2883 "camp.tab.c"
    break;

  case 170: /* join: DASHES  */
#line 601 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos,(yyvsp[0].ps).sym); }
#line 2889 "camp.tab.c"
    break;

  case 171: /* join: basicjoin  */
#line 603 "camp.y"
                   { (yyval.j) = (yyvsp[0].j); }
#line 2895 "camp.tab.c"
    break;

  case 172: /* join: dir basicjoin  */
#line 605 "camp.y"
                   { (yyvsp[-1].se)->setSide(camp::OUT);
                     (yyval.j) = (yyvsp[0].j); (yyval.j)->pushFront((yyvsp[-1].se)); }
#line 2902 "camp.tab.c"
    break;

  case 173: /* join: basicjoin dir  */
#line 608 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2909 "camp.tab.c"
    break;

  case 174: /* join: dir basicjoin dir  */
#line 611 "camp.y"
                   { (yyvsp[-2].se)->setSide(camp::OUT); (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushFront((yyvsp[-2].se)); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2916 "camp.tab.c"
    break;

  case 175: /* dir: '{' CURL exp '}'  */
#line 616 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].e)); }
#line 2922 "camp.tab.c"
    break;

  case 176: /* dir: '{' exp '}'  */
#line 617 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].pos), symbol::opTrans("spec"), (yyvsp[-1].e)); }
#line 2928 "camp.tab.c"
    break;

  case 177: /* dir: '{' exp ',' exp '}'  */
#line 619 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-4].pos), symbol::opTrans("spec"),
				      new pairExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2935 "camp.tab.c"
    break;

  case 178: /* dir: '{' exp ',' exp ',' exp '}'  */
#line 622 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-6].pos), symbol::opTrans("spec"),
				      new tripleExp((yyvsp[-4].pos), (yyvsp[-5].e), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2942 "camp.tab.c"
    break;

  case 179: /* basicjoin: DOTS  */
#line 627 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2948 "camp.tab.c"
    break;

  case 180: /* basicjoin: DOTS tension DOTS  */
#line 629 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2954 "camp.tab.c"
    break;

  case 181: /* basicjoin: DOTS controls DOTS  */
#line 631 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2960 "camp.tab.c"
    break;

  case 182: /* basicjoin: COLONS  */
#line 632 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2966 "camp.tab.c"
    break;

  case 183: /* basicjoin: LONGDASH  */
#line 633 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2972 "camp.tab.c"
    break;

  case 184: /* tension: TENSION exp  */
#line 637 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, false)); }
#line 2979 "camp.tab.c"
    break;

  case 185: /* tension: TENSION exp AND exp  */
#line 640 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, false)); }
#line 2986 "camp.tab.c"
    break;

  case 186: /* tension: TENSION ATLEAST exp  */
#line 643 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-2].ps).pos, (yyvsp[0].e), (yyvsp[-2].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, true)); }
#line 2993 "camp.tab.c"
    break;

  case 187: /* tension: TENSION ATLEAST exp AND exp  */
#line 646 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-4].ps).pos, (yyvsp[-2].e), (yyvsp[-4].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, true)); }
#line 3000 "camp.tab.c"
    break;

  case 188: /* controls: CONTROLS exp  */
#line 651 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 3006 "camp.tab.c"
    break;

  case 189: /* controls: CONTROLS exp AND exp  */
#line 653 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e)); }
#line 3012 "camp.tab.c"
    break;

  case 190: /* stm: ';'  */
#line 657 "camp.y"
                   { (yyval.s) = new emptyStm((yyvsp[0].pos)); }
#line 3018 "camp.tab.c"
    break;

  case 191: /* stm: blockstm  */
#line 658 "camp.y"
                   { (yyval.s) = (yyvsp[0].s); }
#line 3024 "camp.tab.c"
    break;

  case 192: /* stm: stmexp ';'  */
#line 659 "camp.y"
                   { (yyval.s) = (yyvsp[-1].s); }
#line 3030 "camp.tab.c"
    break;

  case 193: /* stm: IF '(' exp ')' stm  */
#line 661 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3036 "camp.tab.c"
    break;

  case 194: /* stm: IF '(' exp ')' stm ELSE stm  */
#line 663 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-6].pos), (yyvsp[-4].e), (yyvsp[-2].s), (yyvsp[0].s)); }
#line 3042 "camp.tab.c"
    break;

  case 195: /* stm: WHILE '(' exp ')' stm  */
#line 665 "camp.y"
                   { (yyval.s) = new whileStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3048 "camp.tab.c"
    break;

  case 196: /* stm: DO stm WHILE '(' exp ')' ';'  */
#line 667 "camp.y"
                   { (yyval.s) = new doStm((yyvsp[-6].pos), (yyvsp[-5].s), (yyvsp[-2].e)); }
#line 3054 "camp.tab.c"
    break;

  case 197: /* stm: FOR '(' forinit ';' fortest ';' forupdate ')' stm  */
#line 669 "camp.y"
                   { (yyval.s) = new forStm((yyvsp[-8].pos), (yyvsp[-6].run), (yyvsp[-4].e), (yyvsp[-2].sel), (yyvsp[0].s)); }
#line 3060 "camp.tab.c"
    break;

  case 198: /* stm: FOR '(' type ID ':' exp ')' stm  */
#line 671 "camp.y"
                   { (yyval.s) = new extendedForStm((yyvsp[-7].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3066 "camp.tab.c"
    break;

  case 199: /* stm: BREAK ';'  */
#line 672 "camp.y"
                   { (yyval.s) = new breakStm((yyvsp[-1].pos)); }
#line 3072 "camp.tab.c"
    break;

  case 200: /* stm: CONTINUE ';'  */
#line 673 "camp.y"
                   { (yyval.s) = new continueStm((yyvsp[-1].pos)); }
#line 3078 "camp.tab.c"
    break;

  case 201: /* stm: RETURN_ ';'  */
#line 674 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-1].pos)); }
#line 3084 "camp.tab.c"
    break;

  case 202: /* stm: RETURN_ exp ';'  */
#line 675 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-2].pos), (yyvsp[-1].e)); }
#line 3090 "camp.tab.c"
    break;

  case 203: /* stmexp: exp  */
#line 679 "camp.y"
                   { (yyval.s) = new expStm((yyvsp[0].e)->getPos(), (yyvsp[0].e)); }
#line 3096 "camp.tab.c"
    break;

  case 204: /* blockstm: block  */
#line 683 "camp.y"
                   { (yyval.s) = new blockStm((yyvsp[0].b)->getPos(), (yyvsp[0].b)); }
#line 3102 "camp.tab.c"
    break;

  case 205: /* forinit: %empty  */
#line 687 "camp.y"
                   { (yyval.run) = 0; }
#line 3108 "camp.tab.c"
    break;

  case 206: /* forinit: stmexplist  */
#line 688 "camp.y"
                   { (yyval.run) = (yyvsp[0].sel); }
#line 3114 "camp.tab.c"
    break;

  case 207: /* forinit: barevardec  */
#line 689 "camp.y"
                   { (yyval.run) = (yyvsp[0].vd); }
#line 3120 "camp.tab.c"
    break;

  case 208: /* fortest: %empty  */
#line 693 "camp.y"
                   { (yyval.e) = 0; }
#line 3126 "camp.tab.c"
    break;

  case 209: /* fortest: exp  */
#line 694 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 3132 "camp.tab.c"
    break;

  case 210: /* forupdate: %empty  */
#line 698 "camp.y"
                   { (yyval.sel) = 0; }
#line 3138 "camp.tab.c"
    break;

  case 211: /* forupdate: stmexplist  */
#line 699 "camp.y"
                   { (yyval.sel) = (yyvsp[0].sel); }
#line 3144 "camp.tab.c"
    break;

  case 212: /* stmexplist: stmexp  */
#line 703 "camp.y"
                   { (yyval.sel) = new stmExpList((yyvsp[0].s)->getPos()); (yyval.sel)->add((yyvsp[0].s)); }
#line 3150 "camp.tab.c"
    break;

  case 213: /* stmexplist: stmexplist ',' stmexp  */
#line 705 "camp.y"
                   { (yyval.sel) = (yyvsp[-2].sel); (yyval.sel)->add((yyvsp[0].s)); }
#line 3156 "camp.tab.c"
    break;


#line 3160 "camp.tab.c"

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

