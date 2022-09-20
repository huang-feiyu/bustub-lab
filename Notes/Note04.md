# Database Storage - Part II

> How the DBMS represents the database in **files on disk**?

[TOC]

```diff
  Query Planning
  Operator Execution
  Access Methods
  Buffer Pool Manager
+ Disk Manager <- Today

Bottom-up
```

### Data Representation

> Tuple Storage: 由 DBMS 解释的字节序列

* INTEGER: C/C++ Definition
* REAL(IEEE-754) vs. DECIMAL(Fixed-point)
  * 浮点数有专门硬件支持，但是有舍入误差
  * 定点数需要一些处理，比较慢；像 VARCHAR 一样存储
* VARCHAR: Header + Data
* TIME: 各有不同，一般是 Timestamp
* Large Values
  * 大多数 DBMS 不允许超过 page size 的 tuple（很难优化） ，不过可以使用有限制的 <u><b>overflow</b></u> storage pages.
  * External Value Storage(BLOB): 大数值存储在外部文件中，DBMS 无法操纵外部文件（没有事务保证）

### System Catalogs

> System Catalogs: 存储 meta-data

* 如果存储在一个特别的表中，使用 INFORMATION_SCHEMA 获取信息
* `.schema ${table}` & `.tables` 是 SQLite 获取 catalog 的 shortcut

### Storage Models

> Relational model does **not** specify that how we store tuples.

Workloads

* OLTP: Online Transaction Processing
  * 简单查询/更新仅仅有关数据库中的一个 entity
  * 场景：Amazon 订单
  * 简单查询，大量写
* OLAP: Online Analytical Processing
  * 通过复杂的查询获得一些(只读)数据，从中获取新的信息（Data Science）
  * 场景：数据分析
  * 复杂查询（join）
* HTAP: Hybrid Transaction Analytical Processing
  * OLTP + OLAP

Database Types

* Traditional SQL: 更侧重 OLAP
* NoSQL/NewSQL: 大多数更侧重 OLTP

Data Storage Models

* ***n*-ary storage model**(NSM): row storage
  * 将单个 tuple 的所有属性连续存储一个 page 中
  * Good for OLTP，Bad for 对子集进行 OLAP
* Decomposition Storage Model(DSM): column store
  * 将所有 tuple 的一个属性连续存储在 page 中
  * Good for OLAP，Slow for single tuple process
  * Tuple Identification
    1. Fixed-length Offsets（相同的大小）
    2. Embedded Tuple Ids（bad，为每个 domain 维护一个 id）

---

* OLTP = Row Store
* OLAP = Column Store

