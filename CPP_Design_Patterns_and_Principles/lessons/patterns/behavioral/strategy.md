# Strategy

## 1. Definition

Strategy is a behavioral pattern that encapsulates a family of interchangeable
algorithms behind a common contract. A context delegates a particular task to a
selected strategy instead of embedding every algorithm in its own implementation.

## 2. The Problem It Solves

A system may perform the same conceptual task in several ways: routing, pricing,
compression, or scheduling. A growing switch intertwines those implementations
with the surrounding workflow. Subclassing the entire context for each algorithm
duplicates responsibilities that did not change.

The varying algorithm should become a collaborator, leaving the context's stable
work intact.

## 3. Understand the Mechanism

The strategy contract states the input, result, and guarantees shared by algorithms.
Concrete strategies implement different policies. A client or composition point
chooses a strategy and supplies it to the context. The context validates common
requirements, delegates, and uses the result without concrete-type inspection.

Interchangeability is semantic. Algorithms must agree on units, valid inputs,
failure reporting, and any promised properties. Two methods returning integers are
not equivalent strategies if one returns cents and the other returns whole currency units.

## 4. Real-World Scenario

A route planner offers fastest, shortest, and accessible-route policies. The
application supplies a selected policy while keeping origin/destination handling
and result presentation stable.

Accessible routing may need richer map information than shortest-path routing.
If the common interface cannot provide necessary context, the contract must evolve
explicitly. A hidden global lookup is not a good way to force an incompatible
algorithm through an underspecified interface.

## 5. Understand the C++ Example

Open [strategy.cpp](../../../patterns/behavioral/strategy.cpp).

`ShippingPolicy` defines a price operation. Standard and express implementations
provide formulas. `Checkout` borrows the selected policy.

1. Standard shipping is installed initially.
2. For 2 kg, its formula gives 300 + 2 * 50 = 400 cents.
3. `use(express)` replaces the selected policy without changing checkout code.
4. The same weight now gives 600 + 2 * 100 = 800 cents.
5. Checkout rejects zero weight before invoking a strategy.
6. Checks verify both results and the common validation rule.

Both strategy objects outlive checkout. Storing a reference to a temporary would
be unsafe. The formulas are demonstrated only within the context's validated range.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** independently testable algorithms, runtime replacement, and reduced
coupling between algorithm details and surrounding workflow.

**Drawbacks:** callers must choose a suitable policy; interfaces can become awkward
when algorithms require different context; trivial policies may produce excessive classes.

A callable or `std::function` can express a one-operation strategy. Templates give
compile-time policy selection. A small closed switch can be simplest. Template
Method varies inherited steps in a skeleton rather than composing a policy object.

## 7. Check Your Understanding

**Question:** Must Strategy use inheritance?

**Answer:** No. The idea is replaceable algorithms under a contract. Virtual classes,
callables, and template policies are different C++ mechanisms for expressing it.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).