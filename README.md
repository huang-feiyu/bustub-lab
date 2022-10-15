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

Abort Reason:
* Deadlock
* Lock on Shrinking
* Shared Lock on Read Uncommitted
* Upgrade Conflict

---

<details>

<summary>Isolation Levels:</summary>

![lock scheme](https://user-images.githubusercontent.com/70138429/195600747-ecb46d17-60e8-4661-b8eb-631dd307dec0.png)

![txn exception](https://user-images.githubusercontent.com/70138429/195600692-9bb12fbb-9d3c-4134-9bf8-c1fffa888092.png)

</details>

## Task #1: Lock Manager

**Before this task, read text Section 18.1 first.**

The basic idea of a LM is that it maintains an internal data structure about the
locks currently held by active transactions. Transactions then issue lock
requests to the LM before they are allowed to access a data item. The LM will
either grant the lock to the calling transaction, block that transaction, or
abort it.

Before **locking**, we first need to validate the arguments:
1. Cannot lock when txn is *aborted*
2. Cannot lock on *shrink phase*
3. Cannot S lock on *Read Uncommitted*

In `LockShared`, the normal stuff is:
1. Check whether we already get the [S|X]-lock. If is, return ture;
2. Otherwise, add the lock request to LockRequestQueue. (aka. blocking)
3. If one of the lock is released, will notify every lock request in the queue.
   One lock can be granted to txn if:<br/>
   All previous requests do not acquire an **X-lock**
4. When gets the lock, add to txn lock_set & update request internal data

In `LockExclusive`, the normal stuff is:
1. Check whether we already get the X-lock. If is, return ture;
2. Otherwise, add the lock request to LockRequestQueue. (aka. blocking)
3. If one of the lock is released, will notify every lock request in the queue.
   One lock can be granted to txn if:<br/>
   All previous requests do not acquire a[n] **[S|X] lock**
4. When gets the lock, add to txn lock_set & update request internal data

In `LockUpgrade`, we need also validate the arguments first.
The normal stuff is:
1. Check whether we already get the X-lock. If is, return ture;
2. Otherwise, update S-lock request to X-lock. (aka. blocking)
3. If one of the lock is released, will notify every lock request in the queue.
   One lock can be granted to txn if:<br/>
   All previous requests do not acquire a[n] **[S|X] lock**
4. When gets the lock, remove & add to txn lock_set and update request internal
   data

In `Unlock`, the normal stuff is:
1. Check whether hold the lock. If not, return false;
2. Otherwise, remove request from queue
3. Update txn state according to isolation level and remove from lock_set
4. Notify all requests in the queue

## Taksk #2: Deadlock Prevention

Wound-Wait ("Young Waits for Old")
* If requesting txn has higher priority (Older Timestamp) than holding txn,
  then holding txn aborts and releases lock.
* Otherwise requesting txn waits.

In our case, it seems like *txn_id* determines the priorty of a txn.
Bigger *txn_id* => Younger txn => Lower priorty

What we need to do is: add a *KillYoung* before every locking operation.

