# Dependency Injection (DI)

## 1. Definition

Dependency injection supplies a component's collaborators from outside rather than
having it construct or globally locate them itself. It makes the choices needed
for an object's behavior explicit at a composition boundary.

## 2. The Problem It Solves

An object that directly reads the wall clock, opens a database, or creates a network
client is coupled to external conditions. Tests become slow or nondeterministic,
and different deployments require changing internal construction logic.

The object's core job should be separable from the choice of collaborators used
to perform that job.

## 3. Understand the Principle

Constructor injection makes required dependencies available for the whole valid
lifetime. Method injection supplies a collaborator for one operation. Setter
injection permits later replacement but may create temporarily incomplete objects.

Injection and ownership are independent. A constructor can receive an owned value,
an owning pointer, or a borrowed reference. Receiving a callable by value does not
make references captured by that callable owned.

DI is also distinct from DIP. DI describes supplying dependencies. DIP describes
the direction and abstraction of those dependencies. Injecting a concrete vendor
object is still DI, even if the policy remains tied to the vendor's details.

## 4. Real-World Scenario

A session service decides whether a login session has expired. Reading the actual
clock inside every decision makes boundary tests depend on timing and waiting.
Supplying a clock lets tests evaluate just before, exactly at, and just after expiry.

Production code can supply an appropriate real clock. Duration-based deadlines
usually need a monotonic source; civil-time deadlines have different semantics.
Injection makes the choice visible but does not choose the correct clock for you.

## 5. Understand the C++ Example

Open [dependency_injection.cpp](../../principles/dependency_injection.cpp).

`Expiration` receives `std::function<int()>` as its clock dependency and rejects
an empty callable.

1. The test creates an integer clock at 99.
2. A lambda capturing that integer by reference is injected into `Expiration`.
3. For deadline 100, the result is false at 99.
4. Changing the controlled clock to 100 makes the result true.
5. At 101 it remains true, with no sleep or wall-clock dependency.

The object owns the callable, but the callable borrows the integer. That integer
must remain alive for every invocation. The comparison explicitly treats the
deadline itself as expired, making the boundary contract testable.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** deterministic tests, visible dependencies, replaceable environments,
and centralized construction choices.

**Drawbacks:** wiring and lifetime responsibilities move to composition code;
over-injection fragments simple operations; excessive mocks can miss real integration behavior.

Use manual wiring until a container solves an actual problem. A service locator
offers discovery but often hides mandatory dependencies instead of making them explicit.

## 7. Check Your Understanding

**Question:** Does `std::function` owning a lambda keep every captured object alive?

**Answer:** No. A reference capture remains borrowed. The capture and ownership
choices must match the dependency's required lifetime.

See the [principles technical notes](../../principles/README.md).