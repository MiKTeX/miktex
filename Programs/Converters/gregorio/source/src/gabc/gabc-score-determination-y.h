/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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
    INITIAL_STYLE = 271,
    END_OF_DEFINITIONS = 272,
    END_OF_FILE = 273,
    COLON = 274,
    SEMICOLON = 275,
    SPACE = 276,
    CHARACTERS = 277,
    NOTES = 278,
    HYPHEN = 279,
    ATTRIBUTE = 280,
    OPENING_BRACKET = 281,
    CLOSING_BRACKET = 282,
    CLOSING_BRACKET_WITH_SPACE = 283,
    I_BEGIN = 284,
    I_END = 285,
    TT_BEGIN = 286,
    TT_END = 287,
    UL_BEGIN = 288,
    UL_END = 289,
    C_BEGIN = 290,
    C_END = 291,
    B_BEGIN = 292,
    B_END = 293,
    SC_BEGIN = 294,
    SC_END = 295,
    SP_BEGIN = 296,
    SP_END = 297,
    VERB_BEGIN = 298,
    VERB = 299,
    VERB_END = 300,
    CENTER_BEGIN = 301,
    CENTER_END = 302,
    ELISION_BEGIN = 303,
    ELISION_END = 304,
    TRANSLATION_BEGIN = 305,
    TRANSLATION_END = 306,
    TRANSLATION_CENTER_END = 307,
    ALT_BEGIN = 308,
    ALT_END = 309,
    NLBA_B = 310,
    NLBA_E = 311,
    EUOUAE_B = 312,
    EUOUAE_E = 313,
    NABC_CUT = 314,
    NABC_LINES = 315
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
#define INITIAL_STYLE 271
#define END_OF_DEFINITIONS 272
#define END_OF_FILE 273
#define COLON 274
#define SEMICOLON 275
#define SPACE 276
#define CHARACTERS 277
#define NOTES 278
#define HYPHEN 279
#define ATTRIBUTE 280
#define OPENING_BRACKET 281
#define CLOSING_BRACKET 282
#define CLOSING_BRACKET_WITH_SPACE 283
#define I_BEGIN 284
#define I_END 285
#define TT_BEGIN 286
#define TT_END 287
#define UL_BEGIN 288
#define UL_END 289
#define C_BEGIN 290
#define C_END 291
#define B_BEGIN 292
#define B_END 293
#define SC_BEGIN 294
#define SC_END 295
#define SP_BEGIN 296
#define SP_END 297
#define VERB_BEGIN 298
#define VERB 299
#define VERB_END 300
#define CENTER_BEGIN 301
#define CENTER_END 302
#define ELISION_BEGIN 303
#define ELISION_END 304
#define TRANSLATION_BEGIN 305
#define TRANSLATION_END 306
#define TRANSLATION_CENTER_END 307
#define ALT_BEGIN 308
#define ALT_END 309
#define NLBA_B 310
#define NLBA_E 311
#define EUOUAE_B 312
#define EUOUAE_E 313
#define NABC_CUT 314
#define NABC_LINES 315

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
