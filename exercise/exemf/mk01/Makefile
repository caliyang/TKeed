.PHONY : all test clean

all : libcompare.so

# 如果是libcompare.so: max.o，那么less.o不会生成。可以看出其他目标是为最终目标的生成服务的。
libcompare.so : max.o less.o
# gcc -shared $^ -o $@等价于gcc -shared max.o less.o -o $@ 
# gcc -shared $< -o $@等价于gcc -shared max.o -o $@ 
	gcc -shared $^ -o $@

# max.o : max.c compare.h，在此处可以加这条规则，与第15行的规则并存。
# 由于11行的规则没有command，按理说需要用隐式规则来推导，但是后面定义了显示规则，因此还是使用该显示规则的command。
# 这样有个好处就是把头文件的更新考虑进源文件的重新编译上了，因为原来的%.o : %.c的静态模式是无法把头文件的修改导致源文件的重新编译考虑进去。
# 在第15行，不带compare.h的话，虽不影响Makefile指令的执行，但头文件的修改不会导致相关源文件的重新编译和目标文件的重新链接，因为依赖的源文件没有改变。
max.o : max.c # compare.h
	gcc -c -fPIC $<

less.o : less.c # compare.h
	gcc -c -fPIC $<

test : testexe
testexe : test.c libcompare.so
	gcc test.c -L. -lcompare -o testexe
	LD_LIBRARY_PATH=. ./testexe

clean :
	rm -f *.o *.so testexe