# Query Execution

> 2021 fall [PROJECT #3 - QUERY EXECUTION](https://15445.courses.cs.cmu.edu/fall2021/project3/)

[TOC]

## Prepare

The query plan is a tree composed by multiple query plan nodes. Each executor is
responsible for process its plan node. Execution engine takes a query plan and
the context. Execution engine converts a query plan to query execution, and
executes it until getting a null indicator.

What we need to do is to implement an executor for every node.

## Executors

> It's so cumbersome.

* Sequential Scan
  * Node: table, predicate
* Limit
  * Node: limit, child
* Insert
  * Node: raw values, child, table
  * Need also to insert into indexes
* Update
  * Node: update values, update attrs, child, table
* Delete
  * Node: child, table
