#include <future>
#include <iostream>
#include <mutex>
#include <shared_mutex>

struct Snapshot
{
    int revision;
    int total;
};

class Ledger
{
    mutable std::shared_mutex mutex_;
    Snapshot state_{0, 0};

public:
    void update(int revision)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        state_ = {revision, revision * 10};
    }

    Snapshot read() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return state_;
    }
};

int main()
{
    Ledger ledger;
    auto writer = std::async(std::launch::async, [&]
    {
        for (int revision = 1; revision <= 1000; ++revision)
        {
            ledger.update(revision);
        }
    });
    auto inspect = [&]
    {
        for (int iteration = 0; iteration < 1000; ++iteration)
        {
            const Snapshot snapshot = ledger.read();
            if (snapshot.total != snapshot.revision * 10)
            {
                return false;
            }
        }
        return true;
    };
    auto first_reader = std::async(std::launch::async, inspect);
    auto second_reader = std::async(std::launch::async, inspect);
    writer.get();
    const bool first_valid = first_reader.get();
    const bool second_valid = second_reader.get();
    const Snapshot final = ledger.read();
    std::cout << "Consistent snapshots: " << std::boolalpha << (first_valid && second_valid) << '\n';
    std::cout << "Final revision / total: " << final.revision << " / " << final.total << '\n';
    return first_valid && second_valid && final.revision == 1000 && final.total == 10000 ? 0 : 1;
}