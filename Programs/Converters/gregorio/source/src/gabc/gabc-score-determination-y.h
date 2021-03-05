/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED
# define YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gabc_score_determination_debug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NAME = 258,                    /* NAME  */
    AUTHOR = 259,                  /* AUTHOR  */
    GABC_COPYRIGHT = 260,          /* GABC_COPYRIGHT  */
    SCORE_COPYRIGHT = 261,         /* SCORE_COPYRIGHT  */
    LANGUAGE = 262,                /* LANGUAGE  */
    STAFF_LINES = 263,             /* STAFF_LINES  */
    ORISCUS_ORIENTATION = 264,     /* ORISCUS_ORIENTATION  */
    DEF_MACRO = 265,               /* DEF_MACRO  */
    OTHER_HEADER = 266,            /* OTHER_HEADER  */
    ANNOTATION = 267,              /* ANNOTATION  */
    MODE = 268,                    /* MODE  */
    MODE_MODIFIER = 269,           /* MODE_MODIFIER  */
    MODE_DIFFERENTIA = 270,        /* MODE_DIFFERENTIA  */
    END_OF_DEFINITIONS = 271,      /* END_OF_DEFINITIONS  */
    END_OF_FILE = 272,             /* END_OF_FILE  */
    COLON = 273,                   /* COLON  */
    SEMICOLON = 274,               /* SEMICOLON  */
    CHARACTERS = 275,              /* CHARACTERS  */
    NOTES = 276,                   /* NOTES  */
    HYPHEN = 277,                  /* HYPHEN  */
    ATTRIBUTE = 278,               /* ATTRIBUTE  */
    OPENING_BRACKET = 279,         /* OPENING_BRACKET  */
    CLOSING_BRACKET = 280,         /* CLOSING_BRACKET  */
    CLOSING_BRACKET_WITH_SPACE = 281, /* CLOSING_BRACKET_WITH_SPACE  */
    I_BEGIN = 282,                 /* I_BEGIN  */
    I_END = 283,                   /* I_END  */
    TT_BEGIN = 284,                /* TT_BEGIN  */
    TT_END = 285,                  /* TT_END  */
    UL_BEGIN = 286,                /* UL_BEGIN  */
    UL_END = 287,                  /* UL_END  */
    C_BEGIN = 288,                 /* C_BEGIN  */
    C_END = 289,                   /* C_END  */
    B_BEGIN = 290,                 /* B_BEGIN  */
    B_END = 291,                   /* B_END  */
    SC_BEGIN = 292,                /* SC_BEGIN  */
    SC_END = 293,                  /* SC_END  */
    SP_BEGIN = 294,                /* SP_BEGIN  */
    SP_END = 295,                  /* SP_END  */
    VERB_BEGIN = 296,              /* VERB_BEGIN  */
    VERB_END = 297,                /* VERB_END  */
    CENTER_BEGIN = 298,            /* CENTER_BEGIN  */
    CENTER_END = 299,              /* CENTER_END  */
    ELISION_BEGIN = 300,           /* ELISION_BEGIN  */
    ELISION_END = 301,             /* ELISION_END  */
    TRANSLATION_BEGIN = 302,       /* TRANSLATION_BEGIN  */
    TRANSLATION_END = 303,         /* TRANSLATION_END  */
    TRANSLATION_CENTER_END = 304,  /* TRANSLATION_CENTER_END  */
    ALT_BEGIN = 305,               /* ALT_BEGIN  */
    ALT_END = 306,                 /* ALT_END  */
    NLBA_B = 307,                  /* NLBA_B  */
    NLBA_E = 308,                  /* NLBA_E  */
    EUOUAE_B = 309,                /* EUOUAE_B  */
    EUOUAE_E = 310,                /* EUOUAE_E  */
    NABC_CUT = 311,                /* NABC_CUT  */
    NABC_LINES = 312,              /* NABC_LINES  */
    CLEAR = 313,                   /* CLEAR  */
    PROTRUSION = 314,              /* PROTRUSION  */
    PROTRUSION_VALUE = 315,        /* PROTRUSION_VALUE  */
    PROTRUSION_END = 316,          /* PROTRUSION_END  */
    PROTRUDING_PUNCTUATION = 317   /* PROTRUDING_PUNCTUATION  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define NAME 258
#define AUTHOR 259
#define GABC_COPYRIGHT 260
#define SCORE_COPYRIGHT 261
#define LANGUAGE 262
#define STAFF_LINES 263
#define ORISCUS_ORIENTATION 264
#define DEF_MACRO 265
#define OTHER_HEADER 266
#define ANNOTATION 267
#define MODE 268
#define MODE_MODIFIER 269
#define MODE_DIFFERENTIA 270
#define END_OF_DEFINITIONS 271
#define END_OF_FILE 272
#define COLON 273
#define SEMICOLON 274
#define CHARACTERS 275
#define NOTES 276
#define HYPHEN 277
#define ATTRIBUTE 278
#define OPENING_BRACKET 279
#define CLOSING_BRACKET 280
#define CLOSING_BRACKET_WITH_SPACE 281
#define I_BEGIN 282
#define I_END 283
#define TT_BEGIN 284
#define TT_END 285
#define UL_BEGIN 286
#define UL_END 287
#define C_BEGIN 288
#define C_END 289
#define B_BEGIN 290
#define B_END 291
#define SC_BEGIN 292
#define SC_END 293
#define SP_BEGIN 294
#define SP_END 295
#define VERB_BEGIN 296
#define VERB_END 297
#define CENTER_BEGIN 298
#define CENTER_END 299
#define ELISION_BEGIN 300
#define ELISION_END 301
#define TRANSLATION_BEGIN 302
#define TRANSLATION_END 303
#define TRANSLATION_CENTER_END 304
#define ALT_BEGIN 305
#define ALT_END 306
#define NLBA_B 307
#define NLBA_E 308
#define EUOUAE_B 309
#define EUOUAE_E 310
#define NABC_CUT 311
#define NABC_LINES 312
#define CLEAR 313
#define PROTRUSION 314
#define PROTRUSION_VALUE 315
#define PROTRUSION_END 316
#define PROTRUDING_PUNCTUATION 317

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE gabc_score_determination_lval;
extern YYLTYPE gabc_score_determination_lloc;
int gabc_score_determination_parse (void);

#endif /* !YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED  */
