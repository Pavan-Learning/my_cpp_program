# Core Design Principles: Reason About the Tradeoffs

## Concept-First Lessons

Each standalone lesson begins with the principle's definition and deeper reasoning,
then a real-world scenario, and finally a numbered walkthrough of its C++ example.

Each diagram link opens the **flow diagram**, followed immediately by the **class**
and **sequence diagrams**, with explanations tied to that C++ program. Examples
without custom classes use clearly labeled function and data boxes in the class view.

| Topic | Standalone lesson | C++ diagrams |
| --- | --- | --- |
| DRY | [Lesson](../lessons/principles/dry.md) | [Flow, class, sequence](../lessons/principles/dry.md#c-flow-diagram) |
| KISS | [Lesson](../lessons/principles/kiss.md) | [Flow, class, sequence](../lessons/principles/kiss.md#c-flow-diagram) |
| YAGNI | [Lesson](../lessons/principles/yagni.md) | [Flow, class, sequence](../lessons/principles/yagni.md#c-flow-diagram) |
| Separation of Concerns | [Lesson](../lessons/principles/separation_of_concerns.md) | [Flow, class, sequence](../lessons/principles/separation_of_concerns.md#c-flow-diagram) |
| Cohesion and Coupling | [Lesson](../lessons/principles/cohesion_and_coupling.md) | [Flow, class, sequence](../lessons/principles/cohesion_and_coupling.md#c-flow-diagram) |
| Encapsulation | [Lesson](../lessons/principles/encapsulation.md) | [Flow, class, sequence](../lessons/principles/encapsulation.md#c-flow-diagram) |
| Composition Over Inheritance | [Lesson](../lessons/principles/composition.md) | [Flow, class, sequence](../lessons/principles/composition.md#c-flow-diagram) |
| Programming to Interfaces | [Lesson](../lessons/principles/interfaces.md) | [Flow, class, sequence](../lessons/principles/interfaces.md#c-flow-diagram) |
| Law of Demeter | [Lesson](../lessons/principles/law_of_demeter.md) | [Flow, class, sequence](../lessons/principles/law_of_demeter.md#c-flow-diagram) |
| Dependency Injection | [Lesson](../lessons/principles/dependency_injection.md) | [Flow, class, sequence](../lessons/principles/dependency_injection.md#c-flow-diagram) |
| Immutability | [Lesson](../lessons/principles/immutability.md) | [Flow, class, sequence](../lessons/principles/immutability.md#c-flow-diagram) |
| Contracts and Strong Types | [Lesson](../lessons/principles/contracts.md) | [Flow, class, sequence](../lessons/principles/contracts.md#c-flow-diagram) |
| RAII and Ownership | [Lesson](../lessons/principles/raii_and_ownership.md) | [Flow, class, sequence](../lessons/principles/raii_and_ownership.md#c-flow-diagram) |
| Value Semantics and Rule of Zero | [Lesson](../lessons/principles/value_semantics.md) | [Flow, class, sequence](../lessons/principles/value_semantics.md#c-flow-diagram) |

See the [complete lesson index](../lessons/README.md). The material below retains
additional technical details, related principles, and discussions of conflicting advice.

## Technical Notes and Related Principles

There is no universally closed list of every software design principle. This
chapter teaches fourteen core topics with separate runnable C++ examples, followed
by additional related principles and explicit conflicts between them. SOLID has
its own [complete chapter](../solid/README.md).

Principles guide judgment. A design can follow the wording of a principle and
still be worse if it adds unnecessary complexity or ignores the domain contract.
For each topic, identify the concrete problem, inspect the linked program, then
consider where the advice stops being useful.

## 1. DRY: Don't Repeat Yourself

**Source:** [dry.cpp](dry.cpp). **Target:** `dry`.

### Concept and Worked Example

DRY means keeping a piece of knowledge authoritative in one place. It does not
mean that every pair of similar-looking lines must become a shared helper.

The rule “shipping costs 500 cents below a subtotal of 5000 cents and is free at
or above that threshold” belongs to `ShippingRules`. Both the web and kiosk totals
ask that object for the fee. At 4999, the total is 5499; at 5000, it is 5000. The
program checks those boundaries and that both channels agree for the same input.

The two client functions intentionally remain separate, even though they look
alike. The shared business decision is centralized; the clients can later differ
in unrelated presentation or workflow requirements.

**Benefits:** One rule update reaches all intended users; boundary tests have a
natural home; duplicated policies cannot silently drift as easily.

**Drawbacks:** Prematurely merging coincidentally similar rules couples things
that should change independently. A shared helper with many flags can be harder
to understand than two clear implementations. Avoid DRY-by-text-matching.

**C++ and failure cases:** The rules object is borrowed during a call. Negative
subtotals are rejected. In this sample, only small below-threshold values receive
the added fee, so that addition does not approach integer overflow. More general
discount/tax arithmetic needs its own numeric limits.

**Exercise with answer:** A country introduces a different shipping policy. Should
you add country checks to every client? No. Model the changed policy in one place,
possibly using a selected strategy. But do not force unrelated countries into one
formula if their business rules are genuinely independent.

## 2. KISS: Keep It Simple

**Source:** [kiss.cpp](kiss.cpp). **Target:** `kiss`.

### Concept and Worked Example

Choose the simplest design that correctly fulfills the actual requirement. Simple
does not mean shortest, cleverest, or least validated. It means few unnecessary
concepts and behavior a reader can predict.

`largest()` uses `std::max_element` and returns `optional<int>`. Empty input has no
largest value, so it returns `nullopt`. The negative-only input `{-8,-2,-5}` returns
-2, and duplicate maxima are harmless. Returning zero for empty input would be
shorter but would confuse “no answer” with a valid result and fail negative cases.

**Benefits:** Standard algorithms communicate intent; edge cases are explicit;
there is no custom traversal class or sorting step. Runtime is O(N), with O(1)
extra space.

**Drawbacks:** An overly simple local solution can ignore real concurrency,
performance, or extensibility requirements. “Keep it simple” is not permission
to omit necessary error handling or to leave a growing god function intact.

**Alternatives:** Sorting costs O(N log N) and may mutate or copy the collection,
so it is unnecessary for one maximum. For repeated updates and queries, a maintained
data structure may be simpler overall than rescanning every time.

**Exercise with answer:** Why not return `INT_MIN` for empty input? Because that
can be a legitimate maximum. `optional` keeps absence separate from every valid
integer value.

## 3. YAGNI: You Aren't Gonna Need It

**Source:** [yagni.cpp](yagni.cpp). **Target:** `yagni`.

### Concept and Worked Example

Do not build speculative capabilities without a current justified need. The
present requirement is a text report of available stock. `stock_report()` returns
`Available: 12` for 12 and handles zero. It does not introduce a plugin registry,
format factory, report scheduler, or serialization framework.

This intentionally small example demonstrates that deciding not to add a pattern
can be the correct design decision. No output-format extensibility requirement
has been established.

**Benefits:** Less code to maintain and test; faster feedback on real needs;
fewer guessed abstractions that later turn out to be wrong.

**Drawbacks:** Applying YAGNI too literally can ignore foreseeable irreversible
decisions. Data migration, safety, security, API compatibility, and capacity planning
can be current requirements even when their consequences arrive later.

**Boundaries:** The function formats a count; it does not enforce inventory validity.
Negative stock, if forbidden by the domain, should be prevented by the stock model
or validated at the relevant input boundary. YAGNI is not an argument against
validation, tests, RAII, or documenting assumptions.

**Exercise with answer:** A second real requirement asks for JSON. Is adding an
output abstraction now a YAGNI violation? No. Reassess using the concrete requirement.
Two small functions may still be enough; add polymorphism only if selection and
future variation justify it.

## 4. Separation of Concerns

**Source:** [separation_of_concerns.cpp](separation_of_concerns.cpp).
**Target:** `separation_of_concerns`.

### Concept and Worked Trace

Keep different kinds of work behind clear boundaries: interpreting input, applying
business rules, and presenting results are different concerns.

```text
"2" -> parse_weight() -> 2
2   -> shipping_cents() -> 400
400 -> display_price() -> "400 cents"
```

Parsing uses `std::from_chars`, verifies both the error code and full consumption,
and rejects `2kg` instead of silently treating it as 2. The pricing function checks
the domain range 1..1000 independently of textual syntax. A syntactically valid
`"0"` parses, but is not a valid shipping weight.

**Benefits:** Business logic can be tested without a terminal, file, or UI. The
same policy can serve different frontends. Parsing and formatting changes do not
need to alter the price formula.

**Drawbacks:** Too many artificial layers create unnecessary mapping and forwarding.
Some work, such as orchestration of a single use case, legitimately connects concerns.
Separate responsibilities without hiding the actual workflow.

**C++ details:** `string_view` borrows input only for the call; it is not retained.
The parser rejects empty strings, trailing text, and out-of-range integers. Leading
whitespace is not accepted by this parser; normalization is an explicit upstream
choice, not an accidental side effect.

**Exercise with answer:** Should `shipping_cents()` print an error and return zero
for invalid weight? No. That mixes presentation with domain decisions and makes an
error indistinguishable from free shipping. Report an explicit failure and let the
caller choose how to display it.

## 5. High Cohesion and Low Coupling

**Source:** [cohesion_and_coupling.cpp](cohesion_and_coupling.cpp).
**Target:** `cohesion_and_coupling`.

### Concept and Worked Example

Cohesion measures how closely a module's responsibilities belong together. Coupling
describes its dependencies on other modules, especially their details. Good design
usually puts closely related state and behavior together and minimizes unnecessary
knowledge across boundaries.

`Stock` owns its count and the reservation rule. `Fulfillment` asks it to reserve
a quantity; it cannot directly decrement a public field. Starting with 3 units,
reserving 2 leaves 1. Another request for 2 fails and leaves 1 unchanged.

**Benefits:** The inventory invariant has one owner. The caller depends on an
operation rather than storage representation. Related changes are more localized.

**Drawbacks:** “Low coupling” is not “no coupling.” A warehouse must depend on some
stock behavior. Excess abstraction can increase conceptual coupling through complex
protocols even while reducing direct includes. Cohesion cannot be measured simply
by counting methods.

**C++ and limits:** This example intentionally uses a concrete reference because
there is only one stock implementation. A virtual interface is not required just
to demonstrate a useful boundary. The object is single-threaded; `reserve()` is
one logical operation but not automatically atomic across threads.

**Exercise with answer:** Would exposing `int& count()` simplify fulfillment?
It would let callers violate the stock invariant and couple them to storage.
Keep operations such as reserve/release on the object that owns the rule.

## 6. Encapsulation and Information Hiding

**Source:** [encapsulation.cpp](encapsulation.cpp). **Target:** `encapsulation`.

### Concept and Worked Example

Encapsulation groups state and behavior behind an API. Information hiding keeps
volatile representation decisions out of clients. A private field with a public
unrestricted setter is not strong invariant protection.

`Account` starts with zero cents. `deposit(500)` followed by `withdraw(200)` leaves
300. Withdrawing 400 fails without mutation. A deposit that would overflow `int`
is rejected before arithmetic. These are domain operations, not general field writes.

**Benefits:** Validity is preserved at one boundary. Clients cannot create a
negative balance by changing a field. Representation can change without exposing
new internal details.

**Drawbacks:** An overly restrictive API can force awkward workarounds. A large set
of trivial forwarding methods can conceal rather than improve the model. Hiding
data from the language's access rules is not a security boundary against hostile
code in the same process.

**C++ details:** The overflow guard checks `max - balance` before addition.
`balance()` returns a value, not a mutable reference. With the nonnegative balance
invariant, the subtraction used by the guard is safe. Invalid positive-range inputs
throw; insufficient funds is an ordinary false result. Those two failure categories
are deliberately distinguished.

**Exercise with answer:** Should there be `set_balance(int)` for convenience?
Usually not for ordinary callers. An import/recovery path may need a separate
validated operation with authority and audit semantics, rather than unrestricted
mutation of a financial invariant.

## 7. Favor Composition Over Inheritance

**Source:** [composition.cpp](composition.cpp). **Target:** `composition`.

### Concept and Worked Example

Use “has-a” collaboration to combine capabilities unless a true behavioral
substitution relationship justifies public inheritance. An inspection robot has a
motor and a camera; it is not a kind of motor or camera.

`InspectionRobot` stores both as values and calls their operations in `inspect()`.
The result is `moving: photo`. No virtual functions or dynamic allocations are
needed to express these fixed relationships.

**Benefits:** Capabilities can evolve independently. The robot does not inherit
unrelated motor operations as part of its public API. Value members have simple
ownership and automatic cleanup.

**Drawbacks:** Composition can require forwarding methods. Runtime replacement
requires a selected interface or callable representation. Some framework designs
are explicitly based on inheritance and cannot be rewritten locally as composition.

**Alternatives:** Public inheritance fits a real substitutable interface, such as
a concrete renderer satisfying `Renderer`. Private inheritance can support special
implementation techniques but should not be used merely because access to
protected members is convenient. A template can compose policies at compile time.

**Exercise with answer:** Add a thermal camera. Must you derive
`ThermalInspectionRobot`? Not necessarily. Supply a suitable camera component,
possibly through a template or runtime interface, if interchangeable cameras are
an actual requirement.

## 8. Program to Interfaces, Not Implementations

**Source:** [interfaces.cpp](interfaces.cpp). **Target:** `interfaces`.

### Concept and Worked Trace

An interface is a behavioral contract, not necessarily a class with virtual
functions. `emit_report(Sink&)` needs an object that accepts `write(string_view)`.
It is a template, so unrelated types can satisfy that protocol.

`StringSink` appends to a string. `StreamSink` writes to a borrowed output stream.
The same report function produces `status: ready\n` through both, and checks
compare the results. The stream in the test is an `ostringstream`, so no file or
terminal dependency is required to inspect its output.

**Benefits:** Clients depend on what they need rather than concrete representation.
Templates can permit inlining and avoid runtime dispatch. Alternative implementations
are straightforward to test.

**Drawbacks:** C++17 template errors can be verbose, headers expose implementations,
and instantiations can increase code size. Runtime plugin selection is harder than
with a virtual interface or type-erased callable.

**Contract details:** Both sinks consume the view during the call rather than
retaining a borrowed pointer. A real stream sink must define failure reporting:
iostream insertion may set error flags without throwing unless exceptions are
enabled. Matching `write()` syntax alone does not prove identical error semantics.
C++20 concepts can name the compile-time requirements but still cannot express
every behavioral guarantee.

**Exercise with answer:** Does adding `virtual` automatically satisfy this principle?
No. An abstract interface that exposes an entire vendor SDK still couples clients
to that implementation's concepts. The abstraction must describe the client's need.

## 9. Law of Demeter: Limit Knowledge of Object Internals

**Source:** [law_of_demeter.cpp](law_of_demeter.cpp). **Target:** `law_of_demeter`.

### Concept and Worked Example

A client that calls `order.customer().address().postcode()` knows the internal
navigation path. Changing where an address lives can break every such client.
The Law of Demeter encourages collaboration with immediate partners rather than
reaching through their internals.

The client asks `order.shipping_label()`. The order delegates to its customer,
and the customer asks its address. The result is `Ship to 10115`. Each object
depends on its direct collaborator's meaningful operation.

**Benefits:** Representation changes have a smaller blast radius. Client code
expresses a goal rather than a chain of storage lookups. Internal objects can
remain hidden.

**Drawbacks:** Blindly replacing every access chain with forwarding methods can
bloat outer APIs. Read-only data transfer objects, fluent builders, and ranges may
legitimately use chaining. Count knowledge dependencies, not dots in source code.

**C++ details:** The example owns nested objects by value, so there are no dangling
references from getters. Returning labels as strings gives callers independent
results. In a real system, address formatting policy might belong to a dedicated
formatter if localization varies independently from address data.

**Exercise with answer:** Is `builder.url(...).timeout(...).build()` a violation
because it has many dots? Not for that reason. The chain uses the builder's own
fluent API; it does not navigate through unrelated private collaborators.

## 10. Dependency Injection

**Source:** [dependency_injection.cpp](dependency_injection.cpp).
**Target:** `dependency_injection`.

### Concept and Worked Example

Supply dependencies from outside instead of constructing or locating them inside
the object that uses them. Time is a dependency just as much as a database is.

`Expiration` receives a clock callable. The test controls an integer clock and
checks deadline 100 at times 99, 100, and 101. Expiration is false before the
deadline and true at and after it. No sleep, wall clock, or flaky timing test is
needed. An empty clock is rejected by the constructor.

**Benefits:** Dependencies are explicit; tests control external conditions;
configuration can be centralized in a composition root such as `main()`.

**Drawbacks:** Constructor argument lists can grow, and lifetime management remains
the caller's responsibility. Excessive mocking can test only wiring rather than
real contracts. Injection of every trivial operation can fragment a design.

**C++ details:** `std::function` owns the callable, but the lambda captures `clock`
by reference. Owning the lambda does not own the referenced integer. The clock
outlives `Expiration` in the example. Capture by value when a snapshot is intended,
or own a separate clock object when shared evolving time is needed.

Use a monotonic clock such as `steady_clock` for durations/deadlines in production,
unless the requirement explicitly uses civil time. Constructor injection makes
mandatory dependencies explicit; setter injection permits late replacement but
can allow temporarily invalid objects. A DI container is optional.

**Exercise with answer:** Is passing a concrete database by reference DIP? It is
DI. It becomes dependency inversion only when high-level policy depends on an
appropriate abstraction rather than that low-level concrete detail.

## 11. Immutability and Functional Core

**Source:** [immutability.cpp](immutability.cpp). **Target:** `immutability`.

### Concept and Worked Example

An immutable value is not modified after creation; a transformation produces a new
value. Pure functions compute outputs from inputs without externally visible side
effects. Keeping a functional core inside an imperative shell makes business rules
easier to test and reason about.

`DocumentVersion("draft").append(" reviewed")` produces another version while the
original remains `draft`. The program checks both and an empty append. There is no
in-place text setter; callers receive const access to the text.

**Benefits:** Old versions remain usable; aliasing is easier to reason about;
deterministic transformations simplify testing. Properly published immutable data
can be read concurrently without locking around the data itself.

**Drawbacks:** Copying large values can be expensive. Many versions retain memory.
Persistent data structures or shared immutable chunks can help but add complexity.
I/O and state changes still need an imperative boundary.

**C++ details:** This is an immutable public text API with ordinary value replacement
semantics: a nonconst `DocumentVersion` variable can still be assigned another whole
value. A `const` pointer does not imply a const pointee, and a `const` method can
mutate `mutable` fields or external objects. Returning a const string reference
borrows the version's lifetime. Concurrent destruction or unsynchronized publication
is not made safe merely by removing setters.

**Exercise with answer:** Would storing `shared_ptr<string>` make versions safely
immutable? Not by itself. Shared mutable strings can change through another alias.
Use immutable pointees and ensure no mutable aliases remain, or copy the data.

## 12. Design by Contract, Fail Fast, and Strong Types

**Source:** [contracts.cpp](contracts.cpp). **Target:** `contracts`.

### Concept and Worked Example

A precondition says what a caller must provide. A postcondition says what a
successful operation guarantees. An invariant is a property preserved by every
valid object state. Fail-fast validation rejects invalid state near its source.

`Percentage` can be constructed only with values 0..100. Its `of()` operation accepts
amounts 0..1,000,000 cents. It multiplies in `long long` before dividing, then returns
integer cents. Tests cover 0%, 100%, 25% of 999 yielding 249, and rejection of 101%.

The type stores a validated percentage instead of passing an unqualified integer
around and repeatedly wondering whether it means percent, cents, or kilograms.

**Benefits:** Invalid values are blocked at construction; error locations are
clear; strong types reduce accidental mixing of units; contracts support tests.

**Drawbacks:** Excess checks at every internal layer can duplicate work and obscure
the trusted boundary. Exceptions may not fit every embedded or real-time policy.
A contract requiring valid input still needs a defined enforcement strategy for
untrusted input.

**C++ details:** `assert` may disappear when `NDEBUG` is defined, so it must not be
the only validation for external input. This project uses throwing checks for its
tests so Release runs still execute them. C++17 does not have the C++23
`std::expected` type; use an appropriate result representation or library when
exceptions are unsuitable. Avoid relying on signed integer overflow, which is
undefined behavior.

The multiplication casts before multiplying; casting an already-overflowed result
would be too late. Integer division truncates here, so rounding is part of the
documented domain policy, not an accidental implementation detail.

**Exercise with answer:** Should invalid 101% silently clamp to 100%? Only if
clamping is the intended API contract. Silent repair otherwise hides caller bugs
and can produce financially incorrect results.

## 13. RAII and Explicit Ownership

**Source:** [raii_and_ownership.cpp](raii_and_ownership.cpp).
**Target:** `raii_and_ownership`.

### Concept and Worked Trace

Resource Acquisition Is Initialization ties a resource's lifetime to an object's
lifetime. Destruction releases the resource on normal scope exit and exception
unwinding. Resources include memory, files, locks, sockets, and transactions that
need rollback unless committed.

The demo `Connection` increments an active-resource count on construction and
decrements it on destruction. It is a deterministic resource simulation, not a
network connection. `unique_ptr<Connection>` is the exclusive owner.

The test moves ownership from one pointer to another without creating a second
connection, then checks cleanup at scope exit. A separate operation throws after
acquisition; unwinding still brings the active count back to zero.

**Benefits:** Cleanup is local and automatic; early returns and exceptions do not
need duplicated cleanup branches; ownership transfer is explicit through moves.

**Drawbacks:** Destructors must not let exceptions escape during unwinding.
Resources with fallible finalization may need an explicit `close()` or `commit()`
that reports errors, with a nonthrowing destructor fallback. Lifetime may be less
obvious when ownership is shared across many objects.

**C++ choices:** Use values first, `unique_ptr` for exclusive dynamic ownership,
`shared_ptr` for genuine shared lifetime, and references/raw pointers for documented
borrowing. `weak_ptr` observes shared ownership without extending it. Two shared
pointers owning each other form a cycle. `shared_ptr` control-block safety does not
make the pointee thread-safe.

RAII does not run cleanup after every kind of termination: abrupt process exit,
crashes, and power loss require OS or durable recovery mechanisms. The static demo
counter is single-threaded. A real resource handle must also define move, invalid
handle, acquisition failure, and close semantics.

**Exercise with answer:** Why is a manual `delete` in a catch block weaker here?
Every return and exception path must remember the same cleanup, and future edits
can miss one. A single owning object handles all ordinary scope exits consistently.

## 14. Value Semantics and the Rule of Zero

**Source:** [value_semantics.cpp](value_semantics.cpp). **Target:** `value_semantics`.

### Concept and Worked Example

A value behaves like an independent piece of data. Copying a notebook should copy
its notes, not make later edits to one notebook unexpectedly affect the other.
Rule of Zero means using members that already manage their resources so the class
does not need custom copy/move/destruction functions.

`Notebook` stores `vector<string>`. A copy receives the same initial note; adding
a second note to the copy leaves the original size at one. Moving the copy into a
new value preserves both notes in the destination. The moved-from notebook is
then assigned a fresh value and reused.

Compile-time checks confirm copy construction and nonthrowing move construction.
Runtime checks verify independence and destination data. No custom destructor,
copy constructor, or move assignment is needed.

**Benefits:** Standard members handle cleanup and exception safety; APIs have
predictable copy behavior; moves can transfer expensive storage cheaply; fewer
special functions mean fewer ownership bugs.

**Drawbacks:** Deep copies can be costly, and not every domain object is naturally
a value. Services, mutexes, identity-bearing entities, and self-referential objects
may need copying disabled or a carefully defined clone operation.

**C++ details:** A moved-from standard container is valid but generally unspecified;
do not teach that every moved-from vector must be empty. The example checks the
destination, then reassigns the source before checking new contents. A generated
copy of a `shared_ptr` member shares its pointee rather than deep-copying it, so
Rule of Zero alone does not prove value independence for every member choice.

If you directly manage a resource, the Rule of Five reminds you to consider the
destructor, copy constructor, copy assignment, move constructor, and move assignment
together. Prefer placing that complexity in a small resource owner, then composing
it into higher-level Rule-of-Zero types.

**Exercise with answer:** Add a raw owning `char*` to `Notebook`. Is the generated
copy still safe? No. It copies the address, not the allocation. Prefer `string` or
another owning value; otherwise implement and test the full ownership policy.

## Additional Principles and Their Practical Meaning

### Tell, Don't Ask

Prefer `account.withdraw(amount)` over fetching a balance, calculating externally,
and calling a generic setter. The object with the invariant decides whether the
operation is allowed. This helps encapsulation and reduces duplicated rules.
It is not a ban on queries: reports and views legitimately ask for data. The
account example demonstrates telling for mutation and querying for presentation.

### Command-Query Separation

A query observes without changing logical state; a command changes state. Clear
separation makes repeated reads predictable. However, an operation such as atomic
`try_pop()` must both remove an item and report success to avoid a race between
separate empty-check and pop calls. Treat CQS as guidance, not a reason to split an
operation that must be atomic. CQRS, the architectural separation of read and write
models, is a larger decision and not required by this method-level principle.

### Principle of Least Astonishment

Name and shape an API so ordinary calls do what readers expect. `size()` should not
silently clear a container; a copy should not unexpectedly steal ownership. State
important exceptions explicitly, such as a proxy's first call performing I/O.
Familiar behavior lowers learning cost, but conventions must not hide real domain
requirements or important failure behavior.

### Explicit Is Better Than Implicit

Use meaningful units, named configuration, explicit constructors where conversion
would surprise, and visible dependency injection. A timeout represented as
`chrono::milliseconds` communicates more than a bare `int`. Excess verbosity can
also hide intent, so standard idioms such as range-for remain valuable.

### Acyclic Dependencies and Stable Boundaries

If module A needs B, B needs C, and C needs A, isolated changes and builds become
difficult. Move shared policy or a narrow interface into an appropriate lower-level
contract module instead of introducing mutual includes. Forward declarations can
reduce compilation dependencies but do not eliminate a conceptual cycle. Stable
core policy should not import volatile UI or database details simply to compile.

### Package Cohesion and Dependency Principles

The Common Closure Principle groups code that tends to change together. The Common
Reuse Principle warns against packaging unrelated facilities so every consumer
must depend on all of them. The Reuse/Release Equivalence Principle connects a
reusable unit with a versioned release unit. These can conflict: an easy-to-release
large package may impose unnecessary dependencies on small clients.

The Acyclic Dependencies Principle avoids cycles between packages. The Stable
Dependencies Principle favors depending toward more stable contracts. The Stable
Abstractions Principle encourages widely depended-on, difficult-to-change modules
to expose extensible abstractions where variation is needed. “Stable” means costly
to change because of dependents, not simply old code. Do not split every class
into its own package or make all stable data types abstract; evaluate real consumers.

### Locality, Modularity, and Explicit State

Keep a feature's related knowledge near its owner, expose small module APIs, and
minimize mutable global state. Locality improves comprehension and limits change
impact, but one giant file is not modularity. Explicit state machines can make
invalid transitions visible where scattered boolean flags cannot. Two independent
booleans permit four combinations; if only three are valid, model the actual states.

### Least Privilege and Secure Defaults

Give a component only the authority it needs: a reader need not receive a deletion
capability, and an unauthenticated request should not gain access merely because
a chain has no terminal handler. This principle reduces damage from mistakes but
needs real enforcement at trust boundaries. C++ access specifiers and in-process
interfaces alone do not isolate malicious code or replace authorization checks.

### Design for Testability and Observability

Separate deterministic decisions from I/O, inject time and randomness, and expose
meaningful errors. Record enough operational context to diagnose failures without
logging secrets. Over-mocking can miss integration defects, and excessive logging
can add cost or reveal sensitive data. A good boundary supports both focused unit
tests and contract tests against real implementations.

## When Principles Conflict

| Tension | How to decide |
| --- | --- |
| DRY vs coupling | Share the same knowledge, not merely similar syntax |
| OCP vs YAGNI | Add an extension point for demonstrated or well-founded variation |
| KISS vs scalability | Optimize the full real workflow, not only today's smallest input |
| Encapsulation vs test access | Test through meaningful APIs before exposing private state |
| Immutability vs performance | Measure copying; share immutable chunks if justified |
| Low coupling vs too many interfaces | Add boundaries where they isolate real change or effects |
| CQS vs atomic operations | Preserve atomic correctness and report the outcome explicitly |
| Fail fast vs service resilience | Reject invalid state locally; isolate failures at service boundaries |

**Worked decision:** A pricing rule appears in web and kiosk clients and changes
together. Extract that rule (DRY). Keep input parsing separate (separation of
concerns). Do not add a plugin system for hypothetical providers (YAGNI). When two
actual policies arrive, inject a focused policy (DIP/DI and OCP). Test both against
the same rounding and range contract (LSP). Each step responds to evidence rather
than applying every principle mechanically.