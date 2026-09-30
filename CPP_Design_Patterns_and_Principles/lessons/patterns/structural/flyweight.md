# Flyweight

## 1. Definition

Flyweight saves memory by storing repeated information once and letting many
objects share it. Each object still keeps the information that is different for it.

For example, a thousand letters can share one font style, but each letter needs
its own character and position. The shared part is called **intrinsic state**;
the part kept separately for each use is called **extrinsic state**.

The idea is not simply “cache objects.” It is **separate what can be shared from
what must remain unique, then represent many objects using that shared core**.

## 2. The Problem It Solves

Millions of logical objects may repeat expensive information: fonts, meshes,
textures, formatting, or classifications. Storing the repeated information inside
every object wastes memory. But sharing the whole object is also wrong when
positions, identities, or other context differ.

The design must identify the boundary between shared meaning and individual state.
That boundary depends on the domain, not a fixed rule about field types.

## 3. Understand the Mechanism

A factory keeps a collection of shared values. When asked for a style such as
`(Mono, 12)`, it returns the existing style if one is available. Otherwise, it
creates and stores that style. Each letter keeps a pointer to the shared style
alongside its own character and position.

Shared data is usually **immutable**, meaning it cannot be changed after creation.
Otherwise editing one shared style could unexpectedly change every letter using
it. The lookup key must include all the shared details: looking up by font alone
would confuse 12-point and 20-point text.

Sharing has a cost too: every letter needs a pointer, and finding a style takes
work. It helps only when these costs are smaller than storing repeated copies.
The factory also needs rules for removing unused styles and, if several threads
use it, protecting its collection. `shared_ptr` alone does not protect that collection.

## 4. Real-World Scenario

A map viewer displays thousands of identical category markers. Their icon geometry
and color scheme can be shared, while coordinates and labels stay per marker.
Changing a marker's position should not require copying the icon geometry.

If users can edit one marker's appearance, the viewer can select another immutable
style rather than mutating the shared style. Very small icons or mostly unique
styles may not justify a flyweight pool; measurements decide whether it helps.

## 5. Understand the C++ Example

Open [flyweight.cpp](../../../patterns/structural/flyweight.cpp).

`GlyphStyle` contains font and size. `StyleFactory` maps a `(font, size)` key to
`shared_ptr<const GlyphStyle>`. Each `Glyph` stores character, position, and a style.

1. Requesting `(Mono, 12)` creates a style for the first glyph.
2. Requesting the same key returns the same object for the second glyph.
3. Requesting `(Mono, 20)` creates a different style for the heading.
4. Pointer checks prove actual sharing, not just equal descriptions.
5. Another check confirms positions remain independent.
6. Output is `AB share Mono`.

Both glyphs and the factory own style handles. A style can outlive the factory if
a glyph retains it. The factory keeps every created style while it lives; no
eviction or multithreaded lookup is implemented.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** reduces repeated payload, encourages immutable sharing, and can
improve memory locality for large populations.

**Drawbacks:** extra handles and lookups, retention costs, more context parameters,
and accidental shared-mutation hazards.

Use it when repetition is large enough to matter. Ordinary values are simpler for
small populations. Weak caches can reclaim unused values but add recreation and
expired-entry handling. An object pool reuses instances; it need not share state
among simultaneously existing logical objects as Flyweight does.

## 7. Check Your Understanding

**Question:** Where should a glyph's screen position live?

**Answer:** In its extrinsic state. Putting position into a shared font style would
make unrelated glyphs share a property that must differ for each placement.

See the [structural technical notes](../../../patterns/structural/README.md).