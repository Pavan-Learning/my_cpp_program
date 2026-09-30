# Single Responsibility Principle (SRP)

## 1. Definition

The Single Responsibility Principle says that a module should have one coherent
reason to change: its responsibilities should serve a closely related stakeholder
or business concern. Behavior that changes for unrelated reasons should not be
forced into the same module.

It does **not** mean one method per class. Several methods may cooperate to maintain
one responsibility, while one large method may mix many responsibilities.

## 2. The Problem It Solves

When business calculations, storage, presentation, and communication live together,
a change requested by one stakeholder risks behavior owned by another. Tests become
entangled, and even a formatting change may require constructing database services.

The issue is change coupling: unrelated work has been made to move together. SRP
asks which changes belong together and gives each coherent concern an appropriate home.

## 3. Understand the Principle

Identify responsibilities from the domain and its change pressures, not from
method count. Payroll calculation and tax-report layout may use the same data but
follow different rules and release schedules. Sharing data does not automatically
make them one responsibility.

Keep invariant-maintaining behavior with the object that owns the invariant.
Separating everything into independent functions that mutate public fields merely
scatters one responsibility. A coordinator can legitimately orchestrate one use
case; coordination itself may be a coherent responsibility.

The boundary is a judgment supported by actual changes, testing difficulty, and
stakeholder needs. SRP does not prescribe a universal class size.

## 4. Real-World Scenario

A payroll service computes wages, stores payroll records, and creates a bank export.
Compensation rules come from payroll specialists, storage changes from infrastructure,
and export format changes from the bank integration.

Separating calculation from persistence and export allows each to change and be
tested independently. A payroll-run application service can still coordinate the
whole process. Splitting into components must not remove the required transactional
and audit behavior of the overall payroll run.

## 5. Understand the C++ Example

Open [srp.cpp](../../solid/srp.cpp).

The `before::Invoice` calculates a total and formats CSV. The `after::Invoice`
only calculates; `CsvInvoiceFormatter` owns the formatting policy.

1. Both invoice versions receive amounts 100 and 250 cents.
2. The after invoice totals them to 350 without producing any output format.
3. The formatter queries that total and creates `total_cents\n350`.
4. A comparison proves that the refactoring preserved the before version's output.
5. A separate check confirms an empty invoice totals zero.

The important improvement is independent reasons to change, not extra classes.
The formatter could be a free function. No virtual interface is necessary merely
to demonstrate SRP. The sample uses small values and does not model complete accounting.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** localized changes, focused tests, clearer ownership, and fewer
unrelated modifications in the same component.

**Drawbacks:** excessive splitting introduces navigation and wiring overhead.
Choosing boundaries prematurely can separate work that really belongs together.

Apply SRP where concerns actually differ. Keep small cohesive value types intact.
Separation of concerns is broader; SRP specifically asks whether a module has
unrelated reasons or stakeholder pressures to change.

## 7. Check Your Understanding

**Question:** Does an invoice class violate SRP because it has `add_item()`,
`remove_item()`, and `total()`?

**Answer:** Not necessarily. Those methods can all maintain the invoice's coherent
domain state. Formatting, sending email, or managing database connections may be
separate responsibilities because they change for different reasons.

See the [SOLID technical notes](../../solid/README.md).