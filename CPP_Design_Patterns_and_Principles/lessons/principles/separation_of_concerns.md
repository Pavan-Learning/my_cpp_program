# Separation of Concerns

## 1. Definition

Separation of concerns organizes software so different kinds of responsibility
have distinct boundaries. Input interpretation, domain decisions, storage, and
presentation should not become inseparable merely because one use case needs them all.

## 2. The Problem It Solves

A function that reads input, calculates a price, updates a database, and prints a
message is difficult to test or reuse in another interface. Changing presentation
can accidentally affect calculation, and testing the rule requires irrelevant I/O.

The solution is to separate the concerns while preserving explicit orchestration
of the whole workflow.

## 3. Understand the Principle

Each boundary converts one kind of information into another through a clear
contract. Parsing decides whether text has valid syntax. Domain validation decides
whether the resulting value is permitted. Formatting decides how a valid result
is represented to a user.

These decisions are related but not identical. The text `0` can be syntactically
valid while being an invalid parcel weight. A domain operation should report that
failure without deciding whether a GUI should show a dialog or an API should
return an error response.

Separation need not mean separate classes, processes, or repositories. Focused
functions can provide the required boundaries in a small program.

## 4. Real-World Scenario

A booking price rule is used by a website, mobile application, and support console.
Keeping the rule separate from input widgets and response formatting allows each
frontend to reuse the same domain decision.

The application layer still coordinates parsing, calculation, and persistence.
Too many layers that only forward values can add noise; the goal is independence
of concerns, not a prescribed number of directories.

## 5. Understand the C++ Example

Open [separation_of_concerns.cpp](../../principles/separation_of_concerns.cpp).

1. `parse_weight("2")` uses `from_chars` to produce integer 2.
2. It verifies full input consumption, so `2kg` is rejected rather than partially accepted.
3. `shipping_cents(2)` checks the permitted range and computes 400.
4. `display_price(400)` produces `400 cents`.
5. `main()` composes the stages and also tests them separately.

The price calculation requires no text parsing or terminal access. A different
frontend can call it directly. The input view is borrowed during parsing only;
no reference to temporary input is retained.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** reusable domain logic, deterministic tests, clearer errors, and
localized changes to input or presentation.

**Drawbacks:** artificial layers can create repeated data mapping and obscure
simple workflows. Boundaries still require explicit coordination.

Use the smallest meaningful separation. SRP asks about one module's reasons to
change; separation of concerns describes the broader organization of responsibilities.

## 7. Check Your Understanding

**Question:** Should the pricing function print an error and return zero?

**Answer:** That mixes presentation with policy and confuses failure with free
shipping. Return or throw an explicit failure and let the presentation boundary
decide how to communicate it.

See the [principles technical notes](../../principles/README.md).