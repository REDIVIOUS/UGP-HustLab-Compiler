%error-verbose   /*给出错误的性质*/
%locations       /*方便给当前单词位置的出行号和列号*/
%{
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "def.h"
extern int yylineno;   /*当前行号*/
extern char *yytext;
extern FILE *yyin;
void yyerror(const char* fmt, ...);
void display(struct ASTNode *,int);
void semantic_Analysis(struct ASTNode *T);
%}

%union { //语义值的类型定义
	int    type_int;
	float  type_float;
        char   type_char;
	char   type_id[32];
        char   type_string[32];
	struct ASTNode *ptr;
};

// %type 定义非终结符的语义值类型
%type <ptr> program ExtDefList ExtDef Specifier StructSpecifier StructName ExtDecList VarDec FuncDec VarList ParamDec CompSt StmList Stmt DefList Def DecList Dec Exp Args 

// %token 定义终结符的语义值类型
%token <type_int> INT              /*指定INT的语义值是type_int，有词法分析得到的数值*/
%token <type_id> ID RELOP TYPE    /*指定ID,RELOP 的语义值是type_id，有词法分析得到的标识符字符串*/
%token <type_float> FLOAT          /*指定ID的语义值是type_id，有词法分析得到的标识符字符串*/
%token <type_char> CHAR           /*指定CHAR的语义值为type_char，由词法分析得到字符*/
%token <type_string> STRING       /*指定STRING的语义值为type_string，由词法分析得到字符串*/

%token LP RP LB RB LC RC SEMI COMMA /*左右小括号、中括号、大括号、分号和逗号*/
%token ASSIGNOP PLUS MINUS STAR DIV AND OR NOT DPLUS DMINUS UMINUS /*各种运算符号*/
%token STRUCT RETURN IF ELSE WHILE BREAK CONTINUE FOR DOT /*各种结构有关的符号*/

// 以下为接在上述token后依次编码的枚举常量，作为AST结点类型标记
%token EXT_DEF_LIST EXT_DEC_LIST EXT_VAR_DEF FUNC_DEF ARGS FUNC_CALL EXT_STRU_DEF STRUTYPE VAR_ARR FUNC_DEC PARAM_LIST PARAM_DEC COMP_STM STM_LIST EXP_STMT IF_THEN IF_THEN_ELSE DEF_LIST VAR_DEF ARR_DEF DEC_LIST VAR_STRU

// 生成中间代码时候所需要的标记
%token FUNCTION PARAM LABEL GOTO JLE JLT JGE JGT EQ NEQ ARG CALL ARRAYOP ARR_ASSIGN

// 定义优先级和结合性
%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left DPLUS DMINUS /*自增自减的左结合*/
%left PLUS MINUS /*加减的左结合*/ 
%left STAR DIV /*乘除法的左结合*/
%right UMINUS NOT /*负号、非运算*/
%left DOT LB RB /*成员运算最高优先级*/

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE
%%
/*显示语法树,语义分析*/
program: ExtDefList    {semantic_Analysis0($1);}
       ;

/*外部定义*/
/*多个外部定义（外部变量和函数）*/
ExtDefList: {$$=NULL;}
          | ExtDef ExtDefList {$$=mknode(2,EXT_DEF_LIST,yylineno,$1,$2);}   //每一个EXT_DEF_LIST的结点，其第1棵子树对应一个外部变量声明或函数
          ;
/*外部定义单句*/
ExtDef: Specifier ExtDecList SEMI   {$$=mknode(2,EXT_VAR_DEF,yylineno,$1,$2);}  //该结点对应一个外部变量声明
      | Specifier FuncDec CompSt    {$$=mknode(3,FUNC_DEF,yylineno,$1,$2,$3);}  //该结点对应一个函数定义
      | StructSpecifier VarDec SEMI {$$=mknode(2,EXT_STRU_DEF,yylineno,$1,$2);$$->type=STRUCT;}  //该结点对应一个结构体声明
      | error SEMI   {$$=NULL;}
      ;      
/*类型描述符*/
/*普通变量描述符*/
Specifier: TYPE {$$=mknode(0,TYPE,yylineno);strcpy($$->type_id,$1);
        if(!strcmp($1,"int")) $$->type=INT;
        if(!strcmp($1,"float")) $$->type=FLOAT;
	if(!strcmp($1,"char")) $$->type=CHAR;
        if(!strcmp($1,"string")) $$->type=STRING;}
        ;
/*结构体描述符*/
StructSpecifier: STRUCT StructName LC ExtDefList RC {$$=mknode(2,STRUTYPE,yylineno,$2,$4);$$->type=STRUCT;}
               | STRUCT ID {$$=mknode(0,STRUTYPE,yylineno);strcpy($$->type_id,$2);}
               ;
StructName: {$$=NULL;}
          | ID {$$=mknode(0,ID,yylineno);strcpy($$->type_id,$1);}   //ID结点，标识符符号串存放结点的type_id
          ;
/*逗号分隔多个变量*/
ExtDecList: VarDec {$$=$1;} /*每一个EXT_DEC_LIST的结点，其第一棵子树对应一个变量名(ID类型的结点),第二棵子树对应剩下的外部变量名*/
          | VarDec COMMA ExtDecList {$$=mknode(2,EXT_DEC_LIST,yylineno,$1,$3);}
          ;
/*一个变量*/ 
VarDec: ID {$$=mknode(0,ID,yylineno);strcpy($$->type_id,$1);}   //ID结点，标识符符号串存放结点的type_id
      | VarDec LB Exp RB {$$=mknode(2,VAR_ARR,yylineno,$1,$3);} //声明多维数组
      ;
/*函数的声明*/
FuncDec: ID LP VarList RP{$$=mknode(1,FUNC_DEC,yylineno,$3);strcpy($$->type_id,$1);}  //函数名存放在$$->type_id
       | ID LP RP {$$=mknode(0,FUNC_DEC,yylineno);strcpy($$->type_id,$1);$$->ptr[0]=NULL;}  //函数名存放在$$->type_id
       ;
/*形式参数列表*/
VarList: ParamDec {$$=mknode(1,PARAM_LIST,yylineno,$1);}
       | ParamDec COMMA VarList {$$=mknode(2,PARAM_LIST,yylineno,$1,$3);}
       ;
/*形参的定义*/
ParamDec: Specifier VarDec {$$=mknode(2,PARAM_DEC,yylineno,$1,$2);}
        ;
/*复合语句（花括号语句块）*/
CompSt: LC DefList StmList RC {$$=mknode(2,COMP_STM,yylineno,$2,$3);}
      ;
/*多条语句*/
StmList: {$$=NULL; }  
       | Stmt StmList  {$$=mknode(2,STM_LIST,yylineno,$1,$2);}
       ;
/*一条语句*/
Stmt: Exp SEMI    {$$=mknode(1,EXP_STMT,yylineno,$1);}
    | CompSt      {$$=$1;}      //复合语句结点直接最为语句结点，不再生成新的结点
    | RETURN Exp SEMI   {$$=mknode(1,RETURN,yylineno,$2);}
    | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE   {$$=mknode(2,IF_THEN,yylineno,$3,$5);} //这条语句优先级在else后面
    | IF LP Exp RP Stmt ELSE Stmt   {$$=mknode(3,IF_THEN_ELSE,yylineno,$3,$5,$7);}
    | WHILE LP Exp RP Stmt {$$=mknode(2,WHILE,yylineno,$3,$5);}
    | FOR LP Exp SEMI Exp SEMI Exp RP Stmt {$$=mknode(4,FOR,yylineno,$3,$5,$7,$9);}
    | BREAK SEMI {$$=mknode(0,BREAK,yylineno);}
    | CONTINUE SEMI {$$=mknode(0,CONTINUE,yylineno);}
    ;
/*多个局部变量的定义*/
DefList: {$$=NULL; }
       | Def DefList {$$=mknode(2,DEF_LIST,yylineno,$1,$2);}
       | error SEMI   {$$=NULL;}
       ;
/*局部变量的定义(包括数组)单句*/
Def: Specifier DecList SEMI {$$=mknode(2,VAR_DEF,yylineno,$1,$2);}
   ;
/*多个局部变量*/ 
DecList: Dec {$$=mknode(1,DEC_LIST,yylineno,$1);}
       | Dec COMMA DecList  {$$=mknode(2,DEC_LIST,yylineno,$1,$3);}
       ;
/*局部变量单句*/
Dec: VarDec  {$$=$1;}
   | VarDec ASSIGNOP Exp  {$$=mknode(2,ASSIGNOP,yylineno,$1,$3);strcpy($$->type_id,"ASSIGNOP");}
   ;
/*一个表达式*/
Exp: Exp ASSIGNOP Exp {$$=mknode(2,ASSIGNOP,yylineno,$1,$3);strcpy($$->type_id,"ASSIGNOP");} //$$结点type_id空置未用，正好存放运算符
   | Exp AND Exp   {$$=mknode(2,AND,yylineno,$1,$3);strcpy($$->type_id,"AND");}
   | Exp OR Exp    {$$=mknode(2,OR,yylineno,$1,$3);strcpy($$->type_id,"OR");}
   | Exp RELOP Exp {$$=mknode(2,RELOP,yylineno,$1,$3);strcpy($$->type_id,$2);}  //词法分析关系运算符号自身值保存在$2中
   | Exp PLUS Exp  {$$=mknode(2,PLUS,yylineno,$1,$3);strcpy($$->type_id,"PLUS");}
   | Exp MINUS Exp {$$=mknode(2,MINUS,yylineno,$1,$3);strcpy($$->type_id,"MINUS");}
   | Exp STAR Exp  {$$=mknode(2,STAR,yylineno,$1,$3);strcpy($$->type_id,"STAR");}
   | Exp DIV Exp   {$$=mknode(2,DIV,yylineno,$1,$3);strcpy($$->type_id,"DIV");}
   | LP Exp RP     {$$=$2;}
   | MINUS Exp %prec UMINUS   {$$=mknode(1,UMINUS,yylineno,$2);strcpy($$->type_id,"UMINUS");}
   | NOT Exp       {$$=mknode(1,NOT,yylineno,$2);strcpy($$->type_id,"NOT");}
   | ID LP Args RP {$$=mknode(1,FUNC_CALL,yylineno,$3);strcpy($$->type_id,$1);}
   | ID LP RP      {$$=mknode(0,FUNC_CALL,yylineno);strcpy($$->type_id,$1);}
   | ID            {$$=mknode(0,ID,yylineno);strcpy($$->type_id,$1);}
   | INT           {$$=mknode(0,INT,yylineno);$$->type_int=$1;$$->type=INT;}
   | FLOAT         {$$=mknode(0,FLOAT,yylineno);$$->type_float=$1;$$->type=FLOAT;}
   | CHAR          {$$=mknode(0,CHAR,yylineno);$$->type_char=$1;$$->type=CHAR;}
   | STRING        {$$=mknode(0,STRING,yylineno);strcpy($$->type_string,$1);$$->type=STRING;}
   | Exp DOT ID    {$$=mknode(1,VAR_STRU,yylineno,$1);strcpy($$->type_id,$3);} //结构体成员访问
   | Exp LB Exp RB {$$=mknode(2,VAR_ARR,yylineno,$1,$3);} //多维数组
   | DPLUS Exp     {$$=mknode(1,DPLUS,yylineno,$2);strcpy($$->type_id,"DPLUS");}
   | Exp DPLUS     {$$=mknode(1,DPLUS,yylineno,$1);strcpy($$->type_id,"DPLUS");}
   | DMINUS Exp    {$$=mknode(1,DMINUS,yylineno,$2);strcpy($$->type_id,"DMINUS");}
   | Exp DMINUS    {$$=mknode(1,DMINUS,yylineno,$1);strcpy($$->type_id,"DMINUS");}
   ;

/*形式参数列表*/
Args: Exp COMMA Args    {$$=mknode(2,ARGS,yylineno,$1,$3);}
    | Exp               {$$=mknode(1,ARGS,yylineno,$1);}
    ;     
%%

int main(int argc, char *argv[]){
	yyin=fopen(argv[1],"r");
	if (!yyin) return 1;
	yylineno=1;
	yyparse();
	return 0;
	}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}