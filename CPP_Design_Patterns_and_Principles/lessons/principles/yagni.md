# YAGNI: You Aren't Gonna Need It

## 1. Definition

YAGNI advises against implementing speculative capabilities before there is a
justified requirement for them. Build what is needed, while preserving ordinary
clarity and changeability, rather than paying now for imagined future features.

## 2. The Problem It Solves

Predicted requirements are often wrong. A generic framework built for them creates
code to test, document, secure, and maintain before it creates useful value. The
chosen abstraction may later make the actual requirement harder to implement.

The cost is not only development time. Unused features enlarge the system's state
space and distract from learning what real users need.

## 3. Understand the Principle

Distinguish a foreseeable obligation from unsupported speculation. Security,
correctness, data durability, and compatibility can be current requirements even
when their consequences arrive later. YAGNI does not excuse ignoring them.

Prefer reversible decisions when uncertainty is high. Clear modules, tests, and
ordinary ownership make later changes easier without building every possible
extension point in advance.

When a real second requirement appears, reassess. Avoid both extremes: refusing
to generalize forever and generalizing for an unlimited imaginary product roadmap.

## 4. Real-World Scenario

An internal stock tool must display a text availability report. Building PDF themes,
scheduled email, plugin discovery, and a distributed report queue would be
speculative if nobody needs those capabilities.

A later contractual requirement for monthly PDF exports changes the evidence.
Implementing that capability then is not inconsistency; it is responding to a
real need. A simple second formatter may still be enough without a plugin platform.

## 5. Understand the C++ Example

Open [yagni.cpp](../../principles/yagni.cpp).

`stock_report()` directly formats the available count. Its small size is intentional:
the example demonstrates a design decision not to introduce an unnecessary pattern.

1. The function receives a count.
2. It converts that count to text and prefixes `Available: `.
3. Tests verify both zero and 12.
4. The program prints `Available: 12`.

There is no format registry or base report hierarchy because none is required.
The function formats data; it does not own inventory validity. If negative counts
are forbidden, the relevant domain or input boundary should enforce that rule.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** smaller maintenance burden, faster feedback, and fewer guessed
abstractions or unused failure paths.

**Drawbacks:** misapplication can ignore expensive-to-reverse architectural
constraints. Some planning is needed for durable data, public APIs, and safety.

Use evidence and the cost of reversal to decide what must be designed now. OCP is
useful for known variation; it does not require extension points for every imagined one.

## 7. Check Your Understanding

**Question:** Should tests be omitted because future bugs are uncertain?

**Answer:** No. Tests support the current behavior and safe change. YAGNI targets
speculative functionality, not the engineering needed to deliver today's contract.

See the [principles technical notes](../../principles/README.md).