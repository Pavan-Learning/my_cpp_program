# Factory Method

## 1. Definition

Factory Method is a creational design pattern in which a base creator defines an
operation for producing a product, while subclasses decide the concrete product
type. The creator's workflow uses the product through a common interface.

In plain language: **keep the work the same, but let a specialized creator choose
the tool used to do that work**. The pattern is about an overridable construction
decision, not simply putting `new` inside a function.

## 2. The Problem It Solves

Imagine a document-processing framework. Its workflow opens a document, validates
it, and displays it. If that workflow constructs a PDF document directly, supporting
another format requires changing the workflow or copying it into another class.
Repeated format-specific edits make shared behavior harder to maintain consistently.

There are two different responsibilities: deciding which product to create and
deciding how to use a product. Factory Method separates those responsibilities
without requiring a different workflow for every concrete product.

## 3. Understand the Mechanism

The **product interface** states what the workflow needs. **Concrete products**
implement it. The **creator** contains the workflow and the overridable creation
operation. **Concrete creators** supply that operation.

Control moves down into a specialized creator only for construction, then returns
to the shared workflow. The workflow does not inspect the product's concrete type.
This matters: replacing construction with a factory but retaining type checks
throughout the workflow has not removed the original dependency.

The variation point is product selection. Product behavior must still satisfy a
common contract. Factory Method cannot make incompatible products interchangeable
just because both pointers have the same base type.

## 4. Real-World Scenario

Consider a hypothetical desktop editor with PDF and spreadsheet editions. Both
editions follow an open/validate/display workflow. Each edition overrides a
document-creation operation to return its supported document implementation.
Adding a presentation edition adds a new document and creator; the shared opening
workflow need not change.

This is a useful fit when the application already extends a framework by
subclassing. If formats are selected dynamically from a filename, a registry of
constructor functions may be a simpler design. The scenario illustrates a possible
architecture, not a claim about a particular commercial editor.

## 5. Understand the C++ Example

Open [factory_method.cpp](../../../patterns/creational/factory_method.cpp).

| Code element | Role | Why it exists |
| --- | --- | --- |
| `Transport` | Product interface | Gives the workflow a common `deliver()` operation |
| `Truck`, `Ship` | Concrete products | Supply road and sea behavior |
| `Logistics` | Creator | Owns the shared `fulfill()` workflow |
| `create_transport()` | Factory method | Isolates the variable creation decision |
| `RoadLogistics`, `SeaLogistics` | Concrete creators | Select the concrete transport |

1. `main()` constructs a road creator and a sea creator.
2. `road.fulfill()` enters the inherited workflow, not a road-specific copy.
3. Its call to `create_transport()` dispatches to the road override.
4. That override creates a truck and returns `unique_ptr<Transport>`.
5. The workflow calls `deliver()` through the interface and receives `road`.
6. It returns `Deliver by road`; the local owning pointer destroys the truck.
7. The sea creator follows the same steps and produces `Deliver by sea`.

The key evidence is **different products, unchanged workflow**. The virtual
destructor makes deletion through `Transport` correct. `unique_ptr` expresses one
owner; it is an ownership choice, not the definition of Factory Method.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** separates construction from shared work, supports focused extension,
and prevents concrete constructor knowledge from spreading through the workflow.

**Drawbacks:** introduces creator subclasses and can produce unnecessary parallel
hierarchies. Every product must obey the shared contract. Registration or selection
code still has to choose a creator somewhere.

**Use it when** a reusable creator workflow genuinely needs subclass-controlled
construction. **Avoid it when** a direct constructor or supplied callable is enough.
A simple factory switches inside one creation function; Abstract Factory selects
a family of related products. Neither is synonymous with Factory Method.

## 7. Check Your Understanding

**Question:** Does adding a static `make_truck()` function implement this pattern?

**Answer:** Not by itself. It is a named construction function. The GoF Factory
Method collaboration includes an overridable creation decision used by a creator.
Look at who chooses the product and who runs the common workflow, not the name.

For additional diagrams, error cases, and variants, see the
[creational technical notes](../../../patterns/creational/README.md#1-factory-method).