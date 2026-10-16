
# Kernel Thread

### Video :-

[![Youtube Video](https://img.youtube.com/vi/BBMvdqaV2F8/0.jpg)](https://www.youtube.com/watch?v=BBMvdqaV2F8)


## 1. What is a Process?

A process is a running instance of a program in user space.  
Each process has:
* Its own virtual memory space
* User-space stack and heap
* File descriptors
* Security context
* One or more threads

![](img/process.png)

Examples:    
* bash | firefox | Any application run in Linux

-----------------------------------

## 2. What is a Thread?

A `thread` is the smallest unit of execution inside a process.

Threads:

* Share the same memory space
* Share global variables
* Share file descriptors
* Have their own stack
* Have their own CPU context (registers)

<h4>Process with Multiple Threads</h4>

![](img/multiple_threads.png)

-----------------------

## 3. User-Space Threads vs Kernel Threads

### 3.1 User-Space Threads

User-space threads are threads created inside a user application.

Example:

* `pthread` in C
* `std::thread` in C++
* Threads in Java, Python, Rust

They:

* Run in user space
* Cannot directly access hardware
* Cannot access kernel memory
* Must use system calls to interact with the kernel

![](img/User-Space-Threads.png)

The kernel scheduler decides which thread runs, but execution happens in user mode.


### 3.2 Kernel Threads (kthread)

Kernel threads:

* Run entirely in kernel space
* Have no user-space memory
* Can directly access kernel APIs
* Can access hardware
* Can sleep and block

![](img/kernel_thread.png)


#### They are managed by the Linux scheduler just like processes.

![](img/cpu_sheduler.png)

## 4. Process vs Kernel Thread

| Feature                       | User Process   | Kernel Thread           |
| ----------------------------- | -------------- | ----------------------- |
| Runs in                       | User Space     | Kernel Space            |
| Has user memory               | Yes            | No                      |
| Can access hardware directly  | No             | Yes                     |
| Can call kernel APIs directly | No             | Yes                     |
| Created by                    | fork(), exec() | kthread_create()        |
| Can sleep                     | Yes            | Yes                     |
| Used for                      | Applications   | Driver background tasks |
| Overhead                      | high           | medium                  |
| Process is scheduled by       | operating system using process table | perating system using thread table  |
| can suspended?           | can            | can not                  |
| Its types are            | user process and system process | kernel level single thread and kernel level multi thread


## 5. Where Do Kernel Threads Live?

Kernel threads:

* Exist only in kernel space
* Have a `task_struct`
* Are visible in `ps` output
* Usually appear in brackets

Example:
```
[kworker/0:1]
[ksoftirqd/0]
[kthreadd]
```

You can see them using:
```bash
ps -ef
```

Or:
```bash
top
```

## 6. Important Concept: They Are Scheduled Like Processes

Even though they run in kernel space:

* They are scheduled by the same Linux scheduler
* They compete for CPU time
* They have priorities
* They can be preempted

So internally:

```
CPU Scheduler
     |
     +-- User Process Thread
     +-- User Process Thread
     +-- Kernel Thread
     +-- Kernel Thread
```

All are just schedulable entities.

## 7. Why Do Drivers Use Kernel Threads?

Device drivers use kernel threads when they need:

* Continuous background processing
* Periodic hardware polling
* Long-running kernel tasks
* Blocking operations
* Monitoring hardware state

Example use cases:

* Sensor data polling
* Firmware communication
* DMA buffer management
* Periodic health monitoring

## 8. Think Like this:

* A process is a container.
* A thread is a worker inside the container.
* A kernel thread is a worker that lives inside the kernel building, not in the user building.
```
User Building (User Space)
    -> Applications
    -> User Threads

Kernel Building (Kernel Space)
    -> Scheduler
    -> Drivers
    -> Kernel Threads
```

