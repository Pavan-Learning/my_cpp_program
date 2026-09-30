# Composite

## 1. Definition

Composite is a structural pattern that represents part-whole hierarchies as trees
and lets clients use individual objects and groups through a common interface.
A group implements an operation by applying it to its children, which may themselves
be groups.

## 2. The Problem It Solves

A graphics editor contains individual objects and groups of objects. A group can
contain nested groups. If every client must distinguish each possible level, moving,
drawing, and calculating bounds require repeated type checks and recursion logic.

The client should ask a node to perform the meaningful operation. The node decides
whether to act directly or delegate recursively.

## 3. Understand the Mechanism

A component defines operations common to all nodes. A leaf performs the operation
directly. A composite stores children and combines or forwards their results.
Recursion terminates at leaves, so arbitrary nesting follows one simple rule.

The common interface should contain operations meaningful for both kinds. Child
insertion can stay on the composite rather than forcing leaves to implement a
meaningless `add()` method. This trades uniform mutation syntax for a safer API.

Ownership is separate from traversal. A tree often has one owner per child. Shared
nodes produce a graph, requiring explicit rules for cycles and double-counting.

## 4. Real-World Scenario

In a slide editor, a user groups a title, a chart, and a nested legend group.
Dragging the outer group moves all descendants. The editor invokes movement on
one component rather than manually enumerating every object type.

Production implementations must define coordinate systems, clipping, event routing,
and whether group transforms are stored or applied to children. Composite supplies
the recursive organization, not those geometry policies.

## 5. Understand the C++ Example

Open [composite.cpp](../../../patterns/structural/composite.cpp).

`Node` defines `size()`. A `File` returns its byte count. A `Directory` owns
`unique_ptr<Node>` children and adds their sizes.

1. An empty root reports zero.
2. A nested directory receives a 20-byte file.
3. The root receives a 10-byte file and ownership of the nested directory.
4. `root.size()` asks the file for 10 and the nested directory for its own total.
5. The nested directory asks its file for 20; the root adds 10 and 20.
6. A check verifies 30; another verifies null insertion is rejected without mutation.

The same `size()` interface works at every level. Exclusive child ownership makes
destruction recursive and automatic. Small `int` totals are sufficient for the
demonstration, but real large trees need overflow and depth policies.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** uniform recursive operations, fewer client type checks, and a natural
model for trees such as menus, scenes, and document structures.

**Drawbacks:** recursion can exhaust the stack, common interfaces can become too
broad, and shared/cyclic graphs require more than simple tree ownership.

Use it for genuine hierarchical composition. A flat list or plain data tree may
be simpler when polymorphic node behavior is unnecessary. Visitor can add new
operations to a stable set of node types without changing the tree structure.

## 7. Check Your Understanding

**Question:** Must clients know a node is a directory before asking its size?

**Answer:** No. The common operation hides that distinction. They need directory
capability only for directory-specific behavior such as adding children.

See the [structural technical notes](../../../patterns/structural/README.md).