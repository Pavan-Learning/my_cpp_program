#include "support/check.hpp"

#include <iostream>
#include <memory>
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
    std::string display() override {
        if (!image_) {
            image_ = std::make_unique<RealImage>();
            ++load_count_;
        }
        return image_->display();
    }
    int load_count() const { return load_count_; }

private:
    std::unique_ptr<RealImage> image_;
    int load_count_ = 0;
};

int main() {
    LazyImageProxy proxy;
    check(proxy.load_count() == 0, "Construction must remain lazy");
    check(proxy.display() == "display image", "Proxy preserves the Image operation");
    proxy.display();
    check(proxy.load_count() == 1, "Repeated access reuses the real subject");
    std::cout << "Image loaded " << proxy.load_count() << " time\n";
}