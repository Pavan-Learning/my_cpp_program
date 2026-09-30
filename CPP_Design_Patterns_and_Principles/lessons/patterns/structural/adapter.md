# Adapter

## 1. Definition

Adapter is a structural pattern that translates an existing component's interface
into the interface a client expects. It allows otherwise incompatible components
to collaborate without requiring the client or existing component to be rewritten.
**Preserve the component; translate how the client uses it.**

## 2. The Problem It Solves

A useful library may expose different method names, units, data formats, or error
conventions from the application's own contract. Teaching every caller those
differences spreads integration knowledge throughout the application. Modifying
the library may be impossible, especially for third-party or legacy software.

The integration needs one explicit translation boundary. Matching function
signatures alone is insufficient: the meanings of the arguments and results must
also match what the client expects.

## 3. Understand the Mechanism

The **target** is the client's desired interface. The **adaptee** is the existing
component. The **adapter** implements the target, invokes the adaptee, and converts
inputs, outputs, and failures as needed. Clients depend on the target contract.

An object adapter holds an adaptee through composition. A class adapter uses
inheritance to connect interfaces. Composition usually makes borrowing, ownership,
and replacement choices more explicit.

An adapter cannot honestly supply a guarantee the old system lacks. Wrapping a
blocking API does not inherently make it nonblocking; renaming an operation does
not repair different transaction semantics.

## 4. Real-World Scenario

Imagine a payment service integrating a bank SDK that returns numeric status codes
and accepts amounts in minor currency units. The application expects a typed
payment result and a validated money value. An adapter performs conversions and
maps known bank failures into the application's error model.

The benefit is that switching SDKs affects the boundary rather than every checkout
caller. The difficult part is preserving meaning: a timeout may mean an unknown
payment outcome, not a definite decline. An honest adapter keeps that distinction.

## 5. Understand the C++ Example

Open [adapter.cpp](../../../patterns/structural/adapter.cpp).

`TemperatureSensor` is the target. `LegacyThermometer` is the adaptee and exposes
Fahrenheit. `TemperatureAdapter` implements Celsius readings while borrowing the
legacy object.

1. Two thermometers hold 32 and 212 Fahrenheit.
2. An adapter wraps each without changing the legacy class.
3. Calling `celsius()` reads Fahrenheit through the wrapped object.
4. It computes `(fahrenheit - 32) * 5 / 9` using floating-point arithmetic.
5. Checks confirm 0 and 100 Celsius, including a call through the target interface.
6. The program prints `100 C`.

The adapter stores a const reference, not ownership. The thermometers must outlive
it. This small example translates units; a real sensor adapter also needs a policy
for stale readings, unavailable hardware, and nonfinite values.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** isolates integration knowledge, supports incremental migration, and
gives clients one consistent API.

**Drawbacks:** adds a layer and can hide semantic mismatches if translation is
careless. It cannot create missing capabilities without additional implementation.

Use it for an actual mismatch. Decorator adds behavior while retaining an interface;
Proxy controls access; Facade simplifies a subsystem. Similar wrapper structure
does not make their intentions identical.

## 7. Check Your Understanding

**Question:** Would forwarding Fahrenheit unchanged through `celsius()` be valid?

**Answer:** No. The method would compile but violate its unit contract. Correct
adaptation preserves meaning, not just names and return types.

See the [structural technical notes](../../../patterns/structural/README.md).