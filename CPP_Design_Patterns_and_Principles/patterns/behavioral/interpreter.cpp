#include "support/check.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using Context = std::map<std::string, bool>;

struct Expression {
    virtual ~Expression() = default;
    virtual bool evaluate(const Context& context) const = 0;
};

class Variable final : public Expression {
public:
    explicit Variable(std::string name) : name_(std::move(name)) {}
    bool evaluate(const Context& context) const override { return context.at(name_); }

private:
    std::string name_;
};

class And final : public Expression {
public:
    And(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : left_(std::move(left)), right_(std::move(right)) {
        if (!left_ || !right_) { throw std::invalid_argument("Two expressions required"); }
    }
    bool evaluate(const Context& context) const override {
        return left_->evaluate(context) && right_->evaluate(context);
    }

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

class CountedVariable final : public Expression {
public:
    CountedVariable(std::string name, int& evaluations) : name_(std::move(name)), evaluations_(evaluations) {}
    bool evaluate(const Context& context) const override {
        ++evaluations_;
        return context.at(name_);
    }

private:
    std::string name_;
    int& evaluations_;
};

void demonstrate_drawback() {
    const Context context{{"signed_in", true}, {"paid", true}};
    int evaluations = 0;
    const And rule(std::make_unique<CountedVariable>("signed_in", evaluations),
                   std::make_unique<CountedVariable>("paid", evaluations));
    check(rule.evaluate(context) && rule.evaluate(context), "Both runs produce the same answer");
    check(evaluations == 4, "Two runs repeat both variable lookups");

    // Drawback: each run walks the objects again; there is no compiled or cached
    // answer here. Also, constructing Variable does NOT parse user-entered text:
    // this entire string becomes one variable name, not an AND expression.
    bool text_rejected = false;
    try { static_cast<void>(Variable("signed_in AND paid").evaluate(context)); }
    catch (const std::out_of_range&) { text_rejected = true; }
    check(text_rejected, "Reading a rule from text requires a separate parser");
    std::cout << "Drawback: two evaluations perform four lookups; "
                 "rule text still needs a separate parser.\n";
}

int main() {
    const And access(std::make_unique<Variable>("signed_in"), std::make_unique<Variable>("paid"));
    check(access.evaluate({{"signed_in", true}, {"paid", true}}), "Both terminals true");
    check(!access.evaluate({{"signed_in", true}, {"paid", false}}), "One terminal false");
    check(!access.evaluate({{"signed_in", false}}), "AND short-circuits missing right variable");
    bool rejected = false;
    try { access.evaluate({{"signed_in", true}}); }
    catch (const std::out_of_range&) { rejected = true; }
    check(rejected, "Evaluated missing variable is an error");
    std::cout << "signed_in AND paid verified\n";
    demonstrate_drawback();
}