#include "support/check.hpp"

#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

struct Node {
    virtual ~Node() = default;
    virtual int size() const = 0;
};

class File final : public Node {
public:
    explicit File(int bytes) : bytes_(bytes) {
        if (bytes < 0) { throw std::invalid_argument("Negative size"); }
    }
    int size() const override { return bytes_; }

private:
    int bytes_;
};

class Directory final : public Node {
public:
    void add(std::unique_ptr<Node> child) {
        if (!child) { throw std::invalid_argument("Null child"); }
        children_.push_back(std::move(child));
    }

    int size() const override {
        int total = 0;
        for (const auto& child : children_) {
            const int child_size = child->size();
            // Each file can fit in int while the combined total cannot. Check
            // BEFORE adding: overflowing a signed int is undefined behavior.
            if (child_size < 0 || child_size > std::numeric_limits<int>::max() - total) {
                throw std::overflow_error("Directory total does not fit in int");
            }
            total += child_size;
        }
        return total;
    }

private:
    std::vector<std::unique_ptr<Node>> children_;
};

void demonstrate_drawback() {
    Directory large;
    large.add(std::make_unique<File>(std::numeric_limits<int>::max()));
    large.add(std::make_unique<File>(1));
    bool rejected = false;
    try { static_cast<void>(large.size()); }
    catch (const std::overflow_error&) { rejected = true; }
    check(rejected, "Valid individual sizes can produce an unrepresentable total");

    // Drawback: the tree abstraction does not solve numeric limits or recursion
    // limits. We show the numeric limit safely; deliberately exhausting the
    // call stack with a very deep tree would crash, not teach a portable result.
    std::cout << "Drawback: two valid files exceed int's total-size limit; "
                 "the explicit overflow guard rejects the sum.\n";
}

int main() {
    Directory root;
    check(root.size() == 0, "Empty composite has no bytes");
    auto nested = std::make_unique<Directory>();
    nested->add(std::make_unique<File>(20));
    root.add(std::make_unique<File>(10));
    root.add(std::move(nested));
    check(root.size() == 30, "Composite recursively sums its children");
    bool rejected = false;
    try { root.add(nullptr); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected && root.size() == 30, "Null insertion must leave the tree unchanged");
    std::cout << "Total bytes: " << root.size() << '\n';
    demonstrate_drawback();
}