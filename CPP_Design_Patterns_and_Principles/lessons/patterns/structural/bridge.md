# Bridge

## 1. Definition

Bridge is a structural pattern that separates a high-level abstraction from its
implementation so the two can vary independently. The abstraction delegates
implementation work through a separate interface instead of combining every
abstraction/implementation pair into one inheritance hierarchy.

## 2. The Problem It Solves

Suppose both device types and remote-control features vary. Separate classes for
every combination grow rapidly: basic television remote, advanced television
remote, basic radio remote, advanced radio remote, and so on.

These are two axes of change. Treating every pair as a unique subtype duplicates
logic and makes adding either axis expensive. The design should represent each
axis directly, then connect instances through composition.

## 3. Understand the Mechanism

The abstraction supplies the client's high-level operations. The implementor
interface supplies the underlying capabilities needed to perform them. Refined
abstractions extend high-level behavior; concrete implementors provide backends.

The abstraction holds an implementor reference or owning handle. It delegates
primitive work while retaining responsibility for its own policy. The client can
combine an abstraction with a suitable implementation at construction time.

Independence depends on a stable implementor vocabulary. If every new high-level
feature needs a new backend method, both sides still change. Bridge reduces
unnecessary coupling; it does not guarantee arbitrary future changes are free.

## 4. Real-World Scenario

A remote-control product line supports televisions and radios. Basic remotes offer
power and volume; advanced remotes add a mute operation implemented through the
same device contract. Remote features belong on one side, device-specific protocols
on the other.

Adding another device then need not duplicate every remote feature. The common
contract must still make sense: a device without volume cannot honestly satisfy a
volume-dependent abstraction merely because it fits a class diagram.

## 5. Understand the C++ Example

Open [bridge.cpp](../../../patterns/structural/bridge.cpp).

`Shape` is the abstraction and `Circle` refines it. `Renderer` is the implementor
interface. `VectorRenderer` and `RasterRenderer` supply two implementations.

1. `main()` creates both renderers.
2. Each circle receives a renderer reference and radius 4.
3. `Circle::draw()` delegates to its renderer's `circle(radius_)` operation.
4. The vector renderer returns `vector circle 4`.
5. The raster renderer returns `raster circle 4`.
6. Both checks verify backend selection without separate circle/backend subclasses.

Renderers outlive circles because circles borrow them. The program returns strings,
not pixels. Adding a rectangle would require new renderer support in this small
interface; a richer primitive drawing vocabulary could reduce such changes.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** separates independent variations, reduces combination subclasses,
and supports backend replacement and independent tests.

**Drawbacks:** requires another interface and indirection. A poorly chosen backend
contract can become a bottleneck for both sides.

Use it when two real dimensions vary. Adapter usually reconciles already-existing
interfaces. Strategy usually substitutes an algorithm. Pimpl hides representation,
but need not model two independently extensible hierarchies.

## 7. Check Your Understanding

**Question:** Does replacing an `if` with a renderer pointer prove Bridge is useful?

**Answer:** No. Identify the independent axes and expected changes. If there is
only one stable shape and one backend, the added structure may have no practical value.

See the [structural technical notes](../../../patterns/structural/README.md).