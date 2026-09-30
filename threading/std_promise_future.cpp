/*
 * std::promise AND std::future: A ONE-TIME RESULT BETWEEN THREADS
 *
 * promise = producer endpoint: set_value() or set_exception().
 * future  = consumer endpoint: wait(), wait_for(), or get().
 * Both endpoints refer to the same shared state.
 * Neither promise nor future starts a thread; std::thread does that here.
 *
 * Unlike std_lock.cpp, the worker uses account copies. Only main updates
 * the original accounts after future.get() returns the complete result.
 * This teaches result delivery, not a concurrent banking implementation.
 *
 * Build from the workspace root:
 * g++ -std=c++17 -Wall -Wextra -pthread threading/std_promise_future.cpp -o out/std_promise_future
 * Run: ./out/std_promise_future
 */

#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

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

void calculate_transfer(Account from, Account to, int amount,
                        std::promise<TransferResult> result_promise)
{
    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));

        if (amount <= 0)
        {
            throw std::invalid_argument("Transfer amount must be positive");
        }
        if (from.balance < amount)
        {
            throw std::runtime_error("Insufficient balance in " + from.name + "'s account");
        }

        result_promise.set_value({from.balance - amount, to.balance + amount});
    }
    catch (...)
    {
        result_promise.set_exception(std::current_exception());
    }
}

void run_transfer(Account& from, Account& to, int amount)
{
    std::cout << "\nRequest: " << from.name << " -> " << to.name
              << ", amount = " << amount << '\n';

    std::promise<TransferResult> result_promise;
    std::future<TransferResult> result_future = result_promise.get_future();

    // Move the producer endpoint; main keeps the consumer endpoint.
    std::thread worker(calculate_transfer, from, to, amount, std::move(result_promise));

    if (result_future.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
    {
        std::cout << "Not ready yet; main can do other work before get().\n";
    }

    try
    {
        // get() waits if necessary, then returns the value or rethrows the exception.
        TransferResult result = result_future.get();
        from.balance = result.from_balance;
        to.balance = result.to_balance;
        std::cout << "Transfer applied by main.\n";
    }
    catch (const std::exception& error)
    {
        std::cout << "Transfer failed: " << error.what() << '\n';
    }

    // Result readiness is not a substitute for joining the thread.
    worker.join();
    std::cout << "Future valid after get(): " << std::boolalpha << result_future.valid() << '\n';
    std::cout << from.name << ": " << from.balance << ", "
              << to.name << ": " << to.balance << '\n';
}

void demonstrate_broken_promise()
{
    std::future<int> abandoned_future;
    {
        std::promise<int> abandoned_promise;
        abandoned_future = abandoned_promise.get_future();
    }

    try
    {
        abandoned_future.get();
    }
    catch (const std::future_error& error)
    {
        if (error.code() != std::make_error_code(std::future_errc::broken_promise))
        {
            throw;
        }
        std::cout << "\nBroken promise: producer was destroyed without providing a result.\n";
    }
}

int main()
{
    Account pavan{"Pavan", 50000};
    Account sagar{"Sagar", 60000};

    run_transfer(pavan, sagar, 500);
    run_transfer(pavan, sagar, 100000);
    demonstrate_broken_promise();

    return pavan.balance == 49500 && sagar.balance == 60500 ? 0 : 1;
}