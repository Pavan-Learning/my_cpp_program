#include "support/check.hpp"

#include <iostream>
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
        for (const auto& child : children_) { total += child->size(); }
        return total;
    }

private:
    std::vector<std::unique_ptr<Node>> children_;
};

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
}