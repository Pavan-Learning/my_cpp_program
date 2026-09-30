# Observer

## 1. Definition

Observer is a behavioral pattern that establishes a one-to-many notification
relationship: when a subject publishes a change or event, subscribed observers
receive notification without the subject depending on their concrete types.

## 2. The Problem It Solves

One state change may interest several independent consumers, such as a display,
alarm, recorder, and analytics component. Hardcoding all consumers into the producer
makes it responsible for unrelated behavior and difficult to extend.

The producer should describe what happened. Consumers should independently decide
what that event means for them, with subscription controlling who participates.

## 3. Understand the Mechanism

Observers register a callback or observer interface with the subject. The subject
retains the subscription and invokes it when publishing. Unsubscription ends that
relationship. Events may push data directly, or observers may query the subject
after receiving a change notification.

Delivery rules are part of the design: synchronous or queued, ordered or unordered,
and fail-fast or isolated when callbacks throw. Subscription changes during delivery
must also have defined semantics. An observer pattern diagram does not answer
these questions automatically.

## 4. Real-World Scenario

A building-monitoring system publishes room temperatures. A dashboard refreshes
its view, an alarm component evaluates thresholds, and a recorder stores history.
The sensor source does not implement any of those consumer responsibilities.

A slow recorder should not accidentally block safety-sensitive processing. A
production design may therefore use queues and separate execution contexts, with
explicit backpressure and delivery guarantees. That is additional infrastructure
beyond an in-process synchronous observer list.

## 5. Understand the C++ Example

Open [observer.cpp](../../../patterns/behavioral/observer.cpp).

`Sensor` stores callbacks under numeric tokens. `subscribe()` returns a token;
`unsubscribe()` removes it. `publish()` snapshots tokens before delivering values.

1. A display callback records readings 20 and 21.
2. A one-shot callback removes its own subscription after receiving 20.
3. The second publication skips that removed token.
4. Removing the display prevents it receiving 22.
5. Another callback adds a subscriber during publication; it receives only later events.

Each token is looked up immediately before invocation, so removed subscriptions
are skipped. The callable is copied before invoking it, protecting an executing
callback from self-removal. Consequently, mutable state captured by value changes
in that invocation's copy, not persistently in the stored callable. The example
captures externally owned state by reference and keeps it alive during publication.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** loosely coupled consumers, dynamic subscriptions, and reusable event
sources with no concrete listener knowledge.

**Drawbacks:** indirect control flow, dangling captures, reentrancy, callback errors,
and possible ownership cycles. This implementation is synchronous and not thread-safe.

Use a direct call when there is one fixed collaborator. Mediator coordinates a
workflow; Observer distributes notifications. A brokered publish/subscribe system
adds transport and delivery semantics beyond the local pattern.

## 7. Check Your Understanding

**Question:** What happens if one callback throws here?

**Answer:** The exception propagates and later callbacks in that publication may
not run. A system requiring listener isolation needs an explicit different policy.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).