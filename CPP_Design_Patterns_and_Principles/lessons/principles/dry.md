# DRY: Don't Repeat Yourself

## 1. Definition

DRY says that each piece of authoritative knowledge should have one clear
representation in a system. A business rule, format definition, or invariant
should not need independently coordinated edits in several places.

The principle targets duplicated **knowledge**, not every repeated line of syntax.

## 2. The Problem It Solves

When the same rule is copied into multiple clients, one copy eventually changes
without the others. The system then gives inconsistent answers depending on which
path a user takes. Tests also duplicate assumptions and can drift together with code.

Centralizing the real decision makes its ownership and change location clear.
However, combining unrelated rules merely because they currently look alike
creates a different maintenance problem: accidental coupling.

## 3. Understand the Principle

Ask whether two pieces of code must change together for the same reason. If they
express the same policy, share the policy. If they only happen to use the same
formula today, keep independent responsibilities independent until evidence shows
a meaningful common abstraction.

The authoritative representation can be a function, value object, schema, or
generated definition. It need not be a universal utility class. A good abstraction
names the shared knowledge; a helper with many mode flags may merely conceal
unrelated implementations behind one API.

## 4. Real-World Scenario

A subscription system applies an eligibility rule in a web checkout, support tool,
and renewal job. If each reimplements the rule, customers may qualify in one path
but fail in another. A shared policy or authoritative service keeps the decision
consistent.

Two different countries' tax rules may initially have the same percentage but
change independently. Treating them as one rule would be false DRY. Common arithmetic
can be reused while each jurisdiction's policy retains its own identity.

## 5. Understand the C++ Example

Open [dry.cpp](../../principles/dry.cpp).

`ShippingRules` owns the free-shipping threshold and fee policy. Web and kiosk
functions both ask it for the fee rather than repeating the threshold decision.

1. A subtotal below 5000 receives a 500-cent fee.
2. The 4999 boundary therefore totals 5499.
3. At 5000, shipping is free and the total stays 5000.
4. Both clients produce the same result for a 2000-cent subtotal.
5. Negative subtotals are rejected by the rule's validation.

The client functions remain separate even though their current arithmetic looks
similar. The policy is what must stay consistent; unrelated client workflows may
later evolve independently.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** consistent decisions, one policy change point, and focused boundary tests.

**Drawbacks:** premature unification couples unrelated concerns and can produce
flag-heavy generic helpers. Shared dependencies also require intentional versioning.

Share stable knowledge; tolerate some local repetition while the real relationship
is uncertain. KISS and YAGNI help prevent an elaborate abstraction created only
to eliminate a few similar lines.

## 7. Check Your Understanding

**Question:** Must two functions using `amount * 2` become one function?

**Answer:** Not necessarily. One might calculate double points and the other a
two-person booking. Similar arithmetic is not proof of one business rule.

See the [principles technical notes](../../principles/README.md).