/* A Bison parser, made by GNU Bison 3.7.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON 30704

/* Bison version string.  */
#define YYBISON_VERSION "3.7.4"

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
  YYSYMBOL_idpair = 84,                    /* idpair  */
  YYSYMBOL_idpairlist = 85,                /* idpairlist  */
  YYSYMBOL_strid = 86,                     /* strid  */
  YYSYMBOL_stridpair = 87,                 /* stridpair  */
  YYSYMBOL_stridpairlist = 88,             /* stridpairlist  */
  YYSYMBOL_vardec = 89,                    /* vardec  */
  YYSYMBOL_barevardec = 90,                /* barevardec  */
  YYSYMBOL_type = 91,                      /* type  */
  YYSYMBOL_celltype = 92,                  /* celltype  */
  YYSYMBOL_dims = 93,                      /* dims  */
  YYSYMBOL_dimexps = 94,                   /* dimexps  */
  YYSYMBOL_decidlist = 95,                 /* decidlist  */
  YYSYMBOL_decid = 96,                     /* decid  */
  YYSYMBOL_decidstart = 97,                /* decidstart  */
  YYSYMBOL_varinit = 98,                   /* varinit  */
  YYSYMBOL_block = 99,                     /* block  */
  YYSYMBOL_arrayinit = 100,                /* arrayinit  */
  YYSYMBOL_basearrayinit = 101,            /* basearrayinit  */
  YYSYMBOL_varinits = 102,                 /* varinits  */
  YYSYMBOL_formals = 103,                  /* formals  */
  YYSYMBOL_explicitornot = 104,            /* explicitornot  */
  YYSYMBOL_formal = 105,                   /* formal  */
  YYSYMBOL_fundec = 106,                   /* fundec  */
  YYSYMBOL_typedec = 107,                  /* typedec  */
  YYSYMBOL_slice = 108,                    /* slice  */
  YYSYMBOL_value = 109,                    /* value  */
  YYSYMBOL_argument = 110,                 /* argument  */
  YYSYMBOL_arglist = 111,                  /* arglist  */
  YYSYMBOL_tuple = 112,                    /* tuple  */
  YYSYMBOL_exp = 113,                      /* exp  */
  YYSYMBOL_join = 114,                     /* join  */
  YYSYMBOL_dir = 115,                      /* dir  */
  YYSYMBOL_basicjoin = 116,                /* basicjoin  */
  YYSYMBOL_tension = 117,                  /* tension  */
  YYSYMBOL_controls = 118,                 /* controls  */
  YYSYMBOL_stm = 119,                      /* stm  */
  YYSYMBOL_stmexp = 120,                   /* stmexp  */
  YYSYMBOL_blockstm = 121,                 /* blockstm  */
  YYSYMBOL_forinit = 122,                  /* forinit  */
  YYSYMBOL_fortest = 123,                  /* fortest  */
  YYSYMBOL_forupdate = 124,                /* forupdate  */
  YYSYMBOL_stmexplist = 125                /* stmexplist  */
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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
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
#define YYLAST   1979

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  200
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  376

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
       0,   191,   191,   195,   196,   201,   202,   207,   208,   209,
     214,   215,   216,   218,   223,   224,   225,   227,   232,   233,
     234,   235,   237,   239,   241,   242,   244,   246,   248,   249,
     254,   256,   260,   261,   266,   267,   272,   274,   278,   279,
     284,   288,   292,   293,   297,   301,   302,   306,   307,   312,
     313,   318,   319,   324,   325,   326,   328,   333,   334,   338,
     343,   344,   346,   348,   353,   354,   355,   359,   361,   366,
     367,   368,   370,   375,   376,   380,   382,   384,   387,   390,
     396,   398,   403,   404,   408,   409,   410,   411,   415,   416,
     418,   419,   421,   422,   425,   429,   430,   432,   434,   436,
     440,   441,   445,   446,   448,   450,   456,   457,   461,   462,
     463,   464,   466,   467,   469,   471,   473,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   498,
     500,   502,   504,   506,   508,   513,   515,   520,   522,   523,
     524,   526,   532,   534,   537,   539,   540,   547,   548,   550,
     553,   556,   562,   563,   564,   567,   573,   574,   576,   578,
     579,   583,   585,   588,   591,   597,   598,   603,   604,   605,
     606,   608,   610,   612,   614,   616,   618,   619,   620,   621,
     625,   629,   633,   634,   635,   639,   640,   644,   645,   649,
     650
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

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    43,    45,    42,    47,    37,
      35,    94,   280,   281,   282,    63,    58,   283,   284,   285,
     123,   125,    40,    41,    46,    44,    91,    93,    59,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312
};
#endif

#define YYPACT_NINF (-285)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-45)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -285,    41,   472,  -285,  -285,   873,   873,   873,   873,  -285,
     873,  -285,   873,  -285,    14,    20,    14,    16,     8,    23,
      31,    20,    20,    26,    49,   614,    56,    86,    91,   687,
    -285,   141,  -285,  -285,  -285,    18,  -285,   543,  -285,  -285,
     100,    53,  -285,  -285,  -285,  -285,   122,  1545,  -285,   125,
    -285,   224,   148,   148,   148,   148,  1931,   330,   203,    60,
    1146,   190,  -285,   194,  -285,    88,     0,   155,   161,   166,
     168,    -1,   169,  -285,   180,    34,  -285,   236,   207,    15,
     873,   873,   193,   873,  -285,  -285,  -285,   961,   148,   235,
     254,   616,   212,  -285,  -285,  -285,  -285,  -285,    35,   217,
    -285,   229,   685,   262,   741,   873,    79,  -285,  -285,   141,
    -285,   873,   873,   873,   873,   873,   873,   873,   873,   873,
     873,   873,   873,   873,   873,   873,   873,   873,   873,   873,
     873,   873,   761,   873,  -285,   231,  -285,   741,  -285,  -285,
     141,   114,  -285,   873,  -285,   873,   270,    14,  -285,  -285,
    -285,  -285,  -285,    24,    39,   401,  -285,   227,   134,   -14,
     797,   105,   234,  1272,  1312,   233,  -285,   278,  -285,   243,
     237,  -285,   249,  -285,   893,  -285,   107,  1545,  -285,   873,
    -285,   245,   998,   247,   -10,   212,   236,   817,  -285,   118,
    -285,   248,  1035,  1545,   873,   853,   291,   292,   197,  1763,
    1791,  1819,  1847,  1875,  1875,  1903,  1903,  1903,  1903,  1939,
     258,   258,   148,   148,   148,   148,   148,  1931,  1545,  1508,
     873,  1188,   197,   231,  -285,  -285,   873,  1545,  1545,  -285,
    -285,   295,   252,  -285,   136,   253,   162,  -285,    -3,   180,
     239,  -285,   149,    20,  -285,  1072,   146,    94,  -285,   797,
     212,   614,   614,   873,    46,   873,   873,   873,  -285,  -285,
     893,   893,  1545,  -285,   873,  -285,  -285,   180,   157,  -285,
    -285,  -285,  1545,  -285,  -285,  -285,  1580,   873,  1617,  -285,
    -285,   873,  1432,  -285,   873,  -285,  -285,   300,  -285,   301,
    -285,  -285,  -285,  -285,   186,  -285,  -285,   180,   239,   239,
     305,  -285,  -285,  -285,   817,  -285,     4,   265,   180,   191,
    1109,   251,  -285,  1352,   873,  1545,   264,  -285,  1545,  -285,
    -285,  1545,  -285,   180,   873,  1654,   873,  1727,  -285,  1230,
    -285,  -285,  -285,  -285,  -285,  -285,    27,   279,   273,  -285,
     817,   817,  -285,   180,  -285,   614,   267,  1392,   873,  -285,
    1691,   873,  1691,  -285,   873,   282,   817,  -285,   276,  -285,
    -285,  -285,  -285,   614,   275,   237,  1691,  1470,   817,  -285,
    -285,  -285,   614,  -285,  -285,  -285
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     7,     0,     0,     0,     0,     9,
       0,     5,     0,   177,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   110,   111,    15,    14,   108,     4,     0,    10,    18,
       0,     0,    42,   191,    19,    20,   109,   190,    11,     0,
     178,   108,   153,   152,   115,   116,   117,     0,   108,     0,
       0,    36,    35,     0,    38,     0,     0,     0,     0,     0,
       7,     0,     0,     3,     0,    44,    83,     0,    44,   138,
       0,     0,     0,   192,   186,   187,   188,     0,   112,     0,
       0,     0,    43,    17,    16,    12,    13,    40,    53,    41,
      49,    51,     0,     0,     0,     0,   166,   169,   157,   154,
     170,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   151,   158,   179,     0,    59,     6,
      98,     0,   149,     0,    97,     0,     0,     0,    21,    24,
      27,    28,    29,     0,     0,     0,    82,     0,    53,    74,
       0,   141,   139,     0,     0,     0,   194,     0,   199,     0,
     193,   189,     7,    93,     0,   102,     0,   100,     8,    84,
      45,     0,     0,     0,    74,    54,     0,     0,    95,     0,
      88,     0,     0,   155,     0,     0,     0,     0,   137,   132,
     131,   135,   134,   129,   130,   125,   126,   127,   128,   133,
     118,   119,   120,   121,   122,   123,   124,   136,   148,     0,
       0,     0,   150,   159,   160,   113,     0,   107,   106,    37,
      39,    30,     0,    32,     0,     0,     0,   156,    74,     0,
      74,    73,     0,     0,    69,     0,     0,    74,   142,     0,
     140,     0,     0,     0,    53,   195,     0,     0,   103,    94,
       0,     0,    86,    91,    85,    89,    46,    55,     0,    50,
      52,    58,    57,    96,    92,    90,   175,     0,   171,   167,
     168,     0,     0,   163,     0,   161,   114,     0,    23,     0,
      22,    26,    25,    55,     0,   143,    70,     0,    74,    74,
      75,    47,    60,    64,     0,    67,     0,    65,     0,     0,
       0,   180,   182,     0,     0,   196,     0,   200,   101,   104,
     105,    87,    80,    56,     0,   173,     0,   147,   162,     0,
      31,    33,    56,   145,    71,    72,    53,    76,     0,    62,
       0,    66,   144,     0,    48,     0,     0,     0,   197,    81,
     176,     0,   172,   164,     0,    78,     0,    61,     0,    68,
     146,   181,   183,     0,     0,   198,   174,     0,     0,    77,
      63,   185,     0,   165,    79,   184
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -285,  -285,   250,  -285,    10,   263,  -285,   285,    36,   170,
     309,    -6,  -285,   307,   246,   -13,   308,   -20,  -285,  -285,
     145,  -284,  -210,   260,   171,  -285,  -285,  -164,  -285,  -227,
    -285,  -285,   238,  -285,  -165,   241,  -285,    -5,  -285,  -121,
     205,  -285,  -285,   -19,   -79,  -138,  -285,  -285,  -285,    -8
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    57,    51,    36,    37,    38,   233,   234,
      63,    64,    65,    39,    40,    41,    42,   185,   162,    99,
     100,   101,   270,    43,   271,   306,   307,   242,   243,   244,
      44,    45,   181,    46,   175,   176,    59,    47,   133,   134,
     135,   196,   197,    48,    49,    50,   169,   316,   364,   170
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      52,    53,    54,    55,   168,    56,    82,    60,    77,   258,
      67,    70,    35,   296,   224,    92,   337,    61,    96,    68,
     268,   -44,    58,     4,    87,    66,    88,   231,    71,   239,
     158,    75,    78,   267,    74,   240,   305,     9,   141,   240,
     293,     3,   231,    90,    90,   339,   240,    35,   149,     9,
     153,   232,   355,   340,   241,    92,    98,   159,   241,   161,
      89,   160,    90,    73,    91,   241,   235,    35,    80,   238,
     167,   334,   335,   157,   294,   163,   164,   184,    90,    62,
     157,   157,   314,   309,   177,    62,   182,    69,   238,   194,
     195,    81,   157,    35,   338,   319,   320,   177,    83,   192,
     193,   295,   285,   142,   198,   143,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   221,   222,   322,
     358,   359,   182,   147,    84,   225,   148,   308,   227,    85,
     228,   230,   250,   240,     4,   246,   369,   247,    97,     4,
     259,   183,   260,     5,     6,   245,   261,   226,   374,   333,
     183,   273,   241,   260,   102,    35,   103,   261,   104,   177,
     342,     7,     8,   136,   262,     9,   238,   317,    10,   128,
     157,   289,   272,    12,   290,   349,   246,   302,    12,   276,
     278,   303,   297,   -34,   298,   304,    19,   146,   299,    22,
     323,    19,   298,   150,    22,   360,   299,   289,    30,   151,
     292,    31,    32,    30,   152,   282,    31,    32,   -34,   154,
      11,   286,   122,   123,   124,   125,   126,   127,   128,   332,
     300,   298,   311,   312,   343,   299,   298,   132,   172,   158,
     299,   272,     5,     6,   310,    89,   140,    90,   313,    91,
     315,    90,   318,    75,   165,   177,   177,   178,   183,   321,
       7,     8,   186,   187,     9,   190,    89,    10,    90,   168,
     137,   132,   325,   229,   180,   253,   327,    12,   173,   329,
     249,   254,   256,   257,   174,   124,   125,   126,   127,   128,
      19,   255,   263,    22,   266,   274,   279,   280,   287,   272,
     288,   291,    30,   330,   231,    31,    32,   241,   336,   347,
     341,   345,   348,   356,   357,   362,   368,   370,   372,   350,
     139,   352,    95,   155,   236,   331,   361,    72,    76,   166,
      79,   269,   248,     4,   156,   272,   272,     5,     6,   223,
     365,     0,   191,   189,   371,     0,   366,     0,     0,   367,
       0,   272,     0,   375,     0,     7,     8,     0,     0,     9,
       0,     0,    10,   272,     0,     0,     0,     0,     0,     0,
      11,   138,    12,     0,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
       0,    24,    25,    26,    27,    28,    29,    30,     0,     0,
      31,    32,    33,    34,     4,     0,     0,     0,     5,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     7,     8,     0,     0,
       9,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,    11,   237,    12,     0,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,     0,    24,    25,    26,    27,    28,    29,    30,     0,
       0,    31,    32,    33,    34,     4,     0,     0,     0,     5,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     8,     0,
       0,     9,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,    11,     0,    12,     0,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,     0,    24,    25,    26,    27,    28,    29,    30,
       0,     0,    31,    32,    33,    34,     4,     0,     0,     0,
       5,     6,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     7,     8,
       0,     0,     9,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,    11,     0,    12,     0,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,     0,    24,    25,    26,    27,    28,    29,
      30,     0,     0,    31,    32,    93,    94,     4,     0,     4,
       0,     5,     6,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     7,     8,     9,     0,     9,    10,     0,    10,     0,
       0,     0,   179,     0,    11,     0,    12,     0,    12,     0,
       0,     0,    13,   180,     0,     0,     0,     0,     0,    19,
       0,    19,    22,    23,    22,    24,    25,    26,    27,    28,
      29,    30,     0,    30,    31,    32,    31,    32,   172,     0,
       4,     0,     5,     6,     5,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     8,     7,     8,     9,     0,     9,    10,     0,    10,
       0,     0,     0,     0,     0,     0,     0,    12,   188,    12,
       0,     0,     0,     0,   174,    86,     0,     0,     0,     0,
      19,     0,    19,    22,     4,    22,     0,     0,     5,     6,
       0,     0,    30,     0,    30,    31,    32,    31,    32,     0,
       0,     0,     0,     0,     4,     0,     7,     8,     5,     6,
       9,     0,     0,    10,   220,     0,     0,   179,     0,     0,
       0,     0,     0,    12,     0,     0,     7,     8,     0,     0,
       9,     0,     0,    10,     0,     0,    19,     0,     0,    22,
       4,     0,     0,    12,     5,     6,     0,     0,    30,     0,
       0,    31,    32,     0,     0,     0,    19,     0,     0,    22,
       4,     0,     7,     8,     5,     6,     9,     0,    30,    10,
       0,    31,    32,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     7,     8,   180,     0,     9,     0,     0,    10,
       0,     0,    19,     0,     0,    22,     4,   246,     0,    12,
       5,     6,     0,     0,    30,   277,     0,    31,    32,     0,
       0,     0,    19,     0,     0,    22,     4,     0,     7,     8,
       5,     6,     9,     0,    30,    10,     0,    31,    32,     0,
       0,     0,     0,     0,     0,    12,   172,     0,     7,     8,
       5,     6,     9,     0,     0,    10,     0,     0,    19,     0,
       0,    22,     0,     0,     0,    12,     0,     0,     7,     8,
      30,     0,     9,    31,    32,    10,     0,     0,    19,     0,
       0,    22,     0,     0,     0,    12,     0,     0,     0,     0,
      30,     0,     0,    31,    32,     0,     0,     0,    19,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,     0,
      30,     0,     0,    31,    32,   105,   106,   107,   108,   109,
     110,     0,     0,     0,     0,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,     0,   130,   131,     0,     0,     0,
       0,   132,   105,   106,   107,   108,   109,   110,     0,   171,
       0,     0,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,     0,   130,   131,   264,     0,     0,     0,   132,   105,
     106,   107,   108,   109,   110,   265,     0,     0,     0,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,     0,   130,
     131,   264,     0,     0,     0,   132,   105,   106,   107,   108,
     109,   110,   275,     0,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     0,   130,   131,     0,     0,
       0,     0,   132,   105,   106,   107,   108,   109,   110,   301,
       0,     0,     0,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,     0,   130,   131,     0,     0,     0,     0,   132,
     105,   106,   107,   108,   109,   110,   344,     0,     0,     0,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,     0,
     130,   131,     0,     0,     0,     0,   132,     0,     0,   144,
       0,   145,   105,   106,   107,   108,   109,   110,     0,     0,
       0,     0,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,     0,   130,   131,     0,     0,     0,     0,   132,   283,
       0,     0,     0,   284,   105,   106,   107,   108,   109,   110,
       0,     0,     0,     0,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,     0,   130,   131,     0,     0,     0,     0,
     132,   353,     0,     0,     0,   354,   105,   106,   107,   108,
     109,   110,     0,     0,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     0,   130,   131,     0,     0,
       0,     0,   132,     0,     0,   251,   105,   106,   107,   108,
     109,   110,     0,     0,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     0,   130,   131,     0,     0,
       0,     0,   132,     0,     0,   252,   105,   106,   107,   108,
     109,   110,     0,     0,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     0,   130,   131,     0,     0,
       0,     0,   132,     0,     0,   346,   105,   106,   107,   108,
     109,   110,     0,     0,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     0,   130,   131,     0,     0,
       0,     0,   132,     0,     0,   363,   105,   106,   107,   108,
     109,   110,     0,     0,     0,     0,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     0,   130,   131,     0,     0,
       0,     0,   132,   328,   105,   106,   107,   108,   109,   110,
       0,     0,     0,     0,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,     0,   130,   131,     0,     0,     0,     0,
     132,   373,   105,   106,   107,   108,   109,   110,     0,     0,
       0,     0,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,     0,   130,   131,   281,     0,     0,     0,   132,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,     0,   130,
     131,     0,     0,     0,   105,   132,   107,   108,   109,   110,
       0,     0,     0,     0,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,     0,   130,   131,     0,     0,     0,   324,
     132,   105,     0,   107,   108,   109,   110,     0,     0,     0,
       0,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
       0,   130,   131,     0,     0,     0,   326,   132,   105,     0,
     107,   108,   109,   110,     0,     0,     0,     0,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,     0,   130,   131,
       0,     0,     0,   351,   132,   105,     0,   107,   108,   109,
     110,     0,     0,     0,     0,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,     0,   130,   131,     0,     0,     0,
       0,   132,   106,   107,   108,   109,   110,     0,     0,     0,
       0,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
       0,     0,   131,     0,     0,     0,     0,   132,   106,   107,
     108,   109,   110,     0,     0,     0,     0,     0,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   106,   107,   108,   109,
     110,     0,     0,   132,     0,     0,     0,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   106,   107,   108,   109,   110,     0,
       0,   132,     0,     0,     0,     0,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   106,   107,   108,   109,   110,     0,     0,   132,
       0,     0,     0,     0,     0,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     106,   107,   108,   109,   110,     0,     0,   132,     0,     0,
       0,     0,     0,     0,     0,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   106,   107,
     108,   109,   110,     0,     0,   132,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   106,   107,   108,   109,
     110,     0,     0,   132,   106,   107,   108,   109,   110,     0,
       0,     0,     0,     0,     0,   121,   122,   123,   124,   125,
     126,   127,   128,     0,   122,   123,   124,   125,   126,   127,
     128,   132,     0,     0,     0,     0,     0,     0,     0,   132
};

static const yytype_int16 yycheck[] =
{
       5,     6,     7,     8,    83,    10,    25,    12,    21,   174,
      16,     3,     2,   240,   135,    35,   300,     3,    37,     3,
     184,     3,    12,     3,    29,    15,    31,     3,    18,    43,
       3,    21,    22,    43,     3,    49,   246,    29,    58,    49,
      43,     0,     3,    44,    44,    41,    49,    37,    48,    29,
      51,    27,   336,    49,    68,    75,     3,    42,    68,    79,
      42,    46,    44,    40,    46,    68,    27,    57,    42,    42,
      83,   298,   299,    46,   238,    80,    81,    42,    44,    71,
      46,    46,    36,   247,    89,    71,    91,    71,    42,    10,
      11,    42,    46,    83,   304,   260,   261,   102,    42,   104,
     105,   239,   223,    43,   109,    45,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   267,
     340,   341,   137,    45,    48,   140,    48,    43,   143,    48,
     145,   147,   162,    49,     3,    40,   356,    42,    48,     3,
      43,    46,    45,     7,     8,   160,    49,    43,   368,   297,
      46,    43,    68,    45,    42,   155,    44,    49,    46,   174,
     308,    25,    26,    48,   179,    29,    42,   256,    32,    31,
      46,    45,   187,    42,    48,   323,    40,    41,    42,   194,
     195,    45,    43,     3,    45,    49,    55,     3,    49,    58,
      43,    55,    45,    48,    58,   343,    49,    45,    67,    48,
      48,    70,    71,    67,    48,   220,    70,    71,    50,    50,
      40,   226,    25,    26,    27,    28,    29,    30,    31,    43,
     243,    45,   251,   252,    43,    49,    45,    40,     3,     3,
      49,   246,     7,     8,   249,    42,    43,    44,   253,    46,
     255,    44,   257,   243,    61,   260,   261,     3,    46,   264,
      25,    26,    45,    34,    29,     3,    42,    32,    44,   348,
      46,    40,   277,     3,    47,    42,   281,    42,    43,   284,
      46,     3,    45,    34,    49,    27,    28,    29,    30,    31,
      55,    48,    47,    58,    47,    47,     5,     5,     3,   304,
      48,    48,    67,     3,     3,    70,    71,    68,     3,   314,
      45,    60,    48,    34,    41,    48,    34,    41,    43,   324,
      57,   326,    37,    73,   154,   289,   345,    18,    21,    83,
      22,   186,   161,     3,    74,   340,   341,     7,     8,   134,
     348,    -1,   104,   102,   363,    -1,   351,    -1,    -1,   354,
      -1,   356,    -1,   372,    -1,    25,    26,    -1,    -1,    29,
      -1,    -1,    32,   368,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      -1,    61,    62,    63,    64,    65,    66,    67,    -1,    -1,
      70,    71,    72,    73,     3,    -1,    -1,    -1,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    61,    62,    63,    64,    65,    66,    67,    -1,
      -1,    70,    71,    72,    73,     3,    -1,    -1,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    -1,    61,    62,    63,    64,    65,    66,    67,
      -1,    -1,    70,    71,    72,    73,     3,    -1,    -1,    -1,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    61,    62,    63,    64,    65,    66,
      67,    -1,    -1,    70,    71,    72,    73,     3,    -1,     3,
      -1,     7,     8,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    25,    26,    29,    -1,    29,    32,    -1,    32,    -1,
      -1,    -1,    36,    -1,    40,    -1,    42,    -1,    42,    -1,
      -1,    -1,    48,    47,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    55,    58,    59,    58,    61,    62,    63,    64,    65,
      66,    67,    -1,    67,    70,    71,    70,    71,     3,    -1,
       3,    -1,     7,     8,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    26,    25,    26,    29,    -1,    29,    32,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    42,
      -1,    -1,    -1,    -1,    49,    48,    -1,    -1,    -1,    -1,
      55,    -1,    55,    58,     3,    58,    -1,    -1,     7,     8,
      -1,    -1,    67,    -1,    67,    70,    71,    70,    71,    -1,
      -1,    -1,    -1,    -1,     3,    -1,    25,    26,     7,     8,
      29,    -1,    -1,    32,    13,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    -1,    25,    26,    -1,    -1,
      29,    -1,    -1,    32,    -1,    -1,    55,    -1,    -1,    58,
       3,    -1,    -1,    42,     7,     8,    -1,    -1,    67,    -1,
      -1,    70,    71,    -1,    -1,    -1,    55,    -1,    -1,    58,
       3,    -1,    25,    26,     7,     8,    29,    -1,    67,    32,
      -1,    70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    -1,    25,    26,    47,    -1,    29,    -1,    -1,    32,
      -1,    -1,    55,    -1,    -1,    58,     3,    40,    -1,    42,
       7,     8,    -1,    -1,    67,    12,    -1,    70,    71,    -1,
      -1,    -1,    55,    -1,    -1,    58,     3,    -1,    25,    26,
       7,     8,    29,    -1,    67,    32,    -1,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    42,     3,    -1,    25,    26,
       7,     8,    29,    -1,    -1,    32,    -1,    -1,    55,    -1,
      -1,    58,    -1,    -1,    -1,    42,    -1,    -1,    25,    26,
      67,    -1,    29,    70,    71,    32,    -1,    -1,    55,    -1,
      -1,    58,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    70,    71,    -1,    -1,    -1,    55,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    70,    71,     4,     5,     6,     7,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,
      -1,    40,     4,     5,     6,     7,     8,     9,    -1,    48,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    36,    -1,    -1,    -1,    40,     4,
       5,     6,     7,     8,     9,    47,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    36,    -1,    -1,    -1,    40,     4,     5,     6,     7,
       8,     9,    47,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,     4,     5,     6,     7,     8,     9,    47,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,    40,
       4,     5,     6,     7,     8,     9,    47,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
      34,    35,    -1,    -1,    -1,    -1,    40,    -1,    -1,    43,
      -1,    45,     4,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    -1,    -1,    -1,    -1,    40,    41,
      -1,    -1,    -1,    45,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,
      40,    41,    -1,    -1,    -1,    45,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    43,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    43,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    43,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    43,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    34,    35,    -1,    -1,
      -1,    -1,    40,    41,     4,     5,     6,     7,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,
      40,    41,     4,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,    35,    36,    -1,    -1,    -1,    40,     4,
       5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,     4,    40,     6,     7,     8,     9,
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
      -1,    -1,    -1,    39,    40,     4,    -1,     6,     7,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,
      -1,    40,     5,     6,     7,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    40,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,     5,     6,     7,     8,
       9,    -1,    -1,    40,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,     5,     6,     7,     8,     9,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,     5,     6,     7,     8,     9,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       5,     6,     7,     8,     9,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,     5,     6,
       7,     8,     9,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    25,    26,
      27,    28,    29,    30,    31,    32,     5,     6,     7,     8,
       9,    -1,    -1,    40,     5,     6,     7,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    25,    26,    27,    28,    29,    30,
      31,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    77,    78,     0,     3,     7,     8,    25,    26,    29,
      32,    40,    42,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    61,    62,    63,    64,    65,    66,
      67,    70,    71,    72,    73,    80,    81,    82,    83,    89,
      90,    91,    92,    99,   106,   107,   109,   113,   119,   120,
     121,    80,   113,   113,   113,   113,   113,    79,    80,   112,
     113,     3,    71,    86,    87,    88,    80,    87,     3,    71,
       3,    80,    86,    40,     3,    80,    89,    91,    80,    92,
      42,    42,   119,    42,    48,    48,    48,   113,   113,    42,
      44,    46,    93,    72,    73,    83,   119,    48,     3,    95,
      96,    97,    42,    44,    46,     4,     5,     6,     7,     8,
       9,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      34,    35,    40,   114,   115,   116,    48,    46,    41,    81,
      43,    93,    43,    45,    43,    45,     3,    45,    48,    48,
      48,    48,    48,    51,    50,    78,    99,    46,     3,    42,
      46,    93,    94,   113,   113,    61,    90,    91,   120,   122,
     125,    48,     3,    43,    49,   110,   111,   113,     3,    36,
      47,   108,   113,    46,    42,    93,    45,    34,    43,   111,
       3,   108,   113,   113,    10,    11,   117,   118,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
      13,   113,   113,   116,   115,   113,    43,   113,   113,     3,
      87,     3,    27,    84,    85,    27,    85,    41,    42,    43,
      49,    68,   103,   104,   105,   113,    40,    42,   100,    46,
      93,    43,    43,    42,     3,    48,    45,    34,   110,    43,
      45,    49,   113,    47,    36,    47,    47,    43,   103,    96,
      98,   100,   113,    43,    47,    47,   113,    12,   113,     5,
       5,    36,   113,    41,    45,   115,   113,     3,    48,    45,
      48,    48,    48,    43,   103,   121,   105,    43,    45,    49,
      91,    47,    41,    45,    49,    98,   101,   102,    43,   103,
     113,   119,   119,   113,    36,   113,   123,   120,   113,   110,
     110,   113,   121,    43,    39,   113,    39,   113,    41,   113,
       3,    84,    43,   121,   105,   105,     3,    97,    98,    41,
      49,    45,   121,    43,    47,    60,    43,   113,    48,   121,
     113,    39,   113,    41,    45,    97,    34,    41,    98,    98,
     121,   119,    48,    43,   124,   125,   113,   113,    34,    98,
      41,   119,    43,    41,    98,   119
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    76,    77,    78,    78,    79,    79,    80,    80,    80,
      81,    81,    81,    81,    82,    82,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      84,    84,    85,    85,    86,    86,    87,    87,    88,    88,
      89,    90,    91,    91,    92,    93,    93,    94,    94,    95,
      95,    96,    96,    97,    97,    97,    97,    98,    98,    99,
     100,   100,   100,   100,   101,   101,   101,   102,   102,   103,
     103,   103,   103,   104,   104,   105,   105,   105,   105,   105,
     106,   106,   107,   107,   108,   108,   108,   108,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     110,   110,   111,   111,   111,   111,   112,   112,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   114,   114,   114,
     114,   114,   115,   115,   115,   115,   116,   116,   116,   116,
     116,   117,   117,   117,   117,   118,   118,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     120,   121,   122,   122,   122,   123,   123,   124,   124,   125,
     125
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     0,     2,     1,     3,     1,
       1,     1,     2,     2,     1,     1,     2,     2,     1,     1,
       1,     3,     5,     5,     3,     5,     5,     3,     3,     3,
       1,     3,     1,     3,     1,     1,     1,     3,     1,     3,
       2,     2,     1,     2,     1,     2,     3,     3,     4,     1,
       3,     1,     3,     1,     2,     3,     4,     1,     1,     3,
       2,     4,     3,     5,     1,     1,     2,     1,     3,     1,
       2,     3,     3,     1,     0,     2,     3,     5,     4,     6,
       5,     6,     3,     2,     1,     2,     2,     3,     3,     4,
       4,     4,     4,     3,     4,     3,     4,     3,     3,     1,
       1,     3,     1,     2,     3,     3,     3,     3,     1,     1,
       1,     1,     2,     4,     5,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       4,     3,     4,     5,     6,     6,     7,     5,     3,     3,
       3,     2,     2,     2,     2,     3,     4,     1,     1,     2,
       2,     3,     4,     3,     5,     7,     1,     3,     3,     1,
       1,     2,     4,     3,     5,     2,     4,     1,     1,     2,
       5,     7,     5,     7,     9,     8,     2,     2,     2,     3,
       1,     1,     0,     1,     1,     0,     1,     0,     1,     1,
       3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


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
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
    goto yyexhaustedlab;
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
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
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
#line 191 "camp.y"
                   { absyntax::root = (yyvsp[0].b); }
#line 1825 "camp.tab.c"
    break;

  case 3: /* fileblock: %empty  */
#line 195 "camp.y"
                   { (yyval.b) = new file(lexerPos(), false); }
#line 1831 "camp.tab.c"
    break;

  case 4: /* fileblock: fileblock runnable  */
#line 197 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1837 "camp.tab.c"
    break;

  case 5: /* bareblock: %empty  */
#line 201 "camp.y"
                   { (yyval.b) = new block(lexerPos(), true); }
#line 1843 "camp.tab.c"
    break;

  case 6: /* bareblock: bareblock runnable  */
#line 203 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1849 "camp.tab.c"
    break;

  case 7: /* name: ID  */
#line 207 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 1855 "camp.tab.c"
    break;

  case 8: /* name: name '.' ID  */
#line 208 "camp.y"
                   { (yyval.n) = new qualifiedName((yyvsp[-1].pos), (yyvsp[-2].n), (yyvsp[0].ps).sym); }
#line 1861 "camp.tab.c"
    break;

  case 9: /* name: '%'  */
#line 209 "camp.y"
                   { (yyval.n) = new simpleName((yyvsp[0].ps).pos,
                                  symbol::trans("operator answer")); }
#line 1868 "camp.tab.c"
    break;

  case 10: /* runnable: dec  */
#line 214 "camp.y"
                   { (yyval.run) = (yyvsp[0].d); }
#line 1874 "camp.tab.c"
    break;

  case 11: /* runnable: stm  */
#line 215 "camp.y"
                   { (yyval.run) = (yyvsp[0].s); }
#line 1880 "camp.tab.c"
    break;

  case 12: /* runnable: modifiers dec  */
#line 217 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].d)); }
#line 1886 "camp.tab.c"
    break;

  case 13: /* runnable: modifiers stm  */
#line 219 "camp.y"
                   { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].s)); }
#line 1892 "camp.tab.c"
    break;

  case 14: /* modifiers: MODIFIER  */
#line 223 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].mod).pos); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1898 "camp.tab.c"
    break;

  case 15: /* modifiers: PERM  */
#line 224 "camp.y"
                   { (yyval.ml) = new modifierList((yyvsp[0].perm).pos); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1904 "camp.tab.c"
    break;

  case 16: /* modifiers: modifiers MODIFIER  */
#line 226 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 1910 "camp.tab.c"
    break;

  case 17: /* modifiers: modifiers PERM  */
#line 228 "camp.y"
                   { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 1916 "camp.tab.c"
    break;

  case 18: /* dec: vardec  */
#line 232 "camp.y"
                   { (yyval.d) = (yyvsp[0].vd); }
#line 1922 "camp.tab.c"
    break;

  case 19: /* dec: fundec  */
#line 233 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1928 "camp.tab.c"
    break;

  case 20: /* dec: typedec  */
#line 234 "camp.y"
                   { (yyval.d) = (yyvsp[0].d); }
#line 1934 "camp.tab.c"
    break;

  case 21: /* dec: ACCESS stridpairlist ';'  */
#line 236 "camp.y"
                   { (yyval.d) = new accessdec((yyvsp[-2].pos), (yyvsp[-1].ipl)); }
#line 1940 "camp.tab.c"
    break;

  case 22: /* dec: FROM name UNRAVEL idpairlist ';'  */
#line 238 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), (yyvsp[-1].ipl)); }
#line 1946 "camp.tab.c"
    break;

  case 23: /* dec: FROM name UNRAVEL '*' ';'  */
#line 240 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), WILDCARD); }
#line 1952 "camp.tab.c"
    break;

  case 24: /* dec: UNRAVEL name ';'  */
#line 241 "camp.y"
                   { (yyval.d) = new unraveldec((yyvsp[-2].pos), (yyvsp[-1].n), WILDCARD); }
#line 1958 "camp.tab.c"
    break;

  case 25: /* dec: FROM strid ACCESS idpairlist ';'  */
#line 243 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, (yyvsp[-1].ipl)); }
#line 1964 "camp.tab.c"
    break;

  case 26: /* dec: FROM strid ACCESS '*' ';'  */
#line 245 "camp.y"
                   { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, WILDCARD); }
#line 1970 "camp.tab.c"
    break;

  case 27: /* dec: IMPORT stridpair ';'  */
#line 247 "camp.y"
                   { (yyval.d) = new importdec((yyvsp[-2].pos), (yyvsp[-1].ip)); }
#line 1976 "camp.tab.c"
    break;

  case 28: /* dec: INCLUDE ID ';'  */
#line 248 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].ps).sym); }
#line 1982 "camp.tab.c"
    break;

  case 29: /* dec: INCLUDE STRING ';'  */
#line 250 "camp.y"
                   { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].stre)->getString()); }
#line 1988 "camp.tab.c"
    break;

  case 30: /* idpair: ID  */
#line 254 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 1994 "camp.tab.c"
    break;

  case 31: /* idpair: ID ID ID  */
#line 256 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2000 "camp.tab.c"
    break;

  case 32: /* idpairlist: idpair  */
#line 260 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2006 "camp.tab.c"
    break;

  case 33: /* idpairlist: idpairlist ',' idpair  */
#line 262 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2012 "camp.tab.c"
    break;

  case 34: /* strid: ID  */
#line 266 "camp.y"
                   { (yyval.ps) = (yyvsp[0].ps); }
#line 2018 "camp.tab.c"
    break;

  case 35: /* strid: STRING  */
#line 267 "camp.y"
                   { (yyval.ps).pos = (yyvsp[0].stre)->getPos();
                     (yyval.ps).sym = symbol::literalTrans((yyvsp[0].stre)->getString()); }
#line 2025 "camp.tab.c"
    break;

  case 36: /* stridpair: ID  */
#line 272 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2031 "camp.tab.c"
    break;

  case 37: /* stridpair: strid ID ID  */
#line 274 "camp.y"
                   { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2037 "camp.tab.c"
    break;

  case 38: /* stridpairlist: stridpair  */
#line 278 "camp.y"
                   { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2043 "camp.tab.c"
    break;

  case 39: /* stridpairlist: stridpairlist ',' stridpair  */
#line 280 "camp.y"
                   { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2049 "camp.tab.c"
    break;

  case 40: /* vardec: barevardec ';'  */
#line 284 "camp.y"
                   { (yyval.vd) = (yyvsp[-1].vd); }
#line 2055 "camp.tab.c"
    break;

  case 41: /* barevardec: type decidlist  */
#line 288 "camp.y"
                   { (yyval.vd) = new vardec((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dil)); }
#line 2061 "camp.tab.c"
    break;

  case 42: /* type: celltype  */
#line 292 "camp.y"
                   { (yyval.t) = (yyvsp[0].t); }
#line 2067 "camp.tab.c"
    break;

  case 43: /* type: name dims  */
#line 293 "camp.y"
                   { (yyval.t) = new arrayTy((yyvsp[-1].n), (yyvsp[0].dim)); }
#line 2073 "camp.tab.c"
    break;

  case 44: /* celltype: name  */
#line 297 "camp.y"
                   { (yyval.t) = new nameTy((yyvsp[0].n)); }
#line 2079 "camp.tab.c"
    break;

  case 45: /* dims: '[' ']'  */
#line 301 "camp.y"
                   { (yyval.dim) = new dimensions((yyvsp[-1].pos)); }
#line 2085 "camp.tab.c"
    break;

  case 46: /* dims: dims '[' ']'  */
#line 302 "camp.y"
                   { (yyval.dim) = (yyvsp[-2].dim); (yyval.dim)->increase(); }
#line 2091 "camp.tab.c"
    break;

  case 47: /* dimexps: '[' exp ']'  */
#line 306 "camp.y"
                   { (yyval.elist) = new explist((yyvsp[-2].pos)); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2097 "camp.tab.c"
    break;

  case 48: /* dimexps: dimexps '[' exp ']'  */
#line 308 "camp.y"
                   { (yyval.elist) = (yyvsp[-3].elist); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2103 "camp.tab.c"
    break;

  case 49: /* decidlist: decid  */
#line 312 "camp.y"
                   { (yyval.dil) = new decidlist((yyvsp[0].di)->getPos()); (yyval.dil)->add((yyvsp[0].di)); }
#line 2109 "camp.tab.c"
    break;

  case 50: /* decidlist: decidlist ',' decid  */
#line 314 "camp.y"
                   { (yyval.dil) = (yyvsp[-2].dil); (yyval.dil)->add((yyvsp[0].di)); }
#line 2115 "camp.tab.c"
    break;

  case 51: /* decid: decidstart  */
#line 318 "camp.y"
                   { (yyval.di) = new decid((yyvsp[0].dis)->getPos(), (yyvsp[0].dis)); }
#line 2121 "camp.tab.c"
    break;

  case 52: /* decid: decidstart ASSIGN varinit  */
#line 320 "camp.y"
                   { (yyval.di) = new decid((yyvsp[-2].dis)->getPos(), (yyvsp[-2].dis), (yyvsp[0].vi)); }
#line 2127 "camp.tab.c"
    break;

  case 53: /* decidstart: ID  */
#line 324 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2133 "camp.tab.c"
    break;

  case 54: /* decidstart: ID dims  */
#line 325 "camp.y"
                   { (yyval.dis) = new decidstart((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym, (yyvsp[0].dim)); }
#line 2139 "camp.tab.c"
    break;

  case 55: /* decidstart: ID '(' ')'  */
#line 326 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, 0,
                                            new formals((yyvsp[-1].pos))); }
#line 2146 "camp.tab.c"
    break;

  case 56: /* decidstart: ID '(' formals ')'  */
#line 329 "camp.y"
                   { (yyval.dis) = new fundecidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym, 0, (yyvsp[-1].fls)); }
#line 2152 "camp.tab.c"
    break;

  case 57: /* varinit: exp  */
#line 333 "camp.y"
                   { (yyval.vi) = (yyvsp[0].e); }
#line 2158 "camp.tab.c"
    break;

  case 58: /* varinit: arrayinit  */
#line 334 "camp.y"
                   { (yyval.vi) = (yyvsp[0].ai); }
#line 2164 "camp.tab.c"
    break;

  case 59: /* block: '{' bareblock '}'  */
#line 339 "camp.y"
                   { (yyval.b) = (yyvsp[-1].b); }
#line 2170 "camp.tab.c"
    break;

  case 60: /* arrayinit: '{' '}'  */
#line 343 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-1].pos)); }
#line 2176 "camp.tab.c"
    break;

  case 61: /* arrayinit: '{' ELLIPSIS varinit '}'  */
#line 345 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[-3].pos)); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2182 "camp.tab.c"
    break;

  case 62: /* arrayinit: '{' basearrayinit '}'  */
#line 347 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2188 "camp.tab.c"
    break;

  case 63: /* arrayinit: '{' basearrayinit ELLIPSIS varinit '}'  */
#line 349 "camp.y"
                   { (yyval.ai) = (yyvsp[-3].ai); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2194 "camp.tab.c"
    break;

  case 64: /* basearrayinit: ','  */
#line 353 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].pos)); }
#line 2200 "camp.tab.c"
    break;

  case 65: /* basearrayinit: varinits  */
#line 354 "camp.y"
                   { (yyval.ai) = (yyvsp[0].ai); }
#line 2206 "camp.tab.c"
    break;

  case 66: /* basearrayinit: varinits ','  */
#line 355 "camp.y"
                   { (yyval.ai) = (yyvsp[-1].ai); }
#line 2212 "camp.tab.c"
    break;

  case 67: /* varinits: varinit  */
#line 359 "camp.y"
                   { (yyval.ai) = new arrayinit((yyvsp[0].vi)->getPos());
		     (yyval.ai)->add((yyvsp[0].vi));}
#line 2219 "camp.tab.c"
    break;

  case 68: /* varinits: varinits ',' varinit  */
#line 362 "camp.y"
                   { (yyval.ai) = (yyvsp[-2].ai); (yyval.ai)->add((yyvsp[0].vi)); }
#line 2225 "camp.tab.c"
    break;

  case 69: /* formals: formal  */
#line 366 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2231 "camp.tab.c"
    break;

  case 70: /* formals: ELLIPSIS formal  */
#line 367 "camp.y"
                   { (yyval.fls) = new formals((yyvsp[-1].pos)); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2237 "camp.tab.c"
    break;

  case 71: /* formals: formals ',' formal  */
#line 369 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2243 "camp.tab.c"
    break;

  case 72: /* formals: formals ELLIPSIS formal  */
#line 371 "camp.y"
                   { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2249 "camp.tab.c"
    break;

  case 73: /* explicitornot: EXPLICIT  */
#line 375 "camp.y"
                   { (yyval.boo) = true; }
#line 2255 "camp.tab.c"
    break;

  case 74: /* explicitornot: %empty  */
#line 376 "camp.y"
                   { (yyval.boo) = false; }
#line 2261 "camp.tab.c"
    break;

  case 75: /* formal: explicitornot type  */
#line 381 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), 0, 0, (yyvsp[-1].boo), 0); }
#line 2267 "camp.tab.c"
    break;

  case 76: /* formal: explicitornot type decidstart  */
#line 383 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dis), 0, (yyvsp[-2].boo), 0); }
#line 2273 "camp.tab.c"
    break;

  case 77: /* formal: explicitornot type decidstart ASSIGN varinit  */
#line 385 "camp.y"
                   { (yyval.fl) = new formal((yyvsp[-3].t)->getPos(), (yyvsp[-3].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-4].boo), 0); }
#line 2279 "camp.tab.c"
    break;

  case 78: /* formal: explicitornot type ID decidstart  */
#line 388 "camp.y"
                   { bool k = checkKeyword((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-2].t)->getPos(), (yyvsp[-2].t), (yyvsp[0].dis), 0, (yyvsp[-3].boo), k); }
#line 2286 "camp.tab.c"
    break;

  case 79: /* formal: explicitornot type ID decidstart ASSIGN varinit  */
#line 391 "camp.y"
                   { bool k = checkKeyword((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-5].boo), k); }
#line 2293 "camp.tab.c"
    break;

  case 80: /* fundec: type ID '(' ')' blockstm  */
#line 397 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-2].pos), (yyvsp[-4].t), (yyvsp[-3].ps).sym, new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2299 "camp.tab.c"
    break;

  case 81: /* fundec: type ID '(' formals ')' blockstm  */
#line 399 "camp.y"
                   { (yyval.d) = new fundec((yyvsp[-3].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2305 "camp.tab.c"
    break;

  case 82: /* typedec: STRUCT ID block  */
#line 403 "camp.y"
                   { (yyval.d) = new recorddec((yyvsp[-2].pos), (yyvsp[-1].ps).sym, (yyvsp[0].b)); }
#line 2311 "camp.tab.c"
    break;

  case 83: /* typedec: TYPEDEF vardec  */
#line 404 "camp.y"
                   { (yyval.d) = new typedec((yyvsp[-1].pos), (yyvsp[0].vd)); }
#line 2317 "camp.tab.c"
    break;

  case 84: /* slice: ':'  */
#line 408 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), 0, 0); }
#line 2323 "camp.tab.c"
    break;

  case 85: /* slice: exp ':'  */
#line 409 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[0].pos), (yyvsp[-1].e), 0); }
#line 2329 "camp.tab.c"
    break;

  case 86: /* slice: ':' exp  */
#line 410 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), 0, (yyvsp[0].e)); }
#line 2335 "camp.tab.c"
    break;

  case 87: /* slice: exp ':' exp  */
#line 411 "camp.y"
                   { (yyval.slice) = new slice((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2341 "camp.tab.c"
    break;

  case 88: /* value: value '.' ID  */
#line 415 "camp.y"
                   { (yyval.e) = new fieldExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].ps).sym); }
#line 2347 "camp.tab.c"
    break;

  case 89: /* value: name '[' exp ']'  */
#line 416 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].e)); }
#line 2354 "camp.tab.c"
    break;

  case 90: /* value: value '[' exp ']'  */
#line 418 "camp.y"
                   { (yyval.e) = new subscriptExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e)); }
#line 2360 "camp.tab.c"
    break;

  case 91: /* value: name '[' slice ']'  */
#line 419 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].slice)); }
#line 2367 "camp.tab.c"
    break;

  case 92: /* value: value '[' slice ']'  */
#line 421 "camp.y"
                     { (yyval.e) = new sliceExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].slice)); }
#line 2373 "camp.tab.c"
    break;

  case 93: /* value: name '(' ')'  */
#line 422 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos),
                                      new nameExp((yyvsp[-2].n)->getPos(), (yyvsp[-2].n)),
                                      new arglist()); }
#line 2381 "camp.tab.c"
    break;

  case 94: /* value: name '(' arglist ')'  */
#line 426 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), 
                                      new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)),
                                      (yyvsp[-1].alist)); }
#line 2389 "camp.tab.c"
    break;

  case 95: /* value: value '(' ')'  */
#line 429 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-1].pos), (yyvsp[-2].e), new arglist()); }
#line 2395 "camp.tab.c"
    break;

  case 96: /* value: value '(' arglist ')'  */
#line 431 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].alist)); }
#line 2401 "camp.tab.c"
    break;

  case 97: /* value: '(' exp ')'  */
#line 433 "camp.y"
                   { (yyval.e) = (yyvsp[-1].e); }
#line 2407 "camp.tab.c"
    break;

  case 98: /* value: '(' name ')'  */
#line 435 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[-1].n)->getPos(), (yyvsp[-1].n)); }
#line 2413 "camp.tab.c"
    break;

  case 99: /* value: THIS  */
#line 436 "camp.y"
                   { (yyval.e) = new thisExp((yyvsp[0].pos)); }
#line 2419 "camp.tab.c"
    break;

  case 100: /* argument: exp  */
#line 440 "camp.y"
                   { (yyval.arg).name = symbol::nullsym; (yyval.arg).val=(yyvsp[0].e); }
#line 2425 "camp.tab.c"
    break;

  case 101: /* argument: ID ASSIGN exp  */
#line 441 "camp.y"
                   { (yyval.arg).name = (yyvsp[-2].ps).sym; (yyval.arg).val=(yyvsp[0].e); }
#line 2431 "camp.tab.c"
    break;

  case 102: /* arglist: argument  */
#line 445 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2437 "camp.tab.c"
    break;

  case 103: /* arglist: ELLIPSIS argument  */
#line 447 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2443 "camp.tab.c"
    break;

  case 104: /* arglist: arglist ',' argument  */
#line 449 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2449 "camp.tab.c"
    break;

  case 105: /* arglist: arglist ELLIPSIS argument  */
#line 451 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2455 "camp.tab.c"
    break;

  case 106: /* tuple: exp ',' exp  */
#line 456 "camp.y"
                   { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[-2].e)); (yyval.alist)->add((yyvsp[0].e)); }
#line 2461 "camp.tab.c"
    break;

  case 107: /* tuple: tuple ',' exp  */
#line 457 "camp.y"
                   { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].e)); }
#line 2467 "camp.tab.c"
    break;

  case 108: /* exp: name  */
#line 461 "camp.y"
                   { (yyval.e) = new nameExp((yyvsp[0].n)->getPos(), (yyvsp[0].n)); }
#line 2473 "camp.tab.c"
    break;

  case 109: /* exp: value  */
#line 462 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2479 "camp.tab.c"
    break;

  case 110: /* exp: LIT  */
#line 463 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 2485 "camp.tab.c"
    break;

  case 111: /* exp: STRING  */
#line 464 "camp.y"
                   { (yyval.e) = (yyvsp[0].stre); }
#line 2491 "camp.tab.c"
    break;

  case 112: /* exp: LIT exp  */
#line 466 "camp.y"
                   { (yyval.e) = new scaleExp((yyvsp[-1].e)->getPos(), (yyvsp[-1].e), (yyvsp[0].e)); }
#line 2497 "camp.tab.c"
    break;

  case 113: /* exp: '(' name ')' exp  */
#line 468 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-2].n)->getPos(), new nameTy((yyvsp[-2].n)), (yyvsp[0].e)); }
#line 2503 "camp.tab.c"
    break;

  case 114: /* exp: '(' name dims ')' exp  */
#line 470 "camp.y"
                   { (yyval.e) = new castExp((yyvsp[-3].n)->getPos(), new arrayTy((yyvsp[-3].n), (yyvsp[-2].dim)), (yyvsp[0].e)); }
#line 2509 "camp.tab.c"
    break;

  case 115: /* exp: '+' exp  */
#line 472 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2515 "camp.tab.c"
    break;

  case 116: /* exp: '-' exp  */
#line 474 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2521 "camp.tab.c"
    break;

  case 117: /* exp: OPERATOR exp  */
#line 475 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2527 "camp.tab.c"
    break;

  case 118: /* exp: exp '+' exp  */
#line 476 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2533 "camp.tab.c"
    break;

  case 119: /* exp: exp '-' exp  */
#line 477 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2539 "camp.tab.c"
    break;

  case 120: /* exp: exp '*' exp  */
#line 478 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2545 "camp.tab.c"
    break;

  case 121: /* exp: exp '/' exp  */
#line 479 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2551 "camp.tab.c"
    break;

  case 122: /* exp: exp '%' exp  */
#line 480 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2557 "camp.tab.c"
    break;

  case 123: /* exp: exp '#' exp  */
#line 481 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2563 "camp.tab.c"
    break;

  case 124: /* exp: exp '^' exp  */
#line 482 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2569 "camp.tab.c"
    break;

  case 125: /* exp: exp LT exp  */
#line 483 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2575 "camp.tab.c"
    break;

  case 126: /* exp: exp LE exp  */
#line 484 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2581 "camp.tab.c"
    break;

  case 127: /* exp: exp GT exp  */
#line 485 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2587 "camp.tab.c"
    break;

  case 128: /* exp: exp GE exp  */
#line 486 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2593 "camp.tab.c"
    break;

  case 129: /* exp: exp EQ exp  */
#line 487 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2599 "camp.tab.c"
    break;

  case 130: /* exp: exp NEQ exp  */
#line 488 "camp.y"
                   { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2605 "camp.tab.c"
    break;

  case 131: /* exp: exp CAND exp  */
#line 489 "camp.y"
                   { (yyval.e) = new andExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2611 "camp.tab.c"
    break;

  case 132: /* exp: exp COR exp  */
#line 490 "camp.y"
                   { (yyval.e) = new orExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2617 "camp.tab.c"
    break;

  case 133: /* exp: exp CARETS exp  */
#line 491 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2623 "camp.tab.c"
    break;

  case 134: /* exp: exp AMPERSAND exp  */
#line 492 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2629 "camp.tab.c"
    break;

  case 135: /* exp: exp BAR exp  */
#line 493 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2635 "camp.tab.c"
    break;

  case 136: /* exp: exp OPERATOR exp  */
#line 494 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2641 "camp.tab.c"
    break;

  case 137: /* exp: exp INCR exp  */
#line 495 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2647 "camp.tab.c"
    break;

  case 138: /* exp: NEW celltype  */
#line 497 "camp.y"
                   { (yyval.e) = new newRecordExp((yyvsp[-1].pos), (yyvsp[0].t)); }
#line 2653 "camp.tab.c"
    break;

  case 139: /* exp: NEW celltype dimexps  */
#line 499 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), (yyvsp[0].elist), 0, 0); }
#line 2659 "camp.tab.c"
    break;

  case 140: /* exp: NEW celltype dimexps dims  */
#line 501 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), (yyvsp[-1].elist), (yyvsp[0].dim), 0); }
#line 2665 "camp.tab.c"
    break;

  case 141: /* exp: NEW celltype dims  */
#line 503 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), 0, (yyvsp[0].dim), 0); }
#line 2671 "camp.tab.c"
    break;

  case 142: /* exp: NEW celltype dims arrayinit  */
#line 505 "camp.y"
                   { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), 0, (yyvsp[-1].dim), (yyvsp[0].ai)); }
#line 2677 "camp.tab.c"
    break;

  case 143: /* exp: NEW celltype '(' ')' blockstm  */
#line 507 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-4].pos), (yyvsp[-3].t), new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2683 "camp.tab.c"
    break;

  case 144: /* exp: NEW celltype dims '(' ')' blockstm  */
#line 509 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos),
                                             new arrayTy((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-3].dim)),
                                             new formals((yyvsp[-2].pos)),
                                             (yyvsp[0].s)); }
#line 2692 "camp.tab.c"
    break;

  case 145: /* exp: NEW celltype '(' formals ')' blockstm  */
#line 514 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-5].pos), (yyvsp[-4].t), (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2698 "camp.tab.c"
    break;

  case 146: /* exp: NEW celltype dims '(' formals ')' blockstm  */
#line 516 "camp.y"
                   { (yyval.e) = new newFunctionExp((yyvsp[-6].pos),
                                             new arrayTy((yyvsp[-5].t)->getPos(), (yyvsp[-5].t), (yyvsp[-4].dim)),
                                             (yyvsp[-2].fls),
                                             (yyvsp[0].s)); }
#line 2707 "camp.tab.c"
    break;

  case 147: /* exp: exp '?' exp ':' exp  */
#line 521 "camp.y"
                   { (yyval.e) = new conditionalExp((yyvsp[-3].pos), (yyvsp[-4].e), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2713 "camp.tab.c"
    break;

  case 148: /* exp: exp ASSIGN exp  */
#line 522 "camp.y"
                   { (yyval.e) = new assignExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2719 "camp.tab.c"
    break;

  case 149: /* exp: '(' tuple ')'  */
#line 523 "camp.y"
                   { (yyval.e) = new callExp((yyvsp[-2].pos), new nameExp((yyvsp[-2].pos), SYM_TUPLE), (yyvsp[-1].alist)); }
#line 2725 "camp.tab.c"
    break;

  case 150: /* exp: exp join exp  */
#line 525 "camp.y"
                   { (yyvsp[-1].j)->pushFront((yyvsp[-2].e)); (yyvsp[-1].j)->pushBack((yyvsp[0].e)); (yyval.e) = (yyvsp[-1].j); }
#line 2731 "camp.tab.c"
    break;

  case 151: /* exp: exp dir  */
#line 527 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::OUT);
                     joinExp *jexp =
                         new joinExp((yyvsp[0].se)->getPos(), SYM_DOTS);
                     (yyval.e)=jexp;
                     jexp->pushBack((yyvsp[-1].e)); jexp->pushBack((yyvsp[0].se)); }
#line 2741 "camp.tab.c"
    break;

  case 152: /* exp: INCR exp  */
#line 533 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_PLUS); }
#line 2747 "camp.tab.c"
    break;

  case 153: /* exp: DASHES exp  */
#line 535 "camp.y"
                   { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_MINUS); }
#line 2753 "camp.tab.c"
    break;

  case 154: /* exp: exp INCR  */
#line 538 "camp.y"
                   { (yyval.e) = new postfixExp((yyvsp[0].ps).pos, (yyvsp[-1].e), SYM_PLUS); }
#line 2759 "camp.tab.c"
    break;

  case 155: /* exp: exp SELFOP exp  */
#line 539 "camp.y"
                   { (yyval.e) = new selfExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2765 "camp.tab.c"
    break;

  case 156: /* exp: QUOTE '{' fileblock '}'  */
#line 541 "camp.y"
                   { (yyval.e) = new quoteExp((yyvsp[-3].pos), (yyvsp[-1].b)); }
#line 2771 "camp.tab.c"
    break;

  case 157: /* join: DASHES  */
#line 547 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos,(yyvsp[0].ps).sym); }
#line 2777 "camp.tab.c"
    break;

  case 158: /* join: basicjoin  */
#line 549 "camp.y"
                   { (yyval.j) = (yyvsp[0].j); }
#line 2783 "camp.tab.c"
    break;

  case 159: /* join: dir basicjoin  */
#line 551 "camp.y"
                   { (yyvsp[-1].se)->setSide(camp::OUT);
                     (yyval.j) = (yyvsp[0].j); (yyval.j)->pushFront((yyvsp[-1].se)); }
#line 2790 "camp.tab.c"
    break;

  case 160: /* join: basicjoin dir  */
#line 554 "camp.y"
                   { (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2797 "camp.tab.c"
    break;

  case 161: /* join: dir basicjoin dir  */
#line 557 "camp.y"
                   { (yyvsp[-2].se)->setSide(camp::OUT); (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushFront((yyvsp[-2].se)); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2804 "camp.tab.c"
    break;

  case 162: /* dir: '{' CURL exp '}'  */
#line 562 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].e)); }
#line 2810 "camp.tab.c"
    break;

  case 163: /* dir: '{' exp '}'  */
#line 563 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-2].pos), symbol::opTrans("spec"), (yyvsp[-1].e)); }
#line 2816 "camp.tab.c"
    break;

  case 164: /* dir: '{' exp ',' exp '}'  */
#line 565 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-4].pos), symbol::opTrans("spec"),
				      new pairExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2823 "camp.tab.c"
    break;

  case 165: /* dir: '{' exp ',' exp ',' exp '}'  */
#line 568 "camp.y"
                   { (yyval.se) = new specExp((yyvsp[-6].pos), symbol::opTrans("spec"),
				      new tripleExp((yyvsp[-4].pos), (yyvsp[-5].e), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2830 "camp.tab.c"
    break;

  case 166: /* basicjoin: DOTS  */
#line 573 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2836 "camp.tab.c"
    break;

  case 167: /* basicjoin: DOTS tension DOTS  */
#line 575 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2842 "camp.tab.c"
    break;

  case 168: /* basicjoin: DOTS controls DOTS  */
#line 577 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2848 "camp.tab.c"
    break;

  case 169: /* basicjoin: COLONS  */
#line 578 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2854 "camp.tab.c"
    break;

  case 170: /* basicjoin: LONGDASH  */
#line 579 "camp.y"
                   { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2860 "camp.tab.c"
    break;

  case 171: /* tension: TENSION exp  */
#line 583 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, false)); }
#line 2867 "camp.tab.c"
    break;

  case 172: /* tension: TENSION exp AND exp  */
#line 586 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, false)); }
#line 2874 "camp.tab.c"
    break;

  case 173: /* tension: TENSION ATLEAST exp  */
#line 589 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-2].ps).pos, (yyvsp[0].e), (yyvsp[-2].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, true)); }
#line 2881 "camp.tab.c"
    break;

  case 174: /* tension: TENSION ATLEAST exp AND exp  */
#line 592 "camp.y"
                   { (yyval.e) = new ternaryExp((yyvsp[-4].ps).pos, (yyvsp[-2].e), (yyvsp[-4].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, true)); }
#line 2888 "camp.tab.c"
    break;

  case 175: /* controls: CONTROLS exp  */
#line 597 "camp.y"
                   { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2894 "camp.tab.c"
    break;

  case 176: /* controls: CONTROLS exp AND exp  */
#line 599 "camp.y"
                   { (yyval.e) = new binaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e)); }
#line 2900 "camp.tab.c"
    break;

  case 177: /* stm: ';'  */
#line 603 "camp.y"
                   { (yyval.s) = new emptyStm((yyvsp[0].pos)); }
#line 2906 "camp.tab.c"
    break;

  case 178: /* stm: blockstm  */
#line 604 "camp.y"
                   { (yyval.s) = (yyvsp[0].s); }
#line 2912 "camp.tab.c"
    break;

  case 179: /* stm: stmexp ';'  */
#line 605 "camp.y"
                   { (yyval.s) = (yyvsp[-1].s); }
#line 2918 "camp.tab.c"
    break;

  case 180: /* stm: IF '(' exp ')' stm  */
#line 607 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 2924 "camp.tab.c"
    break;

  case 181: /* stm: IF '(' exp ')' stm ELSE stm  */
#line 609 "camp.y"
                   { (yyval.s) = new ifStm((yyvsp[-6].pos), (yyvsp[-4].e), (yyvsp[-2].s), (yyvsp[0].s)); }
#line 2930 "camp.tab.c"
    break;

  case 182: /* stm: WHILE '(' exp ')' stm  */
#line 611 "camp.y"
                   { (yyval.s) = new whileStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 2936 "camp.tab.c"
    break;

  case 183: /* stm: DO stm WHILE '(' exp ')' ';'  */
#line 613 "camp.y"
                   { (yyval.s) = new doStm((yyvsp[-6].pos), (yyvsp[-5].s), (yyvsp[-2].e)); }
#line 2942 "camp.tab.c"
    break;

  case 184: /* stm: FOR '(' forinit ';' fortest ';' forupdate ')' stm  */
#line 615 "camp.y"
                   { (yyval.s) = new forStm((yyvsp[-8].pos), (yyvsp[-6].run), (yyvsp[-4].e), (yyvsp[-2].sel), (yyvsp[0].s)); }
#line 2948 "camp.tab.c"
    break;

  case 185: /* stm: FOR '(' type ID ':' exp ')' stm  */
#line 617 "camp.y"
                   { (yyval.s) = new extendedForStm((yyvsp[-7].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].e), (yyvsp[0].s)); }
#line 2954 "camp.tab.c"
    break;

  case 186: /* stm: BREAK ';'  */
#line 618 "camp.y"
                   { (yyval.s) = new breakStm((yyvsp[-1].pos)); }
#line 2960 "camp.tab.c"
    break;

  case 187: /* stm: CONTINUE ';'  */
#line 619 "camp.y"
                   { (yyval.s) = new continueStm((yyvsp[-1].pos)); }
#line 2966 "camp.tab.c"
    break;

  case 188: /* stm: RETURN_ ';'  */
#line 620 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-1].pos)); }
#line 2972 "camp.tab.c"
    break;

  case 189: /* stm: RETURN_ exp ';'  */
#line 621 "camp.y"
                    { (yyval.s) = new returnStm((yyvsp[-2].pos), (yyvsp[-1].e)); }
#line 2978 "camp.tab.c"
    break;

  case 190: /* stmexp: exp  */
#line 625 "camp.y"
                   { (yyval.s) = new expStm((yyvsp[0].e)->getPos(), (yyvsp[0].e)); }
#line 2984 "camp.tab.c"
    break;

  case 191: /* blockstm: block  */
#line 629 "camp.y"
                   { (yyval.s) = new blockStm((yyvsp[0].b)->getPos(), (yyvsp[0].b)); }
#line 2990 "camp.tab.c"
    break;

  case 192: /* forinit: %empty  */
#line 633 "camp.y"
                   { (yyval.run) = 0; }
#line 2996 "camp.tab.c"
    break;

  case 193: /* forinit: stmexplist  */
#line 634 "camp.y"
                   { (yyval.run) = (yyvsp[0].sel); }
#line 3002 "camp.tab.c"
    break;

  case 194: /* forinit: barevardec  */
#line 635 "camp.y"
                   { (yyval.run) = (yyvsp[0].vd); }
#line 3008 "camp.tab.c"
    break;

  case 195: /* fortest: %empty  */
#line 639 "camp.y"
                   { (yyval.e) = 0; }
#line 3014 "camp.tab.c"
    break;

  case 196: /* fortest: exp  */
#line 640 "camp.y"
                   { (yyval.e) = (yyvsp[0].e); }
#line 3020 "camp.tab.c"
    break;

  case 197: /* forupdate: %empty  */
#line 644 "camp.y"
                   { (yyval.sel) = 0; }
#line 3026 "camp.tab.c"
    break;

  case 198: /* forupdate: stmexplist  */
#line 645 "camp.y"
                   { (yyval.sel) = (yyvsp[0].sel); }
#line 3032 "camp.tab.c"
    break;

  case 199: /* stmexplist: stmexp  */
#line 649 "camp.y"
                   { (yyval.sel) = new stmExpList((yyvsp[0].s)->getPos()); (yyval.sel)->add((yyvsp[0].s)); }
#line 3038 "camp.tab.c"
    break;

  case 200: /* stmexplist: stmexplist ',' stmexp  */
#line 651 "camp.y"
                   { (yyval.sel) = (yyvsp[-2].sel); (yyval.sel)->add((yyvsp[0].s)); }
#line 3044 "camp.tab.c"
    break;


#line 3048 "camp.tab.c"

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
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
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

