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