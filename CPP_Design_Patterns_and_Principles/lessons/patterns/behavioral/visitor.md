# Visitor

## 1. Definition

Visitor is a way to add new work to a group of object types without putting every
new operation inside those classes. For example, circles and rectangles can stay
focused on representing shapes, while separate visitors calculate area or count shapes.

**In simple words:** keep the objects in place and bring a separate worker to them.
Each worker knows what to do with each supported kind of object.

## 2. The Problem It Solves

A stable object model may need many changing operations: export, validation,
measurement, printing, and analysis. Adding all operations to every element class
mixes unrelated concerns into the model and repeatedly modifies its interface.

Visitor moves one operation's behavior into a separate object, provided the set
of element types is sufficiently stable to make that tradeoff worthwhile.

## 3. Understand the Mechanism

The visitor interface has a `visit()` function for each supported object type.
These functions have the same name but different parameter types; this is called
**overloading**. A circle's `accept()` calls the circle version, while a rectangle's
`accept()` calls the rectangle version.

There are two choices: which object is being visited, and which worker is visiting
it. For example, a circle visited by an area worker runs the circle-area calculation.
The same circle visited by a counting worker increments a count. This arrangement
is commonly called **double dispatch**.

Why is `accept()` needed? A caller holding a `Shape*` does not give C++ enough
information to choose a circle-specific overload directly. Calling `accept()` first
reaches the circle's own code. There, `*this` is known to be a circle, so the correct
`visit()` overload can be selected.

The main tradeoff is easy to remember: adding a new worker is convenient, but adding
a new object type means teaching every existing worker how to handle it.

## 4. Real-World Scenario

A compiler has expression-node types for literals, additions, and calls. Separate
passes perform type checking, formatting, optimization analysis, or code generation.
A visitor can collect each pass's type-specific logic outside the node classes.

This works best when node types evolve less often than passes. If the language adds
new syntax frequently, updating every visitor becomes significant maintenance.
Traversal and visitation are also separate: someone still decides how to walk children.

## 5. Understand the C++ Example

Open [visitor.cpp](../../../patterns/behavioral/visitor.cpp).

`Shape` declares `accept()`. Circle and rectangle implement it. `Area` and `Count`
are visitors over the same shape set.

1. A vector owns a radius-2 circle and a 3-by-4 rectangle through shape pointers.
2. Calling `accept(area)` on the circle dispatches to `Circle::accept()`.
3. Inside it, `*this` is a `const Circle&`, selecting the circle overload.
4. The area visitor adds `pi * 2 * 2`; the rectangle overload adds 12.
5. A separate count visitor increments once for each shape.
6. Checks verify area within tolerance and count 2; output reports two visited shapes.

Visitors accumulate state, so reusing the same instance for another traversal adds
again. The vector owns elements; visitors borrow each element during its call.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** new operations without editing element classes, cohesive operation
logic, and explicit type-specific behavior with accumulation.

**Drawbacks:** expensive new element types, increased knowledge of concrete types,
and possible pressure to expose internal data to visitors.

Use ordinary virtual methods when element types vary more than operations.
`std::variant` with `std::visit` is a value-oriented alternative for a closed type
set. Iterator supplies traversal; Visitor supplies operations on visited elements.

## 7. Check Your Understanding

**Question:** Which is cheaper here: adding perimeter calculation or a triangle?

**Answer:** Perimeter is a new visitor implementing existing overloads. A triangle
requires extending the visitor interface and updating every existing visitor.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).