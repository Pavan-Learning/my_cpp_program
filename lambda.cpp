#include <iostream>

class MyClass {
private:
    int memberVariable;
public:
    MyClass() : memberVariable(455) {}
    void memberFunction(int x) {
        memberVariable = x;
    }

    void printMemberVariable() const {
        std::cout << "Member variable value: " << memberVariable << std::endl;
    }

    void testLambdaWithThis(int x) {
        auto myClassLambdaWithCapture = [*this](int x) mutable {
            memberFunction(x);
            std::cout << "Inside lambda ([*this] copy), member variable: " << memberVariable << std::endl;
            printMemberVariable(); // This will print the member variable of the lambda's copy, not the original object
        };

        auto myClassLambdaWithCaptureRef = [this](int x) {
            memberFunction(x);
            std::cout << "Inside lambda ([this] reference), member variable: " << memberVariable << std::endl;
            printMemberVariable(); // This will print the member variable of the original object
        };
        myClassLambdaWithCaptureRef(x);
        myClassLambdaWithCapture(x);
        
        std::cout << "After lambda call, original object member variable: " << memberVariable << std::endl;
    }
};

int main(){
    auto lambda = [](int x) { return x * x; };
    int result = lambda(5);
    std::cout << "The square of 5 is: " << result << std::endl;

    int temp = 10;
    auto captureLambda = [&temp](int x) { temp += x;
        return x + temp; };
    int captureResult = captureLambda(5);
    std::cout << "The result of adding 5 and captured temp (10) is: " << captureResult << std::endl;   
    
    // why we need mutable lambda here is inside lambda we first copy the temp value and it is defultly const, 
    // so we cannot modify it, but if we want to modify the captured value we need to use mutable lambda or 
    // need to use reference capture like above and this will modify the original temp value as well.
    // if we use mutable lambda, we can modify the captured value inside the lambda, but it will not affect
    // the original value outside the lambda.
    auto mutableLambda = [temp](int x) mutable { temp += x; return temp; };
    int mutableResult = mutableLambda(5);
    std::cout << "The result of mutable lambda after adding 5 to captured temp (10) is: " << mutableResult << std::endl;
    std::cout << "The value of temp after mutable lambda is: " << temp << std::endl; // temp remains unchanged

    auto myClassLambda = [](MyClass& obj, int x) { obj.memberFunction(x); };
    MyClass myObj;
    myClassLambda(myObj, 42);
    myObj.printMemberVariable();
    myObj.memberFunction(455); // reset member variable to original value

    std::cout << "********* before default object value ********"<<std::endl;

    myObj.printMemberVariable();

    std::cout << "********** after lambda with capture ********"<<std::endl;
    auto myClassLambdaWithCapture = [&myObj, temp](int x) mutable { x += temp; myObj.memberFunction(x); };
    myClassLambdaWithCapture(100);
    myObj.printMemberVariable();

    myObj.testLambdaWithThis(200);

    myObj.printMemberVariable();


    int a = 10;
    int b = 20;
    int c = 30;

    auto caputure = [z = 100] (int x, int y ){ // here z is declared and initialized to 100, and it is captured by value. This means that z is a local variable to the lambda and cannot be accessed outside of it.
        return z =  x + y;
    } (50, 60);
    // std::cout << z << std::endl; // z cannot be printed because [z = 100] creates a variable local to the lambda, and it is not accessible outside that lambda.
    
    /*
    
    class Lambda {
        int z = 100;   // stored inside object

    public:
        int operator()(int x, int y) const {  // ❗ const function
            return z = x + y; // ❌ not allowed
        }
    };

    this is way lambda will create with operator() const, so we cannot modify the captured value inside the lambda, because it is a const function.
    here it will store the variables in stack and registier like how object

    2. Here when we create any lambda it will create new stack frame for that lambda varibale like below 
   
    +----------------------------------+
    | main() stack frame               |
    |----------------------------------|
    | a = 10                           |
    | result (uninitialized)           |
    | f (lambda object)                |
    |   └── z = 100                    |
    +----------------------------------+

    this is for lambda stack frame
    +----------------------+
    | return address       |
    | x = 50               |
    | y = 60               |
    | this → f             |
    +----------------------+
    | f object             |
    | result               |
    +----------------------+

Top of Stack
      ↓

    +----------------------------------+
    | Lambda stack frame               |
    |----------------------------------|
    | return address                   |
    | x = 50                           |
    | y = 60                           |
    | this → f (lambda object)         |
    +----------------------------------+
    | main() stack frame               |
    |----------------------------------|
    | a = 10                           |
    | result                           |
    | f → { z = 100 }                  |
    +----------------------------------+


        Before call:

    Stack:
    +--------------------------+
    | main frame               |
    | a = 10                   |
    | result                   |
    | f → { z=100 }            |
    +--------------------------+

    During call:

    +--------------------------+
    | lambda frame             |
    | return address           |
    | x = 50                   |
    | y = 60                   |
    | this → f                 |
    +--------------------------+
    | main frame               |
    +--------------------------+

    After call:

    +--------------------------+
    | main frame               |
    | result = 210             |
    | f → { z=100 }            |
    +--------------------------+


    */

    return 0;
}