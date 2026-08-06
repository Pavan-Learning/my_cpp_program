# Thread-Safe Async Logger - Design Document

## Overview

An asynchronous, thread-safe logging system that decouples log message production from consumption using a producer-consumer pattern with a background worker thread.

## Architecture

### Data Flow

```
Producer Threads                     Consumer Thread
─────────────────                    ───────────────
                                    
  Thread 1 ─┐                       ┌─── LoggerThread ──── LegacyLogger
  Thread 2 ──┼── LoggerAdapter ──── ThreadSafeQueue ───┘      (Singleton)
  Thread N ─┘     (ILogger)         (bounded: 100)
                     │                     │
                  push(msg)             pop(msg)
                     │                     │
              ┌──────┴──────┐       ┌──────┴──────┐
              │ If queue is │       │ If queue is │
              │ full: BLOCK │       │ empty: WAIT │
              └─────────────┘       └─────────────┘
```

### Component Responsibilities

| Component | Role |
|-----------|------|
| **LoggerAdapter** | Public API (`log`, `flush`, `shutdown`). Pushes messages into the queue |
| **ThreadSafeQueue** | Bounded thread-safe buffer (max 100). Blocks producers when full, blocks consumer when empty |
| **LoggerThread** | Background worker. Pops messages and forwards to LegacyLogger |
| **LegacyLogger** | Singleton. Performs the actual log write with a thread-safe counter |

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
        -condition_variable cv
        -bool workerthreadbusy
        -mutex mtx
        -ThreadSafeQueue& logQueue
        -LegacyLogger& legacyLogger
        +LoggerThread(ThreadSafeQueue&, LegacyLogger&)
        +~LoggerThread()
        +start()
        +stop()
        +waitToFinishFlush()
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

### 1. Construction & Logging

```mermaid
sequenceDiagram
    participant C as Client Thread
    participant LA as LoggerAdapter
    participant Q as ThreadSafeQueue
    participant LT as LoggerThread (worker)
    participant LL as LegacyLogger

    rect rgb(230, 245, 230)
    Note right of LA: CONSTRUCTION
    LA->>LT: start()
    LT->>LT: spawn workerThread
    LT-->>Q: pop() [blocks, waiting for data]
    end

    rect rgb(230, 240, 255)
    Note right of C: LOG MESSAGE (non-blocking for producer)
    C->>LA: log(INFO, "msg")
    LA->>Q: push("msg", INFO)
    Note right of Q: if queue full (≥100):<br/>producer blocks until space
    Q-->>LT: cv.notify_one() [wake consumer]
    LT->>Q: pop() → {msg, INFO}
    Note right of LT: sets workerthreadbusy = true
    LT->>LL: writeLog(INFO, "msg")
    Note right of LT: sets workerthreadbusy = false
    end
```

### 2. Flush & Shutdown

```mermaid
sequenceDiagram
    participant C as Client Thread
    participant LA as LoggerAdapter
    participant Q as ThreadSafeQueue
    participant LT as LoggerThread (worker)

    rect rgb(255, 245, 220)
    Note right of C: FLUSH (blocks caller until queue drained)
    C->>LA: flush()
    LA->>LT: waitToFinishFlush()
    Note right of LT: cv.wait() until:<br/>queue is empty AND<br/>worker is not busy
    LT->>LT: worker finishes processing
    LT-->>LT: cv.notify_all()
    LT-->>LA: return
    LA-->>C: flush complete
    end

    rect rgb(255, 230, 230)
    Note right of C: SHUTDOWN (stops worker permanently)
    C->>LA: shutdown()
    LA->>LT: stop()
    LT->>Q: setShutdown()
    Q-->>LT: cv.notify_all() [unblock consumer]
    Note right of LT: pop() returns nullopt<br/>processLogs() loop exits
    LT->>LT: workerThread.join()
    LT-->>LA: stopped
    end
```

## Thread Safety Mechanisms

| Component | Mechanism | Details |
|-----------|-----------|---------|
| `ThreadSafeQueue` | `std::mutex` + `std::condition_variable` | Guards all queue operations; `cv.wait()` blocks consumer when queue is empty |
| `LegacyLogger` | `std::atomic<int>` for logcount | Meyers' Singleton guarantees thread-safe initialization |
| `LoggerAdapter` | Single consumer thread | Only one thread reads from the queue, avoiding contention on output |
| Queue bounded size | Max 100 entries | Blocks producers via `cv.wait()` when full (back-pressure) |
| `LoggerThread` | `std::mutex` + `std::condition_variable` | `workerthreadbusy` flag allows flush to wait until worker is idle and queue is empty |

## Flush Protocol

1. `LoggerAdapter::flush()` calls `LoggerThread::waitToFinishFlush()`
2. `waitToFinishFlush()` waits on `cv` until `logQueue.isEmpty() && !workerthreadbusy`
3. `processLogs()` sets `workerthreadbusy = true` before writing, `false` after, and calls `cv.notify_all()` when queue is empty and not busy
4. `flush()` returns once all queued messages have been written

## Shutdown Protocol

1. `LoggerAdapter::shutdown()` calls `LoggerThread::stop()`
2. `stop()` calls `logQueue.setShutdown()` — sets shutdown flag and notifies all waiters
3. `processLogs()` loop exits when `pop()` returns `std::nullopt`
4. Worker thread is joined via `workerThread.join()`
5. Destructor `~LoggerAdapter()` also calls `shutdown()` as a safety net

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
