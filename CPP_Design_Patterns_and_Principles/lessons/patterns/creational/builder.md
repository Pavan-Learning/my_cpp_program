# Builder

## 1. Definition

Builder is a creational pattern that separates step-by-step construction of a
complex object from the object's final representation. Construction decisions
accumulate in a builder; a usable product is produced when construction is complete.
The classic form can use the same construction sequence for different representations.

## 2. The Problem It Solves

A product may have required fields, optional fields, defaults, and rules connecting
several fields. A long constructor makes arguments difficult to interpret. Public
setters allow clients to observe half-configured objects. Many overloaded
constructors create a growing number of combinations.

The important distinction is between **valid construction progress** and a **valid
finished product**. A partially filled builder can be acceptable even when a
partially filled product would be unsafe to use.

## 3. Understand the Mechanism

The builder records construction choices. Its final operation verifies the choices
and returns the product. An optional director applies a reusable recipe by calling
builder steps in a particular sequence. The product exposes its useful behavior
without needing every construction setter in its normal public API.

A fluent interface returns the builder from setters so calls can be chained. Fluent
syntax alone is not Builder: a chain that mutates a live object may offer no
separation of construction and use. A staged builder goes further by using types
to make required construction steps impossible to skip at compilation time.

## 4. Real-World Scenario

Consider generating a travel itinerary. Destinations, dates, travelers, transport,
and optional accommodation are chosen progressively. Before producing a bookable
itinerary, the system checks date ordering and incompatible selections.

A “business trip” recipe might apply defaults, while a “family trip” recipe applies
different ones. Neither recipe should expose a supposedly complete itinerary before
its invariant holds. Real booking still needs availability checks and transactions;
a builder validates configuration but cannot guarantee external reservations.

## 5. Understand the C++ Example

Open [builder.cpp](../../../patterns/creational/builder.cpp).

`Request` is the finished value. Its constructor is private. `Request::Builder`
holds pending URL, timeout, and authentication choices. `RequestDirector` provides
a health-check recipe.

1. A fresh builder has an empty URL, timeout 1000, and authentication disabled.
2. `url("/orders")` supplies a required value and returns the same builder.
3. `timeout(500)` replaces the default; `authenticate()` enables the option.
4. `build()` rejects an empty URL or a nonpositive timeout.
5. On success, it calls the private constructor to return a complete `Request`.
6. The program verifies all three options and prints `/orders timeout=500`.
7. Separate checks verify the director's 200 ms recipe and invalid configurations.

`build() const` copies configuration, so the builder remains reusable. Fluent
references are safe during the temporary's full expression, but retaining a
reference to that temporary builder after the statement would dangle.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** names configuration choices, centralizes validation, supports recipes,
and protects finished products from incomplete construction state.

**Drawbacks:** duplicates some fields, adds maintenance, and may defer mistakes to
runtime. Reusing a builder can accidentally retain an old option unless its reuse
semantics are clear.

Use it when construction complexity justifies it. For a small product, a constructor
or configuration value is simpler. A staged builder trades additional types for
compile-time restrictions; a move-consuming builder can avoid expensive copies.

## 7. Check Your Understanding

**Question:** Why validate in `build()` when setters could check values?

**Answer:** Setters can reject individual invalid values, but some rules require
the whole configuration, such as start date preceding end date. The final boundary
must establish all invariants before a usable product escapes.

See the [creational technical notes](../../../patterns/creational/README.md).