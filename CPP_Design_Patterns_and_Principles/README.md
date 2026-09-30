# C++ Design Patterns and Principles: A Fresh, Self-Contained Collection

## Start with the Concept, Then the Code

Open the **[concept-first lesson index](lessons/README.md)**. Each of the 42 topics
now has its own standalone lesson: definition, the problem it solves, a deeper
conceptual explanation, a real-world scenario, a step-by-step C++ walkthrough,
benefits, drawbacks, alternatives, and an explained question.

For example, begin with the [Factory Method lesson](lessons/patterns/creational/factory_method.md) or
the [Single Responsibility lesson](lessons/solid/srp.md). The category chapters below
remain supplementary technical notes; they are not the only explanations.

This collection is written independently of the existing practice examples. It
has its own folders, sources, build configuration, and tests. Nothing needs to be
copied from the older design-pattern project.

The emphasis is understanding: the problem before the pattern, exact control flow,
ownership, useful variations, benefits, drawbacks, alternatives, and failure cases.
Every runnable topic has a worked explanation and an exercise with its answer.
The SOLID sources include both a problematic design and its correction.

## Coverage and Honest Boundaries

- **All 23 canonical Gang of Four (GoF) patterns:** 5 creational, 7 structural,
  and 11 behavioral, each with a separate C++ program.
- **All five SOLID principles:** separate before-and-after C++ programs.
- **14 core design-principle topics:** separate C++ programs, plus additional
  principles explained in the same chapter.
- **C++ and architectural companion:** ownership, exception guarantees, idioms,
  architectural patterns, concurrency patterns, and production decision-making.

There are **42 runnable examples**. “Every design pattern” or “every possibility”
has no finite universal definition beyond a named catalog such as GoF. This is
complete for GoF and SOLID, and explicitly scoped for the wider subject. The
companion explains additional families; it does not claim to implement every
architectural, concurrency, distributed-system, or C++ idiom variation.

Examples are deliberately small, single-process, and single-threaded. They are
teaching programs, not production authentication, payment, graphics, database, or
traffic-control implementations. Each lesson identifies important assumptions.

## Read the Lessons

| Chapter | What it teaches |
| --- | --- |
| [Standalone Concept-First Lessons](lessons/README.md) | All 42 topics taught from definition and practical context through code |
| [C++ Foundations and Beyond GoF](docs/CPP_Foundations_and_Beyond_GoF.md) | Ownership, dispatch, guarantees, idioms, architecture, production reasoning |
| [Creational Patterns](patterns/creational/README.md) | Construction choices, families, assembly, cloning, unique instance access |
| [Structural Patterns](patterns/structural/README.md) | Interface translation, composition, trees, wrappers, sharing, controlled access |
| [Behavioral Patterns](patterns/behavioral/README.md) | Requests, undo, expressions, traversal, coordination, notifications, state, algorithms |
| [SOLID](solid/README.md) | Responsibility, extensibility, substitution, narrow interfaces, dependency direction |
| [Core Design Principles](principles/README.md) | Simplicity, knowledge sharing, boundaries, contracts, lifetime, values, tradeoffs |

The chapters contain the actual teaching content, not just a reading plan. Read a
topic, open its linked source, predict each check, and then run it. The checks show
which behavior is demonstrated, while the limitation sections explain what remains
outside the example.

## Build and Run

Requirements: a C++17 compiler, CMake 3.16 or newer, and a supported build tool.
No third-party C++ packages, network services, or downloads are required.

From the workspace root:

```sh
cmake -S CPP_Design_Patterns_and_Principles \
      -B CPP_Design_Patterns_and_Principles/build \
      -DCMAKE_BUILD_TYPE=Debug
cmake --build CPP_Design_Patterns_and_Principles/build -j2
ctest --test-dir CPP_Design_Patterns_and_Principles/build --output-on-failure
```

Build and run one example:

```sh
cmake --build CPP_Design_Patterns_and_Principles/build --target command
./CPP_Design_Patterns_and_Principles/build/command
ctest --test-dir CPP_Design_Patterns_and_Principles/build -R '^command$' -V
```

Expected direct output from `command`:

```text
hello C++
```

`ctest -V` displays successful programs' output too. Normally CTest shows detailed
output only for failures. A failed `check()` throws and makes the executable fail;
checks remain active in Release builds, unlike disabled standard `assert` calls.
The common helper is [support/check.hpp](support/check.hpp).

Compile one source without CMake, using the collection root as an include path:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
    -I CPP_Design_Patterns_and_Principles \
    CPP_Design_Patterns_and_Principles/patterns/behavioral/command.cpp \
    -o /tmp/cpp_design_command
/tmp/cpp_design_command
```

Each source has its own `main()` and intentionally reuses illustrative names such
as `Shape`. Do not link all sources into one executable. CMake creates one target
and one CTest entry per source. With multi-configuration generators, select the
configuration at build/test time and use its corresponding executable directory.

## Complete GoF Example Index

| Category | Pattern | Source and target |
| --- | --- | --- |
| Creational | Abstract Factory | [abstract_factory.cpp](patterns/creational/abstract_factory.cpp), `abstract_factory` |
| Creational | Builder | [builder.cpp](patterns/creational/builder.cpp), `builder` |
| Creational | Factory Method | [factory_method.cpp](patterns/creational/factory_method.cpp), `factory_method` |
| Creational | Prototype | [prototype.cpp](patterns/creational/prototype.cpp), `prototype` |
| Creational | Singleton | [singleton.cpp](patterns/creational/singleton.cpp), `singleton` |
| Structural | Adapter | [adapter.cpp](patterns/structural/adapter.cpp), `adapter` |
| Structural | Bridge | [bridge.cpp](patterns/structural/bridge.cpp), `bridge` |
| Structural | Composite | [composite.cpp](patterns/structural/composite.cpp), `composite` |
| Structural | Decorator | [decorator.cpp](patterns/structural/decorator.cpp), `decorator` |
| Structural | Facade | [facade.cpp](patterns/structural/facade.cpp), `facade` |
| Structural | Flyweight | [flyweight.cpp](patterns/structural/flyweight.cpp), `flyweight` |
| Structural | Proxy | [proxy.cpp](patterns/structural/proxy.cpp), `proxy` |
| Behavioral | Chain of Responsibility | [chain_of_responsibility.cpp](patterns/behavioral/chain_of_responsibility.cpp), `chain_of_responsibility` |
| Behavioral | Command | [command.cpp](patterns/behavioral/command.cpp), `command` |
| Behavioral | Interpreter | [interpreter.cpp](patterns/behavioral/interpreter.cpp), `interpreter` |
| Behavioral | Iterator | [iterator.cpp](patterns/behavioral/iterator.cpp), `iterator` |
| Behavioral | Mediator | [mediator.cpp](patterns/behavioral/mediator.cpp), `mediator` |
| Behavioral | Memento | [memento.cpp](patterns/behavioral/memento.cpp), `memento` |
| Behavioral | Observer | [observer.cpp](patterns/behavioral/observer.cpp), `observer` |
| Behavioral | State | [state.cpp](patterns/behavioral/state.cpp), `state` |
| Behavioral | Strategy | [strategy.cpp](patterns/behavioral/strategy.cpp), `strategy` |
| Behavioral | Template Method | [template_method.cpp](patterns/behavioral/template_method.cpp), `template_method` |
| Behavioral | Visitor | [visitor.cpp](patterns/behavioral/visitor.cpp), `visitor` |

## SOLID and Principle Example Index

| Topic | Source and target |
| --- | --- |
| Single Responsibility | [srp.cpp](solid/srp.cpp), `srp` |
| Open/Closed | [ocp.cpp](solid/ocp.cpp), `ocp` |
| Liskov Substitution | [lsp.cpp](solid/lsp.cpp), `lsp` |
| Interface Segregation | [isp.cpp](solid/isp.cpp), `isp` |
| Dependency Inversion | [dip.cpp](solid/dip.cpp), `dip` |
| DRY | [dry.cpp](principles/dry.cpp), `dry` |
| KISS | [kiss.cpp](principles/kiss.cpp), `kiss` |
| YAGNI | [yagni.cpp](principles/yagni.cpp), `yagni` |
| Separation of Concerns | [separation_of_concerns.cpp](principles/separation_of_concerns.cpp), `separation_of_concerns` |
| Cohesion and Coupling | [cohesion_and_coupling.cpp](principles/cohesion_and_coupling.cpp), `cohesion_and_coupling` |
| Encapsulation | [encapsulation.cpp](principles/encapsulation.cpp), `encapsulation` |
| Composition | [composition.cpp](principles/composition.cpp), `composition` |
| Programming to Interfaces | [interfaces.cpp](principles/interfaces.cpp), `interfaces` |
| Law of Demeter | [law_of_demeter.cpp](principles/law_of_demeter.cpp), `law_of_demeter` |
| Dependency Injection | [dependency_injection.cpp](principles/dependency_injection.cpp), `dependency_injection` |
| Immutability | [immutability.cpp](principles/immutability.cpp), `immutability` |
| Contracts and Strong Types | [contracts.cpp](principles/contracts.cpp), `contracts` |
| RAII and Ownership | [raii_and_ownership.cpp](principles/raii_and_ownership.cpp), `raii_and_ownership` |
| Value Semantics and Rule of Zero | [value_semantics.cpp](principles/value_semantics.cpp), `value_semantics` |

## What the Tests Establish

Each executable checks its key mechanism, not merely that it prints a pattern name.
Examples include polymorphic product selection, independent cloning, shared style
identity, no eager proxy load, recursive totals, undo/redo branching, subscription
changes during delivery, exact template-hook order, and cleanup during unwinding.

SOLID checks compare before/after behavior or detect a deliberately broken contract.
General-principle checks include numeric boundaries, full-input parsing, independent
value copies, and controlled time. These are focused demonstrations, not exhaustive
proofs, stress tests, race-detector runs, or production integration tests.

## A Practical Way to Choose

Start by describing the requirement without naming a pattern. Identify what changes,
who owns state, who initiates calls, and which failures are possible. Prefer a
straightforward function or value type until a concrete need justifies another
abstraction. Then select the pattern whose intent matches that need, and test the
contract that makes it useful.

Being able to explain why a pattern is unnecessary is part of understanding it.