# Query Optimization- Part I

> SQL: declarative => DBMS determines how to query

[TOC]

```diff
+ Query Planning <- Today
- Operator Execution
- Access Methods
- Buffer Pool Manager
- Disk Manager <- Done
```

- **Heuristics/Rules**: Rewrite the query to remove inefficient things<br/>
  Do not need to examine data
- Cost-based Search: Use a model to estimate the cost of executing a plan<br/>
  Evaluate multiple equivalent plans for a query and pick the one with the lowest cost

```sql
Overview:
	[-] optional [*] must

SQL Query
[-] -{Rewriter}-> SQL
[*] -{Parser}-> AST
[*] -{Binder+Catalog}-> Logical Plan
[-] -{Tree Rewriter+Catalog}-> Logical Plan
[*] -{Optimizer+Catalog+Cost Model}-> Physical Plan

* Logical Plan: relational algebra expr
* Physical Plan: specific execution strategy
```

### Relational Algebra Equivalences

2 SQL queries that produce same set of tuples are equivalent<br/>=> DBMS wants to rewrite SQL/Query Plan to be more efficient

### Logical Query Optimization

Predicate Pushdown

* Selections
  * Filter as early as possible
  * Reorder predicates(判断条件)
  * Break a complex predicate, and push down
* Projections
  * Perform early for smaller tuples and reduce intermediate results
  * Project out all domains except the ones required

=> Rewrite the SQL query without knowing the data

### Cost Estimation

*n*-way join => $4^n$ orderings => DBMS has to reduce the number of query plans

Cost: CPU, Disk, Memory, Network

