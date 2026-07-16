/*
 * ============================================================================
 * LVALUE REFERENCE vs RVALUE REFERENCE vs CONST LVALUE REFERENCE OVERLOADS
 * ============================================================================
 * 
 * This program demonstrates function overloading with different reference types:
 *
 * 1. printValue(int& value)
 *    - Takes a NON-CONST lvalue reference
 *    - Can ONLY bind to lvalues (variables with memory addresses)
 *    - Called when: printValue(a) where 'a' is a variable
 *    - Output: "lvalue"
 *
 * 2. printValue(const int& value)
 *    - Takes a CONST lvalue reference
 *    - Can bind to lvalues, const lvalues, AND rvalues
 *    - More general/permissive than other overloads
 *    - Called for temporaries and rvalues in many cases
 *    - Output: "const lvalue"
 *
 * 3. printValue(int&& value)
 *    - Takes an rvalue reference
 *    - Can ONLY bind to rvalues (temporaries without names)
 *    - Used for move semantics and forwarding
 *    - Called when: printValue(std::move(a)) or printValue(20)
 *    - Output: "rvalue"
 *
 * ============================================================================
 * KEY CONCEPT: When both const lvalue ref and rvalue ref match, most
 * compilers prefer const lvalue ref because it's more general.
 * ============================================================================
 */

#include <iostream>

int a = 20;
int printValue(int& value) { // here value is an lvalue reference, it can only bind to lvalues, it cannot bind to rvalues
    std::cout << " lvalue: " << value << std::endl; // output: value of the argument passed to the function
    return value; // return the value of the argument passed to the function
}

int printValue(const int& value) { // here value is a const lvalue reference, it can bind to both lvalues and rvalues, but it cannot modify the value of the argument passed to the function
    std::cout << " const lvalue: " << value << std::endl; // output: value of the argument passed to the function
    return value; // return the value of the argument passed to the function
}

// rvalue are temporary objects that do not have a name and cannot be modified, they are typically used for move semantics and perfect forwarding, they can be created using std::move or by using a literal value like 20, they can also be created by calling a function that returns an rvalue, such as std::move(a) or by using a lambda function that returns an rvalue
int printValue(int&& value) { // here value is an rvalue reference, it can only bind to rvalues, it cannot bind to lvalues
    std::cout << " rvalue: " << value << std::endl; // output: value of the argument passed to the function
    return value; // return the value of the argument passed to the function
}


int main() {
    // Lambda functions with capture-default cannot be defined at global scope
    // They must be defined inside a function (like main) or a class member function
    auto checkwithlambda = [&](const int& x) { a+= x; return a;};
    int a = 10; // here a is the lvalue, it have name of variable and it have memory address, we can take the address of a using &a
    const int& refA = a; // here refA is the lvalue reference to a, it is an alias for a, it also have name and memory address, we can take the address of refA using &refA, but it will give the same address as a because refA is just an alias for a
    std::cout << "Value of a: " << a << std::endl; // output: 10
    std::cout << "Value of refA: " << refA << std::endl; // output: 10
    std::cout << "Address of a: " << &a << std::endl; // output: address of a
    std::cout << "Address of refA: " << &refA << std::endl; // output: same address as a
    printValue(a); // here we are passing an lvalue to the function, it will work fine because printValue takes an lvalue reference
    printValue(20); // here we are passing an lvalue reference to the function, it will work fine because printValue takes an lvalue reference
    // the  calling with 20 of rvalue it is calling the printValue(const int& value) function because 20 is an rvalue and it can bind to const lvalue reference, but it cannot bind to lvalue reference due some complier prefer here to take const lvalue reference over rvalue reference, so it will call the printValue(const int& value) function 
    // but in other compilers it may call the printValue(int&& value) function because 20 is an rvalue and it can bind to rvalue reference, but it cannot bind to lvalue reference, so it will call the printValue(int&& value) function

    std::cout << " the value of checkwithlambda :" << checkwithlambda(5) << std::endl; // here we are passing an lvalue reference to the lambda function, it will work fine because the lambda function takes a const lvalue reference, it will add 5 to a and return the new value of a, so the output will be 15
    std::cout << " the value of a after calling lambda: " << a << std::endl; // here we are printing the value of a after calling the lambda function, it will
}