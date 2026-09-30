# Chain of Responsibility

## 1. Definition

Chain of Responsibility is a behavioral pattern that passes a request through a
sequence of handlers. Each handler decides whether to handle, reject, transform,
or forward the request according to the chain's contract. The sender does not
need to know which concrete handler ultimately resolves it.

## 2. The Problem It Solves

A request may require several independently changing checks or may need escalation
until a capable receiver is found. Hardcoding all decisions in the sender couples
it to every processing detail. Copying those decisions across senders causes drift.

The desired flexibility is the sequence of processing responsibilities. Moving
that sequence into composable handlers lets senders focus on submitting requests.

## 3. Understand the Mechanism

Each handler knows its successor rather than the entire application. It performs
its local decision and either returns a result or delegates onward. Assembly code
selects the handlers and their order.

There are important variants. In a first-capable-handler chain, one handler consumes
the request. In a filtering chain, every successful filter forwards it and any
filter may reject it. End-of-chain behavior must be explicit: success, unhandled,
or failure are different contracts.

Ordering is observable. A validation handler may protect later steps from invalid
input. Authentication before account-specific processing may prevent disclosure.
The pattern makes ordering configurable, not automatically correct.

## 4. Real-World Scenario

An expense request is considered by a team lead, then a department manager, then
finance. Each role can approve up to a limit; larger requests move onward. The
submission form does not encode every approval limit.

That is a first-capable-handler chain. A web request pipeline applying authentication,
quota, and input checks is the filtering variation. Do not assume one variation's
terminal behavior is suitable for the other, especially when approval is sensitive.

## 5. Understand the C++ Example

Open [chain_of_responsibility.cpp](../../../patterns/behavioral/chain_of_responsibility.cpp).

`Handler` owns its successor. `Authentication` and `Quota` inspect different fields
of a request. `then()` assembles the filtering chain.

1. `main()` creates authentication and gives it ownership of a quota handler.
2. An unauthenticated request returns `unauthorized` immediately.
3. An authenticated request is delegated to quota.
4. Exceeded quota returns `quota exceeded`.
5. A request passing both checks reaches the base terminal result, `accepted`.
6. Checks verify all three paths, including the first rejection taking precedence.

The booleans simulate decisions; they do not implement authentication. The sample
accepts at the end. A production security chain may instead need a deny-by-default
terminal policy and verification that mandatory handlers are present.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** independent handlers, reusable processing steps, configurable order,
and reduced sender knowledge.

**Drawbacks:** requests can go unhandled, order is easy to misconfigure, and long
chains complicate debugging. Repeated forwarding can hide control flow.

Use it when handlers genuinely vary. A fixed sequence of a few ordinary functions
may be clearer. Observer broadcasts an event; a chain normally controls onward
processing and can stop it.

## 7. Check Your Understanding

**Question:** Does every handler always run?

**Answer:** No. The contract determines forwarding. Here a rejection returns before
later handlers run, which is why the first failing condition controls the result.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).