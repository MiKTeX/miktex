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
#line 21 "gram.y" /* yacc.c:339  */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>

#include "common.h"
#include "output.h"

unsigned block_level;
symbol_t * prog_symbol;

namespace {
  pascal_type last_type;
  void * last_type_ptr;
  value_t last_value;
  array_node * last_array_node;
  std::stack<array_node *> array_type_stack;
  std::stack<parameter_node *> parameter_node_stack;
  declarator_node * last_declarator;
  parameter_node * last_parameter;
  bool by_reference;
  record_section_node * last_record_section;
  variant_node * last_variant;
  std::string file_spec;
  bool file_specified;
  pascal_type file_type;
  size_t buf_mark;
  size_t buf_mark2;
  bool to_eoln;
  unsigned nglobalvars;
  unsigned routine_handle;
}

#define push_array_type(arr)                    \
  array_type_stack.push (last_array_node);      \
  last_array_node = arr

#define pop_array_type()                        \
  last_array_node = array_type_stack.top();     \
  array_type_stack.pop ()

#define push_parameter_node(par)                \
  parameter_node_stack.push (par)

#define pop_parameter_node(par)                 \
  par = parameter_node_stack.top();             \
  parameter_node_stack.pop ()

#define get_array_offset(arr)  arr->lower_bound


#line 119 "gram.tab.c" /* yacc.c:339  */

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
   by #include "gram.h".  */
#ifndef YY_YY_C_WORK3_MIKTEX_NMAKE_AMD64_BUILDUTILITIES_C4P_GRAM_H_INCLUDED
# define YY_YY_C_WORK3_MIKTEX_NMAKE_AMD64_BUILDUTILITIES_C4P_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    AND = 258,
    ARRAY = 259,
    BEGIn = 260,
    CASE = 261,
    CONST = 262,
    DIV = 263,
    DO = 264,
    DOWNTO = 265,
    ELSE = 266,
    END = 267,
    FILe = 268,
    FOR = 269,
    FORWARD = 270,
    FUNCTION = 271,
    GOTO = 272,
    IF = 273,
    IN = 274,
    LABEL = 275,
    MOD = 276,
    NIL = 277,
    NOT = 278,
    OF = 279,
    OR = 280,
    PACKED = 281,
    PROCEDURE = 282,
    PROGRAM = 283,
    RECORD = 284,
    REPEAT = 285,
    SET = 286,
    THEN = 287,
    TO = 288,
    TYPE = 289,
    UNTIL = 290,
    VAR = 291,
    WHILE = 292,
    WITH = 293,
    OTHERS = 294,
    DOTDOT = 295,
    NEQ = 296,
    GEQ = 297,
    LEQ = 298,
    ASSIGN = 299,
    PASCAL_KEYWORD = 300,
    TYPE_IDENTIFIER = 301,
    FIELD_IDENTIFIER = 302,
    C_RESERVED = 303,
    CONSTANT_IDENTIFIER = 304,
    FUNCTION_IDENTIFIER = 305,
    PROCEDURE_IDENTIFIER = 306,
    UNDEFINED_IDENTIFIER = 307,
    VARIABLE_IDENTIFIER = 308,
    PARAMETER_IDENTIFIER = 309,
    READ = 310,
    READLN = 311,
    WRITE = 312,
    WRITELN = 313,
    BUILD_IN_IDENTIFIER = 314,
    CHARACTER_STRING = 315,
    CHARACTER_CONSTANT = 316,
    UNSIGNED_NUMBER = 317,
    REAL_CONSTANT = 318,
    LONG_REAL_CONSTANT = 319,
    IDENTIFIER = 320
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 74 "gram.y" /* yacc.c:355  */

  symbol_t * sym;
  C4P_integer value;
  pascal_type type;
  C4P_real fvalue;
  void * type_ptr;
  unsigned buf_mark;

#line 234 "gram.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_C_WORK3_MIKTEX_NMAKE_AMD64_BUILDUTILITIES_C4P_GRAM_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 251 "gram.tab.c" /* yacc.c:358  */

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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   328

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  82
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  186
/* YYNRULES -- Number of rules.  */
#define YYNRULES  297
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  445

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   320

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      68,    69,    81,    73,    70,    74,    67,    80,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    66,
      78,    72,    79,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    75,     2,    76,    77,     2,     2,     2,     2,     2,
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
      65
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   125,   125,   129,   128,   141,   143,   147,   151,   158,
     166,   179,   147,   225,   226,   230,   235,   237,   241,   242,
     246,   247,   251,   255,   267,   254,   287,   295,   323,   327,
     326,   338,   342,   351,   341,   362,   384,   386,   385,   397,
     397,   405,   404,   419,   444,   446,   445,   457,   464,   466,
     471,   474,   477,   479,   484,   485,   490,   489,   523,   529,
     535,   541,   547,   553,   560,   566,   572,   578,   588,   598,
     609,   617,   628,   629,   633,   637,   645,   653,   661,   664,
     666,   670,   671,   676,   675,   687,   696,   700,   701,   702,
     706,   713,   721,   722,   729,   730,   731,   738,   747,   746,
     760,   768,   772,   774,   773,   786,   791,   832,   840,   847,
     851,   853,   852,   865,   868,   876,   878,   877,   889,   898,
     901,   908,   915,   921,   923,   922,   935,   939,   943,   939,
     954,   960,   962,   966,   967,   971,   996,   998,   997,  1009,
    1017,  1022,  1021,  1046,  1047,  1052,  1051,  1056,  1059,  1061,
    1065,  1077,  1080,  1081,  1082,  1087,  1086,  1092,  1091,  1097,
    1096,  1102,  1101,  1107,  1106,  1132,  1135,  1140,  1139,  1169,
    1171,  1170,  1180,  1221,  1224,  1229,  1228,  1258,  1260,  1259,
    1269,  1308,  1307,  1336,  1340,  1339,  1350,  1351,  1352,  1359,
    1360,  1366,  1370,  1365,  1379,  1383,  1382,  1396,  1400,  1395,
    1411,  1413,  1417,  1423,  1424,  1429,  1428,  1440,  1441,  1445,
    1445,  1453,  1460,  1461,  1462,  1467,  1471,  1466,  1483,  1488,
    1482,  1501,  1514,  1518,  1500,  1539,  1545,  1551,  1555,  1563,
    1574,  1562,  1587,  1594,  1598,  1599,  1603,  1607,  1620,  1624,
    1663,  1664,  1669,  1668,  1695,  1699,  1706,  1710,  1720,  1719,
    1733,  1735,  1734,  1764,  1768,  1772,  1776,  1780,  1784,  1788,
    1795,  1797,  1796,  1808,  1813,  1812,  1840,  1839,  1847,  1851,
    1858,  1860,  1859,  1869,  1868,  1896,  1895,  1903,  1907,  1911,
    1918,  1923,  1928,  1933,  1940,  1947,  1967,  1966,  2007,  2006,
    2016,  2015,  2025,  2027,  2031,  2033,  2032,  2040
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "AND", "ARRAY", "BEGIn", "CASE", "CONST",
  "DIV", "DO", "DOWNTO", "ELSE", "END", "FILe", "FOR", "FORWARD",
  "FUNCTION", "GOTO", "IF", "IN", "LABEL", "MOD", "NIL", "NOT", "OF", "OR",
  "PACKED", "PROCEDURE", "PROGRAM", "RECORD", "REPEAT", "SET", "THEN",
  "TO", "TYPE", "UNTIL", "VAR", "WHILE", "WITH", "OTHERS", "DOTDOT", "NEQ",
  "GEQ", "LEQ", "ASSIGN", "PASCAL_KEYWORD", "TYPE_IDENTIFIER",
  "FIELD_IDENTIFIER", "C_RESERVED", "CONSTANT_IDENTIFIER",
  "FUNCTION_IDENTIFIER", "PROCEDURE_IDENTIFIER", "UNDEFINED_IDENTIFIER",
  "VARIABLE_IDENTIFIER", "PARAMETER_IDENTIFIER", "READ", "READLN", "WRITE",
  "WRITELN", "BUILD_IN_IDENTIFIER", "CHARACTER_STRING",
  "CHARACTER_CONSTANT", "UNSIGNED_NUMBER", "REAL_CONSTANT",
  "LONG_REAL_CONSTANT", "IDENTIFIER", "';'", "'.'", "'('", "')'", "','",
  "':'", "'='", "'+'", "'-'", "'['", "']'", "'^'", "'<'", "'>'", "'/'",
  "'*'", "$accept", "program", "program_heading", "$@1",
  "optional_program_parameters", "program_block", "$@2", "$@3", "$@4",
  "$@5", "$@6", "program_parameter_list", "program_parameter",
  "procedure_and_function_declaration_part",
  "procedure_and_function_declarations",
  "procedure_or_function_declaration", "function_declaration", "$@7",
  "$@8", "function_head", "optional_formal_parameter_section_list", "$@9",
  "procedure_declaration", "$@10", "$@11", "procedure_head",
  "formal_parameter_section_list", "@12", "formal_parameter_section",
  "$@13", "$@14", "parameter_group", "parameter_declarator_list", "@15",
  "parameter_declarator", "label_declaration_part", "label_list",
  "constant_definition_part", "constant_definitions",
  "constant_definition", "@16", "constant", "bound",
  "type_definition_part", "type_definitions", "type_definition", "$@17",
  "type_denoter", "type", "ordinal_type", "subrange_type",
  "structured_type", "unpacked_structured_type", "file_type", "array_type",
  "@18", "pointer_type", "component_type", "index_type_list", "@19",
  "index_type", "record_type", "field_list", "fixed_part",
  "record_section_list", "@20", "record_section", "field_declarator_list",
  "@21", "field_declarator", "variant_part", "variant_selector",
  "tag_type", "variant_list", "@22", "variant", "$@23", "$@24",
  "variant_field_list", "variable_declaration_part",
  "variable_declarations", "variable_declaration",
  "variable_declarator_list", "@25", "variable_declarator",
  "statement_part", "compound_statement", "$@26", "statement_sequence",
  "statement", "$@27", "optional_label", "label", "simple_statement",
  "procedure_statement", "$@28", "$@29", "$@30", "$@31", "$@32",
  "readln_parameter_list", "read_parameter_list", "$@33",
  "read_parameter_list1", "$@34", "read_parameter",
  "writeln_parameter_list", "write_parameter_list", "$@35",
  "write_parameter_list1", "$@36", "write_expression", "$@37",
  "output_width", "$@38", "structured_statement", "conditional_statement",
  "if_statement", "$@39", "$@40", "else_part", "$@41", "case_statement",
  "$@42", "$@43", "optional_semicolon", "case_index",
  "case_list_element_sequence", "case_list_element", "$@44",
  "case_constant_list", "case_constant", "$@45", "repetitive_statement",
  "while_statement", "$@46", "$@47", "repeat_statement", "$@48", "$@49",
  "for_statement", "$@50", "$@51", "$@52", "initial_value", "final_value",
  "to_or_downto", "assignment_statement", "$@53", "$@54", "goto_statement",
  "assign", "variable_access", "buffer_variable", "entire_variable",
  "variable_identifier", "component_variable", "indexed_variable", "@55",
  "array_variable", "field_designator", "record_variable",
  "index_expression_list", "$@56", "expression", "@57",
  "boolean_expression", "relational_operator", "simple_expression", "$@58",
  "@59", "$@60", "adding_operator", "term", "$@61", "@62", "$@63",
  "multiplying_operator", "factor", "$@64", "$@65", "$@66",
  "optional_actual_parameter_list", "actual_parameter_list", "$@67",
  "actual_parameter", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,    59,    46,    40,    41,
      44,    58,    61,    43,    45,    91,    93,    94,    60,    62,
      47,    42
};
# endif

#define YYPACT_NINF -279

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-279)))

#define YYTABLE_NINF -247

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      35,    -9,    73,    27,  -279,  -279,  -279,    29,    38,    95,
      55,  -279,  -279,    62,  -279,  -279,   -48,  -279,  -279,   -21,
     124,  -279,    55,  -279,    66,    76,  -279,  -279,  -279,    77,
      76,  -279,   129,  -279,  -279,   108,   145,   148,   112,   108,
    -279,   128,  -279,  -279,  -279,  -279,  -279,  -279,  -279,   152,
     169,   122,  -279,  -279,  -279,   128,  -279,    85,  -279,     9,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,     5,
    -279,  -279,     5,   135,   140,  -279,     9,  -279,  -279,   125,
    -279,   132,   151,   217,   121,   177,  -279,  -279,   206,   -27,
      18,     5,   207,   182,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,   128,   183,    -3,   184,   245,  -279,   236,
     238,   -32,     5,  -279,  -279,   242,   249,   190,  -279,    88,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,   113,  -279,  -279,
    -279,  -279,   186,  -279,  -279,  -279,  -279,   192,    95,   193,
      95,   206,   220,   -26,  -279,  -279,  -279,   196,  -279,  -279,
    -279,     5,  -279,  -279,   226,   198,   202,  -279,   124,  -279,
     124,   113,  -279,   241,  -279,   243,  -279,   177,   177,  -279,
    -279,    60,  -279,   201,  -279,   197,    -2,  -279,   162,  -279,
     129,   129,  -279,   -32,  -279,     3,  -279,  -279,   201,  -279,
    -279,  -279,  -279,   126,  -279,  -279,  -279,   202,  -279,   204,
    -279,  -279,  -279,  -279,    -4,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,   145,   145,  -279,     5,   205,  -279,   231,
    -279,   226,  -279,   208,  -279,    48,   228,    48,   202,    48,
     212,  -279,  -279,  -279,  -279,   -20,  -279,  -279,  -279,  -279,
      94,  -279,  -279,  -279,  -279,  -279,   200,  -279,   209,   272,
     273,  -279,  -279,  -279,   133,  -279,   148,  -279,   201,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,   211,  -279,    83,  -279,
     256,    94,   123,    12,    11,  -279,  -279,   123,   250,   -18,
      17,  -279,   213,   213,   215,   215,   216,  -279,   244,  -279,
     221,  -279,  -279,     3,   231,   219,  -279,  -279,    83,  -279,
      48,    11,    83,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,    48,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,    48,  -279,  -279,  -279,    48,  -279,   202,
     202,  -279,  -279,  -279,  -279,    48,    58,    11,   231,    48,
      83,    83,    83,    83,    83,    13,   123,   202,    48,   202,
     224,    48,   123,   149,  -279,    48,    47,   123,    15,    28,
     177,   154,  -279,   225,  -279,   158,    12,    11,    11,  -279,
    -279,  -279,  -279,  -279,  -279,   274,   123,  -279,  -279,   166,
    -279,    94,   168,  -279,    61,  -279,  -279,   123,  -279,  -279,
     227,   229,   249,   223,  -279,     4,   278,  -279,    48,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,    48,    48,  -279,  -279,
    -279,  -279,  -279,  -279,   202,   285,   123,   202,   224,    48,
      48,  -279,   123,  -279,  -279,  -279,  -279,  -279,  -279,   111,
     202,  -279,  -279,    48,   123
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,     3,     1,     7,     5,     0,    48,
       0,     4,     2,     0,     8,    15,     0,    13,    50,     0,
      52,     6,     0,    49,     0,     0,     9,    14,    51,     0,
      53,    54,    79,    56,    55,     0,   131,     0,     0,    80,
      81,     0,    10,    70,    71,    64,    58,    61,    67,     0,
       0,     0,    83,    82,   139,   132,   133,     0,   136,    16,
      65,    59,    62,    68,    66,    60,    63,    69,    57,     0,
     134,   137,     0,     0,     0,    11,    17,    18,    21,     0,
      20,     0,     0,     0,     0,   113,    78,    72,    85,     0,
       0,     0,     0,     0,    86,    87,    90,    88,    92,    96,
      94,    89,    95,     0,     0,    26,    28,     0,    19,    23,
      32,     0,     0,    93,   118,     0,   119,   109,   110,     0,
     115,    73,    76,    74,    77,   101,   100,     0,    84,   138,
     135,    29,     0,    35,   141,    12,   140,     0,    48,     0,
      48,   106,     0,     0,   102,    97,   107,     0,   108,   111,
     116,     0,    75,    91,    39,     0,   148,    22,    52,    31,
      52,     0,   103,     0,   122,     0,   121,   113,     0,   114,
      41,     0,    36,     0,    27,     0,     0,   143,   145,   149,
      79,    79,   105,     0,    98,   127,   112,   117,     0,    37,
      30,    47,    40,     0,    44,   150,   142,   148,   197,     0,
     191,   218,   215,   186,   151,   147,   187,   189,   190,   188,
     213,   212,   214,   131,   131,   104,     0,   120,   123,   209,
      42,    39,    45,     0,   144,     0,     0,     0,   148,     0,
       0,   155,   157,   159,   161,   163,   146,   153,   152,   154,
     229,   236,   234,   238,   235,   240,     0,   241,     0,     0,
       0,    99,   124,   211,     0,   207,     0,    38,     0,    43,
     290,   283,   284,   280,   281,   282,   239,   288,     0,   261,
       0,   285,   202,   250,   260,   270,   221,   253,     0,     0,
       0,   232,     0,   165,     0,   173,   292,   237,     0,   242,
       0,    24,    33,   127,   209,     0,   210,    46,     0,   286,
       0,   263,     0,   198,   259,   257,   256,   258,   254,   255,
     251,   266,   269,   268,   264,   275,   278,   279,   271,   277,
     273,     0,   192,   219,   216,   167,   156,   158,   166,   175,
     160,   162,   174,     0,   164,   233,   230,     0,   245,   148,
     148,   125,   208,   128,   291,     0,     0,   262,   209,     0,
       0,     0,     0,     0,     0,     0,   225,   148,     0,   148,
       0,     0,   297,     0,   294,     0,     0,   247,     0,     0,
     113,     0,   289,   200,   203,     0,   252,   267,   265,   276,
     272,   274,   228,   227,   222,   194,   220,   217,   239,     0,
     169,   172,     0,   177,   180,   293,   295,   231,   248,   243,
       0,     0,   119,     0,   287,   209,     0,   205,     0,   195,
     193,   168,   170,   176,   178,   181,     0,     0,    25,    34,
     130,   129,   204,   199,   148,     0,   226,   148,     0,     0,
       0,   296,   249,   206,   223,   196,   171,   179,   182,   183,
     148,   184,   224,     0,   185
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,   275,  -279,  -279,   222,  -279,  -279,  -279,  -279,
     194,  -279,  -279,  -279,  -279,  -279,  -279,  -279,    75,  -279,
    -279,   114,  -279,  -279,    41,    32,  -279,    19,  -279,   271,
    -279,    49,   -99,    59,  -279,   264,  -279,   -61,  -279,  -279,
    -279,  -279,   230,  -279,  -279,  -279,  -279,  -109,  -279,  -279,
     127,  -279,  -279,   -66,  -279,  -279,   139,  -279,  -279,   141,
     -95,  -279,  -279,  -279,  -279,    20,  -279,  -279,  -279,    31,
    -279,   253,  -279,  -279,   214,  -279,   134,  -279,  -221,  -197,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,    33,  -279,  -279,  -279,  -117,  -279,    30,  -279,  -279,
    -279,  -111,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,   -86,  -279,
     101,    34,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -203,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -223,  -279,  -279,  -279,   -28,  -279,
    -279,  -279,  -279,  -263,  -279,  -279,  -279,  -279,  -278,  -279,
    -279,  -279,  -279,   -23,  -279,   -93
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     7,    11,     8,     9,    20,    32,    59,
     107,    16,    17,    75,    76,    77,    78,   138,   339,    79,
     132,   154,    80,   140,   340,    81,   171,   221,   172,   173,
     188,   192,   193,   258,   194,    14,    19,    26,    30,    31,
      37,    51,    92,    36,    39,    40,    69,   125,    94,    95,
      96,    97,    98,    99,   100,   216,   101,   126,   143,   183,
     144,   102,   115,   116,   117,   167,   118,   119,   168,   120,
     148,   165,   166,   217,   293,   218,   219,   370,   403,    42,
      55,    56,    57,   103,    58,   135,   136,   156,   176,   177,
     204,   178,   179,   236,   237,   282,   283,   284,   285,   286,
     327,   326,   360,   389,   428,   390,   331,   330,   361,   392,
     429,   393,   430,   438,   443,   205,   206,   207,   227,   357,
     410,   427,   208,   225,   348,   406,   270,   373,   374,   424,
     375,   255,   256,   209,   210,   229,   359,   211,   228,   358,
     212,   321,   408,   440,   355,   425,   384,   238,   288,   365,
     239,   336,   271,   241,   242,   243,   244,   245,   337,   246,
     247,   248,   366,   417,   362,   349,   278,   310,   273,   302,
     351,   350,   314,   274,   353,   354,   352,   320,   275,   345,
     300,   298,   334,   363,   416,   364
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     224,   240,   272,   145,   277,   301,   280,   279,    93,    82,
     196,   104,   142,   230,   315,  -126,  -201,   323,    83,   316,
     344,    21,    22,   382,  -239,    73,   324,   400,   153,    86,
      87,    84,   317,   141,    85,   121,    74,   311,   122,   347,
     401,    89,    90,   253,   162,    23,   383,  -239,   197,    24,
     163,   231,   232,   233,   234,  -239,     4,  -239,   304,   305,
     306,   235,   182,     1,   197,   131,    86,    87,   -28,  -126,
      88,   260,  -126,     5,   379,   380,   381,   346,    89,    90,
     123,   197,    91,   124,   142,   312,   313,   377,   378,   307,
     169,   318,   319,     6,   197,   308,   309,    10,   356,   304,
     305,   306,   304,   305,   306,    12,   260,   251,   261,   262,
     263,   264,   265,   266,   367,    13,   267,   398,   368,   369,
      15,   268,   269,   399,    18,    82,   189,   372,    28,   190,
     307,    25,   415,   307,    83,   386,   308,   309,   394,   308,
     309,    29,   397,   261,   262,   263,   264,   265,   266,    33,
      85,   267,   304,   305,   306,    71,    72,   391,   150,   151,
     385,  -246,   387,    35,   304,   305,   306,   134,   198,  -244,
     158,   287,   160,    38,    86,    87,   199,   180,   152,   181,
     200,    41,   441,   307,    52,   426,    89,    90,    68,   308,
     309,   109,   201,    54,   432,   307,   222,   223,   110,   202,
     105,   308,   309,   294,   295,   106,   394,   439,    43,    44,
      45,    46,    47,    48,    60,    61,    62,    63,   395,   396,
     444,    49,    50,   404,   396,   391,   111,   433,   294,   407,
     435,    64,    65,    66,    67,   411,   412,   413,   414,   213,
     214,   112,   114,   442,   249,   250,   -75,   127,   128,   130,
     134,   137,   131,   139,   146,   147,   149,   155,   157,   159,
     161,   164,   170,   174,   175,   184,   191,   185,   195,   226,
     253,   252,   276,   259,   281,   289,   290,   291,   292,   299,
     303,   325,   322,   329,   333,   409,   338,   343,   335,   388,
     423,   405,   421,   418,   434,   419,   257,    27,   108,   297,
     133,    34,   220,    53,   402,   296,   186,   420,    70,   187,
     215,   436,   203,   341,   113,   332,   328,   129,   437,   422,
     254,   376,   371,   431,     0,     0,     0,     0,   342
};

static const yytype_int16 yycheck[] =
{
     197,   204,   225,   112,   227,   268,   229,   228,    69,     4,
      12,    72,   111,    17,     3,    12,    12,    35,    13,     8,
     298,    69,    70,    10,    44,    16,     9,    12,   127,    61,
      62,    26,    21,    65,    29,    62,    27,    25,    65,   302,
      12,    73,    74,    39,    70,    66,    33,    67,    66,    70,
      76,    55,    56,    57,    58,    75,    65,    77,    41,    42,
      43,    65,   161,    28,    66,    68,    61,    62,    71,    66,
      65,    23,    69,     0,   352,   353,   354,   300,    73,    74,
      62,    66,    77,    65,   183,    73,    74,   350,   351,    72,
     151,    80,    81,    66,    66,    78,    79,    68,   321,    41,
      42,    43,    41,    42,    43,    67,    23,   216,    60,    61,
      62,    63,    64,    65,   337,    20,    68,    70,   339,   340,
      65,    73,    74,    76,    62,     4,    66,    69,    62,    69,
      72,     7,    71,    72,    13,   358,    78,    79,   361,    78,
      79,    65,   365,    60,    61,    62,    63,    64,    65,    72,
      29,    68,    41,    42,    43,    70,    71,   360,    70,    71,
     357,    67,   359,    34,    41,    42,    43,     5,     6,    75,
     138,    77,   140,    65,    61,    62,    14,   158,    65,   160,
      18,    36,    71,    72,    72,   408,    73,    74,    66,    78,
      79,    66,    30,    65,   417,    72,    70,    71,    66,    37,
      65,    78,    79,    70,    71,    65,   429,   430,    60,    61,
      62,    63,    64,    65,    62,    63,    64,    65,    69,    70,
     443,    73,    74,    69,    70,   428,    75,   424,    70,    71,
     427,    62,    63,    64,    65,    69,    70,    69,    70,   180,
     181,    24,    65,   440,   213,   214,    40,    40,    66,    66,
       5,    15,    68,    15,    12,     6,    66,    71,    66,    66,
      40,    65,    36,    65,    62,    24,    65,    24,    71,    65,
      39,    66,    44,    65,    62,    75,    67,     5,     5,    68,
      24,    68,    32,    68,    68,    11,    65,    68,    44,    65,
      12,    66,    69,    66,     9,    66,   221,    22,    76,   258,
     106,    30,   188,    39,   370,   256,   167,   402,    55,   168,
     183,   428,   178,   293,    84,   285,   283,   103,   429,   405,
     219,   349,   345,   416,    -1,    -1,    -1,    -1,   294
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    28,    83,    84,    65,     0,    66,    85,    87,    88,
      68,    86,    67,    20,   117,    65,    93,    94,    62,   118,
      89,    69,    70,    66,    70,     7,   119,    94,    62,    65,
     120,   121,    90,    72,   121,    34,   125,   122,    65,   126,
     127,    36,   161,    60,    61,    62,    63,    64,    65,    73,
      74,   123,    72,   127,    65,   162,   163,   164,   166,    91,
      62,    63,    64,    65,    62,    63,    64,    65,    66,   128,
     163,    70,    71,    16,    27,    95,    96,    97,    98,   101,
     104,   107,     4,    13,    26,    29,    61,    62,    65,    73,
      74,    77,   124,   129,   130,   131,   132,   133,   134,   135,
     136,   138,   143,   165,   129,    65,    65,    92,    97,    66,
      66,    75,    24,   134,    65,   144,   145,   146,   148,   149,
     151,    62,    65,    62,    65,   129,   139,    40,    66,   166,
      66,    68,   102,   102,     5,   167,   168,    15,    99,    15,
     105,    65,   124,   140,   142,   139,    12,     6,   152,    66,
      70,    71,    65,   124,   103,    71,   169,    66,   117,    66,
     117,    40,    70,    76,    65,   153,   154,   147,   150,   129,
      36,   108,   110,   111,    65,    62,   170,   171,   173,   174,
     119,   119,   124,   141,    24,    24,   148,   151,   112,    66,
      69,    65,   113,   114,   116,    71,    12,    66,     6,    14,
      18,    30,    37,   168,   172,   197,   198,   199,   204,   215,
     216,   219,   222,   125,   125,   142,   137,   155,   157,   158,
     113,   109,    70,    71,   171,   205,    65,   200,   220,   217,
      17,    55,    56,    57,    58,    65,   175,   176,   229,   232,
     234,   235,   236,   237,   238,   239,   241,   242,   243,   161,
     161,   139,    66,    39,   212,   213,   214,   110,   115,    65,
      23,    60,    61,    62,    63,    64,    65,    68,    73,    74,
     208,   234,   246,   250,   255,   260,    44,   246,   248,   170,
     246,    62,   177,   178,   179,   180,   181,    77,   230,    75,
      67,     5,     5,   156,    70,    71,   123,   116,   263,    68,
     262,   255,   251,    24,    41,    42,    43,    72,    78,    79,
     249,    25,    73,    74,   254,     3,     8,    21,    80,    81,
     259,   223,    32,    35,     9,    68,   183,   182,   183,    68,
     189,   188,   189,    68,   264,    44,   233,   240,    65,   100,
     106,   157,   213,    68,   260,   261,   246,   255,   206,   247,
     253,   252,   258,   256,   257,   226,   246,   201,   221,   218,
     184,   190,   246,   265,   267,   231,   244,   246,   170,   170,
     159,   265,    69,   209,   210,   212,   250,   255,   255,   260,
     260,   260,    10,    33,   228,   171,   246,   171,    65,   185,
     187,   234,   191,   193,   246,    69,    70,   246,    70,    76,
      12,    12,   145,   160,    69,    66,   207,    71,   224,    11,
     202,    69,    70,    69,    70,    71,   266,   245,    66,    66,
     152,    69,   210,    12,   211,   227,   246,   203,   186,   192,
     194,   267,   246,   171,     9,   171,   187,   193,   195,   246,
     225,    71,   171,   196,   246
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,    82,    83,    85,    84,    86,    86,    88,    89,    90,
      91,    92,    87,    93,    93,    94,    95,    95,    96,    96,
      97,    97,    98,    99,   100,    98,   101,   101,   102,   103,
     102,   104,   105,   106,   104,   107,   108,   109,   108,   111,
     110,   112,   110,   113,   114,   115,   114,   116,   117,   117,
     118,   118,   119,   119,   120,   120,   122,   121,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   124,   124,   124,   124,   124,   124,   124,   125,
     125,   126,   126,   128,   127,   129,   129,   130,   130,   130,
     131,   132,   133,   133,   134,   134,   134,   135,   137,   136,
     138,   139,   140,   141,   140,   142,   142,   143,   144,   145,
     146,   147,   146,   148,   148,   149,   150,   149,   151,   152,
     152,   153,   154,   155,   156,   155,   157,   158,   159,   157,
     160,   161,   161,   162,   162,   163,   164,   165,   164,   166,
     167,   169,   168,   170,   170,   172,   171,   171,   173,   173,
     174,   175,   175,   175,   175,   177,   176,   178,   176,   179,
     176,   180,   176,   181,   176,   182,   182,   184,   183,   185,
     186,   185,   187,   188,   188,   190,   189,   191,   192,   191,
     193,   194,   193,   195,   196,   195,   197,   197,   197,   198,
     198,   200,   201,   199,   202,   203,   202,   205,   206,   204,
     207,   207,   208,   209,   209,   211,   210,   212,   212,   214,
     213,   213,   215,   215,   215,   217,   218,   216,   220,   221,
     219,   223,   224,   225,   222,   226,   227,   228,   228,   230,
     231,   229,   232,   233,   234,   234,   234,   235,   236,   237,
     238,   238,   240,   239,   241,   242,   243,   244,   245,   244,
     246,   247,   246,   248,   249,   249,   249,   249,   249,   249,
     250,   251,   250,   250,   252,   250,   253,   250,   254,   254,
     255,   256,   255,   257,   255,   258,   255,   259,   259,   259,
     260,   260,   260,   260,   260,   260,   261,   260,   262,   260,
     263,   260,   264,   264,   265,   266,   265,   267
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     0,     4,     0,     3,     0,     0,     0,
       0,     0,    11,     1,     3,     1,     0,     1,     1,     2,
       1,     1,     4,     0,     0,    12,     2,     5,     0,     0,
       4,     4,     0,     0,    12,     3,     1,     0,     4,     0,
       2,     0,     3,     3,     1,     0,     4,     1,     0,     3,
       1,     3,     0,     2,     1,     2,     0,     5,     1,     2,
       2,     1,     2,     2,     1,     2,     2,     1,     2,     2,
       1,     1,     1,     2,     2,     1,     2,     2,     1,     0,
       2,     1,     2,     0,     5,     1,     1,     1,     1,     1,
       1,     3,     1,     2,     1,     1,     1,     3,     0,     7,
       2,     1,     1,     0,     4,     3,     1,     3,     2,     1,
       1,     0,     4,     0,     3,     1,     0,     4,     1,     0,
       4,     1,     1,     1,     0,     4,     0,     0,     0,     7,
       2,     0,     2,     1,     2,     4,     1,     0,     4,     1,
       1,     0,     4,     1,     3,     0,     3,     2,     0,     1,
       2,     0,     1,     1,     1,     0,     3,     0,     3,     0,
       3,     0,     3,     0,     3,     0,     1,     0,     4,     1,
       0,     4,     1,     0,     1,     0,     4,     1,     0,     4,
       1,     0,     4,     1,     0,     4,     1,     1,     1,     1,
       1,     0,     0,     7,     0,     0,     3,     0,     0,     8,
       0,     1,     1,     1,     3,     0,     4,     1,     3,     0,
       2,     1,     1,     1,     1,     0,     0,     6,     0,     0,
       6,     0,     0,     0,    11,     1,     1,     1,     1,     0,
       0,     5,     2,     1,     1,     1,     1,     2,     1,     1,
       1,     1,     0,     5,     1,     3,     1,     1,     0,     4,
       1,     0,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     3,     2,     0,     4,     0,     4,     1,     1,
       1,     0,     4,     0,     4,     0,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     5,     0,     4,
       0,     3,     0,     3,     1,     0,     4,     1
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
        case 3:
#line 129 "gram.y" /* yacc.c:1646  */
    {
                  prog_symbol = (yyvsp[0].sym);
                  open_name_file();
                  open_header_file();
                  if (!def_filename.empty())
                  {
                    open_def_file();
                  }
                }
#line 1717 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 147 "gram.y" /* yacc.c:1646  */
    {
                  cppout.redir_file(H_FILE_NUM);
                }
#line 1725 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 151 "gram.y" /* yacc.c:1646  */
    {
                  if (false && !def_filename.empty())
                  {
                    cppout.redir_file(DEF_FILE_NUM);
                  }
                }
#line 1736 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 158 "gram.y" /* yacc.c:1646  */
    {
                  if (false && !def_filename.empty())
                  {
                    cppout.redir_file(H_FILE_NUM);
                  }
                }
#line 1747 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 166 "gram.y" /* yacc.c:1646  */
    {
                  if (nglobalvars > 0 && !var_struct_name.empty())
                  {
                    cppout.out_s("};\n");
                    if (class_name.empty())
                    {
                      cppout.out_s("C4PEXTERN ");
                    }
                    cppout.out_s("struct " + var_struct_class_name + " " + var_struct_name + ";\n");
                  }
                  cppout.redir_file(C_FILE_NUM);
                }
#line 1764 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 179 "gram.y" /* yacc.c:1646  */
    {
                  if (!one_c_file)
                  {
                    begin_new_c_file(prog_symbol->s_repr, 1);
                  }
                  routine_handle = 0;
                  forget_fast_vars();
                  if (class_name.empty())
                  {
                    if (dll_flag)
                    {
                      cppout.redir_file(H_FILE_NUM);
                      cppout.out_s("\n#ifdef __cplusplus\nextern \"C++\" int C4PCEECALL " + entry_name + "(int argc, char* argv[]);\n#endif\n");
                      cppout.redir_file(C_FILE_NUM);
                      cppout.out_s("\nextern \"C++\" int C4PCEECALL " + entry_name + "(int argc, char* argv[])\n\n");
                    }
                    else
                    {
                      cppout.out_s("\nvoid C4PCEECALL int main(int argc, const char* argv[])\n\n");
                    }
                  }
                  else
                  {
                    cppout.redir_file(H_FILE_NUM);
                    cppout.out_s("int Run(int argc, char* argv[]);\n");
                    cppout.redir_file(C_FILE_NUM);
                    cppout.out_s("\nint " + class_name + "::Run(int argc, char* argv[])\n\n");
                  }
                }
#line 1798 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 209 "gram.y" /* yacc.c:1646  */
    {
                  if (n_fast_vars)
                  {
                    declare_fast_var_macro(routine_handle);
                  }
                  cppout.close_file(C_FILE_NUM);
                  close_header_file();
                  if (!def_filename.empty())
                  {
                    close_def_file();
                  }
                  close_name_file();
                }
#line 1816 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 231 "gram.y" /* yacc.c:1646  */
    {
                }
#line 1823 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 252 "gram.y" /* yacc.c:1646  */
    {
                }
#line 1830 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 255 "gram.y" /* yacc.c:1646  */
    {
                  routine_handle += 1;
                  begin_routine(reinterpret_cast<prototype_node*>((yyvsp[-1].type_ptr)), routine_handle);
                }
#line 1839 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 267 "gram.y" /* yacc.c:1646  */
    {
                  symbol_t * result_type = reinterpret_cast<prototype_node *>((yyvsp[-7].type_ptr))->result_type;
                  cppout.out_s("\n" + std::string(result_type->s_translated_type != nullptr ? result_type->s_translated_type : result_type->s_repr) + " c4p_result;\n\n");
                  cppout.out_s("C4P_PROC_ENTRY (" + std::to_string(routine_handle) + ");\n");
                }
#line 1849 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 273 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("\n");
                  if (auto_exit_label_flag)
                  {
                    cppout.out_s("c4p_L" + std::to_string(auto_exit_label) + ":\n");
                    auto_exit_label_flag = false;
                  }
                  cppout.out_s("C4P_PROC_EXIT (" + std::to_string(routine_handle) + ");\n");
                  cppout.out_s("return (c4p_result);\n");
                  end_routine(routine_handle);
                }
#line 1865 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 288 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != FUNCTION_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a function name", (yyvsp[0].sym)->s_repr);
                  }
                  (yyval.type_ptr) = (yyvsp[0].sym)->s_type_ptr;
                }
#line 1877 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 297 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[-3].sym)->s_kind == FUNCTION_IDENTIFIER)
                  {
                    /* already declared (forward */
                    (yyval.type_ptr) = (yyvsp[-3].sym)->s_type_ptr;
                  }
                  else
                  {
                    if ((yyvsp[-3].sym)->s_kind != UNDEFINED_IDENTIFIER)
                    {
                      c4p_error("`%s' already defined", (yyvsp[-3].sym)->s_repr);
                    }
                    if ((yyvsp[0].sym)->s_kind != TYPE_IDENTIFIER)
                    {
                      c4p_error("`%s' is not a type identifier", (yyvsp[0].sym)->s_repr);
                    }
                    (yyval.type_ptr) = new_type_node(PROTOTYPE_NODE, (yyvsp[-3].sym), (yyvsp[-2].type_ptr), (yyvsp[0].sym));
                    (yyvsp[-3].sym)->s_kind = FUNCTION_IDENTIFIER;
                    (yyvsp[-3].sym)->s_type = PROTOTYPE_NODE;
                    (yyvsp[-3].sym)->s_type_ptr = (yyval.type_ptr);
                  }
                }
#line 1904 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 323 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = nullptr;
                }
#line 1912 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 327 "gram.y" /* yacc.c:1646  */
    {
                  mark_symbol_table();
                }
#line 1920 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 331 "gram.y" /* yacc.c:1646  */
    {
                  unmark_symbol_table();
                  (yyval.type_ptr) = (yyvsp[-1].type_ptr);
                }
#line 1929 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 339 "gram.y" /* yacc.c:1646  */
    {
                }
#line 1936 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 342 "gram.y" /* yacc.c:1646  */
    {
                  routine_handle += 1;
                  begin_routine(reinterpret_cast<prototype_node*>((yyvsp[-1].type_ptr)), routine_handle);
                }
#line 1945 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 351 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("C4P_PROC_ENTRY (" + std::to_string(routine_handle) + ");\n");
                }
#line 1953 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 355 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("C4P_PROC_EXIT (" + std::to_string(routine_handle) + ");\n");
                  end_routine(routine_handle);
                }
#line 1962 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 363 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[-1].sym)->s_kind == PROCEDURE_IDENTIFIER)
                  {
                    /* already declared (forward) */
                    (yyval.type_ptr) = (yyvsp[-1].sym)->s_type_ptr;
                  }
                  else
                  {
                    if ((yyvsp[-1].sym)->s_kind != UNDEFINED_IDENTIFIER)
                    {
                      c4p_error("`%s' already declared", (yyvsp[-1].sym)->s_repr);
                    }
                    (yyval.type_ptr) = new_type_node(PROTOTYPE_NODE, (yyvsp[-1].sym), (yyvsp[0].type_ptr), 0);
                    (yyvsp[-1].sym)->s_kind = PROCEDURE_IDENTIFIER;
                    (yyvsp[-1].sym)->s_type = PROTOTYPE_NODE;
                    (yyvsp[-1].sym)->s_type_ptr = (yyval.type_ptr);
                  }
                }
#line 1985 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 386 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_parameter;
                }
#line 1993 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 390 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<parameter_node *>((yyvsp[-1].type_ptr))->next = reinterpret_cast<parameter_node*>((yyvsp[0].type_ptr));
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2002 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 397 "gram.y" /* yacc.c:1646  */
    {
                  by_reference = false;
                }
#line 2010 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 401 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = (yyvsp[0].type_ptr);
                }
#line 2018 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 405 "gram.y" /* yacc.c:1646  */
    {
                  by_reference = true;
                }
#line 2026 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 409 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = (yyvsp[0].type_ptr);
                }
#line 2034 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 420 "gram.y" /* yacc.c:1646  */
    {
                  parameter_node * par = reinterpret_cast<parameter_node*>((yyvsp[-2].type_ptr));
                  if ((yyvsp[0].sym)->s_kind != TYPE_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a type identifier", (yyvsp[0].sym)->s_repr);
                  }
                  while (par != nullptr)
                  {
                    if ((yyvsp[0].sym)->s_type != ARRAY_NODE)
                    {
                      par->by_reference = by_reference;
                    }
                    else
                    {
                      par->by_reference = false;
                    }
                    par->type_symbol = (yyvsp[0].sym);
                    par = par->next;
                  }
                  (yyval.type_ptr) = (yyvsp[-2].type_ptr);
                }
#line 2060 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 446 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_parameter;
                }
#line 2068 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 450 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<parameter_node *>((yyvsp[-1].type_ptr))->next = reinterpret_cast<parameter_node*>((yyvsp[0].type_ptr));
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2077 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 458 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(PARAMETER_NODE, (yyvsp[0].sym)->s_repr);
                  last_parameter = reinterpret_cast<parameter_node*>((yyval.type_ptr));
                }
#line 2086 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 472 "gram.y" /* yacc.c:1646  */
    {
                }
#line 2093 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 490 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("static constexpr ");
                  (yyval.buf_mark) = cppout.get_buf_mark();
                  cppout.out_s("                    " + std::string((yyvsp[-1].sym)->s_repr) + " { ");
                }
#line 2103 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 496 "gram.y" /* yacc.c:1646  */
    {
                  if (last_type == REAL_TYPE)
                  {
                    cppout.out_buf_over((yyvsp[-2].buf_mark), "C4P_real            ", 20);
                  }
                  else if (last_type == LONG_REAL_TYPE)
                  {
                    cppout.out_buf_over((yyvsp[-2].buf_mark), "C4P_longreal        ", 20);
                  }
                  else if (last_type == INTEGER_TYPE)
                  {
                    cppout.out_buf_over((yyvsp[-2].buf_mark), "C4P_integer         ", 20);
                  }
                  else if (last_type == STRING_TYPE)
                  {
                    cppout.out_buf_over((yyvsp[-2].buf_mark), "const char*         ", 20);
                  }
                  else if (last_type == CHARACTER_TYPE)
                  {
                    cppout.out_buf_over((yyvsp[-2].buf_mark), "char                ", 20);
                  }
                  cppout.out_s(" };\n");
                  define_symbol((yyvsp[-4].sym), CONSTANT_IDENTIFIER, block_level, last_type, nullptr, &last_value);
                }
#line 2132 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 524 "gram.y" /* yacc.c:1646  */
    {
                  last_value.fvalue = (yyvsp[0].fvalue);
                  last_type = REAL_TYPE;
                  cppout.out_s(std::to_string(last_value.fvalue));
                }
#line 2142 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 530 "gram.y" /* yacc.c:1646  */
    {
                  last_value.fvalue = (yyvsp[0].fvalue);
                  last_type = REAL_TYPE;
                  cppout.out_s(std::to_string(last_value.fvalue));
                }
#line 2152 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 536 "gram.y" /* yacc.c:1646  */
    {
                  last_value.fvalue = - (yyvsp[0].fvalue);
                  last_type = REAL_TYPE;
                  cppout.out_s(std::to_string(last_value.fvalue));
                }
#line 2162 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 542 "gram.y" /* yacc.c:1646  */
    {
                  last_value.fvalue = (yyvsp[0].fvalue);
                  last_type = LONG_REAL_TYPE;
                  cppout.out_s(std::to_string(last_value.fvalue));
                }
#line 2172 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 548 "gram.y" /* yacc.c:1646  */
    {
                  last_value.fvalue = (yyvsp[0].fvalue);
                  last_type = LONG_REAL_TYPE;
                  cppout.out_s(std::to_string(last_value.fvalue));
                }
#line 2182 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 554 "gram.y" /* yacc.c:1646  */
    {
                  last_value.fvalue = - (yyvsp[0].fvalue);
                  last_type = LONG_REAL_TYPE;
                  cppout.out_s(std::to_string(last_value.fvalue));
                }
#line 2192 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 561 "gram.y" /* yacc.c:1646  */
    {
                  last_value.ivalue = (yyvsp[0].value);
                  last_type = INTEGER_TYPE;
                  cppout.out_s(std::to_string(last_value.ivalue) + integer_literal_suffix);
                }
#line 2202 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 567 "gram.y" /* yacc.c:1646  */
    {
                  last_value.ivalue = (yyvsp[0].value);
                  last_type = INTEGER_TYPE;
                  cppout.out_s(std::to_string(last_value.ivalue) + integer_literal_suffix);
                }
#line 2212 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 573 "gram.y" /* yacc.c:1646  */
    {
                  last_value.ivalue = - (yyvsp[0].value);
                  last_type = INTEGER_TYPE;
                  cppout.out_s(std::to_string(last_value.ivalue) + integer_literal_suffix);
                }
#line 2222 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 579 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != CONSTANT_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a constant identifier", (yyvsp[0].sym)->s_repr);
                  }
                  last_value = (yyvsp[0].sym)->s_value;
                  last_type = (yyvsp[0].sym)->s_type;
                  cppout.out_s((yyvsp[0].sym)->s_repr);
                }
#line 2236 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 589 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != CONSTANT_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a constant identifier", (yyvsp[0].sym)->s_repr);
                  }
                  last_value = (yyvsp[0].sym)->s_value;
                  last_type = (yyvsp[0].sym)->s_type;
                  cppout.out_s((yyvsp[0].sym)->s_repr);
                }
#line 2250 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 599 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != CONSTANT_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a constant identifier", (yyvsp[0].sym)->s_repr);
                  }
                  /* fixme: suspicious: */
                  last_value.ivalue = - (yyvsp[0].sym)->s_value.ivalue;
                  last_type = (yyvsp[0].sym)->s_type;
                  cppout.out_s("(-" + std::string((yyvsp[0].sym)->s_repr) + ")");
                }
#line 2265 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 610 "gram.y" /* yacc.c:1646  */
    {
                  last_value.ivalue = (yyvsp[0].value);
                  last_type = STRING_TYPE;
                  cppout.out_s("\"");
                  cppout.translate_string(get_string((STRING_PTR)(yyvsp[0].value)));
                  cppout.out_s("\"");
                }
#line 2277 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 618 "gram.y" /* yacc.c:1646  */
    {
                  last_value.ivalue = (yyvsp[0].value);
                  last_type = CHARACTER_TYPE;
                  cppout.out_s("\'");
                  cppout.translate_char(static_cast<int>((yyvsp[0].value)));
                  cppout.out_s("\'");
                }
#line 2289 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 630 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.value) = (yyvsp[0].value);
                }
#line 2297 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 634 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.value) = - (yyvsp[0].value);
                }
#line 2305 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 638 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != CONSTANT_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a constant identifier", (yyvsp[0].sym)->s_repr);
                  }
                  (yyval.value) = (yyvsp[0].sym)->s_value.ivalue;
                }
#line 2317 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 646 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != CONSTANT_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a constant identifier", (yyvsp[0].sym)->s_repr);
                  }
                  (yyval.value) = (yyvsp[0].sym)->s_value.ivalue;
                }
#line 2329 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 654 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != CONSTANT_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a constant identifier", (yyvsp[0].sym)->s_repr);
                  }
                  (yyval.value) = - (yyvsp[0].sym)->s_value.ivalue;
                }
#line 2341 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 676 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("typedef ");
                }
#line 2349 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 680 "gram.y" /* yacc.c:1646  */
    {
                  define_type((yyvsp[-4].sym), block_level, last_type, (yyvsp[-1].type_ptr));
                  cppout.out_s(";\n");
                }
#line 2358 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 688 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != TYPE_IDENTIFIER)
                  {
                    c4p_error ("`%s' is not a type identifier", (yyvsp[0].sym)->s_repr);
                  }
                  last_type = NAMED_TYPE_NODE;
                  (yyval.type_ptr) = new_type_node(NAMED_TYPE_NODE, (yyvsp[0].sym));
                }
#line 2371 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 714 "gram.y" /* yacc.c:1646  */
    {
                  last_type = SUBRANGE_NODE;
                  (yyval.type_ptr) = new_type_node(SUBRANGE_NODE, (yyvsp[-2].value), (yyvsp[0].value));
                }
#line 2380 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 723 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = (yyvsp[0].type_ptr);
                }
#line 2388 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 739 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(FILE_NODE, last_type, (yyvsp[0].type_ptr));
                  last_type = FILE_NODE;
                }
#line 2397 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 747 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_array_node;
                }
#line 2405 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 751 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<array_node *>((yyvsp[-1].type_ptr))->component_type = last_type;
                  reinterpret_cast<array_node *>((yyvsp[-1].type_ptr))->component_type_ptr = (yyvsp[0].type_ptr);
                  last_type = ARRAY_NODE;
                  (yyval.type_ptr) = (yyvsp[-4].type_ptr);
                }
#line 2416 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 761 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(POINTER_NODE, last_type, (yyvsp[0].type_ptr));
                  last_type = POINTER_NODE;
                }
#line 2425 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 774 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_array_node;
                }
#line 2433 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 778 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<array_node *>((yyvsp[-1].type_ptr))->component_type = ARRAY_NODE;
                  reinterpret_cast<array_node *>((yyvsp[-1].type_ptr))->component_type_ptr = (yyvsp[0].type_ptr);
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2443 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 787 "gram.y" /* yacc.c:1646  */
    {
                  last_array_node = reinterpret_cast<array_node*>(new_type_node(ARRAY_NODE, (yyvsp[-2].value), (yyvsp[0].value)));
                  (yyval.type_ptr) = reinterpret_cast<array_node*>(last_array_node);
                }
#line 2452 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 792 "gram.y" /* yacc.c:1646  */
    {
                  C4P_integer lbound;
                  C4P_integer ubound;
                  if ((yyvsp[0].sym)->s_kind != TYPE_IDENTIFIER)
                  {
                    c4p_error("`%s' is not a type identifier", (yyvsp[0].sym)->s_repr);
                  }
                  if ((yyvsp[0].sym)->s_type == CHARACTER_TYPE)
                  {
                    if (chars_are_unsigned)
                    {
                      lbound = 0;
                      ubound = 255;
                    }
                    else
                    {
                      lbound = -128;
                      ubound = 127;
                    }
                  }
                  else if ((yyvsp[0].sym)->s_type == BOOLEAN_TYPE)
                  {
                    lbound = 0;
                    ubound = 1;
                  }
                  else if ((yyvsp[0].sym)->s_type != SUBRANGE_NODE)
                  {
                    c4p_error("`%s' is not a subrange type identifier", (yyvsp[0].sym)->s_repr);
                  }
                  else
                  {
                    lbound = (reinterpret_cast<subrange_node*>((yyvsp[0].sym)->s_type_ptr)->lower_bound);
                    ubound = (reinterpret_cast<subrange_node*>((yyvsp[0].sym)->s_type_ptr)->upper_bound);
                  }
                  last_array_node = (reinterpret_cast<array_node*>(new_type_node(ARRAY_NODE, lbound, ubound)));
                  (yyval.type_ptr) = last_array_node;
                }
#line 2494 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 833 "gram.y" /* yacc.c:1646  */
    {
                  last_type = RECORD_NODE;
                  (yyval.type_ptr) = new_type_node(RECORD_NODE, (yyvsp[-1].type_ptr));
                }
#line 2503 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 841 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(FIELD_LIST_NODE, (yyvsp[-1].type_ptr), (yyvsp[0].type_ptr));
                }
#line 2511 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 853 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_record_section;
                }
#line 2519 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 857 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<record_section_node *>((yyvsp[-1].type_ptr))->next = reinterpret_cast<record_section_node*>((yyvsp[0].type_ptr));
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2528 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 865 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = nullptr;
                }
#line 2536 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 869 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(RECORD_SECTION_NODE, (yyvsp[-2].type_ptr), last_type, (yyvsp[0].type_ptr));
                  last_record_section = reinterpret_cast<record_section_node*>((yyval.type_ptr));
                }
#line 2545 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 878 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_declarator;
                }
#line 2553 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 882 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<declarator_node *>((yyvsp[-1].type_ptr))->next = reinterpret_cast<declarator_node*>((yyvsp[0].type_ptr));
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2562 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 890 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(DECLARATOR_NODE, (yyvsp[0].sym));
                  last_declarator = reinterpret_cast<declarator_node*>((yyval.type_ptr));
                }
#line 2571 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 898 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = nullptr;
                }
#line 2579 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 902 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(VARIANT_NODE, (yyvsp[0].type_ptr));
                }
#line 2587 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 916 "gram.y" /* yacc.c:1646  */
    {
                }
#line 2594 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 923 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_variant;
                }
#line 2602 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 927 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<variant_field_list_node *>((yyvsp[-1].type_ptr))->next = reinterpret_cast<variant_field_list_node*>((yyvsp[0].type_ptr));
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2611 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 935 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_variant = nullptr;
                }
#line 2619 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 939 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_stop();
                }
#line 2627 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 943 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_resume();
                }
#line 2635 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 947 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = (yyvsp[-1].type_ptr);
                  last_variant = reinterpret_cast<variant_node*>((yyval.type_ptr));
                }
#line 2644 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 955 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(VARIANT_FIELD_LIST_NODE, (yyvsp[-1].type_ptr), (yyvsp[0].type_ptr));
                }
#line 2652 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 972 "gram.y" /* yacc.c:1646  */
    {
                  if (block_level == 0)
                  {
                    if (class_name.length() == 0 && var_struct_name.length() == 0)
                    {
                      cppout.out_s("C4PEXTERN ");
                    }
                    if (nglobalvars == 0 && !var_struct_name.empty())
                    {
                      cppout.out_s("struct " +  var_struct_class_name);
                      if (!var_struct_base_class_name.empty())
                      {
                        cppout.out_s(" : public " + var_struct_base_class_name);
                      }
                      cppout.out_s(" {\n");
                    }
                    nglobalvars += 1;
                  }
                  declare_var_list(reinterpret_cast<declarator_node*>((yyvsp[-3].type_ptr)), VARIABLE_IDENTIFIER, block_level, last_type, (yyvsp[-1].type_ptr));
                  cppout.out_s(";\n");
                }
#line 2678 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 998 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = last_declarator;
                }
#line 2686 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1002 "gram.y" /* yacc.c:1646  */
    {
                  reinterpret_cast<declarator_node *>((yyvsp[-1].type_ptr))->next = reinterpret_cast<declarator_node*>((yyvsp[0].type_ptr));
                  (yyval.type_ptr) = (yyvsp[-3].type_ptr);
                }
#line 2695 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1010 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type_ptr) = new_type_node(DECLARATOR_NODE, (yyvsp[0].sym));
                  last_declarator = reinterpret_cast<declarator_node*>((yyval.type_ptr));
                }
#line 2704 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1022 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("{\n");
                  curly_brace_level += 1;
                  if (routine_handle == 0 && curly_brace_level == 1)
                  {
                    if (n_fast_vars)
                    {
                      cppout.out_s("C4P_FAST_VARS_0\n");
                    }
                    cppout.out_s("C4P_BEGIN_PROGRAM(\"" + std::string(prog_symbol->s_repr) + "\", argc, argv);\n");
                  }
                }
#line 2721 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1035 "gram.y" /* yacc.c:1646  */
    {
                  if (routine_handle == 0 && curly_brace_level == 1)
                  {
                    cppout.out_s("C4P_END_PROGRAM();\n");
                  }
                  curly_brace_level -= 1;
                  cppout.out_s("}\n");
                }
#line 2734 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1052 "gram.y" /* yacc.c:1646  */
    {
                  buf_mark2 = NO_MARK;
                }
#line 2742 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1066 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("\nc4p_L" + std::to_string((yyvsp[-1].value)) + ":\n");
                  if (auto_exit_label >= 0 && (yyvsp[-1].value) == auto_exit_label)
                  {
                    auto_exit_label_flag = false;
                  }
                }
#line 2754 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1077 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(";\n");
                }
#line 2762 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1087 "gram.y" /* yacc.c:1646  */
    {
                  to_eoln = false;
                }
#line 2770 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1092 "gram.y" /* yacc.c:1646  */
    {
                  to_eoln = true;
                }
#line 2778 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1097 "gram.y" /* yacc.c:1646  */
    {
                  to_eoln = false;
                }
#line 2786 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1102 "gram.y" /* yacc.c:1646  */
    {
                  to_eoln = true;
                }
#line 2794 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1107 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[0].sym)->s_kind != PROCEDURE_IDENTIFIER)
                  {
                    c4p_warning("`%s' is not a procedure identifier", (yyvsp[0].sym)->s_repr);
                  }
                  cppout.out_s(std::string((yyvsp[0].sym)->s_repr) + " (");
                  push_parameter_node (last_parameter);
                  if ((yyvsp[0].sym)->s_kind == PROCEDURE_IDENTIFIER)
                  {
                    last_parameter = (reinterpret_cast<prototype_node*>((yyvsp[0].sym)->s_type_ptr)->formal_parameter);
                  }
                  else
                  {
                    last_parameter = 0;
                  }
                }
#line 2815 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1124 "gram.y" /* yacc.c:1646  */
    {
                  pop_parameter_node(last_parameter);
                  cppout.out_s(");\n");
                }
#line 2824 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1132 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("c4p_readln ();\n");
                }
#line 2832 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1140 "gram.y" /* yacc.c:1646  */
    {
                  if (to_eoln)
                  {
                    cppout.out_s("C4P_READLN_BEGIN () ");
                  }
                  else
                  {
                    cppout.out_s("C4P_READ_BEGIN () ");
                  }
                  cppout.out_s("c4p_read_");
                  buf_mark = cppout.get_buf_mark();
                  cppout.out_s("_ (");
                  file_specified = false;
                  file_spec = "input";
                }
#line 2852 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1156 "gram.y" /* yacc.c:1646  */
    {
                  if (to_eoln)
                  {
                    cppout.out_s(", " + file_spec + "); C4P_READLN_END (" + file_spec + ")\n");
                  }
                  else
                  {
                    cppout.out_s(", " + file_spec + "); C4P_READ_END (" + file_spec + ")\n");
                  }
                }
#line 2867 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1171 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(", " + file_spec + "); c4p_read_");
                  buf_mark = cppout.get_buf_mark();
                  cppout.out_s("_ (");
                }
#line 2877 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1181 "gram.y" /* yacc.c:1646  */
    {
                  if (file_specified && file_type != CHARACTER_TYPE)
                  {
                    cppout.out_buf_over(buf_mark, "v", 1);
                  }
                  else
                  {
                    switch (last_type)
                    {
                    case FILE_NODE:
                      cppout.out_buf_over(buf_mark, "f", 1);
                      file_specified = true;
                      cppout.get_buf_text(file_spec, buf_mark + 3, 0);
                      file_type = reinterpret_cast<file_node *>(last_type_ptr)->type;
                      break;
                    case REAL_TYPE:
                    case LONG_REAL_TYPE:
                      cppout.out_buf_over(buf_mark, "r", 1);
                      break;
                    case INTEGER_TYPE:
                      cppout.out_buf_over(buf_mark, "i", 1);
                      break;
                    case SUBRANGE_NODE:
                      cppout.out_buf_over(buf_mark, "i", 1);
                      break;
                    case CHARACTER_TYPE:
                      cppout.out_buf_over(buf_mark, "c", 1);
                      break;
                    case STRING_TYPE:
                      cppout.out_buf_over(buf_mark, "s", 1);
                      break;
                    default:
                      c4p_error("internal error: unknown read type");
                    }
                  }
                }
#line 2918 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1221 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("c4p_writeln ();\n");
                }
#line 2926 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1229 "gram.y" /* yacc.c:1646  */
    {
                  if (to_eoln)
                  {
                    cppout.out_s("C4P_WRITELN_BEGIN () ");
                  }
                  else
                  {
                    cppout.out_s("C4P_WRITE_BEGIN () ");
                  }
                  cppout.out_s("c4p_write_");
                  buf_mark = cppout.get_buf_mark();
                  cppout.out_s("__ (");
                  file_specified = false;
                  file_spec = "output";
                }
#line 2946 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1245 "gram.y" /* yacc.c:1646  */
    {
                  if (to_eoln)
                  {
                    cppout.out_s(", " + file_spec + "); C4P_WRITELN_END (" + file_spec + ")\n");
                  }
                  else
                  {
                    cppout.out_s(", " + file_spec + "); C4P_WRITE_END (" + file_spec + ")\n");
                  }
                }
#line 2961 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1260 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(", " + file_spec + "); c4p_write_");
                  buf_mark = cppout.get_buf_mark();
                  cppout.out_s("__ (");
                }
#line 2971 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1270 "gram.y" /* yacc.c:1646  */
    {
                  if (file_specified && file_type != CHARACTER_TYPE)
                  {
                    cppout.out_buf_over(buf_mark, "v ", 2);
                  }
                  else
                  {
                    switch (last_type)
                    {
                    case FILE_NODE:
                      cppout.out_buf_over(buf_mark, "f ", 2);
                      file_specified = true;
                      cppout.get_buf_text(file_spec, buf_mark + 4, 0);
                      file_type = reinterpret_cast<file_node*>(last_type_ptr)->type;
                      break;
                    case REAL_TYPE:
                    case LONG_REAL_TYPE:
                      cppout.out_buf_over(buf_mark, "r ", 2);
                      break;
                    case INTEGER_TYPE:
                      cppout.out_buf_over(buf_mark, "i ", 2);
                      break;
                    case SUBRANGE_NODE:
                      cppout.out_buf_over(buf_mark, "i ", 2);
                      break;
                    case CHARACTER_TYPE:
                      cppout.out_buf_over(buf_mark, "c ", 2);
                      break;
                    case ARRAY_NODE:
                    case STRING_TYPE:
                      cppout.out_buf_over(buf_mark, "s ", 2);
                      break;
                    default:
                      c4p_error("internal error: unknown write type: %u", last_type);
                    }
                  }
                }
#line 3013 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1308 "gram.y" /* yacc.c:1646  */
    {
                  switch (last_type)
                  {
                  case REAL_TYPE:
                  case LONG_REAL_TYPE:
                    cppout.out_buf_over(buf_mark, "r1", 2);
                    break;
                  case INTEGER_TYPE:
                    cppout.out_buf_over(buf_mark, "i1", 2);
                    break;
                  case SUBRANGE_NODE:
                    cppout.out_buf_over(buf_mark, "i1", 2);
                    break;
                  case CHARACTER_TYPE:
                    cppout.out_buf_over(buf_mark, "c1", 2);
                    break;
                  case STRING_TYPE:
                    cppout.out_buf_over(buf_mark, "s1", 2);
                    break;
                  default:
                    c4p_error("internal error: unkown write type: %u", last_type);
                  }
                  cppout.out_s(", ");
                }
#line 3042 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1337 "gram.y" /* yacc.c:1646  */
    {
                }
#line 3049 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1340 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(", ");
                }
#line 3057 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1344 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_buf_over(buf_mark + 1, "2", 1); break;
                }
#line 3065 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1366 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("if (");
                }
#line 3073 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1370 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(")\n");
                  extra_indent += 1;
                }
#line 3082 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1379 "gram.y" /* yacc.c:1646  */
    {
                  extra_indent -= 1;
                }
#line 3090 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1383 "gram.y" /* yacc.c:1646  */
    {
                  extra_indent -= 1;
                  cppout.out_s("else\n");
                  extra_indent += 1;
                }
#line 3100 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1389 "gram.y" /* yacc.c:1646  */
    {
                  extra_indent -= 1;
                }
#line 3108 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1396 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("switch (");
                }
#line 3116 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1400 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(") {\n");
                  curly_brace_level += 1;
                }
#line 3125 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1405 "gram.y" /* yacc.c:1646  */
    {
                  curly_brace_level -= 1;
                  cppout.out_s("}\n");
                }
#line 3134 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1418 "gram.y" /* yacc.c:1646  */
    {
                }
#line 3141 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 1429 "gram.y" /* yacc.c:1646  */
    {
                  extra_indent += 1;
                }
#line 3149 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1433 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("break;\n");
                  extra_indent -= 1;
                }
#line 3158 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1445 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("case ");
                }
#line 3166 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1449 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(":\n");
                }
#line 3174 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1454 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("default:\n");
                }
#line 3182 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1467 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("while (");
                }
#line 3190 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 1471 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(")\n");
                  extra_indent += 1;
                }
#line 3199 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1476 "gram.y" /* yacc.c:1646  */
    {
                  extra_indent -= 1;
                }
#line 3207 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1483 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("do {\n");
                  curly_brace_level += 1;
                }
#line 3216 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1488 "gram.y" /* yacc.c:1646  */
    {
                  curly_brace_level -= 1;
                  cppout.out_s("} while (! (");
                }
#line 3225 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1493 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("));\n");
                }
#line 3233 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1501 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("C4P_FOR_BEGIN (");
                  if ((yyvsp[-1].sym)->s_block_level == 0 && (yyvsp[-1].sym)->s_kind == VARIABLE_IDENTIFIER && ! ((yyvsp[-1].sym)->s_flags & S_PREDEFINED))
                  {
                    if (!var_struct_name.empty())
                    {
                      cppout.out_s(var_struct_name + ".");
                    }
                    cppout.out_s(var_name_prefix.c_str());
                  }
                  cppout.out_s(std::string((yyvsp[-1].sym)->s_repr) + ", ");
                }
#line 3250 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1514 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(", " + std::string((yyvsp[0].value) > 0 ? "<=" : ">=") + ", ");
                }
#line 3258 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1518 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(")\n");
                  extra_indent += 1;
                }
#line 3267 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1523 "gram.y" /* yacc.c:1646  */
    {
                  extra_indent -= 1;
                  cppout.out_s("C4P_FOR_END (");
                  if ((yyvsp[-9].sym)->s_block_level == 0 && (yyvsp[-9].sym)->s_kind == VARIABLE_IDENTIFIER && ! ((yyvsp[-9].sym)->s_flags & S_PREDEFINED))
                  {
                    if (!var_struct_name.empty())
                    {
                      cppout.out_s(var_struct_name + ".");
                    }
                    cppout.out_s(var_name_prefix);
                  }
                  cppout.out_s(std::string((yyvsp[-9].sym)->s_repr) + ", " + ((yyvsp[-5].value) > 0 ? "++" : "--") + ")\n");
                }
#line 3285 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1540 "gram.y" /* yacc.c:1646  */
    {
                }
#line 3292 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1546 "gram.y" /* yacc.c:1646  */
    {
                }
#line 3299 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 1552 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.value) = 1;
                }
#line 3307 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1556 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.value) = -1;
                }
#line 3315 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1563 "gram.y" /* yacc.c:1646  */
    {
                  if (last_type == FUNCTION_TYPE)
                  {
                    cppout.out_s("c4p_result = ");
                  }
                  else
                  {
                    cppout.out_s(" = ");
                  }
                }
#line 3330 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1574 "gram.y" /* yacc.c:1646  */
    {
                  if (buf_mark2 != NO_MARK)
                  {
                    cppout.out_buf_over(buf_mark2, "w", 1);
                  }
                }
#line 3341 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1581 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(";\n");
                }
#line 3349 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1588 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("goto c4p_L" + std::to_string((yyvsp[0].value)) + ";\n");
                }
#line 3357 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1608 "gram.y" /* yacc.c:1646  */
    {
                  if (last_type != FILE_NODE)
                  {
                    c4p_error("left side of `^' must be a file variable");
                  }
                  cppout.out_s(".bufref()");
                  last_type = reinterpret_cast<file_node*>(last_type_ptr)->type;
                  last_type_ptr = reinterpret_cast<file_node*>(last_type_ptr)->type_ptr;
                }
#line 3371 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1625 "gram.y" /* yacc.c:1646  */
    {
                  switch ((yyvsp[0].sym)->s_kind)
                  {
                  case VARIABLE_IDENTIFIER:
                  case PARAMETER_IDENTIFIER:
                  case CONSTANT_IDENTIFIER:
                    last_type = (yyvsp[0].sym)->s_type;
                    last_type_ptr = (yyvsp[0].sym) ->s_type_ptr;
                    if ((yyvsp[0].sym)->s_flags & S_FAST)
                    {
                      cppout.out_s("c4p_fast_" + std::string((yyvsp[0].sym)->s_repr) + "_" + std::to_string(routine_handle));
                      remember_fast_var((yyvsp[0].sym)->s_repr);
                    }
                    else
                    {
                      if ((yyvsp[0].sym)->s_block_level == 0 && (yyvsp[0].sym)->s_kind == VARIABLE_IDENTIFIER && ! ((yyvsp[0].sym)->s_flags & S_PREDEFINED))
                      {
                        if (!var_struct_name.empty())
                        {
                          cppout.out_s(var_struct_name + ".");
                        }
                        cppout.out_s(var_name_prefix.c_str());
                      }
                      cppout.out_s((yyvsp[0].sym)->s_repr);
                    }
                    break;
                  case FUNCTION_IDENTIFIER:
                    last_type = FUNCTION_TYPE;
                    last_type_ptr = (yyvsp[0].sym)->s_type_ptr;
                    break;
                  default:
                    c4p_error("`%s' isn't defined", (yyvsp[0].sym)->s_repr);
                    break;
                  }
                }
#line 3411 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1669 "gram.y" /* yacc.c:1646  */
    {
                  if (last_type != ARRAY_NODE && last_type != POINTER_NODE)
                  {
                    c4p_error("left side of `[' must be an array variable");
                  }
                  cppout.out_s("[ ");
                  (yyval.value) = 0;
                  if (last_type == POINTER_NODE)
                  {
                    push_array_type(reinterpret_cast<pointer_node*>(last_type_ptr)->array_node_ptr);
                  }
                  else
                  {
                    push_array_type(reinterpret_cast<array_node*>(last_type_ptr));
                  }
                }
#line 3432 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1686 "gram.y" /* yacc.c:1646  */
    {
                  last_type = last_array_node->component_type;
                  last_type_ptr = last_array_node->component_type_ptr;
                  pop_array_type();
                  cppout.out_s(" ]");
                }
#line 3443 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1700 "gram.y" /* yacc.c:1646  */
    {
                  last_type = translate_field_name((yyvsp[0].sym)->s_repr, reinterpret_cast<record_node*>(last_type_ptr), &last_type_ptr);
                }
#line 3451 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1711 "gram.y" /* yacc.c:1646  */
    {
                  if (last_array_node->lower_bound != 0)
                  {
                    cppout.out_s(" - (" + std::to_string(get_array_offset(last_array_node)) + ")");
                  }
                  last_type = last_array_node->component_type;
                  last_type_ptr = last_array_node->component_type_ptr;
                }
#line 3464 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1720 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" ][ ");
                }
#line 3472 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1724 "gram.y" /* yacc.c:1646  */
    {
                  last_array_node = reinterpret_cast<array_node*>(last_array_node->component_type_ptr);
                  cppout.out_s(" - (" + std::to_string(get_array_offset(last_array_node)) + ")");
                  last_type = last_array_node->component_type;
                  last_type_ptr = last_array_node->component_type_ptr;
                }
#line 3483 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1735 "gram.y" /* yacc.c:1646  */
    {
                  if (relational_cast_expressions)
                  {
                    (yyval.buf_mark) = cppout.get_buf_mark();
                    cppout.out_s("(C4P_integer)     ");
                  }
                }
#line 3495 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1743 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = coerce((yyvsp[-3].type), (yyvsp[0].type));
                  if (relational_cast_expressions)
                  {
                    if ((yyval.type) == REAL_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_real)        ", 18);
                    }
                    if ((yyval.type) == LONG_REAL_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_longreal)    ", 18);
                    }
                    if ((yyval.type) == LONG_INTEGER_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_longinteger) ", 18);
                    }
                  }
                }
#line 3518 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1769 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" < ");
                }
#line 3526 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1773 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" > ");
                }
#line 3534 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1777 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" <= ");
                }
#line 3542 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1781 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" >= ");
                }
#line 3550 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1785 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" == ");
                }
#line 3558 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1789 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" != ");
                }
#line 3566 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1797 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" - ");
                  if (other_cast_expressions)
                  {
                    cppout.out_s("(C4P_integer) ");
                  }
                }
#line 3578 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1805 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = (yyvsp[0].type);
                }
#line 3586 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1809 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = (yyvsp[0].type);
                }
#line 3594 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1813 "gram.y" /* yacc.c:1646  */
    {
                  if (other_cast_expressions)
                  {
                    (yyval.buf_mark) = cppout.get_buf_mark();
                    cppout.out_s("(C4P_integer)     ");
                  }
                }
#line 3606 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1821 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = coerce((yyvsp[-3].type), (yyvsp[0].type));
                  if (other_cast_expressions)
                  {
                    if ((yyval.type) == REAL_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_real)        ", 18);
                    }
                    if ((yyval.type) == LONG_REAL_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_longreal)    ", 18);
                    }
                    if ((yyval.type) == LONG_INTEGER_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_longinteger) ", 18);
                    }
                  }
                }
#line 3629 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1840 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" || ");
                }
#line 3637 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1848 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" - ");
                }
#line 3645 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1852 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" + ");
                }
#line 3653 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1860 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" / ");
                  if (other_cast_expressions)
                  {
                    cppout.out_s("(C4P_real) ");
                  }
                }
#line 3665 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1869 "gram.y" /* yacc.c:1646  */
    {
                  if (other_cast_expressions)
                  {
                    (yyval.buf_mark) = cppout.get_buf_mark();
                    cppout.out_s("(C4P_integer)     ");
                  }
                }
#line 3677 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1877 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = coerce((yyvsp[-3].type),(yyvsp[0].type));
                  if (other_cast_expressions)
                  {
                    if ((yyval.type) == REAL_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_real)        ", 18);
                    }
                    if ((yyval.type) == LONG_REAL_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_longreal)    ", 18);
                    }
                    if ((yyval.type) == LONG_INTEGER_TYPE)
                    {
                      cppout.out_buf_over((yyvsp[-1].buf_mark), "(C4P_longinteger) ", 18);
                    }
                  }
                }
#line 3700 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 1896 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" && ");
                }
#line 3708 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1904 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" * ");
                }
#line 3716 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 1908 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" / ");
                }
#line 3724 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1912 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" % ");
                }
#line 3732 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1919 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = last_type = INTEGER_TYPE;
                  cppout.out_s(std::to_string((yyvsp[0].value)) + integer_literal_suffix);
                }
#line 3741 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1924 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = last_type = REAL_TYPE;
                  cppout.out_s(std::to_string((yyvsp[0].fvalue)));
                }
#line 3750 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1929 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = last_type = LONG_REAL_TYPE;
                  cppout.out_s(std::to_string((yyvsp[0].fvalue)));
                }
#line 3759 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1934 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = last_type = STRING_TYPE;
                  cppout.out_s("\"");
                  cppout.translate_string(get_string ((STRING_PTR) (yyvsp[0].value)));
                  cppout.out_s("\"");
                }
#line 3770 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1941 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = last_type = CHARACTER_TYPE;
                  cppout.out_s("\'");
                  cppout.translate_char(static_cast<int>((yyvsp[0].value)));
                  cppout.out_s("\'");
                }
#line 3781 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 1948 "gram.y" /* yacc.c:1646  */
    {
                  if (last_type == FUNCTION_TYPE)
                  {
                    prototype_node * proto = reinterpret_cast<prototype_node*>(last_type_ptr);
                    if (strcmp(proto->name->s_repr, "eoln") == 0)
                    {
                      cppout.out_s("eoln(input)");
                    }
                    else if (strcmp(proto->name->s_repr, "eof") == 0)
                    {
                      cppout.out_s("eof(input)");
                    }
                    cppout.out_s(std::string(proto->name->s_repr) + " ()");
                    last_type = proto->result_type->s_type;
                    last_type_ptr = proto->result_type->s_type_ptr;
                  }
                  (yyval.type) = last_type;
                }
#line 3804 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1967 "gram.y" /* yacc.c:1646  */
    {
                  if ((yyvsp[-1].sym)->s_kind != FUNCTION_IDENTIFIER)
                  {
                    c4p_warning("`%s' is not a function identifier", (yyvsp[-1].sym)->s_repr);
                  }
                  else if ((yyvsp[-1].sym)->s_type != PROTOTYPE_NODE)
                  {
                    c4p_error("internal error: `%1' has no type", (yyvsp[-1].sym)->s_repr);
                  }
                  cppout.out_s(std::string((yyvsp[-1].sym)->s_repr) + " (");
                  push_parameter_node(last_parameter);
                  if ((yyvsp[-1].sym)->s_kind == FUNCTION_IDENTIFIER)
                  {
                    last_parameter = (reinterpret_cast<prototype_node *>((yyvsp[-1].sym)->s_type_ptr)->formal_parameter);
                  }
                  else
                  {
                    last_parameter = nullptr;
                  }
                }
#line 3829 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1988 "gram.y" /* yacc.c:1646  */
    {
                  prototype_node * proto = reinterpret_cast<prototype_node*>((yyvsp[-4].sym)->s_type_ptr);
                  cppout.out_s(")");
                  pop_parameter_node(last_parameter);
                  if ((yyvsp[-4].sym)->s_kind == FUNCTION_IDENTIFIER)
                  {
                    (yyval.type) = last_type = proto->result_type->s_type;
                    last_type_ptr = proto->result_type->s_type_ptr;
                  }
                  else
                  {
                    (yyval.type) = last_type = INTEGER_TYPE;
                    last_type_ptr = nullptr;
                  }
                }
#line 3849 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 2007 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(" (");
                }
#line 3857 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 2011 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(") ");
                  (yyval.type) = (yyvsp[-1].type);
                }
#line 3866 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 2016 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s("! ");
                }
#line 3874 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 2020 "gram.y" /* yacc.c:1646  */
    {
                  (yyval.type) = (yyvsp[0].type);
                }
#line 3882 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 2033 "gram.y" /* yacc.c:1646  */
    {
                  cppout.out_s(", ");
                }
#line 3890 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 2041 "gram.y" /* yacc.c:1646  */
    {
                  if (last_parameter != nullptr)
                  {
                     last_parameter = last_parameter->next;
                  }
                }
#line 3901 "gram.tab.c" /* yacc.c:1646  */
    break;


#line 3905 "gram.tab.c" /* yacc.c:1646  */
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
