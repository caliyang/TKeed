#include <iostream>
using namespace std;

// Cat类禁用默认构造函数, 无法默认初始化
class Cat {
 public:
    int age;
    Cat() = delete;
};

int main() {
    int *pi2 = new int(); 
    cout << *pi2 << ", " << endl;
	
	return 0;
}