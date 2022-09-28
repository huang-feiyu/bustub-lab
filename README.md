# Buffer Pool Manager

> 2021 fall [PROJECT #1 - BUFFER POOL](https://15445.courses.cs.cmu.edu/fall2021/project1/)

<details>
Okay, I have to use a bash script to submit zip file for 2021 fall projects.
It is inconvenient, but works after all.
</details>

[TOC]

## Prepare

The buffer pool is responsible for moving physical pages back and forth from
main memory to disk. It allows a DBMS to support databases that are larger than
the amount of memory that is available to the system.

*frame*: an array of fixed-size pages. In this lab, it is 1.

## LRU REPLACEMENT POLICY

Replacer is responsible for tracking page usage in the buffer pool.

Basic idea: Use a Queue in xv6 [bio.c](https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/bio.c).
In cpp, I choose to use `std::list` as the queue.

Easy to implement.

For concurrency control and efficiency, we should define our own data structures
to ensure thread-safety when **necessary**.
