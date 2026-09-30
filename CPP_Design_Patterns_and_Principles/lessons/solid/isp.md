# Interface Segregation Principle (ISP)

## 1. Definition

The Interface Segregation Principle says that clients should not be forced to
depend on operations they do not need. Shape interfaces around coherent client
capabilities rather than requiring every implementation to support one oversized API.

## 2. The Problem It Solves

A broad interface can force small implementations to provide meaningless methods
and force clients to learn or recompile against unrelated features. “Unsupported”
stubs are often evidence that the abstraction groups incompatible capabilities.

The goal is truthful, focused dependencies. A client asking only to read should
not automatically require a full read/write/delete administration interface.

## 3. Understand the Principle

Identify what each client actually needs to accomplish its task. Group operations
that form a coherent capability, then let concrete objects implement the capabilities
they genuinely support. A more capable object can satisfy several narrow interfaces.

ISP is not a rule to create one interface per method. Operations such as begin,
commit, and rollback may belong together because clients need their combined
transaction contract. Splitting them without regard for meaning can weaken the API.

Narrow interfaces also make tests smaller and LSP easier to satisfy, but they do
not prove semantic correctness. An implementation still must honor each capability's
promises.

## 4. Real-World Scenario

A document platform has preview, edit, and administrative-delete clients. The preview
component needs only a read capability. Editors need update operations, while an
administration service needs deletion authority.

Separate interfaces prevent ordinary preview code from depending on administrative
functions. This reduces accidental coupling, but interface separation is not by
itself security enforcement; permission checks are still needed at trust boundaries.

## 5. Understand the C++ Example

Open [isp.cpp](../../solid/isp.cpp).

The before `Machine` interface combines print and scan. A basic printer is forced
to implement scanning by throwing an unsupported-operation exception.

1. The test calls that unsupported operation and verifies the problem is observable.
2. The after design separates `Printer` from `Scanner`.
3. `BasicPrinter` implements only `Printer`, making no scanning promise.
4. `OfficeMachine` implements both capabilities.
5. `print_job()` accepts only `Printer`, so either device can satisfy its dependency.
6. A separate check confirms scanning works on the office machine.

The multiple inheritance combines abstract capabilities, not shared implementation
state. The improvement is that clients and implementers can depend on truthful
roles rather than negotiate a fat interface full of exceptions.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** focused dependencies, smaller fakes, fewer unsupported operations,
and reduced impact from unrelated interface changes.

**Drawbacks:** too many fragments create wiring and discovery overhead. Repeated
casts to discover capabilities can make the design harder to follow.

Use coherent client-oriented interfaces, templates, or capability concepts as
appropriate. C++ concepts express syntactic requirements but do not replace
behavioral contracts.

## 7. Check Your Understanding

**Question:** Does an office machine implementing two interfaces violate ISP?

**Answer:** No. ISP limits what a client is forced to depend on. A concrete device
can truthfully provide many capabilities while clients consume only what they need.

See the [SOLID technical notes](../../solid/README.md).