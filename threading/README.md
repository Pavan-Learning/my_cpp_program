# C++ Threading Concepts and Examples

The folder already had lessons for `std::lock`, `std::promise` / `std::future`, and `std::async`. The 16 additions below fill the core portable threading gaps, from thread lifetime to C++20 coordination and a small thread pool.

Each new topic has all three companion files: a runnable C++ program, a self-contained numbered guide with explained examples and answers, and an SVG diagram. The guides include build commands and expected output. SVG images open directly in a browser and remain sharp when enlarged.

## 1. Concepts That Were Missing

The version column is the build standard used by the companion program, not necessarily the standard that first introduced every API it discusses.

| Order | Added concept and coverage | C++ | Program | Detailed guide | Image |
|---|---|---|---|---|---|
| 1 | Thread creation, argument passing, move ownership, join/detach, lifetime, worker exceptions | 17 | [std_thread.cpp](cpp_examples/std_thread.cpp) | [Thread](readme_files/Std_Thread_Guide.md) | [Diagram](images/std_thread.svg) |
| 2 | Data races, mutexes, critical sections, lock_guard, unique_lock, scoped_lock, recursive-lock caveats | 17 | [std_mutex.cpp](cpp_examples/std_mutex.cpp) | [Mutex and RAII](readme_files/Std_Mutex_Guide.md) | [Diagram](images/std_mutex.svg) |
| 3 | Timed locking, try operations, deadlines, failure handling | 17 | [std_timed_mutex.cpp](cpp_examples/std_timed_mutex.cpp) | [Timed mutex](readme_files/Std_Timed_Mutex_Guide.md) | [Diagram](images/std_timed_mutex.svg) |
| 4 | Condition variables, predicates, spurious wakeups, bounded producer/consumer queue, shutdown | 17 | [std_condition_variable.cpp](cpp_examples/std_condition_variable.cpp) | [Condition variable](readme_files/Std_Condition_Variable_Guide.md) | [Diagram](images/std_condition_variable.svg) |
| 5 | Atomics, read-modify-write, compare-exchange, logical races, lock-free caveats | 17 | [std_atomic.cpp](cpp_examples/std_atomic.cpp) | [Atomic](readme_files/Std_Atomic_Guide.md) | [Diagram](images/std_atomic.svg) |
| 6 | Happens-before, release/acquire publication, relaxed and sequentially consistent ordering | 17 | [memory_order.cpp](cpp_examples/memory_order.cpp) | [Memory ordering](readme_files/Memory_Order_Guide.md) | [Diagram](images/memory_order.svg) |
| 7 | Reader/writer locking, shared_lock, consistent snapshots, upgrade limitations | 17 | [std_shared_mutex.cpp](cpp_examples/std_shared_mutex.cpp) | [Shared mutex](readme_files/Std_Shared_Mutex_Guide.md) | [Diagram](images/std_shared_mutex.svg) |
| 8 | call_once, once_flag, initialization publication, exception-retry contract, local statics | 17 | [std_call_once.cpp](cpp_examples/std_call_once.cpp) | [Call once](readme_files/Std_Call_Once_Guide.md) | [Diagram](images/std_call_once.svg) |
| 9 | Thread-local storage, per-thread versus per-task state, reduction, false sharing | 17 | [thread_local.cpp](cpp_examples/thread_local.cpp) | [Thread local](readme_files/Thread_Local_Guide.md) | [Diagram](images/thread_local.svg) |
| 10 | packaged_task, shared_future, multiple readers, result ownership, exceptions | 17 | [std_packaged_task.cpp](cpp_examples/std_packaged_task.cpp) | [Packaged task](readme_files/Std_Packaged_Task_Guide.md) | [Diagram](images/std_packaged_task.svg) |
| 11 | jthread, stop_token, stop_source, stop_callback, cooperative cancellation | 20 | [std_jthread.cpp](cpp_examples/std_jthread.cpp) | [Jthread](readme_files/Std_Jthread_Guide.md) | [Diagram](images/std_jthread.svg) |
| 12 | Counting and binary semaphores, permits, resource limits, signaling | 20 | [std_semaphore.cpp](cpp_examples/std_semaphore.cpp) | [Semaphore](readme_files/Std_Semaphore_Guide.md) | [Diagram](images/std_semaphore.svg) |
| 13 | One-shot latch, event counting, result visibility, completion obligations | 20 | [std_latch.cpp](cpp_examples/std_latch.cpp) | [Latch](readme_files/Std_Latch_Guide.md) | [Diagram](images/std_latch.svg) |
| 14 | Reusable barrier, phases, completion callbacks, participant startup and departure | 20 | [std_barrier.cpp](cpp_examples/std_barrier.cpp) | [Barrier](readme_files/Std_Barrier_Guide.md) | [Diagram](images/std_barrier.svg) |
| 15 | Atomic wait/notify, value-based blocking, publication, missed transient values | 20 | [std_atomic_wait.cpp](cpp_examples/std_atomic_wait.cpp) | [Atomic wait](readme_files/Std_Atomic_Wait_Guide.md) | [Diagram](images/std_atomic_wait.svg) |
| 16 | Thread pool, task queue, futures, draining shutdown, deadlock, starvation, debugging | 20 | [thread_pool.cpp](cpp_examples/thread_pool.cpp) | [Thread pool](readme_files/Thread_Pool_Guide.md) | [Diagram](images/thread_pool.svg) |

This is a core C++ threading collection, not a claim to cover every concurrency specialty. Platform-specific priorities, affinity, real-time scheduling, interprocess synchronization, advanced lock-free memory reclamation, parallel algorithms, and asynchronous I/O/coroutine frameworks are separate subjects. ABA, false sharing, progress limitations, and sanitizer-based diagnosis are introduced where relevant without pretending to implement production-grade solutions for them.

## 2. Existing Lessons

| Concept | Program | Guide | Image |
|---|---|---|---|
| Multi-mutex deadlock avoidance | [std_lock.cpp](cpp_examples/std_lock.cpp) | [Std_Lock_Guide.md](readme_files/Std_Lock_Guide.md) | [std_lock.png](images/std_lock.png) |
| Promise and future | [std_promise_future.cpp](cpp_examples/std_promise_future.cpp) | [Std_Promise_Future_Guide.md](readme_files/Std_Promise_Future_Guide.md) | [std_promise_future.svg](images/std_promise_future.svg) |
| Async launch policies and results | [std_async.cpp](cpp_examples/std_async.cpp) | [Std_Async_Guide.md](readme_files/Std_Async_Guide.md) | [std_async.svg](images/std_async.svg) |

The existing lock source intentionally retains its demonstration toggle and limitations. Its guide explains those limitations and supplies a corrected complete transfer example. The bulk checks below target the new examples, not that older demonstration.

## 3. How the Pieces Fit

Start with thread lifetime and mutex ownership before condition variables. Read the existing `std::lock` guide after the RAII lesson. Read the existing promise/future and async guides before packaged tasks. Finish with memory ordering and the C++20 primitives, then the pool that combines queue ownership, waiting, futures, and shutdown.

| Need | Starting choice |
|---|---|
| Protect a multi-field invariant | Mutex with a named RAII guard |
| Wait for a queue or another compound predicate | Condition variable with a mutex-protected predicate |
| Update an independent numeric value | Atomic read-modify-write |
| Publish immutable data once | Release/acquire protocol or another established synchronization mechanism |
| Read a shared snapshot frequently | Consider shared_mutex; measure against a plain mutex |
| Initialize once | Function-local static or call_once |
| Return one task result or error | Future from promise, async, or packaged_task |
| Share one result with several readers | Separate copies of shared_future |
| Own a cancellable worker | Jthread with a cooperative stop protocol |
| Limit concurrent access to a resource group | Counting semaphore plus safe resource selection |
| Wait for one group of completion events | Latch |
| Coordinate repeated rounds | Barrier |
| Wait for one atomic value to change | Atomic wait plus store/notify protocol |
| Reuse workers for queued tasks | A suitable executor or a carefully bounded teaching pool |

`sleep_for()` and `yield()` do not synchronize shared data. `volatile` does not make ordinary shared memory thread-safe. More threads, atomics, and weaker memory orders are not automatic performance improvements.

## 4. Build and Run All New Examples

Run these commands from the repository root. Each program has its own `main()`, so build separate executables. These commands leave binaries outside the repository and do not require changes to the root Makefile.

C++17 examples:

```sh
mkdir -p /tmp/cpp-threading
for name in std_thread std_mutex std_timed_mutex std_condition_variable std_atomic memory_order std_shared_mutex std_call_once thread_local std_packaged_task; do
    g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -pthread "threading/cpp_examples/$name.cpp" -o "/tmp/cpp-threading/$name" || exit 1
    timeout 10 "/tmp/cpp-threading/$name" || exit 1
done
```

C++20 examples:

```sh
for name in std_jthread std_semaphore std_latch std_barrier std_atomic_wait thread_pool; do
    g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread "threading/cpp_examples/$name.cpp" -o "/tmp/cpp-threading/$name" || exit 1
    timeout 10 "/tmp/cpp-threading/$name" || exit 1
done
```

`-pthread` supplies the compiler/linker threading option. C++20 examples require both compiler language support and the corresponding standard-library implementations. The shell checks use the `timeout` utility available in this Linux container; run the binary directly on platforms without it. A timeout catches a hang but is not a proof of deadlock or a performance benchmark.

Each new executable returns `0` only when its documented result checks pass. Outputs are printed by main after the relevant synchronization, so they do not depend on worker print interleaving. Repeated passes do not prove every possible interleaving safe: the guides explain ownership and synchronization arguments too.

## 5. Toolchain Notes

The examples were developed against GCC/libstdc++ 13.2.1 in this Alpine Linux container. The [call_once guide](readme_files/Std_Call_Once_Guide.md) documents an observed exception-path limitation: an injected exception escaping an initializer terminated rather than reaching the caller's catch. The runnable example checks successful once-only publication; retry semantics are explained separately and should be validated on the deployment toolchain.

The [thread-pool guide](readme_files/Thread_Pool_Guide.md) explains optional ThreadSanitizer checks on supported platforms. Sanitizer availability and support are not assumed here. The new examples avoid deliberately executing undefined behavior or an intentional deadlock.