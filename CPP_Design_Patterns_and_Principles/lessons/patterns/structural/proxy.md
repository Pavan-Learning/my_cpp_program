# Proxy

## 1. Definition

Proxy is a structural pattern in which a substitute object controls access to a
real subject while presenting the subject's interface. The proxy can decide when,
whether, or how the real operation occurs.

## 2. The Problem It Solves

A useful object may be expensive to construct, remote, access-restricted, or worth
caching. Clients should use a stable contract without each implementing loading,
authorization, communication, or cache logic themselves.

The design introduces an access boundary in front of the real subject. Unlike a
general convenience wrapper, preserving the subject's client-facing contract is
part of the intent.

## 3. Understand the Mechanism

The subject interface is shared by proxy and real subject. The client invokes the
proxy. The proxy applies its access policy and delegates when appropriate.

A virtual proxy delays expensive creation. A protection proxy checks authority.
A remote proxy communicates with another process. A caching proxy may answer
without invoking the real subject when its consistency rules permit it.

The same signature does not erase differences in behavior. A remote call still
has latency and partial-failure possibilities. A cached result may be stale. The
contract must disclose important guarantees instead of pretending the proxy is
operationally indistinguishable from a cheap local object.

## 4. Real-World Scenario

A document viewer contains hundreds of high-resolution images. Opening the
document creates inexpensive placeholders; decoding starts only when a page
actually needs an image. Each placeholder preserves the image display interface.

The viewer must decide what happens if decoding fails, whether concurrent requests
share initialization, and how decoded memory is released. The lazy proxy controls
access timing; cache eviction and asynchronous loading are additional policies.

## 5. Understand the C++ Example

Open [proxy.cpp](../../../patterns/structural/proxy.cpp).

`Image` declares `display()`. `RealImage` supplies the operation. `LazyImageProxy`
owns an initially empty `unique_ptr<RealImage>` and a creation count.

1. Constructing the proxy does not construct a real image; the count is zero.
2. The first `display()` sees the empty pointer and creates `RealImage`.
3. It increments the count and delegates, returning `display image`.
4. The second call finds the existing image and delegates without reconstruction.
5. The final check requires exactly one load, and output reports that count.

This demonstrates laziness and reuse, not real decoding. The proxy exclusively
owns its subject. Concurrent calls would race on the pointer and count unless
initialization and access were synchronized.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** centralizes access policy, avoids unnecessary eager work, and keeps
client usage stable while resource handling changes.

**Drawbacks:** hides costs if poorly documented, moves failures to first use,
complicates identity, and requires explicit caching or authorization semantics.

Use it when access mediation has real value. Direct ownership is simpler otherwise.
Decorator layers extra responsibilities; Adapter changes an interface. A proxy
may use those techniques too, but its primary purpose is controlling access.

## 7. Check Your Understanding

**Question:** Is every lazy proxy a Singleton?

**Answer:** No. Laziness controls *when* an object is created; Singleton restricts
*how many* instances are accessible. Two proxies can independently own two subjects.

See the [structural technical notes](../../../patterns/structural/README.md).