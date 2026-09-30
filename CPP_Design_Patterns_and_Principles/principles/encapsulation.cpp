#include "support/check.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>

class Account {
public:
    void deposit(int cents) {
        if (cents <= 0 || cents > std::numeric_limits<int>::max() - balance_) {
            throw std::invalid_argument("Deposit outside supported range");
        }
        balance_ += cents;
    }
    bool withdraw(int cents) {
        if (cents <= 0) { throw std::invalid_argument("Positive withdrawal required"); }
        if (cents > balance_) { return false; }
        balance_ -= cents;
        return true;
    }
    int balance() const { return balance_; }

private:
    int balance_ = 0;
};

int main() {
    Account account;
    account.deposit(500);
    check(account.withdraw(200) && account.balance() == 300, "Valid domain operation");
    check(!account.withdraw(400) && account.balance() == 300, "Invariant survives insufficient funds");
    bool rejected = false;
    try { account.deposit(std::numeric_limits<int>::max()); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected && account.balance() == 300, "Overflow rejected before mutation");
    std::cout << "Balance: " << account.balance() << '\n';
}