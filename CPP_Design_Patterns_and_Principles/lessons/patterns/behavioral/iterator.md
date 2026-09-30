# Iterator

## 1. Definition

Iterator is a behavioral pattern that provides a way to access elements of a
collection in sequence without exposing its internal representation. Traversal
position is represented separately from the collection itself.

## 2. The Problem It Solves

Algorithms need to search, count, and process collections, but should not require
every container's storage layout. A vector, linked structure, or tree may all
support traversal even though locating the next element differs.

Separating the traversal protocol from storage lets algorithms operate on a range
without knowing how the container organizes its elements.

## 3. Understand the Mechanism

An iterator represents a position and supports defined operations such as reading
the current element, advancing, and detecting the end. Two iterators can represent
independent positions when the traversal model is multi-pass.

C++ standard iterators express this protocol through operators and traits, not
necessarily inheritance. A half-open range includes its beginning but excludes
its end. The end position is a boundary, not a valid element to dereference.

Capabilities matter: input iteration may be single-pass, forward iteration supports
multiple passes, and stronger categories add backward movement or random access.
Algorithms rely on the advertised guarantees, including complexity.

## 4. Real-World Scenario

A search tool processes records from different collections through a common
traversal protocol. A caller can look for a matching record without knowing whether
the collection stores contiguous values or walks an index.

A streaming database cursor may be single-pass and involve I/O, unlike a multi-pass
memory iterator. A common-looking traversal API must not hide those important
differences or promise operations the underlying source cannot support.

## 5. Understand the C++ Example

Open [iterator.cpp](../../../patterns/behavioral/iterator.cpp).

`Playlist` owns a vector of track names. Its nested `Iterator` wraps a vector const
iterator and exposes forward-iterator operations.

1. An empty playlist has equal begin and end positions.
2. Adding `Intro` and `Finale` creates a two-element range.
3. Post-increment saves the old position and advances the current iterator.
4. Checks verify the saved iterator reads `Intro` while the current one reads `Finale`.
5. `std::distance` counts two elements, and `std::find` locates `Finale`.
6. Range-for uses the same protocol to print both titles.

Iterators borrow storage; they do not keep the playlist alive. Adding tracks can
invalidate them through vector reallocation. The wrapper advertises only forward
operations, so algorithms cannot assume constant-time random-access distance.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** storage-independent algorithms, independent traversal positions, and
interoperability with standard algorithms.

**Drawbacks:** invalidation and lifetime hazards, category requirements, and possible
hidden I/O or traversal cost when contracts are vague.

Use standard container iterators when sufficient; a custom wrapper is not always
needed. Ranges and views can compose traversal transformations. Visitor determines
what to do with an element, whereas Iterator determines how to reach elements.

## 7. Check Your Understanding

**Question:** Is it safe to append tracks while iterating this playlist?

**Answer:** Not without a specific mutation strategy. Vector insertion can invalidate
the iterators used by the loop. Defer mutation or traverse a suitable snapshot.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).