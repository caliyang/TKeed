#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct
{
    int val[100];
    int top;
} Num;

void Init(Num *temp)                   //也可定义一个指向结构体的指针*Linklist
{                                      //这里为了便于理解，使用二重指针
    temp = (Num *)malloc(sizeof(Num)); //由于函数内是对结构体指针本身做操作，而不是结构体内的变量操作
    temp->top = 0;                     //因此调用函数进行操作时，同样需要使用指针，且为二重指针，即指针的指针
    temp->val[0] = -1;                 //才能在函数内对结构体分配空间，并传出函数
}

int main()
{
    Num *ptr;
    Init(ptr);
    printf("val=%d top=%d \n", ptr->val[0], ptr->top);

    return 0;
}
