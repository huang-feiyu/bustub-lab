//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_bucket_page.cpp
//
// Identification: src/storage/page/hash_table_bucket_page.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/hash_table_bucket_page.h"
#include "common/logger.h"
#include "common/util/hash_util.h"
#include "storage/index/generic_key.h"
#include "storage/index/hash_comparator.h"
#include "storage/table/tmp_tuple.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::GetValue(KeyType key, KeyComparator cmp, std::vector<ValueType> *result) {
  bool flag = false;
  for (uint32_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    auto k2v = array_[i];
    if (cmp(key, k2v.first) == 0) {
      result->push_back(k2v.second);
      flag = true;
    }
  }
  return flag;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::Insert(KeyType key, ValueType value, KeyComparator cmp) {
  if (IsFull()) {
    return false;
  }
  std::vector<ValueType> *values;
  if (GetValue(key, cmp, values)) {
    if (std::find(values->begin(), values->end(), value) == values->end()) {
      // there is a same KV pair
      return false;
    }
  }

  uint32_t free_id = GetFreeId();
  array_[free_id] = std::make_pair(key, value);
  SetOccupied(free_id);
  SetReadable(free_id);

  size_++;
  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::Remove(KeyType key, ValueType value, KeyComparator cmp) {
  for (uint32_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    auto k2v = array_[i];
    if (readable_[i] && cmp(key, k2v.first) == 0 && k2v.second == value) {
      assert(occupied_[i]);
      ClrReadable(i);
      size_--;
      return true;
    }
  }
  return false;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
KeyType HASH_TABLE_BUCKET_TYPE::KeyAt(uint32_t bucket_idx) const {
  if (!IsReadable(bucket_idx)) {
    return {};
  }
  return array_[bucket_idx].first;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
ValueType HASH_TABLE_BUCKET_TYPE::ValueAt(uint32_t bucket_idx) const {
  if (!IsReadable(bucket_idx)) {
    return {};
  }
  assert(IsOccupied(bucket_idx));
  return array_[bucket_idx].second;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::RemoveAt(uint32_t bucket_idx) {
  if (!IsReadable(bucket_idx)) {
    return;
  }
  assert(IsOccupied(bucket_idx));
  ClrReadable(bucket_idx);
  size_--;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsOccupied(uint32_t bucket_idx) const {
  uint8_t bkt_ocp = static_cast<uint8_t>(occupied_[bucket_idx / 8]);
  bkt_ocp &= 1 << (bucket_idx % 8);
  return bkt_ocp != 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetOccupied(uint32_t bucket_idx) {
  uint8_t bkt_ocp = static_cast<uint8_t>(occupied_[bucket_idx / 8]);
  bkt_ocp |= 1 << (bucket_idx % 8);
  occupied_[bucket_idx / 8] = static_cast<char>(bkt_ocp);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsReadable(uint32_t bucket_idx) const {
  uint8_t bkt_read = static_cast<uint8_t>(readable_[bucket_idx / 8]);
  bkt_read &= 1 << (bucket_idx % 8);
  return bkt_read != 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetReadable(uint32_t bucket_idx) {
  uint8_t bkt_read = static_cast<uint8_t>(readable_[bucket_idx / 8]);
  bkt_read |= 1 << (bucket_idx % 8);
  readable_[bucket_idx / 8] = static_cast<char>(bkt_read);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::ClrReadable(uint32_t bucket_idx) {
  if (!IsReadable(bucket_idx)) return;

  uint8_t bkt_read = static_cast<uint8_t>(readable_[bucket_idx / 8]);
  bkt_read ^= 1 << (bucket_idx % 8);
  readable_[bucket_idx / 8] = static_cast<char>(bkt_read);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsFull() {
  return size_ == BUCKET_ARRAY_SIZE;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_BUCKET_TYPE::NumReadable() {
  return size_;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsEmpty() {
  return size_ == 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_BUCKET_TYPE::GetFreeId() {
  for (uint32_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (!IsReadable(i)) {
      return i;
    }
  }
  // always call IsFull before use GetFreeId
  assert(false);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::PrintBucket() {
  uint32_t size = 0;
  uint32_t taken = 0;
  uint32_t free = 0;
  for (size_t bucket_idx = 0; bucket_idx < BUCKET_ARRAY_SIZE; bucket_idx++) {
    if (!IsOccupied(bucket_idx)) {
      break;
    }

    size++;

    if (IsReadable(bucket_idx)) {
      taken++;
    } else {
      free++;
    }
  }

  LOG_INFO("Bucket Capacity: %lu, Size: %u, Taken: %u, Free: %u", BUCKET_ARRAY_SIZE, size, taken, free);
}

// DO NOT REMOVE ANYTHING BELOW THIS LINE
template class HashTableBucketPage<int, int, IntComparator>;

template class HashTableBucketPage<GenericKey<4>, RID, GenericComparator<4>>;
template class HashTableBucketPage<GenericKey<8>, RID, GenericComparator<8>>;
template class HashTableBucketPage<GenericKey<16>, RID, GenericComparator<16>>;
template class HashTableBucketPage<GenericKey<32>, RID, GenericComparator<32>>;
template class HashTableBucketPage<GenericKey<64>, RID, GenericComparator<64>>;

// template class HashTableBucketPage<hash_t, TmpTuple, HashComparator>;

}  // namespace bustub
