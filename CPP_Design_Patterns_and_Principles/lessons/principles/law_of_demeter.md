# Law of Demeter

## 1. Definition

The Law of Demeter encourages an object to collaborate with its immediate partners
rather than navigating through their internal object relationships. It is often
summarized as limiting knowledge of other objects' internal structure.

It is not a mechanical rule against multiple dots in an expression.

## 2. The Problem It Solves

A client that reaches through order, customer, address, and postcode objects knows
how another part of the system is assembled. Moving address information elsewhere
can then break many clients that never needed that structural knowledge.

Such navigation leaks representation across boundaries and spreads responsibility
for interpreting it. The client should often ask for the meaningful result instead.

## 3. Understand the Principle

An operation can ask its direct collaborator to perform relevant work. That
collaborator may delegate to its own direct partner. Each boundary exposes behavior
rather than the entire path through private state.

Delegation is useful when it protects real independence. Blindly adding forwarding
methods for every nested property can bloat outer APIs and hide a simpler data model.
Read-only transfer objects and deliberate navigation structures may reasonably
expose their data.

The deeper question is how many unrelated implementation decisions a caller must
understand. Fluent calls on one builder do not necessarily reveal any such decisions.

## 4. Real-World Scenario

A shipping screen needs a destination label for an order. If it retrieves nested
customer records and formats the address itself, changes to saved addresses or
guest checkout spread into UI code.

A shipping-label operation can own that decision, possibly delegating formatting
to a separate service when localization varies independently. The boundary should
represent the use case, not merely conceal a getter chain with another name.

## 5. Understand the C++ Example

Open [law_of_demeter.cpp](../../principles/law_of_demeter.cpp).

`Order` owns a `Customer`, which owns an `Address`. The client asks only the order
for `shipping_label()`.

1. An address is created with postcode `10115`.
2. The customer receives that address, and the order receives the customer.
3. The order delegates its label request to its direct customer.
4. The customer asks its direct address for the formatted label.
5. The returned value is `Ship to 10115`, which is checked and printed.

Nested objects are owned by value. The returned string is an independent result,
not a mutable reference into private storage. The caller does not depend on accessors
for the internal object graph.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** reduced knowledge of representation, smaller change impact, and
client code expressed in domain goals.

**Drawbacks:** excessive forwarding can enlarge APIs and conceal legitimate data
access. The principle should not forbid sensible immutable data models.

Apply it where navigation exposes volatile relationships. Use explicit query
models or DTOs when clients genuinely need structured read-only data.

## 7. Check Your Understanding

**Question:** Is `builder.url(...).timeout(...).build()` necessarily a violation?

**Answer:** No. The chain uses one intended fluent API. Count knowledge dependencies,
not punctuation.

See the [principles technical notes](../../principles/README.md).