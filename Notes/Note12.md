# Query Execution - Part I

[TOC]

```diff
+ Query Planning <- Today
- Operator Execution
- Access Methods
- Buffer Pool Manager
- Disk Manager <- Done
```

### Processing Models

Processing Model: How the system executes a query plan

1. Iterator/Pipeline Model: Each query plan operator implements a **Next()** function
   * Refers to [Slide](https://15445.courses.cs.cmu.edu/fall2021/slides/11-queryexecution1.pdf)
   * Some operators are pipeline breaker: Joins, Subqueries, Order By, Limit
2. Materialization Model: Each operator processes its input all at once and then emits its output **all at once**
   * Better for OLTP workloads because they only query for a little tuples
3. Vectorized/Batch Model: Each operator implements a **Next()** function, but each operator **emits a batch of tuples** instead of a single tuple.
   * Ideal for OLAP queries
   * Allow for operators to use SIMD to process batches of tuples

Plan Processing Direction

1. **Top-to-Bottom**
2. Bottom-to-Top (better for CPU)

### Access Methods

Access Method: a way that the DBMS can access the data stored in a table (Not defined by SQL)

* Sequential Scan: worst thing to execute a query
  * Optimization
    1. Zone Maps: 预先生成 page 的聚合信息 (MIN, MAX ...)<br/>Good for OLAP
    2. Late Materialization: 推迟某些读操作直到上层 query plan 获取
    3. Heap Clustering: 顺序扫描已排序的 leaf node

```python
for page in table.pages:
    for t in page.tuples:
        if evalPred(t):
            # Do something
```

* Index Scan: picks an index to find the tuples that the query needs
  * Use index to avoid reading the tuples we do not need
* Multi-Index Scan: If there are multiple indexes that the DBMS can use for a query, do something to fully use them
* Index Scan page Sorting: DBMS figure out all the tuples that it needs and then sort them based on their page id

### Expression Evaluation

DBMS represents a `WHERE` clause as an expression tree

```sql
WHERE B.value = ? + 1
# => Expression tree
                     =
Attribute(S.value)              +
                    Parameter(0)  Constant(1)
# => Optimization: JIT, evaluate the expression directly
```

