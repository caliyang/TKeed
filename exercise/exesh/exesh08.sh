#!/bin/bash

function sum()
{
    s=0
    s=$[$1+$2]
    echo "$s"
    return 20
}

read -p "Please input the number1: " n1
read -p "Please input the number2: " n2
sum $n1 $n2
