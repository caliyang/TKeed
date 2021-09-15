#!/bin/bash

if [ $1 -eq "1" ]; then
    echo "banzhang zhen shuai"
    elif [ $1 -eq "2" ]; then
    echo "cls zhen mei"
    elif [ $1 -eq "3" ]; then
    echo "cls zhen mei"
fi

if [ -f file.txt ]; then
    echo "文件存在!"
else
    echo "文件不存在!"
fi

