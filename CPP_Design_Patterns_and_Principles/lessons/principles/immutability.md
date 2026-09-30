# Immutability and Functional Core

## 1. Definition

Immutability means a value's observable content does not change after creation;
transformations produce new values. A functional core computes results from inputs
without external side effects, while an imperative shell performs necessary I/O
and state updates around it.

## 2. The Problem It Solves

Shared mutable state makes behavior depend on who changed a value most recently.
Aliases can observe surprising changes, tests become order-sensitive, and concurrent
readers need coordination with writers.

Keeping old values unchanged makes reasoning more local: a result depends on its
inputs rather than a hidden mutation history.

## 3. Understand the Principle

Distinguish an immutable value from an unchangeable variable. A variable may be
assigned a new whole value while each produced version is treated as independent.
Also distinguish shallow constness from an immutable reachable object graph.

A const method can modify `mutable` state or objects reached through references.
A const smart pointer can still point at mutable data. Genuine shared immutability
requires controlling mutable aliases, not just adding `const` at one level.

Immutable data reduces synchronization needs for readers, but publication, lifetime,
and replacement still require safe coordination. It is not a blanket guarantee of
thread safety for the surrounding system.

## 4. Real-World Scenario

A configuration service builds a complete validated configuration version, then
publishes it to request handlers. Existing requests continue using their old version,
while new requests use the new one. No request observes half an update.

The pointer publication and old-version lifetime must be synchronized. Large
versions can share immutable substructures, but retaining too many versions costs
memory. These are explicit tradeoffs rather than reasons to mutate shared fields ad hoc.

## 5. Understand the C++ Example

Open [immutability.cpp](../../principles/immutability.cpp).

`DocumentVersion` exposes text for reading and an `append()` transformation that
returns a new version instead of modifying its receiver.

1. The original contains `draft`.
2. Appending ` reviewed` constructs a new string and a new version.
3. Checks confirm the original is still `draft`.
4. The new version contains `draft reviewed`.
5. An empty append produces equivalent content without changing the original.

The public API does not offer in-place text mutation. Ordinary whole-object
assignment remains possible for a nonconst variable. The returned text reference
borrows the version's lifetime and must not be kept after destruction.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** predictable aliases, easy snapshots, deterministic transformations,
and simpler reasoning about concurrent reads after safe publication.

**Drawbacks:** copying and retained versions can consume time and memory. Persistent
structures reduce copying but add implementation complexity.

Use mutation inside a clear exclusive-ownership boundary where it is simpler and
measured performance warrants it. Immutability is a powerful default for shared
values, not a requirement to copy every buffer on every operation.

## 7. Check Your Understanding

**Question:** Does `shared_ptr<string>` guarantee immutable versions?

**Answer:** No. Another owner can mutate the same string. Shared immutable storage
needs an immutable contract and no remaining mutable aliases.

See the [principles technical notes](../../principles/README.md).