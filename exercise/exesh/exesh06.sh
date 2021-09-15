#!/bin/bash

# 类似Makefile当中的$<
for i in $*
do
    echo "01 ban zhang love $i "
done

for j in $@
do
    echo "02 ban zhang love $j"
done

# 类似Makefile当中的$^
for k in "$*"
do
    echo "03 ban zhang love $k"
done

for l in "$@"
do
    echo "04 ban zhang love $l"
done