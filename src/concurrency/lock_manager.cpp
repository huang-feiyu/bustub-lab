//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.cpp
//
// Identification: src/concurrency/lock_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "concurrency/lock_manager.h"

#include <utility>
#include <vector>

namespace bustub {

bool LockManager::LockShared(Transaction *txn, const RID &rid) {
  auto mode = LockMode::SHARED;
  auto txn_id = txn->GetTransactionId();

  if (!ValidateLocking(txn, mode)) {
    return false;
  }

  // Has gotten a lock before?
  if (txn->GetExclusiveLockSet()->find(rid) != txn->GetExclusiveLockSet()->end() ||
      txn->GetSharedLockSet()->find(rid) != txn->GetSharedLockSet()->end()) {
    return true;
  }

  // Insert into queue
  auto lck_reqs = InsertQueue(txn, rid, mode);

  // Can grant the txn?
  auto can_grant = [&]() {
    for (auto &req : lck_reqs->request_queue_) {
      if (req.txn_id_ == txn_id) {
        return true;  // no previous (X-lock) requests
      }
      if (req.lock_mode_ == LockMode::EXCLUSIVE) {
        return false;  // a previous requests needs X-lock
      }
    }
    return true;
  };

  std::unique_lock cv_mutex{lck_reqs->latch_};
  while (!can_grant()) {
    lck_reqs->cv_.wait(cv_mutex);
  }

  // Wait done, grant the lock
  for (auto &itr : lck_reqs->request_queue_) {
    if (itr.txn_id_ == txn_id) {
      BUSTUB_ASSERT(itr.lock_mode_ == LockMode::SHARED, "lock mode error");
      itr.granted_ = true;
    }
  }
  txn->GetSharedLockSet()->emplace(rid);

  return true;
}

bool LockManager::LockExclusive(Transaction *txn, const RID &rid) {
  auto mode = LockMode::EXCLUSIVE;
  auto txn_id = txn->GetTransactionId();

  if (!ValidateLocking(txn, mode)) {
    return false;
  }

  // Has gotten X-lock before?
  if (txn->GetExclusiveLockSet()->find(rid) != txn->GetExclusiveLockSet()->end()) {
    return true;
  }

  // Insert into queue
  auto lck_reqs = InsertQueue(txn, rid, mode);

  // Can grant the txn?
  auto can_grant = [&]() { return lck_reqs->request_queue_.front().txn_id_ == txn_id; };

  std::unique_lock cv_mutex{lck_reqs->latch_};
  while (!can_grant()) {
    lck_reqs->cv_.wait(cv_mutex);
  }

  // Wait done, grant the lock
  for (auto &itr : lck_reqs->request_queue_) {
    if (itr.txn_id_ == txn_id) {
      BUSTUB_ASSERT(itr.lock_mode_ == mode, "lock mode error");
      itr.granted_ = true;
    }
  }
  txn->GetExclusiveLockSet()->emplace(rid);

  return true;
}

bool LockManager::LockUpgrade(Transaction *txn, const RID &rid) {
  // TODO(Huang):
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->emplace(rid);
  return true;
}

bool LockManager::Unlock(Transaction *txn, const RID &rid) {
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->erase(rid);
  return true;
}

}  // namespace bustub
