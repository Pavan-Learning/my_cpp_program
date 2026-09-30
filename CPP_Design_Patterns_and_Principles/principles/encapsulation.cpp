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

void demonstrate_drawback() {
    Account sender;
    Account receiver;
    sender.deposit(500);
    receiver.deposit(std::numeric_limits<int>::max());
    check(sender.withdraw(100), "First half of the proposed transfer succeeds");
    bool failed = false;
    try { receiver.deposit(100); }
    catch (const std::invalid_argument&) { failed = true; }
    check(failed && sender.balance() == 400 && receiver.balance() == std::numeric_limits<int>::max(),
          "Each account is valid, but the two-step transfer is incomplete");

    // Drawback: private fields plus small operations do not provide every useful
    // domain operation. A caller assembling a transfer must handle partial failure.
    // Do not bypass private data; provide a proper transfer/recovery policy instead.
    // This local refund is safe because the withdrawal freed exactly this capacity.
    sender.deposit(100);
    check(sender.balance() == 500, "Explicit recovery restores the sender in this in-memory example");
    std::cout << "Drawback: recipient rejected the transfer after sender lost 100; "
                 "an explicit refund was needed to restore the sender.\n";
}

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
    demonstrate_drawback();
}