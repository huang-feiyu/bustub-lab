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
  auto txn_id = txn->GetTransactionId();

  /* Validation Begin */
  // violate read uncommitted
  if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED) {
    txn->SetState(TransactionState::ABORTED);
    throw new TransactionAbortException(txn_id, AbortReason::LOCKSHARED_ON_READ_UNCOMMITTED);
    return false;
  }

  // cannot lock on aborted txn
  if (txn->GetState() == TransactionState::ABORTED) {
    LOG_DEBUG("Lock in aborted txn [%d]", txn_id);
    return false;
  }

  // no lock in shrinking phase
  if (txn->GetState() == TransactionState::SHRINKING) {
    txn->SetState(TransactionState::ABORTED);
    throw new TransactionAbortException(txn_id, AbortReason::LOCK_ON_SHRINKING);
    return false;
  }

  BUSTUB_ASSERT(txn->GetState() == TransactionState::GROWING, "Only grant lock when growing");

  // no re-enter
  BUSTUB_ASSERT(txn->GetExclusiveLockSet()->find(rid) == txn->GetExclusiveLockSet()->end() &&
                    txn->GetSharedLockSet()->find(rid) == txn->GetSharedLockSet()->end(),
                "re-enter");
  /* Validation End */

  /* Do normal stuff: grant lock if possible, wait otherwise */
  // make a queue if first access rid
  if (lock_table_.find(rid) == lock_table_.end()) {
    latch_.lock();
    lock_table_.emplace(std::make_pair(rid, std::make_unique<LockRequestQueue>()));
    latch_.unlock();
  }

  auto lr_queue = lock_table_.at(rid);
  std::unique_lock u_lock{lr_queue->latch_};

  LockRequest lock_request{txn_id, LockMode::SHARED};
  lr_queue->request_queue_.emplace_back(lock_request);
  while (true) {
    auto can_grant = true;
    // walk through queue to check if able to grant
    for (auto &lck_req : lr_queue->request_queue_) {
      if (lck_req.granted_ && lck_req.lock_mode_ == LockMode::EXCLUSIVE) {
        can_grant = false;
      }
    }
    if (can_grant) {
      if (txn->GetState() == TransactionState::ABORTED) {
        LOG_DEBUG("Lock in aborted txn [%d]", txn_id);
        return false;
      }
      break;
    }
    lr_queue->cv_.wait(u_lock);
  }

  // grant the lock
  for (auto itr = lr_queue->request_queue_.begin(); itr != lr_queue->request_queue_.end(); itr++) {
    if ((*itr).txn_id_ == txn_id) {
      BUSTUB_ASSERT((*itr).lock_mode_ == LockMode::SHARED, "");
      (*itr).granted_ = true;
    }
  }

  txn->GetSharedLockSet()->emplace(rid);

  return true;
}

bool LockManager::LockExclusive(Transaction *txn, const RID &rid) {
  auto txn_id = txn->GetTransactionId();

  /* Validation Begin */
  // cannot lock on aborted txn
  if (txn->GetState() == TransactionState::ABORTED) {
    LOG_DEBUG("Lock in aborted txn [%d]", txn_id);
    return false;
  }

  // no lock in shrinking phase
  if (txn->GetState() == TransactionState::SHRINKING) {
    txn->SetState(TransactionState::ABORTED);
    throw new TransactionAbortException(txn_id, AbortReason::LOCK_ON_SHRINKING);
    return false;
  }

  BUSTUB_ASSERT(txn->GetState() == TransactionState::GROWING, "Only grant lock when growing");

  // no re-enter
  BUSTUB_ASSERT(txn->GetExclusiveLockSet()->find(rid) == txn->GetExclusiveLockSet()->end() &&
                    txn->GetSharedLockSet()->find(rid) == txn->GetSharedLockSet()->end(),
                "re-enter");
  /* Validation End */

  /* Do normal stuff: grant lock if possible, wait otherwise */
  // make a queue if first access rid
  if (lock_table_.find(rid) == lock_table_.end()) {
    latch_.lock();
    lock_table_.emplace(std::make_pair(rid, std::make_unique<LockRequestQueue>()));
    latch_.unlock();
  }

  auto lr_queue = lock_table_.at(rid);
  std::unique_lock u_lock{lr_queue->latch_};

  LockRequest lock_request{txn_id, LockMode::EXCLUSIVE};
  lr_queue->request_queue_.emplace_back(lock_request);
  while (true) {
    auto can_grant = true;
    // walk through queue to check if able to grant
    for (auto &lck_req : lr_queue->request_queue_) {
      if (lck_req.granted_) {
        can_grant = false;
      }
    }
    if (can_grant) {
      if (txn->GetState() == TransactionState::ABORTED) {
        LOG_DEBUG("Lock in aborted txn [%d]", txn_id);
        return false;
      }
      break;
    }
    lr_queue->cv_.wait(u_lock);
  }

  // grant the lock
  for (auto itr = lr_queue->request_queue_.begin(); itr != lr_queue->request_queue_.end(); itr++) {
    if ((*itr).txn_id_ == txn_id) {
      BUSTUB_ASSERT((*itr).lock_mode_ == LockMode::EXCLUSIVE, "");
      (*itr).granted_ = true;
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
