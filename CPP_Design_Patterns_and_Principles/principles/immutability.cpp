#include "support/check.hpp"

#include <iostream>
#include <string>
#include <utility>

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

int main() {
    const DocumentVersion original("draft");
    const auto revised = original.append(" reviewed");
    check(original.text() == "draft", "Original value remains unchanged");
    check(revised.text() == "draft reviewed", "Transformation creates a new version");
    check(original.append("").text() == original.text(), "Empty transformation preserves value");
    std::cout << original.text() << " -> " << revised.text() << '\n';
}