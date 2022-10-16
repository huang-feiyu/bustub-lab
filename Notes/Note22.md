# Introduction to Distributed Databases

[TOC]

Parallel DBMSs: Multiple Cores

Distributed DBMSs: Multiple Machines

### System Architectures

one-node system: Shared Everything

Multi-nodes system:

```bash
Shared Memory:
[CPU1] [CPU2] [CPU3]
[===  Network   ===]
[= Shared Memory  =]
[===    Disk    ===]

Shared Disk: (Cloud Storage)
[CPU1] [CPU2] [CPU3]
[Mem1] [Mem2] [Mem3]
[===  Network   ===]
[== Shared Disk  ==]

Shared Nothing: (most)
[====  Network   ====]
[CPU1]  [CPU2]  [CPU3]
[Mem1]  [Mem2]  [Mem3]
[Disk1] [Disk2] [Disk3]
(Use Message Fabric)
```

Advantage of Distributed DBMS: Vertical Scale is more expensive than Horizontal Scale

### Design Issues

* How does the application find data?
* How to execute queries on distributed data?
* How does the DBMS ensure correctness

---

* Homogenous Nodes: Every node can perform the same things
* Heterogenous Nodes: Nodes are assign specific tasks

### Partitioning Schemes

> Data Transparency: users do not know where data is physically located (Hide all details)

* Database Partitioning (sharding): split database across multiple resources
  * Naive approach: Each node stores one and only table
  * Horizontal Partitioning: Split a table's tuples into disjoint subsets (Need to select proper partitioning key; Logical/Physical partitioning)

A single-node txn only accesses data that is contained on one partition

A distributed txn accesses data at one or more partitions

=> Transaction Coordination:

1. Centralized: Global "traffic cop"
2. Decentralized: Nodes organize themselves

### Distributed Concurrency Control

This is harder because of:

* Replication
* Network Communication Overhead
* Node Failures
* Clock Skew

