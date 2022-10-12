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
* Distinct
  * Node: output_schema (with distinct keys)
* Nested Loop Join (Need to handle duplicate keys)
  * Node: left_table, right_table, predicate
* Hash Join (Need to handle duplicate keys)
  * Node: left_table, right_table, left_key, right_key, predicate
* Aggregation
  * Node: child, having clause, agg_keys, 

### Debug

<b>*</b> SchemaChangeSequentialScan failed => bug01

Remove redundant validation step.

```diff
@@ -21,13 +21,6 @@ void SeqScanExecutor::Init() {
   // retrieve the corresponding table to scan
   table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid());

-  // validate outputSchema
-  auto schema = table_info_->schema_;
-  auto o_schema = plan_->OutputSchema();
-  for (auto &col : o_schema->GetColumns()) {
-    schema.GetColIdx(col.GetName());  // inside function: if not exist, throw a logical_expect
-  }
-
   cur_ = table_info_->table_->Begin(exec_ctx_->GetTransaction());
   end_ = table_info_->table_->End();
 }
```

<b>*</b> NestedLoopJoin issue => bug02

Sometimes, there is one more tuple. Cannot find the bug ->
Re-implement nested loop join [Naive version].

I choose to do everything with `Init()`.

<b>*</b> HashJoinOuterTableDuplicateJoinKeys failed => bug03

```diff
bool HashJoinExecutor::Next(Tuple *tuple, RID *rid) {

  while (left_->Next(&l_tuple, &l_rid)) {
+   auto val = plan_->LeftJoinKeyExpression()->Evaluate(&l_tuple, left_->GetOutputSchema());
-   auto val = plan_->LeftJoinKeyExpression()->Evaluate(&l_tuple, right_->GetOutputSchema());
    auto hash = HashValue(val);

```
