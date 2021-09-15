#include "hello.h"

int main()
{
    Hello *phello = new Hello();
    phello->Hello_print();
    delete phello;
    return 0;
}