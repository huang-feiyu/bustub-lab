//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/delete_executor.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void DeleteExecutor::Init() {
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  index_infos_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);

  child_executor_->Init();
}

bool DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  auto txn = exec_ctx_->GetTransaction();
  auto lck_mgr = exec_ctx_->GetLockManager();

  Tuple d_tuple;
  auto deleted = false;
  if (!child_executor_->Next(&d_tuple, rid)) {
    return false;
  }
  if (txn->IsSharedLocked(*rid)) {
    lck_mgr->LockUpgrade(txn, *rid);
  } else {
    lck_mgr->LockExclusive(txn, *rid);
  }

  deleted = table_info_->table_->MarkDelete(*rid, exec_ctx_->GetTransaction());

  // if deleted, need to insert into indexes
  if (deleted && !index_infos_.empty()) {
    auto w_record = TableWriteRecord(*rid, WType::DELETE, d_tuple, table_info_->table_.get());
    txn->GetWriteSet()->emplace_back(w_record);

    for (auto index : index_infos_) {
      auto key = d_tuple.KeyFromTuple(table_info_->schema_, index->key_schema_, index->index_->GetKeyAttrs());
      index->index_->DeleteEntry(key, *rid, exec_ctx_->GetTransaction());

      auto wi_record =
          IndexWriteRecord(*rid, table_info_->oid_, WType::DELETE, d_tuple, index->index_oid_, exec_ctx_->GetCatalog());
      txn->GetIndexWriteSet()->emplace_back(wi_record);
    }
  }

  return deleted;
}

}  // namespace bustub
