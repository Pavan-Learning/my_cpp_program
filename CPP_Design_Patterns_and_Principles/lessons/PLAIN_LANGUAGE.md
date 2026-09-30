# Design Terms in Plain Language

Use this as a quick reference while reading a lesson. The technical names are
useful in C++ books and interviews; the examples explain what those names mean.

## Basic C++ Words

| Term | Plain meaning | Small example |
| --- | --- | --- |
| Class | A definition of a kind of object, including its data and functions | `Truck` describes truck objects |
| Object or instance | One actual value created from a type | One truck created for a delivery |
| Method or member function | A function belonging to a class | A truck's `deliver()` function |
| Constructor | The function that initializes a new object | Set a circle's radius when creating it |
| Destructor | The function used for an object's cleanup | Release an owned connection when its owner is destroyed |
| Base class | A class other classes extend | `Shape` describes shared shape operations |
| Derived class or subclass | A class built from a base class | `Circle` supplies circle-specific behavior |
| Virtual function | An operation whose version can be supplied by a derived class | Ask a `Shape` for area and run the circle's calculation |
| Override | A derived class's version of a virtual function | `RoadLogistics` supplies its truck-creation function |
| Pointer | A value that stores how to reach an object | A pointer refers to an existing circle |
| Reference | Another name for an existing object, not a copy of it | A function receives access to the caller's document |
| Scope | A region of code, often inside braces | An ordinary local owner is cleaned up when its block is left |
| Exception | An error signal that leaves normal execution for a handler | Reject constructing an invalid percentage |
| Lambda | A small function written where it is needed | A test supplies a function returning a chosen time |
| Template | A recipe the compiler can use with suitable types | Build the same report-writing function for different output types |

## Objects and Their Jobs

| Term | Plain meaning | Small example |
| --- | --- | --- |
| State | Information an object currently holds | An account's current balance |
| Behavior | Work an object can do | An account's `deposit()` operation |
| Interface | The operations a caller can use | A printer offers `print(document)` |
| Implementation | The code that actually does the work | Sending the document to a particular printer |
| Client or caller | Code that uses another object or function | A report function that calls the printer |
| Abstraction | Showing the useful operations while hiding details | Asking for a stock count without writing database queries |
| Dependency or collaborator | Another object or service needed to do a job | A checkout needs a payment service |
| Contract | The rules and promises of an operation or type | Which inputs `save()` accepts, what success means, and how failure is reported |

An interface is not automatically an abstract base class. C++ can express the
needed operations through a virtual base class, a template requirement, or a
callable object. Choose the form that fits the program.

## Relationships and Changes

| Term | Plain meaning | Small example |
| --- | --- | --- |
| Cohesion | How closely the work inside one part belongs together | Stock counting and reserving belong with stock rules |
| Coupling | How much one part depends on another part's details | A report that knows database column names is tied to that schema |
| Composition | Building something using other objects as parts | A robot contains a motor and a camera |
| Inheritance | Defining a type based on another type | `Circle` derives from `Shape`; it must honor `Shape`'s promises |
| Polymorphism | Using one common operation with different implementations | Calling `area()` through `Shape` runs the circle or rectangle calculation |
| Delegation | Asking a helper to perform part of the work | A checkout asks a shipping policy to calculate delivery cost |
| Encapsulation | Keeping data behind operations that protect its rules | Calling `withdraw()` instead of directly editing the balance |
| Extension | Adding a new supported behavior or type | Adding another shipping policy |

Low coupling does not mean no dependencies. It means avoiding unnecessary
knowledge of another part's internal details. A useful object can still depend
on a clear, stable interface.

## Rules and Valid Data

| Term | Plain meaning | Small example |
| --- | --- | --- |
| Precondition | What must be true before an operation | A withdrawal amount must be positive |
| Postcondition | What is promised after success | The withdrawn amount has been subtracted |
| Invariant | A rule that stays true throughout normal use | A percentage stays between 0 and 100 |
| Immutable | Not changeable after creation | Create a new shared text style instead of editing an existing one |
| Atomic operation | An operation treated as one indivisible step, according to its stated guarantee | Other threads do not see a partly completed atomic counter update |
| Transaction | Related operations grouped under stated success, failure, and visibility guarantees | A database transfer updates both accounts together or rolls both updates back |

A function that calls several services is not automatically a transaction. A
successful payment followed by a failed stock reservation still needs an explicit
recovery design. Also, one atomic variable does not make a whole workflow atomic.

## Lifetime and Sharing

| Term | Plain meaning | Small example |
| --- | --- | --- |
| Lifetime | The period when an object exists and can be used | An ordinary automatic local object lives until its scope is left |
| Ownership | Responsibility for releasing a resource | A `unique_ptr` destroys its owned object |
| Borrowing | Using a resource without owning it | A reference lets a report use a printer owned elsewhere |
| Move | An operation that can transfer stored resources instead of copying them | Moving a `unique_ptr` transfers responsibility for the same object |
| Aliasing | Two ways to reach the same object | Two pointers refer to one shared style |
| Dangling reference | A reference to an object whose lifetime has ended | A saved reference still points at a destroyed local variable |
| Stack unwinding | Cleanup as an exception leaves function scopes | A lock guard unlocks before a surrounding `catch` handles the error |
| Iterator invalidation | A collection change makes an existing iterator unusable | A vector grows into a new allocation |

Borrowing does not keep an object alive. Shared ownership can keep it alive, but
does not automatically make simultaneous reads and writes safe. A moved-from
object is not generally promised to be empty; the relevant type defines its guarantees.

## Terms Used by Particular Patterns

| Term | Plain meaning | Small example |
| --- | --- | --- |
| Intrinsic state | Data suitable for sharing across many uses | A font style shared by many letters |
| Extrinsic state | Data kept separately for each use | Each letter's position |
| Dispatch | Choosing which operation implementation runs | A virtual call selects a circle's `accept()` implementation |
| Double dispatch | Selecting behavior using two object types | Visitor combines the shape type with the visitor type |
| Hook | A step designed for customization | A report subclass supplies its own header |
| Callback | A function supplied for other code to call when needed | A listener function runs when a temperature changes |
| High-level policy | An application's business rules | Deciding whether an order can be accepted |
| Low-level detail | A particular tool used to carry out those rules | Reading stock through a database driver |
| Composition point | Setup code that creates and connects the objects | `main()` passes a stock reader into a warehouse |

Return to the [lesson index](README.md) for full explanations, scenarios, C++
walkthroughs, benefits, drawbacks, and questions with answers.