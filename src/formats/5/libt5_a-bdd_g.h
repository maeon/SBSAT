/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTNUMBER = 258,
     ID = 259,
     S_OP = 260,
     OP = 261,
     OP_ITE = 262,
     U_OP = 263,
     BDDID = 264,
     P_BDD = 265,
     ADD_STATE = 266,
     STRING = 267,
     C_OP = 268
   };
#endif
#define INTNUMBER 258
#define ID 259
#define S_OP 260
#define OP 261
#define OP_ITE 262
#define U_OP 263
#define BDDID 264
#define P_BDD 265
#define ADD_STATE 266
#define STRING 267
#define C_OP 268




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 16 "bdd_g.yy"
typedef union YYSTYPE {
    int  num;      /* For returning numbers.               */
    char id[200];  /* For returning ids.                   */
} YYSTYPE;
/* Line 1248 of yacc.c.  */
#line 67 "libt5_a-bdd_g.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE bdd_lval;



