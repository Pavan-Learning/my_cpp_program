#include <iostream>


class base {
public:
    virtual void display() = 0; // pure virtual function
    virtual ~base() {} // virtual destructor
};

class derived1 : public base {
public:
    void display() override {
        std::cout << "Derived1 display function called." << std::endl;
    }

    void show() {
        std::cout << "Derived1 show function called." << std::endl;
    }
};

class derived2 : public derived1 {
public:
    void display() override {
        std::cout << "Derived2 display function called." << std::endl;
    }

    void show() {
        std::cout << "Derived2 show function called." << std::endl;
    }
};

int main() {
    derived1 d1;
    base& obj1 = d1;
    obj1.display();

    derived2 d2;
    base& obj2 = d2;
    obj2.display();

    // base b = d1; // we cannot create an object of base class because it is abstract, but we can create a reference or pointer to base class and assign it to derived class object.

    // derived2 de = d1; // this is also give compile time error because we cannot assign derived class object to base class object.

    derived1 de = d2; // this will not give compile time error due assigning derived obj to base obj but it might give object slicing.

    de.show(); // this will show only derived1 show funtion beacuse d2 part is sliced off and only derived1 part is assigned to de.

    derived1* ptr1 = new derived1();
    base* basePtr1 = ptr1;
    basePtr1->display();
    delete basePtr1;

    derived1* derivedPtr2 =  new derived2();; // upcasting
    derivedPtr2->display(); // this will call derived2 display function because  here we have vptr of derived2 class and it will call derived2 display function.
    std::cout<<"*******"<<std::endl;
    derivedPtr2->show(); // here we are derived1 is base class of derived2 and it have show function of derived1 class so it will call derived1 show function.
                        // due to it is non virtual fucntion and in complie time, complie decides like I have derived1 pointer and derived1 class have show function so I will call derived1 show function.
    delete derivedPtr2;

    base* basePtr2 = new derived2(); // upcasting
    basePtr2->display(); // this will call derived2 display function because  here we have vptr of derived2 class and it will call derived2 display function.
    // basePtr2->show(); // this will give compile time error because base class doesn't have show function so it doesn't in base class vtable and it will not be able to call derived2 show function.
    delete basePtr2;

    return 0;
}