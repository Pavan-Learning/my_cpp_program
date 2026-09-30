# C++ std::async

**Give the library a function. Receive its return value or exception through a future.**

![A visual guide to C++ std::async](std_async.svg)

Open [the infographic](std_async.svg) for a larger view.

This lesson continues [std::promise and std::future](Std_Promise_Future_Guide.md), using the same bank-transfer calculation. The runnable example is [std_async.cpp](std_async.cpp).

## 1. Why do we need std::async?

In the promise/future example, we explicitly created a result channel and a worker:

```cpp
std::promise<TransferResult> result_promise;
std::future<TransferResult> result_future = result_promise.get_future();
std::thread worker(calculate_transfer, from, to, amount, std::move(result_promise));
```

The worker had to call `set_value()` on success, catch exceptions and call `set_exception()` on failure, and main had to arrange `join()` even if receiving the result failed.

That control is useful when the producer decides independently when to publish a result. But what if the requirement is simply **run this function and give me its result when it finishes**?

```cpp
std::future<TransferResult> result_future =
    std::async(std::launch::async, calculate_transfer, from, to, amount);

TransferResult result = result_future.get();
```

Here `calculate_transfer` is an ordinary function returning `TransferResult`. There is no promise parameter. It returns normally or throws normally; the library stores the outcome for the future.

| Responsibility | std::thread + promise/future | std::async |
|---|---|---|
| Arrange execution | Construct a thread | Call `async()` with a launch policy |
| Supply a successful result | `promise.set_value(result)` | Return from the callable |
| Supply a failure | Catch and call `set_exception()` | Let the exception leave the callable |
| Receive the outcome | `future.get()` | `future.get()` |
| Handle execution lifetime | Join the thread explicitly or with RAII | Async shared-state lifetime rules apply |
| Choose publication time | Producer chooses when to publish | Outcome comes from callable completion |

`std::async` is not automatically faster than a direct function call. Starting a thread, storing a result, and synchronizing have costs. Use it when independent work can overlap or when asynchronous result delivery is useful.

## 2. The smallest complete example

```cpp
#include <future>
#include <iostream>

int calculate_answer()
{
    return 42;
}

int main()
{
    std::future<int> answer_future =
        std::async(std::launch::async, calculate_answer);

    std::cout << "Main can do other work here.\n";
    int answer = answer_future.get();
    std::cout << "Answer: " << answer << '\n';
}
```

Output:

```text
Main can do other work here.
Answer: 42
```

Step by step:

1. `<future>` declares `std::async`, `std::future`, and the launch policies.
2. `std::launch::async` requests execution on a separate thread.
3. `calculate_answer` passes the function to the library. Writing `calculate_answer()` instead would call it immediately and pass its result, which is not the intended callable.
4. `std::async` returns a `std::future<int>` connected to the function's eventual outcome.
5. The worker may finish before or after main prints its first line. Neither order requires a sleep.
6. `get()` waits if necessary, returns `42`, and consumes the future's shared-state association.

You do not call `join()` on a future. There is no user-owned `std::thread` object here.

## 3. Follow execution and the shared state

```text
Main / consumer                 Shared state                Async worker
---------------                 ------------                ------------
async(launch::async, function) -> pending ------------------> invoke function
receives future
does independent work
future.get() ------------------> waits if needed
                                value or exception <------- return or throw
receives value / exception <----- ready
future is now invalid
```

The future is a handle to the result channel, not the calculation itself. The library manages the shared state and associated async execution.

Successful waiting establishes the synchronization needed to observe the outcome and the task's prior writes. For an async-policy task, the waiting operation that observes readiness also waits for its associated thread to complete.

**This does not make arbitrary shared objects thread-safe.** Main must not access an object concurrently with a worker modifying it unless that access has its own synchronization.

## 4. Launch policies: when and where does the function run?

| Call | Behavior | Does it guarantee a separate thread? |
|---|---|---|
| `std::async(std::launch::async, task)` | Execute asynchronously | Yes, if launching succeeds |
| `std::async(std::launch::deferred, task)` | Save the call for a later non-timed wait | No |
| `std::async(task)` | Library chooses a permitted policy; portable code must allow async or deferred | No |
| `std::async(std::launch::async \| std::launch::deferred, task)` | Library chooses async or deferred | No |

### 4.1. Explicit async

```cpp
auto result = std::async(std::launch::async, [] { return 42; });
```

The function can overlap with the caller's work. It is not guaranteed to begin before the next statement, and it may already be finished when the caller first checks the future.

Thread creation can fail. With an async-only policy, failure to start a thread is reported by `std::system_error`; resource allocation can also throw. Such launch failures occur at the `std::async` call, not later inside `get()`.

### 4.2. Deferred execution

```cpp
auto result = std::async(std::launch::deferred, [] { return 42; });

auto status = result.wait_for(std::chrono::milliseconds(0));
// status is std::future_status::deferred; the function has not run.

int answer = result.get();
```

Use `<chrono>` for the duration. The first non-timed wait, such as `wait()` or `get()`, executes the saved function in the thread performing that wait. In this example, `get()` runs the function on main's thread.

`wait_for()` and `wait_until()` do not start a deferred function. If nobody performs a non-timed wait and the future is discarded, the deferred function never runs.

Deferred execution is lazy evaluation, not background execution. The arguments are still evaluated and stored when `std::async` is called; only invocation of the saved callable is deferred.

### 4.3. Why specify the policy in teaching examples?

The name `async` can suggest that every call starts background work, but the default policy does not guarantee that. Use `std::launch::async` when the example depends on concurrency. Use `std::launch::deferred` when it is intentionally demonstrating laziness.

Do not depend on the default-policy choice being the same across implementations or runs.

## 5. Bank transfer: ordinary return, asynchronous delivery

The companion program uses these data types:

```cpp
struct Account
{
    std::string name;
    int balance;
};

struct TransferResult
{
    int from_balance;
    int to_balance;
};
```

Pavan starts with `50000`, and Sagar starts with `60000`. A transfer of `500` should produce `49500` and `60500`.

The worker calculates using copies:

```cpp
TransferResult calculate_transfer(Account from, Account to, int amount)
{
    if (amount <= 0)
    {
        throw std::invalid_argument("Transfer amount must be positive");
    }
    if (from.balance < amount)
    {
        throw std::runtime_error("Insufficient balance in " + from.name + "'s account");
    }

    return {from.balance - amount, to.balance + amount};
}
```

Main launches the calculation and applies only a successful result:

```cpp
auto result_future =
    std::async(std::launch::async, calculate_transfer, from, to, amount);

TransferResult result = result_future.get();
from.balance = result.from_balance;
to.balance = result.to_balance;
```

The worker never changes the original accounts. Only main writes them, after receiving both calculated balances. No mutex is needed for these copies and sequential updates.

The runnable program makes three requests in sequence:

| Request | Outcome | Balances afterward |
|---|---|---|
| Transfer `500` | Return a `TransferResult`; main applies it | `49500`, `60500` |
| Transfer `100000` | Throw insufficient-balance exception | Unchanged |
| Transfer `0` | Throw invalid-amount exception | Unchanged |

This is a result-delivery lesson, not a production banking system. It uses small integer amounts and omits overflow handling, persistence, and transaction isolation. Launching multiple transfers from the same old balance snapshots and applying them later could lose updates, even if all writes happen on main.

## 6. Exceptions: throw in the task, catch at get()

```cpp
auto result = std::async(std::launch::async, []() -> int
{
    throw std::runtime_error("Calculation failed");
});

try
{
    int answer = result.get();
    std::cout << answer << '\n';
}
catch (const std::exception& error)
{
    std::cout << error.what() << '\n';
}
```

With `<future>`, `<iostream>`, and `<stdexcept>` included, this fragment prints `Calculation failed`.

The library catches the exception leaving the callable and stores it in the shared state. `get()` rethrows it in the receiving thread. This works for deferred tasks too.

Compare that with an ordinary `std::thread`: an uncaught exception escaping its thread function calls `std::terminate()`. Merely surrounding the thread construction with a `try` block in main does not catch worker exceptions.

`wait()` only waits; it does not rethrow the stored task exception. Destroying the future also does not report that exception to your error-handling code. Call `get()` to observe the outcome.

For an ordinary valid future, `get()` consumes its state association even when it rethrows the task's stored exception. The companion program checks that `valid()` is false after both success and failure.

## 7. Waiting is not retrieving

| Operation | Waits? | Returns the stored value? | Consumes an ordinary future? |
|---|---|---|---|
| `valid()` | No | No; reports whether a state is associated | No |
| `wait()` | Until ready; starts deferred work if needed | No | No |
| `wait_for(duration)` | Up to a relative wait, subject to scheduling delays | No; returns a status | No |
| `wait_until(deadline)` | Until ready or a deadline, subject to scheduling delays | No; returns a status | No |
| `get()` | Until ready; starts deferred work if needed | Yes, or rethrows the task exception | Yes |

`valid()` does not mean "the answer is ready." A pending future can be valid.

### 7.1. Handle all three timed-wait statuses

```cpp
auto result = std::async([] { return 42; });
auto status = result.wait_for(std::chrono::milliseconds(10));

if (status == std::future_status::ready)
{
    std::cout << "Outcome available.\n";
}
else if (status == std::future_status::timeout)
{
    std::cout << "Still running; main can do independent work.\n";
}
else
{
    std::cout << "Deferred; get() will execute the task here.\n";
}

std::cout << result.get() << '\n';
```

A `ready` outcome may contain an exception rather than a value. A `timeout` does not cancel the task. A `deferred` status means no background calculation has started through that deferred state.

A loop that repeatedly calls `wait_for()` until it returns `ready` can loop forever for a deferred task. Handle `deferred` or explicitly request `std::launch::async`.

The runnable example checks deferred status without sleeps. It also calls `wait()` on an async future, checks that it remains valid, and then receives `42` with `get()`.

## 8. Arguments, captures, and object lifetimes

### 8.1. Arguments are normally stored by value

In C++17, `std::async` decay-copies or moves the callable and arguments into its internal storage. Plain lvalue arguments are copied; `std::move()` enables moving appropriate objects.

```cpp
int balance = 50000;
auto result = std::async(std::launch::async,
    [](int snapshot) { return snapshot - 500; }, balance);
balance = 100;
std::cout << result.get() << '\n';
```

This prints `49500`. The worker calculates using the stored copy of `50000`, not main's later `100`.

### 8.2. References require deliberate lifetime management

Use `std::ref()` from `<functional>` when the callable must receive the original object:

```cpp
int balance = 50000;
auto completion = std::async(std::launch::async,
    [](int& original) { original -= 500; }, std::ref(balance));
completion.get();
std::cout << balance << '\n';
```

This prints `49500`. Main does not touch `balance` until `get()` completes, and `balance` remains alive throughout the task. Those facts make this particular example safe without a mutex.

`std::ref()` does not add synchronization. A simultaneous read in main and write in the worker would be a data race without additional protection.

Lambda captures follow the same ownership question: `[balance]` captures a value, while `[&balance]` keeps a reference. Copying a lambda with reference captures does not copy the referenced objects. Capturing `this` likewise does not keep the owning object alive.

### 8.3. Move-only inputs and outputs work

```cpp
auto input = std::make_unique<int>(42);
auto result = std::async(std::launch::async,
    [](std::unique_ptr<int> value) { return value; }, std::move(input));
auto output = result.get();
```

Include `<memory>` and `<utility>` as well as `<future>`. Ownership moves into the task, and then the returned pointer moves out through the future. Do not dereference the moved-from `input`.

The future itself is also movable, not copyable. Use `future.share()` to obtain a `std::shared_future` when several consumers need the same completed outcome. Sharing the future does not repeat the computation.

## 9. The important surprise: destruction can wait

A future associated with an async-policy `std::async` call has special lifetime behavior: releasing the last reference to its shared state can wait for the associated thread to finish. This commonly happens when a local future is destroyed without first waiting or calling `get()`.

```cpp
{
    auto result = std::async(std::launch::async, [] { return 42; });
}
```

The task is not detached at the closing brace. The outcome is discarded, and leaving this scope may wait for the task to finish. Future destruction does not rethrow a stored task exception.

This is not a rule that every future destructor waits. Futures from ordinary promises or packaged tasks do not acquire this async-thread joining behavior merely because their result is pending. Discarding an unstarted deferred task does not execute it.

### 9.1. Discarded temporaries can serialize work

Assume `first_task` and `second_task` are ordinary callable functions:

```cpp
std::async(std::launch::async, first_task);
std::async(std::launch::async, second_task);
```

Each temporary future is destroyed at the end of its statement. The first statement waits for its task before the second task is launched. Compilers may also warn about the discarded result.

Keep both futures to allow overlap:

```cpp
auto first_result = std::async(std::launch::async, first_task);
auto second_result = std::async(std::launch::async, second_task);

first_result.get();
second_result.get();
```

Launch both before waiting for either. Immediately calling `get()` after every launch also prevents those tasks from overlapping with one another.

### 9.2. Waiting while holding a needed mutex can deadlock

Suppose main holds a mutex, launches a task that needs that mutex, and then waits on the future before releasing the mutex. Main waits for the worker, while the worker waits for main's mutex.

The same problem can happen through implicit waiting in a future destructor. Review destruction order and exception paths, not only visible `get()` calls. Arrange for main to release any mutex needed by the task before waiting or releasing the last async-state reference.

## 10. What std::async does not provide

| Requirement | What to know |
|---|---|
| Cancel a running task | There is no `future.cancel()`; cancellation requires a cooperative protocol |
| Hard completion deadline | A timed wait only bounds that wait approximately; the task continues, and cleanup may wait |
| Detach and forget a task | Discarding an async future may wait; it is not a detach API |
| Control a fixed-size worker pool | `std::async` exposes no portable pool-size or queue controls |
| Guarantee a speedup | Tiny tasks can cost more to launch than to compute |
| Make shared account updates safe | Use ownership, locking, or another appropriate synchronization design |
| Deliver a stream of results | One future represents one outcome; use a queue/channel for repeated messages |

Launching thousands of async-only tasks is not a substitute for a bounded executor. When the work count is large, consider a task system with explicit resource limits.

For work with no return value, `std::async` returns `std::future<void>`. Its `get()` waits and reports failure, but returns no value.

## 11. Choose the tool by the job

| Tool | Useful when |
|---|---|
| Direct function call | No concurrency or delayed evaluation is needed |
| `std::thread` | You want explicit control of a thread and its lifetime |
| `std::promise` + `std::future` | A producer explicitly supplies a value or exception, possibly from a callback |
| `std::async` | A function's return value or exception is the eventual result |
| `std::packaged_task` | You want a callable connected to a future, but execution is scheduled separately |
| `std::shared_future` | Multiple consumers need access to one completed outcome |

`std::packaged_task` does not start a thread by itself. `std::async` combines invocation policy and future-based result delivery.

## 12. Build, run, and read the output

From the workspace root, using a C++17-capable compiler:

```sh
mkdir -p out
g++ -std=c++17 -Wall -Wextra -pthread threading/std_async.cpp -o out/std_async
./out/std_async
```

Expected output:

```text
Request: Pavan -> Sagar, amount = 500
Transfer applied by main.
Future valid after get(): false
Pavan: 49500, Sagar: 60500

Request: Pavan -> Sagar, amount = 100000
Transfer failed: Insufficient balance in Pavan's account
Future valid after get(): false
Pavan: 49500, Sagar: 60500

Request: Pavan -> Sagar, amount = 0
Transfer failed: Transfer amount must be positive
Future valid after get(): false
Pavan: 49500, Sagar: 60500

Explicit async used a separate thread: true
Timed wait left deferred task unstarted: true
Deferred get() ran on the calling thread: true

Future valid after wait(): true
Answer from get(): 42
Future valid after get(): false

All checks passed: true
```

All printing happens on main, so worker output cannot interleave. The checks use results and thread identities, not timing assumptions. The program exits with `0` on success and `1` if a check fails or an unexpected exception occurs.

In the debugger, place breakpoints in `calculate_transfer`, at `result_future.get()`, and inside the deferred lambda. The async calculation runs on a different thread; the deferred lambda runs on main when main calls `get()`.

## 13. Check your understanding

### Q1. Does std::async always start a new thread?

**Answer:** No. Explicit `std::launch::async` requests a separate thread and can fail to launch. Explicit `deferred` delays invocation until a non-timed wait. The default policy does not guarantee background execution.

### Q2. Why does the async worker not call set_value()?

**Answer:** Its normal return value becomes the future's result. The library handles publication, including storing an exception if the callable throws.

### Q3. Does wait() consume the future or throw its stored task exception?

**Answer:** Neither. It waits for readiness, starting deferred work if necessary. The future remains valid, and `get()` is still needed to retrieve the value or rethrow the stored exception.

### Q4. What happens if get() is called twice?

**Answer:** After the first call on an ordinary future, it has no associated state. Do not call `get()` again on that invalid future; that violates the API precondition. Some implementations diagnose `no_state`, but portable code must not rely on that diagnosis.

### Q5. Why can polling a deferred future never finish?

**Answer:** Timed waits return `deferred` without invoking the function. Repeating them does not make progress. A non-timed wait such as `get()` must execute the deferred task.

### Q6. If wait_for() times out, has the task stopped?

**Answer:** No. Only the caller's wait ended. The task continues, and destroying the last future associated with async execution may still wait for completion.

### Q7. Can main catch a worker exception even if it was thrown before get()?

**Answer:** Yes. The exception is stored in the shared state until `get()` retrieves and rethrows it. Consumer timing does not lose the outcome.

### Q8. Does passing std::ref(account) make account thread-safe?

**Answer:** No. It passes access to the original account rather than a copy. You must keep the account alive and prevent unsynchronized conflicting access.

### Q9. Why might two async calls run sequentially?

**Answer:** Discarding each returned future can wait at the end of each statement. Calling `get()` immediately after each launch has a similar effect. Save both futures before waiting to allow overlap.

### Q10. What happens to a deferred task that is never waited on?

**Answer:** It is never invoked. Destroying its future discards the saved computation. Do not use deferred execution for required side effects unless you ensure it is executed.

### Q11. Is std::async always preferable to promise/future?

**Answer:** No. Async fits a callable whose completion supplies the outcome. A promise fits an explicit producer that may publish from an event handler or before completing its other work. Choose based on who controls execution and publication.