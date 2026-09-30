#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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

void demonstrate_drawback() {
    Editor editor;
    std::vector<Editor::Snapshot> history;
    std::size_t copied_characters = 0;
    for (int revision = 0; revision < 3; ++revision) {
        editor.type(std::string(1024, 'x'));
        copied_characters += editor.text().size();
        history.push_back(editor.save());
    }
    check(editor.text().size() == 3072 && copied_characters == 6144,
          "Full snapshots retain the sum of all saved text lengths");

    // Drawback: snapshots copy the WHOLE text, not only the latest addition.
    // 1024 + 2048 + 3072 = 6144 retained characters for a 3072-character document.
    // This counts text payload only, not string capacities or allocator overhead.
    // Old text also remains recoverable until the snapshots themselves are removed.
    editor.restore(history.front());
    check(editor.text().size() == 1024, "Old content remains in the saved snapshot");
    editor.restore(history.back());
    check(editor.text().size() == 3072, "Restoring old state does not erase newer snapshots");
    std::cout << "Drawback: three snapshots copy " << copied_characters
              << " characters, and old revisions remain recoverable.\n";
}

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
    demonstrate_drawback();
}