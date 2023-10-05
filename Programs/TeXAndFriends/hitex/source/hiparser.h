/* A Bison parser, made by GNU Bison 3.8.2.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
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
    START = 258,                   /* "<"  */
    END = 259,                     /* ">"  */
    GLYPH = 260,                   /* "glyph"  */
    UNSIGNED = 261,                /* UNSIGNED  */
    REFERENCE = 262,               /* REFERENCE  */
    SIGNED = 263,                  /* SIGNED  */
    STRING = 264,                  /* STRING  */
    CHARCODE = 265,                /* CHARCODE  */
    FPNUM = 266,                   /* FPNUM  */
    DIMEN = 267,                   /* "dimen"  */
    PT = 268,                      /* "pt"  */
    MM = 269,                      /* "mm"  */
    INCH = 270,                    /* "in"  */
    XDIMEN = 271,                  /* "xdimen"  */
    H = 272,                       /* "h"  */
    V = 273,                       /* "v"  */
    FIL = 274,                     /* "fil"  */
    FILL = 275,                    /* "fill"  */
    FILLL = 276,                   /* "filll"  */
    PENALTY = 277,                 /* "penalty"  */
    INTEGER = 278,                 /* "int"  */
    LANGUAGE = 279,                /* "language"  */
    RULE = 280,                    /* "rule"  */
    RUNNING = 281,                 /* "|"  */
    KERN = 282,                    /* "kern"  */
    EXPLICIT = 283,                /* "!"  */
    GLUE = 284,                    /* "glue"  */
    PLUS = 285,                    /* "plus"  */
    MINUS = 286,                   /* "minus"  */
    TXT_START = 287,               /* TXT_START  */
    TXT_END = 288,                 /* TXT_END  */
    TXT_IGNORE = 289,              /* TXT_IGNORE  */
    TXT_FONT_GLUE = 290,           /* TXT_FONT_GLUE  */
    TXT_FONT_HYPHEN = 291,         /* TXT_FONT_HYPHEN  */
    TXT_FONT = 292,                /* TXT_FONT  */
    TXT_LOCAL = 293,               /* TXT_LOCAL  */
    TXT_GLOBAL = 294,              /* TXT_GLOBAL  */
    TXT_CC = 295,                  /* TXT_CC  */
    HBOX = 296,                    /* "hbox"  */
    VBOX = 297,                    /* "vbox"  */
    SHIFTED = 298,                 /* "shifted"  */
    HPACK = 299,                   /* "hpack"  */
    HSET = 300,                    /* "hset"  */
    VPACK = 301,                   /* "vpack"  */
    VSET = 302,                    /* "vset"  */
    DEPTH = 303,                   /* "depth"  */
    ADD = 304,                     /* "add"  */
    TO = 305,                      /* "to"  */
    LEADERS = 306,                 /* "leaders"  */
    ALIGN = 307,                   /* "align"  */
    CENTER = 308,                  /* "center"  */
    EXPAND = 309,                  /* "expand"  */
    BASELINE = 310,                /* "baseline"  */
    LIGATURE = 311,                /* "ligature"  */
    DISC = 312,                    /* "disc"  */
    PAR = 313,                     /* "par"  */
    MATH = 314,                    /* "math"  */
    ON = 315,                      /* "on"  */
    OFF = 316,                     /* "off"  */
    ADJUST = 317,                  /* "adjust"  */
    TABLE = 318,                   /* "table"  */
    ITEM = 319,                    /* "item"  */
    IMAGE = 320,                   /* "image"  */
    WIDTH = 321,                   /* "width"  */
    HEIGHT = 322,                  /* "height"  */
    LABEL = 323,                   /* "label"  */
    BOT = 324,                     /* "bot"  */
    MID = 325,                     /* "mid"  */
    LINK = 326,                    /* "link"  */
    OUTLINE = 327,                 /* "outline"  */
    UNKNOWN = 328,                 /* "unknown"  */
    STREAM = 329,                  /* "stream"  */
    STREAMDEF = 330,               /* "stream (definition)"  */
    FIRST = 331,                   /* "first"  */
    LAST = 332,                    /* "last"  */
    TOP = 333,                     /* "top"  */
    NOREFERENCE = 334,             /* "*"  */
    PAGE = 335,                    /* "page"  */
    RANGE = 336,                   /* "range"  */
    DIRECTORY = 337,               /* "directory"  */
    SECTION = 338,                 /* "entry"  */
    DEFINITIONS = 339,             /* "definitions"  */
    MAX = 340,                     /* "max"  */
    PARAM = 341,                   /* "param"  */
    FONT = 342,                    /* "font"  */
    CONTENT = 343                  /* "content"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 79 "parser.y"

	#line 11164 "format.w"
	uint32_t u;  int32_t i;  char *s;  float64_t f;  Glyph c;
	Dimen d;Stretch st;Xdimen xd;Kern kt;
	Rule r;Glue g;Image x;
	List l;Box h;Disc dc;Lig lg;
	Ref rf;Info info;Order o;bool b;
	

#line 162 "hiparser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_H_INCLUDED  */
