# Mediator

## 1. Definition

Mediator is a behavioral pattern that centralizes interaction rules among a group
of objects. Colleague objects communicate through the mediator rather than encoding
detailed knowledge of one another's behavior.

## 2. The Problem It Solves

When many components directly update one another, coordination becomes scattered.
A change in one component can trigger several others, which call back into the
first. Understanding the workflow requires tracing a web of relationships.

The goal is not to eliminate collaboration. It is to give the collaboration rules
a clear owner while keeping individual components focused on their local behavior.

## 3. Understand the Mechanism

Colleagues report events through a mediator interface. The concrete mediator
interprets those events in context and decides what should happen next. Colleagues
do not need to know every other colleague's type or state layout.

This moves coupling toward the mediator, which deliberately knows the workflow.
That concentration is beneficial only while the mediator has a coherent scope.
A single mediator controlling unrelated workflows becomes another god object.

Notifications can cause feedback loops. If reacting to one change modifies another
component that immediately notifies again, batching or explicit reentrancy policies
may be needed.

## 4. Real-World Scenario

A flight-search form has destination, travel dates, passenger count, and a search
button. Changes can affect validation, available options, and button state. A form
coordinator recomputes those relationships instead of teaching each field about
every other field.

This is coordination, not merely broadcasting events. An Observer can deliver field
notifications to the coordinator; the Mediator decides how the form should respond.
Server-side validation remains necessary even if the form enables the button.

## 5. Understand the C++ Example

Open [mediator.cpp](../../../patterns/behavioral/mediator.cpp).

`TextField` stores text and a mediator reference. `SignInForm` owns username and
password fields and implements `changed()`.

1. Both fields begin empty, so submission is disabled.
2. Setting username updates that field, then notifies the form.
3. The form checks both fields and stays disabled because password is empty.
4. Setting password notifies the form again, enabling submission.
5. Clearing username causes another recomputation and disables submission.

All four states are checked. Fields only know `Mediator`; the form owns the
cross-field rule. Copying the form is disabled because copied fields would otherwise
retain references to the original mediator. Field constructors store references
without invoking callbacks on a partially constructed form.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** localized workflow rules, reusable colleagues, and fewer direct
colleague dependencies.

**Drawbacks:** a central complexity hotspot, possible event loops, and another
coordination layer for simple cases.

Use it when interactions are complex enough to need a dedicated owner. A small
controller function can be sufficient. Facade offers a simplified external entry
point; Mediator manages relationships among participating colleagues.

## 7. Check Your Understanding

**Question:** Does the mediator authenticate the user in this program?

**Answer:** No. It checks only whether two fields are nonempty. Coordination of UI
state is not credential verification or a security boundary.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).