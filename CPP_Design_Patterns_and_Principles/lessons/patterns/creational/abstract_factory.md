# Abstract Factory

## 1. Definition

Abstract Factory is a creational pattern that provides an interface for creating
families of related or compatible objects without exposing their concrete classes
to the client. **Choose a family once, then obtain its matching products through
one factory.** A family contains different product roles, not merely many copies
of one product.

## 2. The Problem It Solves

A system needs several collaborating objects: a connection, a command, and a
transaction, for example. Selecting each implementation independently can produce
an invalid combination. Scattering provider checks throughout the system also
makes switching provider difficult and error-prone.

The design must separate what the application needs from which provider supplies
the complete group. Centralizing individual constructors is insufficient if the
client must still remember which implementations belong together.

## 3. Understand the Mechanism

There are two dimensions. Product categories describe different roles. Product
families provide compatible implementations of those roles. An abstract factory
has an operation for each category, and each concrete factory supplies one family.

The client receives a factory and asks it for products through abstract interfaces.
It never needs to assemble concrete-class combinations. Adding a family usually
leaves the client unchanged; adding a category changes every factory. That asymmetry
is the central extension tradeoff, not an incidental implementation detail.

Compatibility is a contract, not a magical property of the pattern. Clients can
still mix manually constructed objects. Stronger requirements may need typed
bundles, provider identity checks, or a session that owns all related products.

## 4. Real-World Scenario

Imagine a database application supporting two database engines. Each provider
supplies a connection, transaction, and command implementation. Selecting the
provider factory ensures routine creation uses one engine's family.

This helps avoid passing a transaction from one driver into a command from another.
However, products must often also belong to the same *session*, not just the same
provider. A production factory would preserve that relationship explicitly.
Different engines may have different transaction semantics, so the common contract
must describe the behavior the application actually relies on.

## 5. Understand the C++ Example

Open [abstract_factory.cpp](../../../patterns/creational/abstract_factory.cpp).

`Button` and `Checkbox` are product categories. Light and dark are the families.
`WidgetFactory` declares `button()` and `checkbox()`; each concrete factory returns
its matching implementations. `draw_form()` is the client.

1. `draw_form()` receives a factory reference, without learning its concrete type.
2. It requests a button and checkbox from that factory.
3. With `DarkFactory`, these are `DarkButton` and `DarkCheckbox`.
4. Calls to their `draw()` methods yield `dark button` and `dark checkbox`.
5. Combining those values yields `dark button + dark checkbox`.
6. The light-family check repeats the same client operation with another factory.

Each product is returned in a `unique_ptr`, so temporary products are destroyed
after use. These products have no session relationship; matching appearance is the
sample's compatibility requirement. The test verifies both family outputs, not a
universal compile-time ban on mixed products.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** centralizes family selection, keeps concrete types outside clients,
and makes provider-family replacement straightforward.

**Drawbacks:** creates many classes and makes new product categories expensive.
A weak common contract can conceal important provider differences.

Use it for genuine families of collaborating products. For one product, a factory
function or Factory Method may suffice. Builder assembles a product step by step;
it addresses a different problem and can be used alongside an Abstract Factory.

## 7. Check Your Understanding

**Question:** Why is adding a theme easier than adding a slider category?

**Answer:** A theme implements the existing factory operations. A slider adds a
new operation to the factory contract, so every existing family must implement it.
The design is open along the family dimension, not every possible dimension.

See the [creational technical notes](../../../patterns/creational/README.md).