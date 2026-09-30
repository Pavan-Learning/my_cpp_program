# Value Semantics and the Rule of Zero

## 1. Definition

Value semantics means an object behaves like an independent piece of data:
copying produces an equivalent value whose later changes do not unexpectedly alter
the original. The Rule of Zero recommends composing resource-managing members so
a higher-level class needs no custom copy, move, assignment, or destructor functions.

## 2. The Problem It Solves

Hand-managed allocations make copies and destruction difficult to implement
correctly. Default copying of an owning raw pointer duplicates an address rather
than the resource, leading to shared mutation, double deletion, or leaks.

Higher-level domain types should not repeat resource bookkeeping when suitable
standard members already provide it.

## 3. Understand the Principle

Use members with the intended semantics: strings and vectors own their elements
as values; unique pointers represent exclusive ownership; shared pointers share
their pointees. Compiler-generated operations compose those member behaviors.

Rule of Zero does not automatically prove independent value semantics. A generated
copy of a `shared_ptr` shares the object. Choose members according to the domain's
copy meaning, not merely to avoid writing special functions.

Moves transfer state efficiently where supported. A moved-from standard-library
object remains valid but often has unspecified contents. Do not assume it is empty
unless the specific type's contract says so.

## 4. Real-World Scenario

A user edits a draft copy of application preferences. Changes to the draft should
not affect the active preferences until the user applies them. Independent values
make cancel behavior straightforward: discard the draft.

A shared mutable settings pointer would instead expose edits immediately. Services
and identity-bearing objects may deliberately use reference semantics, so copying
is not the right abstraction for every domain object.

## 5. Understand the C++ Example

Open [value_semantics.cpp](../../principles/value_semantics.cpp).

`Notebook` stores `vector<string>` and declares no special resource-management
functions. Its generated copy and move operations use the members' semantics.

1. The original notebook receives `first`.
2. Copy construction gives a second notebook equivalent contents.
3. Adding `second` to the copy leaves the original size at one.
4. Moving the copy transfers its data into another notebook with two notes.
5. The moved-from source is assigned a fresh notebook before being reused.
6. Static checks verify copy construction and nonthrowing move construction;
   runtime checks verify independence and destination contents.

The test intentionally does not assume a moved-from vector is empty. It checks
what is guaranteed and reinitializes before relying on new contents.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** predictable copies, automatic cleanup, less special-function code,
and efficient moves from standard owning members.

**Drawbacks:** deep copies can cost memory and time; identity-bearing or
self-referential objects need different copy policies.

For a raw-resource owner, consider all five special operations together or disable
copying. Keep that complexity localized, then let enclosing classes return to
Rule-of-Zero composition.

## 7. Check Your Understanding

**Question:** Would adding an owning raw `char*` preserve the current safe copying?

**Answer:** No. The generated copy would copy the address. Prefer `string` or a
properly designed owner so ownership and copy behavior remain correct.

See the [principles technical notes](../../principles/README.md).