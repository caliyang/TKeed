root_path := $(shell pwd)
build_path := $(root_path)/bin
obj_path := $(root_path)/obj_path

# all和mknewdir均是标签，有些意思
all : mknewdir hello.o main.o
	g++ $(obj_path)/hello.o $(obj_path)/main.o -o $(build_path)/hello

hello.o : hello.cpp
	g++ -c hello.cpp -o $(obj_path)/hello.o 

main.o : main.cpp
	g++ -c main.cpp -o $(obj_path)/main.o

mknewdir :
	mkdir -p $(obj_path)
	mkdir -p $(build_path)

# Makefile中的shell，每一行是一个进程，不同行之间变量值不能传递。
# 所以，Makefile中的shell不管多长也要写在一行.
test1 :
# ./hello在cd bin的基础上执行
	cd bin; ./hello

test2 :
# 以./开头就行 
	./bin/hello

test3 :
# 下面两个指令是独立的，都是相对于当前工作目录而言
	cd bin
	./hello

clean :
	rm -r $(obj_path) $(build_path)


