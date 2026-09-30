# Open/Closed Principle (OCP)

## 1. Definition

The Open/Closed Principle says that a software component should support extension
of an identified kind of behavior without repeatedly modifying its stable core.
It is open to a chosen variation and closed against routine changes caused by that
variation, not permanently immune to all edits.

## 2. The Problem It Solves

A growing algorithm switch forces every new case into a shared function. Even a
small extension can require retesting unrelated cases, coordinating edits between
teams, and changing code that previously worked.

When the same dimension varies repeatedly, it can deserve a deliberate extension
point. Stable policy and varying detail should not always be edited together.

## 3. Understand the Principle

First identify the likely change: pricing policy, export format, storage provider,
or another concrete dimension. Then define a contract allowing new implementations
of that variation. Existing orchestration uses the contract rather than enumerating
every implementation.

Extension does not eliminate modification everywhere. Composition or registration
must still select the new implementation. Bugs and new requirements may require
changes to the core or contract. An incorrect abstraction should be revised, not
preserved at the cost of hidden dependencies and workarounds.

OCP is therefore relative: a design can make new families easy while making new
product categories difficult. Ask what the design is open to, not whether it is
“fully open/closed” in the abstract.

## 4. Real-World Scenario

A monitoring platform supports several alert-routing policies. Adding a policy
should not require changing the stable process that validates an event, obtains
a routing decision, and records delivery attempts.

The policy interface must provide enough event context and specify error behavior.
If a new policy fundamentally changes delivery guarantees, it may require a real
contract change rather than fitting neatly into the old extension point.

## 5. Understand the C++ Example

Open [ocp.cpp](../../solid/ocp.cpp).

The before function switches between regular and member customers. The after
function validates input and invokes a `PricingRule`.

1. `Regular` returns the original price.
2. `Member` subtracts one tenth using integer arithmetic.
3. `Festival` adds a new one-fifth discount rule without changing the after function.
4. For 1000 cents, the three results are 1000, 900, and 800.
5. Checks compare existing cases with the before implementation and verify the new one.

The varying formula moved behind a contract; common validation remains in `price()`.
Call-site rule objects are borrowed only during the call. Rounding is part of the
contract: integer subtraction of a fraction can differ from multiplying then truncating.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** localized extensions, independently testable policies, and reduced
risk to unrelated branches of stable code.

**Drawbacks:** additional abstractions, selection wiring, and the risk of guessing
the wrong future variation. A vague contract lets extensions behave inconsistently.

Use a small exhaustive switch for a genuinely closed set when that is clearer.
Strategies, callbacks, and templates can all provide extension points. YAGNI warns
against building extensibility for requirements with no evidence.

## 7. Check Your Understanding

**Question:** Is editing `main()` to choose a new policy an OCP violation?

**Answer:** No. Selection must occur somewhere. The protected boundary is the
stable algorithm, not every line in the application.

See the [SOLID technical notes](../../solid/README.md).