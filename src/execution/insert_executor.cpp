//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void InsertExecutor::Init() {
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  index_infos_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
  if (plan_->IsRawInsert()) {
    insert_index_ = 0;
  } else {
    child_executor_->Init();
  }
}

bool InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  auto i_tuple = std::make_unique<Tuple>();
  auto inserted = false;

  if (plan_->IsRawInsert()) {
    auto values = plan_->RawValues();
    if (insert_index_ == values.size()) {
      return false;
    }
    *i_tuple = Tuple(values[insert_index_++], &table_info_->schema_);
  } else {
    if (!child_executor_->Next(i_tuple.get(), rid)) {
      return false;
    }
  }

  // NOTE: need to validate values schema
  assert(i_tuple != nullptr);
  inserted = table_info_->table_->InsertTuple(*i_tuple, rid, exec_ctx_->GetTransaction());

  // if inserted, need to insert into indexes
  if (inserted && !index_infos_.empty()) {
    for (auto index : index_infos_) {
      index->index_->InsertEntry(*i_tuple, *rid, exec_ctx_->GetTransaction());
    }
  }

  return inserted;
}

}  // namespace bustub
