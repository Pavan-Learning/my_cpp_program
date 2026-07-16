#include <iostream>


/// here in this code demonstates the move semantics like std::move, Move constructor, Move assignement operator,
// key points if we create object and do assinment with std::move it will call the move constructor
/// if we create object and do assignment with std::move it will call the move assignment operator
//  another important point is in std::move it will cast the lvalue to rvalue with remove_reference by doing static cast,
class MyClass {
    private:
        int* memberVariable;
        int size;
public:
    MyClass() : memberVariable(new int(455)), size(10) {}
    MyClass(int x, int s) : memberVariable(new int(x)), size(s) {}
    MyClass(const MyClass& other) : memberVariable(new int(*other.memberVariable)), size(other.size) {
        std::cout << "Copy constructor called" << std::endl;
    }
    MyClass(MyClass&& other) noexcept : memberVariable(other.memberVariable), size(other.size) {
        std::cout << "Move constructor called" << std::endl;
        other.memberVariable = nullptr; // Reset the source object's member variable to a default value
        other.size = 0; // Reset the source object's size to a default value
    }
    MyClass& operator=(const MyClass& other) {
        std::cout << "Copy assignment operator called" << std::endl;
        if (this != &other) {
            delete memberVariable; // Free the existing resource
            memberVariable = new int(*other.memberVariable);
            size = other.size;
        }
        return *this;
    }
    MyClass& operator=(MyClass&& other) noexcept {
        std::cout << "Move assignment operator called" << std::endl;
        if (this != &other) {
            delete memberVariable; // Free the existing resource
            memberVariable = other.memberVariable;
            size = other.size;
            other.memberVariable = nullptr; // Reset the source object's member variable to a default value
            other.size = 0; // Reset the source object's size to a default value
        }
        return *this;
    }
    void printMemberVariableAndSize() const {
        std::cout << "Member variable value: " << memberVariable << std::endl;
        std::cout << "Size value: " << size << std::endl;
    }
    ~MyClass() {
        delete memberVariable; // Free the allocated resource
    }
};

class TestClass {
    private:
        MyClass myObj;
    public:
        TestClass(int x, int s) : myObj(x, s) {}
        void testMoveSemantics() {
            std::cout << "Before move, myObj state:" << std::endl;
            myObj.printMemberVariableAndSize();
            MyClass newObj = std::move(myObj); // Move myObj to newObj
            std::cout << "After move, newObj state:" << std::endl;
            newObj.printMemberVariableAndSize();
            std::cout << "After move, myObj state (should be reset):" << std::endl;
            myObj.printMemberVariableAndSize(); // Should show reset values
        }
    
};

int main() {
    TestClass test(100, 20);
    test.testMoveSemantics();
    // const TestClass test2(500, 11);
    // std::cout << "**********Testing move semantics with const object:**************" << std::endl;
    // test2.testMoveSemantics(); // This will not compile because test2 is const and we cannot move from a const object
    MyClass obj1(200, 30);
    std::cout << "**********Testing move semantics with non-const MyClass object:**************" << std::endl;
    MyClass obj2 = std::move(obj1);
    const MyClass obj3(400, 30);
    std::cout << "**********Testing move semantics with const MyClass object:**************" << std::endl;
    MyClass obj4 = std::move(obj3); // here std::move(obj3) → const MyClass&&; here  Move constructor cannot accept const objects, so it will call the copy constructor instead of move constructor, because move constructor cannot accept const objects, so it will call the copy constructor instead of move constructor.
    return 0;
}