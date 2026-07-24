#include <iostream>
#include <functional>

using namespace std;

int simple_add(int a, int b) {
    cout<< "this is simple_add function" << endl;
    return a + b;
}

void display_result(std::function<int(int, int)> func, int x, int y)
{
    cout<<"Result: " << func(x, y) << endl;
}

class call{
    int a, b;
    public:
        call(int a, int b) : a(a), b(b) {}
        int operator() (int a, int b)
        {
            this->a = a;
            this->b = b;
            cout << "this is call from class" << endl;
            return a + b;
        }
};

int main() {
    // Using a function pointer
    int (*fun_ptr)(int, int) = simple_add;

    // def for callback : A function (or any callable) that you pass to another function or object so that it can call it later, at the appropriate time.
    // Using std::function
    // here we called it as callback for fun_ptr
    display_result(fun_ptr, 5, 3);

    display_result([](int a, int b) {
        cout << "this is lambda function" << endl;
        return a + b;
    }, 10, 20);

    call c(0, 0);

    // this is the call to the functor object
    c(5, 10);

    // Using std::function to store the callable object of functor here so we called it as callback for functor
    display_result(c,15,20);
    return 0;
}