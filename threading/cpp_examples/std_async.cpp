#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

struct Account
{
    std::string name;
    int balance;
};

struct TransferResult
{
    int from_balance;
    int to_balance;
};

TransferResult calculate_transfer(Account from, Account to, int amount)
{
    if (amount <= 0)
    {
        throw std::invalid_argument("Transfer amount must be positive");
    }
    if (from.balance < amount)
    {
        throw std::runtime_error("Insufficient balance in " + from.name + "'s account");
    }

    return {from.balance - amount, to.balance + amount};
}

bool run_transfer(Account& from, Account& to, int amount)
{
    std::cout << "\nRequest: " << from.name << " -> " << to.name
              << ", amount = " << amount << '\n';

    std::future<TransferResult> result_future =
        std::async(std::launch::async, calculate_transfer, from, to, amount);

    bool applied = false;
    try
    {
        TransferResult result = result_future.get();
        from.balance = result.from_balance;
        to.balance = result.to_balance;
        applied = true;
        std::cout << "Transfer applied by main.\n";
    }
    catch (const std::exception& error)
    {
        std::cout << "Transfer failed: " << error.what() << '\n';
    }

    if (result_future.valid())
    {
        throw std::logic_error("get() must consume the future");
    }

    std::cout << "Future valid after get(): " << result_future.valid() << '\n';
    std::cout << from.name << ": " << from.balance << ", "
              << to.name << ": " << to.balance << '\n';
    return applied;
}

bool demonstrate_launch_policies()
{
    const std::thread::id caller_id = std::this_thread::get_id();
    auto async_result = std::async(std::launch::async, []
    {
        return std::this_thread::get_id();
    });
    const bool separate_thread = async_result.get() != caller_id;

    bool deferred_started = false;
    auto deferred_result = std::async(std::launch::deferred, [&deferred_started]
    {
        deferred_started = true;
        return std::this_thread::get_id();
    });

    const auto status = deferred_result.wait_for(std::chrono::milliseconds(0));
    const bool remained_deferred =
        status == std::future_status::deferred && !deferred_started;
    const bool same_thread = deferred_result.get() == caller_id;

    std::cout << "\nExplicit async used a separate thread: " << separate_thread << '\n';
    std::cout << "Timed wait left deferred task unstarted: " << remained_deferred << '\n';
    std::cout << "Deferred get() ran on the calling thread: " << same_thread << '\n';

    return separate_thread && remained_deferred && deferred_started && same_thread;
}

bool demonstrate_wait_and_get()
{
    auto answer_future = std::async(std::launch::async, [] { return 42; });
    answer_future.wait();
    const bool valid_after_wait = answer_future.valid();
    const int answer = answer_future.get();

    std::cout << "\nFuture valid after wait(): " << valid_after_wait << '\n';
    std::cout << "Answer from get(): " << answer << '\n';
    std::cout << "Future valid after get(): " << answer_future.valid() << '\n';

    return valid_after_wait && answer == 42 && !answer_future.valid();
}

int main()
{
    std::cout << std::boolalpha;
    try
    {
        Account pavan{"Pavan", 50000};
        Account sagar{"Sagar", 60000};

        const bool success = run_transfer(pavan, sagar, 500);
        const bool insufficient = run_transfer(pavan, sagar, 100000);
        const bool invalid = run_transfer(pavan, sagar, 0);
        const bool policies_ok = demonstrate_launch_policies();
        const bool waiting_ok = demonstrate_wait_and_get();
        const bool checks_passed = success && !insufficient && !invalid
            && pavan.balance == 49500 && sagar.balance == 60500
            && policies_ok && waiting_ok;

        std::cout << "\nAll checks passed: " << checks_passed << '\n';
        return checks_passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Example failed: " << error.what() << '\n';
        return 1;
    }
}