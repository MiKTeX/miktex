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
#if defined(MIKTEX) && defined(THIS)
#  undef THIS
#endif
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
#line 60 "camp.y" /* yacc.c:1909  */

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

#line 159 "camp.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_CAMP_TAB_H_INCLUDED  */
