# Operating System Concepts: From Foundations to Design Decisions

Detailed, concept-by-concept lessons covering hardware, processes, threads, synchronization, system calls, memory, files, I/O, scheduling, permissions, and isolation. Each topic includes explanations, examples, or worked questions, progressing from beginner understanding toward architectural reasoning.

**Starting point:** basic C++ variables, functions, and compiling a small program. No previous OS or QNX knowledge is required. The examples run on Linux without a QNX license.

**Validation status:** the three complete programs (first thread, protected counter, and process lifecycle) were compiled with C++17 and warnings treated as errors, then run successfully on Linux. The process-lifecycle example also requires POSIX facilities and `/bin/sh`. Linux results do not establish native QNX behavior.

These lessons were separated from the [QNX learning guide](QNX_Learning_Guide.md). The original 0.1-0.38 numbering is retained for existing lesson references. Continue with that guide for QNX architecture, native IPC, resource managers, and real-time application design.

## Contents

- [0.1 Hardware: CPU, RAM, and storage](#01-what-is-the-computer-actually-doing)
- [0.2 Why an operating system exists](#02-why-do-we-need-an-operating-system)
- [0.3 Programs, processes, and threads](#03-program-process-and-thread-three-different-things)
- [0.4 Shared thread memory](#04-what-memory-do-threads-share)
- [0.5 First thread example](#05-first-working-example-start-a-thread-and-join-it)
- [0.6 Threads on one CPU core](#06-how-can-two-threads-run-on-one-cpu-core)
- [0.7 Shared data and coordination](#07-why-shared-data-needs-coordination)
- [0.8 Protected counter example](#08-second-working-example-protect-a-shared-counter)
- [0.9 Waiting for data](#09-how-does-one-thread-wait-for-another-to-produce-data)
- [0.10 Shutdown and deadlock](#010-shutdown-and-deadlock-explained-separately)
- [0.11 Connection to QNX](#011-where-qnx-fits-into-this-picture)
- [0.12 Beginner exercises and answers](#012-beginner-exercises-and-answers)
- [0.13 From source code to execution](#013-from-a-c-statement-to-an-executing-thread)
- [0.14 Memory visibility](#014-visibility-means-more-than-the-bytes-existing-in-ram)
- [0.15 Five concurrent-design questions](#015-separate-the-five-questions-in-a-concurrent-design)
- [0.16 User mode and kernel mode](#016-user-mode-kernel-mode-and-protection)
- [0.17 Functions and system calls](#017-a-function-call-is-not-necessarily-a-system-call)
- [0.18 Interrupts, exceptions, and signals](#018-interrupts-exceptions-and-signals)
- [0.19 Blocked threads and scheduling](#019-follow-a-blocked-thread-through-the-scheduler)
- [0.20 Process creation and termination](#020-how-a-process-starts-and-ends)
- [0.21 Waiting, zombies, and orphans](#021-exit-wait-zombies-and-orphaned-children)
- [0.22 Process lifecycle example](#022-complete-linux-example-start-and-reap-a-child)
- [0.23 Address spaces](#023-a-process-address-space-is-a-map)
- [0.24 Pages and address translation](#024-pages-page-tables-and-address-translation)
- [0.25 TLB misses and page faults](#025-tlb-misses-and-page-faults-are-different)
- [0.26 Allocators and fragmentation](#026-allocators-stack-space-and-fragmentation)
- [0.27 Paging and memory pressure](#027-demand-paging-copy-on-write-swapping-and-memory-pressure)
- [0.28 Paths and descriptors](#028-pathnames-descriptors-and-open-file-descriptions)
- [0.29 Filesystems and persistence](#029-filesystems-names-metadata-and-persistence)
- [0.30 I/O modes](#030-blocking-nonblocking-and-asynchronous-io)
- [0.31 Pipes, backpressure, and EOF](#031-pipes-explain-byte-streams-backpressure-and-eof)
- [0.32 An input byte through the OS](#032-follow-one-input-byte-through-the-os)
- [0.33 Scheduling algorithms](#033-scheduling-algorithms-optimize-different-goals)
- [0.34 Deadlock conditions](#034-the-four-deadlock-conditions-and-what-they-mean)
- [0.35 IPC choices](#035-choose-ipc-by-the-communication-contract)
- [0.36 Permissions and resource limits](#036-users-permissions-and-resource-limits)
- [0.37 Containers and virtual machines](#037-containers-virtual-machines-and-the-host-kernel)
- [0.38 Worked OS questions](#038-worked-os-questions-that-connect-the-mechanisms)

## 0. Start from Zero: OS and Threading

Read these lessons in order. The same sensor-and-logger example connects the ideas and continues in the companion [QNX learning guide](QNX_Learning_Guide.md).

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

**Priority** concerns which eligible thread should run first; it does not remove a deadlock. **Priority inversion** is a different problem: important work waits on a resource held by lower-priority work, which can itself be delayed. Read the [QNX guide's detailed scheduling section](QNX_Learning_Guide.md#7-real-time-scheduling) after you understand basic waiting and ownership.

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

The following subsections explain execution and the OS mechanisms underneath these examples. Sections 0.16 onward develop the OS foundations before the companion QNX guide. Consult the [QNX guide's vocabulary](QNX_Learning_Guide.md#3-vocabulary) when a term is unfamiliar; it is a reference, not a prerequisite vocabulary test.

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

This distinction is the first step toward architectural reasoning: replace "the code is safe" with a precise statement about which property is established, under which assumptions. The [QNX guide](QNX_Learning_Guide.md) applies these same five questions to processes, devices, and service recovery.

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

An interrupt handler must not assume the same environment as an ordinary application function. Its allowed operations, blocking behavior, and work budget depend on the attachment model. Long work is often deferred to an appropriate thread; the [QNX guide's driver chapter](QNX_Learning_Guide.md#16-boot-bsps-and-drivers) explains why acknowledgment and device ownership matter.

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

Separate descriptors do not therefore guarantee separate offsets. Conversely, opening the same resource twice does not mean both operations share one cursor. This distinction explains many [resource-manager tests in the QNX guide](QNX_Learning_Guide.md#14-resource-managers).

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

This trace joins the earlier concepts: a controlled OS request crosses a protection boundary, a blocked thread leaves the runnable set, a device event changes state, scheduling resumes work, and a descriptor identifies the opened resource. The companion [QNX guide](QNX_Learning_Guide.md) explains how channels, pulses, and resource managers implement specific parts of this broader OS story.

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

**Earliest Deadline First (EDF)** chooses by the earliest current absolute deadline in its idealized scheduling model. It is useful as scheduling theory, not a claim that a particular QNX configuration provides that policy. For QNX's actual supported policies and their semantics, use the [QNX scheduling chapter](QNX_Learning_Guide.md#7-real-time-scheduling) and the matching release documentation.

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

QNX abilities and policies supply additional operation-specific controls discussed in the [QNX security chapter](QNX_Learning_Guide.md#19-security-reliability-and-safety). Traditional owner/group permissions are a useful foundation, not the full QNX security model.

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

