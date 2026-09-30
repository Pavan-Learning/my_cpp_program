# High Cohesion and Low Coupling

## 1. Definition

Cohesion describes how strongly a module's responsibilities belong together.
Coupling describes the dependencies between modules. Aim to keep related state
and behavior together while minimizing unnecessary knowledge of other modules'
representations and implementation details.

## 2. The Problem It Solves

Scattered rules make correctness depend on many callers coordinating perfectly.
At the other extreme, a large utility object may contain unrelated work that
changes for many reasons. Both arrangements make changes spread unpredictably.

The design should put each rule with its natural owner and expose the narrow
collaboration needed by other components.

## 3. Understand the Principle

High cohesion is not simply small size. A substantial inventory class can be
cohesive if its operations all maintain inventory meaning. A tiny helper combining
unrelated date, network, and invoice operations can still have low cohesion.

Low coupling is not no coupling. Useful modules collaborate. The question is
whether a collaborator depends on a stable operation or on private fields, ordering
assumptions, and storage details.

An extra interface can reduce implementation coupling, but an elaborate protocol
can increase conceptual coupling. Judge how much a reader must know and how far
a real change propagates, not just how many headers are included.

## 4. Real-World Scenario

A warehouse service reserves inventory for orders. If every order channel reads
stock, subtracts quantities, and writes it back itself, the stock invariant is
distributed across callers. A reservation operation keeps the rule with inventory.

For concurrent orders, that operation also needs an atomic implementation. Giving
it a cohesive home makes enforcement possible but does not itself provide locks
or database transaction semantics.

## 5. Understand the C++ Example

Open [cohesion_and_coupling.cpp](../../principles/cohesion_and_coupling.cpp).

`Stock` owns count and reservation. `Fulfillment` borrows stock and requests the
operation rather than modifying count directly.

1. Stock starts with three units.
2. Fulfillment requests two; stock checks availability and leaves one.
3. A second request for two fails.
4. The failed reservation leaves the count unchanged at one.
5. Tests verify both outcomes and print the remaining count.

The dependency is concrete because the small example has one implementation.
That does not erase the useful boundary: fulfillment depends on behavior rather
than the representation of the count.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** localized invariants, clearer ownership, smaller change impact, and
more understandable collaborations.

**Drawbacks:** over-isolation introduces forwarding and coordination overhead.
Some data and operations are genuinely shared across a use case.

Start with a coherent operation. Introduce a polymorphic boundary when testing,
deployment, or implementation variation justifies it, not solely to lower a count
of concrete dependencies.

## 7. Check Your Understanding

**Question:** Would returning `int&` to the stock count reduce coupling?

**Answer:** No. It would let callers depend on storage and bypass rules. A meaningful
reservation operation communicates less implementation knowledge and preserves ownership.

See the [principles technical notes](../../principles/README.md).