# RAII and Explicit Ownership

## 1. Definition

Resource Acquisition Is Initialization (RAII) is the C++ habit of putting a resource
inside an object that takes care of releasing it. When the object's lifetime ends,
its destructor performs the cleanup automatically.

**In simple words:** give every resource a responsible owner, so callers do not
have to remember cleanup at every exit. Resources include allocated memory, open
files, held locks, and network connections.

**Ownership** means being responsible for release. **Borrowing** means being allowed
to use the resource without becoming responsible for releasing it.

## 2. The Problem It Solves

Manual cleanup must be repeated on normal return, early return, and every exception
path. Adding one new exit can create a leak. Unclear ownership can also cause two
callers to release the same resource or neither to release it.

The language's scope and destruction rules should carry the cleanup obligation
instead of requiring each caller to remember every path.

## 3. Understand the Principle

Imagine one object owns an open file. Other code may borrow access to read it, but
the owner is responsible for closing it. Moving ownership passes that responsibility
to another object; it does not need to copy the file. Any borrower must stop using
the file before the owner closes it.

Prefer values for ordinary contained objects, `unique_ptr` for exclusive dynamic
ownership, and `shared_ptr` only for genuinely shared lifetime. `weak_ptr` observes
shared ownership without extending it. Reference counting does not make the pointed
object thread-safe, and ownership cycles can prevent destruction.

Destructors should normally not throw exceptions. If an operation such as saving
buffered data can fail, provide a separate function that reports failure, while
keeping destructor cleanup nonthrowing. During normal returns and exception handling,
C++ destroys local objects as it leaves their scopes. This exception cleanup is
called **stack unwinding**. It does not run reliably after abrupt process termination
or power loss.

## 4. Real-World Scenario

A file-processing operation opens a file, acquires a mutex, then parses content.
Parsing may fail. A file owner and lock guard ensure both resources are released
when control leaves the scope, regardless of the parsing result.

RAII prevents resource leaks, but it does not necessarily undo bytes already written.
Atomic output or transactional rollback requires an additional deliberate protocol.

## 5. Understand the C++ Example

Open [raii_and_ownership.cpp](../../principles/raii_and_ownership.cpp).

`Connection` simulates a resource using an active count. Construction increments
the count; destruction decrements it. Copying is disabled.

1. A `unique_ptr` creates one connection.
2. Moving that pointer empties the source and transfers ownership to the destination.
3. The active count stays one because no connection was duplicated.
4. Scope exit destroys the owner, returning the count to zero.
5. Another operation creates a connection, then throws deliberately.
6. Unwinding destroys its owner before the catch; the final count is again zero.

The test covers both normal and exceptional cleanup. The counter is a deterministic
single-threaded simulation, not real network resource management.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** deterministic ordinary cleanup, explicit lifetime, exception-path
safety, and less repeated release code.

**Drawbacks:** fallible finalization needs extra design, shared lifetime can become
unclear, and asynchronous borrowing requires careful coordination.

Use existing standard or library owners before writing a custom one. Keep raw-resource
management in a small tested layer, then compose it into higher-level objects.

## 7. Check Your Understanding

**Question:** Does moving a `unique_ptr` move or copy the pointed-to connection?

**Answer:** It transfers the owning handle. The connection stays where it is; the
new pointer becomes responsible for eventual destruction.

See the [principles technical notes](../../principles/README.md).