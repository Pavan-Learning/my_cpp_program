# Encapsulation and Information Hiding

## 1. Definition

Encapsulation groups state with the operations that govern it and controls access
through an API. Information hiding keeps internal representation and changeable
implementation decisions outside that API. Together, they let an object maintain
its invariants rather than relying on every caller to do so.

## 2. The Problem It Solves

Public mutable fields allow any caller to create invalid combinations or bypass
business rules. Replacing those fields with unrestricted getters and setters may
change syntax without improving the situation.

The important question is who can create or change valid state. A useful API exposes
domain operations, not merely remote control of every private field.

## 3. Understand the Principle

An invariant is a condition that must hold for every valid observable object state.
Constructors establish it, operations preserve it, and read APIs avoid exposing
mutable access that bypasses it.

Information hiding also protects evolution. Clients calling `reserve()` need not
know whether stock is a field, database record, or computed aggregate. The operation's
contract can remain stable while representation changes.

Encapsulation is not secrecy or a complete security mechanism. C++ access control
helps structure trusted code; it does not isolate hostile machine code in the same
process or replace authorization at system boundaries.

## 4. Real-World Scenario

A ticket inventory should never sell more seats than it owns. Exposing a seat-count
setter lets callers accidentally create negative availability or overwrite another
reservation. A `reserve(quantity)` operation can validate and update consistently.

Production concurrency still requires atomic enforcement. The encapsulated API
is the place to implement that enforcement, not proof that it already exists.

## 5. Understand the C++ Example

Open [encapsulation.cpp](../../principles/encapsulation.cpp).

`Account` hides its balance and exposes deposit, withdrawal, and a value-returning query.

1. Depositing 500 raises the balance from zero to 500.
2. Withdrawing 200 succeeds and leaves 300.
3. Withdrawing 400 returns false and preserves the balance.
4. A deposit that would overflow is rejected before arithmetic changes state.
5. Checks verify all outcomes, including the unchanged balance after rejection.

The overflow condition compares against `max - balance_` before addition.
`balance()` returns a copy, so callers cannot mutate the field through the query.
Invalid arguments and ordinary insufficient-funds results use distinct failure paths.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** centralized validity, reduced caller assumptions, and safer internal
representation changes.

**Drawbacks:** an overly restrictive API can become awkward, and excessive forwarding
can obscure simple data. Data-transfer records may appropriately expose plain values.

Protect domain invariants where they exist. Do not add private fields and trivial
setters merely for appearance. Tell, Don't Ask complements encapsulation by moving
decisions to the object that owns the relevant rule.

## 7. Check Your Understanding

**Question:** Does a public `set_balance(-100)` preserve encapsulation's purpose
just because the actual field is private?

**Answer:** No. It still allows callers to violate the intended invariant. An API
must control meaningful state transitions, not only field visibility.

See the [principles technical notes](../../principles/README.md).