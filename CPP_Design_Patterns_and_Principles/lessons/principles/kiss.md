# KISS: Keep It Simple

## 1. Definition

KISS recommends the simplest design that correctly satisfies the actual
requirements. Simplicity means understandable behavior and few unnecessary
concepts, not merely the shortest source code.

## 2. The Problem It Solves

An implementation can become harder to maintain through clever expressions,
unneeded frameworks, excessive configuration, or custom solutions to already-solved
problems. Every additional concept increases the effort needed to predict behavior.

At the same time, omitting error handling may make code shorter but behavior less
clear. Simplicity must be judged against the full contract, including edge cases.

## 3. Understand the Principle

Start with the smallest correct model. Use standard algorithms and explicit
representations for absence and failure. Separate exceptional cases clearly instead
of encoding them as surprising magic values.

Evaluate complexity over the whole workflow. A linear scan is simple for one
maximum query; a maintained index may be simpler operationally for millions of
repeated queries on changing data. Requirements and measurements decide when a
more elaborate implementation becomes justified.

Simplicity is not resistance to abstraction. An abstraction that removes repeated
reasoning can simplify a system, while one that only adds names and forwarding may not.

## 4. Real-World Scenario

A service needs to validate a fixed configuration at startup. A small typed
configuration reader with clear validation may suffice. Building a live plugin
framework and expression language for those fixed options adds failure modes without
serving a present requirement.

If runtime reconfiguration later becomes necessary, the design can add versioning
and synchronization deliberately. KISS does not forbid that complexity when the
requirement actually demands it.

## 5. Understand the C++ Example

Open [kiss.cpp](../../principles/kiss.cpp).

`largest()` returns an optional maximum using `std::max_element`.

1. Empty input returns `nullopt`, making absence explicit.
2. Nonempty input uses a standard linear scan.
3. For `{-8, -2, -5}`, the maximum is -2, not an assumed initial zero.
4. Repeated maxima such as two nines require no special case.
5. The example prints the maximum of a known nonempty input.

The algorithm is O(N) with constant extra space. Sorting would do unnecessary
work and might require copying or mutating the input. Returning an optional avoids
confusing a valid integer with an “empty” sentinel.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** easier review, fewer moving parts, familiar behavior, and less
maintenance of custom algorithms.

**Drawbacks:** “simple” can become an excuse to ignore real performance, safety,
or concurrency requirements. Local brevity can push complexity onto callers.

Choose the simplest complete contract and implementation. Introduce specialized
structures when the workload demonstrates their value, not merely because they
look more sophisticated.

## 7. Check Your Understanding

**Question:** Why not return zero for an empty collection?

**Answer:** Zero is a legitimate value and is not the maximum of negative-only
input. Explicit absence is slightly more syntax but much clearer semantics.

See the [principles technical notes](../../principles/README.md).