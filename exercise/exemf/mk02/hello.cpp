
#include "hello.h"
#include <stdio.h>
#include <typeinfo>

Hello::Hello() {
    printf("%s Constructor \n", __FUNCTION__);
}

Hello::~Hello() {
    printf("%s Destructor\n", __FUNCTION__);
}

int Hello::Hello_print() {
    printf("%s:%s hello Makefile\n", __FILE__, __FUNCTION__);

    return 0;
}