# Database Storage - Part I

> How the DBMS represents the database in **files on disk**?

[TOC]

```diff
  Query Planning
  Operator Execution
  Access Methods
  Buffer Pool Manager
+ Disk Manager <- Today

Bottom up
```

### Overview

* Disk-Oriented Architecture
  * 数据主要存储在 Disk 上
  * DBMS 需要协调数据在 DRAM/Mem & Disk 的传递
* Volatile & Non-Volatile
  * Volatile: Registers, Caches, DRAM;<br/>字节寻址
  * Non-volatile Memory: 非易失性内存 (未广泛使用)
  * Non-Volatile: SSD, HDD, Network Storage<br/>块寻址 (更快的顺序寻址)

---

Goal: 最小化从磁盘读取数据的代价

```shell
# 类似于 OS 虚拟内存
Mem:  Buffer Pool
Disk: Database File (Pages)
```

* Q: 为什么不用 OS VM `mmap`?
* A: OS 会在 缺页 时暂停 DBMS，DBMS 应该在这时去做别的事情<br/>DBMS always do better than OS.

### File Storage

Database: A bunch of files on disk.

数据库文件 *一般* 存储在 OS File System 上，它在 OS 看来只是一堆 Binary（也就是使用 OS 的 fs API）

**Storage manager**: Maintain a database's files as collection of pages

### Page Layout

* Page: Fixed-size block of data, with a unique id
  * DBMS uses an **indirection layer** to map page ids to PA
  * Page can contain any data (meta-data, data...)
* Page types
  * Hardware Page (4KB)
  * OS Page (4KB)
  * **Database Page (512-16KB)**
* Page Storage Arch (one of): Heap File Organization
  * heap file: unordered collection of pages
  * ADT: Linked List or **Page Directory**

---

* Linked List: Free list & Data list
* Page Directory
  * DBMS maintains special pages that tracks the location of data pages in the database files
  * The directory also records the number of free slots per page
  * 必须保证 Page Directory 与 Data Page 之间的一致性

---

Page layout

```bash
# Page
| Header | (size, checksum...)
+--------+
| Data   |
```

* Data/Tuple inside the page
  * Tuple-oriented
    1. Strawman Idea: 顺序插入 (Bad for deleting)
    2. **Slotted pages**: Tuple 大小可相等/不相等; Tuple id (page_id + slot/offset)
  * Log-structed: DBMS only stores <u>log records</u>, just appends log records
    to the file. (fast, 顺序写入; Bad for read)

```bash
# slotted page
       slot array
-------------------
Header| | | | | | |
| | ...............
...................
| Tuple_3 | Tuple_2
    |   Tuple_1   |
-------------------
```

### Tuple Layout

Tuple: a sequence of bytes

```bash
+-------------------------+
| Header | Attribute Data |
+-------------------------+
Header: meta-data
```

*denormalize*/pre-join: 将多表的数据存在一个 page 中（一般不使用）

