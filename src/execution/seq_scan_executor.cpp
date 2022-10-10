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

  // construct key_attrs
  key_attrs_.reserve(o_schema->GetColumnCount());
  for (uint32_t i = 0; i < o_schema->GetColumnCount(); i++) {
    auto col_name = o_schema->GetColumn(i).GetName();
    key_attrs_.push_back(schema.GetColIdx(col_name));
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
      *tuple = item->KeyFromTuple(table_info_->schema_, *plan_->OutputSchema(), key_attrs_);
      *rid = tuple->GetRid();
      return true;
    }
  }
  return false;  // end of iteration
}

}  // namespace bustub
