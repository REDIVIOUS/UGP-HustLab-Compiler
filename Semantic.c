#include "def.h"
int optflag; //是否优化中间代码标志


// 串s1和s2的连接
char *strcat0(char *s1,char *s2){
    static char result[10];
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

// 新别名
char *newAlias() {
    static int no=1;
    char s[10];
    sprintf(s,"%d",no++);
    return strcat0("v",s);
}

// 创建新的标签
char *newLabel() {
    static int no=1;
    char s[10];
    sprintf(s,"%d",no++);
    return strcat0("label",s);
}

// 临时变量
char *newTemp(){
    static int no=1;
    char s[10];
    sprintf(s,"%d",no++);
    return strcat0("temp",s);
}

//生成一条TAC代码的结点组成的双向循环链表，返回头指针
struct codenode *genIR(int op,struct opn opn1,struct opn opn2,struct opn result){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=op;
    h->opn1=opn1;
    h->opn2=opn2;
    h->result=result;
    h->next=h->prior=h;
    return h;
}

//生成一条标号语句，返回头指针
struct codenode *genLabel(char *label){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=LABEL;
    strcpy(h->result.id,label);
    h->next=h->prior=h;
    return h;
}

//生成GOTO语句，返回头指针
struct codenode *genGoto(char *label){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=GOTO;
    strcpy(h->result.id,label);
    h->next=h->prior=h;
    return h;
}

//合并多个中间代码的双向循环链表，首尾相连
struct codenode *merge(int num,...){
    struct codenode *h1,*h2,*p,*t1,*t2;
    va_list ap;
    va_start(ap,num);
    h1=va_arg(ap,struct codenode *);
    while (--num>0) {
        h2=va_arg(ap,struct codenode *);
        if (h1==NULL) h1=h2;
        else if (h2){
            t1=h1->prior;
            t2=h2->prior;
            t1->next=h2;
            t2->next=h1;
            h1->prior=t2;
            h2->prior=t1;
            }
        }
    va_end(ap);
    return h1;
}

//输出中间代码
void prnIR(struct codenode *head){
    char opnstr1[32],opnstr2[32],resultstr[32];
    struct codenode *h=head;
    do {
        //第一操作数
        if (h->opn1.kind==INT)
             sprintf(opnstr1,"#%d",h->opn1.const_int);
        if (h->opn1.kind==FLOAT)
             sprintf(opnstr1,"#%f",h->opn1.const_float);
        if (h->opn1.kind==CHAR)
            sprintf(opnstr1,"#%c",h->opn1.const_char);
        if (h->opn1.kind==ID)
             sprintf(opnstr1,"%s",h->opn1.id);

        //第二操作数
        if (h->opn2.kind==INT)
             sprintf(opnstr2,"#%d",h->opn2.const_int);
        if (h->opn2.kind==FLOAT)
             sprintf(opnstr2,"#%f",h->opn2.const_float);
        if(h->opn2.kind==CHAR)
            sprintf(opnstr2,"#%c",h->opn2.const_char);
        if (h->opn2.kind==ID)
             sprintf(opnstr2,"%s",h->opn2.id);

        // 处理result
        if (h->result.kind==INT)
             sprintf(resultstr,"#%d",h->result.const_int);
        if (h->result.kind==FLOAT)
             sprintf(resultstr,"#%f",h->result.const_float);
        if(h->result.kind==CHAR)
            sprintf(resultstr,"#%c",h->result.const_char);
        if(h->result.kind!=INT&&h->result.kind!=FLOAT&&h->result.kind!=CHAR)
            sprintf(resultstr,"%s",h->result.id);

        // 处理运算符
        switch (h->op) {
            case ASSIGNOP:  printf("  %s := %s\n",resultstr,opnstr1);
                            break;
            // 处理OP 
            case PLUS:
            case MINUS:
            case STAR:
            case DIV: printf("  %s := %s %c %s\n",resultstr,opnstr1, \
                      h->op==PLUS?'+':h->op==MINUS?'-':h->op==STAR?'*':'/',opnstr2);
                      break;
            case DPLUS:    printf("  %s := %s + #1\n",resultstr,resultstr);
                           break;
            case DMINUS:   printf("  %s := %s - #1\n",resultstr,resultstr);
                           break;
            case ARRAYOP:  printf("  %s := %s[%s]\n",resultstr,opnstr1,opnstr2);
                           break;
            case ARR_ASSIGN: printf("  %s[%s] := %s\n",opnstr1,opnstr2,resultstr);
                             break;
            case FUNCTION: printf("\nFUNCTION %s :\n",resultstr);
                           break;
            case PARAM:    printf("  PARAM %s\n",resultstr);
                           break;
            case LABEL:    printf("LABEL %s :\n",resultstr);
                           break;
            case UMINUS:   printf("  %s := -%s\n",resultstr,opnstr1);
                           break;
            case GOTO:     printf("  GOTO %s\n",resultstr);
                           break;
            case JLE:      printf("  IF %s <= %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case JLT:      printf("  IF %s < %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case JGE:      printf("  IF %s >= %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case JGT:      printf("  IF %s > %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case EQ:       printf("  IF %s == %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case NEQ:      printf("  IF %s != %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case ARG:      printf("  ARG %s\n",h->result.id);
                           break;
            case CALL:     if (!strcmp(opnstr1,"write"))
                                printf("  CALL  %s\n", opnstr1);
                            else
                                printf("  %s := CALL %s\n",resultstr, opnstr1);
                           break;
            case RETURN:   if (h->result.kind)
                                printf("  RETURN %s\n",resultstr);
                           else
                                printf("  RETURN\n");
                           break;
        }
    h=h->next;
    } while (h!=head);
}

void semantic_error(int line,char *msg1,char *msg2){
    //这里可以只收集错误信息，最后一次显示
    printf("在%d行,%s %s\n",line,msg1,msg2);
}

// 显示正确的type
const char* symbol_type(int a) {
    switch (a){
        case INT:
            return "int";
        case FLOAT:
            return "float";
        case CHAR:
            return "char";
        case STRUCT:
            return "struct";
        default:
            return "NULL";
    }
}

// 得到正确的label
int symbol_label(char* a){
    if(!strcmp(a,"int")){
        return INT;
    }
    if(!strcmp(a,"float")){
        return FLOAT;
    }
    if(!strcmp(a,"char")){
        return CHAR;
    }
}

// 类型的长度
int symbol_width(int a){
    switch(a){
        case INT:
            return 4;
        case FLOAT:
            return 8;
        case CHAR:
            return 1;
        default:
            return 4;
    }
}

void prn_symbol(){ //显示符号表
    int i=0;
    printf("%6s %6s %6s  %6s %4s %6s\n","变量名","别 名","层 号","类  型","标记","偏移量");
    for(i=0;i<symbolTable.index;i++)
        printf("%6s %6s %6d %6s %4c %6d\n",symbolTable.symbols[i].name,\
                symbolTable.symbols[i].alias,symbolTable.symbols[i].level,\
                symbol_type(symbolTable.symbols[i].type),\
                symbolTable.symbols[i].flag,symbolTable.symbols[i].offset);
        printf("\n");
}

int searchSymbolTable(char *name) {
    int i,flag=0;
    for(i=symbolTable.index-1;i>=0;i--){
        if (symbolTable.symbols[i].level==0)
            flag=1;
        if (flag && symbolTable.symbols[i].level==1)
            continue;   //跳过前面函数的形式参数表项
        if (!strcmp(symbolTable.symbols[i].name, name))  return i;
    }
    return -1;
}

int searchSymbolTablestruct(char *name) {  //专门查找结构体变量（用在测试结构体的地方）
    int i;
    for(i=symbolTable.index-1;i>=0;i--){
        if (symbolTable.symbols[i].level==1 && symbolTable.symbols[i].flag=='V' && !strcmp(symbolTable.symbols[i].name, name)){ //第一层的全局变量，一定是声明过的结构体成员
            return i;
        }
    }
    return -1;
}

int searchSymbolTablearray(char *name){ //专门查找数组
    int i;
    for(i=symbolTable.index-1;i>=0;i--){
        if (symbolTable.symbols[i].flag=='A' && !strcmp(symbolTable.symbols[i].name, name)){
            return i;
        }
    }
    return -1;
}

// 填写符号表内容、符号查重
int fillSymbolTable(char *name,char *alias,int level,int type,char flag,int offset) {
    //首先根据name查符号表，不能重复定义 重复定义返回-1
    int i;
    /*符号查重，考虑外部变量声明前有函数定义，
    其形参名还在符号表中，这时的外部变量与前函数的形参重名是允许的*/
    for(i=symbolTable.index-1; i>=0 && (symbolTable.symbols[i].level==level||level==0); i--) {
        if (level==0 && symbolTable.symbols[i].level==1) continue;  //外部变量和形参不必比较重名
        if (!strcmp(symbolTable.symbols[i].name, name))  return -1;
        }
    //填写符号表内容
    strcpy(symbolTable.symbols[symbolTable.index].name,name);
    strcpy(symbolTable.symbols[symbolTable.index].alias,alias);
    symbolTable.symbols[symbolTable.index].level=level;
    symbolTable.symbols[symbolTable.index].type=type;
    symbolTable.symbols[symbolTable.index].flag=flag;
    symbolTable.symbols[symbolTable.index].offset=offset;
    return symbolTable.index++; //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}

//填写临时变量到符号表，返回临时变量在符号表中的位置
int fill_Temp(char *name,int level,int type,char flag,int offset) {
    strcpy(symbolTable.symbols[symbolTable.index].name,"");
    strcpy(symbolTable.symbols[symbolTable.index].alias,name);
    symbolTable.symbols[symbolTable.index].level=level;
    symbolTable.symbols[symbolTable.index].type=type;
    symbolTable.symbols[symbolTable.index].flag=flag;
    symbolTable.symbols[symbolTable.index].offset=offset;
    return symbolTable.index++; //返回的是临时变量在符号表中的位置序号
}

// 处理变量列表
void ext_var_list(struct ASTNode *T){
    int rtn,num=1;
    switch (T->kind){
        case EXT_DEC_LIST:
            T->ptr[0]->type=T->type;              //将类型属性向下传递变量结点
            T->ptr[0]->offset=T->offset;          //外部变量的偏移量向下传递
            T->ptr[1]->type=T->type;              //将类型属性向下传递变量结点
            T->ptr[1]->offset=T->offset+T->width; //外部变量的偏移量向下传递
            T->ptr[1]->width=T->width;
            ext_var_list(T->ptr[0]);
            ext_var_list(T->ptr[1]);
            T->num=T->ptr[1]->num+1;
            break;
        case VAR_ARR:
            rtn=fillSymbolTable(T->ptr[0]->type_id,newAlias(),LEV,T->type,'A',T->offset);
            if (rtn==-1)
                semantic_error(T->pos,T->type_id, "变量重复定义");
            T->num = T->ptr[1]->type_int;
            symbolTable.symbols[rtn].arraysize = T->num * symbol_width(T->type);
            break;
        case ID:
            rtn=fillSymbolTable(T->type_id,newAlias(),LEV,T->type,'V',T->offset);  //最后一个变量名
            if (rtn==-1)
                semantic_error(T->pos,T->type_id, "变量重复定义");
            T->num=1;
            break;
    }
}

// 处理函数调用参数（类型与数量）
int match_param(int i,struct ASTNode *T){
    int j,num=symbolTable.symbols[i].paramnum;
    int type1,type2,pos=T->pos;
    T=T->ptr[0];
    if (num==0 && T==NULL) return 1;
    for (j=1;j<=num;j++) {
        if (!T){
            semantic_error(pos,"", "函数调用参数太少!");
            return 0;
            }
        type1=symbolTable.symbols[i+j].type;  //形参类型
        type2=T->ptr[0]->type;
        if (type1!=type2){
            semantic_error(pos,"", "参数类型不匹配");
            return 0;
        }
        T=T->ptr[1];
    }
    if (T){ //num个参数已经匹配完，还有实参表达式
        semantic_error(pos,"", "函数调用参数太多!");
        return 0;
        }
    return 1;
    }

//处理布尔表达式
void boolExp(struct ASTNode *T){
  struct opn opn1, opn2, result;
  int op;
  int rtn;
  if (T)
	{
	switch (T->kind) {
        case INT:  //处理布尔表达式中的int型
                    T->width = 0;
                    T->type = INT;
                    if(T->type_int){
                        T->code = genGoto(T->Etrue);
                    }
                    else{
                        T->code = genGoto(T->Efalse);
                    }
                    break;
        case FLOAT: //处理布尔表达式中的float型 
                    T->width = 0;
                    T->type = FLOAT;
                    if(T->type_float){
                        T->code = genGoto(T->Etrue);
                    }
                    else{
                        T->code = genGoto(T->Efalse);
                    }
                    break;
        case CHAR:  //处理布尔表达式中的char型
                    T->width = 0;
                    T->type = CHAR;
                    semantic_error(T->pos,T->type_id,"不可以在布尔表达式中判断char类型");
                    break;
        case ID:    // 处理布尔表达式汇中ID类型
                    rtn = searchSymbolTable(T->type_id);
                    if(rtn == -1){
                        rtn = searchSymbolTablestruct(T->type_id);
                        if(rtn == -1){
                            semantic_error(T->pos,T->type_id,"该变量名未定义");
                        }
                    }
                    if(symbolTable.symbols[rtn].flag == 'S'){
                        semantic_error(T->pos,"","此处不能使用结构体名");
                    }
                    else if(symbolTable.symbols[rtn].flag == 'F'){
                        semantic_error(T->pos,"","此处不能使用函数名");
                    }
                    else{ //如果标识符对应的值不为0，跳转到正确处执行否则跳转到错误处执行
                        //第一操作数
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                        opn1.offset=symbolTable.symbols[rtn].offset;
                        //第二操作数
                        opn2.kind=INT; 
                        opn2.const_int=0;
                        //结果
                        result.kind=ID; 
                        strcpy(result.id,T->Etrue);
                        T->code = genIR(NEQ,opn1,opn2,result);
                        T->code = merge(2,T->code,genGoto(T->Efalse));   
                    }
                    break;
        case RELOP: //处理关系运算表达式,2个操作数都按基本表达式处理
                    if(optflag == 0){
                        T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                        Exp(T->ptr[0]);
                        T->width=T->ptr[0]->width;
                        Exp(T->ptr[1]);
                        if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                        //第一操作数
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                        opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                        //第二操作数
                        opn2.kind=ID; 
                        strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                        opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                        //结果
                        result.kind=ID;
                        strcpy(result.id,T->Etrue);
                        if (strcmp(T->type_id,"<") == 0)
                                op = JLT;
                        else if (strcmp(T->type_id,"<=") == 0)
                                op = JLE;
                        else if (strcmp(T->type_id,">") == 0)
                                op = JGT;
                        else if (strcmp(T->type_id,">=") == 0)
                                op = JGE;
                        else if (strcmp(T->type_id,"==") == 0)
                                op = EQ;
                        else if (strcmp(T->type_id,"!=") == 0)
                                op = NEQ;
                        T->code=genIR(op,opn1,opn2,result);
                        T->code=merge(4,T->ptr[0]->code,T->ptr[1]->code,T->code,genGoto(T->Efalse));
                    }
                    else{
                        T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                        Exp(T->ptr[0]);
                        T->width=T->ptr[0]->width;
                        Exp(T->ptr[1]);
                        if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                        //第一操作数
                        if(T->ptr[0]->kind == INT){
                            opn1.kind=INT;
                            opn1.const_int = T->ptr[0]->type_int;
                        }
                        else{
                            opn1.kind=ID; 
                            strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                            opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                        }
                        //第二操作数
                        if(T->ptr[1]->kind == INT){
                            opn2.kind=INT;
                            opn2.const_int = T->ptr[1]->type_int;
                        }
                        else{
                            opn2.kind=ID; 
                            strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                            opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                        }
                        //结果
                        result.kind=ID;
                        strcpy(result.id,T->Etrue);
                        if (strcmp(T->type_id,"<") == 0)
                                op = JLT;
                        else if (strcmp(T->type_id,"<=") == 0)
                                op = JLE;
                        else if (strcmp(T->type_id,">") == 0)
                                op = JGT;
                        else if (strcmp(T->type_id,">=") == 0)
                                op = JGE;
                        else if (strcmp(T->type_id,"==") == 0)
                                op = EQ;
                        else if (strcmp(T->type_id,"!=") == 0)
                                op = NEQ;
                        T->code=genIR(op,opn1,opn2,result);
                        T->code=merge(4,T->ptr[0]->code,T->ptr[1]->code,T->code,genGoto(T->Efalse));
                    }
                    break;
        case AND:   //and的关系处理
                    //判断第一个
                    strcpy(T->ptr[0]->Etrue,newLabel());
                    strcpy(T->ptr[0]->Efalse,T->Efalse);

                    //判断第二个
                    strcpy(T->ptr[1]->Etrue,T->Etrue);
                    strcpy(T->ptr[1]->Efalse,T->Efalse);

                    //递归判断两边
                    T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                    boolExp(T->ptr[0]);
                    T->width=T->ptr[0]->width;
                    boolExp(T->ptr[1]);

                    // 计算width
                    if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                    T->code=merge(3,T->ptr[0]->code,genLabel(T->ptr[0]->Etrue),T->ptr[1]->code);
                    break;
        case OR:
                    //判断第一个
                    strcpy(T->ptr[0]->Etrue,T->Etrue);
                    strcpy(T->ptr[0]->Efalse,newLabel());
                    
                    //判断第二个
                    strcpy(T->ptr[1]->Etrue,T->Etrue);
                    strcpy(T->ptr[1]->Efalse,T->Efalse);
                    
                    //递归判断两边
                    T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                    boolExp(T->ptr[0]);
                    T->width=T->ptr[0]->width;
                    boolExp(T->ptr[1]);

                    // 计算width
                    if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                    T->code=merge(3,T->ptr[0]->code,genLabel(T->ptr[0]->Efalse),T->ptr[1]->code);
                    break;
        case NOT:   
                    strcpy(T->ptr[0]->Etrue,T->Efalse);
                    strcpy(T->ptr[0]->Efalse,T->Etrue);
                    boolExp(T->ptr[0]);
                    T->code=T->ptr[0]->code;
                    boolExp(T->ptr[0]);
                    break;
        }
	}
}

// 处理基本表达式
void Exp(struct ASTNode *T)
{
  int rtn,num,width;
  int op;
  struct ASTNode *T0;
  struct opn opn1,opn2,result;
  struct opn opn3,opn4,opn5,opn6; //构造0、1节点和转移节点
  if (T)
	{
	switch (T->kind) {
	case ID:    //查符号表，获得符号表中的位置，类型送type
                rtn=searchSymbolTable(T->type_id);
                if (rtn==-1)
                    semantic_error(T->pos,T->type_id, "变量未定义");
                if (symbolTable.symbols[rtn].flag=='F')
                    semantic_error(T->pos,T->type_id, "是函数名，类型不匹配");
                else {
                    T->place=rtn;       //结点保存变量在符号表中的位置
                    T->code=NULL;
                    T->type=symbolTable.symbols[rtn].type;
                    T->offset=symbolTable.symbols[rtn].offset;
                    T->width=0;   //未再使用新单元
                    }
                break;
    case INT:   //为整常量生成一个临时变量
                if(optflag == 0){
                    T->type = INT;
                    T->place = fill_Temp(newTemp(),LEV,T->type,'T',T->offset);
                    // 第一个操作
                    opn1.kind = INT; 
                    opn1.const_int = T->type_int;
                    //结果
                    result.kind = ID;
                    strcpy(result.id,symbolTable.symbols[T->place].alias); 
                    result.offset = symbolTable.symbols[T->place].offset;
                    T->code = genIR(ASSIGNOP,opn1,opn2,result);
                    T->width = symbol_width(T->type);
                }
                break;
    case FLOAT:  //为浮点常量生成一个临时变量
                if(optflag == 0){
                    T->type = FLOAT;
                    T->place = fill_Temp(newTemp(),LEV,T->type,'T',T->offset);
                    opn1.kind = FLOAT; opn1.const_float = T->type_float;
                    result.kind = ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                    result.offset = symbolTable.symbols[T->place].offset;
                    T->code = genIR(ASSIGNOP,opn1,opn2,result);
                    T->width = symbol_width(T->type);
                }
                break;
    case CHAR: //为字符型常量生成一个临时变量
                if(optflag == 0){
                    T->type = CHAR;
                    T->place = fill_Temp(newTemp(),LEV,T->type,'T',T->offset); 
                    opn1.kind = CHAR; opn1.const_char = T->type_char;
                    result.kind = ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                    result.offset = symbolTable.symbols[T->place].offset;
                    T->code = genIR(ASSIGNOP,opn1,opn2,result);
                    T->width = symbol_width(T->type);
                }
                break;
    case VAR_ARR: //数组下标中间代码生成以及语义检查
                if(T->ptr[1]->kind==FLOAT){
                    semantic_error(T->pos,"","数组访问下标不允许是浮点数");
                    }
                else if(T->ptr[1]->kind==UMINUS){
                    semantic_error(T->pos,"","数组访问下标需要大于0");
                }
                else{
                    Exp(T->ptr[1]);
                }
                // 判断
                if(T->ptr[0]->kind==ID){
                    rtn = searchSymbolTablearray(T->ptr[0]->type_id);
                    if(rtn==-1){
                        semantic_error(T->pos,T->ptr[0]->type_id,"不是数组变量，不能用[]运算");
                    }
                    else{
                        Exp(T->ptr[0]);
                    }
                }
                T->place = fill_Temp(newTemp(),LEV,T->type,'T',T -> offset);
                //第一操作数(数组名)
                opn1.kind=ID;
                opn1.type=T->ptr[0]->type;
                strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                //第二操作数(数组下标)
                if(T->ptr[1]->kind == ID){
                    opn2.kind = ID;
                    strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                }
                else{
                    opn2.kind = INT;
                    opn2.const_int = T->ptr[1]->type_int;
                }
                opn2.type = T->ptr[1]->type;
                opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                //结果
                result.kind=ID;
                result.type=T->type;
                strcpy(result.id,symbolTable.symbols[T->place].alias); //处理左值
                result.offset=symbolTable.symbols[T->place].offset;
                // 生成代码
                T->code=genIR(ARRAYOP,opn1,opn2,result);
                break;
    case VAR_STRU:
                if(T->ptr[0]->kind==ID){
                    rtn = searchSymbolTable(T->ptr[0]->type_id);
                    if(rtn==-1 || symbolTable.symbols[rtn].flag != 'S'){
                        semantic_error(T->pos,T->ptr[0]->type_id, "不是非结构体变量名，不能使用.运算");
                    }
                }
                else{ //表达式Exp向下寻找ID
                    Exp(T->ptr[0]);
                    T->type=T->ptr[0]->type;
                }
                // 判断是否是结构体变量名
                rtn = searchSymbolTablestruct(T->type_id);
                if(rtn==-1) //在第一层的外部变量必定是结构体成员变量
                     semantic_error(T->pos,T->type_id, "不是结构体成员名");
                break;
	case ASSIGNOP:
                if(optflag == 0){
                    if (T->ptr[0]->kind != ID && T->ptr[0]->kind != VAR_ARR &&T->ptr[0]->kind != VAR_STRU){
                        semantic_error(T->pos,"", "赋值语句需要左值");
                        }
                    else{
                        Exp(T->ptr[0]);   //处理左值，例中仅为变量
                        T->ptr[1]->offset=T->offset;
                        Exp(T->ptr[1]);
                        T->type=T->ptr[0]->type;
                        T->width=T->ptr[1]->width;
                        if(T->ptr[0]->kind != VAR_ARR)
                        {
                            T->code=merge(2,T->ptr[0]->code,T->ptr[1]->code);
                        }
                        //第一操作数
                        if(T->ptr[1]->kind == VAR_STRU){
                            rtn = searchSymbolTablestruct(T->ptr[1]->type_id);
                            opn1.kind = ID;
                            opn1.type = T->type;
                            strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                            opn1.offset = symbolTable.symbols[rtn].offset;
                        }
                        else{
                            opn1.kind=ID;
                            opn1.type=T->ptr[1]->type;
                            strcpy(opn1.id,symbolTable.symbols[T->ptr[1]->place].alias);//右值一定是个变量或临时变量
                            opn1.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                        }
                        //结果
                        if(T->ptr[0]->kind == VAR_STRU){
                            rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                            result.kind = ID;
                            result.type = T->type;
                            strcpy(result.id,symbolTable.symbols[rtn].alias);
                            result.offset = symbolTable.symbols[rtn].offset;
                        }
                        else{
                            result.kind=ID;
                            result.type=T->type;
                            strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias); //处理左值
                            result.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                        }
                        if(T->ptr[0]->kind != VAR_ARR){
                            T->code=merge(2,T->code,genIR(ASSIGNOP,opn1,opn2,result));
                        }
                        else{
                            opn3.kind=ID;
                            opn3.type=T->ptr[0]->ptr[0]->type;
                            strcpy(opn3.id,symbolTable.symbols[T->ptr[0]->ptr[0]->place].alias);
                            opn3.offset=symbolTable.symbols[T->ptr[0]->ptr[0]->place].offset;

                            if(T->ptr[0]->ptr[1]->kind == ID){
                                opn4.kind = ID;
                                strcpy(opn4.id,symbolTable.symbols[T->ptr[0]->ptr[1]->place].alias);
                            }
                            else{
                                opn4.kind = INT;
                                opn4.const_int = T->ptr[0]->ptr[1]->type_int;
                            }
                            opn4.type = T->ptr[1]->type;
                            opn4.offset=symbolTable.symbols[T->ptr[0]->ptr[1]->place].offset;
                            //代码
                            T->code=merge(2,T->ptr[1]->code,genIR(ARR_ASSIGN,opn3,opn4,opn1));
                        }
                        T->offset += symbol_width(T->type) + T->ptr[0]->width + T->ptr[1]->width;
                    }
                }
                else{
                    if (T->ptr[0]->kind != ID && T->ptr[0]->kind != VAR_ARR &&T->ptr[0]->kind != VAR_STRU){
                        semantic_error(T->pos,"", "赋值语句需要左值");
                    }
                    else{
                        Exp(T->ptr[0]);   //处理左值，例中仅为变量
                        T->ptr[1]->offset=T->offset;
                        Exp(T->ptr[1]);
                        T->type=T->ptr[0]->type;
                        T->width=T->ptr[1]->width;
                        if(T->ptr[0]->kind != VAR_ARR)
                        {
                            T->code=merge(2,T->ptr[0]->code,T->ptr[1]->code);
                        }
                        //第一操作数
                        if(T->ptr[1]->kind == VAR_STRU){
                            rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                            opn1.kind = ID;
                            opn1.type = T->type;
                            strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                            opn1.offset = symbolTable.symbols[rtn].offset;
                        }
                        else if(T->ptr[1]->kind == INT){
                            opn1.kind=INT;
                            opn1.const_int=T->ptr[1]->type_int;
                        }
                        else if(T->ptr[1]->kind == FLOAT){
                            opn1.kind=FLOAT;
                            opn1.const_float=T->ptr[1]->type_float;
                        }
                        else if(T->ptr[1]->kind == CHAR){
                            opn1.kind=CHAR;
                            opn1.const_char=T->ptr[1]->type_char;
                        }
                        else{
                            opn1.kind=ID;
                            opn1.type=T->ptr[1]->type;
                            strcpy(opn1.id,symbolTable.symbols[T->ptr[1]->place].alias);//右值一定是个变量或临时变量
                            opn1.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                        }
                        //结果
                        if(T->ptr[0]->kind == VAR_STRU){
                            rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                            result.kind = ID;
                            result.type = T->type;
                            strcpy(result.id,symbolTable.symbols[rtn].alias);
                            result.offset = symbolTable.symbols[rtn].offset;
                        }
                        else{
                            result.kind=ID;
                            result.type=T->type;
                            strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias); //处理左值
                            result.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                        }
                        if(T->ptr[0]->kind != VAR_ARR){
                            T->code=merge(2,T->code,genIR(ASSIGNOP,opn1,opn2,result));
                        }
                        else{
                            opn3.kind=ID;
                            opn3.type=T->ptr[0]->ptr[0]->type;
                            strcpy(opn3.id,symbolTable.symbols[T->ptr[0]->ptr[0]->place].alias);
                            opn3.offset=symbolTable.symbols[T->ptr[0]->ptr[0]->place].offset;

                            if(T->ptr[0]->ptr[1]->kind == ID){
                                opn4.kind = ID;
                                strcpy(opn4.id,symbolTable.symbols[T->ptr[0]->ptr[1]->place].alias);
                            }
                            else{
                                opn4.kind = INT;
                                opn4.const_int = T->ptr[0]->ptr[1]->type_int;
                            }
                            opn4.type = T->ptr[1]->type;
                            opn4.offset=symbolTable.symbols[T->ptr[0]->ptr[1]->place].offset;
                            //代码
                            T->code=merge(2,T->ptr[1]->code,genIR(ARR_ASSIGN,opn3,opn4,opn1));
                        }
                        T->offset += symbol_width(T->type) + T->ptr[0]->width + T->ptr[1]->width;
                    }
                }
                break;
	case AND:   //AND的代码生成逻辑 
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                Exp(T->ptr[1]);
                T->type = INT;
                //位置序号
                T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
                //第一操作数（左边）
                opn1.kind=ID; 
                strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type=T->ptr[0]->type;
                opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                //第二操作数（右边）
                opn2.kind=ID; 
                strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                opn2.type=T->ptr[1]->type;
                opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                //结果
                result.kind=ID; 
                strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.type=T->type;
                result.offset=symbolTable.symbols[T->place].offset;
                //生成0和1
                struct opn opn3,opn4;
                opn3.kind = INT;
                opn3.const_int = 1;
                opn4.kind = INT;
                opn4.const_int = 0;
                strcpy(opn5.id,newLabel());
                //代码
                T->code = merge(6,T->ptr[0]->code,T->ptr[1]->code,genIR(STAR,opn1,opn2,result),genIR(EQ,result,opn4,opn5),genIR(ASSIGNOP,opn3,opn2,result),genLabel(opn5.id));
                T->width=T->ptr[0]->width+T->ptr[1]->width+symbol_width(T->type);
                break;
	case OR:    //OR的代码生成逻辑
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                Exp(T->ptr[1]);
                //位置序号
                T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
                //第一操作数（左边）
                opn1.kind=ID; 
                strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type=T->ptr[0]->type;
                opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                //第二操作数（右边）
                opn2.kind=ID; 
                strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                opn2.type=T->ptr[1]->type;
                opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                //结果
                result.kind=ID; 
                strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.type=T->type;
                result.offset=symbolTable.symbols[T->place].offset;
                //1节点和0节点,以及两个转移节点
                opn3.kind = INT;
                opn3.const_int = 1; 
                opn4.kind = INT;
                opn4.const_int = 0;
                //代码
                strcpy(opn5.id,newLabel());
                strcpy(opn6.id,newLabel());
                T->code=merge(10,T->ptr[0]->code,T->ptr[1]->code,genIR(ASSIGNOP,opn4,opn2,result),genIR(EQ,opn1,opn4,opn5),genIR(ASSIGNOP,opn3,opn2,result),genGoto(opn6.id),genLabel(opn5.id),genIR(EQ,opn2,opn4,opn6),genIR(ASSIGNOP,opn3,opn2,result),genLabel(opn6.id));
                T->width=T->ptr[0]->width+T->ptr[1]->width+symbol_width(T->type);
                break;
	case RELOP: //按算术表达式方式计算布尔值，未写完
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                Exp(T->ptr[1]);
                T->type = INT;
                //位置序号
                T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
                //第一操作数（左边）
                opn1.kind=ID; 
                strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type=T->ptr[0]->type;
                opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                //第二操作数（右边）
                opn2.kind=ID; 
                strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                opn2.type=T->ptr[1]->type;
                opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                //结果
                result.kind=ID; 
                strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.type=T->type;
                result.offset=symbolTable.symbols[T->place].offset;
                //1节点和0节点,以及两个转移节点
                opn3.kind = INT;
                opn3.const_int = 1; 
                opn4.kind = INT;
                opn4.const_int = 0;
                //判断relop类型
                if (strcmp(T->type_id,"<") == 0)
                            op = JLT;
                    else if (strcmp(T->type_id,"<=") == 0)
                            op = JLE;
                    else if (strcmp(T->type_id,">") == 0)
                            op = JGT;
                    else if (strcmp(T->type_id,">=") == 0)
                            op = JGE;
                    else if (strcmp(T->type_id,"==") == 0)
                            op = EQ;
                    else if (strcmp(T->type_id,"!=") == 0)
                            op = NEQ;
                strcpy(opn5.id,newLabel());
                strcpy(opn6.id,newLabel());
                //代码
                T->code=merge(8,T->ptr[0]->code,T->ptr[1]->code,genIR(op,opn1,opn2,opn5),genIR(ASSIGNOP,opn4,opn2,result),genGoto(opn6.id),genLabel(opn5.id),genIR(ASSIGNOP,opn3,opn2,result),genLabel(opn6.id));
                T->width=T->ptr[0]->width+T->ptr[1]->width+symbol_width(T->type);
                break;
    case NOT:   // 非操作的代码
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                //位置序号
                T->type = INT;
                T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width);
                //第一操作数（左边）
                opn1.kind=ID; 
                strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type=T->ptr[0]->type;
                opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                //结果
                result.kind=ID; 
                strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.type=T->type;
                result.offset=symbolTable.symbols[T->place].offset;
                //1节点和0节点,以及两个转移节点
                opn3.kind = INT;
                opn3.const_int = 1; 
                opn4.kind = INT;
                opn4.const_int = 0;
                strcpy(opn5.id,newLabel());
                strcpy(opn6.id,newLabel());
                //代码
                T->code=merge(6,genIR(EQ,opn1,opn4,opn5),genIR(ASSIGNOP,opn4,opn2,result),genGoto(opn6.id),genLabel(opn5.id),genIR(ASSIGNOP,opn3,opn2,result),genLabel(opn6.id));
                T->width=T->ptr[0]->width+symbol_width(T->type);
                break;
	case UMINUS: // 负号的代码
                T->type = T->ptr[0]->type;
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]);
                //位置序号
                T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width);
                //操作数
                opn1.kind = ID;
                strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type = T->ptr[0]->type;
                opn1.offset = T->ptr[0]->offset;
                //结果
                result.kind = ID;
                strcpy(result.id, symbolTable.symbols[T->place].alias);
                result.type = T->type;
                result.offset = symbolTable.symbols[T->place].offset;
                //代码
                T->code = merge(2, T->ptr[0]->code, genIR(UMINUS, opn1, opn2, result));
                T->width = T->ptr[0]->width + symbol_width(T->type);
                break;
    case DPLUS: //自增
                T->type = T->ptr[0]->type;
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]); //处理表达式
                T->place = fill_Temp(newTemp(),LEV,T->type,'T',T->offset + T->ptr[0]->width);
                //结果
                if(T->ptr[0]->kind == VAR_STRU){
                    rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                    result.kind=ID; 
                    strcpy(result.id,symbolTable.symbols[rtn].alias);
                    result.type=T->ptr[0]->type;
                    result.offset=symbolTable.symbols[rtn].offset;
                }
                else{
                    result.kind = ID;
                    strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                    result.type = T->ptr[0]->type;
                    result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
                }
                //代码
                if(T->ptr[0]->kind != VAR_ARR){
                    T->code = merge(2,T->ptr[0]->code,genIR(DPLUS,opn1,opn2,result));
                }
                else{
                    opn3.kind=ID;
                    strcpy(opn3.id,symbolTable.symbols[T->ptr[0]->ptr[0]->place].alias);
                    if(T->ptr[0]->ptr[1]->kind ==  INT){
                        opn4.kind = INT;
                        opn4.const_int = T->ptr[0]->ptr[1]->type_int;
                    }
                    else{
                        opn4.kind =ID;
                        strcpy(opn4.id,symbolTable.symbols[T->ptr[0]->ptr[1]->place].alias);
                    }
                    T->code = merge(3,T->ptr[0]->code,genIR(DPLUS,opn1,opn2,result),genIR(ARR_ASSIGN,opn3,opn4,result));
                }
                T->width = T->ptr[0]->width + symbol_width(T->type);
                break;
    case DMINUS: //自减
                T->type = T->ptr[0]->type;
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]); //处理表达式
                T->place = fill_Temp(newTemp(),LEV,T->type,'T',T->offset + T->ptr[0]->width);
                //第一操作数
                if(T->ptr[0]->kind == VAR_STRU){
                    rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                    result.kind=ID; 
                    strcpy(result.id,symbolTable.symbols[rtn].alias);
                    result.type=T->ptr[0]->type;
                    result.offset=symbolTable.symbols[rtn].offset;
                }
                else{
                    result.kind = ID;
                    strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                    result.type = T->ptr[0]->type;
                    result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
                }
                //代码
                T->code = merge(2,T->ptr[0]->code,genIR(DPLUS,opn1,opn2,result));
                T->width = T->ptr[0]->width + symbol_width(T->type);
                break;
    // 下面统一处理OP运算类型
	case PLUS:  
	case MINUS: 
	case STAR:  
	case DIV:   //四则运算
                if(optflag == 0){
                    T->ptr[0]->offset=T->offset;
                    Exp(T->ptr[0]);
                    T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                    Exp(T->ptr[1]);
                    //判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
                    //下面的类型属性计算，没有考虑错误处理情况
                    if (T->ptr[0]->type==FLOAT || T->ptr[1]->type==FLOAT){
                        T->type=FLOAT;
                        T->width=T->ptr[0]->width+T->ptr[1]->width+4;
                    }
                    else{
                        T->type=INT;
                        T->width=T->ptr[0]->width+T->ptr[1]->width+2;
                    }
                    if((T->ptr[1]->type_int == 0 || T->ptr[1]->type_float == 0) && T->type == DIV){
                        semantic_error(T->pos,"","除数不能为0");
                    }

                    T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset+T->ptr[0]->width+T->ptr[1]->width);

                    //第一操作数（被除数）
                    if(T->ptr[0]->kind == VAR_STRU){
                        rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                        opn1.type=T->ptr[0]->type;
                        opn1.offset=symbolTable.symbols[rtn].offset;
                    }
                    else{
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                        opn1.type=T->ptr[0]->type;
                        opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                    }
                    
                    //第二操作数（除数）
                    if(T->ptr[1]->kind == VAR_STRU){
                        rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                        opn1.type=T->ptr[0]->type;
                        opn1.offset=symbolTable.symbols[rtn].offset;
                    }
                    else{
                        opn2.kind=ID; 
                        strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                        opn2.type=T->ptr[1]->type;
                        opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                    }
                    
                    //结果
                    result.kind=ID; 
                    strcpy(result.id,symbolTable.symbols[T->place].alias);
                    result.type=T->type;
                    result.offset=symbolTable.symbols[T->place].offset; 
                    //代码
                    T->code=merge(3,T->ptr[0]->code,T->ptr[1]->code,genIR(T->kind,opn1,opn2,result));
                    T->width=T->ptr[0]->width+T->ptr[1]->width+symbol_width(T->type);
                }
                else{
                    T->ptr[0]->offset=T->offset;
                    Exp(T->ptr[0]);
                    T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                    Exp(T->ptr[1]);
                    //判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
                    //下面的类型属性计算，没有考虑错误处理情况
                    if (T->ptr[0]->type==FLOAT || T->ptr[1]->type==FLOAT){
                        T->type=FLOAT;
                        T->width=T->ptr[0]->width+T->ptr[1]->width+4;
                    }
                    else{
                        T->type=INT;
                        T->width=T->ptr[0]->width+T->ptr[1]->width+2;
                    }
                    if((T->ptr[1]->type_int == 0 || T->ptr[1]->type_float == 0) && T->type == DIV){
                        semantic_error(T->pos,"","除数不能为0");
                    }

                    T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset+T->ptr[0]->width+T->ptr[1]->width);

                    //第一操作数（被除数）
                    if(T->ptr[0]->kind == VAR_STRU){
                        rtn = searchSymbolTablestruct(T->ptr[0]->type_id);
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                        opn1.type=T->ptr[0]->type;
                        opn1.offset=symbolTable.symbols[rtn].offset;
                    }
                    else if(T->ptr[0]->kind == INT){
                        opn1.kind=INT;
                        opn1.const_int=T->ptr[0]->type_int;
                    }
                    else if(T->ptr[0]->kind == FLOAT){
                        opn1.kind=INT;
                        opn1.const_float=T->ptr[0]->type_float;
                    }
                    else if(T->ptr[0]->kind == CHAR){
                        opn1.kind=CHAR;
                        opn1.const_char=T->ptr[0]->type_char;
                    }
                    else{
                        opn1.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                        opn1.type=T->ptr[0]->type;
                        opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                    }
                    
                    //第二操作数（除数）
                    if(T->ptr[1]->kind == VAR_STRU){
                        rtn = searchSymbolTablestruct(T->ptr[1]->type_id);
                        opn2.kind=ID; 
                        strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                        opn2.type=T->ptr[1]->type;
                        opn2.offset=symbolTable.symbols[rtn].offset;
                    }
                    else if(T->ptr[1]->kind == INT){
                        opn2.kind=INT;
                        opn2.const_int=T->ptr[1]->type_int;
                    }
                    else if(T->ptr[1]->kind == FLOAT){
                        opn2.kind=FLOAT;
                        opn2.const_float=T->ptr[1]->type_float;
                    }
                    else if(T->ptr[1]->kind == CHAR){
                        opn2.kind=CHAR;
                        opn2.const_char=T->ptr[1]->type_char;
                    }
                    else{
                        opn2.kind=ID; 
                        strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                        opn2.type=T->ptr[1]->type;
                        opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                    }
                    
                    //结果
                    result.kind=ID; 
                    strcpy(result.id,symbolTable.symbols[T->place].alias);
                    result.type=T->type;
                    result.offset=symbolTable.symbols[T->place].offset; 
                    //代码
                    T->code=merge(3,T->ptr[0]->code,T->ptr[1]->code,genIR(T->kind,opn1,opn2,result));
                    T->width=T->ptr[0]->width+T->ptr[1]->width+symbol_width(T->type);
                }
                break;
    case FUNC_CALL: //根据T->type_id查出函数的定义
                rtn=searchSymbolTable(T->type_id);
                if (rtn==-1){
                    semantic_error(T->pos,T->type_id, "函数未定义");
                    break;
                    }
                if (symbolTable.symbols[rtn].flag!='F'){
                    semantic_error(T->pos,T->type_id, "不是一个函数");
                     break;
                    }
                T->type=symbolTable.symbols[rtn].type;
                width=symbol_width(T->type);   //存放函数返回值的单数字节数
                if (T->ptr[0]){
                    T->ptr[0]->offset=T->offset;
                    Exp(T->ptr[0]);       //处理所有实参表达式求值，及类型
                    T->width=T->ptr[0]->width+width; //累加上计算实参使用临时变量的单元数
                    T->code=T->ptr[0]->code;
                    }
                else {T->width=width; T->code=NULL;}
                match_param(rtn,T);   //处理所有参数的匹配
                    //处理参数列表的中间代码
                T0=T->ptr[0];
                while (T0) {
                    result.kind=ID;
                    if(T0->ptr[0]->kind == VAR_STRU){
                        rtn = searchSymbolTablestruct(T0->ptr[0]->type_id);
                        strcpy(result.id,symbolTable.symbols[rtn].alias);
                    }
                    else{
                        strcpy(result.id,symbolTable.symbols[T0->ptr[0]->place].alias);
                    }
                    result.offset=symbolTable.symbols[T0->ptr[0]->place].offset;
                    T->code=merge(2,T->code,genIR(ARG,opn1,opn2,result));
                    T0=T0->ptr[1];
                    }
                T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset+T->width - width);
                opn1.kind=ID; strcpy(opn1.id,T->type_id);  //第一操作数保存函数名
                opn1.offset=rtn; //这里offset用以保存函数定义入口,在目标代码生成时，能获取相应信息
                result.kind=ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.offset=symbolTable.symbols[T->place].offset;
                T->code=merge(2,T->code,genIR(CALL,opn1,opn2,result));
                break;
    case ARGS:  //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                T->width=T->ptr[0]->width;
                T->code=T->ptr[0]->code;
                if (T->ptr[1]) {
                    T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                    Exp(T->ptr[1]);
                    T->width+=T->ptr[1]->width;
                    T->code=merge(2,T->code,T->ptr[1]->code);   
                }
                break;
         }
      }
}

int LEV = 0; //初始层数为0
int loopflag=0; //判断该语句是否在循环内
int returnflag=0; //判断是否需要返回语句return
char head[33],tail[33]; //记录循环体开头和末尾位置(label)
void semantic_Analysis(struct ASTNode *T)
{//对抽象语法树的先根遍历,按display的控制结构修改完成符号表管理和语义检查
  int rtn,num,width;
  struct ASTNode *T0;
  struct opn opn1,opn2,result;
  if (T)
	{
	switch (T->kind) {
	case EXT_DEF_LIST:
            if (!T->ptr[0]) break;
            T->ptr[0]->offset=T->offset;
            semantic_Analysis(T->ptr[0]);    //访问外部定义列表中的第一个
            T->code = T->ptr[0]->code;
            if (T->ptr[1]){
                T->ptr[1]->offset=T->ptr[0]->offset+T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
                //生成代码
                T->code = merge(2, T->code, T->ptr[1]->code);
                }
            break;
	case EXT_VAR_DEF:   //处理外部说明,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            T->type=T->ptr[1]->type=symbol_label(T->ptr[0]->type_id);
            T->ptr[1]->offset=T->offset;        //这个外部变量的偏移量向下传递
            T->ptr[1]->width=symbol_width(T->type);  //将一个变量的宽度向下传递
            ext_var_list(T->ptr[1]);            //处理外部变量说明中的标识符序列
            T->width=symbol_width(T->type)* T->ptr[1]->num; //计算这个外部变量说明的宽度
            T->code = NULL;
            break;
    case EXT_STRU_DEF:
            rtn=fillSymbolTable(T->ptr[1]->type_id,newAlias(),LEV,T->type,'S',T->offset); //结构体成员变量
            semantic_Analysis(T->ptr[0]);
            rtn++;
            while(symbolTable.symbols[rtn].level!=0){
                rtn++;
            }
            rtn--;
            T->offset += symbol_width(symbolTable.symbols[rtn].type) + symbolTable.symbols[rtn].offset;
            break;
	case FUNC_DEF:      //填写函数定义信息到符号表
            T->ptr[1]->type=symbol_label(T->ptr[0]->type_id);//获取函数返回类型送到含函数名、参数的结点
            T->width=0;     //函数的宽度设置为0，不会对外部变量的地址分配产生影响
            T->offset=DX;   //设置局部变量在活动记录中的偏移量初值
            semantic_Analysis(T->ptr[1]); //处理函数名和参数结点部分，这里不考虑用寄存器传递参数
            T->offset+=T->ptr[1]->width;   //用形参单元宽度修改函数局部变量的起始偏移量
            T->ptr[2]->offset=T->offset;
            //函数体语句执行结束后的位置属性
            strcpy(T->ptr[2]->Snext,newLabel());
            semantic_Analysis(T->ptr[2]);         //处理函数体结点
            if(returnflag == 0){
                semantic_error(T->pos,"","非void函数需要有返回语句");
            }
            returnflag = 0;
            //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
            symbolTable.symbols[T->ptr[1]->place].offset=T->offset+T->ptr[2]->width;
            //函数体作为函数的代码
            T->code = merge(3,T->ptr[1]->code, T->ptr[2]->code, genLabel(T->ptr[2]->Snext));
            break;
	case FUNC_DEC:      //根据返回类型，函数名填写符号表
            rtn=fillSymbolTable(T->type_id,newAlias(),LEV,T->type,'F',0);//函数不在数据区中分配单元，偏移量为0
            if (rtn==-1){
                semantic_error(T->pos,T->type_id, "函数重复定义");
                break;
                }
            else T->place=rtn;
            //结果代码
            result.kind = ID;
            strcpy(result.id, T->type_id);
            result.offset = rtn;
            T->code = genIR(FUNCTION, opn1, opn2, result);  //生成中间代码 FUNCTION 函数名
            T->offset=DX;   //设置形式参数在活动记录中的偏移量初值
            if (T->ptr[0]) { //判断是否有参数
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);  //处理函数参数列表
                T->width=T->ptr[0]->width;
                symbolTable.symbols[rtn].paramnum=T->ptr[0]->num;
                T->code = merge(2,T->code,T->ptr[0]->code);
                }
            else symbolTable.symbols[rtn].paramnum=0,T->width=0;
            break;
	case PARAM_LIST:    //处理函数形式参数列表
            T->ptr[0]->offset=T->offset;
            semantic_Analysis(T->ptr[0]);
            if (T->ptr[1]){
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]);
                T->num=T->ptr[0]->num+T->ptr[1]->num;        //统计参数个数
                T->width=T->ptr[0]->width+T->ptr[1]->width;  //累加参数单元宽度
                // 连接参数代码
                T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);
                }
            else {
                T->num=T->ptr[0]->num;
                T->width=T->ptr[0]->width;
                T->code = T->ptr[0]->code;
                }
            break;
	case PARAM_DEC:
            rtn=fillSymbolTable(T->ptr[1]->type_id,newAlias(),1,T->ptr[0]->type,'P',T->offset);
            if (rtn==-1)
                semantic_error(T->ptr[1]->pos,T->ptr[1]->type_id, "参数名重复定义");
            else T->ptr[1]->place=rtn;
            T->num=1;       //参数个数计算的初始值
            T->width=symbol_width(T->ptr[0]->type);  //参数宽度
            //结果
            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[rtn].alias);
            result.offset = T->offset;
            T->code = genIR(PARAM, opn1, opn2, result); //参数名称
            break;
    case STRUTYPE:
            LEV++;
            T->width=0;
            if (T->ptr[1]) { 
                T->ptr[1]->offset=T->offset;
                semantic_Analysis(T->ptr[1]);
                T->width+=T->ptr[0]->width;
                }
             LEV--;    //出复合语句，层号减1
             break;
	case COMP_STM:
            LEV++;
            //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbol_scope_TX
            symbol_scope_TX.TX[symbol_scope_TX.top++]=symbolTable.index;
            T->width=0;
            T->code = NULL;
            if (T->ptr[0]) {
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);  //处理该层的局部变量DEF_LIST
                T->width+=T->ptr[0]->width;
                T->code = T->ptr[0]->code;
                }
            if (T->ptr[1]){
                T->ptr[1]->offset=T->offset+T->width;
                semantic_Analysis(T->ptr[1]);       //处理复合语句的语句序列
                T->width+=T->ptr[1]->width;
                T->code = merge(2, T->code, T->ptr[1]->code);
                }
             LEV--;    //出复合语句，层号减1
             symbolTable.index=symbol_scope_TX.TX[--symbol_scope_TX.top]; //删除该作用域中的符号
             break;
    case DEF_LIST:
            T->code = NULL;
            if (T->ptr[0]){
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);   //处理一个局部变量定义
                T->code = T->ptr[0]->code;
                T->width=T->ptr[0]->width;
                }
            if (T->ptr[1]) {
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]);   //处理剩下的局部变量定义
                T->code = merge(2, T->code, T->ptr[1]->code);
                T->width+=T->ptr[1]->width;
                }
                break;
    case VAR_DEF://处理一个局部变量定义,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
                 //类似于上面的外部变量EXT_VAR_DEF，换了一种处理方法
                T->code = NULL;
                T->ptr[1]->type=symbol_label(T->ptr[0]->type_id);  //确定变量序列各变量类型
                T0=T->ptr[1]; //T0为变量名列表子树根指针，对ID、ASSIGNOP类结点在登记到符号表，作为局部变量
                num=0;
                T0->offset=T->offset;
                T->width=0;
                width=symbol_width(T->ptr[1]->type);  //一个变量宽度
                while (T0) {  //处理所以DEC_LIST结点
                    num++;
                    T0->ptr[0]->type=T0->type;  //类型属性向下传递
                    if (T0->ptr[1]) T0->ptr[1]->type=T0->type;
                    T0->ptr[0]->offset=T0->offset;  //类型属性向下传递
                    if (T0->ptr[1]) T0->ptr[1]->offset=T0->offset+width;
                    if (T0->ptr[0]->kind==ID){
                        rtn=fillSymbolTable(T0->ptr[0]->type_id,newAlias(),LEV,T0->ptr[0]->type,'V',T->offset+T->width);//此处偏移量未计算，暂时为0
                        if (rtn==-1)
                            semantic_error(T0->ptr[0]->pos,T0->ptr[0]->type_id, "变量重复定义");
                        else T0->ptr[0]->place=rtn;
                        T->width+=width;
                        }
                    else if (T0->ptr[0]->kind==ASSIGNOP){
                            rtn=fillSymbolTable(T0->ptr[0]->ptr[0]->type_id,newAlias(),LEV,T0->ptr[0]->type,'V',T->offset+T->width);//此处偏移量未计算，暂时为0
                            if (rtn==-1)
                                semantic_error(T0->ptr[0]->ptr[0]->pos,T0->ptr[0]->ptr[0]->type_id, "变量重复定义");
                            else {
                                T0->ptr[0]->place=rtn;
                                T0->ptr[0]->ptr[1]->offset=T->offset+T->width+width;
                                Exp(T0->ptr[0]->ptr[1]);
                                // 右操作数
                                opn1.kind = ID;
                                strcpy(opn1.id,symbolTable.symbols[T0->ptr[0]->ptr[1]->place].alias);
                                // 结果
                                result.kind = ID;
                                strcpy(opn1.id,symbolTable.symbols[T0->ptr[0]->place].alias);
                                // 代码
                                T->code = merge(3, T->code, T0->ptr[0]->ptr[1]->code,genIR(ASSIGNOP, opn1, opn2, result));
                                }
                            T->width+=width+T0->ptr[0]->ptr[1]->width;
                            }
                    T0=T0->ptr[1];
                    }
                break;
	case STM_LIST:
                if (!T->ptr[0]) { T->code=NULL; T->width=0; break;}   //空语句序列
                if (T->ptr[1]) //2条以上语句连接，生成新标号作为第一条语句结束后到达的位置
                    strcpy(T->ptr[0]->Snext,newLabel());
                else     //语句序列仅有一条语句，S.next属性向下传递
                    strcpy(T->ptr[0]->Snext,T->Snext);
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);
                T->code=T->ptr[0]->code;
                T->width=T->ptr[0]->width;
                if (T->ptr[1]){     //2条以上语句连接,S.next属性向下传递
                    strcpy(T->ptr[1]->Snext,T->Snext);
                    T->ptr[1]->offset=T->offset;  //顺序结构共享单元方式
                    semantic_Analysis(T->ptr[1]);
                    //序列中第1条为表达式语句，返回语句，复合语句时，第2条前不需要标号
                    if (T->ptr[0]->kind==RETURN ||T->ptr[0]->kind==EXP_STMT ||T->ptr[0]->kind==COMP_STM)
                        T->code=merge(2,T->code,T->ptr[1]->code);
                    else
                        T->code=merge(3,T->code,genLabel(T->ptr[0]->Snext),T->ptr[1]->code);
                    if (T->ptr[1]->width>T->width) T->width=T->ptr[1]->width; //顺序结构共享单元方式
                }
                break;
	case IF_THEN:
                strcpy(T->ptr[0]->Etrue, newLabel()); //设置条件语句真假转移位置
                strcpy(T->ptr[0]->Efalse, T->Snext); //如果条件为假，跳过
                T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                boolExp(T->ptr[0]);
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,T->Snext); //结束了if子句，接着处理下一个子句
                semantic_Analysis(T->ptr[1]);      //if子句
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                T->code = merge(3, T->ptr[0]->code, genLabel(T->ptr[0]->Etrue),T->ptr[1]->code);
                break;
	case IF_THEN_ELSE:
                strcpy(T->ptr[0]->Etrue,newLabel()); //条件为真转移位置
                strcpy(T->ptr[0]->Efalse,newLabel()); //条件为假转移位置
                T->ptr[0]->offset=T->ptr[1]->offset=T->ptr[2]->offset=T->offset;
                boolExp(T->ptr[0]);
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,T->Snext); //第一句结束完，结尾
                semantic_Analysis(T->ptr[1]);      //if子句
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                strcpy(T->ptr[2]->Snext,T->Snext); //第二句结束完，结尾
                semantic_Analysis(T->ptr[2]);      //else子句
                if (T->width<T->ptr[2]->width) T->width=T->ptr[2]->width;
                T->code=merge(6,T->ptr[0]->code,genLabel(T->ptr[0]->Etrue),T->ptr[1]->code,genGoto(T->Snext),genLabel(T->ptr[0]->Efalse),T->ptr[2]->code);
                break;
	case WHILE: 
                strcpy(T->ptr[0]->Etrue,newLabel()); //子节点继承属性的计算
                strcpy(T->ptr[0]->Efalse,T->Snext); //出错，跳过
                T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                boolExp(T->ptr[0]);
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,newLabel()); //循环体结束，继续
                loopflag = 1;
                strcpy(head,T->ptr[1]->Snext); //循环体头
                strcpy(tail,T->ptr[0]->Efalse); //循环体尾
                semantic_Analysis(T->ptr[1]);      //循环体
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                // 生成代码
                strcpy(T->Snext,newLabel());
                T->code=merge(5,genLabel(T->ptr[1]->Snext),T->ptr[0]->code,genLabel(T->ptr[0]->Etrue),T->ptr[1]->code,genGoto(T->ptr[1]->Snext));
                loopflag = 0;
                break;
    case FOR:   
                strcpy(T->ptr[1]->Etrue, newLabel()); //判断语句正确执行循环
                strcpy(T->ptr[1]->Efalse, T->Snext); //判断错误则跳过循环
                T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->ptr[3]->offset = T->offset;
                // 检查循环开始状态语法
                semantic_Analysis(T->ptr[0]);
                // 判断条件
                boolExp(T->ptr[1]);
                strcpy(T->ptr[2]->Snext, newLabel());
                // 检查循环递增状态语法
                semantic_Analysis(T->ptr[2]);
                loopflag = 1;
                strcpy(head,T->ptr[2]->Snext);  //循环体头
                strcpy(tail,T->ptr[1]->Efalse); //循环体尾
                semantic_Analysis(T->ptr[3]);
                T->width = T->ptr[3]->width;
                // 生成代码
                T->code = merge(7,T->ptr[0]->code,genLabel(T->ptr[2]->Snext),T->ptr[1]->code,genLabel(T->ptr[1]->Etrue),T->ptr[3]->code,T->ptr[2]->code,genGoto(T->ptr[2]->Snext));
                loopflag = 0;
                break;
    case EXP_STMT:
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);
                T->code = T->ptr[0]->code;
                T->width=T->ptr[0]->width;
                break;
	case RETURN:
                if(optflag == 1){
                    if (T->ptr[0]){
                        T->ptr[0]->offset=T->offset;
                        Exp(T->ptr[0]);
                        T->width=T->ptr[0]->width;
                        // 结果
                        if(T->ptr[0]->kind == INT){
                            result.kind = INT;
                            result.const_int = T->ptr[0]->type_int;
                        }
                        else{
                            result.kind = ID;
                            strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                            result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
                        }
                        //生成代码
                        T->code = merge(2, T->ptr[0]->code,genIR(RETURN,opn1,opn2,result));
                    }
                    else{
                            T->width=0;
                            result.kind = 0;
                            T-> code = genIR(RETURN,opn1,opn2,result);
                        }
                    returnflag = 1;
                }
                else{
                    if (T->ptr[0]){
                        T->ptr[0]->offset=T->offset;
                        Exp(T->ptr[0]);
                        T->width=T->ptr[0]->width;
                        // 结果
                        result.kind = ID;
                        strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                        result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
                        //生成代码
                        T->code = merge(2, T->ptr[0]->code,genIR(RETURN,opn1,opn2,result));
                        }
                    else{
                        T->width=0;
                        result.kind = 0;
                        T-> code = genIR(RETURN,opn1,opn2,result);
                    }
                    returnflag = 1;
                }
                break;
    case BREAK: 
                if(loopflag == 0){
                    semantic_error(T->pos,"","break语句需要在循环体内");
                }
                // 跳到函数尾
                T->code = genGoto(tail);
                break;
    case CONTINUE:
                if(loopflag == 0){
                    semantic_error(T->pos,"","continue语句需要在循环体内");
                }
                // 跳转到函数头
                T->code = genGoto(head);
                break;
    // 处理剩下的可以作为Exp的表达式
	case ID:
    case INT:
    case FLOAT:
	case ASSIGNOP:
	case AND:
	case OR:
	case RELOP:
	case PLUS:
	case MINUS:
	case STAR:
	case DIV:
	case NOT:
	case UMINUS:
    case DPLUS:
    case DMINUS:
    case FUNC_CALL:
                    Exp(T);          //处理基本表达式
                    break;
    }
    }
}

void semantic_Analysis0(struct ASTNode *T) {
    symbolTable.index=0;
    fillSymbolTable("read","",0,INT,'F',4);
    symbolTable.symbols[0].paramnum=0;//read的形参个数
    fillSymbolTable("write","",0,INT,'F',4);
    // symbolTable.symbols[1].paramnum=1;
    // fillSymbolTable("x","",1,INT,'P',12);
    symbol_scope_TX.TX[0]=0;  //外部变量在符号表中的起始序号为0
    symbol_scope_TX.top=1;
    optflag = 0; //中间代码优化
    T->offset=0;              //外部变量在数据区的偏移量
    semantic_Analysis(T);
    prn_symbol();
    // prnIR(T->code);
    objectCode(T->code);
 }
