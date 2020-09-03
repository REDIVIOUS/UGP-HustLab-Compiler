/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     ID = 259,
     RELOP = 260,
     TYPE = 261,
     FLOAT = 262,
     CHAR = 263,
     STRING = 264,
     LP = 265,
     RP = 266,
     LB = 267,
     RB = 268,
     LC = 269,
     RC = 270,
     SEMI = 271,
     COMMA = 272,
     ASSIGNOP = 273,
     PLUS = 274,
     MINUS = 275,
     STAR = 276,
     DIV = 277,
     AND = 278,
     OR = 279,
     NOT = 280,
     DPLUS = 281,
     DMINUS = 282,
     UMINUS = 283,
     STRUCT = 284,
     RETURN = 285,
     IF = 286,
     ELSE = 287,
     WHILE = 288,
     BREAK = 289,
     CONTINUE = 290,
     FOR = 291,
     DOT = 292,
     EXT_DEF_LIST = 293,
     EXT_DEC_LIST = 294,
     EXT_VAR_DEF = 295,
     FUNC_DEF = 296,
     ARGS = 297,
     FUNC_CALL = 298,
     EXT_STRU_DEF = 299,
     STRUTYPE = 300,
     VAR_ARR = 301,
     FUNC_DEC = 302,
     PARAM_LIST = 303,
     PARAM_DEC = 304,
     COMP_STM = 305,
     STM_LIST = 306,
     EXP_STMT = 307,
     IF_THEN = 308,
     IF_THEN_ELSE = 309,
     DEF_LIST = 310,
     VAR_DEF = 311,
     ARR_DEF = 312,
     DEC_LIST = 313,
     VAR_STRU = 314,
     FUNCTION = 315,
     PARAM = 316,
     LABEL = 317,
     GOTO = 318,
     JLE = 319,
     JLT = 320,
     JGE = 321,
     JGT = 322,
     EQ = 323,
     NEQ = 324,
     ARG = 325,
     CALL = 326,
     ARRAYOP = 327,
     ARR_ASSIGN = 328,
     LOWER_THEN_ELSE = 329
   };
#endif
/* Tokens.  */
#define INT 258
#define ID 259
#define RELOP 260
#define TYPE 261
#define FLOAT 262
#define CHAR 263
#define STRING 264
#define LP 265
#define RP 266
#define LB 267
#define RB 268
#define LC 269
#define RC 270
#define SEMI 271
#define COMMA 272
#define ASSIGNOP 273
#define PLUS 274
#define MINUS 275
#define STAR 276
#define DIV 277
#define AND 278
#define OR 279
#define NOT 280
#define DPLUS 281
#define DMINUS 282
#define UMINUS 283
#define STRUCT 284
#define RETURN 285
#define IF 286
#define ELSE 287
#define WHILE 288
#define BREAK 289
#define CONTINUE 290
#define FOR 291
#define DOT 292
#define EXT_DEF_LIST 293
#define EXT_DEC_LIST 294
#define EXT_VAR_DEF 295
#define FUNC_DEF 296
#define ARGS 297
#define FUNC_CALL 298
#define EXT_STRU_DEF 299
#define STRUTYPE 300
#define VAR_ARR 301
#define FUNC_DEC 302
#define PARAM_LIST 303
#define PARAM_DEC 304
#define COMP_STM 305
#define STM_LIST 306
#define EXP_STMT 307
#define IF_THEN 308
#define IF_THEN_ELSE 309
#define DEF_LIST 310
#define VAR_DEF 311
#define ARR_DEF 312
#define DEC_LIST 313
#define VAR_STRU 314
#define FUNCTION 315
#define PARAM 316
#define LABEL 317
#define GOTO 318
#define JLE 319
#define JLT 320
#define JGE 321
#define JGT 322
#define EQ 323
#define NEQ 324
#define ARG 325
#define CALL 326
#define ARRAYOP 327
#define ARR_ASSIGN 328
#define LOWER_THEN_ELSE 329




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 16 "parser.y"
{ //语义值的类型定义
	int    type_int;
	float  type_float;
        char   type_char;
	char   type_id[32];
        char   type_string[32];
	struct ASTNode *ptr;
}
/* Line 1529 of yacc.c.  */
#line 206 "parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;
