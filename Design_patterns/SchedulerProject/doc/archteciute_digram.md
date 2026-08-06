# Task Scheduler Architecture Flow

```
                                  CLIENT
                                     │
                                     │
                submitTask(std::unique_ptr<ITask>)
                                     │
                                     ▼
                    +----------------------------------+
                    |         TaskScheduler            |
                    |----------------------------------|
                    | Responsibilities:                |
                    | • Accept new tasks              |
                    | • Validate input                |
                    | • Push tasks into queue         |
                    | • Start/stop worker threads     |
                    | • Coordinate shutdown           |
                    +----------------------------------+
                                     │
                                     │ enqueue(task)
                                     ▼
                    +----------------------------------+
                    |        ThreadSafeQueue           |
                    |----------------------------------|
                    | Responsibilities:                |
                    | • Thread-safe storage           |
                    | • Synchronize producers         |
                    | • Synchronize consumers         |
                    | • Block workers when empty      |
                    | • Wake workers on new tasks     |
                    +----------------------------------+
                                     ▲
                                     │
                       Producer       │        Consumer
                   (Client Threads)   │      (Worker Threads)
                                     │
                                     ▼
                    +----------------------------------+
                    |        Worker Thread(s)          |
                    |----------------------------------|
                    | Responsibilities:                |
                    | • Wait for available tasks      |
                    | • Pop one task from queue       |
                    | • Execute the task              |
                    | • Repeat until shutdown         |
                    +----------------------------------+
                                     │
                                     │
                                     ▼
                    +----------------------------------+
                    |          ITask Interface         |
                    |----------------------------------|
                    | virtual bool execute() = 0      |
                    | virtual string getTaskName()=0  |
                    +----------------------------------+
                                     ▲
                                     │
                ---------------------------------------------
                │                   │                      │
                │                   │                      │
                ▼                   ▼                      ▼
       +----------------+   +----------------+   +------------------+
       |   EmailTask    |   |    FileTask    |   |   DatabaseTask   |
       |----------------|   |----------------|   |------------------|
       | execute()      |   | execute()      |   | execute()        |
       | getTaskName()  |   | getTaskName()  |   | getTaskName()    |
       +----------------+   +----------------+   +------------------+
```

## Step-by-Step Execution Flow

### Step 1 – Client Creates a Task

The client creates a concrete task object.

```
auto task = std::make_unique<EmailTask>();
```

At this point, the scheduler does not know or care that it is an `EmailTask`.

---

### Step 2 – Client Submits the Task

The client submits the task to the scheduler.

```
scheduler.submitTask(std::move(task));
```

The scheduler becomes the owner of the task.

---

### Step 3 – Scheduler Receives the Task

The scheduler:

* Validates the task.
* Ensures the scheduler is still running.
* Pushes the task into the thread-safe queue.

The scheduler does **not** execute the task itself.

---

### Step 4 – ThreadSafeQueue Stores the Task

The queue safely stores the task while multiple client threads may be submitting tasks simultaneously.

Responsibilities:

* Prevent data races.
* Protect the queue using a mutex.
* Wake sleeping worker threads using a condition variable.

---

### Step 5 – Worker Thread Waits

Each worker thread waits until work becomes available.

```
while (running)
{
    auto task = queue.pop();

    if(task)
    {
        task->execute();
    }
}
```

If the queue is empty, the worker sleeps instead of consuming CPU.

---

### Step 6 – Worker Executes the Task

The worker only knows about the `ITask` interface.

```
task->execute();
```

It has no knowledge of whether the task is:

* EmailTask
* FileTask
* DatabaseTask
* ImageProcessingTask
* NetworkTask

This follows the **Dependency Inversion Principle (DIP)** and enables polymorphism.

---

### Step 7 – Concrete Task Performs the Work

Each concrete task implements its own business logic.

For example:

* `EmailTask` sends an email.
* `FileTask` copies a file.
* `DatabaseTask` updates a database.
* `CompressionTask` compresses files.

The worker thread does not need to change when new task types are added.

---

## Responsibilities of Each Component

### Client

* Creates concrete task objects.
* Submits tasks to the scheduler.
* Starts and stops the scheduler.

---

### TaskScheduler

* Owns the thread-safe queue.
* Owns the worker threads.
* Accepts new tasks.
* Coordinates startup and shutdown.
* Never performs the task's work directly.

---

### ThreadSafeQueue

* Stores pending tasks.
* Synchronizes producers and consumers.
* Prevents race conditions.
* Blocks consumers when the queue is empty.

---

### Worker Thread

* Waits for tasks.
* Removes one task from the queue.
* Executes it.
* Repeats until shutdown.

---

### ITask

Defines the common interface that every task must implement.

```
execute()
getTaskName()
```

---

### Concrete Tasks

Contain the actual business logic.

Examples include:

* EmailTask
* FileTask
* DatabaseTask
* BackupTask
* CompressionTask
* ImageProcessingTask

New task types can be added without modifying the scheduler or worker thread, making the design extensible and maintainable.
