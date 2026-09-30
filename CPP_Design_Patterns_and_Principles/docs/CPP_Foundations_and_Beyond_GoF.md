# C++ Foundations and Patterns Beyond GoF

The GoF catalog is an object-oriented vocabulary, not a complete map of C++ or
software architecture. This chapter explains the language and system-level choices
needed to use those patterns responsibly. The examples in other chapters are
executable; snippets here are focused illustrations, not extra build targets.

## 1. Separate Principle, Pattern, Idiom, and Architecture

A **principle** guides a decision: keep dependencies explicit or protect invariants.
A **pattern** describes a recurring problem and a reusable arrangement, such as
Strategy. An **idiom** uses language-specific techniques, such as RAII or copy-and-swap.
An **architecture** organizes larger boundaries, deployment, data, and interaction.

A project can use a layered architecture, apply DIP at the domain boundary, use an
Adapter for a vendor library, and use RAII inside that adapter. Those are different
levels of the same design. Calling everything a “design pattern” makes it harder
to understand what a particular technique actually solves.

**Worked distinction:** `unique_ptr` does not by itself make a class a Factory.
It expresses ownership. A method that creates an object may be a named constructor,
a simple factory, or a GoF factory method depending on its role in the collaboration.

## 2. Decide Ownership Before Drawing Inheritance

For every relationship, answer four questions:

1. Who creates the object?
2. Who destroys it?
3. Can the object outlive its caller or owner?
4. Can someone mutate it through another alias?

| Representation | Meaning to start from | Important risk |
| --- | --- | --- |
| Value member | Contained object with enclosing lifetime | Copy cost or identity mismatch |
| `unique_ptr<T>` | Exclusive dynamic ownership | Use after moving or borrowing past destruction |
| `shared_ptr<T>` | Shared lifetime ownership | Cycles, unclear lifetime, mutable shared state |
| `weak_ptr<T>` | Non-owning observation of shared lifetime | Must lock and handle expiration |
| `T&` | Required borrowed object | Referent must outlive use |
| `T*` | Usually optional/reseatable borrow unless documented otherwise | Null and lifetime checks |
| `string_view` / iterator | Borrowed view into storage | Invalidation by mutation or destruction |

`shared_ptr` is not a universal safer pointer. Copying it extends lifetime, not
thread safety. Two objects strongly owning one another remain alive even after all
external references disappear. Prefer one clear owner and explicit borrowing where
that matches the real lifetime.

### Worked Borrowing Failure

```cpp
struct Reader {
    explicit Reader(const std::string& text) : text_(text) {}
    const std::string& text_;
};

Reader reader(std::string("temporary"));
```

The temporary survives the construction expression, then dies. The stored reference
does not extend its lifetime. This is the same hazard as retaining a Strategy or
Adapter reference to a temporary. Store a value when the object should own the data,
or require a longer-lived caller-owned dependency.

**Benefits of explicit ownership:** cleanup and validity can be reasoned about
locally. **Cost:** APIs sometimes need move operations, noncopyability, or explicit
lifetime restrictions. Those restrictions reveal real constraints rather than
adding arbitrary ceremony.

## 3. Dynamic vs Static Polymorphism

Runtime virtual dispatch selects an implementation through a base pointer or
reference. It fits runtime provider selection, heterogeneous collections, and
some plugin boundaries. A virtual base used for owning polymorphic deletion needs
a virtual destructor.

Templates select and check an implementation at compile time. They fit generic
algorithms and fixed policy composition, often allowing inlining. Their costs
include exposed implementation, longer diagnostics, compilation time, and possible
code duplication. Virtual calls have indirection costs, but allocation and cache
locality can matter much more than the dispatch instruction itself.

Type erasure offers a third choice. `std::function` holds different callable types
behind one runtime signature; it may allocate and adds erased-call overhead. A
custom erased interface can preserve value semantics while hiding concrete types,
but must implement copy, move, destruction, and small-object storage correctly.

`std::variant` represents a closed set of alternatives by value. It avoids a base
hierarchy and works naturally with `std::visit`, but adding an alternative can
require changing visitors throughout the program. Virtual interfaces naturally
support new implementations; variants make the closed set explicit.

**Worked decision:** Shipping mode selected from configuration may use virtual
Strategy. A compression policy fixed by a template library can use a template.
A one-operation callback from a UI may use `std::function`. Do not mechanically
convert every pattern into inheritance or every virtual call into a template.

## 4. Exception Guarantees and Transaction Boundaries

| Guarantee | Promise |
| --- | --- |
| No-throw | The operation does not throw |
| Strong | On failure, observable state remains unchanged |
| Basic | Invariants hold and resources are not leaked, but state may change |
| No guarantee | Even validity may be lost after failure |

RAII solves resource cleanup, not every state rollback. An object can release all
memory correctly while leaving a business transaction half completed.

### Worked Example: Record a Command

If an invoker modifies a document and only then allocates space to record the
command, allocation failure can leave an unrecorded edit. The
[Command example](../patterns/behavioral/command.cpp) reserves history capacity
before execution. It reduces one exception window. The concrete command must still
define whether its own partial effects can occur.

### Worked Example: Payment and Stock

“Charge payment, reserve stock” can fail after charging. “Reserve stock, charge
payment” can fail after reserving. Merely swapping the order is insufficient.
A local transaction may make local data changes atomic; an external payment needs
idempotency and compensation. A Facade can coordinate the workflow but does not
make the two systems one transaction.

**Benefits of explicit guarantees:** callers know whether retry or rollback is
safe. **Costs:** strong guarantees may need staging copies, extra allocations, or
transaction protocols. Some operations cannot provide exact rollback.

Destructors should normally be nonthrowing. When cleanup failure matters, expose
an explicit operation that reports it, while destruction performs a safe fallback.
Do not promise `noexcept` simply to improve performance if called operations can
legitimately throw; escaping it terminates the process.

## 5. Pimpl: Hide Implementation Details

Pointer to implementation keeps private representation out of a public header.
The header declares a nested `Impl` and stores `unique_ptr<Impl>`. The source file
defines `Impl` and the owning class's destructor where `Impl` is complete.

```cpp
class Engine {
public:
    Engine();
    ~Engine();
    Engine(Engine&&) noexcept;
    Engine& operator=(Engine&&) noexcept;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
```

This illustrative declaration needs `<memory>` and out-of-line definitions to
become a complete program. Defining destruction only where the implementation is
complete avoids incomplete-type deletion problems.

**Benefits:** reduced header dependencies and rebuild impact; implementation
changes can preserve class layout. **Drawbacks:** allocation/indirection, more
boilerplate, and explicit copy/move decisions. Pimpl alone does not guarantee ABI
compatibility across arbitrary compilers, standard libraries, or public API changes.

**When not to use it:** tiny value types in hot loops or internal classes with no
header/ABI boundary. Pimpl hides implementation; Bridge specifically separates
independently varying abstraction and implementation dimensions.

## 6. CRTP and Policy-Based Design

The Curiously Recurring Template Pattern passes a derived type to a base template:

```cpp
template<class Derived>
struct Printable {
    std::string print() const {
        return static_cast<const Derived&>(*this).render();
    }
};

struct Ticket : Printable<Ticket> {
    std::string render() const { return "ticket"; }
};
```

The snippet needs `<string>`. The base can call a derived implementation without
virtual dispatch. Correct use requires that the object really has the expected
derived type; CRTP is not a dynamically checked downcast mechanism.

Policy-based design composes template parameters such as storage, synchronization,
or allocation policy. It resembles Strategy selected at compilation rather than
runtime.

**Benefits:** compile-time customization and potential inlining. **Drawbacks:**
template coupling, diagnostics, code size, and limited runtime interchangeability.
Do not use CRTP merely to avoid a virtual call that has never been measured as a
problem. In C++20 and later, concepts can clarify requirements but do not remove
the need for sensible policy boundaries.

## 7. Nonvirtual Interface and Two-Phase Work

The nonvirtual-interface idiom exposes a public nonvirtual method that enforces
validation, locking, or invariants around a private/protected virtual hook. The
[Template Method example](../patterns/behavioral/template_method.cpp) demonstrates
this relationship: `generate()` validates before dispatching format hooks.

**Benefits:** common pre/postconditions remain centralized. **Drawbacks:** the
base class must predict suitable hooks, and exceptions or reentrant callbacks can
complicate shared locking. Do not hold an internal mutex while calling unknown
client code without a deliberate reentrancy/deadlock policy.

Constructor-then-`initialize()` APIs sometimes appear when virtual initialization
is needed. Their cost is an invalid intermediate state and the chance of forgetting
the second step. Prefer a constructor or factory that returns a fully valid object
when possible. Base constructors do not dynamically dispatch to a fully constructed
derived override.

## 8. Copy-and-Swap and Resource-Owner Idioms

Copy-and-swap implements assignment by preparing a new value, then exchanging
state with a nonthrowing swap. If preparation fails, the old value remains.
This can offer the strong exception guarantee and handle self-assignment cleanly.

**Benefits:** simple commit-after-success structure. **Drawbacks:** an extra copy
or allocation may be expensive, and allocator propagation rules can complicate
container-like types. It is not always the most efficient assignment strategy.

Prefer Rule of Zero when standard owning members already do the right thing.
Use a custom resource-owner class only at the layer that actually manages a raw
resource. Higher-level objects should compose that owner rather than repeat raw
cleanup code.

A scope guard executes cleanup on scope exit, useful for rollback of an operation
that is not naturally a standalone resource. C++17 has no standard `scope_exit`;
use a vetted library or a small carefully tested owner when needed. C++23 adds
scope-guard facilities, so examples targeting C++17 cannot assume their availability.

## 9. Object Pool, Multiton, and Service Locator

### Object Pool

A pool reuses expensive objects or fixed storage. Acquisition returns a checked
lease; releasing it makes the object available again.

**Benefits:** can reduce repeated construction and bound allocation behavior.
**Drawbacks:** exhaustion, stale state, use-after-return, synchronization, and
shutdown become explicit design problems. A lease should release through RAII,
and clients must not keep borrowed pointers after returning it. Measure first:
modern allocators or simple values can outperform an unnecessary custom pool.

### Multiton

A keyed instance registry provides one instance per key rather than one instance
for an entire type. It can represent named shared resources, but inherits many
Singleton problems plus key lifetime and eviction concerns. It is not necessarily
a Flyweight: the intent may be unique identity rather than shared intrinsic state.

### Service Locator

A locator lets code ask a central registry for a service. It can simplify plugin
discovery or legacy integration, but hides dependencies and moves missing-service
errors to runtime. A global locator also makes tests order-sensitive. Prefer
constructor injection for ordinary mandatory dependencies; keep discovery at a
composition boundary when it is genuinely needed.

## 10. Architectural Patterns

### Layered Architecture

Presentation calls application use cases, which invoke domain logic and persistence
boundaries. Layers clarify responsibilities and permitted dependency directions.
**Benefits:** familiar organization and separation of concerns. **Drawbacks:**
pass-through layers, domain logic leaking upward/downward, and tightly coupled
changes across every layer. A folder name alone does not enforce a boundary.

### Hexagonal / Ports-and-Adapters Architecture

Core use cases define ports for needed capabilities, and adapters connect databases,
HTTP, files, or UI frameworks. Dependencies point toward policy-owned contracts.
**Benefits:** core behavior can run and be tested without infrastructure.
**Drawbacks:** mapping and interface overhead; overuse can create an adapter around
every trivial operation. The DIP stock-reader example is a small illustration,
not a complete hexagonal application.

### MVC, MVP, and MVVM

These separate model state from presentation with different coordination roles.
MVC introduces controllers, MVP uses presenters that coordinate views, and MVVM
uses a view model suited to binding. Names vary across frameworks, so inspect the
actual data/control flow. **Benefits:** reduced UI/domain entanglement.
**Drawbacks:** synchronization, binding cycles, and oversized coordinators. Observer
and Mediator can help inside these architectures but do not define them alone.

### Repository and Unit of Work

A Repository offers a domain-oriented access boundary over persistence. A Unit of
Work coordinates related changes and their transaction boundary. **Benefits:**
domain code can avoid query/storage details and coordinate commits. **Drawbacks:**
generic CRUD wrappers can hide essential query semantics; transaction scope and
concurrency remain difficult. Do not return zero or an empty collection for every
database failure merely to preserve a simple repository interface.

### CQRS and Event Sourcing

CQRS separates write and read models when their needs differ materially. Event
sourcing stores state-changing facts and derives current state by replaying them.
They are independent choices and need not be used together.

**Benefits:** purpose-built read models, history, and auditable state evolution in
appropriate domains. **Drawbacks:** event/version migration, replay behavior,
eventual consistency, operational complexity, and privacy/deletion constraints.
An undo command list is not automatically an event store, and a database audit log
is not necessarily the source of truth for state.

### Publish/Subscribe and Event Bus

Publishers send events through a channel or broker rather than directly addressing
subscribers. **Benefits:** temporal and deployment decoupling can exceed an
in-process Observer. **Drawbacks:** delivery guarantees, retries, ordering, schema
evolution, and subscriber lag become explicit. At-least-once delivery requires
idempotent consumers; “exactly once” is always a claim within a particular boundary
and failure model, not a universal guarantee.

## 11. Concurrency Patterns Are Separate Contracts

The 42 executable examples are single-threaded. Adding a mutex does not
automatically make an object collaboration correct.

### Producer-Consumer and Thread Pool

Producers enqueue tasks; workers dequeue and execute them. A bounded queue creates
backpressure. **Benefits:** separates task submission from execution and amortizes
thread creation. **Drawbacks:** queue growth, shutdown, task exceptions, starvation,
and deadlocks when workers wait for work scheduled onto the same exhausted pool.
Queued callbacks need valid captures after their submitter's stack frame ends.

### Active Object

An active object owns an execution context and serializes requests to its state.
Commands can represent requests and futures can represent results. **Benefits:**
confining mutation reduces locking in the object. **Drawbacks:** queueing latency,
backpressure, cancellation, and lifecycle complexity. It is not merely any class
that starts a thread.

### Monitor Object

A monitor combines protected state with synchronized operations and condition
waiting. **Benefits:** invariants and locking are kept together. **Drawbacks:**
lock contention, deadlock, and reentrancy. Wait on a condition predicate in a loop;
notifications alone do not establish the desired state and wakeups can be spurious.

### Reactor and Proactor

A Reactor responds to readiness, such as a socket being readable; the application
then performs work. A Proactor responds to completion of an already submitted
asynchronous operation. **Benefits:** scalable I/O coordination without one blocked
thread per request. **Drawbacks:** callback lifetime, cancellation races, ordering,
and platform-specific APIs. Prefer proven event-loop libraries for real systems.

### Read-Mostly State and Immutable Snapshots

Publishing an immutable snapshot can make readers simple, but publication itself
must be synchronized. **Benefits:** readers observe consistent versions without
mutating shared state. **Drawbacks:** retained versions, copy cost, reclamation,
and stale-read policy. Advanced reclamation techniques require rigorous correctness
work; do not replace them with unchecked raw-pointer swaps.

## 12. Distributed Resilience Is Not a Wrapper Detail

Retries can improve availability for transient failures but can multiply load and
duplicate non-idempotent effects. Use bounded attempts, deadlines, backoff, jitter,
and idempotency where appropriate. A timeout means the caller stopped waiting; it
does not prove the remote operation never happened.

A circuit breaker stops repeated calls to an unhealthy dependency and probes for
recovery. **Benefits:** reduces cascading failure and wasted work. **Drawbacks:**
threshold tuning, false trips, and recovery behavior. A bulkhead isolates resource
pools so one overloaded dependency cannot consume all capacity; its cost is lower
resource-sharing efficiency and additional operational tuning.

A Saga coordinates a sequence of local transactions with compensating actions.
**Benefits:** practical coordination across independent services. **Drawbacks:**
intermediate states are observable, compensation can fail, and not every action
is reversible. A transactional outbox writes state and pending events atomically
in one database, then publishes later; consumers still need duplicate handling.

These mechanisms can be expressed with Command, State, Proxy, or Decorator, but
those object patterns do not supply the distributed correctness guarantees.

## 13. Recognize Anti-Patterns

| Smell | Why it hurts | Better first question |
| --- | --- | --- |
| God object | Unrelated reasons to change concentrate in one module | Which responsibilities and invariants belong together? |
| Global mutable Singleton | Hidden dependencies and order-sensitive behavior | Can the application own and inject this state? |
| Inheritance for code reuse only | Derived objects inherit promises they cannot honor | Is this composition rather than substitution? |
| Premature abstraction | Guessed flexibility becomes permanent complexity | What actual variation must be supported? |
| Anemic domain with public mutation | Rules are duplicated across callers | Who should own the invariant-preserving operation? |
| Interface mirroring an entire SDK | Infrastructure concepts still leak into policy | What does this client actually need? |
| Silent catch-and-continue | Failure becomes indistinguishable from success | Which failures can be handled here, and which must propagate? |
| Pattern stacking | Many layers without separate requirements | What concrete problem does each layer solve? |

Not every simple data structure is an anemic-domain problem. DTOs, parsed records,
and message payloads are often intentionally data-only. Diagnose the role before
applying the label.

## 14. Worked Design: Add a New Export Format

Suppose a tool exports measurements as CSV today and must support JSON tomorrow.

First state the contract: ordering, missing values, units, precision, escaping,
stream errors, and whether output must be atomic. Separate those facts from the
choice of formatting mechanism.

If format selection is a small closed enum, a function dispatching to two pure
formatters may suffice. If independent modules provide formats at runtime, an
exporter Strategy and a construction registry may be appropriate. If the export
workflow is fixed but selected steps vary within a framework, Template Method
may fit. If a third-party writer has an incompatible API, wrap it with an Adapter.
Do not add all four unless there are four distinct needs.

Define lifetime next: does the exporter borrow an output stream only during a
call or retain it for asynchronous work? Does the caller own the measurements?
Will cancellation leave a partial file? A temporary-file-and-rename strategy may
support atomic replacement on a particular filesystem, but the exact durability
guarantee depends on the platform and flush protocol.

Test representative values, empty input, escaping, numeric limits, stream failure,
and extension selection. A fake sink makes unit tests deterministic; integration
tests verify real filesystem behavior. Measure large-input throughput before
choosing buffering, pooling, or parallelization.

This is the central habit: use patterns to serve contracts, not to replace them.