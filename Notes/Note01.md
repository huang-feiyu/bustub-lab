# Relational Model & Relational Algebra

[TOC]

### Course Info

Projects:

* Build **storage manager** from scratch
* Difficult than K-V storage

BusTub Architecture:

* Disk-Oriented Storage
* Volcano-style Query Processing
* Pluggable APIs
* Currently does not support SQL

### Database

Database: A collection of data that's related in some way, that tries to model some aspect of real world.

DBMS: 需要付出大量精力去正确实现

### History

Early DBMSs

* Tight coupling between logical and physical layers (逻辑层与物理层高耦合)

Ted Codd: Relational Model

数据库抽象

* Store database in simple database structures (table, tuple)
* Access data through high-level language (SQL)
* Physical storage left up to implementation (物理存储策略由 DBMS 选择，适应存储介质的变化)

### Data model

Data model: collection of **concepts** for describing the data in a database

Schema: a **description** of a particular collection of data, using a given data model

Data models: **Relational**, NoSQL, Matrix...

---

Relational model

* Structure: 关系结构, schema
* Integrity: 数据完整性
* Manipulation: 操纵访问

Tuple(Record): 一条记录 (A, B, ...)<br/>具有原子性, atomic, scalar

Relation(Table): 属性之间的关系

Primary key: unique identifies a single tuple => `id`

Foreign key: 两表之间的映射

---

DML(Data Manipulation Languages)

* Procedural: 指令型 (**Relational Algebra**)
* Non-Procedural: 声明式 (Relational Calculus, 数学推导)

### Relational Algebra

七种基本运算符 on 集合

* 关系代数
  * 输入：一个或多个关系
  * 输出：一个新的关系

Select: Filter, 得到满足 *predicate* 条件的子集

Projection: 生成一个只包含特定属性 *A<sub>1</sub>, A<sub>n</sub>* 的新输出关系

Union: 将两个可匹配(相同属性)关系 *R, S* 取并集

Intersection: 将两个可匹配的关系 *R, S* 取交集

Difference: *R - S*

Product: 取 *R, S* 的笛卡尔积

Join: Natural Join, 获取所有具有两表之间相同(一个或多个)属性的记录

Extra operators: Rename, Assignment...

---

Tell DBMS what to get, use high-level language

SQL is the standard *in fact*.
