# State

## 1. Definition

State is a behavioral pattern that delegates an object's state-dependent behavior
to an object representing its current state. Changing the state object changes
how the context responds to operations.

## 2. The Problem It Solves

An object may react differently to the same event depending on its mode. Repeating
large state switches in every operation scatters transition rules and makes it
easy for one operation to forget a state or permit an invalid transition.

The behavior belonging to each state should be collected in one understandable
place, while the context exposes a stable client interface.

## 3. Understand the Mechanism

The context owns or references its current state. A state interface defines the
operations whose behavior varies. Concrete states implement those operations.
An event may perform work, request a transition, or reject the operation.

Transitions may be chosen by state objects, the context, or a separate transition
table. Their location is a design choice. The state model should identify allowed
transitions, entry/exit effects, and what happens when an operation fails midway.

State is not simply a collection of independent booleans. Explicit states can
exclude impossible combinations, such as being simultaneously disconnected and
actively transmitting through the same connection.

## 4. Real-World Scenario

A network client progresses through disconnected, connecting, connected, and
closing states. A send request may fail while disconnected, queue while connecting,
and transmit while connected. The same public operation has mode-specific behavior.

Real networking also requires deadlines, cancellation, retry limits, and serialized
event handling. A state class hierarchy helps organize those rules but does not
make asynchronous races disappear. A transition table may be clearer for a small
fixed protocol.

## 5. Understand the C++ Example

Open [state.cpp](../../../patterns/behavioral/state.cpp).

`TrafficSignal` is the context. `SignalState` supplies `name()`, `can_go()`, and
`next()`. Red, green, and amber are concrete states.

1. The context starts with a red state, which reports that movement is not allowed.
2. `advance()` asks that state to construct its successor.
3. Only after `next()` returns does the context replace the old state pointer.
4. Green permits movement; the next advance installs amber, which does not.
5. Another advance returns to red.
6. Checks verify the complete cycle and each state-dependent behavior.

Delayed replacement avoids destroying a state while its own method is executing.
The code models only the conceptual cycle, not real traffic safety, timing, or hardware.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** groups mode-specific behavior, exposes transitions, and reduces
repeated state checks in client operations.

**Drawbacks:** many classes for simple machines, scattered transitions if poorly
organized, and allocation costs in this implementation.

Use enums or tables for small closed machines; variants can hold state-specific
data by value. Strategy selects an algorithm, usually externally. State represents
an evolving mode with transition rules.

## 7. Check Your Understanding

**Question:** Who guarantees two simultaneous events do not race to change state?

**Answer:** The concurrency design must do that, for example by serializing events.
Using State objects alone does not synchronize the context.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).