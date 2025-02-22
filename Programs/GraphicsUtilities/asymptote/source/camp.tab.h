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

#ifndef YY_YY_CAMP_TAB_H_INCLUDED
# define YY_YY_CAMP_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
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
    ID = 258,                      /* ID  */
    SELFOP = 259,                  /* SELFOP  */
    DOTS = 260,                    /* DOTS  */
    COLONS = 261,                  /* COLONS  */
    DASHES = 262,                  /* DASHES  */
    INCR = 263,                    /* INCR  */
    LONGDASH = 264,                /* LONGDASH  */
    CONTROLS = 265,                /* CONTROLS  */
    TENSION = 266,                 /* TENSION  */
    ATLEAST = 267,                 /* ATLEAST  */
    CURL = 268,                    /* CURL  */
    COR = 269,                     /* COR  */
    CAND = 270,                    /* CAND  */
    BAR = 271,                     /* BAR  */
    AMPERSAND = 272,               /* AMPERSAND  */
    EQ = 273,                      /* EQ  */
    NEQ = 274,                     /* NEQ  */
    LT = 275,                      /* LT  */
    LE = 276,                      /* LE  */
    GT = 277,                      /* GT  */
    GE = 278,                      /* GE  */
    CARETS = 279,                  /* CARETS  */
    OPERATOR = 280,                /* OPERATOR  */
    LOOSE = 281,                   /* LOOSE  */
    ASSIGN = 282,                  /* ASSIGN  */
    DIRTAG = 283,                  /* DIRTAG  */
    JOIN_PREC = 284,               /* JOIN_PREC  */
    AND = 285,                     /* AND  */
    ELLIPSIS = 286,                /* ELLIPSIS  */
    ACCESS = 287,                  /* ACCESS  */
    UNRAVEL = 288,                 /* UNRAVEL  */
    IMPORT = 289,                  /* IMPORT  */
    INCLUDE = 290,                 /* INCLUDE  */
    FROM = 291,                    /* FROM  */
    QUOTE = 292,                   /* QUOTE  */
    STRUCT = 293,                  /* STRUCT  */
    TYPEDEF = 294,                 /* TYPEDEF  */
    USING = 295,                   /* USING  */
    NEW = 296,                     /* NEW  */
    IF = 297,                      /* IF  */
    ELSE = 298,                    /* ELSE  */
    WHILE = 299,                   /* WHILE  */
    DO = 300,                      /* DO  */
    FOR = 301,                     /* FOR  */
    BREAK = 302,                   /* BREAK  */
    CONTINUE = 303,                /* CONTINUE  */
    RETURN_ = 304,                 /* RETURN_  */
    THIS_TOK = 305,                /* THIS_TOK  */
    EXPLICIT = 306,                /* EXPLICIT  */
    GARBAGE = 307,                 /* GARBAGE  */
    LIT = 308,                     /* LIT  */
    STRING = 309,                  /* STRING  */
    PERM = 310,                    /* PERM  */
    MODIFIER = 311,                /* MODIFIER  */
    UNARY = 312,                   /* UNARY  */
    EXP_IN_PARENS_RULE = 313       /* EXP_IN_PARENS_RULE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 60 "camp.y"

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
  absyntax::astType  *t;
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
  absyntax::typeParam *tp;
  absyntax::typeParamList *tps;

#line 171 "camp.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_CAMP_TAB_H_INCLUDED  */
