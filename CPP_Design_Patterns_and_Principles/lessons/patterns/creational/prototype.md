# Prototype

## 1. Definition

Prototype is a creational pattern that creates new objects by copying existing
configured objects. In a polymorphic system, the original object supplies a cloning
operation that preserves its actual runtime type.

The central idea is **start from this configured example**, rather than “construct
this class and repeat every configuration step.”

## 2. The Problem It Solves

A caller may know only an abstract interface while needing an independent object
with the same current configuration. It cannot name the correct derived constructor.
Copying a base value can discard derived data, and reconstructing from scratch may
lose settings that the caller does not understand.

Copying a pointer is not the answer: that creates another handle to the same object.
The design must state which state is duplicated and which state, if any, is shared.

## 3. Understand the Mechanism

A prototype interface declares `clone()`. Every concrete prototype implements the
operation using its knowledge of its real type and state. The caller gets a new
owned object through the common interface.

Cloning is a semantic decision. Independent mutable fields usually need independent
storage. Immutable resources may be shared. Identity, subscriptions, file handles,
and mutexes often must be reset, recreated, or excluded. A deep copy of an object
graph must preserve intended sharing and handle cycles; blindly recursing is not
a complete clone algorithm.

## 4. Real-World Scenario

Imagine a diagram editor with a library of configured symbols. A user chooses a
styled process box, duplicates it, then edits only the duplicate's caption.
Reconstructing every style setting manually is unnecessary; the selected symbol
already embodies the desired configuration.

The duplicate needs its own editable caption and position. It might share an
immutable font resource, but it should normally receive a new document identity.
Copying the old identity could corrupt selection or history bookkeeping. Prototype
solves object creation from an exemplar, not the entire document duplication policy.

## 5. Understand the C++ Example

Open [prototype.cpp](../../../patterns/creational/prototype.cpp).

`Shape` defines the clone and behavior interface. `Circle` stores a radius and
color. Its `clone()` creates a `Circle` from `*this` and returns it as
`unique_ptr<Shape>`.

1. The original circle begins with radius 5 and color red.
2. Calling `clone()` invokes `Circle`'s implementation, preserving circle data.
3. The generated copy constructor copies the integer and the string value.
4. The first check confirms equivalent descriptions.
5. Changing the clone's color to blue changes its own string, not the original's.
6. The remaining checks confirm `red circle r=5` and `blue circle r=5`.

The original is a stack value; the clone has exclusive dynamic ownership. The
string member provides independent value storage. Replacing it with a shared
mutable pointer would change that independence even if `clone()` stayed unchanged.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** preserves runtime type, reuses existing configuration, and keeps
concrete copy knowledge out of clients.

**Drawbacks:** each subtype needs a correct cloning policy. Complex graphs and
external resources make copying expensive or ambiguous. A clone is not necessarily
cheaper than construction.

Use normal value copying when the concrete type is known. Use Builder when the
product is assembled from choices rather than copied from an existing exemplar.
A prototype registry can supply named templates, but also needs lifetime rules.

## 7. Check Your Understanding

**Question:** Does copying a `shared_ptr` implement an independent clone?

**Answer:** No. Both handles point to the same object. Sharing can be intentional
for immutable resources, but independent mutable state requires a new object and
an appropriate copy policy.

See the [creational technical notes](../../../patterns/creational/README.md).