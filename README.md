# Concurrency Control

> 2021 fall [PROJECT #4 - CONCURRENCY CONTROL](https://15445.courses.cs.cmu.edu/fall2021/project4/)

[TOC]

## Prepare

2PL: 2 Phase Locking
* Phase #1: Growing
* Phase #2: Shrinking

Isolation levels:
* Repeatable Reads: 2PL
* Read Committed: non-2PL, S locks are released immediately
* Read Uncommitted: non-2PL, no S locks

---

<details>

<summary>Isolation Levels:</summary>

![lock scheme](https://user-images.githubusercontent.com/70138429/195600747-ecb46d17-60e8-4661-b8eb-631dd307dec0.png)

![txn exception](https://user-images.githubusercontent.com/70138429/195600692-9bb12fbb-9d3c-4134-9bf8-c1fffa888092.png)

</details>

## Task #1: Lock Manager

The basic idea of a LM is that it maintains an internal data structure about the
locks currently held by active transactions. Transactions then issue lock
requests to the LM before they are allowed to access a data item. The LM will
either grant the lock to the calling transaction, block that transaction, or
abort it.
