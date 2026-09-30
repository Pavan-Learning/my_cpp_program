# Facade

## 1. Definition

Facade is a structural pattern that exposes a simplified, higher-level interface
to a subsystem containing several cooperating components. It provides a convenient
entry point for common use cases without requiring clients to understand all
subsystem details.

## 2. The Problem It Solves

Performing one user-visible action may require many low-level calls in the correct
order. If every client repeats the sequence, subsystem knowledge spreads and
clients can disagree about initialization, validation, and error handling.

The application needs one operation expressed in the client's vocabulary, while
the subsystem retains its smaller specialized components.

## 3. Understand the Mechanism

The client calls the facade. The facade coordinates subsystem objects and returns
a result suitable for that use case. Subsystem objects usually do not need to know
the facade exists; they keep their own responsibilities.

A facade does not necessarily hide all lower-level APIs. Advanced clients may use
them if the architecture permits it. Where bypass would violate invariants, access
must be restricted deliberately rather than assumed from the pattern name.

Convenience is not atomicity. If step three fails after steps one and two produced
effects, the facade needs a defined recovery policy. A single method call can still
represent a partially completed multi-system operation.

## 4. Real-World Scenario

Imagine a video-export application. Export requires selecting a codec, decoding
frames, converting audio, encoding output, and writing a container. An export
facade accepts input and output settings and coordinates that pipeline.

The user-facing client need not understand each codec library. Yet cancellation,
unsupported formats, and partial output must still be surfaced. Hiding every failure
behind a generic “done” result would simplify syntax at the expense of correctness.

## 5. Understand the C++ Example

Open [facade.cpp](../../../patterns/structural/facade.cpp).

`Inventory` owns stock. `Payment` simulates approval. `CheckoutFacade` borrows both
and provides `checkout()`.

1. The facade first checks whether stock exists.
2. With declined payment, it returns `payment declined` without reserving stock.
3. The check confirms that the original unit remains available.
4. With approved payment, it reserves the unit and returns `order confirmed`.
5. A subsequent call returns `out of stock`.

The checks cover successful coordination and both rejection paths. The output
summarizes that verification. Payment is only a boolean simulation, and this
single-threaded sequence is not a production transaction or reservation protocol.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** simpler clients, centralized workflow ordering, reduced exposure to
subsystem changes, and a focused testing boundary.

**Drawbacks:** can grow into an oversized coordinator, conceal necessary features,
or obscure partial failures. It adds coordination rather than eliminating it.

Use it for a meaningful common use case. A thin forwarding method that adds no
abstraction may not help. Adapter translates an incompatible API; Mediator governs
colleague interactions; Facade offers a client-oriented subsystem entry point.

## 7. Check Your Understanding

**Question:** Does `checkout()` being one function prevent two clients buying the
same last item?

**Answer:** No. Concurrent reservation needs an atomic stock operation or another
concurrency mechanism. Facade organizes calls; it does not make them indivisible.

See the [structural technical notes](../../../patterns/structural/README.md).