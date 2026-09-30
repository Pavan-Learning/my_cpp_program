#include "support/check.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

struct GlyphStyle {
    std::string font;
    int size;
};

class StyleFactory {
public:
    std::shared_ptr<const GlyphStyle> get(const std::string& font, int size) {
        const auto key = std::make_pair(font, size);
        const auto found = styles_.find(key);
        if (found != styles_.end()) { return found->second; }
        auto style = std::make_shared<const GlyphStyle>(GlyphStyle{font, size});
        styles_.emplace(key, style);
        return style;
    }

private:
    std::map<std::pair<std::string, int>, std::shared_ptr<const GlyphStyle>> styles_;
};

struct Glyph {
    char character;
    int position;
    std::shared_ptr<const GlyphStyle> style;
};

int main() {
    StyleFactory factory;
    const Glyph first{'A', 0, factory.get("Mono", 12)};
    const Glyph second{'B', 1, factory.get("Mono", 12)};
    const Glyph heading{'A', 2, factory.get("Mono", 20)};
    check(first.style == second.style, "Equal intrinsic state must be shared");
    check(first.style != heading.style, "Different styles need different flyweights");
    check(first.position != second.position, "Extrinsic state stays per glyph");
    std::cout << first.character << second.character << " share " << first.style->font << '\n';
}