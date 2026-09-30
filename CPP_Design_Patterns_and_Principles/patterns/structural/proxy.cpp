#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

struct Image {
    virtual ~Image() = default;
    virtual std::string display() = 0;
};

class RealImage final : public Image {
public:
    std::string display() override { return "display image"; }
};

class LazyImageProxy final : public Image {
public:
    explicit LazyImageProxy(bool source_available = true) : source_available_(source_available) {}
    std::string display() override {
        if (!image_) {
            // A deterministic stand-in for a missing image, without real file I/O.
            if (!source_available_) { throw std::runtime_error("Image source unavailable"); }
            image_ = std::make_unique<RealImage>();
            ++load_count_;
        }
        return image_->display();
    }
    int load_count() const { return load_count_; }

private:
    bool source_available_;
    std::unique_ptr<RealImage> image_;
    int load_count_ = 0;
};

void demonstrate_drawback() {
    LazyImageProxy missing_image(false);
    check(missing_image.load_count() == 0, "Creating a proxy does not verify its source");
    bool failed_on_use = false;
    try { static_cast<void>(missing_image.display()); }
    catch (const std::runtime_error&) { failed_on_use = true; }
    check(failed_on_use && missing_image.load_count() == 0, "First use can fail before any successful load");

    // Drawback: lazy loading moves the work AND possible errors to display().
    // A caller must handle first-use failure even though construction succeeded.
    // Real loading can also cause delay; we do not fake a delay with a sleep.
    std::cout << "Drawback: proxy construction succeeded, but first display failed "
                 "because the image source was unavailable.\n";
}

int main() {
    LazyImageProxy proxy;
    check(proxy.load_count() == 0, "Construction must remain lazy");
    check(proxy.display() == "display image", "Proxy preserves the Image operation");
    proxy.display();
    check(proxy.load_count() == 1, "Repeated access reuses the real subject");
    std::cout << "Image loaded " << proxy.load_count() << " time\n";
    demonstrate_drawback();
}