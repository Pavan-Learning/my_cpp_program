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

void demonstrate_drawback() {
    // weak_ptr lets us observe lifetime without keeping the style alive itself.
    std::weak_ptr<const GlyphStyle> observed;
    {
        StyleFactory cache;
        {
            const auto temporary_user = cache.get("RareFont", 72);
            observed = temporary_user;
        }
        // Drawback: no glyph needs this style now, but the factory's map owns it.
        // Many one-off styles would stay in memory until the factory is destroyed.
        check(!observed.expired() && observed.use_count() == 1,
              "Cache retains a style after its last external user leaves");
        std::cout << "Drawback: unused style remains alive in the cache.\n";
    }
    check(observed.expired(), "Destroying the cache finally releases the unused style");
    std::cout << "After cache destruction: unused style released.\n";
    // Eviction or weak ownership can reduce retention, but need extra lookup and
    // lifetime rules. Sharing is most useful when styles are actually reused.
}

int main() {
    StyleFactory factory;
    const Glyph first{'A', 0, factory.get("Mono", 12)};
    const Glyph second{'B', 1, factory.get("Mono", 12)};
    const Glyph heading{'A', 2, factory.get("Mono", 20)};
    check(first.style == second.style, "Equal intrinsic state must be shared");
    check(first.style != heading.style, "Different styles need different flyweights");
    check(first.position != second.position, "Extrinsic state stays per glyph");
    std::cout << first.character << second.character << " share " << first.style->font << '\n';
    demonstrate_drawback();
}