#include "support/check.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

class Playlist {
public:
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::string*;
        using reference = const std::string&;

        Iterator() = default;
        explicit Iterator(std::vector<std::string>::const_iterator position) : position_(position) {}
        reference operator*() const { return *position_; }
        pointer operator->() const { return &*position_; }
        Iterator& operator++() { ++position_; return *this; }
        Iterator operator++(int) { auto previous = *this; ++(*this); return previous; }
        bool operator==(const Iterator& other) const { return position_ == other.position_; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }

    private:
        std::vector<std::string>::const_iterator position_{};
    };

    void add(std::string title) { tracks_.push_back(std::move(title)); }
    Iterator begin() const { return Iterator(tracks_.begin()); }
    Iterator end() const { return Iterator(tracks_.end()); }

private:
    std::vector<std::string> tracks_;
};

void demonstrate_drawback() {
    Playlist playlist;
    playlist.add("Intro");
    auto stop = playlist.end();
    check(std::distance(playlist.begin(), stop) == 1, "Saved end initially describes one item");
    const std::string saved_title = *playlist.begin();
    playlist.add("Encore");

    // Drawback: vector insertion invalidates its old end iterator, even when
    // storage does not move. If storage grows, existing item iterators are also
    // invalidated. Comparing or dereferencing those positions is not safe.
    // Do not demonstrate a crash: replace the old end and get a fresh begin.
    stop = playlist.end();
    check(std::distance(playlist.begin(), stop) == 2, "Fresh iterators see the changed collection");
    check(saved_title == "Intro", "A copied string survives collection changes independently");
    std::cout << "Drawback: adding a track invalidates the saved end; "
                 "fresh iterators now see two tracks.\n";
}

int main() {
    Playlist playlist;
    check(playlist.begin() == playlist.end(), "Empty range");
    playlist.add("Intro");
    playlist.add("Finale");
    auto current = playlist.begin();
    const auto saved = current++;
    check(*saved == "Intro" && *current == "Finale", "Independent iterator copies");
    check(std::distance(playlist.begin(), playlist.end()) == 2, "Standard algorithm compatibility");
    check(std::find(playlist.begin(), playlist.end(), "Finale") != playlist.end(), "Search without storage access");
    for (const auto& track : playlist) { std::cout << track << '\n'; }
    demonstrate_drawback();
}