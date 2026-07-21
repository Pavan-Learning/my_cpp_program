# Singleton Design Pattern

## Overview

Singleton ensures a class has **only one instance** and provides a **global point of access** to it.

## Class Diagram

```mermaid
classDiagram
    class Singleton {
        -static instance : Singleton*
        -static count : int
        -Singleton()
        +static getInstance() Singleton*
    }

    class ThreadSafeSingleton {
        -static instance : ThreadSafeSingleton*
        -static mtx : mutex
        -ThreadSafeSingleton()
        +static getInstance() ThreadSafeSingleton*
    }

    class SingletoneWithCallOnce {
        -static instance : SingletoneWithCallOnce*
        -static flag : once_flag
        -SingletoneWithCallOnce()
        +static getInstance() SingletoneWithCallOnce*
    }

    class MeyersSingleton {
        -MeyersSingleton()
        +static getInstance() MeyersSingleton&
        +~MeyersSingleton()
        +MeyersSingleton(const MeyersSingleton&) = delete
        +operator=(const MeyersSingleton&) = delete
    }
```

## Implementations

### 1. Basic Singleton (Not Thread-Safe)

```mermaid
sequenceDiagram
    participant Client1
    participant Client2
    participant Singleton

    Client1->>Singleton: getInstance()
    Note over Singleton: instance == nullptr → create new
    Singleton-->>Client1: returns instance

    Client2->>Singleton: getInstance()
    Note over Singleton: instance != nullptr → reuse
    Singleton-->>Client2: returns same instance
```

- Uses `static Singleton* instance` and checks for `nullptr`
- Simple but **NOT safe** when multiple threads call `getInstance()` at the same time

### 2. Thread-Safe Singleton (Double-Checked Locking)

```mermaid
sequenceDiagram
    participant Thread1
    participant Thread2
    participant Singleton
    participant Mutex

    Thread1->>Singleton: getInstance()
    Note over Singleton: instance == nullptr (first check)
    Thread1->>Mutex: lock()
    Note over Singleton: instance == nullptr (second check) → create new
    Thread1->>Mutex: unlock()
    Singleton-->>Thread1: returns instance

    Thread2->>Singleton: getInstance()
    Note over Singleton: instance != nullptr (first check) → skip lock
    Singleton-->>Thread2: returns same instance
```

- First `if` avoids locking every time (fast path)
- Second `if` inside the lock prevents double creation
- Uses `std::mutex` + `std::lock_guard`

### 3. Singleton with `std::call_once`

```mermaid
sequenceDiagram
    participant Thread1
    participant Thread2
    participant Singleton
    participant call_once

    Thread1->>Singleton: getInstance()
    Thread1->>call_once: call_once(flag, lambda)
    Note over call_once: flag not set → run lambda → create instance
    Singleton-->>Thread1: returns instance

    Thread2->>Singleton: getInstance()
    Thread2->>call_once: call_once(flag, lambda)
    Note over call_once: flag already set → skip
    Singleton-->>Thread2: returns same instance
```

- `std::call_once` guarantees the lambda runs exactly once
- Thread-safe, simpler than manual mutex locking
- Requires C++11

### 4. Meyers' Singleton (Recommended)

```mermaid
sequenceDiagram
    participant Client1
    participant Client2
    participant getInstance

    Client1->>getInstance: call
    Note over getInstance: static local variable created (first call only)
    getInstance-->>Client1: returns reference

    Client2->>getInstance: call
    Note over getInstance: static local already exists → reuse
    getInstance-->>Client2: returns same reference

    Note over getInstance: Destructor called automatically at program exit
```

- Uses a **local static variable** — no `new`, no pointers
- Thread-safe by C++11 standard
- Automatic cleanup (destructor runs at exit)
- Copy and assignment are deleted

## Comparison

| Feature | Basic | Thread-Safe (Mutex) | call_once | Meyers' |
|---|---|---|---|---|
| Thread-safe | No | Yes | Yes | Yes |
| Uses `new` | Yes | Yes | Yes | **No** |
| Auto cleanup | No (leak) | No (leak) | No (leak) | **Yes** |
| Complexity | Simple | Medium | Medium | **Simple** |
| C++ version | Any | C++11 | C++11 | C++11 |

## Drawbacks

```mermaid
graph TD
    S[Singleton Pattern]
    S --> D1[Global State]
    S --> D2[Hard to Test]
    S --> D3[Hidden Dependencies]

    D1 --> D1a["Any code can modify the instance"]
    D1a --> D1b["Bugs are hard to track"]

    D2 --> D2a["Tests share the same instance"]
    D2a --> D2b["Tests become interdependent"]

    D3 --> D3a["Dependencies not visible in function signatures"]
    D3a --> D3b["Fix: Use Dependency Injection instead"]
```

### Drawback 1: Global State
`GlobalCounter` — multiple functions modify the same counter without knowing about each other.

### Drawback 2: Hard to Test
`Logger` — `test1()` and `test2()` share the same logger, so log counts leak between tests.

### Drawback 3: Hidden Dependencies
`OrderProcessor` — calls `Logger::getInstance()` internally, but the constructor doesn't reveal this. Better approach: pass `Logger&` as a parameter (Dependency Injection).

## When to Use Singleton
- Database connection pools
- Configuration managers
- Hardware interface access (e.g., printer spooler)
- Logging systems (when DI is not practical)
