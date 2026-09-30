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

class GuardedMirror final : public Mediator {
public:
    GuardedMirror() : source_(*this), mirror_(*this) {}
    GuardedMirror(const GuardedMirror&) = delete;
    GuardedMirror& operator=(const GuardedMirror&) = delete;
    void edit() { source_.set("changed"); }
    int notifications() const { return notifications_; }
    void changed() override {
        ++notifications_;
        if (updating_) { return; }
        updating_ = true;
        struct ResetFlag {
            bool& flag;
            ~ResetFlag() { flag = false; }
        } reset{updating_};
        // Updating another field calls changed() AGAIN. Without the guard above,
        // this version would keep setting the mirror and calling itself forever.
        mirror_.set("updated by coordinator");
    }

private:
    TextField source_;
    TextField mirror_;
    bool updating_ = false;
    int notifications_ = 0;
};

void demonstrate_drawback() {
    GuardedMirror form;
    form.edit();
    check(form.notifications() == 2, "One edit triggers both original and nested notifications");
    form.edit();
    check(form.notifications() == 4, "Guard resets so later edits still work");
    // Drawback: centralizing communication does not prevent feedback loops.
    // The guard is deliberately included; we never run the infinite-loop version.
    std::cout << "Drawback: one edit causes two notifications; "
                 "a re-entry guard prevents the coordinator from looping.\n";
}

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
    demonstrate_drawback();
}