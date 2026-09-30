#include "support/check.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

class DocumentVersion {
public:
    explicit DocumentVersion(std::string text) : text_(std::move(text)) {}
    const std::string& text() const { return text_; }
    DocumentVersion append(const std::string& suffix) const {
        return DocumentVersion(text_ + suffix);
    }

private:
    std::string text_;
};

void demonstrate_drawback() {
    std::vector<DocumentVersion> versions;
    versions.emplace_back(std::string(1024, 'x'));
    versions.push_back(versions.back().append("A"));
    versions.push_back(versions.back().append("B"));
    std::size_t retained_characters = 0;
    for (const auto& version : versions) { retained_characters += version.text().size(); }
    check(versions.back().text().size() == 1026 && retained_characters == 3075,
          "Keeping three complete versions retains 1024 + 1025 + 1026 characters");
    check(versions.front().text() == std::string(1024, 'x'), "Old version stays unchanged");

    // Drawback: two one-character edits produce full new strings in THIS design.
    // Keeping the old versions retains 3075 payload characters for a latest text
    // of 1026 characters. This is not a measurement of total allocated bytes.
    // Structural sharing can help, but needs a more complicated storage design.
    std::cout << "Drawback: retaining three immutable versions stores " << retained_characters
              << " characters although the latest text has only 1026.\n";
}

int main() {
    const DocumentVersion original("draft");
    const auto revised = original.append(" reviewed");
    check(original.text() == "draft", "Original value remains unchanged");
    check(revised.text() == "draft reviewed", "Transformation creates a new version");
    check(original.append("").text() == original.text(), "Empty transformation preserves value");
    std::cout << original.text() << " -> " << revised.text() << '\n';
    demonstrate_drawback();
}