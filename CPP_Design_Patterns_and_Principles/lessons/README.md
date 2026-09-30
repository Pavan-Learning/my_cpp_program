# Concept-First Lessons: Patterns and Principles

Start here if design patterns and design principles are new to you. You do not need
to memorize pattern names or class diagrams before reading. Start with the ordinary
problem, follow the small picture, then connect that idea to the C++ names.

The 42 lessons explain their important technical words where they are used. The
glossary is an optional reminder, not required reading before every lesson. For a
worked starting point, open [Factory Method](patterns/creational/factory_method.md):
road delivery creates a truck, sea delivery creates a ship, and both reuse the same steps.

Each lesson follows this order:

1. **Definition:** what the pattern or principle means in plain language.
2. **The problem it solves:** why the idea is needed and what goes wrong without it.
3. **Step-by-step idea and picture:** explain the parts, define new words, and follow one clear example.
4. **Real-world scenario:** another practical situation showing where the idea helps.
5. **C++ walkthrough and three diagrams:** what the code names mean, what runs first, and what answers the checks expect.
6. **Benefits and drawbacks:** when to use it, when not to, and simpler alternatives.
7. **Question and answer:** a common misunderstanding explained directly in the lesson.

The scenarios describe possible designs, not claims about how a particular commercial
product is built. The small programs teach an idea; they are not finished payment,
security, traffic-control, or other production systems.

## Viewing the Diagrams

Every lesson includes a small picture in section 3. There are at most six boxes,
with the drawing arranged from top to bottom. The explanation above it tells you
what its arrows mean; the **Read it as a sentence** paragraph below walks through it.

- A rectangle names a thing, a value, or a step, as explained in that lesson.
- A diamond asks a question. Follow the arrow labeled with the appropriate answer.
- An arrow can mean "next step," "uses," or "contains." Use the stated meaning for
	that picture; not every diagram shows calls happening over time.
- The picture teaches one idea. The walkthrough below it explains the actual C++
	calls and any details deliberately left out of the picture.

### Three Views of Every C++ Example

Section 5 of every lesson now includes three additional diagrams, separate from
the simple concept picture in section 3:

| View | Question it answers | What to follow |
| --- | --- | --- |
| Flow diagram | What steps or decisions lead to this result or drawback? | Start at the top, follow arrows, and choose the labeled branch at a question |
| Class diagram | Which types exist, and which ones inherit, own, or use others? | Read the relationship labels; this is structure, not execution order |
| Sequence diagram | Who calls whom, and what comes back? | Time runs down the page; solid arrows call and dashed arrows return |

Every view has its own explanation and uses names from the linked C++ source.
Some views focus on `main()` and others on `demonstrate_drawback()`; the nearby
text says which. They show selected details, not every check or repeated output call.

In class diagrams, a hollow triangle points toward a base class, a filled diamond
marks exclusive ownership or a contained value, and ordinary or dotted arrows
have labels explaining borrowing, shared ownership, creation, or temporary use.
`+` means public, `-` private, `#` protected, and `$` marks a static member.
Some return types are shortened; the explanation gives the important C++ details.
When the program has no custom classes, boxes marked `function` or `module` show
its actual free functions and data types rather than inventing an object hierarchy.

For a complete example, open Decorator's [flow](patterns/structural/decorator.md#c-flow-diagram),
[class](patterns/structural/decorator.md#c-class-diagram), or
[sequence](patterns/structural/decorator.md#c-sequence-diagram) view. Every category
README also has a **C++ diagrams** column linking straight to these views.

### Open a Rendered Preview

Diagrams are in the linked Markdown lessons, not rendered inside the `.cpp` editor.
Diagrams use Mermaid code blocks. GitHub renders them in its normal Markdown file
view. In VS Code, open Markdown Preview with `Ctrl+Shift+V`; the preview needs Mermaid
support. If it shows `flowchart`, `classDiagram`, or `sequenceDiagram` and arrows such as `-->`
instead of a picture, that preview is displaying source rather than rendering
Mermaid. Use a Mermaid-capable preview or GitHub's rendered file view. The plain
text editor always shows the diagram source.

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
| [Factory Method](patterns/creational/factory_method.md) | Reuse the same delivery steps while road and sea versions create different vehicles |
| [Abstract Factory](patterns/creational/abstract_factory.md) | Create a matching set of objects, such as light-theme buttons and checkboxes |
| [Builder](patterns/creational/builder.md) | Prepare an object step by step and check it before use |
| [Prototype](patterns/creational/prototype.md) | Create an object by copying an already configured one |
| [Singleton](patterns/creational/singleton.md) | Share one object and understand why global access can make testing harder |

## Structural Patterns

| Topic | What you will understand |
| --- | --- |
| [Adapter](patterns/structural/adapter.md) | Help existing code work with an interface it was not designed for |
| [Bridge](patterns/structural/bridge.md) | Choose the shape separately from the tool used to draw it |
| [Composite](patterns/structural/composite.md) | Use the same operation on one item or a group of items |
| [Decorator](patterns/structural/decorator.md) | Add behavior by wrapping an object, and see why wrapper order matters |
| [Facade](patterns/structural/facade.md) | Offer one simple entry point to several cooperating parts |
| [Flyweight](patterns/structural/flyweight.md) | Share repeated data while keeping each object's own details separate |
| [Proxy](patterns/structural/proxy.md) | Use a stand-in to control when or how the real object is accessed |

## Behavioral Patterns

| Topic | What you will understand |
| --- | --- |
| [Chain of Responsibility](patterns/behavioral/chain_of_responsibility.md) | Pass a request through separate checks and stop when a check rejects it |
| [Command](patterns/behavioral/command.md) | Store an action as an object so it can be run, undone, or redone |
| [Interpreter](patterns/behavioral/interpreter.md) | Build and work out a rule such as signed in AND paid |
| [Iterator](patterns/behavioral/iterator.md) | Visit collection items one by one, and know when an iterator becomes invalid |
| [Mediator](patterns/behavioral/mediator.md) | Put coordination rules in one place instead of connecting every object to every other |
| [Memento](patterns/behavioral/memento.md) | Save and restore an object's state without exposing its private details |
| [Observer](patterns/behavioral/observer.md) | Notify interested listeners when something happens |
| [State](patterns/behavioral/state.md) | Change an object's behavior when its current mode changes |
| [Strategy](patterns/behavioral/strategy.md) | Choose between different ways to perform the same job |
| [Template Method](patterns/behavioral/template_method.md) | Keep report steps in order while different reports supply their own content |
| [Visitor](patterns/behavioral/visitor.md) | Add new operations to known object types without putting every operation inside them |

## SOLID Principles

| Topic | What you will understand |
| --- | --- |
| [Single Responsibility](solid/srp.md) | Keep work that changes for different reasons in separate places |
| [Open/Closed](solid/ocp.md) | Make expected additions possible without repeatedly rewriting stable code |
| [Liskov Substitution](solid/lsp.md) | A replacement must keep the promises that existing callers rely on |
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

The original category chapters are optional advanced notes with comparisons and
more technical detail. Learn the idea in its lesson first. Each lesson links to
its runnable source and the relevant advanced notes.
Use the [collection README](../README.md) for build commands and the executable index.

For C++ idioms, architecture, concurrency, and topics beyond the canonical GoF set,
see [C++ Foundations and Beyond GoF](../docs/CPP_Foundations_and_Beyond_GoF.md).
The complete standalone lesson set covers the 42 executable topics; this wider
companion remains supplementary material rather than another claimed exhaustive catalog.