#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

class Editor {
public:
    class Snapshot {
        friend class Editor;
        Snapshot(const Editor* owner, std::string text) : owner_(owner), text_(std::move(text)) {}
        const Editor* owner_;
        std::string text_;
    };

    Editor() = default;
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
    void type(std::string text) { text_ += text; }
    const std::string& text() const { return text_; }
    Snapshot save() const { return Snapshot(this, text_); }
    void restore(const Snapshot& snapshot) {
        if (snapshot.owner_ != this) { throw std::invalid_argument("Foreign snapshot"); }
        text_ = snapshot.text_;
    }

private:
    std::string text_;
};

int main() {
    Editor editor;
    editor.type("draft");
    const auto checkpoint = editor.save();
    editor.type(" with changes");
    check(editor.text() == "draft with changes", "Edit after saving");
    editor.restore(checkpoint);
    check(editor.text() == "draft", "Restore private state");
    editor.restore(checkpoint);
    check(editor.text() == "draft", "Snapshot is reusable");
    Editor other;
    bool rejected = false;
    try { other.restore(checkpoint); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Snapshot belongs to its originator");
    std::cout << editor.text() << '\n';
}