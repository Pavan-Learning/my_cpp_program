#include "support/check.hpp"

#include <iostream>
#include <string>
#include <utility>

struct Mediator {
    virtual ~Mediator() = default;
    virtual void changed() = 0;
};

class TextField {
public:
    explicit TextField(Mediator& mediator) : mediator_(mediator) {}
    void set(std::string text) {
        text_ = std::move(text);
        mediator_.changed();
    }
    bool empty() const { return text_.empty(); }

private:
    Mediator& mediator_;
    std::string text_;
};

class SignInForm final : public Mediator {
public:
    SignInForm() : username_(*this), password_(*this) {}
    SignInForm(const SignInForm&) = delete;
    SignInForm& operator=(const SignInForm&) = delete;
    void username(std::string value) { username_.set(std::move(value)); }
    void password(std::string value) { password_.set(std::move(value)); }
    bool submit_enabled() const { return submit_enabled_; }
    void changed() override { submit_enabled_ = !username_.empty() && !password_.empty(); }

private:
    TextField username_;
    TextField password_;
    bool submit_enabled_ = false;
};

int main() {
    SignInForm form;
    check(!form.submit_enabled(), "Empty form disabled");
    form.username("student");
    check(!form.submit_enabled(), "One field is insufficient");
    form.password("demo-only");
    check(form.submit_enabled(), "Mediator combines colleague state");
    form.username("");
    check(!form.submit_enabled(), "Clearing a field recomputes state");
    std::cout << "Form coordination verified\n";
}