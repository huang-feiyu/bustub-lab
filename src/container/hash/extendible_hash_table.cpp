//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_hash_table.cpp
//
// Identification: src/container/hash/extendible_hash_table.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "container/hash/extendible_hash_table.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_TYPE::ExtendibleHashTable(const std::string &name, BufferPoolManager *buffer_pool_manager,
                                     const KeyComparator &comparator, HashFunction<KeyType> hash_fn)
    : buffer_pool_manager_(buffer_pool_manager), comparator_(comparator), hash_fn_(std::move(hash_fn)) {
  Page *page = buffer_pool_manager->NewPage(&directory_page_id_);
  HashTableDirectoryPage *dir_page = reinterpret_cast<HashTableDirectoryPage *>(page->GetData());
  dir_page->SetPageId(directory_page_id_);

  page_id_t bucket_page_id;
  buffer_pool_manager->NewPage(&bucket_page_id);  // only one page at first
  dir_page->SetBucketPageId(0, bucket_page_id);
  dir_page->SetLocalDepth(0, 0);

  assert(buffer_pool_manager->UnpinPage(directory_page_id_, true));
  assert(buffer_pool_manager->UnpinPage(bucket_page_id, false));
}

/*****************************************************************************
 * HELPERS
 *****************************************************************************/
/**
 * Hash - simple helper to downcast MurmurHash's 64-bit hash to 32-bit
 * for extendible hashing.
 *
 * @param key the key to hash
 * @return the downcast-ed 32-bit hash
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_TYPE::Hash(KeyType key) {
  return static_cast<uint32_t>(hash_fn_.GetHash(key));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToDirectoryIndex(KeyType key, HashTableDirectoryPage *dir_page) {
  uint32_t hash = Hash(key);
  uint32_t mask = dir_page->GetGlobalDepthMask();
  uint32_t bucket_id = hash & mask;
  return bucket_id;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToPageId(KeyType key, HashTableDirectoryPage *dir_page) {
  uint32_t bucket_id = KeyToDirectoryIndex(key, dir_page);
  uint32_t page_id = dir_page->GetBucketPageId(bucket_id);
  return page_id;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HashTableDirectoryPage *HASH_TABLE_TYPE::FetchDirectoryPage() {
  Page *page = buffer_pool_manager_->FetchPage(directory_page_id_);
  HashTableDirectoryPage *dir_page = reinterpret_cast<HashTableDirectoryPage *>(page->GetData());
  return dir_page;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_BUCKET_TYPE *HASH_TABLE_TYPE::FetchBucketPage(page_id_t bucket_page_id) {
  Page *page = buffer_pool_manager_->FetchPage(bucket_page_id);
  HASH_TABLE_BUCKET_TYPE *bucket_page = reinterpret_cast<HASH_TABLE_BUCKET_TYPE *>(page->GetData());
  return bucket_page;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::GetValue(Transaction *transaction, const KeyType &key, std::vector<ValueType> *result) {
  table_latch_.WLock();

  auto dir_page = FetchDirectoryPage();
  auto bkt_page_id = KeyToPageId(key, dir_page);
  auto bkt_page = FetchBucketPage(bkt_page_id);
  auto success = bkt_page->GetValue(key, comparator_, result);

  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false));
  assert(buffer_pool_manager_->UnpinPage(bkt_page_id, false));
  table_latch_.WUnlock();
  return success;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Insert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.WLock();

  auto dir_page = FetchDirectoryPage();
  auto bkt_page_id = KeyToPageId(key, dir_page);
  auto bkt_page = FetchBucketPage(bkt_page_id);

  // case 1: bucket splitting, and potentially directory growing
  if (bkt_page->IsFull()) {
    assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false));
    assert(buffer_pool_manager_->UnpinPage(bkt_page_id, false));
    table_latch_.WUnlock();
    return SplitInsert(transaction, key, value);  // leave everything to SplitInsert
  }

  // case 2: no bucket splitting
  auto success = bkt_page->Insert(key, value, comparator_);

  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false));
  assert(buffer_pool_manager_->UnpinPage(bkt_page_id, true));
  table_latch_.WUnlock();
  return success;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::SplitInsert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  // TODO(Huang):
  return false;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Remove(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.WLock();

  auto dir_page = FetchDirectoryPage();
  auto bkt_page_id = KeyToPageId(key, dir_page);
  auto bkt_page = FetchBucketPage(bkt_page_id);
  auto success = bkt_page->Remove(key, value, comparator_);

  // case 1: Merging must be attempted when a bucket becomes empty
  if (success && bkt_page->IsEmpty()) {
    assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false));
    assert(buffer_pool_manager_->UnpinPage(bkt_page_id, true));
    table_latch_.WUnlock();
    Merge(transaction, key, value);  // leave everything to Merge
    return success;
  }

  // case 2: no bucket merging
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false));
  assert(buffer_pool_manager_->UnpinPage(bkt_page_id, true));
  table_latch_.WUnlock();
  return success;
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Merge(Transaction *transaction, const KeyType &key, const ValueType &value) {
  // TODO(Huang):
}

/*****************************************************************************
 * GETGLOBALDEPTH - DO NOT TOUCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_TYPE::GetGlobalDepth() {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  uint32_t global_depth = dir_page->GetGlobalDepth();
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false, nullptr));
  table_latch_.RUnlock();
  return global_depth;
}

/*****************************************************************************
 * VERIFY INTEGRITY - DO NOT TOUCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::VerifyIntegrity() {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  dir_page->VerifyIntegrity();
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false, nullptr));
  table_latch_.RUnlock();
}

/*****************************************************************************
 * TEMPLATE DEFINITIONS - DO NOT TOUCH
 *****************************************************************************/
template class ExtendibleHashTable<int, int, IntComparator>;

template class ExtendibleHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class ExtendibleHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class ExtendibleHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class ExtendibleHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class ExtendibleHashTable<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
