# Interpreter

## 1. Definition

Interpreter is a behavioral pattern that represents the grammar of a small language
as composable expressions and defines how those expressions are evaluated within
a context. Each expression type implements the meaning of one grammar construct.

## 2. The Problem It Solves

Applications sometimes need users or configuration to express combinations of rules.
Hardcoding every combination in application branches is inflexible. Treating input
as unrestricted host-language code is much more powerful and risky than necessary.

A small, deliberately limited language can express the required rules while keeping
its syntax and meaning under application control.

## 3. Understand the Mechanism

Terminal expressions represent basic values or variable references. Nonterminal
expressions combine other expressions, forming a syntax tree. A context supplies
runtime values. Evaluation recursively asks each node to interpret itself.

Parsing and interpretation are different responsibilities. A parser turns text
into the tree; an interpreter gives that tree meaning. Operator precedence belongs
to parsing, while short-circuit behavior and missing-value policy belong to semantics.
The GoF pattern does not supply a parser automatically.

## 4. Real-World Scenario

A feature-management system allows a rule such as “signed-in AND subscribed.” The
same rule can be evaluated for different users by supplying different context values.
Adding a small OR operator permits alternative qualifying conditions.

Real deployment requires limits on expression size, validation, error reporting,
and a decision about unknown attributes. For a large language, a proven parsing
and evaluation library is usually preferable to a growing hand-built class hierarchy.

## 5. Understand the C++ Example

Open [interpreter.cpp](../../../patterns/behavioral/interpreter.cpp).

`Expression` declares evaluation. `Variable` reads a named boolean from `Context`.
`And` owns two child expressions and uses short-circuit logical AND.

1. `main()` constructs a tree for `signed_in AND paid` directly, without parsing text.
2. When both context entries are true, evaluation returns true.
3. When `paid` is false, it returns false.
4. If `signed_in` is false, the right child is not evaluated.
5. Therefore a missing `paid` entry does not fail in that short-circuited case.
6. If the left side is true and `paid` is missing, `map::at()` throws; a check
   verifies this explicit missing-variable policy.

Child expressions are exclusively owned, so the tree cleans up automatically.
Context is borrowed only during evaluation. Missing values are errors when evaluated,
not silently treated as false.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** composable rules, visible semantics, reusable expression trees, and
focused operator tests.

**Drawbacks:** many classes for large grammars, allocation and recursion costs,
and additional work for parsing, diagnostics, and resource limits.

Use it for small stable languages. For a fixed handful of rules, ordinary predicates
may suffice. Variant-based syntax trees can replace virtual nodes; Visitor can add
analysis or formatting operations to an existing tree.

## 7. Check Your Understanding

**Question:** Does adding an `Or` class teach the program to parse the word `OR`?

**Answer:** No. It supplies evaluation semantics for an OR node. A separate parser
must recognize text and construct that node with the correct operands and precedence.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).