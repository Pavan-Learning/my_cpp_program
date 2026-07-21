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

    //──────────────────────────────────────────────────────────
    // NOTE: base b = d1;
    // REASON: base is abstract (has pure virtual function)
    //         → cannot create object, only pointer/reference
    //──────────────────────────────────────────────────────────

    //──────────────────────────────────────────────────────────
    // NOTE: derived2 de = d1; → COMPILE ERROR
    // REASON: derived1 doesn't know about derived2's extra members
    // EXAMPLE: Person(name,age) → Student(name,age,rollno)
    //          Person can't fill Student because rollno is unknown
    //──────────────────────────────────────────────────────────

    //──────────────────────────────────────────────────────────
    // NOTE: derived1 de = d2; → OBJECT SLICING
    // REASON: d2 is copied into a derived1 variable
    //         → derived2 part is sliced off, only derived1 remains
    //──────────────────────────────────────────────────────────
    derived1 de = d2;

    de.show(); // RESULT: calls derived1::show() because derived2 part is sliced off

    //──────────────────────────────────────────────────────────
    // NOTE: base pointer to derived1 object
    // REASON: display() is virtual → resolved at runtime via vptr
    // RESULT: calls derived1::display()
    //──────────────────────────────────────────────────────────
    derived1* ptr1 = new derived1();
    base* basePtr1 = ptr1;
    basePtr1->display();
    delete basePtr1;

    //──────────────────────────────────────────────────────────
    // NOTE: derived1 pointer to derived2 object (upcasting)
    // display() → virtual → resolved at runtime → calls derived2::display()
    // show()    → non-virtual → resolved at compile time by pointer type
    //            → compiler sees derived1* → calls derived1::show()
    //──────────────────────────────────────────────────────────
    derived1* derivedPtr2 = new derived2();
    derivedPtr2->display(); // RESULT: derived2::display() — virtual, uses vptr
    std::cout << "*******" << std::endl;
    derivedPtr2->show();    // RESULT: derived1::show() — non-virtual, uses pointer type
    delete derivedPtr2;

    //──────────────────────────────────────────────────────────
    // NOTE: base pointer to derived2 object (upcasting)
    // display() → virtual → calls derived2::display() via vptr
    // show()    → NOT in base class → COMPILE ERROR if called
    // REASON: compiler only sees base* → base has no show()
    //──────────────────────────────────────────────────────────
    base* basePtr2 = new derived2();
    basePtr2->display(); // RESULT: derived2::display() — virtual, uses vptr
    // basePtr2->show(); // ERROR: base class has no show() in its interface
    delete basePtr2;

    return 0;
}