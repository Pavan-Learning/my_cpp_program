# Favor Composition Over Inheritance

## 1. Definition

Prefer building behavior by combining collaborating objects when the relationship
is “has-a” or “uses-a.” Use public inheritance when a derived object truly satisfies
the base object's behavioral contract and can substitute for it.

This is a preference against inheritance merely for code reuse, not a ban on inheritance.

## 2. The Problem It Solves

Inheriting a class to reuse a few methods also inherits its API, assumptions, and
often protected representation. The derived type may advertise operations that
make no sense for it. Multiple independent capabilities can produce deep or
combinatorial subclass hierarchies.

Composition lets an object use capabilities without pretending to be those capabilities.

## 3. Understand the Principle

A composed object delegates selected work to its members. It can expose only the
operations meaningful to its own role. Values, borrowed references, owning pointers,
and template parameters offer different lifetime and replacement choices.

Composition separates capability reuse from substitutability. A car uses an engine,
but callers should not treat the car as an engine. Conversely, a concrete renderer
can truthfully implement a renderer interface through inheritance.

Runtime replacement is optional. Fixed value members may be ideal for a simple
design. Adding polymorphic pointers to every member does not make composition better
unless the replacement requirement actually exists.

## 4. Real-World Scenario

A media player uses a decoder, audio output, and playlist model. Inheriting the
player from a decoder would expose low-level decoding operations as though the
player itself were a decoder. Composing those services preserves their roles.

Different output backends can be supplied through a stable contract if needed.
The player still owns orchestration and must define lifecycle and error handling
across those collaborators.

## 5. Understand the C++ Example

Open [composition.cpp](../../principles/composition.cpp).

`InspectionRobot` contains a `Motor` and `Camera` by value. It inherits from neither.

1. Constructing the robot constructs its member capabilities.
2. `inspect()` asks the motor to move and the camera for a capture description.
3. It combines their returned values as `moving: photo`.
4. The check verifies the combined behavior.

The members need no dynamic allocation and their lifetimes follow the robot.
The methods return pure strings; this is not hardware control or a guarantee about
sequencing physical movement and capture. Real ordered effects should be invoked
in explicit separate statements with appropriate failure handling.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** focused public APIs, independent capabilities, reduced inherited
assumptions, and straightforward member ownership.

**Drawbacks:** forwarding can be verbose; interchangeable components need contracts;
some frameworks deliberately require subclass extension.

Use inheritance for truthful behavioral subtypes. Use templates for fixed policy
composition or virtual interfaces for runtime replacement when justified.

## 7. Check Your Understanding

**Question:** Should an inspection robot inherit from `Camera` because it takes pictures?

**Answer:** Not for that reason. It has a camera capability. Inheritance would promise
that the whole robot can substitute for every valid camera use, which may be false.

See the [principles technical notes](../../principles/README.md).