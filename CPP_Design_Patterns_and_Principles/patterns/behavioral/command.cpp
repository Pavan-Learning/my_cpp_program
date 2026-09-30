#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Document {
public:
    void append(const std::string& text) { text_ += text; }
    void truncate(std::size_t size) { text_.resize(size); }
    const std::string& text() const { return text_; }

private:
    std::string text_;
};

struct Command {
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class Append final : public Command {
public:
    Append(Document& document, std::string text) : document_(document), text_(std::move(text)) {}
    void execute() override {
        previous_size_ = document_.text().size();
        document_.append(text_);
    }
    void undo() override { document_.truncate(previous_size_); }

private:
    Document& document_;
    std::string text_;
    std::size_t previous_size_ = 0;
};

class History {
public:
    void run(std::unique_ptr<Command> command) {
        if (!command) { throw std::invalid_argument("Missing command"); }
        done_.reserve(done_.size() + 1);
        command->execute();
        done_.push_back(std::move(command));
        undone_.clear();
    }
    bool undo() {
        if (done_.empty()) { return false; }
        undone_.reserve(undone_.size() + 1);
        done_.back()->undo();
        undone_.push_back(std::move(done_.back()));
        done_.pop_back();
        return true;
    }
    bool redo() {
        if (undone_.empty()) { return false; }
        done_.reserve(done_.size() + 1);
        undone_.back()->execute();
        done_.push_back(std::move(undone_.back()));
        undone_.pop_back();
        return true;
    }

private:
    std::vector<std::unique_ptr<Command>> done_;
    std::vector<std::unique_ptr<Command>> undone_;
};

int main() {
    Document document;
    History history;
    check(!history.undo() && !history.redo(), "Empty history is harmless");
    history.run(std::make_unique<Append>(document, "hello"));
    history.run(std::make_unique<Append>(document, " world"));
    check(document.text() == "hello world", "Commands invoke receiver");
    check(history.undo() && document.text() == "hello", "Undo restores previous length");
    check(history.redo() && document.text() == "hello world", "Redo replays command");
    history.undo();
    history.run(std::make_unique<Append>(document, " C++"));
    check(!history.redo(), "New edit invalidates redo branch");
    std::cout << document.text() << '\n';
}