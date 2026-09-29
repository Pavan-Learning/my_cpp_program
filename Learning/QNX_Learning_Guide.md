# QNX Explained: OS, Threads, Messages, and Real-Time Systems

Concept-by-concept lessons with plain-language explanations, execution diagrams, worked calculations, complete example programs, and questions with explained answers.

**Depth:** the lessons progress from beginner mental models to implementation reasoning and architect-level decisions. An architect-level explanation states its assumptions, compares alternatives, identifies failure paths, and describes the evidence needed to justify a decision. These sections teach that reasoning; reading them alone does not establish professional mastery or qualify a production system.

**The teaching material is in this document.** Section 0 explains the OS and threading foundations, including privilege, system calls, process lifecycle, virtual memory, files, I/O, scheduling, and isolation. Sections 2 and 6-19 develop the ideas in detail, using a sensor-and-logger system to explain why each mechanism exists, how it works, and what can go wrong. The vocabulary tables are references; you do not need to memorize them before reading the lessons. Exercises come after the explanations, not in place of them.

**Scope of this course:** OS and C++ concurrency foundations, QNX application architecture, native IPC, timing, resource-manager semantics, system integration, and worked architectural analysis. It cannot supply every board's driver implementation, a production security policy, or a safety qualification. Complete runnable examples are labelled separately from conceptual traces and design exercises. Linux results never stand in for QNX execution evidence.

**Reference baseline:** QNX Software Development Platform (SDP) 8.0. Projects using QNX Neutrino 7.x must use their matching documentation, SDK, BSP, and target image. Exact API contracts, board-specific procedures, and certification requirements still require the applicable official documentation.

**Assumed starting point:** basic C++ variables, functions, and compiling a small program. No previous OS, threading, or QNX knowledge is required for the beginner foundation. Start with section 0; use the later terminology tables as references, not as a list to memorize first.

**No QNX license?** You can read all these explanations and run the portable examples on Linux now. See [licensing and access](#licensing-and-access) for the official non-commercial route and [Linux practice without QNX](#linux-practice-without-qnx) for the boundary between portable practice and native execution. A Linux experiment is evidence about that experiment, not a native QNX test.

**Validation status:** the eight Linux-runnable programs (first thread, protected counter, POSIX process lifecycle, one-sample handoff, bounded queue, periodic timing, sensor semantics, and frame decoder) were compiled with C++17 and warnings treated as errors, then run successfully on Linux. The process-lifecycle example additionally requires POSIX facilities and the lab's `/bin/sh`; it is not ISO C++ alone. Document navigation and local links were checked. The native QNX example and target-only labs have not been compiled or run here: no QNX compiler is available on this workspace's current PATH. The snapshot and frame-decoder programs test application semantics, not native resource-manager dispatch or a complete network service.

## Contents

0. [Start from zero: OS and threading](#0-start-from-zero-os-and-threading)

    OS foundations within this chapter: [privilege and system calls](#016-user-mode-kernel-mode-and-protection), [interrupts and faults](#018-interrupts-exceptions-and-signals), [process lifecycle](#020-how-a-process-starts-and-ends), [process example](#022-complete-linux-example-start-and-reap-a-child), [virtual memory](#023-a-process-address-space-is-a-map), [paging and TLBs](#024-pages-page-tables-and-address-translation), [file descriptors](#028-pathnames-descriptors-and-open-file-descriptions), and [blocking I/O](#030-blocking-nonblocking-and-asynchronous-io).

    Further OS concepts: [scheduling algorithms](#033-scheduling-algorithms-optimize-different-goals), [deadlock conditions](#034-the-four-deadlock-conditions-and-what-they-mean), [IPC choices](#035-choose-ipc-by-the-communication-contract), [permissions](#036-users-permissions-and-resource-limits), [containers and VMs](#037-containers-virtual-machines-and-the-host-kernel), and [worked OS questions](#038-worked-os-questions-that-connect-the-mechanisms).

1. [Reading the examples](#1-reading-the-examples)
2. [What QNX is](#2-what-qnx-is)
3. [Vocabulary](#3-vocabulary)
4. [Set up your learning environment](#4-set-up-your-learning-environment)
5. [How the concepts fit together](#5-how-the-concepts-fit-together)
6. [Processes, threads, and ownership](#6-processes-threads-and-ownership)
7. [Real-time scheduling](#7-real-time-scheduling)
8. [Thread synchronization](#8-thread-synchronization)
9. [Worked example: bounded queue](#9-worked-example-bounded-queue)
10. [Native QNX message passing](#10-native-qnx-message-passing)
11. [Worked example: QNX request and reply](#11-worked-example-qnx-request-and-reply)
12. [Pulses, events, and timers](#12-pulses-events-and-timers)
13. [Shared memory and IPC selection](#13-shared-memory-and-ipc-selection)
14. [Resource managers](#14-resource-managers)
15. [Memory and C++ discipline](#15-memory-and-c-discipline)
16. [Boot, BSPs, and drivers](#16-boot-bsps-and-drivers)
17. [Filesystems and networking](#17-filesystems-and-networking)
18. [Debugging and performance](#18-debugging-and-performance)
19. [Security, reliability, and safety](#19-security-reliability-and-safety)
20. [Practice workbook](#20-practice-workbook)
21. [Hints and answer checkpoints](#21-hints-and-answer-checkpoints)
22. [Capstone: supervised telemetry logger](#22-capstone-supervised-telemetry-logger)
23. [Interview and revision questions](#23-interview-and-revision-questions)
24. [Troubleshooting](#24-troubleshooting)
25. [Official reading route](#25-official-reading-route)
26. [Completion checklist](#26-completion-checklist)

## 0. Start from Zero: OS and Threading

Read this section in order before the QNX architecture, vocabulary, or native IPC chapters. The same sensor-and-logger example will connect the ideas. You can run the C++ examples on Linux without a QNX license.

### 0.1 What is the computer actually doing?

Three hardware ideas are enough to begin:

- **CPU:** the processor executes instructions, such as adding numbers, comparing values, and moving data.
- **RAM:** working memory holds instructions and data being used. Ordinary RAM generally loses its contents when power is removed.
- **Storage:** an SSD, disk, or other storage device holds files beyond a program's execution. Reading or writing storage can take much longer than accessing RAM.

A **CPU core** is a unit capable of executing a stream of instructions. For our first examples, assume one core runs one software thread at a time. Real processors may also support hardware multithreading; we do not need that complication yet.

If a program reads a sensor value, adds an offset, and writes a log, those are different kinds of work. The arithmetic needs CPU execution. Reading and writing may require waiting for a device or another service.

### 0.2 Why do we need an operating system?

An **operating system (OS)** manages the machine so applications can run and use resources through controlled interfaces. Linux and QNX are different operating systems.

Imagine a sensor application, a logger, and a terminal all need the CPU, memory, and devices. The OS helps answer:

1. Which runnable work gets a CPU next?
2. Which memory may each application access?
3. How can a program read a device or write a file?
4. How does a program wait for input without constantly using the CPU?
5. How do applications communicate and release resources when they finish?

```text
Your C++ application: "write these bytes to this open file"
                                                 |
                            libraries and OS interfaces
                                                 |
                         OS services and device support
                                                 |
                                        storage hardware
```

Your program usually requests an operation instead of controlling the device directly. The **kernel** is the privileged core of the OS. Some OS services may be outside the kernel; QNX relies extensively on separate service processes. Not every C++ function call enters the kernel: adding two integers normally does not require an OS request.

**Check:** why should three unrelated applications not each decide they own all physical memory? Because they could overwrite one another and corrupt the system. Controlled memory access provides isolation.

### 0.3 Program, process, and thread: three different things

A **program** is the instructions stored in an executable file. A **process** is a running instance of a program, with an address space and resources. A **thread** is one path of execution inside that process.

```text
Program stored on disk: sensor_application
                                |
                    launch the program
                                |
Process: one running instance
        memory and resources belonging to this instance
        main thread: executes the application's instructions
```

Launching the same executable twice normally creates two processes. Calling a function twice does not create two processes. A process starts with an initial thread and can create additional threads.

Think of a process as a workspace and threads as activities within it. Activities can use the same workspace, but this analogy does not explain protection or synchronization by itself; the OS and language rules define those.

| Action | What happens |
|---|---|
| Store a compiled executable | A program exists; it need not be running |
| Launch that executable | A process begins execution |
| Call `read_sensor()` normally | The calling thread enters that function and later returns |
| Create `std::thread` with a function | A new execution path runs that function, subject to scheduling |
| Finish one worker thread | That thread ends; other threads in the process can continue |

A **PID** identifies a process. A **TID** identifies a thread in the relevant OS/tool context. These are identifiers, not priorities, memory sizes, or CPU numbers.

### 0.4 What memory do threads share?

An **address space** is a process's view of memory addresses. Threads in the same process share that address space. Separate processes normally have separate address spaces, unless they explicitly arrange shared mappings.

```text
Process A
    shared objects: queue, configuration, heap allocations
    main thread:   its own stack and execution position
    worker thread: its own stack and execution position

Process B
    separate address space and its own threads
```

A **stack** holds call-related state, including many local variables. A **heap allocation** is dynamically managed storage, for example storage obtained through `new` or by a container. Each thread has its own stack, but stack memory is not automatically inaccessible to other threads: passing a reference to a local variable lets another thread access it while its lifetime lasts.

This gives us two responsibilities:

- **Lifetime:** keep an object alive as long as another thread may use it.
- **Synchronization:** coordinate accesses when threads can touch shared state at overlapping times.

**Check:** can a worker safely use a reference to a local variable after the function owning that variable returns? No. The reference would outlive the object. A separate stack is not a lifetime guarantee for borrowed data.

### 0.5 First working example: start a thread and join it

Use the lab filename `first_thread.cpp`. This is a complete Linux C++ example:

```cpp
#include <iostream>
#include <thread>

int main() {
        int result = 0;

        std::thread worker([&result] {
                result = 7;
        });

        worker.join();
        std::cout << "result=" << result << '\n';
        return result == 7 ? 0 : 1;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror -pthread first_thread.cpp -o first_thread
./first_thread
```

Expected output: `result=7`.

Read the program in this order:

1. The main thread creates `result`, initially zero.
2. `[&result]` is a C++ lambda capture: the small function uses the existing `result` by reference, rather than making a separate value copy.
3. Constructing `worker` starts a new thread to execute the lambda. That thread can begin before the main thread reaches its next statement; do not assume a particular start order.
4. The worker writes 7 and finishes.
5. `join()` waits for that worker to finish if it has not already finished. It also provides the synchronization needed for the later read of `result`.
6. Only after joining does the main thread read and print `result`.

There is no need for a mutex here: the main thread does not access `result` while the worker can be writing it, and joining establishes the required ordering. This does **not** mean joining afterward would repair data races that already occurred between workers.

`join()` does not start, stop, or kill the worker. It waits for completion. A **joinable** C++ thread object still needs appropriate lifecycle handling even if its thread function has returned; destroying it while joinable invokes `std::terminate()`. For these exercises, join your workers rather than detaching them and losing clear lifetime control.

These small examples assume thread creation succeeds. Production code also needs a plan for partial startup failure.

### 0.6 How can two threads run on one CPU core?

The OS **scheduler** selects runnable threads. On a single core under our simple model, only one runs at an instant, but the OS can switch between them. That is **concurrency**: their lifetimes overlap. With enough eligible cores, they may also run simultaneously, which is **parallelism**.

```text
One core, one possible timeline:
time ->  main work | worker work | main work | worker work

Two cores, a possible timeline:
core 1:  main work -------------------->
core 2:  worker work ------------------>
```

The exact ordering is not guaranteed by writing one thread constructor before another. Saving and restoring execution state to change the running thread is a **context switch**.

Three useful states:

| State | Meaning | Sensor example |
|---|---|---|
| RUNNING | Executing on a CPU now | Calculating a sample value |
| READY | Able to execute, waiting to be selected | Worker has data but another thread is executing |
| Blocked | Cannot continue until a condition/event is satisfied | Waiting for an empty queue to receive data |

A blocked thread normally does not consume CPU executing its own instructions while it waits. When its wait condition is satisfied, it can become READY, not necessarily immediately RUNNING.

**Sleeping** waits for time-related wakeup eligibility. **Busy waiting** repeatedly checks in executing code. **Joining** waits for another thread's completion. They are not interchangeable. Sleeping for 100 ms does not prove that a worker has finished, because it may be delayed longer.

### 0.7 Why shared data needs coordination

Suppose two threads both want to increment one counter. At the level of a conceptual read-modify-write, this bad ordering is possible without coordination:

| Step | Thread A | Thread B | Stored counter |
|---|---|---|---|
| 1 | Reads 0 | | 0 |
| 2 | | Reads 0 | 0 |
| 3 | Computes 1 and writes it | | 1 |
| 4 | | Computes 1 and writes it | 1 |

Two intended increments produced only one net increment in this illustration. **Important C++ limit:** actual unsynchronized conflicting accesses to an ordinary `int` form a data race and cause undefined behavior. The language does not promise just this lost-update result. Do not run unsafe code expecting a reliable demonstration of one particular wrong answer.

**Synchronization** means coordinating access and ordering so the participants can safely share state. A **mutex** is an ownership-based mutual-exclusion object: while one participant owns its lock, others following the same locking protocol cannot enter the protected region.

The protected code is a **critical section**. Every participant accessing the shared state must obey the agreed protocol. A mutex sitting beside an unprotected read does not protect that read.

### 0.8 Second working example: protect a shared counter

Use the lab filename `protected_counter.cpp`:

```cpp
#include <iostream>
#include <mutex>
#include <thread>

int main() {
        int counter = 0;
        std::mutex counter_mutex;

        auto increment = [&counter, &counter_mutex] {
                for (int iteration = 0; iteration < 10000; ++iteration) {
                        std::lock_guard<std::mutex> lock(counter_mutex);
                        ++counter;
                }
        };

        std::thread first_worker(increment);
        std::thread second_worker(increment);

        first_worker.join();
        second_worker.join();

        std::cout << "counter=" << counter << '\n';
        return counter == 20000 ? 0 : 1;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror -pthread protected_counter.cpp -o protected_counter
./protected_counter
```

Expected output: `counter=20000`.

`std::lock_guard` acquires the mutex when its object is constructed and releases it when the object leaves scope. Here that scope ends at the end of each loop iteration. This is an example of **RAII**, C++ lifetime-based resource management; you do not manually unlock in every branch.

Two different problems are solved separately:

- The mutex coordinates increments that could otherwise overlap between the two workers.
- Joining both workers ensures they have finished before the main thread reads the total or destroys their referenced objects.

The final read needs no lock because both writers have completed and been joined. Reading the counter during their execution would require the same agreed synchronization. A carefully chosen atomic counter could be an alternative for this simple operation, but it would not automatically protect a larger queue or several related fields.

### 0.9 How does one thread wait for another to produce data?

Now replace the counter with a queue. A **producer** inserts records. A **consumer** removes them. If the queue is empty, the consumer has no useful work yet.

A mutex prevents simultaneous invalid queue access, but it does not alone provide the whole "wait until there is data" protocol. A **condition variable** lets the consumer wait while releasing the associated mutex so the producer can change the state.

```text
Consumer:
    acquire queue mutex
    while queue is empty and not closed:
            wait, releasing the mutex as part of the wait
            resume only with the mutex reacquired; recheck the condition
    take an item, or finish if closed and empty
    release mutex

Producer:
    acquire the same queue mutex
    insert an item if the queue accepts it
    release mutex
    notify a waiter that the state may have changed
```

This is pseudocode, not a complete queue implementation. A **predicate** is the true/false condition being checked, such as "closed or data available." A **notification** tells waiters to recheck; it is not the data and is not saved like a queue item for future waiters.

Why recheck? Another consumer may take the item first, or a wait may wake without the desired change, called a **spurious wakeup**. Correctness comes from the protected predicate, not from assuming each notification means one item belongs to you.

Do not wait while retaining a lock that the producer must acquire to make progress. Also, do not hold the queue mutex during slow file output: remove the item safely, release the mutex, then process it when the design permits.

### 0.10 Shutdown and deadlock, explained separately

**Shutdown** is the planned way to finish:

1. Stop accepting new work according to policy.
2. Change the shared closed/stop state under its synchronization.
3. Wake consumers waiting for data and producers waiting for space.
4. Drain accepted items or discard them according to the requirement.
5. Join workers.
6. Destroy the queue, locks, and other resources only after use has ended.

Calling `join()` without giving an indefinitely waiting worker a way to finish may wait indefinitely too.

**Deadlock** is an unresolvable wait cycle. Example: A holds lock X and waits for Y; B holds Y and waits for X. Each is waiting for the other. A consistent lock order can prevent this particular cycle; other dependency cycles require their own analysis.

**Priority** concerns which eligible thread should run first; it does not remove a deadlock. **Priority inversion** is a different problem: important work waits on a resource held by lower-priority work, which can itself be delayed. Read the detailed scheduling section after you understand basic waiting and ownership.

### 0.11 Where QNX fits into this picture

The general ideas above apply to Linux and QNX. QNX adds native mechanisms you will study after the foundations:

```text
Same process:
    acquisition thread -> shared queue -> logger thread
                                                 mutex/CV

Separate processes:
    sensor process -> defined communication -> logger process
                                                IPC
```

**IPC** means interprocess communication. Separate processes need an explicit way to exchange data because one process cannot treat another's normal variables as its own.

In QNX native messaging, a server provides a **channel** to receive requests. A client creates a **connection** to that channel and sends a request. The server receives it and replies. The client can wait for receipt and then reply. This is a cross-process protocol, not simply a normal function call or a shared queue.

A **real-time** requirement adds "by when?" to "is the result correct?" A sensor application can calculate the right answer but fail its requirement if the answer arrives after its deadline. Threads help structure the work, but neither creating threads nor using QNX automatically proves the deadline.

### 0.12 Beginner exercises and answers

| Exercise | What to do | Answer/checkpoint |
|---|---|---|
| B1: identify the objects | Describe a stored executable, two launches, and an extra thread in one launch | One stored program, two processes, an additional execution path inside one process |
| B2: trace a normal call | Compare calling a function normally with constructing a thread for it | The normal call stays in the caller's execution path; a new thread has a separately scheduled path |
| B3: run the first thread | Build section 0.5 and explain why the output is 7 | The worker writes, then successful join orders completion before the main read |
| B4: run the protected counter | Build section 0.8 and identify both kinds of synchronization | Mutex protects overlapping increments; joins precede the final read and destruction |
| B5: reason about waiting | Consumer waits for an empty queue; producer later inserts a record | Wait releases the mutex; notification prompts a predicate recheck after reacquiring it |
| B6: design shutdown | Worker waits indefinitely; main wants to join it | Publish stop/close and wake the worker so it can finish before joining |
| B7: separate processes | Explain how a separate logger receives a sensor record | Define IPC; do not pass an ordinary pointer and assume shared memory |

Read B1-B3 first, then B4-B6, then B7. Do not rush to native IPC signatures before you can explain these. A useful self-test is to teach each answer aloud without using its abbreviation.

The following subsections explain execution and the OS mechanisms underneath these examples. Sections 0.16 onward develop the OS foundations before the later QNX-specific chapters. Consult section 3 when a term is unfamiliar; it is a reference, not a prerequisite vocabulary test.

### 0.13 From a C++ statement to an executing thread

Start with `result = 7;` from the first-thread example. In the C++ model, this changes the value of an object. The compiler translates the program into machine instructions, subject to the language rules and permitted optimizations. There need not be exactly one instruction for each source statement, and an optimized variable need not occupy a separate RAM location at every moment.

During execution, the processor keeps immediate working state in **registers**, small storage locations associated with execution. An instruction position indicates where execution continues. Function calls also require information such as where to return and how to access parameters and local state. A thread supplies a continuing execution context for this work.

When the OS switches from one thread to another, it preserves the required state of the outgoing thread and restores the incoming thread's state. The outgoing thread's function does not automatically restart from the beginning. It continues from its saved execution context when scheduled again.

The stack helps maintain call-related state. For example, main calls `read_sample()`, which calls `convert_units()`. Returning from `convert_units()` resumes `read_sample()`, and returning from that resumes main. These are nested calls in one thread, not three simultaneous activities. Creating another thread introduces another independently scheduled sequence of calls.

**Why this matters:** source order within one thread is not a complete schedule for multiple threads. Writing the worker constructor first does not mean the worker finishes before main executes its next statement. `join()` supplies the explicit completion relationship that source placement alone cannot provide.

### 0.14 Visibility means more than the bytes existing in RAM

Modern processors may keep copies of memory data in **caches** to reduce access costs. Compilers and processors also use optimizations whose behavior must fit the language and platform contracts. Therefore "the other core will eventually see the RAM update" is not a sufficient explanation of C++ thread safety.

The C++ memory model defines when operations in different threads have the required ordering. A mutex, a successful join, or a correctly designed atomic protocol can provide that ordering. An ordinary shared integer does not provide it by itself.

In the first-thread example, the worker's write is ordered before main's read through successful joining. In the protected-counter example, mutex ownership coordinates overlapping increments. These are two different relationships: completion ordering and repeated mutually exclusive access.

Do not assume that a test on a single core removes the requirement. Threads can still be interleaved between parts of a larger operation, and the language's rules do not change simply because only one core is currently available. Likewise, disabling compiler optimization is not a synchronization protocol.

**Beginner question:** if the final answer looks correct 100 times, why worry? **Answer:** those runs tested only the executions that happened. Correctness must also explain allowed executions that did not occur during your test. Synchronization constrains those executions rather than relying on a fortunate schedule.

### 0.15 Separate the five questions in a concurrent design

Consider a worker that uses a sensor record. Five different questions are often confused:

| Question | Meaning | A mechanism that may help |
|---|---|---|
| Does the record still exist? | Lifetime | An owner that outlives all borrowers |
| Can two participants change it at once? | Exclusion | A consistently used mutex |
| Will the consumer observe the completed update? | Publication and ordering | Mutex synchronization or a proven atomic protocol |
| Will a waiting participant eventually proceed? | Progress | An available producer, wakeup, and eligible execution |
| Will it proceed soon enough? | Timing | Bounded dependencies and schedulability evidence |

A solution to one row is not automatically a solution to another. A live object can still be accessed unsafely. A race-free queue can still deadlock during shutdown. A deadlock-free service can still miss a deadline.

This distinction is the first step toward architectural reasoning: replace "the code is safe" with a precise statement about which property is established, under which assumptions. Later chapters apply these same five questions to processes, devices, and service recovery.

### 0.16 User mode, kernel mode, and protection

An OS must let applications use the machine without letting every application control the entire machine. Otherwise a faulty application could rewrite memory mappings, disable essential interrupts, or access devices belonging to another application.

Processors provide privilege mechanisms that the OS uses to restrict such actions. In a simplified model, ordinary application instructions execute in **user mode**, while the trusted OS core executes in **kernel mode**. Exact privilege levels and transitions depend on the CPU architecture; these two names describe the conceptual boundary.

The difference is permission, not simply speed. Adding two numbers does not become faster because a program is privileged. Kernel-mode code can perform operations and access resources that ordinary user-mode code is not allowed to control directly.

Memory mappings also carry access rules. A region can be readable but not writable, or readable/writable but not executable. If a program writes to a protected code page, the processor can raise a fault for the OS to handle. This protection is different from a C++ `const` declaration: language rules and hardware page permissions operate at different levels.

**Important distinction:** a process running as the administrative user is still normally executing application instructions in user mode. Its identity may authorize more OS requests, but it does not mean all of its instructions execute in kernel mode. User identity, OS authorization, and CPU execution privilege are related but separate concepts.

QNX uses privileged kernel mechanisms together with many user-space service processes. A filesystem or driver service being outside the kernel does not mean it has no permissions or cannot affect hardware. Its actual access and fault consequences depend on the granted capabilities and device configuration.

**Question:** why not run all application code in kernel mode to avoid transitions? **Answer:** the transition is part of enforcing a protection boundary. Removing it can let an application defect compromise the mechanisms protecting every participant. Any performance comparison must account for that change in the system's protection model.

### 0.17 A function call is not necessarily a system call

A normal function call transfers execution within the calling program's execution path. For example, an integer-conversion helper can run entirely in user space using data the process already owns.

A **system call** is a controlled request to an OS mechanism. The application supplies arguments through a defined calling convention, and a CPU-supported entry mechanism transfers control into an authorized OS entry point. The application cannot choose an arbitrary kernel address and execute it with full privilege.

A conceptual request has these stages:

```text
application prepares arguments
    -> library/API wrapper, where applicable
    -> controlled OS entry
    -> validate request and access rights
    -> perform work, delegate it, or arrange a wait
    -> report a result to the caller
```

The actual path depends on the OS and API. In QNX, familiar file APIs often involve messages to a resource manager, with kernel messaging mechanisms supporting the interaction. Do not assume the path is identical to Linux just because both provide `read()`.

Library names alone do not identify kernel entries. `std::cout` can format into user-space buffers and later invoke output operations. A memory allocator can satisfy a request from an existing arena without immediately asking the OS for more memory. Some platforms also provide selected information through user-space helper mappings without a conventional entry for every query.

The OS must treat arguments as untrusted. If an application supplies a buffer pointer and a length, the implementation must handle invalid access and appropriate bounds. Being an integer-sized pointer value does not establish that the referenced memory is accessible or belongs to the intended operation.

**Does a system call always switch threads?** No. A short request can enter the OS and return to the same thread. A request that must wait can block the caller and allow another thread to execute. A **mode transition** changes privilege context; a **thread context switch** changes which thread executes. They are not the same event.

**Question:** does one line containing `printf()` prove exactly one system call occurred? **Answer:** no. Formatting, buffering, output size, errors, and the library implementation affect the underlying operations. Observe or consult the implementation when the distinction matters.

### 0.18 Interrupts, exceptions, and signals

These terms all concern events that alter the usual execution path, but they arise at different layers.

An **interrupt** commonly reports an external hardware event relative to the currently executing instruction stream: a timer expires, a network interface has received data, or a device operation completes. The processor enters the appropriate handling path according to its interrupt architecture and current masking/priority rules.

A **processor exception** is associated with executing an instruction or an explicit controlled-entry mechanism. Examples include accessing an unmapped page, attempting a forbidden instruction, or using an architecture's designated system-call instruction. Terminology such as trap, fault, and exception varies across CPU manuals; learn the architecture-specific meaning before relying on those labels.

A **POSIX signal** is an OS-level notification directed according to process/thread signal rules. It is not the same as a hardware interrupt. An OS can translate a hardware-detected invalid access into a signal such as `SIGSEGV`, but not every signal begins with a processor fault.

| Situation | First mechanism involved | Possible outcome |
|---|---|---|
| A device finishes receiving data | Hardware interrupt | Driver handles completion and wakes waiting work |
| An instruction accesses a page needing OS handling | Processor fault | OS resolves the mapping condition or reports an invalid access |
| A user presses Ctrl+C in a terminal | Terminal/job-control behavior | Foreground process group normally receives `SIGINT` |
| A process requests another process terminate cooperatively | OS signal interface | Permitted target receives a signal such as `SIGTERM` |
| A QNX timer is configured for pulse delivery | Timer event | Pulse becomes available through the configured channel |

An interrupt handler must not assume the same environment as an ordinary application function. Its allowed operations, blocking behavior, and work budget depend on the attachment model. Long work is often deferred to an appropriate thread; the later driver chapter explains why acknowledgment and device ownership matter.

A signal handler also has restrictions. It can interrupt code that already holds an internal library lock, so casually calling an allocating or locking library function in the handler can deadlock or violate its contract. Use documented async-signal-safe operations or arrange supported synchronous signal handling in an ordinary thread. Check process-wide dispositions, per-thread masks, and delivery rules rather than assuming every signal is handled by main.

**Question:** are C++ exceptions the same as processor exceptions? **Answer:** no. C++ `throw` and stack unwinding are language/runtime mechanisms. An invalid pointer access does not portably become a catchable C++ exception.

### 0.19 Follow a blocked thread through the scheduler

The scheduler selects among eligible runnable threads. It cannot choose a thread that still lacks the event or resource needed to continue. A simplified state graph is:

```text
created -> READY -> RUNNING -> finished
             ^        |
             |        +-- waits for resource --> BLOCKED
             |                                    |
             +-------- event makes it ready ------+

RUNNING -- preempted while still runnable --> READY
```

Suppose a sensor thread calls a blocking read with no data available. The OS or service arranges its wait. The sensor is no longer a runnable contender for the CPU, so a logger thread can execute. When data arrives and the wait condition is satisfied, the sensor becomes eligible again. Whether it immediately runs depends on policy, priority, affinity, and other eligible work.

This explains why a blocked thread can have low CPU consumption despite taking a long elapsed time to finish. It is waiting, not executing its own instructions continuously. Conversely, a polling loop can use an entire CPU while accomplishing no useful work.

Schedulers maintain bookkeeping for runnable and waiting work. Textbooks often call thread-related bookkeeping a **Thread Control Block (TCB)** and process-related bookkeeping a **Process Control Block (PCB)**. These are conceptual names, not portable structures your application should inspect or edit. The OS tracks execution state, identifiers, scheduling information, mappings, and resource associations through its implementation.

A context switch has direct state-save/restore costs and possible indirect cache or translation effects. Switching between threads of one process need not change the address space. Switching between processes can require a different memory-translation context. Exact costs depend on hardware and OS implementation; there is no universal microsecond constant.

**Worked question:** a high-priority thread waits on an empty queue while a lower-priority producer is READY. Which thread can make progress? **Answer:** the producer must run to create the data. The high-priority thread cannot make the queue nonempty simply by having a higher priority.

### 0.20 How a process starts and ends

Launching a command is more than calling its `main()` from the shell. The launch mechanism establishes a process context and loads a compatible executable image. The loader and runtime arrange executable mappings, required libraries where applicable, initial stack state, arguments, and environment before control reaches the program's normal entry path and eventually `main()`.

An executable can fail before main because its format, interpreter/loader, or runtime dependencies do not match the environment. It can fail after main starts because permissions, configuration, or required services are unavailable. These are different failure stages.

In POSIX-oriented systems, three important API families describe different actions:

- `fork()` creates a child process with specified inherited state. It is not a thread creation call.
- `exec`-family operations replace the calling process's executable image. A successful `exec` does not return to the old program, and it does not by itself create a second process.
- `posix_spawn()` creates a child running a specified executable through its supported attributes and file actions, avoiding the need for the application to hand-code a separate fork/exec sequence.

These are general concepts. Availability, restrictions, security behavior, and implementation details must be checked for the actual OS release. In particular, do not assume Linux's copy-on-write implementation of `fork()` describes every QNX release or configuration.

After a successful `fork()`, parent and child normally have distinct private memory state: changing an ordinary private variable in one is not a communication mechanism to the other. Some resources are inherited with sharing semantics, including references to open file descriptions. Explicit shared mappings are another separate case.

Forking a multithreaded POSIX process is especially delicate. Only the calling thread continues in the child, while copied synchronization state may describe locks formerly held by other threads. POSIX restricts what the child may safely do before exec. A tiny fork example that works in a single-threaded program is not automatically a safe launch helper inside a threaded logger.

### 0.21 Exit, wait, zombies, and orphaned children

When a process exits, its executing threads stop and the OS releases process-owned resources according to their contracts. It does not undo external effects such as bytes already written to a file or commands already sent to another service.

Normal language/runtime termination can run cleanup that forced termination does not. Returning from `main()` performs its normal C++ termination path; abrupt termination must not be assumed to run destructors or flush user-space output buffers. A supervisor therefore distinguishes graceful stop from forced stop.

For a waitable child under ordinary POSIX parent/child behavior, termination status remains available for collection through functions such as `waitpid()`. A **zombie** is a terminated child whose retained status has not yet been collected under those rules. It is not a running worker secretly consuming CPU. Repeated failure to reap children can consume bookkeeping resources even though their application execution has ended.

An **orphaned child** has lost its original parent. It may still be running. Adoption and reaping responsibilities depend on the OS and supervision environment; on Linux, PID namespaces and subreapers affect the simple textbook "PID 1 adopts everything" story. Zombie and orphan describe different conditions.

A returned child status is encoded. Test whether the child exited normally before interpreting its exit code, or whether it ended due to a signal before inspecting that signal. Comparing the raw `waitpid()` status directly to the desired exit code is incorrect.

**Question:** is `waitpid()` the same as `std::thread::join()`? **Answer:** both can wait for termination, but they manage different entities and contracts. Join handles a C++ thread in the current process; waitpid collects status for an eligible child process. Joining a thread does not reap a child process.

### 0.22 Complete Linux example: start and reap a child

Use the lab filename `process_lifecycle.cpp`. This program launches a shell child that exits with code 7, waits for that specific child, and decodes its status. It uses a fixed trusted command, not untrusted input.

```cpp
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>

extern char** environ;

int main() {
    char shell_name[] = "sh";
    char command_option[] = "-c";
    char command[] = "exit 7";
    char* arguments[] = {shell_name, command_option, command, nullptr};
    pid_t child_pid = 0;

    const int spawn_error = posix_spawn(
        &child_pid, "/bin/sh", nullptr, nullptr, arguments, environ);
    if (spawn_error != 0) {
        std::cerr << "posix_spawn: " << std::strerror(spawn_error) << '\n';
        return 1;
    }

    int status = 0;
    pid_t waited_pid = -1;
    do {
        waited_pid = waitpid(child_pid, &status, 0);
    } while (waited_pid == -1 && errno == EINTR);

    if (waited_pid == -1) {
        std::perror("waitpid");
        return 1;
    }
    if (!WIFEXITED(status)) {
        std::cerr << "child did not exit normally\n";
        return 1;
    }

    const int exit_code = WEXITSTATUS(status);
    std::cout << "child exit=" << exit_code << '\n';
    return exit_code == 7 ? 0 : 1;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror process_lifecycle.cpp -o process_lifecycle
./process_lifecycle
```

Expected output: `child exit=7`. The parent program exits successfully because 7 is the intended child result. A child error code and a test-program failure are not the same thing here.

`arguments` is a null-terminated array of pointers to argument strings. `/bin/sh` identifies the executable, while the first argument conventionally supplies its command name. `environ` passes the current environment to the child. Production launchers should deliberately select inherited environment and descriptors rather than assuming all ambient state is appropriate.

`posix_spawn()` reports its immediate error as an error number, so the code inspects its return rather than reading `errno` unconditionally. `waitpid()` uses `-1` plus `errno` for failure. The loop retries an interrupted wait because this small lab intends to collect the child's status. A real supervisor also needs its own cancellation and elapsed-time policy.

The child may exit before the parent calls waitpid; its retained waitable status makes collection possible. A successful spawn alone is not proof that a production service became ready. Observe child exit and the application's explicit readiness protocol separately.

This is a Linux-tested process-lifecycle example. Recheck the matching QNX spawn contract, target shell path, inherited resources, and permissions before using it there.

### 0.23 A process address space is a map

An **address** is a number used to identify a memory location. On the systems discussed here, ordinary application pointers contain virtual addresses interpreted in the current process's mapping context. They are not universal physical RAM coordinates.

An **address space** contains mappings from ranges of virtual addresses to backing storage, together with access attributes. Some ranges have no mapping at all. Attempting to use a number as a pointer does not create a mapping for it.

A conceptual process map contains these kinds of regions:

```text
executable code             instructions, normally not writable
read-only data              constants and other read-only content
writable static data        process-lifetime mutable objects
dynamic allocations         storage managed through allocators
mapped libraries/files      explicitly or loader-created mappings
thread stacks               call-related storage for each thread
unmapped/guard regions      invalid or deliberately protected gaps
```

This is a list of roles, not a promised address order. Address-space layout randomization, CPU architecture, executable format, loader behavior, and mapping requests affect placement. A heap does not have to be one simple contiguous block growing toward one stack.

The same executable launched twice can have different virtual addresses for corresponding objects. Even if the addresses happen to match, private mappings can refer to different physical storage. Separate process address spaces provide isolation precisely because a pointer from process A does not automatically designate accessible storage in process B.

Multiple mappings can also refer to common storage: shared-memory mappings deliberately do so, and OS implementations may share suitable read-only executable pages. Therefore "separate processes" does not mean every byte of their underlying physical storage must be duplicated.

**Question:** why can two processes both print a pointer with the same numeric value but read different data? **Answer:** address translation uses the process's mapping context as well as the address. The number alone does not identify a physical byte globally.

### 0.24 Pages, page tables, and address translation

Many virtual-memory systems manage mappings in fixed-size units called **pages**. Physical storage units used to back them are often called **page frames**. Page sizes vary; use 4096 bytes, or 4 KiB, only as this arithmetic example's assumption.

A virtual address can be split into a virtual page number and an offset within that page:

```text
virtual page number = virtual address divided by page size, rounded down
page offset         = remainder of that division
physical address    = mapped frame number * page size + page offset
```

For virtual address `0x1234`, decimal 4660, the page number is 1 and the offset is `0x234`, decimal 564. If virtual page 1 maps to physical frame 9, the resulting physical address is `0x9234`, decimal 37428. The offset is preserved; the mapping changes which frame contains it.

If process B maps its virtual page 1 to frame 20, the same virtual address `0x1234` refers to physical address `0x14234`, decimal 82484. This is a numerical illustration of why equal pointer values across processes need not refer to the same bytes.

**Page tables** hold translation information and access attributes in an architecture-defined form. Modern systems often use multiple levels rather than one enormous flat table. The Memory Management Unit (**MMU**) uses the active translation context to translate and enforce access checks, with software participating as the architecture requires.

Changing mappings is privileged because it changes what a process can access. If arbitrary applications could rewrite all translation entries, address-space isolation would disappear. The OS exposes controlled mapping APIs instead of giving every program unrestricted page-table writes.

Paging is not the same as disk swapping. It describes a way to divide and map address spaces. A system can use page-based protection and translation without paging anonymous memory to disk.

### 0.25 TLB misses and page faults are different

Translating every access through a full page-table lookup would add cost. Processors commonly use a **Translation Lookaside Buffer (TLB)** to cache recent address translations and associated permissions. It caches translations, not the application's data bytes.

On a TLB hit, the translation can be obtained from that cache. On a TLB miss, the processor or OS follows the architecture's refill mechanism. If the page tables contain a valid permitted mapping, translation can complete without the application having an invalid access.

A **page fault** means an access requires fault handling under the architecture's rules. Possible reasons include a page not yet backed as needed, a write that triggers a supported copy-on-write mapping, or an access that violates permissions. Some faults can be resolved so execution resumes; others represent an invalid access and lead to an application failure.

```text
virtual access
    -> check cached translation
    -> on a miss, resolve translation using the platform mechanism
    -> if mapping/access is valid, continue
    -> if a fault condition exists, enter OS fault handling
          -> resolve permitted condition and resume
          -> or report an invalid access
```

Do not interpret every fault as a crash or every TLB miss as storage I/O. A first-use zero-filled page can require handling without reading an application page from disk. Linux tools distinguish classes such as minor and major faults under Linux's definitions; those counters are not universal cross-OS terminology or timing guarantees.

**Real-time consequence:** first access to memory can differ from later accesses. Allocating a large buffer during initialization does not prove all of its later accesses have already incurred every first-use cost. Verify the target's allocation, mapping, prefaulting, and memory-locking behavior rather than assuming an allocation call establishes a deadline bound.

### 0.26 Allocators, stack space, and fragmentation

The OS manages mappings and memory resources. A C/C++ allocator usually manages finer-grained allocations inside memory obtained through OS facilities. `new` can obtain storage through the allocation mechanism and construct an object in it; not every `new` requires a new mapping or system call.

Deleting an object destroys it and releases its storage according to its owner and allocator. The allocator may retain the freed storage for future requests rather than immediately returning mappings to the OS. Therefore process memory usage need not drop immediately after every deletion, and a stable retained arena is not automatically a leak.

A **memory leak** means storage remains allocated without the intended usable ownership and release path. **Fragmentation** means available storage is divided or rounded in ways that make it less useful. These are different problems.

With **internal fragmentation**, an allocation or page contains unused space inside its assigned unit. For example, a 6000-byte mapping backed by two 4096-byte pages occupies 8192 bytes of page capacity; up to 2192 bytes in that simplified allocation are not requested payload. Other metadata costs may also exist.

With **external fragmentation**, free regions are separated. An allocator may have three free 8 KiB regions but be unable to satisfy a request requiring a single contiguous 20 KiB region from those regions. Virtual mapping and different allocation strategies can change which forms of contiguity matter; virtual contiguity is not physical contiguity. Drivers and DMA requirements need particular care here.

A thread stack has limited space. Deep recursion and large local arrays consume it; each extra worker may need another stack allocation or reservation. A **guard page** is an inaccessible mapping intended to detect certain overflows. It is a protection aid, not proof that every kind of stack corruption will be detected safely.

**Question:** does using stack allocation instead of heap allocation guarantee real-time safety? **Answer:** no. Stack use can overflow, first-use mapping costs can remain, and the operation can still wait or do unbounded work. Storage choice addresses only part of the problem.

### 0.27 Demand paging, copy-on-write, swapping, and memory pressure

**Demand paging** means needed page contents or backing can be established when accessed rather than preparing everything eagerly. The exact sources and supported behavior depend on the OS. A file-backed mapping and anonymous writable memory have different backing and recovery properties.

**Copy-on-write (COW)** is an implementation technique in which participants can initially share suitable physical contents under protected mappings. When a participant writes, the platform can create a private copy and update that participant's mapping. The programs still observe the required isolation; the delayed physical copy is an optimization beneath that contract.

Linux commonly uses COW in implementing fork. That does not mean parent and child may communicate through ordinary private variables. It also does not imply every OS implements fork or process creation through the same mechanism.

**Swapping or paging to backing storage**, where supported and configured, can move suitable memory contents out of RAM and bring them back later. A page may instead be reclaimable from its file backing, or it may not be eligible for such eviction. Do not import Linux swap assumptions into QNX: page tables, mapped files, and faults do not establish a general swap mechanism on the target.

The **working set** is the memory actively needed over a relevant interval. On a system using demand paging, if active demands exceed available resident capacity, repeated eviction and reload can dominate useful execution. This is called **thrashing**. CPU utilization alone does not describe the resulting application progress.

Textbooks study replacement policies such as FIFO, which evicts an older resident page, and LRU, which aims to evict a page unused for the longest time. Exact LRU can be costly, so real systems use policies and approximations appropriate to their design. These concepts explain the trade-off; they do not describe a mandated QNX policy.

For example, with two frames and accesses A, B, A, C, LRU would retain recently used A and replace B when C arrives. A subsequent access to B would require it again. The relevant result is how the policy interacts with the workload, not that one name always wins.

**Architect lesson:** bound the active memory requirement and verify what happens under exhaustion. Allocation failure, mapping failure, service termination, or system-level recovery behavior is platform- and configuration-dependent. A design that only works while there is "usually enough RAM" has not specified its failure mode.

### 0.28 Pathnames, descriptors, and open file descriptions

A **pathname** is a name used to find a resource, such as a file or a device service. A **file descriptor** is a process-local integer handle produced by opening or otherwise obtaining access to a resource. Its number is an index-like handle, not the contents and not a globally unique identity.

POSIX distinguishes a descriptor from the **open file description** it refers to. The open file description represents open-instance state, including an offset for seekable files and certain status flags. Descriptor flags such as close-on-exec belong to a different layer.

```text
Process descriptor table          Open-instance state          Resource
descriptor 3 -------------------> offset, status flags -------> file
descriptor 4 -- duplicate ------> same open-instance state
descriptor 5 -- independent ---> separate offset/flags ------> same file
```

Duplicating a descriptor with `dup()` normally creates another reference to the same open file description. Opening the same pathname independently normally creates a separate open file description. QNX resource managers implement the corresponding open-state concepts through their framework; their internal representation need not match Linux's implementation.

**Worked example:** a regular file contains `ABCDE`. Descriptor 3 is opened at offset zero, and descriptor 4 duplicates it. Reading two bytes through descriptor 3 returns `AB` and advances their shared open offset to 2. A following one-byte read through descriptor 4 returns `C`. An independent open at descriptor 5 still begins with `A`.

Separate descriptors do not therefore guarantee separate offsets. Conversely, opening the same resource twice does not mean both operations share one cursor. This distinction explains many resource-manager tests in section 14.

Closing one descriptor releases that reference; it does not necessarily destroy the underlying open instance while other references remain. Closed descriptor numbers can be reused. A stale integer passed to unrelated code may later refer to a completely different resource, so descriptor lifetime is part of correctness.

### 0.29 Filesystems, names, metadata, and persistence

A filesystem organizes named objects and their metadata over a storage or service model. Metadata can include ownership, permissions, type, size, and timestamps. A directory maps names to filesystem objects under that filesystem's rules; the name is not the same thing as the open descriptor or the object's data.

On ordinary POSIX-style filesystems, unlinking a filename removes a directory entry. An already open file can remain accessible through its open reference even after its last name is removed, with reclamation occurring after relevant references end. Special resources and filesystems have their own documented behavior; do not generalize this as a command for removing devices safely.

A **mount** attaches a filesystem into a pathname namespace. A pathname under a mounted tree may refer to a different filesystem with different persistence and performance properties. A RAM-backed filesystem does not acquire power-loss persistence merely because applications use normal file APIs to access it.

Caching can occur in the application, OS/service, and device. Returning from a write at one layer does not necessarily mean every lower layer has persisted the data. A **dirty** cached page or buffer has changes not yet written back to its backing destination. Exact caching and writeback behavior depend on the filesystem and platform.

A journaling filesystem records specified changes in a log to support recovery consistency. Journaling is not synonymous with "every recent application write survives power loss." The journal's scope, ordering, flush requirements, and device guarantees determine what survives. Application record validity and durability still need their own contract.

**Question:** if the OS closes a file when a process crashes, does that guarantee buffered output was saved? **Answer:** no. Bytes still held only in that process's user-space buffers may never have reached the file service. Automatic descriptor cleanup does not execute every missing application flush or complete every application transaction.

### 0.30 Blocking, nonblocking, and asynchronous I/O

A **blocking** operation may suspend its calling thread until the operation can make progress or completes according to its contract. "Blocking" does not mean it always waits: a read with available data may return immediately. A successful read may return fewer bytes than requested.

A **nonblocking** operation reports that it cannot proceed immediately instead of waiting for the relevant readiness condition. With supported POSIX descriptor types and configuration, that often appears as `EAGAIN` or `EWOULDBLOCK`. Nonblocking does not mean the call takes zero time or has a proven end-to-end execution bound.

An **asynchronous** interface separates submission from later completion. The caller may submit work and learn the outcome through a completion event, queue, or callback mechanism. Submission success may mean accepted for processing rather than completed. Buffers and operation state must remain valid for the lifetime required by the API.

| Model | When useful | Responsibility left to the application |
|---|---|---|
| Blocking worker | Simple sequential operation flow | Bound waits and provide shutdown/cancellation |
| Nonblocking readiness loop | Coordinate many descriptors without one blocked thread per idle resource | Track partial progress, readiness, and deadlines |
| Asynchronous completion | Separate initiation from later result | Own in-flight buffers, correlate completions, bound outstanding work |

Readiness interfaces such as supported `poll()` or `select()` facilities let a thread wait for readiness changes across descriptors. Readiness is not ownership of all available data: another participant may act first, and the following operation still needs correct error handling. Also, data being readable is not proof that an entire application frame has arrived.

An event loop typically waits, processes a bounded amount of ready work, updates its state, and waits again. If one handler performs a long blocking operation, it delays unrelated handlers sharing that loop. Move such work to an appropriately bounded worker path or choose another architecture when required.

**Architect lesson:** one thread per connection simplifies local control flow but consumes stacks and scheduling resources. One event loop reduces that form of overhead but requires explicit per-operation state and fairness. Neither model is universally superior; compare actual connection count, workload, deadlines, and implementation complexity.

### 0.31 Pipes explain byte streams, backpressure, and EOF

A pipe connects a write endpoint to a read endpoint through finite OS-managed buffering. It is useful for a local byte stream. The ordinary pipe does not preserve an arbitrary sequence of application `write()` calls as separate application records; a reader still needs an agreed representation.

If an ordinary pipe has no bytes buffered but at least one write-end reference remains open, a blocking reader waits. A nonblocking reader normally reports that no data is currently available. If no write-end references remain and all buffered data has been consumed, read returns zero: EOF.

This provides a useful example of why ownership affects progress. Suppose a parent and child inherit a write endpoint, but the parent forgets to close its unused copy. After the child closes its writer, the reader still cannot infer EOF because another writer reference exists. The unused descriptor is a live protocol dependency.

Likewise, when the finite pipe buffer is full, a blocking writer can wait for a reader to consume data. This is **backpressure**. If the reader is waiting for the writer to finish before it reads anything, a sufficiently large transfer can deadlock: writer waits for space, reader waits for writer termination.

POSIX supplies an atomicity rule for certain pipe writes up to `PIPE_BUF`, preventing interleaving with other writers under the documented conditions. It does not mean a read returns exactly one such write, and `PIPE_BUF` must not be confused with the pipe's total buffer capacity.

**Question:** why do process launchers close unused pipe ends and set close-on-exec deliberately? **Answer:** inherited references can keep resources alive, suppress EOF, expose capabilities to unintended children, and prevent shutdown even when the intended user has finished.

### 0.32 Follow one input byte through the OS

Assume an application is waiting for input from a device-backed stream. The exact Linux and QNX implementations differ, but these responsibilities provide a useful conceptual trace:

1. The application owns a valid descriptor or native endpoint and requests data.
2. The responsible service finds no data available and arranges a wait rather than running an infinite application loop.
3. Hardware receives input and reports completion through the configured interrupt or polling mechanism.
4. Device support establishes which bytes are valid and makes the corresponding input state available under its synchronization rules.
5. The wait condition is satisfied and the waiting thread becomes eligible to run.
6. Scheduling permits it to resume, and the operation returns bytes or a relevant result.
7. The application parser decides whether those bytes complete a meaningful record.

Each stage has its own ownership and timing. Device completion is not application parsing completion. A wakeup is not immediate execution. Receiving one byte is not receiving one complete telemetry command.

This trace joins the earlier concepts: a controlled OS request crosses a protection boundary, a blocked thread leaves the runnable set, a device event changes state, scheduling resumes work, and a descriptor identifies the opened resource. Later QNX channels, pulses, and resource managers implement specific parts of this broader OS story.

### 0.33 Scheduling algorithms optimize different goals

A scheduler needs a policy for selecting eligible work. Different policies emphasize fairness, average completion time, responsiveness, or justified deadlines. No one ordering optimizes all of these for every workload.

First define the quantities for a simple job:

```text
turnaround time = completion time - arrival time
first-response time = first execution time - arrival time
READY waiting time = time eligible to run but not executing
```

For the simplified CPU-only jobs below, with no blocking and no switching overhead, waiting time also equals turnaround minus CPU demand. Do not use that subtraction to call all non-executing time READY waiting when a real job also performs blocking I/O.

Assume three jobs arrive at time zero, with CPU demands A=8 ms, B=2 ms, and C=1 ms. A arrives first in the queue, then B, then C. All are equally eligible in this paper exercise.

**First-Come, First-Served (FCFS):** run jobs in arrival order without preemption in this example. A runs from 0 to 8, B from 8 to 10, and C from 10 to 11. The short jobs wait behind the long one. This illustrates the **convoy effect**, where short work queues behind lengthy work.

**Shortest Job First (SJF):** assuming we know the CPU demands, run C from 0 to 1, B from 1 to 3, and A from 3 to 11. For these jobs and assumptions, average turnaround improves. Real workloads rarely reveal exact future CPU demand, and a stream of short arrivals can delay long work unless fairness is addressed. A preemptive shortest-remaining-time variant reevaluates when shorter remaining work becomes eligible.

**Round Robin (RR):** give each eligible job up to a chosen quantum before rotating. With a 2 ms quantum, run A from 0 to 2, B from 2 to 4, C from 4 to 5, then A from 5 to 11 because no other unfinished job remains. A may pass further quantum boundaries without another thread to run.

| Policy in this paper model | Completion A / B / C | Average turnaround | Average READY wait | Average first response |
|---|---|---|---|---|
| FCFS | 8 / 10 / 11 ms | 9.67 ms | 6 ms | 6 ms |
| SJF | 11 / 3 / 1 ms | 5 ms | 1.33 ms | 1.33 ms |
| RR, 2 ms quantum | 11 / 4 / 5 ms | 6.67 ms | 3 ms | 2 ms |

RR improves first response over FCFS here, but a smaller quantum is not free: more rotations can mean more overhead and disturbed locality. A very large quantum tends toward nonpreemptive behavior for short jobs. The exact result depends on workload and actual switching costs.

**Priority scheduling** selects by assigned importance rather than simply arrival order or predicted length. **Aging** is a fairness technique that increases the importance of long-waiting work in policies that support it. Do not assume an RTOS silently applies aging to fixed-priority real-time work; that would change the timing model.

**Earliest Deadline First (EDF)** chooses by the earliest current absolute deadline in its idealized scheduling model. It is useful as scheduling theory, not a claim that the QNX configuration in this guide provides that policy. For QNX's actual supported policies and their semantics, use section 7 and the matching release documentation.

**Key distinction:** the general nonpreemptive FCFS example above is not a complete definition of QNX `SCHED_FIFO`. QNX priority-based scheduling can preempt lower-priority work when higher-priority eligible work becomes READY. FIFO rules concern the relevant ordering within that scheduling framework.

### 0.34 The four deadlock conditions and what they mean

Deadlock is not just "a program is taking a long time." It is a state in which participants cannot progress because the required releases depend on one another in an unresolved wait cycle.

The classic resource-allocation model identifies four necessary conditions for deadlock:

1. **Mutual exclusion:** at least one needed resource cannot be used by all participants simultaneously. A mutex-protected device state is an example.
2. **Hold and wait:** a participant keeps one resource while waiting for another.
3. **No forced preemption of the resource:** the resource cannot simply be taken away and returned safely by an external participant. This is different from preempting CPU execution.
4. **Circular wait:** participants form a cycle of waiting for resources held by others in the cycle.

For example, thread A owns lock X and requests Y, while B owns Y and requests X. The cycle supplies all four conditions in this simple single-owner lock model. Merely observing that a system permits the four conditions does not prove it is currently deadlocked. Resource graphs with multiple instances require more careful reasoning than treating every apparent cycle as a sufficient diagnosis.

**Prevention** removes a condition by design. A consistent global lock order prevents the X/Y cycle: every participant requests X before Y. Obtaining all needed resources before starting is another possible design, but may reduce concurrency and be impractical for dynamic dependencies.

**Avoidance** makes allocation decisions using knowledge of future maximum needs so the system retains a safe completion sequence. Textbook algorithms such as the banker's algorithm illustrate this. They require trustworthy resource-need information that many real applications cannot supply conveniently.

**Detection and recovery** allow potential cycles, detect a bad state, then abort or restart selected work according to a recovery contract. Releasing a mutex arbitrarily from another thread is not a valid generic recovery action; protected state may be half-updated and mutex ownership rules still apply.

**Small safe-state example:** there are three interchangeable units. A and B each hold one, and each may need one more to finish. One unit remains free. Granting it to A lets A finish and release its units, after which B can finish. With only two total units and both participants already holding one while waiting for another, no such completion sequence exists under these assumptions.

Timeouts can break an application's willingness to wait, but recovery must release owned resources and address partial effects. A timed-out operation that keeps its original resources indefinitely has not necessarily broken the dependency cycle.

### 0.35 Choose IPC by the communication contract

Processes need explicit communication because ordinary private variables are not shared. **Interprocess communication** is a family of mechanisms, not one universal queue.

| Mechanism | What the application receives | Suitable starting use | Important missing guarantee |
|---|---|---|---|
| Pipe | Local byte stream | Parent/child data flow or command pipelines | No general application-message framing |
| Stream socket | Connection-oriented byte stream | Local or remote request protocols | No automatic request boundaries or processing acknowledgment |
| Datagram socket | Individual datagrams, subject to transport rules | Discrete notifications or telemetry | Reliability and ordering depend on the chosen transport |
| POSIX message queue | Bounded discrete messages with documented queue behavior | Queued local payloads where supported | No automatic business-level reply or unlimited capacity |
| Shared memory | Access to common storage | Bulk data with explicit ownership | No automatic synchronization or crash recovery |
| Signal | OS notification under signal delivery rules | Termination requests and selected process events | Not a general structured payload transport |
| Native QNX message | Synchronous transaction through channel/connection APIs | Local service request and reply | Timeout does not undo an application effect |
| QNX pulse | Small native notification | Timer or state-change wakeup | Not a reply-bearing transaction |

"Synchronous" and "asynchronous" describe an interaction contract, not whether data moves instantaneously. A synchronous caller waits for the defined completion. An asynchronous caller needs a way to correlate later completion, detect failure, and retain in-flight state.

Choose by asking what must cross the boundary: a request needing a result, a large buffer, a wakeup, or an ordered stream. Then specify message size, frequency, peers, ordering, overload, ownership, and failure behavior. Those requirements often eliminate unsuitable mechanisms before performance measurements are necessary.

For example, using shared memory to convey a tiny period-change command adds initialization and ownership machinery that a request/reply message may avoid. Conversely, copying every large image through several services can consume bandwidth that a shared bounded buffer pool could save. Neither choice is correct without its lifecycle rules.

### 0.36 Users, permissions, and resource limits

An OS associates processes with credentials or identities and uses those when deciding whether an operation is allowed. In a basic POSIX file-permission model, access bits are grouped for owner, group, and others. Additional access-control lists, capabilities, abilities, or security policies can further affect the decision.

For a regular file, read permits reading its contents, write permits modifying them according to the operation, and execute permits using it as an executable subject to other requirements. For a directory, execute commonly means **search/traversal permission**, not "run this directory as code." Directory read concerns listing entries, while modification of names involves directory permissions and additional rules.

As an introductory example, mode `0640` on a regular file grants owner read/write, group read, and no access through the others bits. That does not alone prove a caller can reach the file: pathname traversal, identity/group membership, ACLs, policy, and filesystem conditions still matter.

A check followed by use can race. If a program checks a pathname and later opens it, another participant may change what the name resolves to in between. Security-sensitive file access must use the platform's appropriate descriptor-relative, creation, and resolution facilities rather than treating an earlier pathname check as permanent authorization for an unchanged object.

**Resource limits** constrain consumption such as descriptors, memory, processes, or CPU according to supported facilities. They complement permissions: a process allowed to open files can still exhaust its descriptor budget. Check failure returns and bound accepted work instead of assuming permitted operations always succeed.

QNX abilities and policies supply additional operation-specific controls discussed in section 19. Traditional owner/group permissions are a useful foundation, not the full QNX security model.

**Architect question:** should a logger inherit every descriptor and environment variable from its launcher? **Answer:** no. Ambient access can expose unrelated resources or secrets and keep dependencies alive. Give the child the specific resources and configuration its role requires.

### 0.37 Containers, virtual machines, and the host kernel

A **container** isolates processes and their view or allocation of resources through facilities of the host OS. Linux containers commonly use namespaces for selected views and control groups for resource management. They continue to use the Linux kernel; an alternate filesystem image inside the container does not install a different running kernel.

A **virtual machine** provides a virtual hardware environment in which a guest OS runs its own kernel, under a hypervisor or virtual-machine monitor. The guest must match the supported virtual hardware and still needs suitable licensing, images, drivers, and configuration.

```text
Linux container: application -> shared host Linux kernel -> hardware

QNX virtual machine: application -> guest QNX kernel
                    -> virtual hardware / hypervisor -> hardware
```

These are conceptual paths, not exact performance diagrams. Hardware-assisted virtualization, device passthrough, and other implementations change the detailed path. Neither diagram eliminates resource contention.

This is why the current Alpine Linux development container can compile and execute Linux exercises but cannot implement native QNX messaging by installing QNX-looking headers. A cross-compiler can produce a QNX executable there only in a supported configured host environment; the executable still needs a compatible QNX execution environment.

Virtualization introduces another scheduling layer. A guest thread may be READY inside its guest while the corresponding virtual CPU is not currently executing on a physical CPU. That additional interference helps explain why VM timing results cannot automatically establish deadlines on a board or on another VM deployment.

Isolation also has limits. Processes in a container still share a host kernel; guest VMs may share physical devices, memory bandwidth, and hypervisor infrastructure. An architectural claim must specify what is isolated and what resources remain shared.

### 0.38 Worked OS questions that connect the mechanisms

**A program uses little CPU but takes ten seconds to read a file. Is the CPU too slow?** Not necessarily. The thread may be waiting on storage or another service for most of the interval. Inspect its blocked state and the service dependency before optimizing arithmetic.

**A process has a large virtual-memory size. Has it consumed that much physical RAM?** Not necessarily. Reserved, mapped, resident, shared, and committed quantities differ. Interpret the actual metric and the target's memory model rather than equating every address-space byte with a unique resident physical byte.

**An application enters the kernel and returns immediately. Was there a thread switch?** Not necessarily. There was a controlled privilege transition; the same thread may have continued throughout. Blocking or scheduling decisions can cause switches, but OS entry alone does not prove one.

**A child finished, but the parent's pipe reader never sees EOF. What should you inspect?** Every inherited or duplicated write-end reference. EOF depends on all writers being gone and buffered data being consumed, not simply on the intended producer exiting.

**A page fault occurred, but the program continued. Is that inconsistent?** No. Some faults represent conditions the OS can resolve, such as supported first-use or copy-on-write handling. Invalid mappings or access violations are different cases.

**A server is unreachable after restart even though its PID was printed. What is missing?** A printed PID does not prove readiness or establish a fresh valid connection for an old client. Recheck endpoint lifetime, initialization, authorization, and the reconnection protocol.

**A service is memory-safe but misses deadlines. Can both be true?** Yes. Protection and ownership can be correct while CPU contention, lock waits, I/O, or scheduling delays exceed the timing budget. Correctness is a collection of properties, not a single switch.

The OS foundation is the connection between these facts: protected address spaces contain executing threads; controlled interfaces request resources; waiting and interrupts change eligibility; scheduling supplies CPU time; and ownership determines when resources can be released. QNX's channels, pulses, and resource managers are specific mechanisms built within that picture.

## 1. Reading the Examples

A **complete example** includes its headers and `main()`, followed by a build command and an expected result. A **conceptual trace** shows one possible ordering of events; it is an explanation, not a program to compile. A **model** implements a specific behavior, such as snapshot reads, without claiming to implement the entire QNX service around it.

The explanations after each program describe the state changes and why they are correct. A printed result such as `counter=20000` checks one outcome; the mutex and lifetime reasoning explain why the implementation is safe under its assumptions. Repeated successful runs alone do not prove the absence of a data race.

Environment labels tell you where an example can actually execute:

| Label | Where the work happens |
|---|---|
| Linux | Your existing Linux workspace; portable concepts only |
| Host | A supported development machine with the QNX SDP installed and licensed |
| QNX target | A QNX virtual machine or board running your matching image |
| Design | Paper, diagrams, or Markdown; no target required |

**Lab safety:** use a development VM or spare board, keep a console and recovery path, and save a known-good image. Do not change scheduling, security policy, startup images, or driver configuration on a production or vehicle system. Use bounded workloads, ordinary priorities initially, and an external supervisor for fault experiments. A timeout inside a blocked application is not a substitute for an independent recovery path.

### 1.1 What an example proves, and what it does not

The queue program prints `count=1000 sum=500500`. That confirms the observed count and sum for that run. It is useful, but count and sum alone cannot prove exactly-once delivery: replacing items 1 and 4 with 2 and 3 preserves both count and sum. To test identity, record a bounded set of expected sequence numbers and check each appears exactly once.

An example can therefore supply several kinds of evidence. Compilation checks that the selected compiler accepts it. Runtime assertions check particular observations. Reasoning about the locking protocol addresses possible interleavings. Target tests address the platform integration. Timing analysis and measurements address deadlines under stated conditions.

These kinds of evidence complement one another. A program that compiles can still race. A race-free program can still block forever. A program that meets timing in a VM may fail on the intended board under a different workload.

### 1.2 Read assumptions as part of the result

When a lesson says "one producer and one consumer," that is a boundary on the argument. Adding another consumer changes which thread may remove an item after a wakeup. When it says "same ABI," that limits what can be inferred about copying a struct between programs.

Separate three labels in your reasoning: **given** is an assumption supplied by the problem; **observed** is a measurement from a specific run; **derived** is a conclusion that follows from assumptions and logic. For example, a measured maximum of 2 ms is observed. Calling it a guaranteed maximum for all permitted inputs needs an additional argument.

At architect level, this becomes a contract between components. A queue analysis may assume that the consumer always resumes within a bounded interval. The scheduling and dependency design must justify that assumption; it cannot remain a convenient sentence in the queue document.

## 2. What QNX Is

QNX is a commercial, POSIX-oriented, microkernel-based real-time operating system used in embedded systems, including automotive and industrial products.

Three ideas matter first:

- **Real-time:** correctness depends on both the result and when it becomes available.
- **Microkernel:** a relatively small kernel handles core mechanisms such as thread scheduling, native IPC, and interrupt handling. Many other services run in separate processes.
- **Message-oriented services:** applications can request work from other processes through native message passing; resource managers expose services through familiar file APIs.

```text
Application A       Application B       Diagnostic tool
       |                  |                    |
       +------- messages / POSIX APIs ---------+
                          |
         filesystem / network / device services
                 in separate processes
                          |
       microkernel mechanisms + process management
                          |
                       hardware
```

This is a conceptual picture, not an exact boot or memory map. The `procnto` system component combines the microkernel and process manager. User-space drivers may still have powerful hardware access; process isolation does not make every driver failure harmless.

### Real-time does not mean just fast

Consider a sensor application that must respond within 10 ms:

- System A usually responds in 1 ms but occasionally takes 100 ms.
- System B usually responds in 4 ms and has a justified bound below 10 ms under its specified load.

For this requirement, B is the better real-time design. A low average alone cannot establish a deadline guarantee.

| Class | Meaning | Example requirement |
|---|---|---|
| Hard real-time | Missing a deadline is an unacceptable system failure | A safety-critical control action |
| Firm real-time | Late results have no value, but some misses are tolerable | A stale inspection result that must be discarded |
| Soft real-time | Lateness reduces quality | A delayed dashboard animation |

The classification belongs to the requirement, not automatically to an entire product category.

### QNX versus Linux

| Topic | Linux familiarity | QNX learning focus |
|---|---|---|
| Architecture | Primarily monolithic kernel with modules | Microkernel with many process-based services |
| Application code | C/C++, POSIX, sockets | Much portable code, but not identical APIs or behavior |
| Native IPC | Pipes, sockets, shared memory, etc. | Channels, connections, Send/Receive/Reply, pulses |
| Devices | Often kernel drivers | Often user-space resource managers and driver services |
| Deployment | Distribution packages and services | Frequently a deliberately assembled target image |
| Diagnostics | `ps`, `/proc`, platform tracing tools | `pidin`, QNX debugger tools, kernel tracing |

Linux can also support real-time configurations. QNX does not automatically make application code deterministic. Both require a correct workload model and careful engineering.

### 2.1 Worked lesson: follow one file read

Imagine the logger opens a file and requests 100 bytes. The following is a conceptual service path, not a claim that every read triggers a physical disk operation:

1. Application code calls the file API using a descriptor belonging to its process.
2. QNX's I/O infrastructure directs the request to the service responsible for that opened resource.
3. The service validates the operation and checks whether the requested data is already available.
4. If device work is necessary, the service and relevant driver arrange it. The requesting thread can wait while another thread uses the CPU.
5. The service completes the request with bytes, EOF, or an error. The application must inspect which happened.

Notice three different responsibilities: the application decides what data it wants, a service implements the resource's behavior, and the kernel supplies scheduling, protection, and communication mechanisms. The microkernel does not itself understand every application's log format.

**Why separate services?** If a service corrupts its own ordinary memory, process protection can prevent that write from directly corrupting unrelated process memory. The service may be replaceable without replacing the whole OS. But clients still lose that service until recovery, and a driver with DMA or privileged access needs stronger analysis. Isolation reduces certain failure paths; it does not erase dependencies.

**What is the cost?** Requests may cross address-space boundaries and involve copying, scheduling, and protocol handling. Whether that matters depends on message size, request rate, implementation, and hardware. Measure your workload instead of assuming either "microkernel is always slow" or "IPC is free."

**Self-check:** the filesystem process crashes while the sensor process is alive. Has the whole system necessarily crashed? No. Is logging necessarily still working? Also no. Execution isolation and continued service availability are different properties.

### 2.2 Mechanism versus policy

A **mechanism** supplies a capability. A **policy** decides how that capability should be used. Message passing supplies a mechanism for request/reply communication. The choice to reject a request after 50 ms, return only after durable storage, or limit each client to four outstanding operations is application policy.

This distinction prevents an important mistake: expecting the RTOS to decide business meaning. QNX can schedule a thread at a requested permitted priority, but it does not know whether your diagnostic request is more important than your acquisition job. It can complete a reply transaction, but it does not decide whether that reply means accepted, processed, or persisted.

Good component interfaces make policy visible. Instead of saying "send a record," specify the accepted record format, completion milestone, full-queue behavior, timeout meaning, and restart behavior. The OS mechanism then supports that contract.

### 2.3 Isolation is a property with several dimensions

Process boundaries provide address-space separation, but there are other kinds of interference. Two processes may compete for CPU time, memory bandwidth, storage throughput, kernel resources, or an essential service. A logger that cannot overwrite sensor memory might still occupy the only storage path or consume enough CPU to affect deadlines.

Distinguish **spatial isolation**, restricting memory and access, from **temporal isolation**, controlling interference over time. Distinguish both from **availability**, the ability to provide the required service. A microkernel architecture supports useful separation, but a complete system must still budget shared resources and manage dependencies.

For example, placing status and storage in separate processes may keep status available after a storage crash. It will not help if status synchronously calls storage for every response and waits indefinitely. The dependency graph can defeat the intended availability boundary even when memory isolation works perfectly.

### 2.4 Compare architectures using a concrete requirement

Suppose a product requires acquisition to continue with visible record loss when storage is unavailable for up to five seconds. A single process with separate threads is simpler and can share a queue directly. It also shares one failure boundary for memory corruption and process termination.

A separate logger process adds protocol, deployment, authentication, and reconnection work. In return, the acquisition process can own a defined degraded state while the logger is restarted. That is a useful trade only if acquisition avoids unbounded waits on the failed logger and has a bounded loss or buffering policy.

The architectural decision is not "microkernel means separate everything." It is "this boundary supports this required independent behavior, and its additional failure modes are controlled." Document why the simpler alternative fails the requirement before adding a new boundary.

**Architect checkpoint:** can you identify the resource or dependency shared by supposedly isolated components? If not, the isolation argument is incomplete. If you can, explain how its overload or failure affects each component.

## 3. Vocabulary

| Term | Meaning |
|---|---|
| OS | Operating system |
| RTOS | Real-time operating system |
| POSIX | Portable Operating System Interface; a family of API standards |
| SDP | Software Development Platform; development tools and target components |
| SDK | Software Development Kit; use the exact one required by your project |
| BSP | Board Support Package; board-specific startup, drivers, and configuration |
| Host | Machine on which you build and debug |
| Target | Machine that runs the QNX executable |
| ABI | Application Binary Interface; binary calling and data-layout conventions |
| IPC | Interprocess communication |
| PID / TID | Process identifier / thread identifier |
| Channel / `chid` | A message-receiving endpoint / its identifier in the server |
| Connection / `coid` | A client's link to a channel / its local identifier |
| `rcvid` | Receive identifier used to reply to a particular received request |
| Pulse | Small asynchronous QNX notification, without a reply transaction |
| Resource manager | Process exposing resources through pathname and I/O requests |
| IFS | Image filesystem used in a QNX boot image |
| ISR | Interrupt service routine |
| DMA | Direct memory access |
| WCET | Worst-case execution time under stated conditions |
| Jitter | Variation in timing; specify exactly which timing quantity |
| Deadline | Latest acceptable completion time |
| Priority inversion | High-priority work delayed by a lower-priority resource owner |
| SMP | Symmetric multiprocessing; scheduling across multiple CPU cores |
| Affinity | Restriction on CPUs where a thread can execute |
| Ability | Fine-grained QNX permission for a privileged operation |
| RAII | Resource Acquisition Is Initialization; lifetime-based C++ cleanup |

Do not use `chid`, `coid`, and `rcvid` interchangeably. They have different owners and lifetimes.

### 3.1 Reading technical terms

The table above expands common abbreviations. The explanations below show what the terms actually do. Learn each term by answering: **what is it, what problem does it solve, what owns it, and what does it not guarantee?** These cover the key OS, C++, tooling, and design terms used in both guides; individual API constants still need their release-specific reference.

Example: saying "IPC means interprocess communication" is only expansion. Understanding IPC means explaining how two isolated processes exchange a request, who waits, who owns the data, and what happens when one process exits.

### 3.2 OS structure and execution

| Term | Plain-language explanation | Example or boundary |
|---|---|---|
| Kernel | Privileged OS core that controls fundamental execution and resource mechanisms | Application code requests OS operations; it does not directly decide which thread runs next |
| Microkernel | Architecture keeping core mechanisms relatively small while many services run in processes | A filesystem service can be separate from the kernel; a failure can still affect its clients |
| Monolithic kernel | Architecture placing many OS services in kernel space | Linux is primarily monolithic, even though it supports loadable modules |
| User space / kernel space | Different execution privileges and memory-access domains | User-space code cannot assume it may access physical device registers |
| System call | A controlled request from application code into an OS mechanism | Some library functions make system calls; others work entirely in user space |
| Process | An executing program instance with an address space and owned resources | Two executions of the same binary can be two separate processes |
| Thread | One sequence of execution within a process | Logger and acquisition threads share process memory but have separate stacks |
| Address space | The set of virtual addresses a process can use and their mappings | The address `0x1000` in two processes need not refer to the same physical memory |
| Virtual memory / MMU | Address mapping and protection, often implemented using a memory-management unit | A virtual address is not a raw physical RAM or device address |
| Context switch | Saving one execution context and restoring another | It has costs; not every switch changes the process address space |
| Concurrency | Multiple tasks are in progress over overlapping time intervals | One CPU can interleave two threads without running them at the same instant |
| Parallelism | Multiple tasks execute at the same instant | Two cores may execute two threads simultaneously |
| CPU / core | Central processing unit / an execution core within the processor | The number of cores affects concurrency assumptions but does not remove data races |
| SMP | Symmetric multiprocessing: multiple CPUs/cores supported by one OS scheduling environment | Thread eligibility, affinity, and shared resources still constrain execution |
| Affinity / runmask | Restrictions on which CPUs a thread may run on | Pinning work can simplify an experiment but can also create a bottleneck |
| API | Application Programming Interface: the functions and contracts source code uses | `MsgSend()` is a QNX API; an API name alone does not specify its full behavior |
| ABI | Application Binary Interface: calling conventions, binary layout, and related runtime contracts | Matching CPU architecture is insufficient for a Linux binary to run on QNX |
| POSIX | A family of standardized OS interfaces and behavior | Source portability is not binary portability or identical timing behavior |
| Fault isolation | Restricting how far one component's failure propagates | Separate processes protect address spaces, but a failed storage service still blocks useful storage work |

### 3.3 Timing and scheduling

| Term | Plain-language explanation | Example or boundary |
|---|---|---|
| Scheduler | OS mechanism choosing eligible runnable work | It cannot make a thread run while that thread still waits for a resource |
| RUNNING / READY / blocked | Executing now / able to execute when selected / waiting for an event or resource | READY does not mean guaranteed immediate execution |
| Priority | Relative scheduling importance under the selected policy | In QNX higher numerical values generally mean higher priority |
| Base / effective priority | Configured priority / priority currently used after applicable scheduling mechanisms | Inheritance may temporarily change effective priority |
| Preemption | Taking the CPU from executing work so another eligible thread can run | A higher-priority thread becoming READY may preempt lower-priority work |
| Time slice | Interval used to rotate execution under a time-slicing policy | Round-robin sharing normally concerns eligible equal-priority threads, not all threads |
| FIFO / RR | First-in, first-out / round-robin scheduling policies | These describe scheduling behavior, not guarantees that deadlines will be met |
| Release | When a particular job is intended to become eligible to execute | A periodic sample may have intended releases at 0, 10, and 20 ms |
| Period | Time between intended releases | A 10 ms period does not imply the job itself may take 10 ms in every system |
| Deadline | Latest acceptable completion time | A relative deadline of 8 ms after release becomes an absolute clock timestamp |
| Execution time | CPU time needed to perform the work under stated conditions | Different inputs, caches, and hardware can change it |
| Dispatch latency | Delay between becoming eligible and beginning execution, using a stated measurement convention | Be explicit if your measurement begins at a nominal timer deadline instead |
| Response time | Time from the chosen release/request point until the result is available | Includes waiting and execution; it is not just CPU work |
| Jitter | Variation in a named timing quantity | State whether you mean release-spacing variation, wakeup lateness, or response-time variation |
| Drift | Accumulating displacement from the intended timeline | Work followed by a relative wait can progressively shift a periodic schedule |
| WCET | Worst-case execution time justified under explicit assumptions | The largest value in 100 samples is only the maximum observed, not a WCET proof |
| Throughput | Completed work per unit time | High throughput can coexist with unacceptable response times |
| Utilization | Fraction of processing capacity demanded or used over a defined interval/model | Less than 100% alone does not establish schedulability |
| Monotonic clock | Clock intended for measuring progression of time without wall-clock-setting jumps | Use it for intervals; it is not a human calendar timestamp |
| Absolute / relative wait | Wait until a specified clock time / wait for a duration from a defined starting point | Absolute periodic schedules still require an overrun policy |
| Overrun | Work does not keep up with its intended timing budget or schedule | Decide whether to skip, catch up within a bound, or report a fault |
| Determinism | Predictability of behavior under stated conditions | It does not mean every execution has exactly the same duration |
| p95 / p99 | Percentile summaries: approximately 95% / 99% of samples lie at or below the value | Specify sample count and method; percentiles do not bound the remaining tail |

**Worked timing example:** intended release is 100 ms, execution starts at 103 ms, and finishes at 107 ms. Relative to the intended release, start lateness is 3 ms and response time is 7 ms. If the absolute deadline is 106 ms, the job is 1 ms late. The 4 ms between start and finish is elapsed time in that interval; without more evidence it may include preemption, so do not automatically call all of it CPU execution time.

### 3.4 Synchronization and C++ ownership

| Term | Plain-language explanation | Example or boundary |
|---|---|---|
| Shared state | Data more than one execution participant can access | Sharing an address space does not make simultaneous writes safe |
| Invariant | A condition that must remain true whenever other code observes the protected state | Queue size must stay between zero and capacity |
| Critical section | Region of code accessing state under required exclusion | Keep it bounded; avoid unrelated slow I/O while holding the lock |
| Mutex | Mutual-exclusion object with ownership semantics | Protect queue contents, indices, and closed state consistently |
| Condition variable / CV | Mechanism for waiting until protected state may satisfy a predicate | The predicate, not the notification, records that data is available |
| Predicate | Boolean condition describing whether work may proceed | `closed || !empty` allows a consumer to wake for data or shutdown |
| Spurious wakeup | A wait returns even though the desired predicate need not be true | Always recheck the predicate under the appropriate mutex |
| Semaphore | Counter-based synchronization primitive | It can count available slots; it is not interchangeable with a mutex owner |
| Atomic operation | Indivisible operation with specified memory-order semantics | An atomic flag does not make several ordinary data accesses a correct protocol |
| Happens-before | C++ ordering relation used to establish visibility and race-free ordering across operations | Proper mutex synchronization or thread joining can establish relevant ordering |
| Data race | Conflicting concurrent accesses, including a write, without required synchronization in the C++ model | Ordinary-object data races cause undefined behavior |
| Race condition | A broader bug where correctness depends on an uncontrolled ordering | Two individually synchronized operations can still implement a wrong check-then-act sequence |
| Deadlock | Participants wait in a dependency cycle from which they cannot progress | A owns lock 1 and waits for lock 2 while B does the reverse |
| Starvation | A participant repeatedly fails to obtain needed execution or resources | Sustained higher-priority work can prevent lower-priority progress |
| Livelock | Participants keep acting but fail to complete useful work | Two retrying peers can continually disrupt each other |
| Priority inversion | High-priority work waits on a resource owned by lower-priority work | Medium-priority interference may extend the wait |
| Priority inheritance | Temporarily boosting a resource owner's effective priority through the applicable mechanism | It does not make a sleeping owner runnable or resolve a circular wait |
| Lock-free / wait-free | System-wide progress guarantee / stronger per-operation progress bound under a defined model | Neither label alone establishes your real-time application's deadline |
| Stack / heap | Call/lifetime-associated storage / dynamically managed storage | Both have limits and failure modes; neither is automatically safe |
| RAII | C++ ownership pattern tying cleanup to object lifetime | A file owner closes its descriptor, but workers must first stop using it |
| Rule of zero | Prefer member types that already manage their resources correctly | Avoid handwritten ownership operations when standard owners suffice |
| Move semantics | Transfer of resources through move-aware operations | `std::move` enables overload selection; the selected operation performs any transfer |
| Dangling pointer / use-after-free | Pointer outlives the object / code accesses an object after its lifetime ends | Stopping a worker after destroying its queue is too late |
| Undefined behavior | The language places no requirements on program behavior for that invalid execution | A data race is not merely permission to read a slightly old value |
| Memory leak | Resources remain allocated without intended usable ownership or cleanup | Reference cycles can keep shared ownership alive indefinitely |
| `volatile` | Type qualifier relevant to specified access behavior, including some hardware interfaces | It does not replace atomics, locks, or device-specific memory barriers |

### 3.5 Native IPC and failure semantics

| Term | Plain-language explanation | Example or boundary |
|---|---|---|
| Endpoint | Place through which communication is accepted | A QNX channel is an endpoint, not the same as a TCP port |
| Channel / connection | Server receive endpoint / client attachment used to address that endpoint | `chid` and `coid` are not interchangeable handles |
| Receive ID | Identifier for completing a particular received transaction | Do not store it as a permanent client identity or reuse it after completion |
| Synchronous / asynchronous | Caller waits for specified completion / caller and later processing are decoupled | Asynchronous notification does not mean unlimited capacity or no errors |
| Message / pulse | Request data with transaction semantics / small notification without reply | `MsgReceive()` returning zero identifies a pulse |
| Event | A configured notification action describing how something should be reported | A timer can be configured to deliver a pulse; event and pulse are not synonyms |
| Signal | POSIX process/thread notification with its own delivery and handling rules | Signal handlers have restrictions; a signal is not a QNX pulse |
| Protocol | Agreement about message representation, meaning, valid operations, and failures | Define versions, sizes, commands, replies, and timeout meaning |
| Serialization | Converting data into a defined transferable representation | Copying `std::string` object bytes copies internals, not a portable string message |
| Payload / framing | Actual application content / rules identifying message boundaries | TCP needs application framing; it does not preserve separate sends |
| Timeout | A wait exceeds its allowed timing condition | The server may already have performed the requested action |
| Cancellation | Request to stop work, with explicitly defined acknowledgement and state rules | A client timeout alone is not acknowledged server cancellation |
| Idempotent | Repeating an operation has the same intended effect as applying it once | Set a value to 5 can be idempotent; increment by 5 usually is not |
| Deduplication | Detecting and handling repeated logical requests | Request IDs need a retention and restart policy, not just a number in a packet |
| Acknowledgement | A response confirming a specifically defined milestone | Accepted in memory, written, and durable are different milestones |
| Backpressure | Slowing or rejecting upstream work when downstream capacity is limited | Blocking a producer is one form, but may violate its deadline |
| Backlog | Work accepted but not completed | Sustained input exceeding service rate grows backlog until a bound or failure |
| Bounded queue | Queue with a fixed maximum accepted occupancy | Full behavior must be defined; bounded memory does not imply bounded wait |
| Partial failure | Some participants continue while another fails or becomes unreachable | A live client can face a dead server and an unknown request outcome |
| Generation / instance ID | Value distinguishing one lifetime of a resource from a later replacement | Reject shared state or endpoint data referring to a previous instance |

**Trace the terms:** the server creates a channel; the client attaches a connection; the client sends a versioned payload; the server receives a positive receive ID and replies using that ID. If the client times out before the reply, the effect may still have happened. If the receive result is zero, it is a pulse and there is no reply transaction.

### 3.6 Files, resource managers, and hardware

| Term | Plain-language explanation | Example or boundary |
|---|---|---|
| I/O | Input/output: communication with devices, files, services, or networks | I/O can block; do not assume its duration equals a memory access |
| File descriptor | Process-local handle to an opened I/O resource | It is an integer handle, not the file content or a physical device address |
| Pathname / namespace | Name used to locate a resource / organized space in which names are resolved | `/dev/learn_sensor` can resolve to a service rather than a disk file |
| Resource manager | QNX process handling pathname connection and I/O requests | It supplies the behavior behind a file-like interface |
| Dispatch / handler | Routing received work / function implementing a particular request | Framework routing does not replace application validation |
| OCB | Open control block: state associated with an open instance | An offset should not accidentally be global across independent opens |
| Offset / EOF | Current position / end-of-file condition | A snapshot read reaching its end returns zero bytes; streaming semantics can differ |
| `devctl` | Device-control interface for explicitly defined device-specific operations | Specify command, direction, size, version, and permissions |
| Snapshot / stream | Finite view of data at a defined point / ongoing sequence of data | State which model your sensor resource implements |
| Partial I/O | Fewer bytes are transferred than the complete logical application item | Track progress and do not assume one operation completes a record |
| Persistence / durability | State survives beyond a chosen lifetime / acknowledged state survives the specified failures | Successful `write()` alone may not establish power-loss durability |
| BSP / bootloader / startup | Board support / code loading the OS image / board-specific initialization for OS startup | These solve different parts of booting the actual hardware |
| IFS / buildfile | Image filesystem / instructions describing image contents and startup behavior | `mkifs` builds an image; it does not prove that image works on every board |
| Driver | Software operating a device and presenting a controlled interface | Its register map, interrupts, and permissions depend on the hardware |
| IRQ / ISR | Interrupt request / interrupt service routine | An interrupt announces work; its handler must follow the attachment model's restrictions |
| DMA | Direct memory access: hardware transfers data without CPU instructions copying every byte | Ownership, cache coherence, and completion still require coordination |
| Cache coherency / memory barrier | Consistency rules for cached views / ordering constraints on accesses | CPU atomics and device-access ordering solve related but distinct problems |
| SoC | System-on-chip: processor and other components integrated on one chip | The same CPU instruction set does not imply the same peripheral layout |
| Firmware | Software operating close to hardware, often involved in initialization or device behavior | A BSP is not a universal replacement for board firmware |

### 3.7 Tools, permissions, and error vocabulary

| Term | Plain-language explanation | Example or boundary |
|---|---|---|
| Compiler / linker / loader | Translate source / combine binary references / prepare a program to execute | Errors at these stages have different causes |
| Cross-compilation | Build on one environment for another target environment | Host-side `q++` can produce a QNX executable for a separate board |
| Host / target | Development environment / execution environment in the cross-development workflow | A supported self-hosted setup can combine roles; that does not make Linux a QNX target |
| SDK / SDP | Software Development Kit / QNX Software Development Platform | Tool versions, target libraries, and entitlement must match your workflow |
| ELF | Executable and Linkable Format, used for binary files | Sharing ELF format does not make Linux and QNX binaries interchangeable |
| libc / runtime library | Library implementing C and related runtime services / support needed by compiled programs | Use target-compatible libraries, not arbitrary host libraries |
| Debug symbols / core dump | Information mapping binaries to source / saved crash-state artifact where configured | Preserve matching binaries and libraries to interpret a crash reliably |
| Trace / profile | Ordered event record / summarized measurements of where resources or time are spent | Tracing and profiling can perturb the measured system |
| `pidin` / `qconn` | QNX process inspection utility / development connection service | Linux `ps` is not a substitute for all `pidin` output; restrict debug access |
| Ability / least privilege | Fine-grained QNX permission / granting only required access | A denied operation should trigger policy investigation, not blanket privileges |
| Authentication / authorization | Establish who a peer is / decide what it may do | Knowing an endpoint or PID does not authorize changing device configuration |
| Watchdog / heartbeat | Mechanism detecting lack of expected progress / periodic health indication | A heartbeat from an unrelated live thread can miss a stuck worker |
| Backoff / readiness | Increasing or bounded spacing of retries / ability to perform the required service | A process starting does not prove its dependencies are ready |
| Functional safety | Managing unreasonable risks from incorrect system behavior through the applicable lifecycle | Using an RTOS is not application certification |
| ISO 26262 | Automotive functional-safety standard series for road-vehicle electrical/electronic systems within its scope | Interview awareness is not evidence of compliance or certification |
| `errno` | Thread-local error indicator set by many, but not all, failing C/POSIX-style APIs | Inspect it when that API's documented return indicates failure |
| `EINTR` | Interrupted operation | Whether to retry depends on the operation and cancellation policy |
| `EINVAL` / `EMSGSIZE` | Invalid argument / message-size error | A protocol may use these to reject bad fields or unsupported sizes |
| `ETIMEDOUT` | An applicable timeout expired | It does not reveal every side effect already performed by the peer |
| `EPERM` / `EACCES` | Permission-related failures | Check the specific operation, abilities, resource permissions, and security policy |
| `_r` API variants | QNX variants with documented alternative return/error conventions | Do not mechanically copy `-1` plus `errno` handling from a non-`_r` call |

Many pthread functions return an error number directly. So does `clock_nanosleep()`. Always learn **success result, failure result, and side effects** together; a universal "all C functions return -1" rule is wrong.

### 3.8 API names as a lifecycle, not a spelling test

| API or family | What it contributes | What you must explain with it |
|---|---|---|
| `ChannelCreate` / `ChannelDestroy` | Create/release a native receiving endpoint | Who receives and whether anyone still uses the channel at destruction |
| `ConnectAttach` / `ConnectDetach` | Attach/release a client connection | Endpoint lifetime, access, and stale connections after restart |
| `MsgSend` / `MsgReceive` | Send a synchronous request / receive a message or pulse | Blocking state, buffers, valid lengths, and errors |
| `MsgReply` / `MsgError` | Complete a received request with data/status or an error | Use a valid outstanding receive ID; never reply to a pulse |
| `name_attach` / `name_open` | Publish/open a named service through their documented mechanisms | Connection messages, notifications, and matching cleanup remain necessary |
| `TimerTimeout` | Set up a timeout for specified kernel blocking behavior | Clock, duration, affected states, and outcome after a timeout |
| `timer_create` / `timer_settime` / `timer_delete` | Create, arm/disarm, and remove a POSIX timer | Event destination lifetime and overrun handling |
| `SIGEV_PULSE_INIT` | Initialize an event description for pulse delivery | It describes delivery; it does not by itself create and arm a timer |
| `shm_open` / `ftruncate` / `mmap` | Open shared memory, establish size, and map it | Validate size and initialization before access |
| `munmap` / `close` / `shm_unlink` | Release mapping/descriptor and remove the shared-memory name | Name removal and destruction of all existing mappings are not the same event |
| `dispatch_create` / `resmgr_attach` | Set up dispatch and attach a resource-manager pathname | Function tables, resource attributes, handlers, and cleanup |
| `iofunc_func_init` / `iofunc_attr_init` | Initialize framework callbacks/defaults and resource metadata | Defaults help with common behavior; application I/O semantics still need implementation |
| `pthread_create` / `pthread_join` | Start a thread / wait for its termination and collect its result | Shared-object lifetime, start failure, stop requests, and joining order |
| `sched_get_priority_min` / `sched_get_priority_max` | Query a policy's priority range | A value in the range may still require permission and correct thread attributes |

### 3.9 Translate a technical sentence into observable events

Consider: "The high-priority client is REPLY-blocked on a server whose worker is waiting for a mutex." Decode it in order. There are separate client and server roles. The client's request has already been received. Its calling thread cannot proceed until that transaction completes. The server-side work is waiting for ownership of protected state.

The next question is who owns the mutex and what prevents that owner from releasing it. The client's priority is relevant to scheduling mechanisms, but it does not identify that owner or remove the dependency. The sentence contains enough information to rule out a pure service-discovery problem, but not enough to distinguish a long critical section from deadlock.

Now consider: "A READY thread missed its relative deadline because of interference." READY means able to execute, not executing now. The relative deadline is measured from a defined release. Interference means other work delayed its progress; the next evidence needed is which work ran on eligible CPUs during that interval.

Technical vocabulary becomes useful when every term changes what you would inspect or predict. Expanding an abbreviation without tracing its consequences is only memorization.

### 3.10 Units and boundaries are part of a definition

"Latency is 5" is incomplete. Five nanoseconds, milliseconds, or seconds? Measured from timer expiry, packet arrival, request acceptance, or job release? Ending at handler entry, reply submission, client resumption, or durable completion?

Write an operational definition such as "elapsed monotonic milliseconds from receipt of a valid command to availability of its reply." Also identify the clock domain. Timestamps from unrelated machines cannot be subtracted as if their clocks were synchronized and equally accurate.

The same precision applies to capacity: 100 records, 100 bytes, or 100 outstanding transactions are different limits. It applies to failure: request rejected, client disconnected, service unavailable, and data lost are not synonyms. Accurate vocabulary makes requirements testable.

## 4. Set Up Your Learning Environment

### Licensing and access

**Yes, you can learn deeply without holding a QNX license.** Public documentation, C++/POSIX exercises on Linux, protocol design, and source walkthroughs do not require you to execute QNX. What you cannot establish that way is actual QNX API behavior, QNX scheduling traces, BSP/driver operation, or QNX timing results.

**You may also be able to obtain a license without paying.** As checked on 2026-09-28, the official [QNX Everywhere page](https://qnx.software/en/developers/get-started/qnx-everywhere) advertises free access for non-commercial use, including QNX SDP 8.0. It describes this route:

1. Create or sign in to a myQNX account through the official page.
2. Complete the QNX Everywhere license request form.
3. Receive a non-commercial license after QNX processes the request.
4. Review your actual license terms, available components, and supported installation/target options before downloading and installing.

Free-of-charge access is **not** license-free access. This does not establish your eligibility, guarantee approval, or authorize commercial work, redistribution, or production deployment. Do not assume the program includes your employer's QNX 7.x release or every optional product. Ask QNX or your employer for the appropriate entitlement when the purpose differs.

The same official page links [free online training](https://qnx.software/en/developers/get-started/training/overview), [QNX Everywhere documentation](https://www.qnx.com/developers/docs/qnxeverywhere/index.html), and the [self-hosted Developer Desktop guide](https://www.qnx.com/developers/docs/qnxeverywhere/com.qnx.doc.qdd/topic/about.html). Review their current access, hardware, and host requirements; do not buy hardware or assume Alpine compatibility from the program name alone.

| Access situation | What you can do | What still needs checking |
|---|---|---|
| No QNX license or target | Read public docs; implement portable Linux code; reason through native contracts | Actual QNX calls, traces, timing, drivers, and integration remain unverified |
| Approved QNX Everywhere license | Use provided components for uses allowed by its terms | Supported host/target, component coverage, activation, and non-commercial restrictions |
| Employer or academic access | Use the specific environment you are authorized to access | Assignment, sharing, remote-access, and deployment terms |
| Evaluation/commercial need | Ask QNX for a suitable offering | Eligibility, duration, price, product/version coverage, and deployment rights |

Virtualization does not supply a QNX entitlement: a VM/emulator supplies a machine model, and an authorized compatible QNX image supplies the OS. Docker's Linux kernel does not implement native QNX calls. Copying headers, using a Linux cross-compiler, or renaming socket wrappers to QNX function names does not change that.

### Track A: start now without QNX

Your current workspace is an Alpine Linux development container. Use it for C++, ownership, bounded queues, POSIX threads, shared memory concepts, sockets, and tests.

It is **not** a QNX target. Linux containers share the Linux kernel; they cannot execute QNX native IPC merely by installing a compiler or copying headers. Also, do not assume Alpine is a supported QNX SDP host: check the exact SDP installation requirements before attempting an installation.

Start with E01, E03, E04's Linux measurements, E07, E15, E17's Linux networking, E19, and the design portions of the capstone. E06 scheduling experiments are optional only where permissions and a safe environment allow them; use a paper timeline otherwise. Use the substitutes below for native-only exercises instead of waiting for a license.

### Linux practice without QNX

Keep three evidence labels in your notes: **Linux-tested**, **QNX walkthrough**, and **QNX-tested**. Only use the last label after running on an actual QNX target. A design exercise can pass its own reasoning checks without satisfying the original target-execution gate.

| QNX learning objective | Exercise you can do without QNX | Acceptance check and limit |
|---|---|---|
| Process/thread ownership | Two threads consume a bounded queue; close and join them safely | Verify accepted item count and empty/full shutdown; this tests portable synchronization |
| Request/reply protocol | Build two Linux processes communicating through a local UNIX-domain stream socket with bounded framing | Send a request ID and value, receive a matching reply, reject malformed sizes; sockets do not provide QNX channel/receive IDs |
| Missing reply and timeout | Make the Linux server delay a reply beyond the client's explicit timeout | Client reports timeout; document whether the effect already happened; this is not a `TimerTimeout()` test |
| SEND/REPLY/RECEIVE states | Trace the native example on paper with separate delayed-receive and delayed-reply paths | Label QNX states from their definitions, not from Linux `ps` output |
| Pulse/event design | Model typed notifications in a bounded in-process queue | Receiver validates codes and does not expect a reply; no claim about kernel pulse delivery or priority inheritance |
| Periodic timing | Use a monotonic absolute timeline and fixed-capacity timestamp storage | Report observed lateness and overruns on Linux, not a QNX deadline guarantee |
| Resource-manager semantics | Write a plain C++ snapshot reader with per-open cursor objects | Test one-byte reads, EOF, two independent cursors, and invalid writes; this does not implement QNX dispatch or pathname attachment |
| Shared memory | Use POSIX shared memory and supported process-shared synchronization, or start with a single-process model | Verify initialization, bounds, and cleanup; recheck platform behavior and crash recovery on QNX later |
| Boot/BSP understanding | Draw the documented startup chain and a service dependency graph | Explain readiness and failure cases; no flashing or invented board-specific register values |
| Debugging | Inspect your Linux program using available process/debugger tools and your own request logs | Follow lock/request ownership; Linux diagnostics cannot establish QNX-native blocked states |
| Recovery/system design | Restart a Linux service and reconnect using fresh endpoint information | Bound retries and explain unknown outcomes; this validates your Linux recovery protocol only |

**Suggested first-week sequence:** run the queue from section 9, test its shutdown cases, model a request/reply protocol, inject a delayed reply, then explain the native QNX example line by line. Do not implement fake QNX APIs: that can hide precisely the OS-specific behavior you need to learn.

For each walkthrough, fill in this short trace:

```text
Step | Actor | Operation | Owned resources | Who waits? | What can fail?
1    | ...   | ...       | ...             | ...        | ...

Prediction:
Observation, if executed:
What this proves:
What needs a QNX target later:
```

The [15-day no-license route](QNX_15_Day_Interview_Prep.md#no-license-route-for-all-15-days) maps these choices to interview preparation. Finish the portable and reasoning work now; keep native target gates pending rather than claiming the entire QNX course is practically validated.

### Track B: native QNX development

1. Check the [official Quickstart Guide](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.qnxsdp.quickstart/topic/about.html) and your organization's approved version.
2. Obtain the SDP and a valid license through the official distribution route. Educational or evaluation availability and conditions can change; do not assume all downloads or production deployment are free.
3. Install on an explicitly supported host. Keep SDK installations for different releases separate.
4. Obtain a matching QNX VM image or a BSP and image for your exact board. A CPU architecture match alone does not guarantee a board will boot.
5. Boot the target using its release-specific instructions. Establish serial or VM console access before configuring remote tools.
6. Source the SDK's supplied environment script in the host shell. Use its installed path, not a guessed path.
7. Identify the compiler variants and target architecture.

**Host, after the SDK environment is active:**

```sh
qcc --version
qcc -V
printf 'QNX_HOST=%s\nQNX_TARGET=%s\n' "$QNX_HOST" "$QNX_TARGET"
```

`QNX_HOST` identifies host-side tools; `QNX_TARGET` identifies target headers and libraries. Neither variable is the target's IP address.

**QNX target:**

```sh
uname -a
pidin
```

Record the versions in your experiment notes. Build a small executable on the host, transfer it using the image's approved mechanism, and execute it on the target. An SSH server is optional, not guaranteed to be present. Use the documented IDE/toolkit deployment flow when SSH is unavailable.

### Compiler patterns

For a 64-bit x86 QNX target, the host-side pattern is:

```sh
q++ -Vgcc_ntox86_64 -std=c++17 -D_QNX_SOURCE -Wall -Wextra -g source.cpp -o application
```

For an AArch64 little-endian QNX target:

```sh
q++ -Vgcc_ntoaarch64le -std=c++17 -D_QNX_SOURCE -Wall -Wextra -g source.cpp -o application
```

These are templates: replace the source and output names with your lab files, and confirm the variant with `qcc -V`. Prefer an explicit variant in repeatable builds.

`qcc` is the C compiler driver; `q++` selects C++ compilation and linking. In SDP 8.0, strict `-std=c++17` mode can hide non-ISO APIs unless `_QNX_SOURCE` is defined. QNX threads are provided through libc; unlike typical Linux builds, SDP 8.0 does not need `-pthread` and warns that it is ignored.

Do not mix Linux libraries with a QNX executable, even when both machines use x86-64. CPU architecture, OS ABI, runtime libraries, and compiler settings all matter.

**Setup passes when:** you can explain which machine compiled the executable, which machine ran it, and why the binary is appropriate for that target.

### 4.1 Understand the build before diagnosing it

```text
source and headers -> compiler -> object code
object code and libraries -> linker -> executable
executable and target runtime -> loader -> running process
```

An `#include` makes declarations available while compiling. It does not install an OS service, grant a permission, or necessarily provide a function's machine code. Linking resolves references against suitable object files and libraries. Loading then requires a compatible target ABI and runtime.

| Example failure | Stage | Reasoning and next action |
|---|---|---|
| Header cannot be found | Compilation | Check include search paths and active SDK; a native QNX header is not a Linux package substitute |
| Function has no matching declaration | Compilation | Check spelling, argument types, feature macros, and SDK version |
| Undefined reference to a function | Linking | Check target library requirements and the actual link command |
| Binary has an incompatible format | Loading | Check target architecture and OS ABI, not just executable permissions |
| Process starts but cannot connect | Runtime | Check endpoint lifetime, permissions, and whether the server is ready |

**Worked example:** a C++ file compiles to an object, but linking fails for a socket function. Changing the sensor algorithm will not solve a missing target-library dependency. Look up that socket API's library requirement, then adjust the target link command. Do not link a Linux library simply because its function has the same name.

Debug information (`-g`) helps a debugger relate machine code to source. Optimization changes generated code and can make stepping less intuitive; it is not equivalent to debug information. Keep build flags in experiment notes because timing measured with one build is not automatically representative of another.

**Self-check:** does `QNX_TARGET` name the remote board? No. It names the SDK's target-side headers and libraries on the build host. Deployment is a separate step.

### 4.2 What a reproducible build needs

Source code is only one input to a binary. Headers, libraries, compiler version, target variant, feature macros, optimization flags, and generated configuration can all change its behavior. A reproducible investigation records those inputs so another engineer can build the same intended program.

For QNX, the host compiler executable and the selected target components have different roles. A compiler installed on a Linux host can produce QNX machine code; the host's own libraries must not accidentally satisfy target dependencies. Keep target include and library selection explicit in the build configuration.

An incremental build may reuse old objects. When an ABI-affecting flag or generated header changes, the build system must rebuild affected objects. Mixing incompatible objects can produce failures that source-level inspection alone does not explain.

### 4.3 Deployment is a second dependency problem

Copying an executable to a target is not sufficient if its required runtime libraries, service dependencies, permissions, or configuration are absent. The loader needs compatible binary dependencies; the application then needs its operational dependencies.

Suppose a program runs on the development image but fails on a reduced production-like image. First determine whether it fails before entering main, during initialization, or during a request. A loader failure suggests binary dependencies. An initialization failure suggests missing resources or denied operations. A later request failure points to the runtime interaction being exercised.

Keep the exact unstripped binary and debug symbols associated with each deployed build. Giving the debugger symbols from a similar but different build can lead to misleading addresses and source lines. A build identifier or recorded artifact hash connects observed behavior to the actual binary.

### 4.4 Separate environment confidence from application confidence

A successful hello-world deployment establishes that a particular build/deploy/run path works. It does not establish permissions for public channels, tracing support, driver compatibility, or real-time performance. Add each capability as a separate verified fact.

For teams, this becomes a controlled environment description: SDK and entitlement, supported host, BSP revision, target image, CPU, enabled services, security policy, and build flags. It is engineering configuration, not administrative decoration. A change to one of these can invalidate earlier integration or timing evidence.

## 5. How the Concepts Fit Together

Consider a simulated sensor that produces one temperature record every 10 ms. Each record contains a sequence number, a timestamp, and a temperature. A logger stores the records, while a diagnostic client asks for the current status.

The sensor and logger solve different problems. Acquisition needs to happen at the intended times. Storage may take a variable amount of time. If acquisition performs every disk write itself, a slow write delays the next sample. Separate execution paths let those activities overlap, but we still need rules for exchanging data.

Inside one process, a bounded queue can connect an acquisition thread to a logger thread. A mutex protects the queue's state, condition variables let participants wait, and closure tells them when no more records will arrive. This explains why threads, synchronization, and shutdown belong together.

If the logger must restart independently or run with different access, move it into a separate process. Now ordinary queue pointers cannot cross the boundary. Native QNX messages can carry small records and acknowledgments. Shared memory is another option for large buffers, but requires explicit ownership and recovery rules.

A timer supplies notifications for intended acquisition times. The scheduler determines when an eligible thread actually runs. The application measures when acquisition begins and finishes. These are different stages, so the timer period alone does not prove the response deadline.

Suppose acquisition produces 100 records per second but storage handles only 50. The difference, 50 records per second, accumulates in the queue. An initially empty 100-slot queue fills in about two seconds under this simplified steady-rate model. More threads or a larger queue cannot remove a sustained service-rate deficit. The application must reduce input, improve service, or define rejection and loss behavior.

A resource manager can expose a snapshot of status through a pathname. A network interface can carry records to another machine, using framing and explicit delivery semantics. Permissions control who can inspect or change the service. A supervisor detects loss of useful progress and applies the restart policy.

All these mechanisms answer concrete questions about the same records: who owns them, who may access them, when work can run, what completion means, and what happens if a participant fails. The following chapters explain each mechanism separately.

### 5.1 Turn a vague goal into a contract

"Log sensor data quickly and reliably" leaves too much undecided. For a concrete teaching requirement, suppose a simulated sensor releases one job every 10 ms, each accepted record occupies 64 bytes, and acquisition must finish its local work within 5 ms of intended release. Storage may pause for 200 ms. Records already accepted into RAM are not promised to survive process failure.

These sentences answer different questions. The 10 ms period establishes the normal arrival rate of 100 records/s. The 5 ms deadline limits acquisition response. The 64-byte representation helps budget memory. The pause allowance describes one source of backlog. The last sentence defines the acknowledgment's failure semantics.

Now an implementation can be evaluated. Synchronously waiting for storage during acquisition cannot meet the 5 ms requirement when storage pauses for 200 ms. A bounded queue can decouple those operations, but its full behavior must still be specified. If loss is permitted, a nonblocking rejection with a loss counter may be more appropriate than waiting for space.

### 5.2 Calculate a first budget, then expose its assumptions

At 100 records/s, a 200 ms service pause introduces about 20 records under the simplified steady-rate model. At 64 bytes each, those payloads occupy 1280 bytes. A real memory budget also includes queue metadata, alignment, worker stacks, IPC buffers, and other allocations.

Twenty slots are not automatically sufficient. What was the occupancy before the pause? Can arrivals burst? Does the logger drain faster than arrivals after it resumes? Are several pauses allowed before the backlog clears? The capacity calculation is only as strong as these workload assumptions.

If the logger resumes at 200 records/s while acquisition continues at 100, its net drain rate is 100 records/s. A 20-record backlog clears in about 200 ms in this simplified model. Resuming at exactly 100 records/s prevents further growth but never removes that backlog.

### 5.3 Trace ownership and completion across the whole path

```text
acquisition creates record
    -> queue accepts ownership of a copy
    -> logger client removes record into local ownership
    -> server accepts or rejects the request
    -> storage operation reaches the defined milestone
    -> reply tells the client that milestone was reached
```

At each arrow ask what happens if the next participant fails. Once a record is removed from the queue, a failed send does not automatically put it back. If the server accepted it but the reply is lost, requeueing can duplicate it. The architecture must define these transitions, not just the happy-path arrows.

This is how beginner mechanisms become system design: object lifetime becomes cross-component ownership, a wait predicate becomes a service-progress dependency, and a counter check becomes an end-to-end accounting rule.

## 6. Processes, Threads, and Ownership

A process owns an address space and resources. Threads in that process share its memory but have separate execution state and stacks. Scheduling decisions concern runnable threads, not simply whole processes.

Use processes when fault containment, privileges, or independent lifecycle matters. Use threads when shared state and low-overhead cooperation are appropriate. More threads do not automatically improve throughput or latency.

For every object shared between threads, answer:

1. Who creates and destroys it?
2. Who may mutate it?
3. What synchronizes access?
4. What happens when a participant stops or fails?

For every blocking operation, answer:

1. What event makes it complete?
2. What is the maximum intended wait?
3. Can the event producer itself be waiting on this thread?
4. What happens during shutdown?

Common thread states include RUNNING, READY, and various blocked states. A READY thread can execute when scheduling permits; a blocked thread must first satisfy its wait condition. Increasing the priority of a thread blocked on an unavailable resource does not create that resource.

**Practice:** E03. Draw the ownership graph before writing the program.

### 6.1 Choose a process boundary by following failure

Start with one process containing acquisition and logger threads. Passing a record through a shared queue is convenient: both threads see the same C++ objects. However, an invalid memory access in either thread can compromise the entire process. They also share a process-level deployment and privilege context.

Now move logging to a separate process. Acquisition must send records through IPC instead of borrowing the logger's ordinary pointers. This adds protocol and recovery work, but the logger can have a separate lifetime and restricted storage access. If it exits, acquisition can report a fault or drop records according to policy rather than necessarily exiting with it.

Choose the boundary because of ownership, privilege, or failure requirements. Do not create a process for every small function or a thread for every incoming record. Both approaches can create uncontrolled resource growth.

### 6.2 Work backward from destruction

Consider the section 9 queue. Main owns the queue, the consumer borrows it, and only the consumer updates the result counters. The required ordering is:

```text
construct queue -> start consumer -> produce -> close queue
                -> consumer drains and exits -> join -> destroy queue
```

Why must destruction be last? A blocked `pop()` is still using the mutex, condition variable, and queue state. "The worker is not executing right now" does not mean it no longer needs those objects.

Why close before joining? When the queue is empty, the consumer cannot infer whether more work will arrive. Closure supplies that missing information. A join by itself does not change the queue predicate.

Why join producers before normal close? A still-running producer could have legitimate data left to submit. Closing first would reject it. During an abort, that rejection may be the intended policy; during a normal drain, finish production first.

### 6.3 Startup can fail halfway

Suppose worker one starts, but creating worker two throws. Stack unwinding destroys local C++ objects. If worker one's `std::thread` is still joinable, destroying it terminates the process. Therefore partial startup needs the same careful lifecycle as shutdown:

1. Track which workers actually started.
2. Publish a stop/close state that those workers can observe.
3. Wake affected waits.
4. Join started workers before releasing their dependencies.
5. Report startup failure to the caller.

An exception leaving a thread's entry function also terminates the process unless handled inside that thread. Catch appropriate failures at worker boundaries, publish a synchronized failure result, and trigger the chosen stop policy. Catching an error and silently continuing is not recovery.

**Self-check:** is `detach()` a solution to the failed startup? No. It removes the join obligation from that thread object but leaves the running thread's borrowed-object lifetimes unresolved.

### 6.4 Build a lifetime graph before choosing smart pointers

An ownership graph describes which object is responsible for releasing another object. A use-dependency graph describes which participant still needs an object. These graphs need not be identical: main can own a queue while several workers borrow it.

Suppose main owns a queue, a file, and a logger worker. The worker borrows the queue and file. The queue must remain usable until the worker stops popping, and the file must remain open until the worker stops writing. Therefore "destroy members in the order they appear in the diagram" is not enough. Destruction must respect the use dependencies.

In C++, members are destroyed in reverse declaration order after the destructor body. That rule can support a design, but it is not a substitute for stopping and joining a thread in time. A `std::thread` member's destructor does not automatically perform the cooperative shutdown protocol.

`shared_ptr` can keep an object alive, but it does not force the associated work to finish. Reference cycles can retain objects indefinitely, and the last release may run a costly destructor on an unexpected thread. Prefer a clearly bounded owner/borrower relationship when the component has a well-defined startup and shutdown lifetime.

### 6.5 Cancellation requires cooperation at every wait

A stop flag is information, not a universal interrupt. A worker executing a short calculation can check it between jobs. A worker waiting on a condition variable needs the stop state in its predicate plus a notification. A worker blocked in an external service needs that operation's supported timeout or cancellation behavior.

List every blocking location on the worker's path. For each one, identify its normal completion event and its stop path. If one location can wait forever without observing termination, the whole worker has an unbounded shutdown path even if every other location handles stop correctly.

For example, closing the acquisition queue wakes its consumers, but a logger already stuck in an output operation is no longer waiting on that queue. Queue closure alone cannot unblock the output call. The architecture needs a bounded I/O design or a separate service boundary with an explicitly defined supervisor action.

Forced process termination may be an escalation for a disposable worker service, but it can lose in-memory data and leave external effects uncertain. It is not equivalent to graceful completion and should never be described as such.

### 6.6 Fault containment versus shared fate

Two threads in one process share fate for many failures: an unhandled thread exception or serious memory fault can terminate the process. Separate processes can make independent restart possible, but they introduce stale endpoints, partial failures, and protocol compatibility concerns.

An architect chooses a process boundary where independent behavior has value. A sensor that must continue while storage restarts is a stronger reason than "we have many classes." Conversely, splitting a tightly coupled object graph across processes without redesigning its synchronous dependencies can create a fragile network of waits.

**Worked decision:** acquisition and its fixed-size filter share a short, predictable lifetime and can remain in one process. Storage has variable latency, different permissions, and a restart requirement, making it a candidate for a separate service. A supervisor must not depend exclusively on the storage service to report that storage is unhealthy.

## 7. Real-Time Scheduling

### Priority and policy

QNX generally chooses the highest-priority eligible READY thread. Higher numerical priorities represent higher priorities. On SMP, eligibility includes CPU restrictions and the state of other CPUs; a single-core sketch is not a complete multicore analysis.

| Policy | Basic idea | Important caution |
|---|---|---|
| `SCHED_FIFO` | Highest-priority eligible threads run; equal-priority ordering follows FIFO rules | A CPU-bound thread can starve lower-priority work |
| `SCHED_RR` | Round-robin time slicing among equal-priority eligible threads | Does not provide fairness to lower priorities |
| `SCHED_SPORADIC` | Budget/replenishment-based scheduling behavior | Study support and parameters in your release before using it |

Policy, priority, affinity, permissions, and inheritance must be inspected, not assumed. Discover permitted ranges using `sched_get_priority_min()` and `sched_get_priority_max()` and check the result of every change. Explicit thread attributes may require `PTHREAD_EXPLICIT_SCHED`; otherwise inherited scheduling can override your intended configuration.

Do not start learning by requesting the maximum priority. Stay within the permitted nonprivileged range and use finite work. Some changes require abilities or policy approval.

### Priority inversion

```text
Low-priority thread owns mutex M.
High-priority thread needs M and blocks.
Medium-priority CPU work delays the low-priority owner.
High-priority work is indirectly delayed by medium-priority work.
```

Priority inheritance can temporarily raise the owner's effective priority so it can release the resource. It does not fix deadlock, unbounded critical sections, slow I/O under a mutex, or overload.

For a mutex experiment, explicitly configure `PTHREAD_PRIO_INHERIT` through pthread mutex attributes and check whether the configuration succeeded. Do not assume `std::mutex` provides a particular real-time protocol.

QNX also has message-related priority inheritance and server-boost behavior. This is distinct from mutex inheritance. Channel flags and server structure affect it; see the official architecture documentation before relying on a priority model.

### Timing quantities

Let `release` be when a job should become ready, `start` when it begins execution, and `finish` when its result is available:

```text
start lateness  = start - release
response time    = finish - release
deadline miss   = finish > absolute deadline
```

If `release` is the actual instant the thread became READY, the first difference measures READY-to-start dispatch delay. If it is a nominal timer deadline, the difference can include timer-delivery delay too. Call that start lateness and state the measurement points.

Execution time, blocking time, interference from other work, and I/O delays all contribute to a response-time argument. A single observed maximum is not a proven WCET.

For periodic tasks on one CPU, a useful first calculation is:

$$
U = \sum_i \frac{C_i}{T_i}
$$

Here $C_i$ is execution demand and $T_i$ is period. Utilization below 100% alone does not prove deadlines are met. Under the classic assumptions of independent, preemptive periodic tasks with deadlines equal to periods on one CPU, rate-monotonic scheduling has the sufficient bound $U \le n(2^{1/n}-1)$. Real locks, interrupts, IPC, and multicore behavior require additional analysis.

**Practice:** E04-E06. State the assumptions beside every timing result.

### 7.1 Solve a scheduling timeline by hand

Assume one CPU, preemptive fixed-priority scheduling, no locks, and zero switching overhead for this paper example. These assumptions make the arithmetic understandable; they are not claims about actual overhead.

| Job | Release | CPU work required | Priority |
|---|---|---|---|
| Logger | 0 ms | 5 ms | Lower |
| Sensor | 1 ms | 2 ms | Higher |

At 0 ms, only the logger is READY, so it runs. At 1 ms the sensor becomes READY and preempts it. The sensor executes for 2 ms and finishes at 3 ms. The logger has completed only 1 ms of its 5 ms demand; it resumes and finishes at 7 ms.

```text
time:    0       1               3                               7
CPU:     |logger |    sensor     |             logger             |
```

The sensor's response time is 2 ms. The logger's response time is 7 ms even though its execution demand is 5 ms. Preemption accounts for the extra elapsed time.

Now suppose the sensor blocks at 1 ms waiting for data that will arrive at 4 ms. It cannot use the CPU simply because it has higher priority. The logger can continue until the sensor becomes READY at 4 ms. Always determine readiness before comparing priority.

**Self-check:** if the sensor must finish by 2 ms, does assigning it the higher priority solve the first case? No. Its own 2 ms work after a 1 ms release already requires until 3 ms under these assumptions.

### 7.2 Equal priority is a separate question

For two equal-priority CPU-bound threads under FIFO scheduling, the running thread does not hand over the CPU merely because a time slice expires. It may block, yield, exit, or be preempted by higher-priority work. Under RR, time-slice expiration can rotate execution among eligible equal-priority threads.

Neither policy means "give every thread an equal share." A continuously READY higher-priority thread can prevent lower-priority work from running under both. More cores change which threads can run simultaneously, but affinity can still force several threads to compete for one core.

A periodic thread is usually blocked between releases. A permanently busy high-priority polling loop turns that intermittent demand into continuous demand. That is why replacing polling with a meaningful wait can improve the whole system.

### 7.3 Work through priority inversion numerically

Use one CPU. Low owns a mutex and still needs 2 ms of CPU to release it. High then requests the mutex and blocks. Medium has 10 ms of independent CPU work ready.

Without inheritance, medium can execute its 10 ms before low receives the CPU again. High can wait about 12 ms: medium's 10 ms plus low's remaining 2 ms, ignoring overhead.

With supported priority inheritance, high's wait raises low's effective priority through the mutex protocol. Low can finish its 2 ms ahead of medium, unlock, and allow high to continue. The protected work is still necessary; inheritance removes this medium-priority interference, not the lock-held work itself.

If low is instead waiting for a slow device while holding the mutex, boosting it does not make the device finish. The design improvement is to move slow I/O outside the critical section when the invariant permits it, or redesign the dependency.

**Self-check:** can priority inheritance solve A waiting for B while B waits for A? No. Both are blocked by the dependency cycle, regardless of their priority.

### 7.4 Learn a small response-time calculation

This optional calculation is for independent periodic tasks on one CPU with fixed priorities, known execution bounds, deadlines no longer than periods, and the simplified blocking model stated here. Interrupt costs, release jitter, scheduler overhead, and other interference must be included in a real analysis.

Suppose a high-priority sensor needs 1 ms every 5 ms. A lower-priority worker needs 3 ms every 10 ms, has no lock blocking, and must finish within 10 ms of release. Start by assuming the worker needs only its own 3 ms. During that response interval, one sensor job can interfere, giving 4 ms. Repeating the calculation still gives 4 ms, so it has converged.

The familiar recurrence expresses that reasoning:

$$
R_i^{(k+1)} = C_i + B_i + \sum_{j \in hp(i)} \left\lceil \frac{R_i^{(k)}}{T_j} \right\rceil C_j
$$

Here $R_i$ is the response bound being estimated, $C_i$ is that task's execution bound, $B_i$ is accounted blocking, and $hp(i)$ is the set of higher-priority tasks. The ceiling counts interfering releases. Start with $C_i+B_i$; iterate until stable or beyond the deadline. A deadline overrun means this sufficient test did not establish the requirement under the model.

For our worker: $3 + \lceil 3/5 \rceil \times 1 = 4$, then $3 + \lceil 4/5 \rceil \times 1 = 4$. This is a result about the stated model. Substituting averages for execution bounds does not turn it into a real-time guarantee.

**Checkpoint before experimenting:** distinguish execution demand, time waiting for a resource, and time READY but preempted. A timestamp pair alone often cannot tell them apart.

### 7.5 End-to-end response is a chain, not one priority

Suppose a command travels through validation, queueing, computation, storage, and reply delivery. Its deadline applies to a specified endpoint of that chain. Making the computation thread high priority does not bound the time already spent waiting in a queue or the time spent waiting on storage afterward.

A simple budget can divide a 10 ms requirement into 1 ms for receipt and validation, 2 ms for queueing, 3 ms for computation, 2 ms for completion delivery, and 2 ms of justified margin. These are proposed allocations, not measurements. Every allocation needs an argument under the permitted workload, and the boundaries must avoid omissions or double counting.

If storage cannot supply an appropriate bound, either it cannot be part of this 10 ms completion milestone or the requirement is not established. Returning "accepted into bounded RAM" can be a legitimate alternative only if the caller agrees that acceptance, rather than persistence, is the required result.

### 7.6 Blocking analysis depends on the lock protocol

Priority inheritance limits particular lower-priority interference, but it does not give a universal blocking bound. Nested locks, multiple resources, repeated acquisitions, and other wait dependencies affect the bound. An analysis cannot simply insert "one longest critical section" without checking whether the selected protocol and assumptions justify that value.

For a beginner example, suppose high requests one mutex once per job, only low holds it, and low's remaining protected execution is bounded by 0.5 ms with no I/O or nested waits. That identifies a concrete source of blocking. If low calls an external service inside the critical section, the 0.5 ms arithmetic-only measurement no longer bounds the wait.

For architectural work, record the resource-access graph alongside the task table. The task table describes CPU demand and periods. The resource graph explains dependencies that utilization alone cannot represent.

### 7.7 Why multicore is not single-core analysis multiplied

On two cores, two eligible threads can execute concurrently, but a single mutex still serializes its protected state. A thread restricted to core 0 cannot benefit from an idle core 1. Shared caches and memory bandwidth can alter execution costs even when threads run on different cores.

**Partitioned scheduling** assigns work to particular cores or core sets, making some interference relationships easier to analyze but potentially wasting spare capacity elsewhere. **Global scheduling** permits broader migration and sharing, but the scheduling and cache behavior need a different analysis. These are design concepts, not a claim that every QNX policy implements every research model.

Two independent counters on the same cache line can cause repeated transfer of cache ownership between cores even when the source variables are distinct. This is **false sharing**: a performance issue arising from physical storage granularity, not necessarily a C++ data race. Measure before adding padding, because padding also changes memory footprint and layout.

Affinity can improve predictability or locality, but it can also concentrate interrupts and application work on one CPU. Record both application placement and relevant system activity before attributing improvement to affinity alone.

### 7.8 Admission control makes timing assumptions enforceable

An analysis assuming at most ten requests per second is invalid if the interface accepts unlimited requests. **Admission control** enforces limits on the workload entering the system. It may reject new work, reserve capacity for critical operations, or constrain the number of outstanding requests per peer.

Suppose status queries and STOP commands share one worker queue. A flood of expensive status requests can delay STOP even when STOP is logically more important. Possible designs reserve control capacity, make status cheap through a prepared snapshot, or use a separately bounded control path. Each introduces its own synchronization and ordering questions.

Priorities decide among eligible work; admission and capacity policies decide how much work exists and where it waits. Both are needed when external clients can invalidate the timing model.

**Architect checkpoint:** identify the largest unsupported assumption in a deadline claim. If it is "storage is normally fast" or "clients will not send too much," the next step is an enforceable contract or a redesigned milestone, not another priority increase.

## 8. Thread Synchronization

| Mechanism | Use it for | Avoid this mistake |
|---|---|---|
| Mutex | Protecting an invariant across shared state | Holding it across slow I/O or a synchronous IPC call |
| Condition variable | Waiting for a predicate to become true | Treating notifications as stored work items |
| Semaphore | Counting available resources or events | Assuming it has mutex ownership/inheritance semantics |
| Atomic | Carefully defined small state transitions | Assuming an atomic flag makes a whole algorithm safe |
| Barrier | Coordinating a fixed group at a phase boundary | Forgetting a participant might fail to arrive |

Condition-variable rule: modify and inspect the predicate under the associated mutex, and wait in a predicate loop. Spurious wakeups are allowed. A notification before a waiter starts is harmless only if the predicate records the condition correctly.

`volatile` is not thread synchronization. Unsynchronized concurrent access involving a write to an ordinary C++ object is a data race and undefined behavior.

Prefer cooperative shutdown:

```text
request stop -> prevent new work -> wake blocked waiters
             -> drain or discard by policy -> join -> release resources
```

Destroying a queue, channel, mutex, or connection while another thread uses it is not a shutdown protocol.

### 8.1 Lesson: protect a rule, not just a variable

Suppose a sensor publishes a value and a flag saying the value is available. The rule is: whenever a consumer sees `ready == true`, the associated value must already be valid. That relationship is an **invariant**.

Putting a mutex around only the assignment to the value is insufficient if the consumer reads the flag or value without that mutex. Both sides must use the same synchronization protocol. The lock gives two things: exclusion while changing the state, and the ordering needed for the next lock owner to observe those changes.

Choose the mechanism by asking what must be coordinated:

1. One independent count? An atomic read-modify-write may suffice.
2. Several fields that must agree? Start with one mutex protecting the whole relationship.
3. A thread needs to wait for that relationship to become true? Add a condition variable associated with that mutex.
4. A fixed number of available permits? A semaphore may express that count.
5. Every participant must finish phase one before phase two? A barrier expresses that rendezvous, but needs a participant-failure policy.

An atomic counter and an ordinary vector do not automatically become a safe queue together. The queue's contents and indexing rules still need a correct protocol.

### 8.2 Complete lesson: deliver one sample without polling

**Environment:** Linux. **Prerequisite:** section 0.5's lambda and `join()` explanation. Use the lab filename `one_sample.cpp`.

This is deliberately a one-sample handoff, not a reusable queue. Both producer-first and consumer-first execution must work.

```cpp
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

int main() {
    std::mutex sample_mutex;
    std::condition_variable sample_changed;
    int sample = 0;
    bool ready = false;
    int received = 0;

    std::thread consumer([&] {
        std::unique_lock<std::mutex> lock(sample_mutex);
        sample_changed.wait(lock, [&] { return ready; });
        received = sample;
    });

    {
        std::lock_guard<std::mutex> lock(sample_mutex);
        sample = 25;
        ready = true;
    }
    sample_changed.notify_one();

    consumer.join();
    std::cout << "received=" << received << '\n';
    return received == 25 ? 0 : 1;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror -pthread one_sample.cpp -o one_sample
./one_sample
```

Expected output: `received=25`.

Read each operation as part of a contract:

1. Main creates all shared objects before starting the consumer. They remain alive until after it is joined.
2. The consumer's `unique_lock` acquires the mutex. Unlike `lock_guard`, it supports the temporary unlock/relock required by a condition-variable wait.
3. The predicate overload checks `ready` while owning the lock. If false, waiting atomically releases the mutex and enters the wait with respect to the condition-variable protocol.
4. Main acquires the same mutex, writes the sample, and sets `ready`. The braces end the lock's lifetime and release it.
5. Main notifies. A waiting consumer can wake, acquire the mutex again, and recheck `ready`.
6. The consumer reads the sample under the mutex and returns. Main joins before reading `received`.

**Consumer-first trace:** consumer checks false and waits; main publishes 25 and notifies; consumer checks true and copies 25.

**Producer-first trace:** main publishes 25 and notifies before the consumer waits; consumer later locks and checks true, so it never needs to wait. No saved notification is needed because `ready` records the condition.

**Spurious-wakeup trace:** consumer wakes early, obtains the lock, sees false, and waits again. It does not copy the unready sample.

Calling `notify_one()` after unlocking is valid here. Notification does not publish the data by itself: the protected state and mutex protocol do that. Notifying while holding the mutex can also be correct, but the awakened thread must still acquire it before proceeding.

### 8.3 Turn the handoff into a reusable queue

The single `ready` flag cannot represent several pending samples. If main writes 25 and then 26 before the consumer copies anything, 25 may be overwritten. A queue adds storage and a count so each accepted item has its own place.

There are now two reasons to wait:

| Participant | Must wait when | Can continue when |
|---|---|---|
| Producer | Queue is full and still open | Space appears, or closure requires rejection |
| Consumer | Queue is empty and still open | Data appears, or closure permits termination |

Section 9 implements exactly these rules with two condition variables. Closure is permanent state, not a special notification. Every wait predicate includes closure so shutdown cannot leave a participant waiting for work that will never arrive.

### 8.4 Check your understanding

**Question:** could we replace the wait with a loop that repeatedly locks, checks, and unlocks? **Answer:** it could be race-free, but it would poll and consume CPU while empty. Waiting lets the scheduler use that CPU for other work.

**Question:** can we remove the predicate because there is only one consumer? **Answer:** no. Spurious wakeups remain possible, and notification might precede the wait.

**Question:** does an atomic stop flag wake a condition-variable waiter? **Answer:** no. A sleeping waiter needs a wakeup mechanism as well as a correctly synchronized termination predicate. For this queue design, keep closure under the queue mutex and notify all affected waiters.

**Exercise with answer:** identify the object protecting `received`. It is not read concurrently by main: the consumer is its only writer and main reads after joining. The join provides the necessary ordering. The sample itself is protected by the mutex while both threads may use it.

### 8.5 Semaphores count permits

A semaphore maintains a count of available permits. Waiting obtains a permit if one is available; otherwise the thread can block. Posting returns or adds a permit and can allow a waiter to proceed. Unlike a mutex, the mechanism is not fundamentally about a particular thread owning a protected region.

Imagine three identical processing buffers. A semaphore initialized to three can represent the number available. The first three users can each obtain a permit; the fourth waits until a buffer is returned. The actual choice of buffer still needs a correct ownership protocol. A count of three does not itself identify which three buffer objects are free.

For a producer/consumer queue, an item semaphore can start at zero. A producer stores an item safely and posts an available-item permit; a consumer waits for a permit and removes an item safely. A separate free-space count can limit capacity. Queue indices and storage still need protection unless the complete algorithm supplies it another way.

This differs from a condition-variable notification. Posting a semaphore changes its count; a condition-variable notification is not saved as a future work item. The condition-variable design keeps persistent truth in the queue predicate instead.

Shutdown requires care: posting a fake item permit to wake a consumer does not make a real item exist. Define a stop protocol that distinguishes termination from data and preserves counts. Do not substitute semaphores mechanically for the queue's two condition variables.

### 8.6 Barriers separate phases

A barrier is useful when a known group must all finish one phase before any participant starts the next. For example, three workers each compute one part of a simulation step. They arrive at a barrier, and the next phase begins only after all required arrivals.

A barrier is not a mutex: several workers can run concurrently before it. It is not a work queue: it does not carry arbitrary records from producers to consumers. Its count describes participants in a rendezvous.

If one required worker exits before arriving, the others may wait indefinitely unless the selected barrier mechanism and application define a way to adjust or cancel participation. C++17 does not have the standard C++20 `std::barrier`; choose a supported platform facility if the design genuinely needs this pattern.

### 8.7 Deadlock, starvation, and livelock have different causes

**Deadlock:** A owns the configuration mutex and waits for the queue mutex. B owns the queue mutex and waits for the configuration mutex. Both are waiting for something held by the other. Requiring every participant to acquire configuration before queue removes this particular circular ordering. Reducing unnecessary nested locks is another useful design improvement.

**Starvation:** a participant could make progress, but continually loses access to the CPU or another needed resource. An always-READY high-priority workload can starve lower-priority work. No circular wait is required.

**Livelock:** participants keep taking actions but do not finish useful work. For example, two peers can repeatedly abandon and retry a transaction in response to one another. The system is active, but the progress metric remains unchanged.

The repair depends on the cause. A lock-order rule addresses a lock cycle. Work budgeting and appropriate scheduling can address CPU starvation. Bounded retries with an arbitration or backoff policy can address some livelock patterns. Raising priority indiscriminately does not solve all three.

**Question:** two threads safely lock around individual map operations, but both observe a missing entry and later both create it. Is that necessarily a C++ data race? **Answer:** not if every individual access is synchronized correctly. It can still be a race condition in the larger check-then-act operation. Protect the whole invariant or use an operation that atomically performs the intended check and insertion.

### 8.8 Safety and liveness need separate arguments

A **safety property** says that something invalid never happens, such as removing an item from an empty queue or delivering the same accepted item twice. A **liveness property** says that something useful eventually happens, such as a waiting consumer receiving data when production continues under the required scheduling assumptions.

A queue can preserve its indices perfectly while every thread waits forever. That is safe with respect to those indices, but not live. Conversely, a program can continually produce output while corrupting shared state. Visible activity is not a safety proof.

For the condition-variable queue, safety comes from the protected invariant and predicate checks. Progress depends on producers or closure changing state, notifications making waiters eligible, and scheduling allowing them to run. Finite completion time needs stronger bounds beyond this eventual-progress reasoning.

### 8.9 A publication argument, step by step

Take the one-sample example. Main obtains the mutex, writes `sample`, sets `ready`, and unlocks. The consumer obtains the same mutex, sees `ready`, and reads `sample`. The mutex synchronization establishes the necessary ordering between the protected operations.

It is not the notification that transports the value 25. If the consumer starts after publication, it can read correctly without waiting for any notification. The predicate records the fact, the mutex orders access, and the condition variable avoids spending CPU while the fact is false.

Now change only `ready` to an atomic and write it without the mutex while retaining the old condition-variable wait. Even if a chosen atomic ordering correctly publishes data, the waiter can check false, miss a notification before actually waiting, and then sleep with the condition already true if the notification protocol is not coordinated. Solving publication does not automatically solve waiting. Keep this example's predicate changes under the associated mutex.

An atomic-only design can be correct with a different complete waiting protocol, but it must be designed and justified as such. Mixing pieces from two correct protocols does not guarantee a third correct protocol.

### 8.10 Lock granularity changes correctness and performance

A single mutex around related state makes the invariant easier to explain but can serialize unrelated operations. Splitting it into several locks can increase concurrency but introduces intermediate states, lock-order requirements, and more complicated snapshots.

Suppose configuration contains both a sample period and a buffer size derived from that period. If separate locks allow a reader to observe the new period with the old size, individually protected fields still do not form a coherent configuration. One lock, an immutable published snapshot, or another whole-configuration protocol is needed.

Begin with the simplest design that meets the required throughput and latency. Measure contention duration and frequency before splitting locks. The number of mutexes is not itself a performance metric, and a sophisticated lock-free label is not evidence of an architectural improvement.

## 9. Worked Example: Bounded Queue

**Environment:** Linux now; also suitable for rebuilding with the QNX C++ toolchain later.

This complete example demonstrates a fixed-capacity queue, predicate waits, backpressure, and drain-on-close behavior. It is instructional application code, not a hard-real-time guarantee: scheduling and blocking remain relevant, and `std::mutex` does not specify the priority protocol needed for all real-time designs.

Use the lab filename `bounded_queue.cpp` for this code block:

```cpp
#include <array>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>

class BoundedQueue {
public:
    bool push(int value) {
        std::unique_lock<std::mutex> lock(mutex_);
        space_available_.wait(lock, [this] {
            return closed_ || size_ < storage_.size();
        });
        if (closed_) {
            return false;
        }
        storage_[tail_] = value;
        tail_ = (tail_ + 1) % storage_.size();
        ++size_;
        data_available_.notify_one();
        return true;
    }

    bool pop(int& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        data_available_.wait(lock, [this] {
            return closed_ || size_ != 0;
        });
        if (size_ == 0) {
            return false;
        }
        value = storage_[head_];
        head_ = (head_ + 1) % storage_.size();
        --size_;
        space_available_.notify_one();
        return true;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        data_available_.notify_all();
        space_available_.notify_all();
    }

private:
    std::array<int, 8> storage_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
    bool closed_ = false;
    std::mutex mutex_;
    std::condition_variable data_available_;
    std::condition_variable space_available_;
};

int main() {
    BoundedQueue queue;
    long long sum = 0;
    int count = 0;

    std::thread consumer([&] {
        int value = 0;
        while (queue.pop(value)) {
            sum += value;
            ++count;
        }
    });

    for (int value = 1; value <= 1000; ++value) {
        if (!queue.push(value)) {
            queue.close();
            consumer.join();
            return 1;
        }
    }

    queue.close();
    consumer.join();
    std::cout << "count=" << count << " sum=" << sum << '\n';
    return count == 1000 && sum == 500500 ? 0 : 1;
}
```

**Linux build and run, from your lab directory:**

```sh
g++ -std=c++17 -Wall -Wextra -Werror -pthread bounded_queue.cpp -o bounded_queue
./bounded_queue
```

Expected output:

```text
count=1000 sum=500500
```

**QNX host build for x86-64:**

```sh
q++ -Vgcc_ntox86_64 -std=c++17 -D_QNX_SOURCE -Wall -Wextra -g bounded_queue.cpp -o bounded_queue
```

Transfer and run the QNX binary on the target, not in your Linux shell.

Why this works:

- The mutex protects the ring indices, size, storage, and closed state.
- Closing wakes both producers and consumers, including a producer waiting on a full queue.
- Existing items remain readable after close; only a closed, empty queue ends consumption.
- Only the consumer modifies `sum` and `count`; the main thread reads them after `join()`.
- Queue storage is bounded, but waiting time is not yet bounded. A deadline-aware API is an exercise, not an implied property.

**Practice:** E07 adds shutdown edge cases, multiple producers, and explicit timing behavior.

### 9.1 Trace the ring buffer

`head_` identifies the next readable slot. `tail_` identifies the next writable slot. `size_` distinguishes empty from full, because both can have equal head and tail indices. Modulo arithmetic wraps an index back to zero after the last slot.

Use capacity three on paper; the actual example has capacity eight:

| Operation | Head afterward | Tail afterward | Size afterward | Logical contents |
|---|---|---|---|---|
| Start | 0 | 0 | 0 | Empty |
| Push 10 | 0 | 1 | 1 | 10 |
| Push 20 | 0 | 2 | 2 | 10, 20 |
| Pop | 1 | 2 | 1 | 20 |
| Push 30 | 1 | 0 | 2 | 20, 30 |
| Push 40 | 1 | 1 | 3 | 20, 30, 40 |

The old bytes in a popped slot need not be erased for this integer queue. Its indices and count define which slots are live. The mutex prevents another thread from observing half of an index/count update.

### 9.2 Follow the two shutdown branches

**Empty consumer case:** `pop()` waits because `size_ == 0` and the queue is open. `close()` sets `closed_` while holding the mutex and notifies. On waking, the predicate succeeds because of closure; the size check finds zero, so `pop()` returns false and the consumer loop exits.

**Full producer case:** `push()` waits because all slots are occupied. `close()` wakes it. Its predicate succeeds, but the subsequent `closed_` check returns false from `push()`, rejecting the new value. Existing values remain available to consumers.

These are different outcomes: consumers drain already accepted data; producers cannot accept new data. Checking `closed_` at the very beginning of `pop()` and immediately returning would change the contract to discard queued data.

### 9.3 Capacity and latency are different bounds

A queue of eight integers bounds item storage. It does not limit how long a producer waits for a stalled consumer. To meet a producer deadline, choose a nonblocking rejection/drop policy or a timed operation with a defined timeout result.

For a deadline-based wait, retain one absolute deadline while rechecking the predicate. Restarting a full relative timeout after each wakeup could extend the total wait. Also remember that acquiring the mutex and being scheduled after a wakeup can contribute delay beyond the condition-variable wait itself.

If consumers cannot keep up over the long term, a bigger queue only postpones the full state. For a finite burst, a larger queue may be correct: at 100 incoming records/s and no consumption for 0.2 s, at least 20 free slots are needed just for those arrivals, subject to the burst model and extra margin. Existing occupancy also consumes capacity.

**Exercise with answer:** after pushing 10 and 20, close the queue and make three pops. The first two return true with 10 and 20. The third returns false. Another push returns false. Repeated close leaves the same closed state.

### 9.4 Establish the invariant before considering interleavings

For capacity eight, the queue requires `0 <= size_ <= 8`, valid head and tail indices, and a consistent logical sequence of live slots. Under the mutex, `push()` changes one unused slot into a live slot and increments size. `pop()` changes the oldest live slot into an unused slot and decrements size.

The initial state satisfies the invariant. A successful push proceeds only when space exists and the queue is open, so incrementing cannot exceed capacity. A successful pop proceeds only when an item exists, so decrementing cannot underflow. Modulo arithmetic keeps the indices within the array. Closure changes admission state without changing the accepted item sequence.

This is an inductive argument: a valid starting state and invariant-preserving operations keep the state valid. It assumes every operation follows the mutex protocol and the object remains alive. An unsynchronized debug read of `size_` would violate that assumption even if all push/pop code is correct.

### 9.5 Concurrent operations need a defined logical order

A concurrent operation may take an interval of time, but its effect can often be understood as occurring at a particular point within that interval. This is the intuition behind **linearizability**: operations appear to take effect in an order compatible with their real-time relationships.

For this mutex-based queue, successful state updates occur under exclusive ownership. A push racing with close either obtains its accepted position before closure or observes the closed state and is rejected. The contract does not depend on which call was merely started first while both overlapped.

Do not promise more ordering than the implementation provides. With two producers, FIFO preserves the order in which items are accepted into the queue, not a universal order based on when each producer began preparing its item. If source order matters across producers, carry sequence information and define the merge rule.

### 9.6 Returning from pop is not successful processing

After pop returns, the item belongs to the consumer's local work. If processing then fails, the queue does not automatically restore it. Therefore "every accepted item is popped once" and "every accepted item is durably processed once" are different guarantees.

Retries require a policy. Reinsert at the front and a permanently failing item may block all later work. Reinsert at the back and order changes. Drop it and completeness changes. Record it in a bounded failure channel and you introduce another capacity to manage. Select the policy according to the data's meaning.

A shutdown rule that drains the queue only establishes that queued items were removed before exit if downstream processing is not also tracked. Count in-flight work and define the completion milestone. For the simple integer sum, processing occurs immediately inside the consumer, so join also observes completion of that processing.

### 9.7 Queue capacity is a service guarantee question

A useful general view compares cumulative arrivals with the service that can be guaranteed. Backlog grows where accepted arrivals exceed completed service. A mean arrival rate below a mean service rate does not bound burst backlog or the wait of an individual item.

Under a simplified deterministic model, if at most 30 records can arrive at once and the consumer then handles one record per millisecond, the last burst record may wait roughly 29 ms before its own processing begins, even with no further arrivals. Plenty of memory does not establish a 5 ms response deadline for that record.

An architect must relate capacity, arrival bursts, service delays, and the full policy. A bounded queue is one building block, not the whole overload strategy.

## 10. Native QNX Message Passing

### The core transaction

```text
Server: ChannelCreate -> MsgReceive -----------------> MsgReply
                              ^                          |
Client: ConnectAttach -> MsgSend -------------------------+ -> returns
```

`MsgSend()` is synchronous: the client normally waits for both receipt and reply.

- If no receiver has accepted its message, the client can be **SEND-blocked**.
- Once received, the client becomes **REPLY-blocked** until the transaction completes or fails.
- A server waiting for an incoming message can be **RECEIVE-blocked**.
- If a receiver is already waiting, the client can proceed directly into REPLY blocking without an observable SEND-blocked interval.

### The three identifiers

| Identifier | Obtained from | Used for |
|---|---|---|
| `chid` | `ChannelCreate()` | Server receive operations |
| `coid` | `ConnectAttach()` | Client sends to a channel |
| `rcvid` | A positive return from `MsgReceive()` | Replying or reporting an error for that transaction |

`MsgReceive()` returns:

- A positive receive ID for a message requiring a reply or error response.
- Zero for a pulse; **do not call `MsgReply()` for a pulse**.
- Minus one on failure; inspect `errno`.

Many QNX APIs also have `_r` variants with different error-return conventions. Do not mix their error handling with the non-`_r` forms shown here.

### Protocol rules

Define a message type, protocol version, request ID, size limits, valid ranges, and reply semantics. Validate the actual received size before using fields. Never send `std::string`, `std::vector`, virtual objects, or raw pointers as if their in-memory representation were a portable message.

The server and client are separate address spaces. A pointer value from the client is not a usable pointer in the server. Even a plain struct needs an explicit representation if architectures, ABIs, or protocol versions may differ.

### Cancellation and failure

A missing reply can leave a client blocked indefinitely. Use appropriate timeouts for both send and reply states, handle every valid request exactly once, and define cleanup when a client disappears.

**A client timeout does not roll back work already performed by the server.** Retries of non-idempotent operations need request IDs, deduplication, or an application transaction protocol.

Avoid holding a mutex while calling `MsgSend()` into a service that may call back or need the same resource. Draw a wait-for graph whenever synchronous dependencies form a cycle.

### 10.1 Why a separate process needs messages

Imagine the sensor has calculated a temperature of 25 degrees and wants a logger process to store it. If the logger were an ordinary function in the sensor's process, the sensor could call `log_temperature(25)`. Both functions would use the same address space and the caller's thread would execute the function.

A separate logger process is different. It owns its own memory and has its own execution paths. The sensor cannot call an arbitrary function inside it as though that function were part of its own program. It must communicate through an interface both processes understand.

We divide that communication into three actions:

1. **Request:** the sensor describes what it wants, such as "store this temperature."
2. **Processing:** the logger validates the request and performs the agreed work.
3. **Reply:** the logger reports the result, such as "accepted" or "storage unavailable."

QNX native message passing provides the mechanism for carrying requests and replies. Your application defines what the requests mean. The kernel understands the communication operation; it does not automatically understand your temperature units or logging requirements.

### 10.2 A channel is where a server receives

A **server** is a role: a process provides a service to other participants. It need not be a remote computer. Our logger is a server on the same QNX machine as the sensor.

Before the logger can receive native messages, it creates a **channel**. Think of the channel as an OS-managed receiving endpoint. `ChannelCreate()` returns the channel ID, usually called `chid`. The logger supplies that ID when it calls `MsgReceive()`.

Creating a channel does not start a request-processing thread. The application must run a thread that receives and handles requests. If no thread receives, clients can wait even though the endpoint exists.

The server's PID and channel ID together identify the destination in this local lab. Channel ID 1 in one server is not necessarily the same endpoint as channel ID 1 in another server. A channel ID is not a pointer to server memory and not a TCP port number.

### 10.3 A connection is how a client addresses that channel

A **client** is the participant asking for a service. Our sensor is the logger's client. It calls `ConnectAttach()` with the server's endpoint information and gets a **connection ID**, usually called `coid`.

The client passes this connection ID to `MsgSend()`. It does not pass its own channel ID or the server's receive ID. The connection represents an attachment to a destination; it is not an individual request.

Here is an illustrative ownership picture. The numbers are examples, not values to hard-code:

```text
Sensor process                         Logger process
local connection ID 7  ------------->  channel ID 1
uses 7 for MsgSend                     uses 1 for MsgReceive
```

The client may send multiple requests through a valid connection. It detaches when finished. If the server exits and restarts, do not assume old endpoint information or connections automatically identify the replacement instance.

### 10.4 A receive ID belongs to one transaction

When `MsgReceive()` returns a positive value, the server has received a message and an associated **receive ID**, called `rcvid`. The server uses that value to reply to that particular request.

Suppose two clients send the same bytes. Their requests can still need separate replies to separate waiting callers. The receive ID lets the kernel distinguish those transactions. It is not a permanent client identity and should not be stored as the address for future unrelated messages.

Keep the three questions separate:

- Where does the server receive? Its `chid`.
- Which attached destination does the client send to? Its `coid`.
- Which received request does the server complete? Its `rcvid`.

An application **request ID** is a fourth, different idea: a field you define inside the payload to recognize the same logical operation across retries. A kernel receive ID does not replace that application field.

### 10.5 What synchronous sending actually does

"Synchronous" here means that the calling thread waits for the request/reply transaction to complete. It does not mean every thread in the client process must stop. Other threads may continue if they are runnable.

Follow one execution:

```text
Client thread                         Server thread
MsgSend begins                        doing earlier work
    waits for receipt                 calls MsgReceive
    waits for reply                   validates request
    waits for reply                   computes result
    eligible to resume                calls MsgReply
MsgSend returns when scheduled
```

Before receipt, the client can be **SEND-blocked**. After receipt, it is **REPLY-blocked**. These distinguish two different missing events. In the first case a receiver must accept the request. In the second case the accepted transaction still needs completion.

A server that calls `MsgReceive()` with no incoming work can become **RECEIVE-blocked**. That is often healthy idle behavior: it has no request to process and is not wasting CPU polling.

If the server is already receiving, the client can go directly into waiting for the reply without a visible SEND-blocked interval. A process-inspection tool samples state; failure to see a very short state does not prove the state model is wrong.

### 10.6 Where the request bytes go

The client has a request buffer in its address space. The server has a receive buffer in its address space. Messaging transfers bytes between them. The client also supplies storage for reply bytes; that is separate from the request buffer's role.

A pointer value inside a request does not make its pointed-to object transferable. If the client sends the bytes of a `std::string`, those bytes may include a pointer to storage that belongs only to the client. The logger cannot treat that pointer as its own valid string data.

Instead, define a transferable representation. For text, that might be a version, a bounded byte count, and the actual characters. For a sensor sample, it might be an integer measurement in explicitly stated units and a sequence number. When machines or ABIs differ, also define byte order, widths, and field layout rather than relying on compiler padding.

**Example:** integer `25000` can mean 25 degrees if the protocol defines millidegrees Celsius. Without that definition, another program might interpret it as 25000 degrees or as an unrelated raw ADC count. Correct bytes are insufficient without agreed meaning.

### 10.7 Why message length is checked before fields

Imagine the server expects eight bytes but receives only four. Reading all eight as a request would use bytes the client did not supply. Now imagine the client sends 100 bytes but the server receives only an eight-byte prefix. That prefix might look valid while concealing an invalid whole request.

For section 11's fixed-size protocol, the server checks both the full source length and the amount copied into its receive buffer. Only then does it inspect version and value. This ordering prevents the parser from trusting fields before establishing that those fields actually exist.

Validation and execution should be separate: first establish a valid request, then apply the requested state change. A rejected request should not leave half of a new configuration installed.

### 10.8 A timeout is not an undo operation

Suppose a client requests "increment the stored counter." The server increments from 9 to 10, but its reply is delayed. The client times out. What value does the server hold? It can already be 10. The timeout limits a wait; it does not reverse the increment.

If the client blindly retries, the server may increment to 11. That is why retry rules belong in the protocol. A repeated "set the value to 10" has different semantics from a repeated "add 1." An operation whose repeated application has the same intended effect is called **idempotent**.

For non-idempotent operations, a client can attach a logical request identity, and the server can remember the result of that operation. A repeated request can then receive the previous result instead of being performed again. This is **deduplication**. It still needs bounded retention, handling of conflicting reused IDs, and a restart policy; a number in a message alone provides none of these guarantees.

### 10.9 The QNX details that change timeout behavior

The example uses `ChannelCreate(0)`. With this setup, the client's timeout can end a SEND or REPLY wait. Other channel configurations are not interchangeable. With `_NTO_CHF_UNBLOCK`, after receipt the server can be notified of the client's attempt to unblock and becomes responsible for completing the transaction. A server that ignores that notification can defeat the intended cancellation behavior.

`TimerTimeout()` is thread-local and one-shot. Set it immediately before the call you want to protect. An intervening kernel call can consume/disarm the setting. A later send or retry needs a fresh setting. Restarting the full timeout on every retry can exceed an overall application deadline, so retain the original deadline or remaining budget.

These details follow the [QNX 8.0 timeout contract](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/t/timertimeout.html). They explain why adding channel flags or switching to a named-service framework requires reviewing the receive loop, not just changing the startup call.

### 10.10 Message priority is not mutex priority

With normal message-driven inheritance enabled, received messages and pulses affect the receiving thread's effective priority. Higher-priority incoming work can also trigger server boosting. This helps preserve the importance of work across a client/server dependency.

Unlike the simple mutex story, replying does not by itself restore the receiving thread's previous priority. Subsequent messages, pulses, explicit scheduling changes, permission limits, and channel flags affect it. `_NTO_CHF_FIXED_PRIORITY` disables the channel's normal message-driven priority inheritance.

If the receiver hands a request to an ordinary application worker queue, the worker's scheduling is now also part of the design. Do not assume a C++ queue automatically transfers native IPC priority relationships. The [QNX message priority explanation](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.sys_arch/topic/ipc_Priority_inheritance_messages.html) documents the native behavior.

**Questions with answers:**

1. **The server is alive but the client stays SEND-blocked. What is missing?** Receipt. Check the destination and whether a thread is actually receiving there.
2. **The client is REPLY-blocked. Does that prove the server is using CPU?** No. The server may itself be blocked, or it may have forgotten to reply.
3. **Can a positive receive ID be used twice to complete the same request?** No. Give each accepted transaction one defined completion path; do not treat its identifier as reusable after completion.
4. **Does successful messaging mean a log is durable?** Only if the application defines and implements that meaning for its reply. Acceptance in RAM is a different milestone.

### 10.11 Design a protocol as a contract between versions

Suppose a first service accepts a fixed request containing version and value, while a later service adds a command and request identity. Both programs being written in C++ does not establish compatibility. They need an agreed interpretation of each accepted representation.

For a general protocol, define header width, byte order where relevant, version, command, payload length, maximum sizes, and reply status. State which versions are accepted and how unsupported versions are rejected. Never read a newer field until the received length establishes that it exists.

There are two legitimate evolution approaches. A strict fixed-size protocol rejects any different size/version and requires coordinated deployment. An extensible protocol explicitly defines optional fields, unknown-field handling, and compatibility behavior. Silently accepting a longer message because its prefix looks familiar is not an extensibility design.

Authorization also belongs to the operation contract. A peer permitted to read status is not necessarily permitted to change the sampling period. Request validation establishes that a command is meaningful; authorization establishes that this caller may execute it.

### 10.12 Synchronous dependency cycles can cross process boundaries

Imagine sensor A sends synchronously to logger B. While handling it, B sends synchronously to diagnostic service C. C tries to query A, but A's only receiving path cannot run until its call to B returns. The wait cycle is A -> B -> C -> A. No mutex is required to create this deadlock.

Adding receiver threads can break a particular execution bottleneck, but it can also expose shared-state reentrancy and resource cycles. A clearer architectural rule is to constrain the synchronous call graph, avoid callbacks into blocked dependencies, or separate acceptance from later completion using a designed asynchronous protocol.

Asynchronous completion removes a particular waiting relationship but introduces state: a bounded set of outstanding operations, correlation IDs, completion delivery, timeouts, and abandoned-result cleanup. It changes the problem; it does not remove the need to solve it.

### 10.13 Cancellation is a state transition, not just a pulse

An application operation can be queued, running, committed, or completed. Cancellation while queued might remove it before side effects. Cancellation while running might require a cooperative check. After commitment, the correct response may be "too late to cancel" rather than pretending the effect was undone.

Write which participant owns each transition. If one worker performs the effect and another handles cancellation, they must synchronize the decision about whether commitment has occurred. Otherwise one path may report cancellation while the other path commits the change.

Native unblock/disconnect notifications tell the service something about the communication lifecycle. They do not define these application states. A production server must reconcile communication completion with its own work item, release associated resources, and avoid reusing a transaction identifier after its lifetime ends.

**Architect checkpoint:** draw both state machines, the kernel transaction and the logical application operation. A timeout may end or request the end of one while work in the other has already progressed. That mismatch explains many retry bugs.

## 11. Worked Example: QNX Request and Reply

**Environment:** build on a QNX SDK host; run only on a QNX target.

This single source builds an executable with `server` and `client` modes. The server prints its PID and channel ID, accepts one valid request, replies with twice the value, and exits. The client has a two-second kernel blocking timeout covering both send and reply states.

Manual PID/channel discovery keeps the first lab focused. It is not authentication or robust service discovery. Public channel creation and cross-process connections must be permitted by your target's abilities and security policy. Do not solve permission errors by disabling security.

Use the lab filename `qnx_ipc.cpp`:

```cpp
#include <sys/neutrino.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

struct Request {
    std::uint32_t version;
    std::int32_t value;
};

struct Reply {
    std::int32_t value;
};

bool parse_integer(const char* text, int minimum, int& result) {
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        parsed < minimum || parsed > INT_MAX) {
        return false;
    }
    result = static_cast<int>(parsed);
    return true;
}

int run_server() {
    const int channel_id = ChannelCreate(0);
    if (channel_id == -1) {
        std::perror("ChannelCreate");
        return 1;
    }

    std::printf("pid=%ld chid=%d\n", static_cast<long>(getpid()), channel_id);
    std::fflush(stdout);
    int result = 0;

    for (;;) {
        union Incoming {
            Request request;
            struct _pulse pulse;
        } incoming{};
        struct _msg_info info{};
        const int receive_id = MsgReceive(channel_id, &incoming,
                                          sizeof(incoming), &info);
        if (receive_id == -1) {
            if (errno == EINTR) {
                continue;
            }
            std::perror("MsgReceive");
            result = 1;
            break;
        }
        if (receive_id == 0) {
            continue;
        }
        if (info.srcmsglen != sizeof(Request) ||
            info.msglen < sizeof(Request)) {
            if (MsgError(receive_id, EMSGSIZE) == -1) {
                std::perror("MsgError");
            }
            continue;
        }

        Request request{};
        std::memcpy(&request, &incoming, sizeof(request));
        if (request.version != 1 || request.value < -1000 ||
            request.value > 1000) {
            if (MsgError(receive_id, EINVAL) == -1) {
                std::perror("MsgError");
            }
            continue;
        }

        const Reply reply{request.value * 2};
        if (MsgReply(receive_id, 0, &reply, sizeof(reply)) == -1) {
            std::perror("MsgReply");
            result = 1;
        }
        break;
    }

    if (ChannelDestroy(channel_id) == -1) {
        std::perror("ChannelDestroy");
        result = 1;
    }
    return result;
}

int run_client(int server_pid, int channel_id) {
    const int connection_id = ConnectAttach(0, server_pid, channel_id,
                                             _NTO_SIDE_CHANNEL, 0);
    if (connection_id == -1) {
        std::perror("ConnectAttach");
        return 1;
    }

    const Request request{1, 21};
    Reply reply{};
    std::uint64_t timeout_ns = 2000000000ULL;
    const int timeout_flags = _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY;
    int result = 0;

    if (TimerTimeout(CLOCK_MONOTONIC, timeout_flags, nullptr,
                     &timeout_ns, nullptr) == -1) {
        std::perror("TimerTimeout");
        result = 1;
    } else if (MsgSend(connection_id, &request, sizeof(request),
                       &reply, sizeof(reply)) == -1) {
        std::perror("MsgSend");
        result = 1;
    } else {
        std::printf("reply=%d\n", static_cast<int>(reply.value));
        if (reply.value != 42) {
            result = 1;
        }
    }

    if (ConnectDetach(connection_id) == -1) {
        std::perror("ConnectDetach");
        result = 1;
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::strcmp(argv[1], "server") == 0) {
        return run_server();
    }
    int server_pid = 0;
    int channel_id = 0;
    if (argc == 4 && std::strcmp(argv[1], "client") == 0 &&
        parse_integer(argv[2], 1, server_pid) &&
        parse_integer(argv[3], 0, channel_id)) {
        return run_client(server_pid, channel_id);
    }
    std::fprintf(stderr, "Usage: %s server | client PID CHID\n", argv[0]);
    return 1;
}
```

**Host build for x86-64:**

```sh
q++ -Vgcc_ntox86_64 -std=c++17 -D_QNX_SOURCE -Wall -Wextra -g qnx_ipc.cpp -o qnx_ipc
```

**QNX target, terminal 1:**

```sh
./qnx_ipc server
```

For example, if it prints `pid=12345 chid=1`, use those actual values in another target terminal:

```sh
./qnx_ipc client 12345 1
```

Expected client output is `reply=42`; both processes then exit successfully. Restart the server for another transaction. A server without a valid client remains in its receive loop; use Ctrl+C on the disposable lab server when finished.

### What this example deliberately leaves for later

- It trusts a same-build, same-ABI client/server layout. It is not a network serialization format.
- It does not authorize peers or create a production service namespace.
- It has one receiving thread and no persistent work queue.
- It ignores pulses because the lab does not request application or disconnect pulses. A server enabling notification flags must implement their handling and cleanup.
- The timeout bounds the relevant kernel blocking, not every instruction and scheduling delay around the call; it is not a hard end-to-end completion guarantee.
- It does not implement graceful signal-driven shutdown or restart-safe transactions.

For named services, study `name_attach()`, `name_open()`, and their matching cleanup APIs. Named-service examples have additional connection messages and disconnect/unblock handling requirements; do not replace `ChannelCreate()` with `name_attach()` and assume this loop is complete.

**Practice:** E08-E10. Learn the transaction before adding a thread pool or name service.

### 11.1 Read the program as two roles

`main()` selects `server` or `client` from the command-line arguments. A single launch runs only one role. Starting the server does not automatically create its client; the second terminal starts a separate process.

In server mode, `ChannelCreate(0)` creates the endpoint. Printing and flushing the PID and channel ID gives the lab user the information needed to connect. It does not authenticate the eventual client.

The receive loop allocates a union large enough for the request or a pulse. The return value determines which case to handle before any request field is read. `EINTR` means the receive was interrupted; this lab chooses to try receiving again. Other receive failures end the server with an error.

After validating size, `memcpy()` transfers the received representation into a separate `Request` object. The version must be 1 and the value must be between -1000 and 1000. This range also makes multiplication by two fit comfortably in the signed reply field.

`MsgError()` rejects an invalid transaction and the server continues receiving. `MsgReply()` completes a valid transaction, after which this deliberately one-shot server exits. A persistent service would normally loop after valid requests too and provide a separate shutdown mechanism.

### 11.2 Read the client from connection to cleanup

The client connects using the actual PID and channel ID. `_NTO_SIDE_CHANNEL` places the attachment in side-channel space rather than using an ordinary file-descriptor slot. The returned ID is used for native messaging, not as an open file.

`Request{1, 21}` means protocol version 1 and input value 21. `Reply reply{}` initializes reply storage before the call. The two-second timeout is expressed in nanoseconds: two billion. Its SEND and REPLY bits cover both kinds of waiting for this channel setup.

The next protected operation is `MsgSend()`. Do not insert a diagnostic print between it and `TimerTimeout()`: the print may invoke an intervening kernel call. On failure, the code reports the actual API error. On success, it also checks the application result, which must be 42.

The trusted lab server always returns this fixed reply. A less constrained protocol also needs an explicit way to establish and validate reply format and length. Transport completion alone does not prove that an arbitrary reply is meaningful.

Finally, the client detaches and the server destroys its channel. In a multithreaded service, these resources must stay alive until their users have stopped. This one-shot program keeps that ownership simpler.

### 11.3 Work three cases without guessing

**Case A: receipt is delayed by half a second.** The client can wait in SEND, then move to REPLY after receipt, then complete successfully. The server must still reply within the remaining budget.

**Case B: receipt is immediate but reply is delayed by half a second.** The client waits in REPLY. Its request is already in the server's application code; making the channel easier to discover will not shorten this wait.

**Case C: receipt is immediate but reply is delayed beyond the timeout.** With this example's channel setup, the client can time out. The server may already have computed the result. Its later attempt to reply may fail because the original transaction no longer exists. Observe the actual result on QNX; do not infer rollback.

**Self-check:** why does a receive buffer need room for a pulse when this lab sends only requests? `MsgReceive()` can deliver different kinds of native traffic. The code must safely distinguish the receive result before treating the buffer as a request.

### 11.4 Understand the five MsgSend arguments by ownership

In this example, the first argument identifies the client's connection. The next two describe the request address and byte count. The following two describe the reply-buffer address and its capacity. Grouping them into destination, outgoing buffer, and incoming buffer is less error-prone than memorizing positions.

The request and reply objects must remain alive for the duration of the operation, and concurrent code must not modify or inspect them in ways that violate the API and language requirements. The server's receive storage is a separate object; transferring bytes does not transfer ownership of the client's storage.

The return value is separate from the reply payload. Here the server supplies status zero and a reply containing twice the input. `-1` indicates failure for this non-`_r` call and makes `errno` relevant. A zero status does not automatically mean the payload is 42; the client checks that separately.

### 11.5 Map every server branch to a client outcome

| Server event | Transaction consequence | Application consequence |
|---|---|---|
| Receive fails before accepting a request | No newly accepted receive ID to complete | Server reports or retries its receive failure |
| Pulse is received | No reply transaction | Handle its code according to the channel's protocol |
| Request size is wrong | Attempt error completion with `EMSGSIZE` | Do not interpret unavailable request fields |
| Version or range is invalid | Attempt error completion with `EINVAL` | Do not compute a result for the rejected request |
| Request is valid | Attempt reply completion with status and payload | Client checks the expected result |
| Completion call fails | Transaction may already be unavailable | Report the failure; do not invent successful delivery |

The word "attempt" matters: a client can disappear or time out between validation and completion. Correct server error handling must tolerate that race without treating a stale transaction as a new request.

For the doubling operation, there is no persistent state change. Retrying the same value computes the same result. This makes it a good first lab, but it does not exercise durable deduplication or side-effect rollback. Do not infer those guarantees from its successful run.

### 11.6 From one receiver to a worker pool

A worker pool may allow one slow request to coexist with unrelated work. It also requires bounded pending work, ownership of request copies, synchronization of completion/cancellation, and a policy for client priority and fairness.

The stack-local `incoming` buffer in the example is reused on the next loop iteration. Passing a pointer to it to an asynchronous worker would let later receives overwrite the worker's input. A worker handoff needs its own owned, validated request representation with a lifetime extending through processing.

Keep the one-shot example simple until its transaction is understood. A production conversion is not merely putting the same receive call inside several threads: the application protocol and ownership rules must support concurrency.

## 12. Pulses, Events, and Timers

A pulse carries a small code and value. It is useful for notifications such as timer expiry or an internal wakeup. It does not establish a Send/Receive/Reply transaction.

| Message | Pulse |
|---|---|
| Supports request and reply payloads | Small notification |
| Sender can wait for server reply | No application reply is required |
| Positive receive ID | Receive result is zero |
| Natural fit for a command transaction | Natural fit for a wakeup or state-change notification |

Pulses still consume kernel resources and can fail or accumulate. Check API errors, reserve application codes in the documented range, and do not confuse system pulse codes with your own. A successful send is not proof that the receiver has processed the event.

### Periodic timer recipe

Use the matching release's `timer_create()`, `timer_settime()`, and `SIGEV_PULSE_INIT()` documentation to implement this sequence:

```text
create channel
attach a side-channel connection to that channel
initialize a pulse event with an application pulse code and permitted priority
create a CLOCK_MONOTONIC timer delivering that event
arm it with a nonzero first expiry and an interval
receive and validate pulses in a bounded event loop
disarm and delete timer
detach connection
destroy channel
```

This is an algorithm, not a compilable code block. Check every operation, clean up partial initialization, and keep event delivery endpoints alive until the timer is no longer active. Some event-delivery paths require registered events or permissions; use the exact API and release documentation rather than old example flags.

### Periodic does not mean drift-free execution

```text
Relative loop: work -> wait one period -> work -> wait one period
Absolute loop: next_release += period -> wait until next_release
```

Relative waits can accumulate execution time and scheduling delay. An absolute schedule keeps a fixed intended timeline but still needs an overrun policy: skip late work, catch up with a bounded limit, or report a fault.

Use monotonic time for elapsed durations. Wall-clock adjustments should not unexpectedly change a control-loop interval. Measure timer expiry, thread dispatch, and actual completion separately where possible.

In POSIX code, `clock_nanosleep()` returns an error number directly; do not blindly treat every nonzero return as `-1` plus `errno`. On interruption of an absolute wait, retry the same absolute deadline when that matches your cancellation policy.

**Practice:** E11. Count observed notifications and compare against intended releases; do not assume each pulse is proof of a completed job.

### 12.1 Clock, timer, event, pulse: four different things

A **clock** provides a time domain. A wall clock answers calendar-time questions; a monotonic clock is appropriate for elapsed-time schedules that should not jump when someone changes the date.

A **timer** records an expiry condition against a clock. Creating one does not start sampling. It exists initially disarmed; arming establishes when it should expire.

An **event description** says how expiry should be reported. For a pulse timer, it contains the destination connection, notification code, priority, and small value. Initializing this description does not create or arm the timer.

A **pulse** is the notification received through the channel. The receiver checks its code and decides what work to perform. The timer does not call your C++ sampling function simply because you chose that code.

```text
clock reaches expiry -> timer event delivers notification
                     -> receiver becomes eligible to run
                     -> receiver handles pulse and performs work
```

Each arrow can involve delay. "Timer period is 10 ms" is not the same statement as "my work finishes every 10 ms."

### 12.2 First expiry and repetition interval

The POSIX timer setting has two parts: `it_value` for the first expiry and `it_interval` for repetitions. With relative arming, a first value of 500 ms and an interval of 100 ms means intended expiries roughly 500, 600, 700 ms after arming, and so on.

A zero initial value disarms the timer even if an interval is present. A nonzero initial value and a zero interval make a one-shot timer. Time fields use seconds plus nanoseconds; the nanosecond component must be normalized below one billion.

For native pulse delivery, the event contains a connection ID, so creating a channel alone is insufficient: attach a connection to that channel as the destination. Choose a permitted priority and an application pulse code. A zero receive ID tells you "pulse"; the pulse code tells you which notification it is.

The [QNX 8.0 timer reference](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/t/timer_create.html) states that this timer event does not need registration. That statement is specific to the API; do not generalize it to every event-delivery mechanism.

### 12.3 Why relative waits drift

Suppose work takes 3 ms and is followed by a 10 ms relative sleep. Ignoring other delays, the next iteration starts 13 ms after the previous one. Repeating this loop accumulates the work duration into its schedule.

For a fixed 10 ms timeline, keep intended releases at 10, 20, 30, and 40 ms. If the first job starts at 10 and finishes at 13, wait until 20 rather than waiting another 10 from 13. This preserves the intended phase, although actual wakeup may still be late.

An **overrun policy** decides what happens when work falls behind. Catching up means executing missed jobs; skipping means discarding obsolete releases; faulting means reporting the timing failure and entering a defined state. None is universally correct. A display update often prefers the newest state, while accounting work may not be discardable.

### 12.4 Complete example: measure wakeup lateness

**Linux example:** use the lab filename `periodic_timing.cpp`. This demonstrates a monotonic absolute timeline without native QNX calls.

```cpp
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

int main() {
    using Clock = std::chrono::steady_clock;
    constexpr std::size_t sample_count = 100;
    const auto period = std::chrono::milliseconds(10);
    std::array<long long, sample_count> lateness_us{};
    auto next_release = Clock::now() + period;
    std::size_t skipped_releases = 0;

    for (std::size_t index = 0; index < sample_count; ++index) {
        std::this_thread::sleep_until(next_release);
        const auto actual_start = Clock::now();
        lateness_us[index] =
            std::chrono::duration_cast<std::chrono::microseconds>(
                actual_start - next_release).count();

        next_release += period;
        const auto finished = Clock::now();
        if (finished >= next_release) {
            const auto missed = (finished - next_release) / period + 1;
            skipped_releases += static_cast<std::size_t>(missed);
            next_release += period * missed;
        }
    }

    std::sort(lateness_us.begin(), lateness_us.end());
    std::cout << "samples=" << sample_count
              << " min_us=" << lateness_us.front()
              << " p50_us=" << lateness_us[49]
              << " p95_us=" << lateness_us[94]
              << " p99_us=" << lateness_us[98]
              << " max_us=" << lateness_us.back()
              << " skipped=" << skipped_releases << '\n';
    return 0;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror -pthread periodic_timing.cpp -o periodic_timing
./periodic_timing
```

The output begins with `samples=100`. Other values depend on your machine and load; there is no universal correct microsecond value.

`steady_clock` provides the monotonic time basis. `next_release` is an absolute point on that clock. `sleep_until()` waits for that point, and subtracting it from the observed start gives wakeup lateness. The array stores measurements without growing a container during the loop.

`next_release += period` advances the intended schedule, not the observed wake time. If the next intended release has already passed when this job finishes, the division calculates how many obsolete releases to skip. We do not execute an unlimited backlog of catch-up jobs.

For example, if the first job was due at 10 ms and finishes at 36 ms, the next candidate is 20 ms. Releases at 20 and 30 are skipped, leaving 40 ms as the next target. Choosing 46 ms instead would reset the schedule's phase.

Sorting occurs after measurement. For these exactly 100 samples, indices 49, 94, and 98 give nearest-rank p50, p95, and p99. Change that calculation if you change sample count. These statistics summarize observed lateness, not a proven worst-case bound.

### 12.5 Ending a timer-driven service

Stop future timer generation before destroying the connection and channel that receive it. Otherwise an active source refers to a destination whose intended lifetime has ended. Handle already queued notifications according to your shutdown rule; stopping a timer is not the same as deleting every earlier notification.

If worker threads also process timer-triggered jobs, stop or drain those jobs before destroying their data and synchronization objects. There are two lifecycles: the notification source and the work it has already caused.

**Question:** does receiving ten timer pulses prove ten jobs completed on time? **Answer:** no. You know that notifications were received. Measure intended expiry, actual handling, and completion to make a timing claim.

### 12.6 Timestamp meaning and clock-domain boundaries

A timestamp has a value and a reference domain. A monotonic timestamp is useful for intervals within that clock's lifetime, but it is not automatically comparable across two machines or across a reboot. A calendar timestamp is useful to humans but may be corrected while a system is running.

For local scheduling, keep intended release, observed start, and finish in one suitable monotonic domain. For records sent to another machine, include enough identity and clock information to interpret them. A boot or service generation can distinguish a new lifetime whose sequence numbers or timestamps restarted.

Cross-machine latency measurement needs clock synchronization with a known uncertainty, or a measurement design that avoids directly subtracting unrelated clocks, such as round-trip timing on one host. Even synchronized clocks have error bounds; a claimed microsecond latency cannot ignore a larger synchronization uncertainty.

### 12.7 Timer resolution, precision, and accuracy are different

**Resolution** concerns the granularity a clock or timer exposes. **Accuracy** concerns how close a measurement or event is to the intended reference. **Precision** is often used for repeatability or numerical detail; state which meaning you intend.

A timestamp expressed in nanoseconds does not prove nanosecond accuracy. A fine-resolution timer can still deliver a late notification under load, and a runnable receiver may wait for CPU after notification. Separate expiry behavior from dispatch delay and processing duration.

The periodic example records observed lateness only for jobs it actually executes. Skipped releases are counted separately. Dropping missed jobs without reporting them would make the remaining latency distribution look better while hiding a service-quality failure.

### 12.8 Choose whether notifications represent work or merely wakeups

For a latest-value display, several elapsed periods might collapse into one useful update using the newest state. For accounting, every event may represent work that must be preserved. A pulse's small payload and delivery semantics do not determine which application interpretation is correct.

When correctness requires exact work accounting, maintain explicit bounded state or sequence information and use notifications to prompt examination of that state. Do not infer "one completed job per elapsed period" solely from how many times a receive loop ran.

For overload, define the maximum amount of work handled before the receiver returns to other responsibilities. An unbounded catch-up loop can starve status, cancellation, and shutdown handling. This connects timer policy back to admission and scheduling rather than treating timers as isolated utilities.

## 13. Shared Memory and IPC Selection

Shared memory lets processes map common storage. It does not supply a queue, synchronization, ownership, or recovery protocol.

Typical POSIX lifecycle:

```text
creator: shm_open -> ftruncate -> mmap -> initialize -> publish ready
peer:    shm_open -> validate size/readiness/version -> mmap -> use
both:    stop access -> munmap -> close
owner:   shm_unlink according to the agreed lifecycle
```

Process-shared mutexes and condition variables need explicit supported attributes and correct placement in shared storage. Ordinary process-local `std::mutex` is not a portable interprocess synchronization solution. Avoid raw pointers in shared memory; use validated offsets and lengths relative to the mapping.

| Requirement | Candidate | Main design cost |
|---|---|---|
| Small local request with a result | Native QNX message | Blocking, reply lifecycle, deadlines |
| Small asynchronous wakeup | Pulse | Resource limits and notification interpretation |
| Bulk local data | Shared memory plus notification | Ownership, visibility, bounds, crash recovery |
| Portable communication between machines | TCP/UDP sockets | Framing, connection lifecycle, security, congestion |
| File-like device interface | Resource manager | I/O semantics and per-open state |
| Queued discrete payloads | POSIX message queue | Limits, overload policy, service availability |

A common design uses messages for control, shared memory for data, and pulses for notification. This is useful only when its complexity is justified by measurements.

**Practice:** E12. Write the recovery protocol before optimizing copies away.

### 13.1 How different addresses can refer to the same storage

A process normally uses **virtual addresses**. The processor's memory-management hardware translates those addresses through mappings established by the OS. Two processes can both use address `0x1000` without accessing the same physical memory. The address only has meaning within its mapping context.

Shared memory arranges for a region in each process to refer to common backing storage. The virtual starting addresses need not match:

```text
Writer's view                         Reader's view
mapping starts at 0x100000             mapping starts at 0x800000
record is at offset 64                record is at offset 64
address used: 0x100040                 address used: 0x800040
                 \                   /
                  same shared bytes
```

The addresses are illustrative. What both participants can agree on is the **offset** within the shared region. Sending the writer's absolute address to the reader is wrong: the reader must derive its own address from its own mapping base and a validated offset.

Shared memory can avoid some application data copying for large buffers. It does not remove the need to transfer ownership, make writes visible correctly, or prevent concurrent use of the same slot.

### 13.2 Understand each setup operation

`shm_open()` creates or opens a named shared-memory object and returns a descriptor. A newly created object does not automatically have space for your data structure. The creator establishes the intended size with `ftruncate()`.

`mmap()` maps the object into a process's address space. For shared updates, the mapping must have appropriate shared semantics and permissions. Mapping failure is indicated by `MAP_FAILED`, not by a general assumption that every pointer-returning API fails with null.

Initialization is an application responsibility. The creator must initialize metadata, storage, and supported process-shared synchronization before allowing peers to use them. Another process opening the name does not prove initialization has finished. Checking the object's size also does not prove its contents are ready.

A simple startup design uses a separate control channel: the owner creates and initializes the region first, then replies to a peer's request with the region name, size, version, and generation. The peer maps and validates that published instance. This avoids using an uninitialized shared mutex to protect its own initialization.

At shutdown, `munmap()` removes a process's mapping and `close()` releases its descriptor. `shm_unlink()` removes the shared-memory name. Removing the name does not immediately invalidate existing mappings in other processes. Ownership must specify who may remove the name and when old participants have stopped using the region.

### 13.3 Why a shared struct is not yet a protocol

Suppose a shared record contains a sequence number and a temperature. The writer wants to change `(sequence=10, temperature=25000)` to `(sequence=11, temperature=26000)`.

Without synchronization, a reader might observe the new sequence with the old temperature. Even when the hardware can individually read each field, the two-field record is not automatically a consistent transaction. At the language and platform level, unsupported concurrent accesses may also violate the required memory model.

For a first implementation, put the record and an explicitly process-shared mutex in shared storage. Both processes lock before reading or changing the record. A process-shared condition variable can coordinate waiting if supported and initialized correctly. A default process-local `std::mutex` is not a substitute.

For pthread synchronization, attributes such as `PTHREAD_PROCESS_SHARED` must be configured and every initialization result checked. Its storage must actually be in the shared mapping. Setting an attribute on a local object does not make another process share that object.

The writer holds the mutex while installing both fields. The reader holds it while copying both fields into its own local record, then unlocks before slow processing. The local copy lets the reader do work without blocking future publication for the entire processing time.

### 13.4 Follow a buffer through ownership states

For large data, a producer and consumer can use explicit slot states. Here is a conceptual state machine; the transitions themselves still need a supported synchronization protocol:

```text
FREE -> WRITING -> READY -> READING -> FREE
```

**FREE:** no participant uses the slot's payload. The producer may claim it.

**WRITING:** the producer owns the payload and may modify it. The consumer must not read it yet.

**READY:** the full record is published. The producer must not overwrite it while waiting for consumption.

**READING:** the consumer owns access to the published payload. The producer waits for release or uses another free slot.

After consumption the slot becomes FREE again. With several slots, the participants can overlap work on different slots. With one slot, they alternate ownership. This is the same producer/consumer idea as section 9, now across process boundaries.

The notification says "check for ready work"; the protected slot state says which work actually exists. A notification must not be treated as a substitute for correct publication.

### 13.5 Validate offsets without creating an overflow

Suppose the mapped region has 4096 bytes and a record says its payload starts at offset 4000 with length 200. The payload would extend beyond the mapping, so reject it before deriving a pointer or reading data.

A safe conceptual bounds test is: first require `offset <= region_size`, then require `length <= region_size - offset`. Adding `offset + length` first can overflow an unsigned integer and appear small again. Check alignment and representation requirements too before interpreting bytes as a typed object.

Validate metadata even when the peer is local. Another process can be buggy, stale, built with a different version, or unauthorized. A name and a mapping are not proof of a compatible layout.

### 13.6 What happens when the owner dies

If a process dies while holding a non-robust shared mutex, another process may be unable to obtain it. Supported robust-mutex mechanisms can report owner death, but they do not repair half-written application data. Recovery needs to establish a valid invariant before declaring the protected state consistent.

For an introductory design, use external coordination to stop old participants and create a new region with a new generation. A generation distinguishes one lifetime from its replacement; a sequence distinguishes records within that lifetime. Do not reinitialize synchronization in place while another process might still use it.

**Questions with answers:**

1. **Does mapping the same name guarantee both processes are ready?** No. Creation, sizing, initialization, and publication are separate steps.
2. **Can a shared vector contain ordinary heap pointers?** Not as a portable cross-process representation. The allocation and pointers usually belong to one address space; use a designed shared layout with offsets or suitable shared-memory facilities.
3. **Is shared memory always better than messages?** No. For small commands, avoiding copies may save little while introducing ownership and recovery complexity. Choose it when the data size and measured workload justify those costs.

### 13.7 Shared bytes need a defined representation and lifetime

A region being large enough does not make every C++ type suitable for placement there. Virtual objects carry implementation details; standard containers often point to process-local allocations; many synchronization objects are process-local by default. Layout, alignment, initialization, and the platform's object-lifetime rules all matter.

Begin with a deliberately defined shared representation: fixed-width fields, bounded arrays, validated offsets, and synchronization explicitly supported for interprocess use. Use consistent compiler/ABI assumptions where the representation depends on them, and reject incompatible versions before interpreting later fields.

Do not assume that placing `std::atomic` in mapped bytes automatically supplies a portable interprocess contract for every implementation. Lock-freedom, alignment, initialization, and platform support must be established. Similarly, a sequence-counter design that retries after reading conflicting ordinary fields does not automatically avoid undefined behavior in C++; detecting a concurrent write afterward does not retroactively make an invalid access legal.

This is why the initial shared-memory design uses supported process-shared synchronization instead of presenting a few atomic fields as a universal solution.

### 13.8 Publication is only half of zero-copy ownership

Suppose a writer publishes a shared buffer and sends its offset to a reader. The reader sees the correct completed bytes. Can the writer reuse that buffer immediately? No. Publication establishes that data is available; it does not establish that every reader has finished.

Reclamation requires a separate rule: a release message, a protected reader count, a generation-tagged pool, or another proven ownership mechanism. With multiple readers, the slowest authorized reader can hold capacity. A crashed reader can hold it forever unless recovery resolves that ownership.

For a bounded telemetry service, a copy into a reader's private buffer may simplify this problem enough to be the better design. Zero-copy reduces a data-transfer cost but can increase coordination cost and make worst-case buffer retention harder to bound.

**Worked trade-off:** copying a small 64-byte status record avoids sharing its lifetime. Sharing a multi-megabyte image may save meaningful bandwidth but requires a bounded pool and explicit release protocol. Choose from measured data size and rate, not from the phrase "zero-copy is faster."

### 13.9 Restart-safe naming and generation checks

Imagine `/sample_region` originally names generation 8. The owner exits, and a replacement creates generation 9 under the same name after coordinated removal. An old process may still have a mapping of generation 8 even though a fresh open finds generation 9.

Therefore name equality does not establish shared instance identity. The control protocol and mapped metadata should agree on generation, version, and size. Participants must stop using old mappings according to a defined handover or recovery procedure.

Do not let two recovering processes both decide they are the new owner and initialize competing state without coordination. Ownership election or a supervisor must serialize recreation. A generation value detects stale state; it does not itself decide who is authorized to create the new state.

## 14. Resource Managers

A resource manager is a process that presents a service through the QNX pathname space and handles connection and I/O messages.

For a simulated temperature device:

```text
application open/read/write/devctl
                 |
          /dev/learn_sensor
                 |
   resource-manager dispatch and handlers
                 |
    simulated sensor state or real driver
```

The path is an interface, not necessarily a disk file. It lets existing tools and applications use familiar APIs while your process implements the behavior.

### Construction steps

Read the official resource manager guide and start from its minimal example for your release:

1. Create the dispatch context with `dispatch_create()`.
2. Initialize connect and I/O function tables with `iofunc_func_init()`.
3. Initialize resource attributes with `iofunc_attr_init()`.
4. Override only the required handlers, such as read and write.
5. Attach the pathname through `resmgr_attach()` with appropriate permissions and type.
6. Allocate a dispatch context and execute the block/handler loop.
7. Add controlled shutdown, detach, and context cleanup.

API return conventions differ: verify each function's documented failure return. The framework reduces boilerplate but does not define your device's data semantics.

### Correct read behavior

Suppose a read exposes a snapshot `temperature_mC=25000\n`:

- Validate the operation and access mode using the framework helpers.
- Respect the client's requested byte count.
- Track position in the open control block (OCB), not one global offset.
- Return the remaining bytes for that open instance.
- Return zero bytes at EOF for a snapshot-style interface.
- Define whether seeking resets the snapshot or is rejected.

Without correct EOF behavior, a tool reading until EOF can run forever. Without per-open state, one client's read can change another client's result.

### Correct write and control behavior

Bound the input, handle message data that requires additional reads, validate ranges, and commit state only after the entire request is valid. Do not assume a write buffer is null-terminated.

Use `devctl()` for well-defined device-specific controls when ordinary read/write semantics are awkward. Define command identifiers, data sizes, direction, versioning, and permissions. Do not expose an arbitrary privileged command interface.

**Practice:** E13-E14. Implement a simulated device before touching hardware.

### 14.1 Why a device can look like a file

An application opening `/dev/learn_sensor` is asking the OS to resolve a pathname and establish access to the resource behind it. That resource need not be bytes stored on a disk. A resource manager is a service process that supplies the meaning of operations on the resource.

For our simulated sensor, `read()` could return a text snapshot of temperature. `write()` could request a sampling-period change. A device-control operation could request a structured status response. Those behaviors are decisions made by the service author, not automatic consequences of naming something under `/dev`.

This approach lets applications use familiar interfaces while keeping device-specific logic behind the service boundary. The file descriptor is the client's handle to an opened resource. It is not the temperature itself and not a direct hardware address.

### 14.2 Follow open, read, and close

During `open()`, the pathname is resolved and the resource manager's connection handling establishes an open instance with the permitted access mode. An **open control block**, or OCB, holds state associated with that open description.

Later, a `read()` request is dispatched to the read handler. The handler checks the request and access mode, decides which bytes belong to this operation, and reports the result. The framework helps carry that response back to the caller.

At close, open-instance resources can be released when no longer referenced. A descriptor duplicated from the same open description can share its state; two independent `open()` calls normally need independent cursor state. "Every descriptor is a separate snapshot" is therefore not a universally correct rule.

The dispatch framework handles routing and conventions. It cannot invent your device's snapshot, streaming, blocking, or configuration semantics. Those must be deliberately implemented.

### 14.3 Snapshot versus stream

A **snapshot** is a finite view captured at a defined moment. For example, an open instance might capture `temperature_mC=25000\n` and return that same snapshot over several short reads. Eventually there are no bytes remaining and a positive-length read returns zero, indicating EOF.

A **stream** represents continuing data. Temporarily having no new sample may mean block, return a nonblocking indication, or follow another documented behavior. It should not accidentally look like permanent EOF if the interface promises future records.

Choose snapshot semantics for this first sensor. If the physical temperature changes while the client reads one byte at a time, the client's snapshot should not splice the beginning of one value with the end of another. Capture a stable representation per open, or define an equally clear alternative.

### 14.4 Calculate partial reads

Use the tiny snapshot `ABC\n`, which contains four bytes. A client asks for two bytes at a time:

```text
offset 0: request 2 -> return "AB"  -> offset becomes 2
offset 2: request 2 -> return "C\n" -> offset becomes 4
offset 4: request 2 -> return 0 bytes, EOF
```

The transferable count is the smaller of the requested count and the remaining count. Advance the cursor by the count actually transferred, not by the original request size.

If another independent client opens the same sensor, its cursor starts at zero. A global cursor would make the second client begin halfway through somebody else's read. This is why open-instance state matters.

A zero-length read returns zero without proving the stream reached EOF; it requested no bytes. When testing EOF, use a positive requested length.

### 14.5 Validate a configuration change before installing it

Suppose the write protocol is an ASCII integer from 10 to 1000 representing milliseconds, with an optional final newline. Define a maximum input length before parsing. Input such as `25oops`, an empty payload, a negative value, or `1001` must not silently change the period.

Parse into a temporary value. Check that the entire permitted input was consumed and that the value is in range. Only then replace the active configuration under its synchronization. This separation gives rejected writes a useful guarantee: the previous configuration remains unchanged.

For concurrent handlers, updating configuration also needs coordination with readers and the sampling worker. A correct parser does not make a multi-field configuration update atomic. Define whether an accepted change applies immediately, at the next release, or after a separate acknowledgment.

### 14.6 Complete example: the behavior behind the handlers

**Linux C++ model:** use the lab filename `sensor_semantics.cpp`. It does not register a QNX pathname or implement dispatch. It demonstrates the actual snapshot and validation logic that a native handler must preserve.

```cpp
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

class SnapshotReader {
public:
    explicit SnapshotReader(std::string snapshot)
        : snapshot_(std::move(snapshot)) {}

    std::string read(std::size_t requested) {
        const auto remaining = snapshot_.size() - offset_;
        const auto count = std::min(requested, remaining);
        auto result = snapshot_.substr(offset_, count);
        offset_ += count;
        return result;
    }

private:
    std::string snapshot_;
    std::size_t offset_ = 0;
};

bool set_period(std::string_view input, int& current_period) {
    if (input.empty() || input.size() > 5) {
        return false;
    }
    if (input.back() == '\n') {
        input.remove_suffix(1);
    }
    if (input.empty()) {
        return false;
    }
    int candidate = 0;
    const auto parsed = std::from_chars(
        input.data(), input.data() + input.size(), candidate);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != input.data() + input.size() ||
        candidate < 10 || candidate > 1000) {
        return false;
    }
    current_period = candidate;
    return true;
}

int main() {
    const std::string text = "temperature_mC=25000\n";
    SnapshotReader first(text);
    SnapshotReader second(text);
    bool valid = first.read(0).empty();
    std::string reconstructed;
    for (;;) {
        const auto chunk = first.read(1);
        if (chunk.empty()) {
            break;
        }
        reconstructed += chunk;
    }
    valid = valid && reconstructed == text && first.read(1).empty();
    valid = valid && second.read(5) == "tempe";
    valid = valid && second.read(100) == text.substr(5);
    valid = valid && second.read(1).empty();

    int period = 100;
    valid = valid && set_period("25\n", period) && period == 25;
    for (const auto invalid : {"", "\n", "9", "1001", "-20", "25x", "123456"}) {
        const bool rejected = !set_period(invalid, period);
        valid = valid && rejected && period == 25;
    }
    valid = valid && set_period("10", period) && period == 10;
    valid = valid && set_period("1000\n", period) && period == 1000;
    std::cout << (valid ? "sensor semantics: PASS\n" : "sensor semantics: FAIL\n");
    return valid ? 0 : 1;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror sensor_semantics.cpp -o sensor_semantics
./sensor_semantics
```

Expected output: `sensor semantics: PASS`.

Each `SnapshotReader` owns its text and cursor. `substr()` produces only the requested available bytes; it does not advance any other reader. This teaching model allocates strings and is not an allocation-free real-time implementation.

`string_view` lets the parser inspect a bounded sequence without requiring a terminating null byte. `from_chars()` parses within that supplied range and reports where it stopped. Checking the stopping position rejects a valid number followed by unwanted characters. Updating `current_period` happens only after all checks pass.

The model is single-threaded. A native service must additionally check access modes, retrieve the whole bounded message, follow framework reply conventions, synchronize shared configuration, and clean up open instances. The Linux result validates these model semantics, not those QNX integration steps.

### 14.7 How the native framework connects to this behavior

`dispatch_create()` establishes dispatch infrastructure. The connection and I/O tables select functions for operations such as open and read. `iofunc_func_init()` supplies conventional defaults, while `iofunc_attr_init()` establishes attributes such as type and permissions. Your overrides supply device-specific behavior.

`resmgr_attach()` publishes the pathname with the intended resource attributes. The dispatch loop waits for requests and routes them to handlers. The handler uses the open-instance state rather than creating a fresh cursor on every read. Otherwise every call would return the beginning of the snapshot forever.

For a read handler, think in this order: framework verification, supported request form, per-open snapshot and offset, bounded response length, response preparation, and the documented completion convention. Do not mix a framework-managed reply with an extra manual reply to the same transaction.

`devctl()` is useful when a byte stream is awkward for a control operation. A command identifies a defined operation, with an agreed input/output structure, size, direction, version, and permissions. It is not an invitation to expose arbitrary internal pointers or privileged actions.

**Question:** why might `cat /dev/learn_sensor` print forever? **Answer:** for a snapshot device, inspect whether every read starts at offset zero or whether the handler fails to return EOF. For an intentionally streaming device, continued reading may be the designed behavior. Diagnose against the interface contract.

### 14.8 Choose which state is global and which is per open

For the sensor resource, the current sampling configuration is shared device state. A captured text snapshot and its cursor belong to an open description. Peer-specific authorization or subscriptions may require additional session state. Mixing these scopes causes subtle behavior bugs.

If the cursor is global, independent opens interfere. If the configuration is copied per open without a defined update rule, one client may appear to change the device while another reads stale configuration. If permissions are checked only when a pathname is published, later per-operation authorization requirements may be missed.

A useful decomposition is: device attributes describe the resource, open state describes a particular access lifetime, and worker state describes ongoing device or service work. Each has an owner, synchronization rule, and cleanup trigger.

### 14.9 More handler threads require coherent snapshots

A dispatch thread pool permits concurrent requests. It also means that a status read can overlap a configuration write. Suppose status reports both `period_ms` and `configuration_version`. A reader must not combine the old period with the new version and present that as one coherent state.

One simple approach takes the state mutex briefly, copies all required fields into a local snapshot, releases the mutex, and formats the response afterward. This keeps formatting or other potentially slow work out of the protected region while preserving the relationship among fields.

If an operation must wait for hardware, separate request validation from the long wait and define ownership of the outstanding request. Holding a broad device-state mutex during the wait may block unrelated reads and shutdown. Releasing it introduces a need to detect configuration changes or cancellation before committing a result.

Concurrency increases the number of allowed interactions. It should be introduced with explicit per-operation state transitions, not merely a larger thread count.

### 14.10 File-like operations still need transactional semantics

A write returning success should have a defined meaning: command parsed, configuration accepted for later application, or configuration already applied. If application is deferred, a query should distinguish requested state from active state, for example using a configuration version and activation status.

Consider two clients updating a period based on a previously read value. Even fully synchronized writes can overwrite one another's intent. If conditional updates matter, accept an expected version and reject a stale update rather than silently applying it to a newer configuration. This is an application-level concurrency contract, not a property automatically supplied by `write()`.

State what partial writes mean for your command interface. A stream parser that accumulates bytes across writes needs per-open accumulation limits and cancellation behavior. A one-command-per-write interface can reject incomplete commands, but must document that requirement. The Linux parser model implements validation of one complete bounded input, not arbitrary stream accumulation.

### 14.11 Shutdown must account for open clients

Removing a published pathname prevents or changes future discovery, but it does not alone prove that every existing request or open instance has ended. Stop new work, resolve outstanding operations, wake supported waiters, and release per-open resources through the framework's documented lifecycle.

If clients can hold opens indefinitely, define the service's shutdown policy rather than waiting forever for voluntary close. The exact detachment and cancellation APIs depend on the framework and release. The architectural requirement is stable: no handler may continue using resource state after its destruction.

## 15. Memory and C++ Discipline

Real-time C++ is primarily about controlling lifetime, cost, and blocking behavior.

| Prefer | Reason |
|---|---|
| RAII wrappers for descriptors, mappings, channels, and connections | Cleanup on every exit path |
| Fixed-capacity storage where the workload is bounded | Predictable capacity and explicit overload behavior |
| Initialization before time-critical execution | Moves allocation and setup outside the critical path |
| Small, documented critical sections | Reduces blocking and improves analysis |
| Checked sizes and numeric conversions | Rejects malformed IPC and avoids overflow |
| Explicit exception boundaries | Prevents unhandled failures crossing thread or C API boundaries |

Avoid claiming that `reserve()` makes a container permanently allocation-free: exceeding capacity can still allocate. Avoid deleting or performing the final shared-pointer release on a critical thread if destruction can be expensive.

Study stack limits, mapping behavior, lazy memory effects, and the exact memory-locking facilities of your target. Permission to lock memory and a successful lock do not prove the whole application has bounded execution time. Do not import Linux swap assumptions without checking QNX behavior.

Use Linux sanitizers for portable code where the host toolchain supports them. Do not assume AddressSanitizer or ThreadSanitizer has identical availability on your QNX release and architecture. Clean Linux tests are useful evidence, not proof of target correctness.

**Practice:** E15. Count allocations and inspect error paths before discussing lock-free replacements.

### 15.1 Storage location does not decide object lifetime

An automatic local object usually lives until its scope ends. A dynamically allocated object lives until its owner destroys it. These are lifetime rules; they are not the same question as whether the storage is on a stack or obtained from a heap allocator.

Consider a function that creates a local queue and starts a worker with a reference to it. If the function returns while the worker still runs, the worker's reference becomes invalid. Moving the queue to the heap only changes the storage source. If someone deletes it before the worker finishes, the same lifetime bug remains.

The correct fix is ownership ordering: the queue must outlive every operation using it. Join workers before destroying the queue. Shared ownership can sometimes express this lifetime, but it also introduces reference-counting and potentially expensive last-owner destruction. Choose ownership deliberately rather than assuming a smart pointer fixes the whole concurrency design.

### 15.2 RAII turns cleanup into an ownership rule

RAII means an object's lifetime manages a resource. A lock guard owns a held lock, a unique pointer owns an allocation, and a file wrapper can own a descriptor. When the owner leaves scope, its destructor releases the resource.

The benefit becomes clear on an error path. Without an owner, every return and exception branch must remember to close the file or unlock the mutex. With an appropriate owner, cleanup follows the same lifetime rule regardless of which branch exits.

However, RAII does not know whether another thread still uses the resource. A descriptor wrapper that closes while a worker is writing is not a correct service design. RAII answers "who releases it?" Your lifecycle answers "when is release allowed?"

Destructors should not let exceptions escape during stack unwinding. For failures that must be reported, an explicit shutdown or flush operation can report status before final best-effort cleanup. An unsuccessful flush is a meaningful data-loss risk, not something a destructor can magically repair.

### 15.3 Why allocation can affect timing

Dynamic allocation may involve allocator metadata, contention, finding available space, and obtaining or touching memory pages. Its cost can depend on earlier allocations and the state of the process. Deallocation and destruction can also be expensive.

For a sensor that must perform small regular work, create buffers before entering the time-sensitive loop. Use a defined maximum record count and a clear full-buffer policy. A fixed-capacity array makes the storage bound visible; it does not prove every operation using it meets a deadline.

`vector.reserve(100)` reserves capacity for at least 100 elements at that moment. Adding more can allocate again. A vector of strings may also allocate inside individual strings. Looking only at the outer container misses nested costs.

Page faults and first-use effects can make early iterations different from warmed-up execution. Understand the target's mapping and memory-locking facilities before making timing claims. Do not assume Linux swapping behavior describes QNX.

### 15.4 Atomic does not mean the whole operation is correct

An atomic increment provides an indivisible update to that atomic object. It can solve a shared-count operation that otherwise races. It does not automatically make a separate array update, capacity check, and index increment a correct queue transaction.

Memory ordering governs relationships between operations. A mutex supplies a straightforward synchronization relationship between unlock and a later successful lock on the same mutex. Proper thread joining orders completed worker actions before the joining thread's subsequent actions. That is why the examples can read their result counters after joining.

With atomics, acquire/release operations can publish associated data only when the complete protocol establishes the required relationship. A relaxed atomic counter does not by itself publish unrelated ordinary data. Begin with the mutex-based queue until you can explain every ownership transfer; replacing locks with atomics is an algorithm change, not a spelling change.

"Lock-free" also does not mean every individual operation has a fixed small completion time. A system-wide progress property is different from a per-thread bound, and both are different from a measured application deadline.

### 15.5 Trace an error through initialization

Suppose a service opens storage, allocates a queue, starts a worker, and then fails to create its control endpoint. Work backward through the resources actually acquired:

```text
endpoint creation failed
    -> request worker stop and wake it
    -> join worker
    -> release queue
    -> close storage
    -> report startup failure
```

Closing storage first would be wrong if the worker can still write. Destroying the queue first would be wrong if the worker can still pop. Ignoring the failed endpoint would leave a partially operational service without its promised control interface.

**Question:** should a read of a counter after joining every writer still take its mutex? **Answer:** not for synchronization with those completed writers. The join establishes the necessary ordering. If any other participant can still write, that conclusion no longer applies.

### 15.6 A memory budget includes more than payload arrays

Suppose there are eight workers with a configured stack allowance of 256 KiB each. That is 2 MiB of stack allowance before accounting for queues, code, libraries, allocator overhead, mappings, and kernel-managed resources. Reserved address space and committed physical memory are different quantities, so measure and document which budget you mean.

For a fixed pool, count capacity times slot size, including alignment and metadata. Add buffers held in transit: a record may simultaneously exist in the acquisition buffer, queue, client's outgoing storage, server receive storage, and logger's pending work. Calling the central queue bounded does not bound every duplicate copy elsewhere.

Stack demand depends on call depth, local arrays, library calls, recursion, and interrupt/thread model. An apparently small worker can call a library with substantial stack use. Choose limits using the actual toolchain and target evidence, and treat overflow as a real failure mode rather than assuming "the stack is automatic."

### 15.7 Exception safety and service consistency

An operation can provide different guarantees on failure. A basic guarantee preserves valid state and prevents resource leaks. A stronger rollback-style guarantee leaves externally visible state unchanged. A nonthrowing guarantee says an operation will not emit an exception; it does not mean the operation always succeeds semantically.

The period parser follows a useful pattern: parse and validate temporary state, then perform a small commit. If validation fails, the active period is unchanged. Extending this to a configuration that allocates buffers requires finishing fallible preparation before publishing the new configuration where possible.

Across process or device boundaries, local exception rollback cannot undo arbitrary external effects. If a storage call partly wrote a record, destroying local temporaries does not erase those bytes. Error handling needs both C++ resource cleanup and an application recovery contract.

### 15.8 Deallocation can be on the critical path

Moving allocation out of a time-sensitive loop is insufficient if the loop still destroys large objects or performs final shared-owner releases. Destruction can free nested allocations, take locks inside libraries, or trigger other cleanup.

A fixed object pool can make ownership transitions explicit: initialize slots, claim a free slot, populate it, transfer it, and return it to the pool after all use ends. Exhaustion must have a defined behavior; an exhausted pool that falls back to unrestricted heap allocation no longer has the original bound.

Deferred cleanup can move cost to another thread, but the deferred-work queue must also be bounded. Otherwise the optimization merely relocates unbounded memory growth.

### 15.9 Optimize the measured cost without weakening the contract

Suppose profiling shows expensive formatting dominates logger CPU use. Replacing a queue mutex with an intricate lock-free design may not materially improve latency and can make shutdown and reclamation harder to verify. Batch formatting, a simpler record format, or moving output work may address the actual cost.

Conversely, a measured long lock hold across formatting identifies a concrete improvement: copy the protected data, unlock, then format the local copy. The correctness argument is that the snapshot remains coherent; the performance argument is that unrelated threads no longer wait for formatting while owning the shared-state lock.

An architectural optimization should state the original bottleneck, the contract preserved, the new costs introduced, and the measured result. "Fewer locks" or "more threads" alone is not that argument.

## 16. Boot, BSPs, and Drivers

### A simplified boot sequence

```text
board firmware / bootloader
          -> load QNX image
          -> board-specific startup code and system-page setup
          -> procnto
          -> boot script starts required drivers and services
          -> application services become ready
```

Exact details are board- and release-specific. The boot image includes the components selected by its buildfile. `mkifs` builds image filesystems; a BSP supplies board-specific support and examples.

Learn these separately:

- **Startup:** CPU/board initialization and information needed by the OS.
- **Boot image:** the files and script needed to reach an operational system.
- **Drivers/services:** storage, networking, serial, and other capabilities.
- **Application readiness:** dependencies initialized and the actual service usable.

A process existing is not the same as being ready. Replace arbitrary startup delays with bounded readiness checks and explicit failure reporting.

### Driver and interrupt mindset

Start with a simulated resource manager, then study the BSP's recommended driver framework. Keep work in interrupt context minimal; follow the documented interrupt attachment model and defer processing to an appropriate thread when required. Some models support interrupt service threads directly.

Memory-mapped registers, DMA buffers, cache coherency, interrupt ownership, and abilities are hardware-specific. `volatile` may be part of register access, but it does not replace required device barriers or CPU memory-ordering rules.

Never test arbitrary physical addresses, interrupt numbers, or device register writes on a board. Use the hardware manual, BSP documentation, and reviewed driver examples.

**Practice:** E16. Analyze a known-good image before modifying one.

### 16.1 What happens before main can run

Your C++ application assumes the processor can execute code, memory is usable, and OS services exist. Immediately after reset, that environment has not necessarily been established. Booting builds the environment in stages.

Board firmware or a bootloader performs its platform-defined preparation and loads the QNX image. QNX startup code then performs the board-specific initialization needed by the OS and constructs system information, including the system page. `procnto` brings up the microkernel and process-manager environment. Startup scripts launch the selected drivers and services, followed by applications.

This is why an executable alone is not a complete bootable system. The program might be correct but depend on a storage driver, network service, runtime library, or configuration file absent from the image.

### 16.2 What a BSP actually supplies

A **Board Support Package** connects the OS to a particular hardware platform. Boards with the same instruction set can have different interrupt controllers, memory maps, clocks, storage devices, and peripheral connections. A compiler producing AArch64 instructions does not tell the OS where a particular board's UART registers are.

A BSP typically supplies the board-specific startup support, configuration, drivers or driver integration, and example buildfiles needed for the supported board. Its supported revision and release matter. A nearby board model is not sufficient evidence of compatibility.

Think of the division this way: the application says "read a sensor"; a service defines the read interface; a driver understands the device; the BSP and hardware description establish the platform on which those components operate.

### 16.3 Image contents and startup order are different

An image buildfile describes selected files and startup behavior. `mkifs` creates an image filesystem from that description. Including an executable makes its bytes available; it does not necessarily start the executable or make its dependencies ready.

For a logger storing records on persistent media, a dependency chain could be:

```text
storage hardware support -> filesystem mounted -> log directory usable
                         -> logger opens destination -> logger reports ready
                         -> sensor begins sending records
```

If a service starts too early, the failure may be a missing dependency rather than a bug in its main algorithm. Sleeping for an arbitrary number of seconds is an unreliable dependency check: a slow boot might take longer, and a failed dependency might never become usable.

Readiness should mean something testable, such as "the logger can accept a test request and its output destination is open." Give the readiness check a timeout and an explicit failure action. A PID merely proves a process exists at the observation time.

### 16.4 What a driver does

A driver translates a controlled software request into device-specific operations and reports their outcomes. It must understand the hardware manual: register meanings, legal operation order, completion conditions, errors, and reset behavior.

A **memory-mapped register** appears at a mapped address, but accessing it is not necessarily like reading ordinary RAM. A read may acknowledge a status condition; a write may start hardware activity. Access width and ordering can matter. This is why a guessed address or a generic pointer cast is not an acceptable driver experiment.

User-space placement changes the protection and communication structure, not the need for correct hardware behavior. A process with access to a device that can perform DMA may affect memory beyond its ordinary software writes.

### 16.5 Interrupts announce work

Polling repeatedly asks, "has the device finished?" An interrupt allows the device to announce a condition instead. The OS routes that event through the platform's interrupt mechanism to the appropriate handling context.

The immediate handling context has restrictions and should perform only the work allowed by its attachment model. A common design acknowledges or records the event and lets an appropriate thread perform longer processing. QNX supports release-specific interrupt attachment and interrupt-service-thread models; do not transplant an old ISR signature into a different model.

The device manual determines when to clear, mask, acknowledge, and re-enable a hardware interrupt. Clearing too early can lose useful status; failing to clear a level-triggered condition can cause repeated interrupt activity. Those details cannot be inferred from the word "interrupt."

**Example:** a receive device announces that a buffer has data. The handler establishes which buffer completed and transfers ownership to processing. The processing thread parses the bytes and eventually returns the buffer for reuse. Notification and buffer ownership remain separate concerns.

### 16.6 DMA and cache visibility

With **Direct Memory Access**, a device transfers data to or from memory without the CPU copying every byte through ordinary load/store instructions. The CPU still configures the operation and coordinates completion.

Suppose a device fills a receive buffer. Software must not read it as a complete sample until device completion is established. After the CPU begins reading, the device must not overwrite the same buffer until the ownership protocol permits reuse.

CPU caches add another question: does the CPU's current view reflect the device's writes? Coherent systems and non-coherent systems handle this differently. Platform-specific DMA APIs, cache maintenance, alignment, and memory barriers may be required. A C++ atomic flag alone does not perform every device cache-maintenance operation.

`volatile` can be relevant to compiler treatment of hardware accesses, but it is not a universal device-ordering, synchronization, or cache-coherency solution.

**Question:** why can a sensor application run in a VM while its real driver cannot simply be copied there? **Answer:** the VM exposes a particular virtual hardware model. The driver must match that model or access an explicitly supported passed-through device; the CPU instruction set alone is insufficient.

### 16.7 Boot dependencies form a graph, not only a script

A script is sequential text, but the real dependencies can branch. Console access may be needed for recovery while storage and networking initialize independently. An application might require storage but not networking, while diagnostics require networking but should remain available if storage fails.

Represent readiness dependencies as a directed graph. An edge from storage to logger means logger readiness depends on usable storage, not merely on launching its executable. A cycle such as logger waiting for a supervisor that waits for logger readiness reveals a startup protocol problem.

Start independent branches concurrently only when their resource interactions permit it. Parallel boot can reduce elapsed startup time but increases the importance of precise readiness signals and failure reporting. "Wait 500 ms" does not encode a dependency and cannot distinguish slow initialization from permanent failure.

### 16.8 Driver recovery requires ownership quiescence

**Quiescence** means relevant activity has stopped so state can be safely changed or released. Before freeing a DMA buffer, establish that the device cannot still write it and no software handler can still access it. Stopping the application producer does not necessarily stop the device or an already queued completion.

A conceptual shutdown sequence is: prevent new submissions, stop or complete device work according to hardware rules, synchronize with in-flight handling, release buffer ownership, then remove mappings and device resources. The exact register and interrupt ordering comes from the driver framework and hardware manual.

A timeout while waiting for hardware does not prove the hardware has stopped. Resetting or power-cycling may be part of recovery, but that operation has its own effects on shared devices and other clients. It must be designed, not appended as an arbitrary error-path write.

### 16.9 A bootable update is not automatically a recoverable update

An image update can fail after some bytes are written or after an incompatible image is selected. A robust deployment design preserves a recovery path and decides when an image becomes the active trusted version.

Common architectural ideas include validating an image before activation, retaining a known-good slot, requiring a bounded health confirmation after boot, and falling back when confirmation fails. Actual implementation depends on the boot chain, storage layout, platform security requirements, and product update system.

Integrity and authenticity answer different questions from availability. An authentic image can still have a functional defect; a working image can still be unauthorized. Rollback also needs a security policy, because permitting arbitrary old vulnerable images may be unacceptable.

**Architect checkpoint:** describe what happens after power loss during each update stage. If the only recovery instruction assumes the new OS already boots normally, the recovery argument is incomplete.

## 17. Filesystems and Networking

### Filesystems

Your image may have read-only, RAM-backed, and persistent storage with different guarantees. Discover what is mounted and what is writable before choosing configuration and log paths.

Define log rotation, space limits, error handling, and shutdown flushing. A successful `write()` does not necessarily mean data has reached persistent media. Requirements involving power loss need filesystem- and device-specific durability testing.

### Networking

POSIX sockets carry over, but target services, libraries, and stack versions matter. QNX 7.x material often uses `io-pkt`; QNX 8.0 documentation centers on `io-sock`. Verify your actual image and use its networking guide. Socket programs may require `-lsocket`; follow the release-specific API library requirements.

TCP is a byte stream: one `send()` is not necessarily one `recv()`. Implement framing, partial I/O, orderly close, timeouts, and reconnect behavior. UDP preserves datagram boundaries but does not guarantee delivery or ordering.

For remote command or telemetry interfaces, define authentication, encryption where required, size limits, rate limits, and a safe offline state. Neither native IPC nor ordinary TCP automatically provides application authorization.

Related reading already in this repository:

- [Learning/ID_Buzz/TCP_protocal.md](ID_Buzz/TCP_protocal.md)
- [Learning/ID_Buzz/udp_protocal.md](ID_Buzz/udp_protocal.md)

**Practice:** E17. A loopback success test is only the beginning.

### 17.1 Writing, flushing, and surviving power loss

When an application writes a log record, several different milestones can occur. The bytes may enter a user-space buffer, be accepted by an OS service, reach a device cache, or reach storage with a specified persistence guarantee. These are not interchangeable.

An iostream flush pushes buffered output through its next layer; it does not by itself establish power-loss durability. A successful `write()` reports bytes accepted according to that interface, not a universal guarantee about every lower layer. Facilities such as `fsync()` address a stronger synchronization request, but the actual filesystem, device, error handling, and failure model still matter.

Therefore a reply called "logged" needs a precise meaning. "Accepted into the logger's RAM queue" is fast but can lose data if the logger dies. "Durably committed under the specified storage failure model" requires a different implementation and cost. Pick the milestone your system actually requires.

### 17.2 Partial I/O is normal

A write requesting 100 bytes can report that only 40 were transferred. The application must track that progress and, when appropriate, attempt the remaining 60 starting at the correct offset. Repeating all 100 would duplicate the prefix.

A read can also return fewer bytes than requested. For stream sockets, zero indicates an orderly end of the peer's sending stream, not an empty message. A negative return indicates an error under the non-`_r` POSIX-style convention; inspect the relevant error and decide whether it means retry, wait for readiness, cancellation, or failure.

Nonblocking `EAGAIN`/`EWOULDBLOCK` means the operation cannot proceed immediately; repeatedly retrying without a readiness wait creates busy polling. `EINTR` indicates interruption, but retrying must respect the stop policy and the original overall deadline. Error handling is part of the I/O protocol, not an optional final print statement.

### 17.3 TCP preserves bytes, not application messages

Suppose the sender makes two calls, sending `HELLO` and then `WORLD`. The receiver might get `HEL`, then `LOWOR`, then `LD`, or all ten bytes together. TCP preserves the byte-stream order, but the receiver's read boundaries need not match the sender's calls or network packet boundaries.

A receiver therefore needs **framing**, a rule for determining where one application record ends. Common choices are fixed-size records, delimiters with escaping, or a length prefix. Use a bounded length prefix for this lesson.

Define a frame as a four-byte unsigned payload length in big-endian order, followed by exactly that many payload bytes. Big-endian means the most significant byte appears first. A length of three is represented by `00 00 00 03`. Reject zero length for this particular protocol and cap payloads at 4096 bytes.

The receiver first accumulates all four length bytes, then validates the decoded length, then accumulates that many payload bytes. Only a complete validated payload is delivered to the application. If the connection ends halfway through either part, do not process it as a complete record.

### 17.4 Follow a fragmented frame

The intended bytes are `00 00 00 03 A B C`. The first receive returns only `00 00`: keep them and wait for the rest of the header. The second returns `00 03 A`: finish decoding the length, validate three, and retain the first payload byte. The third returns `B C`: deliver the complete payload `ABC` once.

Now suppose the third receive also contains the next frame. Continue parsing those bytes as a new header; do not discard them because the previous frame completed. This requires persistent parser state across reads and a loop capable of consuming multiple frames from one read.

### 17.5 Complete example: a bounded stream decoder

**Linux C++ example:** use the lab filename `frame_decoder.cpp`. This tests framing independently of sockets, so the meaning of arbitrary receive boundaries is visible without networking setup.

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>

class FrameDecoder {
public:
    template <typename Handler>
    bool feed(std::string_view bytes, Handler&& on_frame) {
        if (failed_) {
            return false;
        }
        for (const char byte : bytes) {
            if (header_bytes_ < 4) {
                length_ = (length_ << 8) | static_cast<unsigned char>(byte);
                ++header_bytes_;
                if (header_bytes_ == 4 &&
                    (length_ == 0 || length_ > payload_.size())) {
                    failed_ = true;
                    return false;
                }
            } else {
                payload_[received_++] = byte;
                if (received_ == length_) {
                    on_frame(std::string_view(payload_.data(), received_));
                    header_bytes_ = 0;
                    length_ = 0;
                    received_ = 0;
                }
            }
        }
        return true;
    }

    bool complete() const {
        return !failed_ && header_bytes_ == 0;
    }

private:
    std::array<char, 4096> payload_{};
    std::uint32_t length_ = 0;
    std::size_t header_bytes_ = 0;
    std::size_t received_ = 0;
    bool failed_ = false;
};

int main() {
    using namespace std::literals::string_view_literals;
    FrameDecoder decoder;
    std::size_t frames = 0;
    bool contents_match = true;
    auto receive_frame = [&](std::string_view frame) {
        const auto expected = frames == 0 ? "ABC"sv : "DE"sv;
        contents_match = contents_match && frame == expected;
        ++frames;
    };

    bool valid = decoder.feed("\0\0"sv, receive_frame);
    valid = valid && frames == 0 && !decoder.complete();
    valid = valid && decoder.feed("\0\3A"sv, receive_frame);
    valid = valid && frames == 0 && !decoder.complete();
    valid = valid && decoder.feed("BC\0\0\0\2DE"sv, receive_frame);
    valid = valid && frames == 2 && contents_match && decoder.complete();

    FrameDecoder oversized;
    valid = valid && !oversized.feed("\0\0\x10\x01"sv, receive_frame);
    valid = valid && !oversized.complete();
    FrameDecoder empty;
    valid = valid && !empty.feed("\0\0\0\0"sv, receive_frame);
    FrameDecoder truncated;
    valid = valid && truncated.feed("\0\0\0\3A"sv, receive_frame);
    valid = valid && !truncated.complete() && frames == 2;

    std::cout << (valid ? "frame decoder: PASS\n" : "frame decoder: FAIL\n");
    return valid ? 0 : 1;
}
```

```sh
g++ -std=c++17 -Wall -Wextra -Werror frame_decoder.cpp -o frame_decoder
./frame_decoder
```

Expected output: `frame decoder: PASS`.

`header_bytes_` tells the decoder which part of a frame it is reading. Shifting the unsigned length by eight positions makes room for the next header byte. Four bytes describe the length; validation occurs before any payload is stored.

`received_` counts only payload bytes. When it reaches the validated length, the callback receives the complete frame and the decoder resets for the next header. The fixed array bounds payload storage. An invalid length puts this decoder in a permanent failed state; the connection-level policy should stop using it and reject the stream.

The `sv` literals preserve embedded zero bytes in the test input. A normal null-terminated string interpretation would stop at the first zero and would not describe these binary frames correctly.

The callback must finish using the supplied view before returning, because the next frame reuses the buffer. This teaching implementation assumes the callback does not throw and that one thread owns the decoder. Copy into separately owned bounded work storage if processing must happen asynchronously.

This example does not implement a network connection, authentication, or a receive deadline. A socket loop feeds each positive-length receive result into it. On peer EOF, `complete()` distinguishes a clean frame boundary from a truncated frame; application success can still require a particular number or kind of messages.

### 17.6 Timeouts must cover the whole frame

A peer could send one byte just before each per-read timeout. If the full timeout starts again after every byte, an incomplete frame can occupy a connection indefinitely. Instead establish one frame deadline, track progress, and wait only for the remaining budget.

Also bound the number of connections and accepted work. A 4096-byte frame limit does not protect memory if an unlimited number of connections each allocate their own buffers. Per-object bounds and total-system bounds are separate.

### 17.7 What UDP changes

UDP preserves datagram boundaries: one received datagram corresponds to a sent datagram, subject to loss and truncation behavior. It does not guarantee delivery, order, or duplicate suppression. If the receive buffer is too small, the receiver must detect and handle truncation using the platform's socket interfaces.

For telemetry where only the newest sample matters, attach a sequence number and timestamp so the receiver can identify stale or reordered data. If every record must arrive, adding retransmission, ordering, congestion control, and recovery becomes substantial protocol work; TCP or another established protocol may be a better starting point.

**Question:** does a successful TCP send prove the remote logger stored the record? **Answer:** no. It does not even establish your application's processing milestone. Use an application reply whose meaning is explicitly defined.

### 17.8 Record integrity and durability are separate

A log can contain bytes after a crash without containing a complete valid record. If a record write is interrupted, recovery must distinguish committed records from an incomplete tail. A defined record length, version, sequence, and integrity check can help identify malformed or torn content.

An integrity checksum detects certain accidental corruption under its algorithm's limits. It is not proof that a record is authorized, and it is not a substitute for a cryptographic authenticity mechanism against deliberate modification. Likewise, a valid checksum does not prove a record was durably persisted before acknowledgment.

A recovery parser must bound lengths before allocating or scanning, reject impossible metadata, and follow a documented rule for incomplete tails. "Read until the file ends" is not enough to decide whether the last operation committed.

Filesystem-specific ordering and synchronization behavior determine how a durable commit is implemented. Operations such as rename and directory updates have platform-specific persistence considerations. Do not import a Linux filesystem recipe as a universal QNX guarantee.

### 17.9 Backpressure propagates across service boundaries

TCP has flow control for byte transport, but that does not automatically bound your application's decoded-work queue. A fast receiver can drain the socket into unlimited heap objects while the processing worker falls behind.

Connect transport admission to application capacity. When the application cannot accept more work, a design can stop reading for a bounded interval, reject a request through its protocol, or close a connection under documented overload behavior. Stopping reads eventually affects the sender's ability to transmit, but the resulting waits still need timeouts and a policy.

Per-client limits prevent one peer from using the entire service budget. A global limit prevents many individually compliant peers from collectively exhausting it. Reserve enough capacity for the control operations required to observe or recover from overload.

### 17.10 Reconnection does not continue the old byte stream

When a connection fails halfway through a frame, the next connection is a new stream. Do not feed its first bytes into a parser that still expects the remainder of the old frame. Reset connection-local framing state, then perform whatever version or identity handshake the protocol requires.

Application operation identity can survive reconnection if deliberately designed, but transport position does not automatically do so. A client resending operation 42 over a new connection needs server-side rules for recognizing duplicates and reporting the previous or current result.

Bound retry attempts and total elapsed retry time. A per-attempt two-second timeout repeated indefinitely is still an unbounded application operation. Add jitter to retry spacing where many clients could otherwise reconnect in synchronized bursts, while preserving the overall recovery budget.

### 17.11 Encryption protects transport, not every application invariant

An established secure transport can provide confidentiality and authenticated integrity under its configuration and threat model. Certificate or credential validation must match the intended peer identity. Encryption alone does not decide which commands that peer is allowed to execute.

Even authenticated data needs length bounds, parsing, rate limits, and semantic validation. A legitimate peer can be buggy or compromised. Conversely, a parser that rejects malformed bytes does not prevent an unauthorized but well-formed administrative command.

Choose established security libraries and platform-supported integration. Handwritten encryption is not an appropriate extension to the framing lesson. Keep credential material out of source, diagnostic payload dumps, and learning logs.

## 18. Debugging and Performance

### Observe before changing priorities

| Question | Evidence to collect |
|---|---|
| Is the thread running, ready, or blocked? | `pidin` thread-state output |
| Who can unblock it? | Channel, mutex, queue, or dependency ownership |
| Is it waiting for receipt or a reply? | SEND/REPLY state and server trace |
| Is the result late because of CPU demand or waiting? | Timestamped execution and blocking intervals |
| Does failure depend on load? | Controlled workload with recorded parameters |
| Is a resource leaking? | Repeated lifecycle tests and resource counts |

**QNX target starting commands:**

```sh
pidin
pidin ar
```

For a known lab process, substitute its actual PID:

```sh
pidin -p 12345 threads
pidin -p 12345 mem
```

Consult the installed `pidin` reference for formats supported by your version and permissions. Minimal images may omit utilities; absence is an image configuration issue, not proof the OS lacks the feature.

### Debugger route

1. Build with debug information and retain the exact unstripped executable.
2. Use the matching QNX debugger and target connection through your approved Momentics, VS Code toolkit, or command-line workflow.
3. Set a breakpoint in a lab handler and inspect arguments, stacks, and thread states.
4. For a crash, preserve the core where configured, executable, shared libraries, image version, and logs.
5. Reproduce against the same binaries before interpreting addresses or stack frames.

`qconn` and debug agents belong on trusted development networks, not exposed production interfaces. A breakpoint changes timing and may freeze dependencies; it is not suitable for measuring deadlines.

### Tracing route

QNX kernel instrumentation, `tracelogger`, and System Profiler can reveal scheduling and IPC relationships when the image and permissions support them. Start with a short, filtered capture, mark your application events, and compare trace timing with your own measurements.

Record count, minimum, median, p95, p99, maximum, deadline misses, and test conditions. Store timing samples during the workload and print summaries afterward to avoid measuring terminal output.

For a reproducible claim, record CPU model, core placement, priorities, policy, load, build optimization, image, and duration. A VM is good for API learning but not representative evidence for a hardware timing guarantee.

**Practice:** E18. Your deliverable is an explanation supported by a trace, not just a screenshot.

### 18.1 Start with a question that evidence can answer

"The application is slow" does not identify a cause. Ask a smaller question: is the acquisition thread using CPU, waiting for a CPU, or waiting for a resource? Each answer leads somewhere different.

A RUNNING observation suggests the thread is executing at that instant. READY means it can run but has not been selected on an eligible CPU. A blocked state means some event or resource is still required. One snapshot can miss transitions, so repeat observations or use a trace when needed.

Before collecting data, state what would change your diagnosis. If you suspect a missing reply, finding the client continuously READY rather than REPLY-blocked would challenge that hypothesis. If you suspect expensive calculation, discovering that almost all elapsed time is a storage wait points to another cause.

### 18.2 Worked diagnosis: the client never finishes

Suppose the sensor's command thread is REPLY-blocked. That tells us its request was received; the server has not yet completed the transaction.

First identify the server and the request. Then inspect where the server's responsible thread is waiting or executing. If it is waiting for storage, follow that dependency. If it has already returned to its receive loop, inspect whether the earlier branch forgot `MsgReply()` or `MsgError()`, unless the design intentionally handed completion to another worker.

Imagine the code handles valid commands and unknown commands, but an invalid-version branch simply continues receiving. That branch leaves the client waiting. The repair is to give that accepted transaction an explicit error completion, not to raise client priority.

After repairing it, resend the invalid request and confirm the client receives the expected error within its waiting policy. Then send a valid request to confirm the service remains usable. This tests both rejection and continued operation.

### 18.3 Worked diagnosis: shutdown hangs

Main is blocked in `join()`. The worker is blocked on the queue's data condition variable. The queue is empty. Main stopped production but never set the closed state or notified the worker.

Draw the dependency:

```text
main waits for worker completion
worker waits for data or closure
no producer remains, and main never published closure
```

There is no useful way for the worker to infer that work has ended. Publish closure under the mutex, notify affected waiters, then join. When diagnosing a full-queue shutdown, also inspect producers waiting for space; waking consumers alone may be insufficient.

The important question is not merely "which call blocks?" but "who is responsible for creating the event that ends this wait?"

### 18.4 A trace tells a time-ordered story

A debugger shows values, stacks, and execution position. A trace records events over time, which makes it useful for scheduling and IPC relationships. A profile summarizes resource use, such as which functions consumed CPU. These tools answer related but different questions.

Consider this invented trace in milliseconds:

```text
100.0  intended sensor release
100.4  sensor becomes READY
101.2  sensor begins execution
102.0  sensor waits for queue mutex
103.0  sensor acquires queue mutex
103.2  result is available
```

Release-to-start lateness is 1.2 ms. READY-to-start delay is 0.8 ms. The observed mutex wait is 1.0 ms. Response time from intended release is 3.2 ms. Those are different measurements; calling all 3.2 ms "CPU execution time" would hide the actual cause.

A breakpoint changes this timeline by stopping execution. Use breakpoints to understand state, and nonintrusive or carefully characterized tracing for timing. Even tracing adds overhead, so record the collection method.

### 18.5 Read statistics without inventing a guarantee

Suppose 99 out of 100 requests finish in 1 ms and one finishes in 100 ms. The mean is 1.99 ms. Under the nearest-rank method, p99 is 1 ms, while the maximum is 100 ms. A favorable percentile can coexist with a severe outlier.

For a 10 ms hard deadline, the outlier matters even though the mean looks good. A test with no observed misses supplies evidence under its tested conditions, not proof that no future execution can miss.

Record sample count, method, test duration, load, hardware, build settings, priority, affinity, and image version. A maximum without those conditions is difficult to interpret or reproduce.

### 18.6 Understand crash evidence

A crash stack can point to where failure became visible, not necessarily where corruption began. A buffer overwrite may corrupt state long before another function dereferences it. Preserve the exact executable, debug symbols, libraries, target image information, and available core dump so addresses can be interpreted against the correct build.

AddressSanitizer can expose certain invalid memory accesses in a supported Linux build. ThreadSanitizer can expose certain data races in a supported environment. Neither proves all bugs absent, and availability varies by toolchain and target. A clean portable test is valuable without being native QNX verification.

**Question:** a worker's heartbeat continues but no records are processed. Is the worker healthy? **Answer:** not necessarily. The heartbeat may come from an unrelated timer or thread. Observe useful progress such as completed sequence numbers, queue consumption, and the age of the last completed operation.

### 18.7 Correlate a logical operation across components

A local timestamp tells you when one process observed an event. A correlation identity tells you which logical operation that event belongs to. For a request path, record events such as accepted, enqueued, started, committed, replied, and failed with an operation identity and service generation.

Do not use a PID or `rcvid` as the sole long-term correlation identity. Their meaning is tied to OS-managed lifetimes and contexts, and identifiers can be reused. A service generation plus an application operation identity makes restart boundaries explicit.

Correlation logging must be bounded and privacy-aware. Logging entire requests may expose secrets or personally identifying information, and synchronous logging can perturb the very timing being diagnosed. Prefer small structured events with controlled retention and clearly defined timestamp boundaries.

### 18.8 A performance experiment needs a discriminating comparison

Suppose you hypothesize that formatting under a mutex causes acquisition delay. Measure lock wait and hold durations under a recorded workload, then move formatting outside the lock while preserving a coherent local snapshot. Repeat under comparable conditions.

If total latency does not improve and lock waits were already negligible, the evidence challenges the hypothesis. Inspect the next largest contributor rather than adding more synchronization changes. If lock waits improve but storage latency remains dominant, report both facts rather than calling the whole deadline problem solved.

Change one relevant factor at a time where practical: workload, optimization, affinity, or trace configuration. Otherwise a better result cannot be confidently attributed to the code change.

### 18.9 Test failures near lifecycle boundaries

Many bugs occur just before or after ownership changes: closing while full, timing out after receipt, exiting after a side effect but before reply, or restarting while a peer still holds an old mapping.

Use controlled synchronization or fault hooks to reach these boundaries. A random delay can expose a failure, but it is a weak way to prove that a specific boundary was exercised. For example, signaling "about to call push" does not establish that the producer has actually entered the full-queue wait; instrumentation or a suitably controlled test arrangement is needed for that precise claim.

Pair each injection with an independent time bound and cleanup. The test harness must remain able to report failure if the application under test deadlocks. An assertion inside a thread that never wakes cannot serve as the experiment's only timeout.

## 19. Security, Reliability, and Safety

### Security

- Run services with only the identities and abilities they need.
- Validate untrusted IPC, file writes, `devctl` data, and network requests.
- Bound buffers, queue lengths, connections, and outstanding transactions.
- Separate administrative commands from ordinary data operations.
- Remove or restrict development services in deployed images.
- Keep secrets out of logs, core artifacts, source control, and public traces.

QNX abilities and security policy provide controls beyond a simple root/non-root model. Diagnose a denied operation against the intended policy rather than granting all privileges.

### Reliability

Design for process exit, timeout, stale data, storage exhaustion, restart, and partial initialization. A heartbeat should represent useful progress, not just a timer thread that remains alive while workers are stuck.

Use bounded restart attempts and backoff. A fast crash/restart loop can turn a local failure into system overload. Define whether clients reconnect, discard old requests, retry idempotent operations, or enter a degraded state.

### Safety

An RTOS, a microkernel, and a watchdog do not make an application functionally safe by themselves. Safety-certified product variants and artifacts have defined scope and assumptions. ISO 26262 or other certification work requires the applicable lifecycle, traceability, analysis, verification, and qualified product documentation.

Keep this course's labs separate from any claim that software is production-ready or safety-certified.

**Practice:** E19-E20. Make failures observable and recovery bounded.

### 19.1 Identity and permission answer different questions

**Authentication** establishes who a participant is. **Authorization** decides what that participant is allowed to do. Discovering a service name or PID does not establish either one by itself.

For a sensor service, reading status may be allowed to diagnostic users while changing the sampling period is restricted. Even an authorized user can supply malformed input, so permission checks do not replace payload validation. Conversely, a well-formed command is not automatically authorized.

A useful sequence is to establish the relevant peer identity through supported mechanisms, verify the operation is permitted, validate its data, and apply it under the required synchronization. Do not trust an identity field in an untrusted payload merely because the client wrote it there.

### 19.2 Least privilege limits consequences

Least privilege means granting only the access required for the component's responsibilities. A logger may need write access to its log destination without needing access to sensor hardware or system-wide process control.

QNX **abilities** control specified privileged operations. Filesystem permissions, peer checks, and security policy can impose additional controls. These are related layers, not a single universal "root fixes everything" setting.

Suppose creating a public endpoint fails with a permission error. The first question is whether the service should create that endpoint under its intended identity and policy. Granting unrestricted privileges can hide a configuration mistake while widening the consequences of a later bug.

For the teaching system, separate ordinary data commands from administrative changes. Bound payload sizes, accepted connections, outstanding requests, and work rates so an authorized but malfunctioning client cannot consume unlimited resources.

### 19.3 Reliability begins with explicit service states

A service is not just "running" or "dead." It can be starting, ready, degraded, stopping, or failed. Each state should constrain which operations are accepted.

```text
STARTING -> READY -> STOPPING -> STOPPED
                  |
                  v
              DEGRADED -> READY or STOPPING
```

During STARTING, storage may not yet be open. During READY, the defined service is usable. During DEGRADED, perhaps storage has failed but status queries still work. During STOPPING, reject new records and drain or discard existing work according to policy.

These states make error behavior understandable. Without them, one thread may continue accepting records while another has already destroyed the only output path.

### 19.4 A watchdog needs a meaningful progress signal

A watchdog detects the absence of an expected action within a chosen interval and triggers a defined response. It does not automatically know whether useful work is happening.

If a timer thread sends "alive" every second while the logger worker is deadlocked, the signal proves only that the timer thread is alive. Instead, observe the age of unfinished work, the last completed sequence, and whether required workers are progressing. An idle service with no pending work should not be declared faulty merely because its completed count stays unchanged.

Choose a threshold that accounts for permitted workload and delays. Too short can produce needless restarts; too long can allow a fault to persist beyond requirements. An independent supervisor provides a different failure boundary from a watchdog thread inside the same possibly stuck process.

### 19.5 Work through a restart with an uncertain result

Client instance A submits request 42. The logger writes the record and crashes before replying. The client observes a failure, but it does not know whether request 42 took effect.

A replacement logger has a new process lifetime and new endpoint information. The client must rediscover and connect to the replacement. Reusing an old receive ID is meaningless; that transaction belonged to the earlier lifetime.

Now choose a data policy explicitly:

- Retrying without deduplication can duplicate the record.
- Not retrying can leave the client uncertain whether the record exists.
- Retrying with a persistent operation identity and a correctly maintained deduplication record can provide stronger duplicate suppression, but the data and deduplication state need a coordinated recovery design.

For learning telemetry, sequence-labelled records plus a documented possibility of gaps or duplicates may be adequate. For accounting or configuration changes, stronger guarantees may be required. The right answer comes from the requirement, not from making the timeout longer.

### 19.6 Recovery must also be bounded

If a service crashes because its configuration is invalid, immediately restarting it a thousand times does not correct the configuration. It can consume CPU, fill logs, and prevent other work from progressing.

Use an attempt limit, increasing or otherwise bounded retry spacing, and an observable terminal or degraded state after repeated failure. Recheck readiness after each successful launch. A newly assigned PID is not evidence that initialization completed.

During recovery, decide what upstream components do: stop accepting work, drop bounded telemetry with counters, keep a bounded backlog, or switch to a defined fallback. An unlimited recovery queue merely converts a service fault into eventual memory exhaustion.

### 19.7 Functional safety is a lifecycle, not a feature flag

Reliability asks whether the service continues to work as intended. Functional safety asks how failures can contribute to unacceptable harm and how that risk is controlled within the applicable system lifecycle. They overlap but are not identical.

A safety argument connects hazards, requirements, architectural controls, implementation, verification, and evidence. For example, detecting stale data is only useful if the system has a justified response to that condition and evidence that detection and response meet their timing requirements.

Using a safety-certified OS component can contribute evidence within that component's certified scope and assumptions. It does not certify your parser, shutdown logic, driver configuration, or whole product. The simulated lessons here teach mechanisms, not a production safety claim.

**Questions with answers:**

1. **Can a privileged service skip validating its messages?** No. Its privilege makes malformed-input handling more important, not less.
2. **Does restart guarantee recovery?** No. Dependencies, state, endpoint discovery, and unknown request outcomes still need handling.
3. **Is an unbounded queue a valid way to avoid dropping telemetry?** No. Under sustained overload it can exhaust memory. Define an achievable storage and overload policy.

### 19.8 Describe the threat boundary before assigning permissions

A **threat model** identifies protected assets, possible adversaries or faulty participants, interfaces they can reach, and the consequences of misuse. In the telemetry example, assets include configuration integrity, availability of acquisition, stored records, and credentials used for remote access.

A local diagnostic process may be trusted to read status but not to submit arbitrary control changes. A network client may need authentication before any expensive request parsing. A driver has hardware access whose consequences exceed an ordinary application's address-space writes.

At each boundary, ask what prevents unauthorized operations and what bounds resource use even by authorized peers. Also ask what evidence is logged without exposing the protected information. Least privilege, validation, and rate limits address different parts of this model.

### 19.9 Reliability requirements must specify tolerated failure

"Never lose a record" is incomplete unless the failure model is stated. Must records survive one logger-process crash, whole-system reset, sudden power loss, failed storage media, or loss of an entire machine? These require different storage and replication strategies.

For example, keeping accepted records only in the acquisition process may tolerate logger restart but not acquisition-process failure. Persisting before acknowledgment may tolerate process failure but still depends on the storage device's power-loss guarantees. Replication adds coordination and recovery questions rather than automatically supplying every guarantee.

State the allowed loss window and recovery time. A system that may lose the latest second of noncritical telemetry has different design choices from one that must preserve every acknowledged transaction. Requirements should expose that difference before implementation effort is spent.

### 19.10 A safety argument connects cause, control, and evidence

Consider a simulated stale-data failure. The cause might be an acquisition worker that stops progressing. The detection could compare the current monotonic time with the timestamp of the last valid completed sample. The response could mark data invalid and notify the supervising application within a defined interval.

The argument still needs assumptions: the monitor runs independently enough to observe the failure, its clock remains suitable, the threshold distinguishes tolerated delay from a fault, and downstream users honor the invalid state. A heartbeat from the stuck worker's own path cannot be assumed to continue when that path fails.

Evidence includes the detection code's correctness, scheduling and timing analysis, fault-injection results, and system-level verification of the response. A test that only prints "stale" does not establish that downstream behavior is safe.

This example teaches the structure of an argument, not a certified safety function. Actual hazards, assurance levels, tools, and lifecycle activities must come from the applicable product context and standards.

### 19.11 Trade-offs should be recorded as decisions with limits

An architectural decision can say: "Use a separate bounded logger service because storage restart must not terminate acquisition. Acknowledgment means accepted into volatile memory; up to the documented pending-record bound may be lost on logger failure. Acquisition rejects new records when its local queue is full and increments a visible counter."

That is more useful than "use microservices for reliability." It gives a reason, a behavior, and a limitation that can be tested. It also exposes disagreement early: if a stakeholder requires durable acknowledgment, the stated design must change rather than relying on ambiguous wording.

Revisit a decision when its assumptions change. A tenfold sampling-rate increase, new remote clients, or a stronger failure guarantee can invalidate a formerly adequate queue, scheduling model, or security boundary.

## 20. Practice Workbook

Each exercise needs code or a design artifact, experiment notes, and a pass/fail result. Read the hints only after your first attempt. Time estimates exclude installing the SDK.

### E01. Explain the architecture

**Environment:** Design. **Time:** 30 minutes.

Draw an application, filesystem service, device service, and microkernel. Trace a file read at a conceptual level. Explain microkernel, real-time, and host/target in your own words.

**Pass:** distinguish a deadline from average speed; identify process boundaries; explain why the existing Linux container cannot run `MsgSend()` natively.

### E02. Prove the toolchain path

**Environment:** Host + QNX target. **Time:** 45-90 minutes.

Build the bounded-queue example for your target architecture and run it there. Record compiler variant, SDP version, target version, command line, and output.

**Pass:** expected count/sum and exit status zero; identify why the QNX executable is not interchangeable with the Linux build. Diagnose, without altering security policy, one deliberately selected wrong-path or wrong-variant build configuration.

### E03. Thread ownership

**Environment:** Linux, then QNX target. **Time:** 45 minutes.

Create two threads that each perform a fixed number of updates to a protected counter. Join both, then check the result. Give each thread independent diagnostic data to avoid unsynchronized output.

**Pass:** exact total across 100 repeated runs and a clear explanation of synchronization. Explain why simply making the counter `volatile` would be incorrect; do not rely on repeated success to prove the absence of a data race.

### E04. Measure periodic release latency

**Environment:** Linux for mechanics; QNX target for QNX results. **Time:** 90 minutes.

Implement 500 releases at a 10 ms period using a monotonic absolute timeline. Store scheduled and actual wake times in fixed-capacity storage. Compare against a relative-wait variant.

**Pass:** report lateness distributions and accumulated phase error; state an overrun policy. Treat 10 ms as a lab parameter, not a guaranteed deadline on all hosts.

### E05. Observe scheduling

**Environment:** QNX target. **Time:** 90 minutes.

Run two finite CPU workloads at permitted priorities with an independent recovery console. Record their policies and effective priorities. If approved and supported, constrain both to one CPU so the experiment has an understandable baseline; repeat without that restriction.

**Pass:** explain observed ordering, equal-priority behavior, and multicore differences. Never use an unbounded high-priority busy loop.

### E06. Demonstrate priority inversion

**Environment:** QNX target; Linux only where scheduling permissions permit. **Time:** 2 hours.

Coordinate low-, medium-, and high-priority threads using explicit startup synchronization. Low holds a mutex for bounded CPU work; high attempts it; medium supplies bounded interference. Compare an explicitly selected no-inheritance protocol with supported priority inheritance.

**Pass:** a timeline showing high's wait and the owner's effective priority. Keep all threads on one approved CPU for the first experiment. Do not use sleeping as the low thread's lock-held workload: an inherited priority cannot make a sleeping owner execute.

### E07. Strengthen the bounded queue

**Environment:** Linux, then QNX target. **Time:** 2 hours.

Extend the section 9 example with tests for: close while empty, close with queued data, close while a producer is blocked on full capacity, repeated close, and rejection of pushes after close. Add two producers with distinct sequence ranges and join producers before closing during normal operation.

**Pass:** no lost or duplicate accepted items; consumers drain before ending; shutdown tests have an external timeout. Use test synchronization to arrange edge cases, not arbitrary delays. Stretch: add an absolute-deadline push API with distinct timeout/closed/success results.

### E08. Trace a native IPC transaction

**Environment:** Host + QNX target. **Time:** 60 minutes.

Build and run section 11. Observe server RECEIVE blocking before the client starts. Add an optional finite delay before the server receives and another before it replies to make SEND and REPLY states observable. Keep each below the client timeout for the success case.

**Pass:** `reply=42`, correct exits, and state observations consistent with the two delays. Restart the one-shot server for every case.

### E09. Validate malformed requests

**Environment:** QNX target. **Time:** 90 minutes.

Extend the client with test modes for a short request, an oversized request, an unsupported version, and an out-of-range value. Preserve the blocking timeout.

**Pass:** bad sizes produce `EMSGSIZE`; invalid supported-size fields produce `EINVAL`; the server remains alive for a subsequent valid request. Do not accept a truncated oversized message merely because its prefix looks valid.

### E10. Exercise timeout semantics

**Environment:** QNX target. **Time:** 90 minutes.

Delay receipt beyond two seconds, then separately delay a reply beyond two seconds. Observe `ETIMEDOUT` and what happens when the server later tries to reply. Also test a server that exits before completion.

**Pass:** the client leaves its blocked state and reports the actual error. Explain why a timeout does not prove the server did no work. Use an independent supervisor to bound total experiment time.

### E11. Build a timer-pulse loop

**Environment:** QNX target. **Time:** 2 hours.

Implement the recipe in section 12 with a 100 ms interval and a normal permitted priority. Receive 50 timer notifications, measure their timing, and perform ordered cleanup. Use an external timeout in case notification setup fails.

**Pass:** only the expected application pulse code is counted; no pulse receives a reply; creation failures clean up earlier resources. Stretch: add bounded slow work and document missed/late-release behavior instead of assuming notification count equals elapsed periods.

### E12. Share a telemetry buffer

**Environment:** QNX target; POSIX subset on Linux. **Time:** 2-3 hours.

Create a fixed-capacity shared region with a magic value, version, capacity, generation, and records. Define one writer and one reader. Use supported process-shared synchronization and a clear ready handshake before the reader accesses initialized state.

**Pass:** readers never observe a partially published record; bad versions and bounds are rejected; close/unmap/unlink ownership is documented. For creator failure, reject the stale generation and recreate under external coordination rather than guessing that in-place state is recoverable.

### E13. Implement a read-only resource manager

**Environment:** QNX target. **Time:** 3 hours.

Use the official minimal resource manager example as a starting point. Expose a simulated sensor at `/dev/learn_sensor` with the snapshot behavior from section 14 and appropriate development permissions.

**Pass:** a full read ends at EOF, one-byte reads reconstruct the same snapshot, and two independent opens have independent offsets. Handle shutdown and attach errors. Do not overwrite an existing device pathname.

### E14. Add resource manager controls

**Environment:** QNX target. **Time:** 2 hours.

Add a validated write command for a simulated sampling period, restricted to 10-1000 ms. Add one documented `devctl` query for the current configuration.

**Pass:** malformed, too-long, and out-of-range writes do not change state; short reads and read-only opens behave correctly; unsupported controls return the documented error rather than a success-shaped response.

### E15. Review C++ resource lifetime

**Environment:** Linux + Design; repeat target paths on QNX. **Time:** 90 minutes.

Choose your logger or queue implementation. List every allocation, lock, descriptor, and worker lifetime. Introduce a failure after each initialization step and verify that previously acquired resources are released.

**Pass:** no joinable worker survives destruction; no blocking call outlives the resource it uses; each owner has one clear cleanup responsibility. Propose one justified fixed-capacity improvement without rewriting unrelated code.

### E16. Explain a boot image

**Environment:** BSP source + Design; optional spare target. **Time:** 2 hours.

Read the buildfile of your approved BSP. Identify startup, `procnto`, console, storage, networking, and application startup. Draw their dependency order and readiness conditions.

**Pass:** explain what would fail if each dependency were missing. Only attempt an image change with a known-good recovery image and board-specific instructions; no flashing is required to pass the analysis exercise.

### E17. Handle network fragmentation

**Environment:** Linux, then QNX target. **Time:** 2 hours.

Implement a TCP telemetry receiver with a fixed-size length prefix and a maximum frame size of 4096 bytes. Specify byte order. Test one frame split across several sends, two frames combined, zero-length policy, oversized lengths, and disconnect mid-frame.

**Pass:** valid messages are reconstructed, incomplete messages are not processed, and invalid lengths do not cause unbounded allocation. Define receive deadlines and bounded reconnect behavior.

### E18. Diagnose a blocked service

**Environment:** QNX target. **Time:** 2 hours.

Introduce one controlled missing-reply bug in a timeout-protected lab service. Capture thread state and, where available, a short IPC trace. Locate the branch that failed to complete the request, repair it, and repeat.

**Pass:** show why the client was REPLY-blocked and why a priority change would not fix it. Every accepted request now has a defined completion or cancellation path.

### E19. Design least-privilege deployment

**Environment:** Design; verify on QNX when available. **Time:** 60 minutes.

For the capstone, list process identities, IPC peers, paths, network endpoints, debug access, and needed abilities. Define permissions for data versus administrative operations.

**Pass:** each privilege has a reason; malformed requests and resource-exhaustion risks have controls; the design does not require unrestricted privileges merely for convenience.

### E20. Recover from process failure

**Environment:** QNX target. **Time:** 2 hours.

Use a disposable version of the capstone service. Terminate it while a client has an outstanding request, then restart it through your lab supervisor. Reconnect using a fresh endpoint and perform a new request.

**Pass:** no indefinite client wait, no reuse of stale transaction identifiers, bounded restart attempts, and an explicit policy for requests whose outcome is unknown. Record the actual failure codes rather than requiring one error code for every race.

### 20.1 What makes an exercise a useful experiment

A useful test has a controlled starting state, an action, an observable outcome, and a limit on how long it can wait. The outcome needs an **oracle**, meaning a rule that distinguishes correct from incorrect behavior. "It did not crash" is usually too weak an oracle for a concurrent service.

For the queue, accepted item identities supply an oracle for loss and duplication. For a rejected configuration request, unchanged configuration supplies an oracle for no partial update. For recovery, a successful fresh request plus rejection of stale state supplies a stronger oracle than merely observing a replacement PID.

Record which event the test actually arranged. A thread created successfully is not necessarily waiting yet. A request sent by a client is not necessarily received yet. A server process launched successfully is not necessarily ready. Test synchronization and traces turn these distinctions into evidence.

### 20.2 Worked testing logic for architecture and build exercises

**E01:** draw process boundaries before arrows. Put the filesystem service outside the application and explain the request/completion path. The discriminating question is whether a failure in the service necessarily corrupts the application's address space. It need not, although it can remove the service the application depends on.

**E02:** preserve the build command and identify the executable's target architecture and required runtime. Run it on the matching QNX target and record its exit status. A Linux build of the same source is a different artifact. A successful portable run cannot fill the target-result field.

**E03:** the expected total is the sum of the workers' iteration counts. Check only after joining every writer. Repetition tests observed behavior, while the proof explains that all conflicting increments share a mutex and the result is not read during modification. Report both; neither replaces the other.

### 20.3 Worked testing logic for timing and scheduling

**E04:** keep intended releases separately from actual starts. Calculate lateness from each intended release, and count skipped releases rather than silently resetting the phase. Comparing a relative-wait version is meaningful only if work and measurement conditions are comparable.

**E05:** establish policy, effective priority, and CPU eligibility before interpreting order. If two jobs run on different cores, observing the lower-priority job progress does not disprove fixed-priority scheduling. If the high-priority job is blocked, it is not competing for CPU at that instant.

**E06:** arrange for low to own the mutex before high tries to acquire it. Then make medium's finite CPU demand available. Observe the owner and high's wait under both protocols. If low sleeps while holding the mutex, inheritance cannot make it execute during that sleep, so the experiment tests a different limitation.

The common lesson is to control the condition that distinguishes competing explanations. A colorful trace with unknown policy or affinity is difficult to interpret.

### 20.4 Worked testing logic for queue shutdown

**E07, empty case:** arrange an open empty queue and a consumer waiting for data. Close it. The consumer should return false and exit, and joining should complete within the external test limit.

**E07, queued case:** insert known identities, then close. Consumers must receive exactly those accepted identities before terminating. Repeated close must not invent an item or reopen admission. New pushes must fail.

**E07, full case:** fill the queue and arrange an additional producer in its space wait. Close while it remains full. The additional push must report rejection rather than remain stuck. Verify the test actually exercised the wait if claiming coverage of blocked-producer wakeup.

With multiple producers, assign distinct bounded identity ranges. Record which pushes returned success and compare that set with consumed identities after all workers finish. A sum is not sufficient to detect all duplicate/loss combinations. For ordinary completion, join producers before close so expected remaining work is not deliberately rejected.

### 20.5 Worked testing logic for native transactions

**E08:** delay before receipt to expose SEND waiting; delay after receipt to expose REPLY waiting. Each success-case delay must fit within the overall timeout budget. A sampling tool may miss brief states, so use sufficiently visible but finite delays on the disposable target.

**E09:** send short and oversized messages separately. Verify that the full source length is checked, not just the copied prefix. Then vary version and value with a correctly sized message. A subsequent valid request establishes that rejection did not accidentally terminate or corrupt the service.

**E10:** cross the timeout before receipt, then separately after receipt. In the second case, record whether work happened and what the server observes when it later attempts completion. The correct conclusion is the actual transaction outcome and its uncertainty, not an assumption that timeout undid the work.

**E11:** distinguish timer creation, arming, notification receipt, and work completion. Failure after connection attachment must release that connection; failure after timer creation must also release the timer. A receive-loop timeout or external supervisor is needed to make a missing-notification test terminate.

### 20.6 Worked testing logic for shared and file-like state

**E12:** test startup with a peer attempting access before publication. The peer must not use uninitialized synchronization. Then test a bad version, bad capacity, and out-of-bounds offset before any payload access. For owner failure, coordinate old-participant shutdown before replacing the region; do not repair a live mapping by guessing.

**E13:** reconstruct a captured snapshot one byte at a time, then issue another positive-length read and expect zero. Open a second independent instance and show that it begins at its own offset zero. If the underlying sensor changes mid-read, confirm the documented snapshot rule.

**E14:** retain the old configuration, submit invalid commands, and compare the full configuration afterward. A returned error with partly modified state fails the contract. Test permitted and denied access as separate cases from valid and invalid payloads.

**E15:** inject a failure after each successful acquisition step. Verify that only acquired resources are released and that no worker outlives anything it borrows. The cleanup order matters more than merely counting destructor calls.

### 20.7 Worked testing logic for system failures

**E16:** remove one dependency from the boot diagram conceptually and trace what loses readiness. If networking is missing, local acquisition might continue while remote diagnostics fail. The answer should follow actual requirements rather than declaring every missing component a whole-system failure.

**E17:** feed one frame across many chunks and multiple frames in one chunk. Keep the same expected messages while varying chunk boundaries. Reject zero/oversized lengths according to the chosen contract, and do not deliver incomplete payloads after disconnect. Separately test socket deadlines and reconnect state, because parser tests do not exercise them.

**E18:** identify the exact received transaction whose completion branch is missing. Repair that branch and confirm both an error case and a normal request finish. Increasing priority without closing the transaction does not satisfy the test.

**E19:** for each permission, identify the operation that requires it and a denied-operation test for something the service should not do. "Everything works as unrestricted root" supplies no evidence of least privilege.

**E20:** terminate the service at more than one point: before acceptance, during work, and after an effect but before reply. These can produce different uncertainties. Restart, wait for readiness, establish a fresh connection, and perform a new request. Verify retry limits and retained-work bounds when repeated startup fails.

## 21. Hints and Answer Checkpoints

These are checkpoints, not substitute measurements. A correct explanation must match your program and target configuration.

| Exercise | Hint / expected reasoning |
|---|---|
| E01 | Separate the OS mechanisms from the services using them; deadlines belong in the requirements |
| E02 | Toolchain output, target architecture, and runtime ABI must agree |
| E03 | Guard the entire read-modify-write; `join()` establishes completion before checking results |
| E04 | Advance one intended timeline rather than resetting it to the actual wake time |
| E05 | Compare READY threads eligible for the same CPU; blocked threads cannot win scheduling |
| E06 | Ensure low owns the mutex before high requests it; medium competes with a runnable low |
| E07 | Closure belongs in both wait predicates; normal close occurs after producers finish |
| E08 | Receipt ends SEND blocking; only reply/error/failure ends the transaction afterward |
| E09 | Validate full source length and bytes actually received before reading fields |
| E10 | The client can time out while the server still has an application-side work item |
| E11 | A zero receive ID identifies a pulse; code validation distinguishes its meaning |
| E12 | Initialization, publication, consumption, and restart need separate ownership rules |
| E13 | Offset belongs to each open instance; snapshot EOF is a zero-byte read |
| E14 | Parse into temporary state, validate fully, then commit once |
| E15 | RAII helps cleanup, but resource destruction still has ordering and timing constraints |
| E16 | A running process is not necessarily ready to serve its dependents |
| E17 | Maintain a framing state machine; neither send boundaries nor TCP packets define records |
| E18 | Trace the request's completion paths; do not infer a scheduling problem from waiting alone |
| E19 | Ask what each process needs to do, not which privileges remove every error |
| E20 | Reconnection creates a new lifetime; timeout/retry is not exactly-once execution |

### Three worked reasoning problems

**Problem A: periodic drift.** A loop does 3 ms of work and then waits a relative 10 ms. Ignoring other delays, release spacing becomes about 13 ms, not 10 ms. With an absolute 10 ms timeline, the remaining wait is about 7 ms when the work completes on time.

**Problem B: queue overload.** A producer generates 200 records/s and a consumer handles 150 records/s. A queue initially empty with 100 usable slots fills in roughly 2 seconds at that sustained net growth rate. A larger queue delays overload; it does not remove it. Choose block, drop, reject, or reduce input based on the requirement.

**Problem C: retry after timeout.** A client asks to increment a persistent counter. The server increments it, but the reply is delayed and the client times out. Blind retry can increment twice. A request ID plus durable deduplication may be needed if the requirement demands restart-safe duplicate suppression; making the timeout longer is not a semantic fix.

### 21.1 Worked derivation: burst, pause, and recovery

Assume an initially empty queue receives a burst of 12 records and then no more than 100 records/s. The consumer provides no service for 200 ms, then guarantees 250 records/s while backlog exists. This is a simplified fluid-rate model; real discrete arrivals, scheduling, and service variation need explicit bounds.

During the pause, the burst plus continued arrivals create approximately `12 + 100 * 0.2 = 32` records. After service resumes, the backlog decreases at `250 - 100 = 150` records/s. Clearing 32 records of backlog takes about `32 / 150 = 0.213` seconds beyond the pause under this model.

A 32-slot queue is therefore the modeled peak without extra margin, not a universal safe selection. Initial occupancy, coincident discrete arrivals, representation of the burst bound, or a weaker service guarantee can change the required capacity. A practical selected capacity must follow the actual discrete contract and memory budget.

Notice the difference between **backlog clearance time** and **one record's response time**. The division by net rate describes when the whole accumulated backlog disappears while new arrivals continue. A particular record's completion depends on its position and the service order.

### 21.2 Worked derivation: response time with repeated interference

Use the simplified single-core fixed-priority model from section 7. A high-priority task needs 2 ms every 5 ms. A lower-priority task needs 4 ms, has no blocking, and has an 8 ms relative deadline.

Start with the lower task's own 4 ms. One higher-priority release can interfere, so the next estimate is `4 + ceil(4/5) * 2 = 6` ms. At 6 ms, two higher-priority releases can interfere, giving `4 + ceil(6/5) * 2 = 8` ms. Repeating at 8 gives 8 again, so the modeled bound is 8 ms.

It exactly meets the modeled deadline with no allowance for omitted overhead. Add a justified 0.5 ms of blocking: the iteration becomes 4.5, then 6.5, then 8.5 ms, which exceeds the deadline. A seemingly small dependency can change the conclusion.

The inputs must be valid execution and blocking bounds for the model, not average measurements. If this model's assumptions do not fit the target scheduling and workload, its arithmetic is not a target certification.

### 21.3 Worked reasoning: acceptance, effect, and acknowledgment

Define three events for an operation: A is accepted into volatile work storage, B is applied to persistent state according to the chosen storage contract, and C is acknowledged to the client. Suppose the intended order is A, then B, then C.

A crash between A and B can lose pending work unless another owner retains it. A crash between B and C can leave the client uncertain about an already performed effect. A crash after C should not violate the guarantee that C advertised; if C promised durability but only A occurred, the implementation was incorrect before the crash.

Moving acknowledgment earlier reduces observed latency but weakens its meaning unless other mechanisms establish the same guarantee. Moving it later may improve the meaning of success but cannot eliminate every lost-reply uncertainty. The solution is an explicit contract and recovery protocol, not simply a different timeout.

### 21.4 How to judge an architectural answer

A strong answer names the requirement, identifies a mechanism, traces a normal execution, traces a failure, and states the conditions under which the conclusion holds. It also describes an observation that would show the design is wrong.

For example, "use a bounded queue" is incomplete. "Use a fixed-capacity queue so acquisition memory cannot grow with logger outage; reject immediately when full and count loss because acquisition must not wait for storage; include queued and in-flight records in accounting" describes a design that can be examined.

The remaining limitation is still important: that design does not promise lossless logging. Architectural maturity includes stating what was not guaranteed, even when the implementation is correct.

## 22. Capstone: Supervised Telemetry Logger

Build on the existing [logger design](../Design_patterns/Design_pattern_project/Design_doc/Thread_Safe_Logger_Design.md) and [thread-safe queue interface](../Design_patterns/Design_pattern_project/inc/Thread_safe_queue.hpp). Reuse portable ideas after reviewing their ownership and shutdown behavior; do not assume the existing implementation already meets the requirements below.

### Goal

Create a simulated sensor service that produces periodic records, accepts local commands through QNX IPC, and writes through a separate logger service without allowing slow storage to block time-sensitive acquisition indefinitely.

```text
timer notification -> acquisition thread -> bounded queue -> logger client
                            |                                   |
                     latest sample state                 native QNX IPC
                            |                                   |
                   status/control service                 logger service
                            |                                   |
                     diagnostic client                    bounded storage

             external supervisor observes useful progress
```

This is a learning system with simulated data. Do not connect it to physical actuators.

### Milestones

| Milestone | Deliverable | Acceptance test |
|---|---|---|
| M1 | Portable data model and bounded queue | Queue close/full/concurrency tests pass on Linux |
| M2 | QNX timer-driven acquisition | Timestamp 1000 intended 10 ms releases; report lateness and misses |
| M3 | Versioned native IPC commands | Valid, invalid, timeout, and disconnected-client tests |
| M4 | Separate logger service | Storage delays do not create unbounded acquisition blocking |
| M5 | Status and configuration | Report queue depth, drops, progress, and current configuration consistently |
| M6 | Optional resource manager facade | Read status and validate controls using file APIs |
| M7 | Recovery and observability | Demonstrate restart, bounded retry, and useful-progress monitoring |

### Required design decisions

- Use monotonically increasing sequence numbers and monotonic timestamps for records.
- Choose and justify queue capacity. Keep memory bounded under sustained overload.
- Define whether full queues drop newest, drop oldest, reject, or wait with a deadline.
- Define acknowledgement meaning: accepted in memory, handed to the logger, written, or durable. These are different guarantees.
- Keep storage I/O and console output out of the acquisition critical section.
- Use versioned commands such as `GET_STATUS`, `SET_PERIOD`, and `STOP`, with explicit authorization for changes.
- Give every synchronous command a bounded waiting policy and unambiguous error reporting.
- Define how clients rediscover the logger after restart and handle uncertain outcomes.
- Define shutdown order: stop new acquisition, disarm timers, wake waiters, drain within a chosen bound, join workers, then release endpoints and storage.

### Test matrix

| Scenario | Expected behavior |
|---|---|
| Normal operation | Ordered accepted records; explain gaps if policy permits drops |
| Slow logger | Bounded memory and visible queue pressure; acquisition follows overload policy |
| Queue full | Documented action and correct drop/reject counters |
| Invalid command | Rejected without partial configuration change |
| Client exits | No permanent orphaned work or unbounded resource retention |
| Logger exits before reply | Client gets a bounded failure; uncertain outcome handled explicitly |
| Restart repeatedly fails | Retry rate and attempt count are bounded |
| Log storage full | Visible fault; acquisition and control do not hang indefinitely |
| Shutdown while queue is full | Producers and consumers unblock in the correct order |
| Long run | Stable resource counts and bounded memory under recorded load |

A practical lab target is a normal-load shutdown within one second and an explicitly bounded degraded shutdown when storage or IPC fails. Treat that as a requirement to measure on your setup, not an OS guarantee.

### Final deliverables

1. Architecture and ownership diagram.
2. Build/deploy/run instructions tied to an exact SDK and image.
3. Protocol specification with sizes, versions, and errors.
4. Tests with failure-injection results.
5. Timing report with load conditions and limitations.
6. Short review explaining remaining risks and what productionization would require.

### 22.1 A concrete reference design and its limits

Use three processes: acquisition/control, logger, and supervisor. Within acquisition/control, one thread handles intended sensor releases, one sends queued records to the logger, and one handles bounded status/control requests. The logger validates records and performs output. The supervisor observes readiness and useful progress from outside those processes.

This is a reference design for reasoning, not an assertion that three processes or these thread counts are optimal. It assumes simulated samples, a 10 ms normal period, a 64-byte conceptual record, and permission to drop records during overload. It does not connect to physical control outputs.

Choose a 64-slot acquisition queue for the example. Its conceptual payload capacity is 4096 bytes, excluding metadata, alignment, copies, and stacks. This accommodates the simplified pause examples with headroom, but the real workload and service guarantees must justify the final choice.

### 22.2 Define the record before choosing the transport

A record contains protocol version, acquisition-instance identity, sequence number, intended release timestamp, actual sample timestamp, and a temperature in explicitly defined units. Sequence numbering occurs at the stated acquisition event so a dropped record can be recognized as a gap.

The instance identity distinguishes a restarted acquisition service from an earlier service that used the same sequence values. Timestamps identify their monotonic domain and are not presented as globally comparable calendar time. The wire or shared-memory representation defines widths, sizes, and byte order where needed rather than relying on a conceptual 64-byte estimate.

A logger that receives a duplicate can identify it only if the duplicate policy defines identity and retention. A logger that restarts with no retained deduplication state must not claim restart-safe suppression.

### 22.3 Acquisition owns timing, not storage completion

At each intended release, acquisition obtains a sample, creates the record, and attempts bounded admission to its local queue. If the queue is full, the chosen teaching policy drops the new record and increments a synchronized drop counter. It does not wait indefinitely for space.

This requires an API different from section 9's blocking `push()`: a genuinely nonblocking or appropriately bounded admission design. A condition-variable timeout alone is not a bound on obtaining an ordinary mutex. The synchronization and scheduling analysis must account for the entire admission operation.

No storage output or synchronous logger call occurs on the acquisition path. The logger-client thread owns those waits. This separates the dependencies, but mutex contention, CPU interference, and memory effects still need timing verification.

### 22.4 Logger acknowledgment and accounting

For this example, define a logger success reply as "the complete record was accepted by the logger's configured output operation," not power-loss durability. If stronger persistence is required, change the milestone and validate its filesystem/device implementation.

Keep local states for queued, in-flight, acknowledged, definitely rejected, and uncertain records. At a coherent observation point after stopping acquisition, the number of produced records should be explainable by the disjoint accounting categories, including locally dropped records. Do not count an uncertain operation as both acknowledged and failed.

If the logger-client times out after sending, record an uncertain outcome unless the protocol supplies stronger evidence. The reference policy can drop that operation from automatic retry and report the uncertainty, avoiding blind duplication at the cost of possible loss. Another application can choose retries, but then it must define duplicate handling.

### 22.5 Control commands have separate completion rules

`GET_STATUS` returns a coherent snapshot including service generation, active configuration version, last completed sample, queue occupancy, loss, and uncertain-outcome counters. It should not wait for the storage device just to describe an outage.

`SET_PERIOD` validates permission and a complete candidate configuration. If applied at the next acquisition boundary, its reply must say accepted-for-application rather than already-active unless the handler waits for that activation acknowledgment. A version lets later status confirm which configuration actually became active.

`STOP` transitions admission into a stopping state and begins shutdown. A prompt "stop accepted" reply is different from a later "all accepted work drained" result. Define which one the caller receives and how completion is observed. Stop requests must remain serviceable under data overload.

### 22.6 Trace normal and degraded shutdown

Normal shutdown prevents new acquisition, stops future timer generation, allows producers to finish their permitted final actions, closes the queue, drains accepted work, joins workers, and then releases communication and storage resources. The status service remains usable as long as required to report progress.

Degraded shutdown has a finite drain deadline. Once it expires, classify remaining queued and in-flight work according to the loss/uncertainty policy. A worker stuck in an external operation cannot be made safe merely by destroying its connection or queue from another thread. The design needs supported cancellation/timeouts or a process-level supervisor escalation with understood consequences.

That is the value of the process boundary: the supervisor may stop a failed logger without destroying acquisition's memory. It does not magically undo effects or complete pending requests. Recovery must account for their actual state.

### 22.7 What would make this architecture unacceptable?

The design is unacceptable if every sample must be durable before the next 10 ms release but storage has no compatible bound. It is also unacceptable if silent gaps are forbidden and no component can retain the maximum outage backlog. Those are requirement/design conflicts, not minor implementation tasks.

It needs revision if acquisition misses its local deadline because logger-client work monopolizes an eligible CPU, if a broad shared lock couples acquisition to control processing, or if administrative commands can be starved behind telemetry. Measurement and dependency analysis must test these risks.

A final review should therefore report established properties and remaining limits separately. Demonstrating normal output alone cannot establish the overload, restart, authorization, or timing behavior that motivated the architecture.

## 23. Interview and Revision Questions

Try answering aloud before reading the checkpoint.

| Question | A strong short answer includes |
|---|---|
| What makes a system real-time? | Correct output within specified timing constraints, not just high speed |
| Why use a microkernel? | Small core mechanisms and process-based services; isolation and IPC trade-offs |
| What is the host/target split? | Cross-build/debug on host, execute target-compatible binaries on target |
| What is SEND-blocked versus REPLY-blocked? | Waiting for receipt versus waiting for completion after receipt |
| Can a pulse be replied to? | No; zero receive ID is not a message transaction |
| Does `MsgSend()` success mean data is durable? | Only if the application reply contract explicitly guarantees durability |
| Does a timeout cancel completed work? | No; outcome may be unknown and retries may duplicate effects |
| How are mutex and message priority inheritance different? | Resource owner boosting versus IPC/server scheduling mechanisms |
| Why is priority inheritance insufficient by itself? | It does not bound work, remove deadlock, or fix overload |
| Why not use shared memory everywhere? | Synchronization, lifecycle, validation, and recovery complexity |
| What is a resource manager? | A process implementing pathname-based connection and I/O behavior |
| Why does `cat` never finish reading your simulated device? | Check the intended stream/snapshot semantics, per-open offset, and EOF |
| How do you investigate a blocked client? | Thread state, server state, request lifecycle, and evidence from traces |
| Does a VM timing test prove board deadlines? | No; hardware, load, scheduling, and instrumentation differ |
| How do you make a service restartable? | Fresh endpoints, state recovery, bounded retries, and explicit in-flight outcomes |
| What makes this ready for a safety product? | Much more than successful labs: applicable certified components and safety lifecycle evidence |

### 23.1 Answer at beginner, implementer, and architect depth

**Question: why use separate processes?** A beginner answer is that processes normally have separate address spaces, while threads in one process share memory. An implementer adds explicit IPC, payload validation, and independent startup/shutdown. An architect connects the boundary to a requirement, such as independent logger restart or different privileges, and accounts for partial failure and shared-resource interference.

**Question: why use a condition variable?** A beginner answer is to wait without continually polling. An implementer explains the mutex, predicate, atomic release-and-wait behavior, reacquisition, and spurious wakeups. An architect asks whether the producer can make progress, whether shutdown wakes every wait, and whether the resulting blocking behavior fits the end-to-end deadline.

**Question: what makes a system real-time?** A beginner identifies correctness by a deadline. An implementer identifies release, start, finish, and relevant waits. An architect supplies the workload model, execution/blocking assumptions, admission limits, interference analysis, and validation evidence needed to justify the timing claim.

Depth is not the number of API names in the answer. It is the ability to connect a mechanism to a requirement and follow its limits.

### 23.2 A full answer about priority inversion

Priority inversion occurs when high-priority work waits on a resource owned by lower-priority work. On a single eligible CPU, medium-priority runnable work can delay the low-priority owner and thereby indirectly delay high. Supported priority inheritance raises the owner's effective priority through the dependency so it can finish the needed work ahead of that interference.

It does not resolve a circular wait, make a blocked device finish, or bound a critical section whose workload is unbounded. A complete answer also distinguishes mutex inheritance from QNX message-driven inheritance and does not assume `std::mutex` selects a particular real-time protocol.

The architect-level consequence is to minimize and bound dependency work, configure the intended protocol, and include remaining blocking in response-time analysis. Priority is one mechanism within that argument, not the whole argument.

### 23.3 A full answer about timeout and retries

A timeout reports that the applicable wait did not complete within its timing condition. The server may not have received the request, may be processing it, or may have performed its side effect without delivering the reply. Channel configuration can also affect how unblocking is handled.

A retry must therefore consider whether repeating the logical operation is safe. Idempotent operations simplify this, while non-idempotent operations may require a stable operation identity and deduplication whose state survives the required failures. Each attempt needs a bounded wait, and all attempts together need an overall budget.

An honest conclusion may be "outcome uncertain." Turning that into a success or a clean rejection without evidence is a correctness bug, not better error handling.

### 23.4 A full answer about shared memory

Shared memory maps common storage into more than one process. It can reduce copying for bulk data, but it supplies neither a portable queue nor a recovery protocol. The design needs representation, initialization publication, supported synchronization, ownership transitions, bounds checks, and reclamation after readers finish.

Offsets avoid assuming identical virtual addresses. Generations distinguish replacement instances. Neither removes the need to coordinate who may initialize or recreate the region. A failed owner can leave synchronization and data in inconsistent states, so recovery must be explicit.

An architect compares these costs with copying small messages. Shared memory is justified by the complete workload and failure model, not by declaring all copies wasteful.

## 24. Troubleshooting

| Symptom | First checks |
|---|---|
| `qcc` not found | Is the SDP installed on a supported host and its environment active? |
| Compiler license error | Correct license activation, entitlement, and license-server access; never bypass checks |
| QNX headers missing on Linux | Native examples need the SDK; installing generic Linux development headers is not enough |
| POSIX/QNX names hidden in strict C++ mode | Check `_QNX_SOURCE` and the matching compiler documentation |
| Executable will not start | Architecture, OS ABI, loader/runtime libraries, execute permission, image compatibility |
| Channel or connection permission error | Target abilities, endpoint visibility, and security policy |
| Client remains SEND-blocked | Does the intended channel still exist, and is any server receiving? |
| Client remains REPLY-blocked | Did every valid branch reply/error, or is work stalled after receipt? |
| `MsgReply()` fails after a delay | The client may have timed out or exited; record the error and lifetime |
| Queue shutdown hangs | Wake both producer and consumer waiters; inspect close predicates and join order |
| Periodic drift grows | Relative waits, accumulated work time, or an incorrect next-deadline calculation |
| Timing worsens when logging | Console/storage I/O, locks, allocation, or trace overhead on the critical path |
| Networking commands differ from a tutorial | Tutorial and image may use different stacks or SDP releases |
| Device read repeats forever | Missing snapshot EOF or incorrect per-open offset behavior |
| Restarted client cannot reconnect | Stale endpoint data, service not ready, permission issue, or stale connection lifetime |

For each symptom, gather one discriminating observation before making a change. For example, SEND versus REPLY blocking tells you which half of the transaction to investigate.

### 24.1 Diagnose in the layer where the failure occurs

Use this decision sequence when a new QNX program does not work:

1. **Did compilation finish?** If not, inspect declarations, headers, feature macros, and selected target tools. Runtime priority cannot repair a missing declaration.
2. **Did linking finish?** If not, inspect symbol definitions, library selection, and ABI compatibility. Copying the binary to a different target cannot repair a binary that was never linked.
3. **Did the loader start the program?** If not, inspect architecture, runtime dependencies, image contents, and permissions.
4. **Did initialization complete?** If not, identify the first failed operation and which earlier resources need cleanup.
5. **Did the operation finish?** If not, identify the waiting thread, its state, and the actor that can release it.
6. **Did it finish correctly and on time?** If not, separate protocol/data correctness from response-time causes.

This sequence is not a requirement to inspect every layer on every bug. Start at the layer supported by the symptom, then move only when evidence points elsewhere.

### 24.2 Worked case: permission denied during channel setup

Record which call failed and the error immediately associated with that failure. Confirm the service is running under its intended identity, with the expected endpoint visibility and applicable abilities/security policy. Compare that operation with the permission the design actually requires.

Do not grant all privileges to see whether the error disappears and call that the fix. Such a test would not establish an acceptable deployment policy. If the operation is required, arrange the specific authorized policy change through the environment's approved process. If it is not required, reconsider the endpoint design.

Separate connection failure from application authorization rejection. The former may prevent reaching the service at all; the latter can be the correct service behavior for an identified but unauthorized caller.

### 24.3 Worked case: queue depth grows but CPU is mostly idle

Low CPU use does not mean there is no bottleneck. The consumer may spend most of its time waiting for storage, another service, or a lock. Compare arrival rate with completed-service rate and inspect the consumer's wait dependency.

If storage latency dominates, adding producer threads makes backlog worse. Increasing queue capacity can absorb a bounded burst but cannot repair sustained throughput below arrival rate. Reducing input, changing the completion milestone, improving the output path, or applying the specified loss policy addresses the actual constraint.

Also check whether the monitoring code counts removed records as completed before downstream work finishes. A shallow queue can coexist with a large untracked in-flight backlog.

### 24.4 Worked case: the fix changes the symptom

Suppose adding debug prints makes a race disappear. The prints changed timing and may introduce internal synchronization or scheduling effects. That is evidence of sensitivity to execution ordering, not evidence that printing fixed the ownership protocol.

Preserve the failing scenario, inspect the intended happens-before relationships, and use appropriate race detection or controlled scheduling where supported. Avoid treating arbitrary delays as synchronization. The final repair should explain why the invalid interleaving is no longer permitted, not merely why it became harder to observe.

## 25. Official Reading Route

Use the documentation for your installed release. The following are QNX 8.0 entry points; API pages also identify required headers, libraries, return values, safety restrictions, and related calls.

1. [SDP 8.0 documentation](https://www.qnx.com/developers/docs/8.0/) and [Quickstart](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.qnxsdp.quickstart/topic/about.html): installation, host/target workflow, first program.
2. [System Architecture](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.sys_arch/topic/about.html): microkernel, scheduling, IPC, services.
3. [QNX OS Programmer's Guide](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.prog/topic/about.html): application development and operating-system behavior.
4. [qcc and q++ reference](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.utilities/topic/q/qcc.html): variants, standards, linking, environment.
5. [IPC architecture](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.sys_arch/topic/ipc.html), [synchronous messaging](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.sys_arch/topic/ipc_Sync_messaging.html), and [message priority inheritance](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.sys_arch/topic/ipc_Priority_inheritance_messages.html).
6. [C Library Reference](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/about.html): look up each call before using it.
7. [MsgSend](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/m/msgsend.html), [MsgReceive](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/m/msgreceive.html), [MsgReply](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/m/msgreply.html), and [TimerTimeout](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/t/timertimeout.html): native IPC lab contracts.
8. [Resource Managers](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.resmgr/topic/about.html): minimal examples, dispatch, read/write, OCBs, and controls.
9. [Utilities Reference](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.utilities/topic/about.html): `pidin`, `mkifs`, tracing, image inspection, and debugging tools.
10. [System Security Guide](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.security.system/topic/manual/about.html): abilities, policies, and deployment security.
11. [Migrating to QNX OS 8.0](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.qnxsdp.migration/topic/about.html): differences that make older tutorials misleading.
12. [QNX Toolkit for Visual Studio Code](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.qnxtoolkit.user_guide/topic/about_this_guide.html): supported editor build, deployment, and debugging workflows.

After the foundations, choose one specialization: BSP/driver development, performance and scheduling analysis, secure system integration, networking, or safety-oriented architecture. Hypervisor and safety-product work have separate documentation and prerequisites.

### 25.1 How to extract an API contract from a reference page

The explanations in this document teach the mechanism. A release-specific reference supplies exact details that source code depends on: signature, argument units, success return, error return, side effects, blocking states, permissions, library requirements, and restrictions on calling context.

For `TimerTimeout()`, extract that the timeout is thread-local and one-shot, its duration is expressed in nanoseconds through the documented argument, the flags select blocking states, and the non-`_r` form reports failure with `-1` and `errno`. Success is not a universal requirement that the return value equal zero; the documented result can describe previous timeout states.

That distinction explains why the native example checks `== -1` instead of `!= 0`. A mechanically copied "nonzero means error" rule from pthread functions would be wrong for this call. Learning the concept and checking the concrete contract are complementary steps.

### 25.2 Read the caveats as part of the behavior

A reference's example often demonstrates one path under selected assumptions. Its surrounding notes may describe permissions, cancellation, size limits, or flags that change the behavior. Omitting those notes can produce code that resembles the example but has a different lifecycle.

For timer events, the `timer_create()` contract says its event does not need registration. For other event-delivery paths, registration requirements can differ. Do not turn one API's statement into a general rule about every `sigevent` use.

When comparing QNX releases, check the migration guide and the matching API pages before carrying over flags or libraries. A familiar function name is not proof that every supported mode, security default, or toolchain behavior is unchanged.

### 25.3 Maintain a small contract record for critical interfaces

For a production-facing component, retain the release and interface version, required permission, accepted representation, blocking/cancellation semantics, and verified failure cases. Link that record to the source and tests that implement it.

This is not an invitation to duplicate the entire vendor manual. It records the subset your design relies on so a future SDK, image, or policy change can be reviewed against actual assumptions. For example, changing channel flags should trigger review of timeout and unblock handling because those are explicit dependencies of the protocol.

## 26. Completion Checklist

- [ ] I can explain real-time requirements without equating them with average speed.
- [ ] I can identify my host, SDK, target architecture, and running image.
- [ ] I have built and run a QNX-target executable.
- [ ] I can explain thread states, scheduling, and priority inversion.
- [ ] I can implement and test bounded queues and cooperative shutdown.
- [ ] I can explain `chid`, `coid`, and `rcvid` using a working transaction.
- [ ] I have tested malformed messages, timeouts, and peer exit.
- [ ] I can distinguish pulses from messages and clean up timers correctly.
- [ ] I can justify an IPC choice and define shared-memory ownership.
- [ ] I have implemented correct read/EOF behavior in a resource manager.
- [ ] I can explain the dependencies in my target's boot image.
- [ ] I can diagnose blocking using target evidence.
- [ ] I have measured timing under stated conditions without claiming an unproven bound.
- [ ] I have defined overload, shutdown, restart, and least-privilege behavior.
- [ ] I have completed the capstone or documented precisely what remains unverified.

### 26.1 Understanding, implementation, and architectural judgment

**Understanding** means you can predict a concrete trace and explain the mechanism in your own words. For example, you can explain why producer-first notification is safe when the protected predicate already records readiness.

**Implementation ability** means you can write and test the behavior with correct lifetimes, validation, error handling, and shutdown. You can identify which execution environment the result actually covers and reproduce a failure without relying on luck.

**Architectural judgment** means you can choose among mechanisms using requirements, budgets, dependency analysis, and failure semantics. You can explain why a separate logger process is justified, what a success reply promises, how overload is bounded, and what evidence still does not establish a deadline.

These are related but distinct achievements. Knowing a definition is not equivalent to verifying an implementation, and a successful small implementation is not equivalent to a production system argument.

### 26.2 An honest evidence summary

| Claim | Appropriate evidence | What remains outside the claim |
|---|---|---|
| I understand predicate waiting | Correct producer-first, consumer-first, and shutdown explanations | Native scheduling and timing |
| My queue preserves accepted items | Invariant argument and identity-based concurrency/closure tests | Persistence after process failure |
| My protocol handles target failures | Matching-QNX malformed-input, timeout, disconnect, and restart tests | All untested channel flags or SDK versions |
| My timing requirement is justified | Workload bounds, applicable analysis, target measurements, and stated assumptions | Behavior outside the permitted environment |
| My deployment is least privilege | Justified policy plus allowed/denied operation tests | Absence of every vulnerability |
| My system meets a safety requirement | Applicable lifecycle and system-level assurance evidence | Certification inferred solely from using QNX |

For this document, portable examples are Linux-tested and native target work remains pending. Reading the architect-level reasoning does not change that evidence boundary. It equips you to identify and obtain the missing evidence instead of overlooking it.

**Final self-check:** explain one normal request, one overloaded request, one timed-out request, and one request crossing a service restart. For each, identify data ownership, the completion milestone, maximum intended waits, and possible uncertainty. Connecting those four cases is a more meaningful sign of understanding than recalling an API list.