# Singleton

## 1. Definition

Singleton is a creational pattern that restricts a type to one accessible instance
within its intended scope and provides a globally reachable access point to it.
It combines **instance-count restriction** with **global access**. Those are two
separate design decisions, even though the pattern packages them together.

## 2. The Problem It Solves

Some applications need one shared coordinator or one set of process-wide metadata.
Uncontrolled construction could create competing instances with inconsistent state.
A Singleton makes construction controlled and gives callers a known access route.

However, “the application currently creates one object” is not enough justification.
One object constructed in `main()` and passed to its users can satisfy the same
instance-count requirement without hidden global dependencies.

## 3. Understand the Mechanism

Construction is inaccessible to ordinary clients. A controlled accessor creates or
retrieves the one instance. Copying must not provide an accidental second instance.
Lifetime and initialization order become part of the type's contract.

Separate three questions: is initialization safe, is later access safe, and is
shutdown safe? Synchronizing initialization does not synchronize mutations. Likewise,
a unique process-local instance does not ensure uniqueness across processes or
machines. An access pattern is not a distributed coordination mechanism.

## 4. Real-World Scenario

Suppose a small command-line application exposes immutable build metadata: version,
build identifier, and enabled features. Every component should see the same data,
and it never changes during execution. A globally accessible immutable object can
be workable here, although namespace constants may be simpler.

Contrast that with application settings that differ for each customer session.
Making those settings global would prevent independent sessions and isolated tests.
The first scenario has uniform process lifetime; the second has multiple legitimate
contexts. Singleton is often misused by treating those contexts as one.

## 5. Understand the C++ Example

Open [singleton.cpp](../../../patterns/creational/singleton.cpp).

`Settings` has a private constructor and deleted copy operations. Its static
`instance()` function returns a const reference to a function-local static object.

1. The first call reaches `static const Settings settings` and initializes it.
2. Later calls return that same object rather than constructing another.
3. Comparing addresses verifies identity, not merely equal field values.
4. Reading `application_name()` returns `Pattern demo`.
5. The const public interface offers no operation that mutates shared settings.

C++11 and later synchronize initialization of the function-local static. This
example does not test mutable concurrent access or shutdown interactions. Other
static destructors must not use this object after its lifetime has ended.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** controlled construction, convenient access, and potentially lazy
initialization of genuinely uniform process-wide state.

**Drawbacks:** hidden dependencies, test interference, inflexible configuration,
shutdown-order hazards, and mutable-global contention. Plugins and shared-library
linkage can complicate assumptions about process-wide uniqueness.

Prefer an application-owned object with dependency injection when clients need
different configurations or explicit lifetime. Use constants for simple fixed
metadata. `thread_local` means one instance per thread, not one global instance.

## 7. Check Your Understanding

**Question:** Does a thread-safe `instance()` make an ordinary counter field safe?

**Answer:** No. Initialization and later mutation are different operations. A
counter needs its own synchronization or an atomic representation with suitable
semantics. Restricting instance count does not prevent data races.

See the [creational technical notes](../../../patterns/creational/README.md).