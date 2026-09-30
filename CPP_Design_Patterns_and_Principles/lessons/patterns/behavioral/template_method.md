# Template Method

## 1. Definition

Template Method is a behavioral pattern that defines an algorithm's overall
structure in a base class while allowing subclasses to specialize selected steps.
The skeleton remains shared; the permitted steps vary.

The word “template” refers to an algorithmic template, not necessarily a C++
`template` declaration.

## 2. The Problem It Solves

Several operations may share a required sequence while differing in details.
Copying that sequence into each implementation leads to inconsistent validation,
ordering, and cleanup. Letting every implementation replace the whole algorithm
makes shared rules difficult to enforce.

The base class should own the sequence and expose deliberate customization points,
rather than expecting each subclass to remember every common rule.

## 3. Understand the Mechanism

A template method calls primitive operations in a defined order. Required steps
can be abstract; optional hooks can have default implementations. Subclasses
override the variable pieces, while clients invoke the shared top-level operation.

The base class is responsible for its promises, including validation before hooks.
Subclass hooks are responsible for their own narrower contracts. Too many hooks
can make the skeleton as unpredictable as unrestricted overriding.

Mandatory resource cleanup should rely on RAII, not a final hook that is skipped
when an earlier step throws. Inheritance also creates lifecycle constraints: base
constructors do not invoke fully constructed derived behavior through virtual calls.

## 4. Real-World Scenario

A data-import framework follows validate-source, read-records, normalize, and store.
CSV and binary importers specialize how records are read, while the framework
retains shared validation and storage orchestration.

This fits a framework deliberately extended through subclasses. If users need to
combine independently chosen readers, normalizers, and writers at runtime,
composition with strategies may fit better than another subclass for each combination.

## 5. Understand the C++ Example

Open [template_method.cpp](../../../patterns/behavioral/template_method.cpp).

`Report::generate()` is the nonvirtual skeleton. `header()` and `body()` are required
hooks; `footer()` has a default newline. CSV and HTML specialize formatting.

1. `generate(42)` first checks that the total is nonnegative.
2. It calls header, body, and footer in separate statements, guaranteeing that order.
3. CSV produces `total\n42\n` using the default footer.
4. HTML supplies its own footer and produces `<p>42</p>\n`.
5. `TracedReport` records `HBF`, verifying actual hook order.
6. A negative input is rejected before any hook runs.

Separate statements matter because joining multiple hook calls in one expression
would not provide the same portable evaluation-order guarantee in C++17.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** shared sequencing and validation, focused subclass extension, and
reduced duplicated algorithm structure.

**Drawbacks:** inheritance coupling, fragile hook contracts, and limited runtime
recombination. A base change can affect every subclass.

Use it for a genuinely stable skeleton. Strategy composes interchangeable policies;
the nonvirtual-interface idiom similarly guards a public operation around virtual
implementation hooks.

## 7. Check Your Understanding

**Question:** Can a footer hook guarantee cleanup after a body exception?

**Answer:** No. Normal execution never reaches the footer in that case. Put
resource cleanup in RAII owners or another deliberate exception-safe mechanism.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).