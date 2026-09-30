#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <thread>

int main()
{
    std::mutex mutex;
    std::condition_variable_any changed;
    std::atomic<int> callback_count{0};
    bool stop_observed = false;
    std::jthread worker([&](std::stop_token token)
    {
        std::unique_lock<std::mutex> lock(mutex);
        const bool work_ready = changed.wait(lock, token, [] { return false; });
        stop_observed = !work_ready && token.stop_requested();
    });
    std::stop_callback on_stop(worker.get_stop_token(), [&]() noexcept
    {
        callback_count.fetch_add(1, std::memory_order_relaxed);
    });
    const bool first_request = worker.request_stop();
    const bool second_request = worker.request_stop();
    worker.join();
    const int callbacks = callback_count.load(std::memory_order_relaxed);
    std::cout << "Worker observed stop: " << std::boolalpha << stop_observed << '\n';
    std::cout << "Stop callbacks: " << callbacks << '\n';
    std::cout << "First / second request: " << first_request << " / " << second_request << '\n';
    return stop_observed && callbacks == 1 && first_request && !second_request ? 0 : 1;
}