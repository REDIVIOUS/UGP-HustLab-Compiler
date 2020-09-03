#include "def.h"

// 初始化寄存器
void RegInit(){
    int i = 0;
    // 初始化寄存器t0到t7
    for(i = 0; i<=7; i++){
        sprintf(r[i].RegName, "$t%d", i); //为寄存器名赋值
        r[i].occupied = 0; //清空
    }
    // 初始化寄存器s0到s7
    for(i = 0; i <= 7; i++){
        sprintf(r[i + 8].RegName,"$s%d", i); //为寄存器名赋值
        r[i + 8].occupied = 0; //清空
    }
}


// 别名分配或匹配寄存器
char* DiliverReg(char *AliasName){
    int i = 0;
    // 首先找一圈，看看是已经给该别名分配寄存器
    for(i = 0; i < REGNUM; i++){
        if(r[i].occupied){
            // 如果找到了该别名的寄存器，返回该寄存器
            if(!strcmp(r[i].AliasName,AliasName)){
                // 如果第二次遇到temp类的寄存器，则在用过后释放
                if(r[i].AliasName[0] == 't'){
                    r[i].occupied = 0;
                }
                return r[i].RegName;
            }
        }
    }
    // 如果没有，找到第一个空闲的寄存器填入
    for(i = 0; i < REGNUM; i++){
        if(!r[i].occupied){
            r[i].occupied = 1; // 该寄存器处于被占有状态
            strcpy(r[i].AliasName,AliasName); // 绑定别名和寄存器
            return r[i].RegName; 
        }
    }
    return NULL;
}

// 保护现场寄存器
void SaveReg(){
    printf("\taddi $sp, $sp, -68\n");
    int i;
    for(i = 0; i<=7; i++){
        printf("\tsw $t%d, %d($sp)\n",i, 68 - (i + 1) * 4);
    }
    for(i = 0; i <= 7; i++){
        printf("\tsw $s%d, %d($sp)\n",i, 68 - (i + 9) *4);
    }
    printf("\tsw $ra, 0($sp)\n");
    printf("\n");
}

// 恢复现场
void ReleaseReg(){
    printf("\n");
    printf("\tlw $ra, 0($sp)\n");
    int i;
    for(i = 7; i >= 0; i--){
        printf("\tlw $s%d, %d($sp)\n",i, 68 - (i + 9) *4);
    }
    for(i = 7; i >= 0; i--){
        printf("\tlw $t%d, %d($sp)\n",i, 68 - (i + 1) * 4);
    }
    printf("\taddi $sp, $sp, 68\n");
}

void CodeHead(){
    RegInit(); // 初始化寄存器
    // 数据段
    printf(".data\n");
    printf("hint: .asciiz \"Enter an integer:\"\n");   //输入数字提示信息，read函数专用
    printf("Creturn: .asciiz \"\\n\"\n");  //回车信息，用于显示换行
    // 为数组开辟空间
    int i, rtn;
    for(i=symbolTable.index-1;i>=0;i--){
        if(symbolTable.symbols[i].flag=='A'){ //如果符号表中有数组类型
            printf("%s: .space ",symbolTable.symbols[i].name); //数组名称
            printf("%d",symbolTable.symbols[i].arraysize); // 开辟一个array大小的空间
            printf("\n");
        }
    }
    // 入口函数
    printf("\n.globl main0\n");
    printf(".text\n");
    printf("main0:");
    printf("\tjal main\n");
    // 结束程序
    printf("\tli $v0, 10\n");
    printf("\tsyscall\n");
    // read函数
    printf("\nread:\n");
    printf("\taddi $sp, $sp, -4\n");
    printf("\tsw $ra, 0($sp)\n\n"); //保护现场与断点
    printf("\tli $v0, 4\n");
    printf("\tla $a0, hint \n");
    printf("\tsyscall \n"); //输出提示信息
    printf("\tli $v0, 5\n");
    printf("\tsyscall\n"); //读入数字
    printf("\n\tlw $ra, 0($sp)\n");
    printf("\taddi $sp, $sp, 4\n"); // 恢复现场与断点
    printf("\tjr $ra\n"); //函数返回
    // write函数
    printf("\nwrite:\n");
    printf("\taddi $sp, $sp, -4\n");
    printf("\tsw $ra, 0($sp)\n\n"); //保护现场与断点
    printf("\tli $v0, 1\n");
    printf("\tsyscall\n"); //输出信息
    printf("\tli $v0, 4\n");
    printf("\tla $a0, Creturn\n");
    printf("\tsyscall\n");// 输出回车
    printf("\tmove $v0, $zero\n");
    printf("\n\tlw $ra, 0($sp)\n");
    printf("\taddi $sp, $sp, 4\n"); // 恢复现场与断点
    printf("\tjr $ra\n");
}

// 将数组的别名转换成真实名字，因为真实名字代表着存储位置
char* AliasToName(char* alias){
    int i;
    for(i=symbolTable.index-1;i>=0;i--){
        if(!strcmp(symbolTable.symbols[i].alias,alias)){
            return symbolTable.symbols[i].name;
        }
    }
    return NULL;
}


// 生成目标代码
void objectCode(struct codenode *head){
    char opnstr1[32],opnstr2[32],resultstr[32]; //存放别名
    char SrcReg1[32],SrcReg2[32],ResultReg[32]; //存放寄存器名或临时变量
    struct codenode *h=head;
    char array_name[32]; //程序过程中识别到array
    int mainflag = 0; //表示当前在main函数中
    CodeHead();  //生成数据段代码、read和write函数代码
    do {
        //第一操作数
        if (h->opn1.kind==INT)
             sprintf(opnstr1,"%d",h->opn1.const_int);
        if (h->opn1.kind==FLOAT)
             sprintf(opnstr1,"%f",h->opn1.const_float);
        if (h->opn1.kind==CHAR)
            sprintf(opnstr1,"%c",h->opn1.const_char);
        if (h->opn1.kind==ID)
             sprintf(opnstr1,"%s",h->opn1.id);

        //第二操作数
        if (h->opn2.kind==INT)
             sprintf(opnstr2,"%d",h->opn2.const_int);
        if (h->opn2.kind==FLOAT)
             sprintf(opnstr2,"%f",h->opn2.const_float);
        if(h->opn2.kind==CHAR)
            sprintf(opnstr2,"%c",h->opn2.const_char);
        if (h->opn2.kind==ID)
             sprintf(opnstr2,"%s",h->opn2.id);

        // 处理result
        if (h->result.kind==INT)
             sprintf(resultstr,"%d",h->result.const_int);
        if (h->result.kind==FLOAT)
             sprintf(resultstr,"%f",h->result.const_float);
        if(h->result.kind==CHAR)
            sprintf(resultstr,"\'%c\'",h->result.const_char);
        if(h->result.kind!=INT&&h->result.kind!=FLOAT&&h->result.kind!=CHAR)
            sprintf(resultstr,"%s",h->result.id);

        // 处理运算符
        switch (h->op) {
            case ASSIGNOP: if(h->opn1.kind == ID){
                                strcpy(ResultReg,DiliverReg(resultstr));
                                strcpy(SrcReg1,DiliverReg(opnstr1));
                                printf("\tmove %s, %s\n",ResultReg,SrcReg1);
                           }
                           else{
                                strcpy(ResultReg,DiliverReg(resultstr));
                                printf("\tli %s, %s\n",ResultReg,opnstr1);
                           }
                           break;
            case PLUS:     strcpy(ResultReg, DiliverReg(resultstr));
                           strcpy(SrcReg1, DiliverReg(opnstr1));
                           strcpy(SrcReg2, DiliverReg(opnstr2));
                           printf("\tadd %s, %s, %s\n",ResultReg,SrcReg1,SrcReg2);
                           break;
            case MINUS:    strcpy(ResultReg, DiliverReg(resultstr));
                           strcpy(SrcReg1, DiliverReg(opnstr1));
                           strcpy(SrcReg2, DiliverReg(opnstr2));
                           printf("\tsub %s, %s, %s\n",ResultReg, SrcReg1, SrcReg2);
                           break;
            case STAR:     strcpy(ResultReg, DiliverReg(resultstr));
                           strcpy(SrcReg1, DiliverReg(opnstr1));
                           strcpy(SrcReg2, DiliverReg(opnstr2));
                           printf("\tmul %s, %s, %s\n",ResultReg, SrcReg1, SrcReg2);
                           break;
            case DIV:      
                           strcpy(SrcReg1, DiliverReg(opnstr1));
                           strcpy(SrcReg2, DiliverReg(opnstr2));
                           printf("\tdiv %s, %s, %s\n",ResultReg ,SrcReg1, SrcReg2);
                           printf("\tmflo %s\n",ResultReg);  // 注意div结果的高位存的是余数，我们只能取得低位
                           break;
            case DPLUS:    strcpy(ResultReg, DiliverReg(resultstr));
                           printf("\taddi %s, %s, 1\n",ResultReg, ResultReg);
                           break;
            case DMINUS:   strcpy(ResultReg, DiliverReg(resultstr));
                           printf("\taddi %s, %s, -1\n",ResultReg, ResultReg);
                           break;
            case ARRAYOP:  strcpy(ResultReg, DiliverReg(resultstr));
                           //获取对应数组在存储中的位置
                           strcpy(array_name,AliasToName(opnstr1));
                           printf("\tla $t8, %s\n",array_name); //t8加载array的地址
                           if(h->opn2.kind == ID){ //如果是ID的话，偏移用mul乘法计算
                                strcpy(SrcReg2, DiliverReg(opnstr2));
                                printf("\tmul $t9, %s, %d\n",SrcReg2, symbol_width(h->opn2.type)); //计算偏移量
                                printf("\tadd $t8, $t8, $t9\n"); //计算地址
                                printf("\tlw %s, 0($t8)\n",ResultReg);  //从存储中导出array
                           }
                           else{
                                int Array_Offset;
                                Array_Offset = h->opn2.const_int * symbol_width(h->opn2.type); //计算偏移量
                                printf("\taddi $t8, $t8, %d\n",Array_Offset); //计算地址
                                printf("\tlw %s, 0($t8)\n",ResultReg);  //从存储中导出array
                           }
                           break;
            case ARR_ASSIGN: strcpy(ResultReg, DiliverReg(resultstr));
                             //获取对应数组在存储中的位置
                             strcpy(array_name,AliasToName(opnstr1));
                             printf("\tla $t8, %s\n",array_name); //t8加载array的地址
                             if(h->opn2.kind == ID){ //如果是ID的话，偏移用mul乘法计算
                                strcpy(SrcReg2, DiliverReg(opnstr2));
                                printf("\tmul $t9, %s, %d\n",SrcReg2, symbol_width(h->opn2.type)); //计算偏移量
                                printf("\tadd $t8, $t8, $t9\n"); //计算地址
                                printf("\tsw %s, 0($t8)\n",ResultReg);  //导入存储
                             }
                             else{
                                int Array_Offset;
                                printf("%d\n, %d\n",h->opn2.const_int, h->opn2.kind);
                                Array_Offset = h->opn2.const_int * symbol_width(h->opn2.type); //计算偏移量
                                printf("\taddi $t8, $t8, %d\n",Array_Offset); //计算地址
                                printf("\tsw %s, 0($t8)\n",ResultReg);  //导入存储
                             }
                             break;
            case FUNCTION: printf("\n%s:\n",resultstr);
                           //保护现场与断点
                           if(!strcmp(resultstr,"main")){
                                printf("\taddi $sp, $sp, -4\n");
                                printf("\tsw $ra, 0($sp)\n\n");
                                mainflag = 1;
                           }
                           else{
                               SaveReg();
                           }
                           break;
            case PARAM:    strcpy(ResultReg, DiliverReg(resultstr));
                           printf("\tmove %s, $a0\n",ResultReg);
                           break;
            case LABEL:    printf("%s:\n",resultstr);
                           break;
            case UMINUS:   strcpy(SrcReg1, DiliverReg(opnstr1));
                           strcpy(ResultReg, DiliverReg(resultstr));
                           printf("\tsub %s, $zero, %s\n",ResultReg,SrcReg1);
                           break;
            case GOTO:     printf("\tj %s\n",resultstr);
                           break;
            case JLE:      strcpy(SrcReg1,DiliverReg(opnstr1));
                           strcpy(SrcReg2,DiliverReg(opnstr2));
                           printf("\tble %s, %s, %s\n",SrcReg1,SrcReg2,resultstr);
                           break;
            case JLT:      strcpy(SrcReg1,DiliverReg(opnstr1));
                           strcpy(SrcReg2,DiliverReg(opnstr2));
                           printf("\tblt %s, %s, %s\n",SrcReg1,SrcReg2,resultstr);
                           break;
            case JGE:      strcpy(SrcReg1,DiliverReg(opnstr1));
                           strcpy(SrcReg2,DiliverReg(opnstr2));
                           printf("\tbge %s, %s, %s\n",SrcReg1,SrcReg2,resultstr);
                           break;
            case JGT:      strcpy(SrcReg1,DiliverReg(opnstr1));
                           strcpy(SrcReg2,DiliverReg(opnstr2));
                           printf("\tbgt %s, %s, %s\n",SrcReg1,SrcReg2,resultstr);
                           break;
            case EQ:       strcpy(SrcReg1,DiliverReg(opnstr1));
                           strcpy(SrcReg2,DiliverReg(opnstr2));
                           printf("\tbeq %s, %s, %s\n",SrcReg1,SrcReg2,resultstr);
                           break;
            case NEQ:      strcpy(SrcReg1,DiliverReg(opnstr1));
                           strcpy(SrcReg2,DiliverReg(opnstr2));
                           printf("\tbne %s, %s, %s\n",SrcReg1,SrcReg2,resultstr);
                           break;
            case ARG:      strcpy(ResultReg,DiliverReg(resultstr));
                           printf("\tmove $a0, %s\n",ResultReg); // 将参数放入$a0
                           break;
            case CALL:      if(!strcmp(opnstr1,"write")){ // write函数
                                printf("\tjal write\n");
                            }
                            else{ //其他函数
                                //结果寄存器
                                strcpy(ResultReg,DiliverReg(resultstr));
                                //跳转到函数执行
                                printf("\tjal %s\n",opnstr1);
                                // 结果返回结果寄存器
                                printf("\tmove %s, $v0\n",ResultReg);
                            }
                           break;
            case RETURN:   if (h->result.kind){
                                strcpy(ResultReg,DiliverReg(resultstr));
                                printf("\tmove $v0, %s\n",ResultReg); //返回参数
                                // 恢复现场与断点
                                if(mainflag == 1){ //在main函数中
                                    printf("\n\tlw $ra, 0($sp)\n");
                                    printf("\taddi $sp, $sp, 4\n");
                                    printf("\tjr $ra\n"); //返回
                                }
                                else{
                                    ReleaseReg();
                                    printf("\tjr $ra\n"); //返回
                                }
                            }
                           else{
                               // 恢复现场
                                printf("\n\tlw $ra, 0($sp)\n");
                                printf("\taddi $sp, $sp, 4\n");
                                printf("\tjr $ra\n"); //返回
                           }
                           break;
        }
    h=h->next;
    } while (h!=head);
}


