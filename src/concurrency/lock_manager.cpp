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

#include "concurrency/transaction_manager.h"

namespace bustub {

bool LockManager::LockShared(Transaction *txn, const RID &rid) {
  BIGLOCK();
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

  // Wait in Queue
  WaitInQueue(txn, lck_reqs, mode);

  // Wait done, grant the lock
  GetIterator(lck_reqs, txn_id)->granted_ = true;
  txn->GetSharedLockSet()->emplace(rid);

  LOG_MINE("LockShared: txn[%d] got rid[%ld]", txn_id, rid.Get());
  return true;
}

bool LockManager::LockExclusive(Transaction *txn, const RID &rid) {
  BIGLOCK();
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

  // Wait in Queue
  WaitInQueue(txn, lck_reqs, mode);

  // Wait done, grant the lock
  GetIterator(lck_reqs, txn_id)->granted_ = true;
  txn->GetExclusiveLockSet()->emplace(rid);

  LOG_MINE("LockExclusive: txn[%d] got rid[%ld]", txn_id, rid.Get());
  return true;
}

bool LockManager::LockUpgrade(Transaction *txn, const RID &rid) {
  BIGLOCK();
  auto mode = LockMode::EXCLUSIVE;
  auto txn_id = txn->GetTransactionId();

  if (!ValidateLocking(txn, mode)) {
    return false;
  }

  // Has gotten X-lock before?
  if (txn->IsExclusiveLocked(rid)) {
    return true;
  }

  // There is S-lock request before
  assert(txn->IsSharedLocked(rid));

  auto lck_reqs = &lock_table_[rid];
  // If another transaction is already waiting to upgrade their lock
  if (lck_reqs->upgrading_ != INVALID_TXN_ID) {
    txn->SetState(TransactionState::ABORTED);
    LOG_MINE("ABORTED: Upgrade Conflict");
    throw TransactionAbortException(txn_id, AbortReason::UPGRADE_CONFLICT);
  }
  lck_reqs->upgrading_ = txn_id;

  KillYoung(lck_reqs, txn_id, mode);
  // Upgrade request to X-lock
  GetIterator(lck_reqs, txn_id)->lock_mode_ = mode;

  // Wait in Queue
  WaitInQueue(txn, lck_reqs, mode);

  // Wait done, grant the lock
  GetIterator(lck_reqs, txn_id)->granted_ = true;
  lck_reqs->upgrading_ = INVALID_TXN_ID;
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->emplace(rid);

  LOG_MINE("LockUpgrade: txn[%d] got rid[%ld]", txn_id, rid.Get());
  return true;
}

bool LockManager::Unlock(Transaction *txn, const RID &rid) {
  BIGLOCK();
  auto txn_id = txn->GetTransactionId();

  /* Validate arguments */
  if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED && txn->IsSharedLocked(rid)) {
    txn->SetState(TransactionState::ABORTED);
    LOG_MINE("ABORTED: Unlock on Shrinking");
    throw TransactionAbortException(txn_id, AbortReason::UNLOCK_ON_SHRINKING);
  }

  // remove from request queue
  auto lck_reqs = RemoveQueue(txn, rid);

  // Update txn state
  if (txn->GetState() == TransactionState::GROWING) {
    if (txn->GetIsolationLevel() == IsolationLevel::REPEATABLE_READ) {
      txn->SetState(TransactionState::SHRINKING);
    } else if (txn->IsExclusiveLocked(rid)) {
      txn->SetState(TransactionState::SHRINKING);
    }
  }
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->erase(rid);

  // Notify all waiting txns
  lck_reqs->cv_.notify_all();

  LOG_MINE("Unlock: txn[%d] got rid[%ld]", txn_id, rid.Get());
  return true;
}

void LockManager::KillYoung(LockRequestQueue *lck_reqs, txn_id_t txn_id, LockMode mode) {
  for (auto itr = lck_reqs->request_queue_.begin(); itr != lck_reqs->request_queue_.end(); itr++) {
    auto id = itr->txn_id_;  // NOLINT
    // Higher txn_id => Lower priorty
    if (id > txn_id && TransactionManager::GetTransaction(id)->GetState() != TransactionState::ABORTED) {
      if (mode == LockMode::EXCLUSIVE || itr->lock_mode_ == LockMode::EXCLUSIVE) {
        itr->granted_ = false;
        TransactionManager::GetTransaction(id)->SetState(TransactionState::ABORTED);
        lck_reqs->cv_.notify_all();
      }
    }
  }
}

}  // namespace bustub
