# Liskov Substitution Principle (LSP)

## 1. Definition

The Liskov Substitution Principle says that a derived object must keep the promises
of its base class. Code written for the base class should still work correctly
when it receives the derived object.

**In simple words:** a replacement must do the job that the original type promised.
Being allowed to write `class Square : public Rectangle` does not prove that a
square can safely replace a rectangle in every program.

## 2. The Problem It Solves

A subtype can compile while rejecting inputs the base accepts, changing unrelated
state, weakening results, or throwing for a supposedly supported operation. Clients
then need concrete-type checks and special cases, undermining polymorphism.

The design needs contracts strong enough that callers can reason from the base
interface without knowing which subtype they receive.

## 3. Understand the Principle

Think of the base class as making promises to the code that uses it:

- **Accepted inputs (preconditions):** if the base accepts an empty document, the
	derived class cannot suddenly require a nonempty one.
- **Promised results (postconditions):** if a successful `save()` promises stored
	data, the derived class cannot report success while discarding the data.
- **Rules that stay true (invariants):** if an account promises a nonnegative
	balance, the derived class must preserve that rule.
- **Rules about changes over time:** if changing height promises to leave width
	unchanged, the derived class must keep that promise too.

Failure behavior, units, ownership, and sometimes timing guarantees are observable
parts of a contract too. Identical signatures do not prove substitution. A cached
implementation may violate a base interface that promises fresh data even if the
returned data type is identical.

When substitution fails, possible repairs include changing the common contract,
making types siblings, or using composition rather than inheritance. The solution
is not always “add another override.”

## 4. Real-World Scenario

A storage interface promises that `save()` persists valid documents. A read-only
store cannot truthfully implement that interface by throwing “unsupported” for
every save. It should expose a read capability instead, unless the original
contract explicitly allows that failure.

This illustrates the connection to ISP: a narrower truthful interface can make
substitution possible. Weakening every contract until anything qualifies, however,
makes the interface less useful to clients.

## 5. Understand the C++ Example

Open [lsp.cpp](../../solid/lsp.cpp).

The before rectangle supports independently settable width and height. Its square
subclass changes both values whenever either setter is called.

1. A client sets width to 4, then height to 5, expecting area 20.
2. A rectangle satisfies that expectation.
3. The square's height setter also changes width to 5, producing area 25.
4. The test deliberately detects this broken rectangle contract.
5. The after design makes rectangle and square siblings under read-only `Shape`.
6. Both truthfully provide `area()` without promising independent dimension setters.

The overall executable passes because it detects the before violation and verifies
the corrected design. Positive small dimensions isolate the substitution issue;
arbitrary geometry inputs would require additional consistent validation.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** reliable polymorphism, reusable contract tests, and fewer client
workarounds based on concrete types.

**Drawbacks:** specifying complete contracts takes effort, and the honest shared
interface can be smaller than clients initially hoped. Testing cannot prove every
behavior for a complex stateful interface.

Prefer composition when there is no true substitutable relationship. A mathematical
classification does not automatically justify mutable software inheritance.

## 7. Check Your Understanding

**Question:** Is a square always an invalid rectangle subtype?

**Answer:** No. The violation depends on the contract. A read-only rectangle view
may permit a square; independent width/height mutation is what fails in this example.

See the [SOLID technical notes](../../solid/README.md).