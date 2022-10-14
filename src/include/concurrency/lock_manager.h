//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.h
//
// Identification: src/include/concurrency/lock_manager.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <condition_variable>  // NOLINT
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/rid.h"
#include "concurrency/transaction.h"

// #define LOCK
#define DEBUG

#ifdef LOCK
#define BIGLOCK() \
  std::scoped_lock<std::mutex> guard { big_latch_ }
#else
#define BIGLOCK() ((void)0)
#endif

#ifdef DEBUG
#define LOG_MINE(...)                      \
  fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__); \
  fprintf(LOG_OUTPUT_STREAM, "\n");        \
  ::fflush(stdout)
#else
#define LOG_INFO(...) ((void)0)
#endif

namespace bustub {

class TransactionManager;

/**
 * LockManager handles transactions asking for locks on records.
 */
class LockManager {
  enum class LockMode { SHARED, EXCLUSIVE };

  class LockRequest {
   public:
    LockRequest(txn_id_t txn_id, LockMode lock_mode) : txn_id_(txn_id), lock_mode_(lock_mode) {}

    txn_id_t txn_id_;
    LockMode lock_mode_;
    bool granted_{false};
  };

  class LockRequestQueue {
   public:
    std::list<LockRequest> request_queue_;
    // for notifying blocked transactions on this rid
    std::condition_variable cv_;
    // txn_id of an upgrading transaction (if any)
    txn_id_t upgrading_ = INVALID_TXN_ID;
    // for cycle reading; writing is protected by latch_
    std::mutex latch_;
  };

 public:
  /**
   * Creates a new lock manager configured for the deadlock prevention policy.
   */
  LockManager() = default;

  ~LockManager() = default;

  /*
   * [LOCK_NOTE]: For all locking functions, we:
   * 1. return false if the transaction is aborted; and
   * 2. block on wait, return true when the lock request is granted; and
   * 3. it is undefined behavior to try locking an already locked RID in the
   * same transaction, i.e. the transaction is responsible for keeping track of
   * its current locks.
   */

  /**
   * Acquire a lock on RID in shared mode. See [LOCK_NOTE] in header file.
   * @param txn the transaction requesting the shared lock
   * @param rid the RID to be locked in shared mode
   * @return true if the lock is granted, false otherwise
   */
  bool LockShared(Transaction *txn, const RID &rid);

  /**
   * Acquire a lock on RID in exclusive mode. See [LOCK_NOTE] in header file.
   * @param txn the transaction requesting the exclusive lock
   * @param rid the RID to be locked in exclusive mode
   * @return true if the lock is granted, false otherwise
   */
  bool LockExclusive(Transaction *txn, const RID &rid);

  /**
   * Upgrade a lock from a shared lock to an exclusive lock.
   * @param txn the transaction requesting the lock upgrade
   * @param rid the RID that should already be locked in shared mode by the
   * requesting transaction
   * @return true if the upgrade is successful, false otherwise
   */
  bool LockUpgrade(Transaction *txn, const RID &rid);

  /**
   * Release the lock held by the transaction.
   * @param txn the transaction releasing the lock, it should actually hold the
   * lock
   * @param rid the RID that is locked by the transaction
   * @return true if the unlock is successful, false otherwise
   */
  bool Unlock(Transaction *txn, const RID &rid);

 private:
  std::mutex big_latch_;

  /** Internal latch for lock_table_. */
  std::mutex latch_;

  /** Lock table for lock requests. */
  std::unordered_map<RID, LockRequestQueue> lock_table_;

  /*===--- Helper Functions ---===*/

  /** Validate arguments when locking */
  bool ValidateLocking(Transaction *txn, LockMode mode) {
    auto txn_id = txn->GetTransactionId();

    // Cannot lock when txn is *aborted*
    if (txn->GetState() == TransactionState::ABORTED) {
      LOG_DEBUG("Lock in aborted txn [%d]", txn_id);
      return false;
    }

    // Cannot lock on *shrink phase*
    if (txn->GetState() == TransactionState::SHRINKING) {
      txn->SetState(TransactionState::ABORTED);
      throw new TransactionAbortException(txn_id, AbortReason::LOCK_ON_SHRINKING);
    }

    // Cannot acquire S-lock on *Read Uncommitted*
    if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED && mode == LockMode::SHARED) {
      txn->SetState(TransactionState::ABORTED);
      throw new TransactionAbortException(txn_id, AbortReason::LOCKSHARED_ON_READ_UNCOMMITTED);
    }

    BUSTUB_ASSERT(txn->GetState() == TransactionState::GROWING, "Only grant lock when growing");

    return true;
  }

  /** Insert into request queue */
  LockRequestQueue *InsertQueue(Transaction *txn, const RID &rid, LockMode mode) {
    std::scoped_lock<std::mutex> guard{latch_};
    if (lock_table_.find(rid) == lock_table_.end()) {
      // First insert, init LockRequestQueue
      lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(rid), std::forward_as_tuple());
    }

    // Insert into queue
    auto lck_reqs = &lock_table_[rid];
    LockRequest lock_request{txn->GetTransactionId(), mode};
    lck_reqs->request_queue_.emplace_back(lock_request);

    return lck_reqs;
  }

  /** Remove from request queue */
  LockRequestQueue *RemoveQueue(Transaction *txn, const RID &rid) {
    std::scoped_lock<std::mutex> guard{latch_};

    // remove from request queue
    auto lck_reqs = &lock_table_[rid];
    lck_reqs->request_queue_.erase(GetIterator(lck_reqs, txn->GetTransactionId()));

    return lck_reqs;
  }

  /** Get iterator by rid */
  std::list<LockManager::LockRequest>::iterator GetIterator(LockRequestQueue *lck_reqs, txn_id_t txn_id) {
    for (auto itr = lck_reqs->request_queue_.begin(); itr != lck_reqs->request_queue_.end(); itr++) {
      if (itr->txn_id_ == txn_id) {
        return itr;
      }
    }
    return lck_reqs->request_queue_.end();
  }
};

}  // namespace bustub
