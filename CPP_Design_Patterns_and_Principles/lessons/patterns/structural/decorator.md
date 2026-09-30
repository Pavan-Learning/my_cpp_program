# Decorator

## 1. Definition

Decorator is a structural pattern that adds responsibilities to an individual
object by wrapping it in another object implementing the same interface. Wrappers
can be combined without creating a subclass for every feature combination.

## 2. The Problem It Solves

Suppose data output may need compression, encryption, checksumming, or combinations
of those features. A subclass for every combination grows quickly, while putting
every optional behavior in one class produces many flags and intertwined branches.

The desired variation is a set of composable responsibilities around an existing
operation. Each responsibility should be understandable independently.

## 3. Understand the Mechanism

A component interface describes the operation. A concrete component supplies the
base behavior. A decorator holds another component and implements the same
interface, doing work before, after, or around delegation.

Because a decorator is itself a component, another decorator can wrap it. Calls
flow inward, while returned results can be transformed outward. The outermost
object is the one the client sees.

Composition order is part of semantics. Encryption followed by compression differs
from compression followed by encryption. Interchangeable interfaces do not imply
commutative operations or unrestricted wrapper ordering.

## 4. Real-World Scenario

A backup application writes data through a checksum stage, a compression stage,
and an encryption stage before the final storage writer. Each stage handles one
responsibility, and the product can assemble supported configurations.

Real streams also need coordinated flush, close, and error behavior. A wrapper
that forwards writes but forgets finalization can corrupt output. Decorator makes
layering possible; it does not automatically preserve the full stream contract.

## 5. Understand the C++ Example

Open [decorator.cpp](../../../patterns/structural/decorator.cpp).

`Beverage` exposes `cost()` and `description()`. `Coffee` supplies the base value.
`BeverageDecorator` owns an inner beverage. `Milk` and `Cinnamon` extend its results.

1. Coffee begins at 200 cents.
2. Moving it into `Milk` creates a wrapper whose price is inner price plus 50.
3. Moving that wrapper into `Cinnamon` creates the outer layer, adding 20.
4. Calling `cost()` goes through cinnamon, milk, then coffee.
5. Returning outward computes 200 + 50 + 20 = 270.
6. Descriptions follow the same chain: `coffee, milk, cinnamon`.

`unique_ptr` transfers exclusive ownership at each step. Destroying the outermost
wrapper destroys the complete chain. Checks verify the base price, combined price,
and description order. Null wrapped components are rejected.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** runtime combinations, reusable individual features, and no subclass
for every combination.

**Drawbacks:** many objects, order-sensitive behavior, forwarding boilerplate, and
difficult inspection or removal of a middle layer. Some combinations should be
forbidden by configuration validation.

Use it for independent layers around a stable contract. Data-driven options may be
simpler for trivial arithmetic features. Proxy controls access; Decorator primarily
adds responsibilities, even though both can wrap the same interface.

## 7. Check Your Understanding

**Question:** Does wrapping twice automatically prevent duplicate features?

**Answer:** No. Two milk wrappers add two charges. If repetition is invalid, the
application must enforce that rule; the structural pattern deliberately permits layers.

See the [structural technical notes](../../../patterns/structural/README.md).