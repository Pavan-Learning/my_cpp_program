#include "support/check.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

class StringSink {
public:
    void write(std::string_view text) { text_.append(text); }
    const std::string& text() const { return text_; }

private:
    std::string text_;
};

class StreamSink {
public:
    explicit StreamSink(std::ostream& stream) : stream_(stream) {}
    void write(std::string_view text) { stream_ << text; }

private:
    std::ostream& stream_;
};

template<class Sink>
void emit_report(Sink& sink) {
    sink.write("status: ready\n");
}

int main() {
    StringSink memory;
    emit_report(memory);
    std::ostringstream output;
    StreamSink stream(output);
    emit_report(stream);
    check(memory.text() == "status: ready\n", "Compile-time behavioral interface");
    check(memory.text() == output.str(), "Two unrelated types fulfill the same protocol");
    std::cout << memory.text();
}