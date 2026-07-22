#include <iostream>
#include <mutex>
#include <thread>
#include <functional>

using std::mutex;
using std::once_flag;

// please refer this page to understand the singleton design pattern in C++: https://medium.com/@lokeshbihani99/singleton-pattern-in-c-8bbf1e53c023

// Singleton class implementation
// Singleton class ensures that only one instance of the class is created and provides a global point of access to that instance.
// The Singleton class is implemented using a private static pointer to the instance and a public static method to get the instance
// The constructor is private to prevent direct instantiation of the class.
// This implementation is not thread-safe.
class Singleton {
private:
    static Singleton* instance;
    static int count;
    Singleton();
public:
    static Singleton* getInstance();
};

Singleton* Singleton::instance = nullptr;
int Singleton::count = 0;
Singleton::Singleton() {
    std::cout << "Singleton instance created." << std::endl;
    count++;
}

Singleton* Singleton::getInstance() {
    if (instance == nullptr) {
        instance = new Singleton();
        std::cout << "Singleton instance count: " << count << std::endl;
    }
    return instance;
}

// Thread-safe Singleton class implementation
// This implementation uses a mutex to ensure that only one thread can create the instance at a time.
// The getInstance() method checks if the instance is null, and if so, it locks the mutex and checks again before creating the instance.
// This ensures that only one instance of the class is created even in a multi-threaded environment.
class ThreadSafeSingleton {
private:
    static ThreadSafeSingleton* instance;
    static mutex mtx;
    ThreadSafeSingleton();
public:
    static ThreadSafeSingleton* getInstance();
};
ThreadSafeSingleton* ThreadSafeSingleton::instance = nullptr;
std::mutex ThreadSafeSingleton::mtx;
ThreadSafeSingleton::ThreadSafeSingleton() {
    std::cout << "ThreadSafeSingleton instance created." << std::endl;
}

ThreadSafeSingleton* ThreadSafeSingleton::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(mtx);
        if (instance == nullptr) {
            instance = new ThreadSafeSingleton();
            std::cout << "ThreadSafeSingleton instance created." << std::endl;
        }
    }
    return instance;
}

// Singleton class implementation using std::call_once
// This implementation uses std::call_once to ensure that the instance is created only once, even in a multi-threaded environment.
// The getInstance() method uses std::call_once to call a lambda function that creates the instance if it is null.
// This ensures that only one instance of the class is created, and it is thread-safe.
// Std::call_once is a C++11 feature that provides a simple and efficient way to ensure that a piece of code is executed only once, even in a multi-threaded environment.

//drawbacks
class SingletoneWithCallOnce {
private:
    static SingletoneWithCallOnce* instance;
    static once_flag flag;
    SingletoneWithCallOnce(){
        std::cout << "SingletoneWithCallOnce instance created." << std::endl;
    }
public:
    static SingletoneWithCallOnce* getInstance();
};

SingletoneWithCallOnce* SingletoneWithCallOnce::instance = nullptr;
once_flag SingletoneWithCallOnce::flag;
SingletoneWithCallOnce* SingletoneWithCallOnce::getInstance() {
    std::call_once(flag, []() {
        instance = new SingletoneWithCallOnce();
        std::cout << "SingletoneWithCallOnce instance created." << std::endl;
    });
    return instance;
}

// Meyers' Singleton - uses a local static variable instead of new.
// Thread-safe since C++11, no heap allocation, automatic cleanup at program exit.
class MeyersSingleton {
private:
    MeyersSingleton() {
        std::cout << "MeyersSingleton instance created." << std::endl;
    }
public:
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
    static MeyersSingleton& getInstance() {
        static MeyersSingleton instance;
        return instance;
    }
    ~MeyersSingleton() {
        std::cout << "MeyersSingleton instance destroyed." << std::endl;
    }
};


// =====================================================================
// Drawbacks of Singleton Pattern (demonstrated with example classes)
// =====================================================================

// --- Drawback 1: Global State ---
// Any function anywhere can modify the singleton's data.
// This makes bugs hard to track because changes happen from unexpected places.
class GlobalCounter {
private:
    int value = 0;
    GlobalCounter() {}
public:
    GlobalCounter(const GlobalCounter&) = delete;
    GlobalCounter& operator=(const GlobalCounter&) = delete;
    static GlobalCounter& getInstance() {
        static GlobalCounter instance;
        return instance;
    }
    void increment() { value++; }
    int getValue() const { return value; }
};

// functionA() and functionB() both change the same counter without knowing about each other.
// If value is wrong, you have to search the entire codebase to find who changed it.
void functionA() {
    GlobalCounter::getInstance().increment(); // value becomes 1
}
void functionB() {
    GlobalCounter::getInstance().increment(); // value becomes 2 — but functionB didn't expect functionA ran first!
}

// --- Drawback 2: Hard to Test ---
// Imagine a Logger singleton. In tests, you want a fake logger that does nothing.
// But the singleton always returns the real one — you can't swap it out easily.
class Logger {
private:
    int logCount = 0;
    Logger() {}
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    void log(const std::string& msg) {
        logCount++;
        std::cout << "[LOG #" << logCount << "] " << msg << std::endl;
    }
    int getLogCount() const { return logCount; }
};

// test1 and test2 share the same Logger. test1's logs affect test2's logCount.
// Tests are no longer independent — running them in different order gives different results.
void test1() {
    Logger::getInstance().log("test1 running");
    // logCount is now 1
}
void test2() {
    Logger::getInstance().log("test2 running");
    // logCount is now 2, NOT 1! Because test1 already incremented it.
    // This test would FAIL if it expected logCount == 1
}

// --- Drawback 3: Hidden Dependencies ---
// OrderProcessor uses Logger internally, but its signature doesn't show it.
// Anyone reading processOrder() can't tell it depends on Logger without reading the body.
class OrderProcessor {
private:
    int orderId;
public:
    OrderProcessor(int id) : orderId(id) {}
    void processOrder() {
        // Hidden dependency! You can't tell from the outside that this uses Logger.
        Logger::getInstance().log("Processing order #" + std::to_string(orderId));
        std::cout << "Order #" << orderId << " processed." << std::endl;
    }
    // Better approach: pass Logger as a parameter (Dependency Injection)
    // void processOrder(Logger& logger) { logger.log(...); }
    // Now the dependency is visible in the function signature.
};

class SingletoneTester {
public:
    template <typename T>
    bool is_singleton(std::function<T*()> factory) {
        T* instance1 = factory();
        T* instance2 = factory();
        return instance1 == instance2;
    }
};


int main()
{
    Singleton* singleton1 = Singleton::getInstance();
    Singleton* singleton2 = Singleton::getInstance();
    SingletoneWithCallOnce* singletoneWithCallOnce1 = SingletoneWithCallOnce::getInstance();
    SingletoneWithCallOnce* singletoneWithCallOnce2 = SingletoneWithCallOnce::getInstance();

    ThreadSafeSingleton* threadSafeSingleton1 = ThreadSafeSingleton::getInstance();
    ThreadSafeSingleton* threadSafeSingleton2 = ThreadSafeSingleton::getInstance();
    ThreadSafeSingleton* threadSafeSingleton3 = ThreadSafeSingleton::getInstance();
    ThreadSafeSingleton* threadSafeSingleton4 = ThreadSafeSingleton::getInstance();

    MeyersSingleton& meyers1 = MeyersSingleton::getInstance();
    MeyersSingleton& meyers2 = MeyersSingleton::getInstance();

    // --- Drawback 1 demo: Global State ---
    std::cout << "\n--- Drawback 1: Global State ---" << std::endl;
    functionA();
    functionB();
    std::cout << "Counter value: " << GlobalCounter::getInstance().getValue()
              << " (both functions changed the same instance!)" << std::endl;

    // --- Drawback 2 demo: Hard to Test ---
    std::cout << "\n--- Drawback 2: Hard to Test ---" << std::endl;
    test1();
    test2();
    std::cout << "Total log count: " << Logger::getInstance().getLogCount()
              << " (test2 sees test1's logs too!)" << std::endl;

    // --- Drawback 3 demo: Hidden Dependencies ---
    std::cout << "\n--- Drawback 3: Hidden Dependencies ---" << std::endl;
    OrderProcessor order(101);
    order.processOrder(); // secretly uses Logger singleton inside

    // --- Singleton Tester Demo ---
    std::cout << "\n--- Singleton Tester Demo ---" << std::endl;
    SingletoneTester tester;
    bool isSingleton = tester.is_singleton<MeyersSingleton>([]() { return &MeyersSingleton::getInstance(); });
    std::cout << "MeyersSingleton is a singleton: " << (isSingleton ? "true" : "false") << std::endl;


    return 0;
}