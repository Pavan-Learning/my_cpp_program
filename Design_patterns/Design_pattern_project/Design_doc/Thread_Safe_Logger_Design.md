# Thread-Safe Async Logger - Design Document

## Overview

An asynchronous, thread-safe logging system that decouples log message production from consumption using a producer-consumer pattern with a background worker thread.

## Architecture

```
┌────────────┐       ┌────────────────┐       ┌─────────────────┐       ┌──────────────┐       ┌──────────────┐
│   Client   │──────▶│  LoggerAdapter │──────▶│ ThreadSafeQueue │──────▶│ LoggerThread │──────▶│ LegacyLogger │
│  (Threads) │ log() │  (ILogger)     │ push()│  (bounded:100)  │ pop() │  (worker)    │write()│  (Singleton) │
└────────────┘       └────────────────┘       └─────────────────┘       └──────────────┘       └──────────────┘
```

## Class Diagram

```mermaid
classDiagram
    direction TB

    class LogLevel {
        <<enumeration>>
        DEBUG
        INFO
        WARNING
        ERROR
        CRITICAL
    }

    class ILogger {
        <<interface>>
        +~ILogger()
        +log(LogLevel, string)*
        +flush()*
        +shutdown()*
    }

    class LoggerAdapter {
        -ThreadSafeQueue logQueue
        -LoggerThread loggingThread
        +LoggerAdapter()
        +~LoggerAdapter()
        +log(LogLevel, string)
        +flush()
        +shutdown()
    }

    class ThreadSafeQueue {
        -queue~pair~string,LogLevel~~ logQueue
        -mutable mutex mtx
        -condition_variable cv
        -bool shutdown
        +push(string, LogLevel)
        +pop() optional~pair~string,LogLevel~~
        +getSize() size_t
        +isEmpty() bool
        +setShutdown()
    }

    class LoggerThread {
        -thread workerThread
        -ThreadSafeQueue& logQueue
        -LegacyLogger& legacyLogger
        +LoggerThread(ThreadSafeQueue&, LegacyLogger&)
        +~LoggerThread()
        +start()
        +stop()
        -processLogs()
    }

    class LegacyLogger {
        <<Singleton>>
        -atomic~int~ logcount
        -LegacyLogger()
        +static getInstance() LegacyLogger&
        +writeLog(LogLevel, string)
    }

    class LoggerFactory {
        <<abstract>>
        +createLogger()* unique_ptr~ILogger~
        +~LoggerFactory()
    }

    class LoggerAdapterFactory {
        +createLogger() unique_ptr~ILogger~
    }

    ILogger <|.. LoggerAdapter : implements
    LoggerAdapter *-- ThreadSafeQueue : owns
    LoggerAdapter *-- LoggerThread : owns
    LoggerThread --> ThreadSafeQueue : reads from
    LoggerThread --> LegacyLogger : writes to
    LoggerFactory <|-- LoggerAdapterFactory : extends
    LoggerAdapterFactory ..> LoggerAdapter : creates
    ILogger ..> LogLevel : uses
    LegacyLogger ..> LogLevel : uses
```

## Design Patterns Used

| Pattern | Component | Purpose |
|---------|-----------|---------|
| **Adapter** | `LoggerAdapter` | Adapts `LegacyLogger` behind `ILogger` interface, adding async behavior |
| **Singleton** | `LegacyLogger` | Single shared logging instance (thread-safe via Meyers' singleton) |
| **Factory Method** | `LoggerFactory` / `LoggerAdapterFactory` | Decouples logger creation from usage |
| **Producer-Consumer** | `ThreadSafeQueue` + `LoggerThread` | Async decoupling of log producers from the log writer |

## Sequence Diagram

```mermaid
sequenceDiagram
    participant Client
    participant LoggerAdapter
    participant ThreadSafeQueue
    participant LoggerThread
    participant LegacyLogger

    Note over LoggerAdapter,LoggerThread: Construction
    LoggerAdapter->>LoggerThread: start()
    LoggerThread->>LoggerThread: spawn workerThread

    Note over Client,LegacyLogger: Logging (non-blocking)
    Client->>LoggerAdapter: log(INFO, "msg")
    LoggerAdapter->>ThreadSafeQueue: push("msg", INFO)
    ThreadSafeQueue-->>LoggerThread: cv.notify_one()
    LoggerThread->>ThreadSafeQueue: pop()
    ThreadSafeQueue-->>LoggerThread: {msg, INFO}
    LoggerThread->>LegacyLogger: writeLog(INFO, "msg")

    Note over Client,LegacyLogger: Shutdown
    Client->>LoggerAdapter: shutdown()
    LoggerAdapter->>LoggerAdapter: flush() [wait queue empty]
    LoggerAdapter->>LoggerThread: stop()
    LoggerThread->>ThreadSafeQueue: setShutdown()
    ThreadSafeQueue-->>LoggerThread: cv.notify_all()
    LoggerThread->>LoggerThread: join workerThread
```

## Thread Safety Mechanisms

| Component | Mechanism | Details |
|-----------|-----------|---------|
| `ThreadSafeQueue` | `std::mutex` + `std::condition_variable` | Guards all queue operations; `cv.wait()` blocks consumer when queue is empty |
| `LegacyLogger` | `std::atomic<int>` for logcount | Meyers' Singleton guarantees thread-safe initialization |
| `LoggerAdapter` | Single consumer thread | Only one thread reads from the queue, avoiding contention on output |
| Queue bounded size | Max 100 entries | Drops oldest entry when full (prevents memory exhaustion) |

## Shutdown Protocol

1. `LoggerAdapter::shutdown()` calls `flush()` — spins until queue is empty
2. `LoggerThread::stop()` calls `logQueue.setShutdown()` — sets shutdown flag and notifies all waiters
3. `LoggerThread::processLogs()` loop exits when `pop()` returns `std::nullopt`
4. Worker thread is joined via `workerThread.join()`

## File Structure

```
Design_pattern_project/
├── CMakeLists.txt
├── inc/
│   ├── log_levels.hpp          # LogLevel enum
│   ├── logger_interface.hpp    # ILogger abstract class
│   ├── Logger_adapter.hpp      # LoggerAdapter declaration
│   ├── Logger_thread.hpp       # LoggerThread declaration
│   ├── Thread_safe_queue.hpp   # ThreadSafeQueue declaration
│   ├── legacy_logger.hpp       # LegacyLogger (Singleton) declaration
│   └── Logger_factory.hpp      # Factory classes
└── src/
    ├── main.cpp                # Entry point + stress test
    ├── Logger_adapter.cpp      # LoggerAdapter implementation
    ├── Logger_thread.cpp       # LoggerThread implementation
    ├── Thread_safe_queue.cpp   # ThreadSafeQueue implementation
    └── legacy_logger.cpp       # LegacyLogger implementation
```

## Build & Run

```bash
cd Design_patterns/Design_pattern_project
cmake -B build && cmake --build build
./build/threading
```
