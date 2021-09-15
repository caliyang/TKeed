#include <iostream>

int total = 0;

int square(int i)
{
    int result=0;

    result = i*i;

    return result;
}

int main(int argc, char **argv)
{
    int i;

    for(i=0; i<10; i++)
    {
        total += square(i);
    }
    return 0;
}