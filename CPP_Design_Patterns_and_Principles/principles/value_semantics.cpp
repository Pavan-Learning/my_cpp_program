#include "support/check.hpp"

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class Notebook {
public:
    void add(std::string note) { notes_.push_back(std::move(note)); }
    std::size_t size() const { return notes_.size(); }
    const std::string& at(std::size_t index) const { return notes_.at(index); }

private:
    std::vector<std::string> notes_;
};

static_assert(std::is_copy_constructible_v<Notebook>);
static_assert(std::is_nothrow_move_constructible_v<Notebook>);

int main() {
    Notebook original;
    original.add("first");
    auto copy = original;
    copy.add("second");
    check(original.size() == 1 && copy.size() == 2, "Value copies are independent");
    auto moved = std::move(copy);
    check(moved.size() == 2 && moved.at(1) == "second", "Default move preserves destination data");
    copy = Notebook{};
    copy.add("reused");
    check(copy.size() == 1, "Moved-from value can be assigned and reused");
    std::cout << "Independent copy and efficient move verified\n";
}