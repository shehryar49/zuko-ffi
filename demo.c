#include <stdio.h>
#include <stdlib.h>

int sum(int x,int y)
{
    return x+y;
}
void sayhello()
{
    puts("hello!");
    fflush(stdout);
}
double doubleSum(double x,double y)
{
    return x+y;
}
void MYPUTS(const char* str)
{
    puts(str);
}

char* getStr()
{
    static char buffer[] = "hello from C!";
    return buffer; // the ffi module should deepcopy if return type is C_STR
    //deepcopy is not done for C_PTR
}
void printPointer(void* p)
{
  printf("ptr = %p\n",p);
}