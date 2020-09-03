int fibo(int a)
{
    if (a == 1 || a == 2) return 1;
    return fibo(a-1)+fibo(a-2);
}
struct test{
    int Tint;
    char Tchar;
}Stest;
int array[10];

int main(){
    int i,m,k;
    i = 0;
    m = read();
    for(i = 1; i <= m; i++){
        array[i] = fibo(i); 
        write(array[i]);
    }
    // 结构体测试
    /*Stest.testint = m;
    write(Stest.testint);*/
    return 1;
}