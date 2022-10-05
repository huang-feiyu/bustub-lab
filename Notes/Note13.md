# Query Execution - Part II

[TOC]

```diff
+ Query Planning <- Today
- Operator Execution
- Access Methods
- Buffer Pool Manager
- Disk Manager <- Done
```

Parallel Execution: Execute query with multiple workers => Better performance, responsiveness and availability

Parallel vs. Distributed

* High level: database is spread out across multiple resources. (SQL do not care how data stored)
* Parallel: cores
* Distributed: machines

### Process Model

**process model**: 进程模型，处理多用户应用

**worker**: DBMS component, responsible for executing tasks on behalf[代表] of the client and returning the results (thread or process)

* Process per DBMS Worker
  * Use a OS process, relies on OS scheduler
  * Use shared-memory for global data structures
  * A worker crash will not kill entire system
* Process Pool
  * A worker uses any process that is free in a pool
  * Still relies on OS scheduler and shared memory
  * Bad for CPU cache locality
* Thread per DBMS Worker
  * Single process with multiple worker threads
  * DBMS manages its own scheduling
  * Thread crash (may) kill the entire system

Use **multi-threaded** architecture

* Less overhead per context switch
* Do not have to manage shared memory
* DBMS manages scheduling itself

### Execution Parallelism

Inter-Query: Different queries are executed concurrently

It's **hard** to update database at the same time

---

Intra-Query: Execute the operations of a single query in parallel

> organize the operators in **producer / consumer** paradigm

对于每个 operator，都有不同的 parallel algorithms

1. Intra-Operator (Horizontal)
   * 将数据分段，每段执行相同的函数。使用 exchange operator 将各段结果合并
   * exchange operator: Gather, Re-partition, Distribute
2. Inter-Operator (Vertical)
   * pipelined parallelism: output of one is the input of another

### I/O Parallelism

> Split the database file or data, spread them across multiple storage devices

* Multi-Disk Parallelism: Use *storage appliances* and *RAID*
  * transparent to DBMS, hardware/OS control this
* Database Partitioning: 将 database 分块存放在分别的地方
  * transparent to application
  * Vertical Partitioning: 类似于 column 存储(无数据压缩)，分不同的 domain 存储
  * Horizontal Partitioning: 根据一些 partitioning key 进行分块

