# Command

## 1. Definition

Command is a behavioral pattern that represents a request as an object containing
the information needed to perform it. This separates the object requesting work
from the object that knows how to do it, allowing requests to be queued, recorded,
composed, or undone when the domain supports those operations.

## 2. The Problem It Solves

A direct function call executes and disappears. A menu action, keyboard shortcut,
automation script, and task queue may all need the same operation, possibly later.
Undo also requires remembering information about the operation that occurred.

The application needs requests to have identity and lifetime beyond one call
expression. Turning them into objects makes that information explicit.

## 3. Understand the Mechanism

The client creates a command with its parameters and receiver. An invoker executes
the command through a common interface. The receiver performs domain work. The
command can retain information needed for undo or result inspection.

Undo is optional, not inherent. Some commands have a precise inverse; others need
a saved snapshot. External effects may require compensation rather than reversal.
Replaying a command is also not always safe: retries can duplicate payments or
messages unless the operation has an idempotency contract.

## 4. Real-World Scenario

In a photo editor, menu actions and shortcuts both create editing commands. A
history manager stores them, allowing the user to undo a crop or adjustment. A
batch-processing tool can invoke similar operations without using the GUI.

A crop might save removed pixels or a previous image snapshot; an upload command
cannot simply “un-upload” an image without a separate remote deletion operation.
The request representation is reusable, but each operation's undo semantics differ.

## 5. Understand the C++ Example

Open [command.cpp](../../../patterns/behavioral/command.cpp).

`Document` is the receiver. `Append` stores text and a borrowed document reference.
`History` owns commands in done and undone stacks.

1. Appending `hello` records the previous document length, then adds the text.
2. Appending ` world` produces `hello world`.
3. Undo truncates to the saved length, restoring `hello`.
4. Redo executes that command again, restoring `hello world`.
5. Undo followed by a new ` C++` edit clears the abandoned redo branch.
6. The final output is `hello C++`; empty-history operations are also checked.

History reserves vector space before changing the document, avoiding an allocation
failure that would lose the history record after an edit. Commands borrow the
document, so it must outlive them. Undo assumes last-in-first-out changes through
this history; unrelated external edits would invalidate its saved-length logic.

## 6. Benefits, Drawbacks, and Alternatives

**Benefits:** reusable invocation paths, explicit request data, deferred execution,
and a natural home for history or macros.

**Drawbacks:** extra objects and retained state, receiver-lifetime concerns, and
potentially complicated undo, exception, and retry semantics.

Use a callable for a simple one-off task. Use a command object when request metadata,
history, or domain-specific behavior matters. Memento captures state rather than
an action, and a command may use a memento to implement undo.

## 7. Check Your Understanding

**Question:** Why does a new edit invalidate redo history?

**Answer:** It creates a different future from the restored state. Replaying the
old future could apply operations to a document state they were not designed for.

See the [behavioral technical notes](../../../patterns/behavioral/README.md).