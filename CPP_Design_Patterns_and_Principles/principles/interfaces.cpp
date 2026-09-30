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

void demonstrate_drawback() {
    std::ostringstream unavailable;
    unavailable.setstate(std::ios::badbit);
    StreamSink sink(unavailable);
    emit_report(sink);
    check(unavailable.fail() && unavailable.str().empty(), "write returns normally although the failed stream stores nothing");

    StringSink working;
    emit_report(working);
    check(working.text() == "status: ready\n", "The same interface works with the memory sink");
    // Drawback: the compiler checks that write() exists, not that output succeeds.
    // StreamSink currently hides the stream's error state. A production contract
    // must specify failures: inspect state, return a result, or enable exceptions.
    // Neither a template nor a virtual interface supplies that promise by itself.
    std::cout << "Drawback: both sinks have write(), but the failed stream "
                 "silently stores no report unless its state is checked.\n";
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
    demonstrate_drawback();
}