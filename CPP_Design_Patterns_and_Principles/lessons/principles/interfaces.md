# Program to Interfaces, Not Implementations

## 1. Definition

Depend on the behavior a collaborator promises rather than its concrete storage,
library, or implementation details. An interface is that behavioral contract; it
does not have to be a C++ class containing virtual functions.

## 2. The Problem It Solves

A client written directly around one implementation's details is difficult to reuse
or test with another. Changing storage or output technology then requires changing
the client even when its real need has not changed.

The client should express the capability it requires, while implementation-specific
choices belong at construction or integration boundaries.

## 3. Understand the Principle

A useful contract includes operations and semantics: valid input, units, ownership,
failure behavior, and effects. Matching method names is not enough. Two writers
may both accept text while disagreeing about whether they retain borrowed memory
or report write failure.

Runtime interfaces support dynamic selection. Templates express compile-time
protocols. Type-erased callables provide another option. Choose the mechanism based
on when implementations vary and what ownership is needed.

Do not mirror an entire vendor API merely to call it an interface. A client-oriented
contract should be as specific as its real responsibility requires.

## 4. Real-World Scenario

A report generator needs to write text. It should not require a disk file merely
because the first application used one. A text-writing contract lets the same
report go to memory for testing or to a stream in production.

The contract must define whether writes are synchronous and how errors surface.
An asynchronous implementation retaining a temporary string view cannot silently
substitute for a synchronous consumer without changing lifetime requirements.

## 5. Understand the C++ Example

Open [interfaces.cpp](../../principles/interfaces.cpp).

`emit_report()` is a template requiring a sink with `write(string_view)`.
`StringSink` appends to memory; `StreamSink` borrows an output stream.

1. `main()` creates a string sink and emits `status: ready\n` into it.
2. It creates an `ostringstream` and a stream sink wrapping that stream.
3. The same report template emits the same content through the second sink.
4. Checks compare both outputs and the expected text.

There is no common base class. The template is compiled for each sink type.
The sinks consume the view during the call rather than retaining it. Production
stream-error handling needs an explicit policy beyond this successful in-memory test.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** reusable clients, substitutable collaborators, focused tests, and
less concrete implementation knowledge.

**Drawbacks:** poorly chosen contracts leak details anyway. Templates can increase
compile times and diagnostics complexity; virtual interfaces add runtime indirection.

Use concrete values where no meaningful variation exists. Select templates, virtual
interfaces, or callable contracts according to actual use, not a universal rule.

## 7. Check Your Understanding

**Question:** Does this principle require adding `virtual` to every method?

**Answer:** No. The example follows a compile-time behavioral protocol. What matters
is the dependency on a suitable contract, not a particular dispatch mechanism.

See the [principles technical notes](../../principles/README.md).