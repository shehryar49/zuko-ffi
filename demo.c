#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//Test void
void test_void()
{
    puts("--test_void()--");
}
//Test int
int test_int(int x,int y)
{
    puts("--test_int()--");
    return x+y;
}
//Test long
long test_long(long x,long y)
{
    puts("--test_long()--");
    return x*y;
}
//Test float
float test_float(float x,float y)
{
    puts("--test_float()--");
    return x/y;
}
//Test double
double test_double(double x,double y)
{
    puts("--test_double()--");
    return x*y;
}
//Test int64
long long int test_int64(long long int x,long long int y)
{
    puts("--test_int64()--");
    return x*y;
}
//Test bool
bool test_bool(bool x,bool y)
{
    puts("--test_bool()--");
    return x^y;
}
//Test char
char test_char(char x,char y)
{
    puts("--test_char()--");
    return x+y;
}
//Test Ptr
int* getAddr()
{
    static int x = 69;
    return &x;
}
int* test_ptr(int* x)
{
    puts("--test_ptr()--");
    printf("x = %d\n",*x);
    return x;
}
//Test C_STR
char* test_str(char* str)
{
    puts("--test_str()--");
    puts(str);
    return str; // the ffi module will make deep copy of this
}