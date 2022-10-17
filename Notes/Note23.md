# Distributed OLTP Databases

[TOC]

OLTP: On-line Transaction Processing

OLAP: On-line Analytical Processing

How to ensure that all nodes agree to commit a txn and then make sure it does commit if we decide that it should.

### Atomic Commit Protocols

When a multi-node txn finishes, the DBMS needs to ask all the nodes involved whether it is safe to commit.

* Two-Phase Commit: Make sure every node agree the request
  * Phase #1 - Prepare
  * Phase #2 - Commit/Abort
  * Log: Each node records the outcome of each phase in a non-volatile storage log
  * Optimization: Early Prepare Voting; Early Acknowledgement After Prepare
* Paxos (Raft is the simplified version): Make sure majority of nodes agree the request

### Replication

The DBMS can replicate data across redundant nodes to increase availability.

* Replica Configurations
  * Approach #1: Master-Replica
  * Approach #2: Multi-Master
* K-Safety: a threshold for determining the fault tolerance of the replicated database, K is the least available replicas per data object
* Propagation Scheme
  * Approach #1: Synchronous (Strong Consistency)
  * Approach #2: Asynchronous (Eventual Consistency)
* Propagation Timing
  * Approach #1: Continuous
  * Approach #2: On Commit
* Apply Changes
  * Approach #1: Active-Active
  * Approach #2: Active-Passive

### Consistency Issues (CAP)

CAP Theorem: choose 2 of 3

* Consistency
* Availability
* Partition Tolerant

NoSQL: AP

NewSQL/Traditional: CP, CA

![Illustration](https://upload.wikimedia.org/wikipedia/commons/thumb/b/be/CAP_Theorem.svg/220px-CAP_Theorem.svg.png)

### Federated Databases

Distributed architecture that connects together multiple DBMSs into a single logical system.

In general, we use a middleware software. But nobody does it well.

