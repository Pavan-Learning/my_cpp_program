# Concept-First Lessons: Patterns and Principles

Start here to understand a topic before studying its implementation. Each of the
42 standalone lessons follows this order:

1. **Definition:** what the pattern or principle means in plain language.
2. **The problem it solves:** why the idea is needed and what goes wrong without it.
3. **Deeper explanation:** how it works, its important distinctions, and its limits.
4. **Real-world scenario:** an illustrative practical situation, independent of the code.
5. **C++ walkthrough:** the actual source, participant roles, execution steps, and checks.
6. **Benefits and drawbacks:** when to use it, when not to, and simpler alternatives.
7. **Question and answer:** a common misunderstanding explained directly in the lesson.

Real-world scenarios illustrate possible designs; they do not claim that a named
commercial product uses a particular implementation. The small runnable programs
demonstrate mechanisms, not complete production systems.

## How the Lessons Are Grouped

A **pattern** is a reusable way to arrange objects to solve a recurring problem.
A **principle** is a guideline for deciding whether a design is clear and easy to
change. A pattern is a possible solution, not a rule that every program must follow.

```text
lessons/
	patterns/
		creational/    Creating objects
		structural/    Connecting and combining objects
		behavioral/    How objects work together
	solid/           Five guidelines for class and interface design
	principles/      Other design guidelines and C++ ownership ideas
```

Unfamiliar word? The [plain-language glossary](PLAIN_LANGUAGE.md) explains terms
such as interface, contract, coupling, ownership, and dispatch with small examples.
The lessons also explain important terms where they are introduced.

## Creational Patterns

| Topic | What you will understand |
| --- | --- |
| [Factory Method](patterns/creational/factory_method.md) | Keep the same workflow but let a subclass choose which object to create |
| [Abstract Factory](patterns/creational/abstract_factory.md) | Create a matching set of objects, such as light-theme buttons and checkboxes |
| [Builder](patterns/creational/builder.md) | Prepare an object step by step and check it before use |
| [Prototype](patterns/creational/prototype.md) | Create an object by copying an already configured one |
| [Singleton](patterns/creational/singleton.md) | Keep one shared instance and understand the testing and lifetime costs |

## Structural Patterns

| Topic | What you will understand |
| --- | --- |
| [Adapter](patterns/structural/adapter.md) | Help existing code work with an interface it was not designed for |
| [Bridge](patterns/structural/bridge.md) | Let two related choices, such as shape and renderer, change separately |
| [Composite](patterns/structural/composite.md) | Use the same operation on one item or a group of items |
| [Decorator](patterns/structural/decorator.md) | Add behavior by wrapping an object, and see why wrapper order matters |
| [Facade](patterns/structural/facade.md) | Offer one simple entry point to several cooperating parts |
| [Flyweight](patterns/structural/flyweight.md) | Share repeated data while keeping each object's own details separate |
| [Proxy](patterns/structural/proxy.md) | Use a stand-in to control when or how the real object is accessed |

## Behavioral Patterns

| Topic | What you will understand |
| --- | --- |
| [Chain of Responsibility](patterns/behavioral/chain_of_responsibility.md) | Pass a request through handlers and decide when to stop |
| [Command](patterns/behavioral/command.md) | Store an action as an object so it can be run, undone, or redone |
| [Interpreter](patterns/behavioral/interpreter.md) | Represent and evaluate the rules of a small language |
| [Iterator](patterns/behavioral/iterator.md) | Visit collection items one by one, and know when an iterator becomes invalid |
| [Mediator](patterns/behavioral/mediator.md) | Put coordination rules in one place instead of connecting every object to every other |
| [Memento](patterns/behavioral/memento.md) | Save and restore an object's state without exposing its private details |
| [Observer](patterns/behavioral/observer.md) | Notify interested listeners when something happens |
| [State](patterns/behavioral/state.md) | Change an object's behavior when its current mode changes |
| [Strategy](patterns/behavioral/strategy.md) | Choose between different ways to perform the same job |
| [Template Method](patterns/behavioral/template_method.md) | Keep a fixed sequence of steps while subclasses customize selected steps |
| [Visitor](patterns/behavioral/visitor.md) | Add new operations to known object types without putting every operation inside them |

## SOLID Principles

| Topic | What you will understand |
| --- | --- |
| [Single Responsibility](solid/srp.md) | Keep work that changes for different reasons in separate places |
| [Open/Closed](solid/ocp.md) | Make expected additions possible without repeatedly rewriting stable code |
| [Liskov Substitution](solid/lsp.md) | A derived type must keep the promises made by its base type |
| [Interface Segregation](solid/isp.md) | Give callers the operations they need, without forcing unrelated ones on them |
| [Dependency Inversion](solid/dip.md) | Let business rules depend on needed services, not a particular database or tool |

## Core Design Principles

| Topic | What you will understand |
| --- | --- |
| [DRY](principles/dry.md) | Keep each business rule in one place without merging unrelated rules |
| [KISS](principles/kiss.md) | Choose the simplest correct design, not merely the shortest code |
| [YAGNI](principles/yagni.md) | Build what is needed now instead of guessing future features |
| [Separation of Concerns](principles/separation_of_concerns.md) | Separate reading input, applying rules, storing data, and displaying results |
| [Cohesion and Coupling](principles/cohesion_and_coupling.md) | Keep related work together and limit how much parts know about one another |
| [Encapsulation](principles/encapsulation.md) | Protect an object's rules instead of allowing unrestricted changes to its data |
| [Composition Over Inheritance](principles/composition.md) | Build an object from useful parts when an inheritance relationship is not needed |
| [Programming to Interfaces](principles/interfaces.md) | Use what an object promises to do without relying on its internal details |
| [Law of Demeter](principles/law_of_demeter.md) | Ask nearby objects for help instead of reaching through their internal parts |
| [Dependency Injection](principles/dependency_injection.md) | Pass needed helpers into an object instead of creating them inside it |
| [Immutability](principles/immutability.md) | Create a new value instead of changing one that other code may be using |
| [Contracts and Strong Types](principles/contracts.md) | State input rules and result promises, and give values meaningful types |
| [RAII and Ownership](principles/raii_and_ownership.md) | Make cleanup automatic and make clear who owns or borrows a resource |
| [Value Semantics and Rule of Zero](principles/value_semantics.md) | Make independent copies and let well-designed members handle cleanup |

## Additional Reading and Running Examples

The original category chapters retain diagrams, comparison tables, and additional
implementation caveats. Each lesson links to its category's notes and source.
Use the [collection README](../README.md) for build commands and the executable index.

For C++ idioms, architecture, concurrency, and topics beyond the canonical GoF set,
see [C++ Foundations and Beyond GoF](../docs/CPP_Foundations_and_Beyond_GoF.md).
The complete standalone lesson set covers the 42 executable topics; this wider
companion remains supplementary material rather than another claimed exhaustive catalog.