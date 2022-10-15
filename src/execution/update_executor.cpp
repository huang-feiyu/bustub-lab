//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void UpdateExecutor::Init() {
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  index_infos_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);

  child_executor_->Init();
}

bool UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  auto txn = exec_ctx_->GetTransaction();
  auto lck_mgr = exec_ctx_->GetLockManager();

  Tuple old_tuple;
  auto updated = false;

  if (!child_executor_->Next(&old_tuple, rid)) {
    return false;
  }

  try {
    if (txn->IsSharedLocked(*rid)) {
      lck_mgr->LockUpgrade(txn, *rid);
    } else {
      lck_mgr->LockExclusive(txn, *rid);
    }
  } catch (TransactionAbortException &e) {
    return false;
  }

  auto u_tuple = GenerateUpdatedTuple(old_tuple);
  updated = table_info_->table_->UpdateTuple(u_tuple, *rid, exec_ctx_->GetTransaction());

  // if updated, need to insert into indexes
  if (updated && !index_infos_.empty()) {
    auto w_record = TableWriteRecord(*rid, WType::UPDATE, u_tuple, table_info_->table_.get());
    txn->GetWriteSet()->emplace_back(w_record);

    for (auto index : index_infos_) {
      auto key = old_tuple.KeyFromTuple(table_info_->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->DeleteEntry(key, *rid, exec_ctx_->GetTransaction());

      key = u_tuple.KeyFromTuple(table_info_->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->InsertEntry(key, *rid, exec_ctx_->GetTransaction());

      auto wi_record =
          IndexWriteRecord(*rid, table_info_->oid_, WType::UPDATE, u_tuple, index->index_oid_, exec_ctx_->GetCatalog());
      wi_record.old_tuple_ = old_tuple;
      txn->GetIndexWriteSet()->emplace_back(wi_record);
    }
  }

  return updated;
}

Tuple UpdateExecutor::GenerateUpdatedTuple(const Tuple &src_tuple) {
  const auto &update_attrs = plan_->GetUpdateAttr();
  Schema schema = table_info_->schema_;
  uint32_t col_count = schema.GetColumnCount();
  std::vector<Value> values;
  for (uint32_t idx = 0; idx < col_count; idx++) {
    if (update_attrs.find(idx) == update_attrs.cend()) {
      values.emplace_back(src_tuple.GetValue(&schema, idx));
    } else {
      const UpdateInfo info = update_attrs.at(idx);
      Value val = src_tuple.GetValue(&schema, idx);
      switch (info.type_) {
        case UpdateType::Add:
          values.emplace_back(val.Add(ValueFactory::GetIntegerValue(info.update_val_)));
          break;
        case UpdateType::Set:
          values.emplace_back(ValueFactory::GetIntegerValue(info.update_val_));
          break;
      }
    }
  }
  return Tuple{values, &schema};
}

}  // namespace bustub
