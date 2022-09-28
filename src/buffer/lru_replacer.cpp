//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"
#include <algorithm>
#include "common/logger.h"

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) { num_pages_ = num_pages; }

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  if (lru_list_.empty()) {
    return false;
  }
  std::lock_guard<std::mutex> guard(list_latch_);
  *frame_id = lru_list_.back();
  lru_list_.pop_back();
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> guard(list_latch_);
  for (std::list<frame_id_t>::iterator it = lru_list_.begin(); it != lru_list_.end();) {
    if (*it == frame_id) {
      it = lru_list_.erase(it);
    } else {
      it++;
    }
  }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> guard(list_latch_);
  // no duplicate
  if (std::find(lru_list_.begin(), lru_list_.end(), frame_id) == lru_list_.end()) {
    if (Size() < num_pages_) {
      lru_list_.push_front(frame_id);
    } else {
      LOG_ERROR("Unpin: More than max size[%lu] in LRU list", num_pages_);
    }
  }
}

size_t LRUReplacer::Size() { return lru_list_.size(); }

}  // namespace bustub
