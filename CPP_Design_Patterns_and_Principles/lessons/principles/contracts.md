# Design by Contract, Fail Fast, and Strong Types

## 1. Definition

Design by Contract means writing down what a function expects and what it promises
in return. It also states the rules that an object must keep while it is used.
For example: a discount must be between 0% and 100%, and an invalid discount must
be rejected before a calculation uses it.

**Fail fast** means reporting bad input close to where it enters the program, not
letting it cause a confusing error later. **Strong types** give different kinds of
values different types, such as `Percentage` and `Money`, so they are harder to mix up.

## 2. The Problem It Solves

A bare integer might mean percent, kilograms, or cents. Without clear contracts,
invalid values travel through the program until they trigger a distant failure or
produce plausible but incorrect results. Repeated informal checks drift between callers.

The design should make validity and meaning explicit at the relevant boundaries.

## 3. Understand the Principle

Three useful terms describe the agreement:

- **Precondition:** what must be true before a call, such as an amount being within
	the supported range.
- **Postcondition:** what is promised after a successful call, such as returning
	the percentage of that amount using the stated rounding rule.
- **Invariant:** a rule kept throughout normal use of an object, such as a stored
	percentage always being between 0 and 100.

Check the percentage when constructing the object, and do not let later operations
break that rule. Calculation code can then trust the stored percentage instead of
checking it repeatedly. It must still check any separate limits on the amount.

Define the failure mechanism deliberately: exceptions, result values, or another
project convention. Debug assertions may disappear in Release builds, so they are
not sufficient for required validation of untrusted input.

Fail fast does not mean terminate the entire service for every error. Reject the
invalid operation locally and handle the failure at an appropriate boundary. Nor
does it require repeating expensive validation at every already-trusted internal layer.

## 4. Real-World Scenario

A billing system accepts a discount percentage from a request. Constructing a
validated discount value at the boundary prevents 150% or negative discounts from
silently reaching calculation code. A separate money type can prevent mixing
currency amounts with percentages.

The contract must also specify rounding and maximum amounts. Correct types do not
automatically make arithmetic overflow-safe or financially appropriate.

## 5. Understand the C++ Example

Open [contracts.cpp](../../principles/contracts.cpp).

`Percentage` checks that its stored integer is between 0 and 100. Its `of()` operation
also limits the input amount and uses a larger integer type for the multiplication,
so the temporary result does not overflow before division.

1. Zero percent of 1000 returns zero.
2. One hundred percent returns 1000.
3. Twenty-five percent of 999 computes 24975 / 100, yielding 249 after truncation.
4. Constructing 101% throws before a usable invalid object escapes.
5. Checks verify the boundaries and rounding example.

The cast to `long long` happens before multiplication. Casting after overflow
would be too late. The amount range is an explicit teaching contract, not a claim
that every financial amount can fit in this representation.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** localized errors, clearer units, fewer invalid states, and testable
boundary conditions.

**Drawbacks:** overvalidation adds noise or cost; exceptions may not fit every
runtime; badly chosen types can make ordinary operations cumbersome.

Choose domain types and validation at meaningful boundaries. Clamping is appropriate
only when explicitly part of the API, not as a silent substitute for rejection.

## 7. Check Your Understanding

**Question:** Why not silently clamp 101% to 100%?

**Answer:** That changes the caller's request and hides a bug unless clamping is
the agreed behavior. Rejection keeps the invalid input visible and actionable.

See the [principles technical notes](../../principles/README.md).