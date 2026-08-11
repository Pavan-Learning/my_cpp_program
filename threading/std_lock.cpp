/*
 * DEADLOCK DEMO & std::lock SOLUTION
 *
 * DEADLOCK scenario (without std::lock):
 *   Thread1: transfer(A1 -> A2) => locks A1.mutex, then tries to lock A2.mutex
 *   Thread2: transfer(A2 -> A1) => locks A2.mutex, then tries to lock A1.mutex
 *
 *   Timeline:
 *     T1: lock(A1.m) ✓ ... waiting for A2.m (held by T2)
 *     T2: lock(A2.m) ✓ ... waiting for A1.m (held by T1)
 *     => Both threads wait forever = DEADLOCK
 *
 * FIX with std::lock:
 *   std::lock(lc1, lc2) locks BOTH mutexes atomically using a deadlock-avoidance
 *   algorithm (try-and-back-off). It guarantees no circular wait regardless of
 *   the order threads call it.
 *
 * Toggle USE_STD_LOCK below to see deadlock vs safe behavior.
 */

#include <bits/stdc++.h>

#define USE_STD_LOCK 1  // Set to 0 to see deadlock, 1 for safe version

class Account
{
    private:
        std::string m_name;
        int m_balance;
        std::mutex m;
    public:
        Account(std::string name, int balance) :m_name(name), m_balance(balance)
        {}
        
        void print()
        {
            std::cout<< " " << m_name << " have this much balance in his/her account: " << m_balance << std::endl;
        }
        
        void transfer(Account& from, Account& to, int amount)
        {
            std::unique_lock<std::mutex> lc1(from.m, std::defer_lock);
            std::unique_lock<std::mutex> lc2(to.m, std::defer_lock);

#if USE_STD_LOCK
            // SAFE: locks both mutexes atomically, no deadlock possible
            std::lock(lc1, lc2);
#else
            // DEADLOCK: each thread locks in opposite order
            lc1.lock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lc2.lock();
#endif
            from.m_balance -= amount;
            to.m_balance += amount;
             
            from.print();
            to.print();
        }
};


int main()
{
    Account A1("Pavan", 50000);
    Account A2("Sagar", 60000);

    std::thread t1(&Account::transfer, &A1, std::ref(A1), std::ref(A2), 500);
    std::thread t2(&Account::transfer, &A2, std::ref(A2), std::ref(A1), 600);
    
    t1.join();
    t2.join();
    return 0;
}
