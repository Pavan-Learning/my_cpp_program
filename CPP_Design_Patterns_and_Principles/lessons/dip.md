# Dependency Inversion Principle (DIP)

## 1. Definition

The Dependency Inversion Principle says that the important rules of an application
should not be tied directly to a particular database, network library, or device.
Instead, the rules describe the service they need through an interface, and the
chosen database or device code implements that interface.

**In simple words:** business code asks for a job to be done without needing to
know which tool does it. For example, a warehouse asks for an item's stock count
without knowing whether the answer comes from memory or a database.

The technical names are **high-level policy** for the business rules,
**low-level details** for tools such as database drivers, and **abstraction** for
the interface between them.

## 2. The Problem It Solves

A business rule that directly constructs a database driver or invokes a vendor
SDK inherits that detail's API, lifecycle, and testing requirements. Changing
infrastructure can force changes to business behavior, and testing a simple
decision may require an entire external service.

The dependency direction should allow details to serve policy rather than making
policy fit whatever a particular detail happens to expose.

## 3. Understand the Principle

First describe what the business code needs, such as `available(item)`. That
interface is a **contract**: it states what callers may ask and what answers or
errors they can expect. The business code uses the interface; storage code provides
an implementation. Setup code, often in `main()`, connects the two.

That setup location is sometimes called a **composition point**. It chooses the
actual objects so the business code does not have to choose them itself.

Keep the interface near the business code or in a shared contract module. If the
interface exposes all the database driver's commands and types, business code still
needs to understand that driver. Adding virtual functions alone has not removed
the unwanted dependency.

Dependency injection supplies an object from outside; DIP governs what the policy
depends on. Injecting a concrete database is DI but still leaves concrete coupling.
A DI container is optional; manual constructor wiring can express both ideas clearly.

## 4. Real-World Scenario

A reservation service needs to ask whether capacity can be reserved. Its business
logic should not know SQL table layouts or HTTP client details. A reservation port
defines the needed atomic outcome, and database or remote adapters implement it.

The contract must reflect reality. Replacing an atomic reservation with a stale
availability query breaks semantics even if both adapters compile. Fakes and real
implementations need shared contract tests, not just interchangeable method names.

## 5. Understand the C++ Example

Open [dip.cpp](../solid/dip.cpp).

The before warehouse contains a concrete simulated `SqlStock`. The after warehouse
depends on `StockReader`, implemented by a configurable `MemoryStock`.

1. One memory reader contains three books; another is empty.
2. Each reader is injected into a warehouse constructor.
3. `can_ship()` asks only for available quantity and tests whether it is positive.
4. The stocked warehouse returns true for `book`.
5. The empty warehouse and unknown-item query return false.
6. Tests exercise those policy outcomes without database infrastructure.

The warehouse borrows its reader, which must outlive it. The sample is an availability
query, not an atomic shipment guarantee. It demonstrates dependency direction while
deliberately simplifying storage and concurrency.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** policy tests without infrastructure, replaceable details, visible
dependencies, and business-oriented contracts.

**Drawbacks:** more interfaces and wiring, possible mismatch between fakes and real
adapters, and overengineering if every stable value receives an abstraction.

Use DIP at meaningful effect or change boundaries. Templates and callable contracts
can also express inverted dependencies; virtual interfaces are not mandatory.

## 7. Check Your Understanding

**Question:** Should database timeout be reported as zero available stock?

**Answer:** Not unless that is the explicit contract. An outage and genuine absence
have different meanings. A useful abstraction preserves important failures instead
of disguising them to keep its interface superficially simple.

See the [SOLID technical notes](../solid/README.md).