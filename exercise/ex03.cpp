#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>

using namespace std;

void check(std::weak_ptr<int> &wp) {
    std::shared_ptr<int> sp = wp.lock();  // 转换为shared_ptr<int>
    if (sp != nullptr) {
      std::cout << "still: " << *sp << std::endl;
    } else {
      std::cout << "still: " << "pointer is invalid" << std::endl;
    }
}

void test()
{
    std::shared_ptr<int> sp1(new int(22));
    std::shared_ptr<int> sp2 = sp1;
    std::weak_ptr<int> wp = sp1;  // 指向shared\_ptr<int>所指对象

    std::cout << "count: " << wp.use_count() << std::endl;  // count: 2
    std::cout << *sp1 << std::endl;  // 22
    std::cout << *sp2 << std::endl;  // 22
    check(wp);  // still: 22
    
    sp1.reset();
    std::cout << "count: " << wp.use_count() << std::endl;  // count: 1
    std::cout << *sp2 << std::endl;  // 22
    check(wp);  // still: 22

    sp2.reset();
    std::cout << "count: " << wp.use_count() << std::endl;  // count: 0
    check(wp);  // still: pointer is invalid

    return;
}

int main()
{
    test();
    return 0;
}