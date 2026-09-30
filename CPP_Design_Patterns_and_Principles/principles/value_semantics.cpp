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

std::size_t count_by_value(Notebook notebook) { return notebook.size(); }
std::size_t count_by_reference(const Notebook& notebook) { return notebook.size(); }

void demonstrate_drawback() {
    Notebook original;
    for (int note = 0; note < 3; ++note) { original.add(std::string(1024, 'x')); }
    const auto independent = original;
    std::size_t copied_characters = 0;
    for (std::size_t index = 0; index < independent.size(); ++index) {
        copied_characters += independent.at(index).size();
    }
    check(copied_characters == 3072, "Copy duplicates the three notes' text payload");
    check(independent.at(0).data() != original.at(0).data(), "Copied large string has independent storage");
    check(count_by_value(original) == 3 && count_by_reference(original) == 3, "Reading the size does not require an independent copy");

    // Drawback: Rule of Zero makes copying correct, not free. Passing this lvalue
    // by value constructs another Notebook with copies of its strings. Use a const
    // reference for a read-only count; use a value when independence is needed.
    // The 3072 count is copied text length, not allocator overhead or a timing test.
    std::cout << "Drawback: an independent notebook copy duplicates 3072 characters; "
                 "a read-only reference can count notes without copying the notebook.\n";
}

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
    demonstrate_drawback();
}