# Memento

## 1. Definition

Memento is a behavioral pattern that captures an object's state so it can be
restored later without exposing that state's internal representation to the object
storing the snapshot.

## 2. The Problem It Solves

Undo, checkpoints, and speculative edits need previous state. Letting an external
history manager read and write every private field breaks encapsulation and forces
it to understand the object's internal invariants.

The object that knows its own state should control capture and restoration. The
history manager should only need to retain an opaque snapshot.

## 3. Understand the Mechanism

The **originator** creates and restores snapshots. The **memento** contains saved
state. The **caretaker** stores mementos without manipulating their internals.
Public snapshot metadata can be exposed deliberately without making restoration
fields freely editable.

A snapshot is a copy of relevant state, not necessarily the whole object. External
connections, callbacks, and transient caches may need exclusion or reconstruction.
Snapshot consistency also matters: concurrent capture of changing fields can
produce a combination that never existed unless capture is synchronized.

## 4. Real-World Scenario

A CAD application allows users to preview a complex operation and cancel it.
Before applying the preview, the model captures restorable geometry state. The
preview controller keeps the snapshot but does not edit private geometry structures.

Cancellation asks the model to restore itself. This does not automatically reverse
external effects such as files already exported. Large models may use deltas or
persistent structures rather than copying the complete model every time.

## 5. Understand the C++ Example

Open [memento.cpp](../../../patterns/behavioral/memento.cpp).

`Editor` is the originator. `Editor::Snapshot` stores text and originator identity
privately, granting `Editor` access. `main()` acts as caretaker.

1. The editor builds the text `draft`.
2. `save()` returns a snapshot containing an independent text value.
3. A further edit produces `draft with changes`.
4. `restore(checkpoint)` assigns the saved text back, giving `draft`.
5. Repeated restoration remains valid.
6. Restoring into another editor throws because the owner identity differs.

The snapshot does not keep the editor alive. Its owner pointer is only a local
identity check, not a durable identifier that survives destruction and address
reuse. The example disables editor copying and assumes snapshots belong to their
live original editor.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** restoration without exposing representation, centralized snapshot
logic, and support for checkpoints or speculative work.

**Drawbacks:** memory and copy cost, sensitive-data retention, versioning issues,
and inability to reverse external side effects merely by restoring memory.

Use inverse commands when they are cheaper and reliable. Use immutable versions
when structural sharing fits. Command records an action; Memento records state,
and the two frequently cooperate.

## 7. Check Your Understanding

**Question:** Should a caretaker modify a snapshot to repair a private field?

**Answer:** No. That defeats the encapsulation boundary. Ask the originator to
perform a valid domain operation or provide an explicit validated migration path.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).