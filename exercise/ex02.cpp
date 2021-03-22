#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>

using namespace std;

class CB;
class CA
{
public:
    CA() { cout << "CA() called! " << endl; }
    ~CA() { cout << "~CA() called! " << endl; }
    void set_ptr(shared_ptr<CB>& ptr) { m_ptr_b = ptr; }
    void b_use_count() { cout << "b_in use count : " << m_ptr_b.use_count() << endl; }
    void show() { cout << "this is class CA!" << endl; }
private:
    shared_ptr<CB> m_ptr_b;
};

class CB
{
public:
    CB() { cout << "CB() called! " << endl; }
    ~CB() { cout << "~CB() called! " << endl; }
    void set_ptr(shared_ptr<CA>& ptr) { m_ptr_a = ptr; }
    void a_use_count() { cout << "a_in use count : " << m_ptr_a.use_count() << endl; }
    void show() { cout << "this is class CB!" << endl; }
private:
    shared_ptr<CA> m_ptr_a;
};

void test_refer_to_each_other()
{
    shared_ptr<CA> ptr_a(new CA());
    shared_ptr<CB> ptr_b(new CB());

    cout << "a use count : " << ptr_a.use_count() << endl;
    ptr_a->b_use_count();
    cout << "b use count : " << ptr_b.use_count() << endl;
    ptr_b->a_use_count();

    ptr_a->set_ptr(ptr_b);
    ptr_b->set_ptr(ptr_a);

    cout << "a use count : " << ptr_a.use_count() << endl;
    ptr_a->b_use_count();
    cout << "b use count : " << ptr_b.use_count() << endl;
    ptr_b->a_use_count();
}

int main()
{
    test_refer_to_each_other();
    return 0;
}

/* 测试结果
CA() called!
CB() called!
a use count : 1
b use count : 1
a use count : 2
b use count : 2
*/