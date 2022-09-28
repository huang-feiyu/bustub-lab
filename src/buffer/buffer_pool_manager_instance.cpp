//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"

#include "common/logger.h"
#include "common/macros.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager)
    : BufferPoolManagerInstance(pool_size, 1, 0, disk_manager, log_manager) {}

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, uint32_t num_instances, uint32_t instance_index,
                                                     DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size),
      num_instances_(num_instances),
      instance_index_(instance_index),
      next_page_id_(static_cast<int>(instance_index)),
      disk_manager_(disk_manager),
      log_manager_(log_manager) {
  BUSTUB_ASSERT(num_instances > 0, "If BPI is not part of a pool, then the pool size should just be 1");
  BUSTUB_ASSERT(
      instance_index < num_instances,
      "BPI index cannot be greater than the number of BPIs in the pool. In non-parallel case, index should just be 1.");
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete replacer_;
}

bool BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  assert(page_id != INVALID_PAGE_ID);

  /* Get frame in buffer pool */
  Page *page = GetPage(page_id);
  if (page == nullptr) {
    // not found
    return false;
  }
  // found: write to disk
  if (page->is_dirty_) {
    disk_manager_->WritePage(page_id, page->data_);
    page->is_dirty_ = false;
  }
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  for (auto p2f : page_table_) {
    assert(FlushPgImp(p2f.first));
  }
}

Page *BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) {
  // 0.   Make sure you call AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  frame_id_t frame_id;
  Page *page = GetEvictedPage(&frame_id);
  if (page == nullptr) {
    return nullptr;
  }

  *page_id = AllocatePage();
  page->page_id_ = *page_id;
  assert(page->pin_count_ == 0);
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  memset(pages_[frame_id].data_, 0, PAGE_SIZE);
  page_table_[*page_id] = frame_id;

  return page;
}

Page *BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  Page *page = GetPage(page_id);
  if (page != nullptr) {
    page->pin_count_++;
    replacer_->Pin(page_table_[page_id]);
    return page;
  }

  // Find replacement page
  frame_id_t frame_id;
  page = GetEvictedPage(&frame_id);
  if (page == nullptr) {
    return nullptr;
  }

  // Update meta-data
  page->page_id_ = page_id;
  assert(page->pin_count_ == 0);
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  disk_manager_->ReadPage(page_id, page->data_);
  page_table_[page_id] = frame_id;

  return page;
}

bool BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) {
  // 0.   Make sure you call DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  Page *page = GetPage(page_id);
  if (page == nullptr) {
    return true;
  }
  if (page->pin_count_ > 0) {
    return false;  // someone is using the page
  }

  page->page_id_ = INVALID_PAGE_ID;
  page->pin_count_ = 0;
  FlushPgImp(page_id);                         // reset is_dirty_
  memset(page->data_, 0, PAGE_SIZE);           // zero-out memory
  free_list_.push_back(page_table_[page_id]);  // return to freelist
  page_table_.erase(page_id);                  // remove from pgtbl

  DeallocatePage(page_id);
  return true;
}

bool BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  Page *page = GetPage(page_id);
  assert(page != nullptr);
  if (page->pin_count_ <= 0) {
    return false;
  }
  page->pin_count_--;
  page->is_dirty_ |= is_dirty;
  if (page->pin_count_ == 0) {
    replacer_->Unpin(page_table_[page_id]);
  }
  return true;
}

page_id_t BufferPoolManagerInstance::AllocatePage() {
  const page_id_t next_page_id = next_page_id_;
  next_page_id_ += static_cast<int32_t>(num_instances_);
  ValidatePageId(next_page_id);
  return next_page_id;
}

void BufferPoolManagerInstance::ValidatePageId(const page_id_t page_id) const {
  assert(page_id % num_instances_ == instance_index_);  // allocated pages mod back to this BPI
}

Page *BufferPoolManagerInstance::GetPage(page_id_t page_id) {
  ValidatePageId(page_id);
  auto itr = page_table_.find(page_id);
  if (itr == page_table_.end()) {
    return nullptr;
  }
  Page *page = &pages_[itr->second];
  return page;
}

Page *BufferPoolManagerInstance::GetEvictedPage(frame_id_t *frame_id) {
  if (free_list_.empty() && replacer_->Size() == 0) {
    return nullptr;
  }

  Page *page;
  if (!free_list_.empty()) {
    // pick from free list first
    *frame_id = free_list_.front();
    free_list_.pop_front();
    page = &pages_[*frame_id];
  } else {
    // pick from replacer
    if (!replacer_->Victim(frame_id)) {
      return nullptr;
    }
    page = &pages_[*frame_id];
    FlushPgImp(page->page_id_);         // flush to disk
    page_table_.erase(page->page_id_);  // remove from pgtbl
  }

  return page;
}

}  // namespace bustub
