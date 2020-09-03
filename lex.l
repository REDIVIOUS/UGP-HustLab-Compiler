%{
#include "parser.tab.h"
#include "string.h"
#include "def.h"
void atostring(char *, char *);   //将yytext保存到string中
int yycolumn=1;
#define YY_USER_ACTION    	yylloc.first_line=yylloc.last_line=yylineno; \
	yylloc.first_column=yycolumn;	yylloc.last_column=yycolumn+yyleng-1; yycolumn+=yyleng;
typedef union {
	int type_int;
	int type_float;
	char type_char;   //加入char类型
	char type_string[32];  //加入string类型
	char type_id[32];
	struct node *ptr;
} YYLVAL;
#define YYSTYPE YYLVAL

%}
/*内部变量，记录当前的行号*/
%option yylineno

id   [A-Za-z][A-Za-z0-9]*  
int    [0-9]+
float  ([0-9]*\.[0-9]+)|([0-9]+\.)
char   '.'
string   \"(.)*\"
linecomment   \/\/(.)*
blockcomment   \/\*(.|\n)*\*\/

%%
{int}        {yylval.type_int=atoi(yytext); return INT; /*将整常数的值保存在yylval的成员type_int中*/ /*atoi将字符串转换为整数*/}
{float}      {yylval.type_float=atof(yytext); return FLOAT;}
{char}       {yylval.type_char=yytext[1]; return CHAR;}
{string}     {atostring(yytext,yylval.type_string); return STRING;}
"int"        {strcpy(yylval.type_id,  yytext); return TYPE;}
"float"      {strcpy(yylval.type_id,  yytext); return TYPE;}
"char"       {strcpy(yylval.type_id,  yytext); return TYPE;}
"string"     {strcpy(yylval.type_id,  yytext); return TYPE;}

"struct"     {return STRUCT;}
"return"     {return RETURN;}
"if"         {return IF;}
"else"       {return ELSE;}
"while"      {return WHILE;}
"break"      {return BREAK;}
"continue"   {return CONTINUE;}
"for"        {return FOR;}


{id}        {strcpy(yylval.type_id,yytext); return ID; /*由于关键字的形式也符合标识符的规则，所以把关键字的处理全部放在标识符的前面，优先识别*/}
";"			{return SEMI;}
","			{return COMMA;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id, yytext); return RELOP;}
"="			{return ASSIGNOP;}
"+"			{return PLUS;}
"-"			{return MINUS;}
"*"			{return STAR;}
"/"			{return DIV;}
"&&"        {return AND;}
"||"        {return OR;}
"!"			{return NOT;}
"("			{return LP;}
")"			{return RP;}
"{"			{return LC;}
"}"			{return RC;}
"["         {return LB;}
"]"         {return RB;}
"++"        {return DPLUS;}
"--"        {return DMINUS;}
"."         {return DOT;}
[\n]        {yycolumn=1;}
[ \r\t]     {}   
{linecomment}   {}
{blockcomment}  {}
.			{printf("Error type A :Mysterious character \"%s\"\n\t at Line %d\n",yytext,yylineno);}
%%
/*string类型的转换*/
void atostring(char *str, char *result) {
	int i = 0;
	for(i = 1; i < strlen(str) - 1; i++){
		result[i-1] = str[i];
	}
	result[i] = '\0';
}
/* 和bison联用时，不需要这部分
void main()
{
yylex();
return 0;
}
*/

int yywrap()
{
return 1;
}