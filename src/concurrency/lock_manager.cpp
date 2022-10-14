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
  if (txn->IsExclusiveLocked(rid) || txn->IsSharedLocked(rid)) {
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
  if (txn->IsExclusiveLocked(rid)) {
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
  auto mode = LockMode::EXCLUSIVE;
  auto txn_id = txn->GetTransactionId();

  if (!ValidateLocking(txn, mode)) {
    return false;
  }

  // There is S-lock request before
  assert(txn->IsSharedLocked(rid));

  // Has gotten X-lock before?
  if (txn->IsExclusiveLocked(rid)) {
    return true;
  }

  auto lck_reqs = std::move(lock_table_.find(rid)->second);
  // If another transaction is already waiting to upgrade their lock
  if (lck_reqs->upgrading_ != INVALID_TXN_ID) {
    txn->SetState(TransactionState::ABORTED);
    throw new TransactionAbortException(txn_id, AbortReason::UPGRADE_CONFLICT);
  }
  lck_reqs->upgrading_ = txn_id;
  // Update request to X-lock
  for (auto &itr : lck_reqs->request_queue_) {
    if (itr.txn_id_ == txn_id) {
      itr.lock_mode_ = mode;
    }
    return false;  // no S-lock request before
  }

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
  lck_reqs->upgrading_ = INVALID_TXN_ID;
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->emplace(rid);
  return true;
}

bool LockManager::Unlock(Transaction *txn, const RID &rid) {
  auto txn_id = txn->GetTransactionId();

  /* Validate arguments */
  if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED && txn->IsSharedLocked(rid)) {
    txn->SetState(TransactionState::ABORTED);
    throw new TransactionAbortException(txn_id, AbortReason::UNLOCK_ON_SHRINKING);
  }

  // remove from request queue
  auto lck_reqs = std::move(lock_table_.find(rid)->second);
  for (auto itr = lck_reqs->request_queue_.begin(); itr != lck_reqs->request_queue_.end(); itr++) {
    if (itr->txn_id_ == txn_id) {
      lck_reqs->request_queue_.erase(itr);
    }
  }
  // Update txn state
  if (txn->GetState() == TransactionState::GROWING) {
    if (txn->GetIsolationLevel() == IsolationLevel::REPEATABLE_READ) {
      txn->SetState(TransactionState::SHRINKING);
    } else if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED && txn->IsExclusiveLocked(rid)) {
      txn->SetState(TransactionState::SHRINKING);
    }
  }
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->erase(rid);

  lck_reqs->cv_.notify_all();

  return true;
}

}  // namespace bustub
