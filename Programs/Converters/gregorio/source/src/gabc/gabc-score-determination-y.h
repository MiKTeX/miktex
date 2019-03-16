/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED
# define YY_GABC_SCORE_DETERMINATION_GABC_GABC_SCORE_DETERMINATION_Y_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gabc_score_determination_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NAME = 258,
    AUTHOR = 259,
    GABC_COPYRIGHT = 260,
    SCORE_COPYRIGHT = 261,
    LANGUAGE = 262,
    STAFF_LINES = 263,
    ORISCUS_ORIENTATION = 264,
    DEF_MACRO = 265,
    OTHER_HEADER = 266,
    ANNOTATION = 267,
    MODE = 268,
    MODE_MODIFIER = 269,
    MODE_DIFFERENTIA = 270,
    END_OF_DEFINITIONS = 271,
    END_OF_FILE = 272,
    COLON = 273,
    SEMICOLON = 274,
    CHARACTERS = 275,
    NOTES = 276,
    HYPHEN = 277,
    ATTRIBUTE = 278,
    OPENING_BRACKET = 279,
    CLOSING_BRACKET = 280,
    CLOSING_BRACKET_WITH_SPACE = 281,
    I_BEGIN = 282,
    I_END = 283,
    TT_BEGIN = 284,
    TT_END = 285,
    UL_BEGIN = 286,
    UL_END = 287,
    C_BEGIN = 288,
    C_END = 289,
    B_BEGIN = 290,
    B_END = 291,
    SC_BEGIN = 292,
    SC_END = 293,
    SP_BEGIN = 294,
    SP_END = 295,
    VERB_BEGIN = 296,
    VERB_END = 297,
    CENTER_BEGIN = 298,
    CENTER_END = 299,
    ELISION_BEGIN = 300,
    ELISION_END = 301,
    TRANSLATION_BEGIN = 302,
    TRANSLATION_END = 303,
    TRANSLATION_CENTER_END = 304,
    ALT_BEGIN = 305,
    ALT_END = 306,
    NLBA_B = 307,
    NLBA_E = 308,
    EUOUAE_B = 309,
    EUOUAE_E = 310,
    NABC_CUT = 311,
    NABC_LINES = 312,
    CLEAR = 313,
    PROTRUSION = 314,
    PROTRUSION_VALUE = 315,
    PROTRUSION_END = 316,
    PROTRUDING_PUNCTUATION = 317
  };
#endif
/* Tokens.  */
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
