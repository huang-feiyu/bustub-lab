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

  cur_ = table_info_->table_->Begin(exec_ctx_->GetTransaction());
  end_ = table_info_->table_->End();
}

bool SeqScanExecutor::Next(Tuple *tuple, RID *rid) {
  auto txn = exec_ctx_->GetTransaction();
  auto lck_mgr = exec_ctx_->GetLockManager();

  while (cur_ != end_) {
    // no S-lock in Read Uncommitted
    if (txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED) {
      try {
        lck_mgr->LockShared(txn, cur_->GetRid());
      } catch (TransactionAbortException &e) {
        return false;
      }
    }

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

      // Read Committed: release S-lock immediately
      if (txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED) {
        try {
          lck_mgr->Unlock(txn, item->GetRid());
        } catch (TransactionAbortException &e) {
          return false;
        }
      }
      return true;
    }

    // no S-lock in Read Uncommitted
    if (txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED) {
      try {
        lck_mgr->Unlock(txn, item->GetRid());
      } catch (TransactionAbortException &e) {
        return false;
      }
    }
  }
  return false;  // end of iteration
}

}  // namespace bustub
