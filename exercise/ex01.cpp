#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>

void test()
{
    std::shared_ptr<int> sp1(new int(22));
    std::shared_ptr<int> sp2 = sp1;
    std::shared_ptr<int> sp3 = sp1;
    std::cout << "cout: " << sp2.use_count() << std::endl; // 打印引用计数, 2

    std::cout << *sp1 << std::endl;  // 22
    std::cout << *sp2 << std::endl;  // 22

    sp1.reset(); // 显示让引用计数减一
    std::cout << "count: " << sp2.use_count() << std::endl; // 打印引用计数, 1

    std::cout << *sp2 << std::endl; // 22

    return;
}
int main()
{
    test();
    return 0;
}