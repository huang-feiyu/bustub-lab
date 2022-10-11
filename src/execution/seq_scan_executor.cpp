//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan), cur_(nullptr, RID{}, nullptr), end_(nullptr, RID{}, nullptr) {}

void SeqScanExecutor::Init() {
  // retrieve the corresponding table to scan
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid());

  // validate outputSchema
  auto schema = table_info_->schema_;
  auto o_schema = plan_->OutputSchema();
  for (auto &col : o_schema->GetColumns()) {
    schema.GetColIdx(col.GetName());  // inside function: if not exist, throw a logical_expect
  }

  cur_ = table_info_->table_->Begin(exec_ctx_->GetTransaction());
  end_ = table_info_->table_->End();
}

bool SeqScanExecutor::Next(Tuple *tuple, RID *rid) {
  while (cur_ != end_) {
    auto item = cur_++;
    assert(item->IsAllocated());
    auto pred = plan_->GetPredicate();
    if (pred == nullptr || pred->Evaluate(&(*item), &table_info_->schema_).GetAs<bool>()) {
      std::vector<Value> vals;
      for (auto &col : GetOutputSchema()->GetColumns()) {
        vals.emplace_back(col.GetExpr()->Evaluate(&*item, &table_info_->schema_));
      }
      *tuple = Tuple(vals, GetOutputSchema());
      *rid = item->GetRid();
      return true;
    }
  }
  return false;  // end of iteration
}

}  // namespace bustub
