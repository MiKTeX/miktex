/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "camp.y" /* yacc.c:339  */

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

#line 125 "camp.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "camp.tab.h".  */
#ifndef YY_YY_CAMP_TAB_H_INCLUDED
# define YY_YY_CAMP_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    SELFOP = 259,
    DOTS = 260,
    COLONS = 261,
    DASHES = 262,
    INCR = 263,
    LONGDASH = 264,
    CONTROLS = 265,
    TENSION = 266,
    ATLEAST = 267,
    CURL = 268,
    COR = 269,
    CAND = 270,
    BAR = 271,
    AMPERSAND = 272,
    EQ = 273,
    NEQ = 274,
    LT = 275,
    LE = 276,
    GT = 277,
    GE = 278,
    CARETS = 279,
    OPERATOR = 280,
    LOOSE = 281,
    ASSIGN = 282,
    DIRTAG = 283,
    JOIN_PREC = 284,
    AND = 285,
    ELLIPSIS = 286,
    ACCESS = 287,
    UNRAVEL = 288,
    IMPORT = 289,
    INCLUDE = 290,
    FROM = 291,
    QUOTE = 292,
    STRUCT = 293,
    TYPEDEF = 294,
    NEW = 295,
    IF = 296,
    ELSE = 297,
    WHILE = 298,
    DO = 299,
    FOR = 300,
    BREAK = 301,
    CONTINUE = 302,
    RETURN_ = 303,
    THIS = 304,
    EXPLICIT = 305,
    GARBAGE = 306,
    LIT = 307,
    STRING = 308,
    PERM = 309,
    MODIFIER = 310,
    UNARY = 311,
    EXP_IN_PARENS_RULE = 312
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 60 "camp.y" /* yacc.c:355  */

  position pos;
  bool boo;
  struct {
    position pos;
    sym::symbol sym;
  } ps;
  absyntax::name *n;
  absyntax::varinit *vi;
  absyntax::arrayinit *ai;
  absyntax::exp *e;
  absyntax::stringExp *stre;
  absyntax::specExp *se;
  absyntax::joinExp *j;
  absyntax::explist *elist;
  absyntax::argument arg;
  absyntax::arglist *alist;
  absyntax::slice *slice;
  absyntax::dimensions *dim;
  absyntax::ty  *t;
  absyntax::decid *di;
  absyntax::decidlist *dil;
  absyntax::decidstart *dis;
  absyntax::runnable *run;
  struct {
    position pos;
    trans::permission val;
  } perm;
  struct {
    position pos;
    trans::modifier val;
  } mod;
  absyntax::modifierList *ml;
  //absyntax::program *prog;
  absyntax::vardec *vd;
  //absyntax::vardecs *vds;
  absyntax::dec *d;
  absyntax::idpair *ip;
  absyntax::idpairlist *ipl;
  absyntax::stm *s;
  absyntax::block *b;
  absyntax::stmExpList *sel;
  //absyntax::funheader *fh;
  absyntax::formal *fl;
  absyntax::formals *fls;

#line 270 "camp.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_CAMP_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 287 "camp.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
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

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
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
static const yytype_uint16 yyrline[] =
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

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ID", "SELFOP", "DOTS", "COLONS",
  "DASHES", "INCR", "LONGDASH", "CONTROLS", "TENSION", "ATLEAST", "CURL",
  "COR", "CAND", "BAR", "AMPERSAND", "EQ", "NEQ", "LT", "LE", "GT", "GE",
  "CARETS", "'+'", "'-'", "'*'", "'/'", "'%'", "'#'", "'^'", "OPERATOR",
  "LOOSE", "ASSIGN", "'?'", "':'", "DIRTAG", "JOIN_PREC", "AND", "'{'",
  "'}'", "'('", "')'", "'.'", "','", "'['", "']'", "';'", "ELLIPSIS",
  "ACCESS", "UNRAVEL", "IMPORT", "INCLUDE", "FROM", "QUOTE", "STRUCT",
  "TYPEDEF", "NEW", "IF", "ELSE", "WHILE", "DO", "FOR", "BREAK",
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
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
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
# endif

#define YYPACT_NINF -285

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-285)))

#define YYTABLE_NINF -45

#define yytable_value_is_error(Yytable_value) \
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
static const yytype_uint8 yystos[] =
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
static const yytype_uint8 yyr1[] =
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
static const yytype_uint8 yyr2[] =
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


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
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
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
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
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
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
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
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
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
        case 2:
#line 191 "camp.y" /* yacc.c:1646  */
    { absyntax::root = (yyvsp[0].b); }
#line 1954 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 195 "camp.y" /* yacc.c:1646  */
    { (yyval.b) = new file(lexerPos(), false); }
#line 1960 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 197 "camp.y" /* yacc.c:1646  */
    { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1966 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 201 "camp.y" /* yacc.c:1646  */
    { (yyval.b) = new block(lexerPos(), true); }
#line 1972 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 203 "camp.y" /* yacc.c:1646  */
    { (yyval.b) = (yyvsp[-1].b); (yyval.b)->add((yyvsp[0].run)); }
#line 1978 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 207 "camp.y" /* yacc.c:1646  */
    { (yyval.n) = new simpleName((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 1984 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 208 "camp.y" /* yacc.c:1646  */
    { (yyval.n) = new qualifiedName((yyvsp[-1].pos), (yyvsp[-2].n), (yyvsp[0].ps).sym); }
#line 1990 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 209 "camp.y" /* yacc.c:1646  */
    { (yyval.n) = new simpleName((yyvsp[0].ps).pos,
                                  symbol::trans("operator answer")); }
#line 1997 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 214 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = (yyvsp[0].d); }
#line 2003 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 215 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = (yyvsp[0].s); }
#line 2009 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 217 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].d)); }
#line 2015 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 219 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = new modifiedRunnable((yyvsp[-1].ml)->getPos(), (yyvsp[-1].ml), (yyvsp[0].s)); }
#line 2021 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 223 "camp.y" /* yacc.c:1646  */
    { (yyval.ml) = new modifierList((yyvsp[0].mod).pos); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 2027 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 224 "camp.y" /* yacc.c:1646  */
    { (yyval.ml) = new modifierList((yyvsp[0].perm).pos); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 2033 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 226 "camp.y" /* yacc.c:1646  */
    { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].mod).val); }
#line 2039 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 228 "camp.y" /* yacc.c:1646  */
    { (yyval.ml) = (yyvsp[-1].ml); (yyval.ml)->add((yyvsp[0].perm).val); }
#line 2045 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 232 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = (yyvsp[0].vd); }
#line 2051 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 233 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = (yyvsp[0].d); }
#line 2057 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 234 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = (yyvsp[0].d); }
#line 2063 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 236 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new accessdec((yyvsp[-2].pos), (yyvsp[-1].ipl)); }
#line 2069 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 238 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), (yyvsp[-1].ipl)); }
#line 2075 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 240 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new unraveldec((yyvsp[-4].pos), (yyvsp[-3].n), WILDCARD); }
#line 2081 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 241 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new unraveldec((yyvsp[-2].pos), (yyvsp[-1].n), WILDCARD); }
#line 2087 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 243 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, (yyvsp[-1].ipl)); }
#line 2093 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 245 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new fromaccessdec((yyvsp[-4].pos), (yyvsp[-3].ps).sym, WILDCARD); }
#line 2099 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 247 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new importdec((yyvsp[-2].pos), (yyvsp[-1].ip)); }
#line 2105 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 248 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].ps).sym); }
#line 2111 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 250 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new includedec((yyvsp[-2].pos), (yyvsp[-1].stre)->getString()); }
#line 2117 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 254 "camp.y" /* yacc.c:1646  */
    { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2123 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 256 "camp.y" /* yacc.c:1646  */
    { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2129 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 260 "camp.y" /* yacc.c:1646  */
    { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2135 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 262 "camp.y" /* yacc.c:1646  */
    { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2141 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 266 "camp.y" /* yacc.c:1646  */
    { (yyval.ps) = (yyvsp[0].ps); }
#line 2147 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 267 "camp.y" /* yacc.c:1646  */
    { (yyval.ps).pos = (yyvsp[0].stre)->getPos();
                     (yyval.ps).sym = symbol::literalTrans((yyvsp[0].stre)->getString()); }
#line 2154 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 272 "camp.y" /* yacc.c:1646  */
    { (yyval.ip) = new idpair((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2160 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 274 "camp.y" /* yacc.c:1646  */
    { (yyval.ip) = new idpair((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].ps).sym , (yyvsp[0].ps).sym); }
#line 2166 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 278 "camp.y" /* yacc.c:1646  */
    { (yyval.ipl) = new idpairlist(); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2172 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 280 "camp.y" /* yacc.c:1646  */
    { (yyval.ipl) = (yyvsp[-2].ipl); (yyval.ipl)->add((yyvsp[0].ip)); }
#line 2178 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 284 "camp.y" /* yacc.c:1646  */
    { (yyval.vd) = (yyvsp[-1].vd); }
#line 2184 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 288 "camp.y" /* yacc.c:1646  */
    { (yyval.vd) = new vardec((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dil)); }
#line 2190 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 292 "camp.y" /* yacc.c:1646  */
    { (yyval.t) = (yyvsp[0].t); }
#line 2196 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 293 "camp.y" /* yacc.c:1646  */
    { (yyval.t) = new arrayTy((yyvsp[-1].n), (yyvsp[0].dim)); }
#line 2202 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 297 "camp.y" /* yacc.c:1646  */
    { (yyval.t) = new nameTy((yyvsp[0].n)); }
#line 2208 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 301 "camp.y" /* yacc.c:1646  */
    { (yyval.dim) = new dimensions((yyvsp[-1].pos)); }
#line 2214 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 302 "camp.y" /* yacc.c:1646  */
    { (yyval.dim) = (yyvsp[-2].dim); (yyval.dim)->increase(); }
#line 2220 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 306 "camp.y" /* yacc.c:1646  */
    { (yyval.elist) = new explist((yyvsp[-2].pos)); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2226 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 308 "camp.y" /* yacc.c:1646  */
    { (yyval.elist) = (yyvsp[-3].elist); (yyval.elist)->add((yyvsp[-1].e)); }
#line 2232 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 312 "camp.y" /* yacc.c:1646  */
    { (yyval.dil) = new decidlist((yyvsp[0].di)->getPos()); (yyval.dil)->add((yyvsp[0].di)); }
#line 2238 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 314 "camp.y" /* yacc.c:1646  */
    { (yyval.dil) = (yyvsp[-2].dil); (yyval.dil)->add((yyvsp[0].di)); }
#line 2244 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 318 "camp.y" /* yacc.c:1646  */
    { (yyval.di) = new decid((yyvsp[0].dis)->getPos(), (yyvsp[0].dis)); }
#line 2250 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 320 "camp.y" /* yacc.c:1646  */
    { (yyval.di) = new decid((yyvsp[-2].dis)->getPos(), (yyvsp[-2].dis), (yyvsp[0].vi)); }
#line 2256 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 324 "camp.y" /* yacc.c:1646  */
    { (yyval.dis) = new decidstart((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2262 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 325 "camp.y" /* yacc.c:1646  */
    { (yyval.dis) = new decidstart((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym, (yyvsp[0].dim)); }
#line 2268 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 326 "camp.y" /* yacc.c:1646  */
    { (yyval.dis) = new fundecidstart((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, 0,
                                            new formals((yyvsp[-1].pos))); }
#line 2275 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 329 "camp.y" /* yacc.c:1646  */
    { (yyval.dis) = new fundecidstart((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym, 0, (yyvsp[-1].fls)); }
#line 2281 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 333 "camp.y" /* yacc.c:1646  */
    { (yyval.vi) = (yyvsp[0].e); }
#line 2287 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 334 "camp.y" /* yacc.c:1646  */
    { (yyval.vi) = (yyvsp[0].ai); }
#line 2293 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 339 "camp.y" /* yacc.c:1646  */
    { (yyval.b) = (yyvsp[-1].b); }
#line 2299 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 343 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = new arrayinit((yyvsp[-1].pos)); }
#line 2305 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 345 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = new arrayinit((yyvsp[-3].pos)); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2311 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 347 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = (yyvsp[-1].ai); }
#line 2317 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 349 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = (yyvsp[-3].ai); (yyval.ai)->addRest((yyvsp[-1].vi)); }
#line 2323 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 353 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = new arrayinit((yyvsp[0].pos)); }
#line 2329 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 354 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = (yyvsp[0].ai); }
#line 2335 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 355 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = (yyvsp[-1].ai); }
#line 2341 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 359 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = new arrayinit((yyvsp[0].vi)->getPos());
		     (yyval.ai)->add((yyvsp[0].vi));}
#line 2348 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 362 "camp.y" /* yacc.c:1646  */
    { (yyval.ai) = (yyvsp[-2].ai); (yyval.ai)->add((yyvsp[0].vi)); }
#line 2354 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 366 "camp.y" /* yacc.c:1646  */
    { (yyval.fls) = new formals((yyvsp[0].fl)->getPos()); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2360 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 367 "camp.y" /* yacc.c:1646  */
    { (yyval.fls) = new formals((yyvsp[-1].pos)); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2366 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 369 "camp.y" /* yacc.c:1646  */
    { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->add((yyvsp[0].fl)); }
#line 2372 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 371 "camp.y" /* yacc.c:1646  */
    { (yyval.fls) = (yyvsp[-2].fls); (yyval.fls)->addRest((yyvsp[0].fl)); }
#line 2378 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 375 "camp.y" /* yacc.c:1646  */
    { (yyval.boo) = true; }
#line 2384 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 376 "camp.y" /* yacc.c:1646  */
    { (yyval.boo) = false; }
#line 2390 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 381 "camp.y" /* yacc.c:1646  */
    { (yyval.fl) = new formal((yyvsp[0].t)->getPos(), (yyvsp[0].t), 0, 0, (yyvsp[-1].boo), 0); }
#line 2396 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 383 "camp.y" /* yacc.c:1646  */
    { (yyval.fl) = new formal((yyvsp[-1].t)->getPos(), (yyvsp[-1].t), (yyvsp[0].dis), 0, (yyvsp[-2].boo), 0); }
#line 2402 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 385 "camp.y" /* yacc.c:1646  */
    { (yyval.fl) = new formal((yyvsp[-3].t)->getPos(), (yyvsp[-3].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-4].boo), 0); }
#line 2408 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 388 "camp.y" /* yacc.c:1646  */
    { bool k = checkKeyword((yyvsp[-1].ps).pos, (yyvsp[-1].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-2].t)->getPos(), (yyvsp[-2].t), (yyvsp[0].dis), 0, (yyvsp[-3].boo), k); }
#line 2415 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 391 "camp.y" /* yacc.c:1646  */
    { bool k = checkKeyword((yyvsp[-3].ps).pos, (yyvsp[-3].ps).sym);
                     (yyval.fl) = new formal((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-2].dis), (yyvsp[0].vi), (yyvsp[-5].boo), k); }
#line 2422 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 397 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new fundec((yyvsp[-2].pos), (yyvsp[-4].t), (yyvsp[-3].ps).sym, new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2428 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 399 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new fundec((yyvsp[-3].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2434 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 403 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new recorddec((yyvsp[-2].pos), (yyvsp[-1].ps).sym, (yyvsp[0].b)); }
#line 2440 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 404 "camp.y" /* yacc.c:1646  */
    { (yyval.d) = new typedec((yyvsp[-1].pos), (yyvsp[0].vd)); }
#line 2446 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 408 "camp.y" /* yacc.c:1646  */
    { (yyval.slice) = new slice((yyvsp[0].pos), 0, 0); }
#line 2452 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 409 "camp.y" /* yacc.c:1646  */
    { (yyval.slice) = new slice((yyvsp[0].pos), (yyvsp[-1].e), 0); }
#line 2458 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 410 "camp.y" /* yacc.c:1646  */
    { (yyval.slice) = new slice((yyvsp[-1].pos), 0, (yyvsp[0].e)); }
#line 2464 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 411 "camp.y" /* yacc.c:1646  */
    { (yyval.slice) = new slice((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2470 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 415 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new fieldExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].ps).sym); }
#line 2476 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 416 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new subscriptExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].e)); }
#line 2483 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 418 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new subscriptExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e)); }
#line 2489 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 419 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new sliceExp((yyvsp[-2].pos),
                              new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)), (yyvsp[-1].slice)); }
#line 2496 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 421 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new sliceExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].slice)); }
#line 2502 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 422 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new callExp((yyvsp[-1].pos),
                                      new nameExp((yyvsp[-2].n)->getPos(), (yyvsp[-2].n)),
                                      new arglist()); }
#line 2510 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 426 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new callExp((yyvsp[-2].pos), 
                                      new nameExp((yyvsp[-3].n)->getPos(), (yyvsp[-3].n)),
                                      (yyvsp[-1].alist)); }
#line 2518 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 429 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new callExp((yyvsp[-1].pos), (yyvsp[-2].e), new arglist()); }
#line 2524 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 431 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new callExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].alist)); }
#line 2530 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 433 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = (yyvsp[-1].e); }
#line 2536 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 435 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new nameExp((yyvsp[-1].n)->getPos(), (yyvsp[-1].n)); }
#line 2542 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 436 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new thisExp((yyvsp[0].pos)); }
#line 2548 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 440 "camp.y" /* yacc.c:1646  */
    { (yyval.arg).name = symbol::nullsym; (yyval.arg).val=(yyvsp[0].e); }
#line 2554 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 441 "camp.y" /* yacc.c:1646  */
    { (yyval.arg).name = (yyvsp[-2].ps).sym; (yyval.arg).val=(yyvsp[0].e); }
#line 2560 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 445 "camp.y" /* yacc.c:1646  */
    { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2566 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 447 "camp.y" /* yacc.c:1646  */
    { (yyval.alist) = new arglist(); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2572 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 449 "camp.y" /* yacc.c:1646  */
    { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].arg)); }
#line 2578 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 451 "camp.y" /* yacc.c:1646  */
    { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->addRest((yyvsp[0].arg)); }
#line 2584 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 456 "camp.y" /* yacc.c:1646  */
    { (yyval.alist) = new arglist(); (yyval.alist)->add((yyvsp[-2].e)); (yyval.alist)->add((yyvsp[0].e)); }
#line 2590 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 457 "camp.y" /* yacc.c:1646  */
    { (yyval.alist) = (yyvsp[-2].alist); (yyval.alist)->add((yyvsp[0].e)); }
#line 2596 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 461 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new nameExp((yyvsp[0].n)->getPos(), (yyvsp[0].n)); }
#line 2602 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 462 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = (yyvsp[0].e); }
#line 2608 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 463 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = (yyvsp[0].e); }
#line 2614 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 464 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = (yyvsp[0].stre); }
#line 2620 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 466 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new scaleExp((yyvsp[-1].e)->getPos(), (yyvsp[-1].e), (yyvsp[0].e)); }
#line 2626 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 468 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new castExp((yyvsp[-2].n)->getPos(), new nameTy((yyvsp[-2].n)), (yyvsp[0].e)); }
#line 2632 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 470 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new castExp((yyvsp[-3].n)->getPos(), new arrayTy((yyvsp[-3].n), (yyvsp[-2].dim)), (yyvsp[0].e)); }
#line 2638 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 472 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2644 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 474 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2650 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 475 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 2656 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 476 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2662 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 477 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2668 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 478 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2674 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 479 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2680 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 480 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2686 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 481 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2692 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 482 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2698 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 483 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2704 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 484 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2710 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 485 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2716 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 486 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2722 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 487 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2728 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 488 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new equalityExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2734 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 489 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new andExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2740 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 490 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new orExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2746 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 491 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2752 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 492 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2758 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 493 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2764 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 494 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2770 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 495 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2776 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 497 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newRecordExp((yyvsp[-1].pos), (yyvsp[0].t)); }
#line 2782 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 499 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), (yyvsp[0].elist), 0, 0); }
#line 2788 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 501 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), (yyvsp[-1].elist), (yyvsp[0].dim), 0); }
#line 2794 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 503 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newArrayExp((yyvsp[-2].pos), (yyvsp[-1].t), 0, (yyvsp[0].dim), 0); }
#line 2800 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 505 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newArrayExp((yyvsp[-3].pos), (yyvsp[-2].t), 0, (yyvsp[-1].dim), (yyvsp[0].ai)); }
#line 2806 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 507 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newFunctionExp((yyvsp[-4].pos), (yyvsp[-3].t), new formals((yyvsp[-2].pos)), (yyvsp[0].s)); }
#line 2812 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 509 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newFunctionExp((yyvsp[-5].pos),
                                             new arrayTy((yyvsp[-4].t)->getPos(), (yyvsp[-4].t), (yyvsp[-3].dim)),
                                             new formals((yyvsp[-2].pos)),
                                             (yyvsp[0].s)); }
#line 2821 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 514 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newFunctionExp((yyvsp[-5].pos), (yyvsp[-4].t), (yyvsp[-2].fls), (yyvsp[0].s)); }
#line 2827 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 516 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new newFunctionExp((yyvsp[-6].pos),
                                             new arrayTy((yyvsp[-5].t)->getPos(), (yyvsp[-5].t), (yyvsp[-4].dim)),
                                             (yyvsp[-2].fls),
                                             (yyvsp[0].s)); }
#line 2836 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 521 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new conditionalExp((yyvsp[-3].pos), (yyvsp[-4].e), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2842 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 522 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new assignExp((yyvsp[-1].pos), (yyvsp[-2].e), (yyvsp[0].e)); }
#line 2848 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 523 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new callExp((yyvsp[-2].pos), new nameExp((yyvsp[-2].pos), SYM_TUPLE), (yyvsp[-1].alist)); }
#line 2854 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 525 "camp.y" /* yacc.c:1646  */
    { (yyvsp[-1].j)->pushFront((yyvsp[-2].e)); (yyvsp[-1].j)->pushBack((yyvsp[0].e)); (yyval.e) = (yyvsp[-1].j); }
#line 2860 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 527 "camp.y" /* yacc.c:1646  */
    { (yyvsp[0].se)->setSide(camp::OUT);
                     joinExp *jexp =
                         new joinExp((yyvsp[0].se)->getPos(), SYM_DOTS);
                     (yyval.e)=jexp;
                     jexp->pushBack((yyvsp[-1].e)); jexp->pushBack((yyvsp[0].se)); }
#line 2870 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 533 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_PLUS); }
#line 2876 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 535 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new prefixExp((yyvsp[-1].ps).pos, (yyvsp[0].e), SYM_MINUS); }
#line 2882 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 538 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new postfixExp((yyvsp[0].ps).pos, (yyvsp[-1].e), SYM_PLUS); }
#line 2888 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 539 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new selfExp((yyvsp[-1].ps).pos, (yyvsp[-2].e), (yyvsp[-1].ps).sym, (yyvsp[0].e)); }
#line 2894 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 541 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new quoteExp((yyvsp[-3].pos), (yyvsp[-1].b)); }
#line 2900 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 547 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = new joinExp((yyvsp[0].ps).pos,(yyvsp[0].ps).sym); }
#line 2906 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 549 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = (yyvsp[0].j); }
#line 2912 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 551 "camp.y" /* yacc.c:1646  */
    { (yyvsp[-1].se)->setSide(camp::OUT);
                     (yyval.j) = (yyvsp[0].j); (yyval.j)->pushFront((yyvsp[-1].se)); }
#line 2919 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 554 "camp.y" /* yacc.c:1646  */
    { (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2926 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 557 "camp.y" /* yacc.c:1646  */
    { (yyvsp[-2].se)->setSide(camp::OUT); (yyvsp[0].se)->setSide(camp::IN);
                     (yyval.j) = (yyvsp[-1].j); (yyval.j)->pushFront((yyvsp[-2].se)); (yyval.j)->pushBack((yyvsp[0].se)); }
#line 2933 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 562 "camp.y" /* yacc.c:1646  */
    { (yyval.se) = new specExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym, (yyvsp[-1].e)); }
#line 2939 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 563 "camp.y" /* yacc.c:1646  */
    { (yyval.se) = new specExp((yyvsp[-2].pos), symbol::opTrans("spec"), (yyvsp[-1].e)); }
#line 2945 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 565 "camp.y" /* yacc.c:1646  */
    { (yyval.se) = new specExp((yyvsp[-4].pos), symbol::opTrans("spec"),
				      new pairExp((yyvsp[-2].pos), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2952 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 568 "camp.y" /* yacc.c:1646  */
    { (yyval.se) = new specExp((yyvsp[-6].pos), symbol::opTrans("spec"),
				      new tripleExp((yyvsp[-4].pos), (yyvsp[-5].e), (yyvsp[-3].e), (yyvsp[-1].e))); }
#line 2959 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 573 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2965 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 575 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2971 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 577 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = new joinExp((yyvsp[-2].ps).pos, (yyvsp[-2].ps).sym); (yyval.j)->pushBack((yyvsp[-1].e)); }
#line 2977 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 578 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2983 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 579 "camp.y" /* yacc.c:1646  */
    { (yyval.j) = new joinExp((yyvsp[0].ps).pos, (yyvsp[0].ps).sym); }
#line 2989 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 583 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, false)); }
#line 2996 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 586 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new ternaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, false)); }
#line 3003 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 589 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-2].ps).pos, (yyvsp[0].e), (yyvsp[-2].ps).sym,
                              new booleanExp((yyvsp[-1].ps).pos, true)); }
#line 3010 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 592 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new ternaryExp((yyvsp[-4].ps).pos, (yyvsp[-2].e), (yyvsp[-4].ps).sym, (yyvsp[0].e),
                              new booleanExp((yyvsp[-3].ps).pos, true)); }
#line 3017 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 597 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new unaryExp((yyvsp[-1].ps).pos, (yyvsp[0].e), (yyvsp[-1].ps).sym); }
#line 3023 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 599 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = new binaryExp((yyvsp[-3].ps).pos, (yyvsp[-2].e), (yyvsp[-3].ps).sym, (yyvsp[0].e)); }
#line 3029 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 603 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new emptyStm((yyvsp[0].pos)); }
#line 3035 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 604 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = (yyvsp[0].s); }
#line 3041 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 605 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = (yyvsp[-1].s); }
#line 3047 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 607 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new ifStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3053 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 609 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new ifStm((yyvsp[-6].pos), (yyvsp[-4].e), (yyvsp[-2].s), (yyvsp[0].s)); }
#line 3059 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 611 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new whileStm((yyvsp[-4].pos), (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3065 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 613 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new doStm((yyvsp[-6].pos), (yyvsp[-5].s), (yyvsp[-2].e)); }
#line 3071 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 615 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new forStm((yyvsp[-8].pos), (yyvsp[-6].run), (yyvsp[-4].e), (yyvsp[-2].sel), (yyvsp[0].s)); }
#line 3077 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 617 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new extendedForStm((yyvsp[-7].pos), (yyvsp[-5].t), (yyvsp[-4].ps).sym, (yyvsp[-2].e), (yyvsp[0].s)); }
#line 3083 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 618 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new breakStm((yyvsp[-1].pos)); }
#line 3089 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 619 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new continueStm((yyvsp[-1].pos)); }
#line 3095 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 620 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new returnStm((yyvsp[-1].pos)); }
#line 3101 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 621 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new returnStm((yyvsp[-2].pos), (yyvsp[-1].e)); }
#line 3107 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 625 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new expStm((yyvsp[0].e)->getPos(), (yyvsp[0].e)); }
#line 3113 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 629 "camp.y" /* yacc.c:1646  */
    { (yyval.s) = new blockStm((yyvsp[0].b)->getPos(), (yyvsp[0].b)); }
#line 3119 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 633 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = 0; }
#line 3125 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 634 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = (yyvsp[0].sel); }
#line 3131 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 635 "camp.y" /* yacc.c:1646  */
    { (yyval.run) = (yyvsp[0].vd); }
#line 3137 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 639 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = 0; }
#line 3143 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 640 "camp.y" /* yacc.c:1646  */
    { (yyval.e) = (yyvsp[0].e); }
#line 3149 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 644 "camp.y" /* yacc.c:1646  */
    { (yyval.sel) = 0; }
#line 3155 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 645 "camp.y" /* yacc.c:1646  */
    { (yyval.sel) = (yyvsp[0].sel); }
#line 3161 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 649 "camp.y" /* yacc.c:1646  */
    { (yyval.sel) = new stmExpList((yyvsp[0].s)->getPos()); (yyval.sel)->add((yyvsp[0].s)); }
#line 3167 "camp.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 651 "camp.y" /* yacc.c:1646  */
    { (yyval.sel) = (yyvsp[-2].sel); (yyval.sel)->add((yyvsp[0].s)); }
#line 3173 "camp.tab.c" /* yacc.c:1646  */
    break;


#line 3177 "camp.tab.c" /* yacc.c:1646  */
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

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
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
