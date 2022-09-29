//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// parallel_buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/parallel_buffer_pool_manager.h"

namespace bustub {

ParallelBufferPoolManager::ParallelBufferPoolManager(size_t num_instances, size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager)
    : num_instances_(num_instances), pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // Allocate and create individual BufferPoolManagerInstances
  bpis_ = new BufferPoolManagerInstance *[num_instances_];
  for (uint32_t i = 0; i < num_instances; i++) {
    bpis_[i] = new BufferPoolManagerInstance(pool_size, num_instances, i, disk_manager, log_manager);
  }
}

// Update constructor to destruct all BufferPoolManagerInstances and deallocate any associated memory
ParallelBufferPoolManager::~ParallelBufferPoolManager() {
  for (uint32_t i = 0; i < num_instances_; i++) {
    delete bpis_[i];
  }
  delete[] bpis_;
}

size_t ParallelBufferPoolManager::GetPoolSize() {
  // Get size of all BufferPoolManagerInstances
  return static_cast<size_t>(pool_size_) * num_instances_;
}

BufferPoolManager *ParallelBufferPoolManager::GetBufferPoolManager(page_id_t page_id) {
  // Get BufferPoolManager responsible for handling given page id. You can use this method in your other methods.
  uint32_t instance_index = page_id % num_instances_;
  return bpis_[instance_index];
}

Page *ParallelBufferPoolManager::FetchPgImp(page_id_t page_id) {
  // Fetch page for page_id from responsible BufferPoolManagerInstance
  auto bpi = GetBufferPoolManager(page_id);
  return bpi->FetchPage(page_id);
}

bool ParallelBufferPoolManager::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  // Unpin page_id from responsible BufferPoolManagerInstance
  auto bpi = GetBufferPoolManager(page_id);
  return bpi->UnpinPage(page_id, is_dirty);
}

bool ParallelBufferPoolManager::FlushPgImp(page_id_t page_id) {
  // Flush page_id from responsible BufferPoolManagerInstance
  auto bpi = GetBufferPoolManager(page_id);
  return bpi->FlushPage(page_id);
}

Page *ParallelBufferPoolManager::NewPgImp(page_id_t *page_id) {
  // create new page. We will request page allocation in a round robin manner from the underlying
  // BufferPoolManagerInstances
  // 1.   From a starting index of the BPMIs, call NewPageImpl until either 1) success and return 2) looped around to
  // starting index and return nullptr
  // 2.   Bump the starting index (mod number of instances) to start search at a different BPMI each time this function
  // is called
  std::scoped_lock lock(latch_);
  uint32_t cnt = 0;
  Page *page;
  BufferPoolManager *bpi;
  while (cnt < num_instances_) {
    bpi = GetBufferPoolManager(static_cast<int>(starting_index_));
    if ((page = bpi->NewPage(page_id)) != nullptr) {
      return page;
    }
    starting_index_++;
    cnt++;
  }
  return nullptr;
}

bool ParallelBufferPoolManager::DeletePgImp(page_id_t page_id) {
  // Delete page_id from responsible BufferPoolManagerInstance
  auto bpi = GetBufferPoolManager(page_id);
  return bpi->DeletePage(page_id);
}

void ParallelBufferPoolManager::FlushAllPgsImp() {
  // flush all pages from all BufferPoolManagerInstances
  for (uint32_t i = 0; i < num_instances_; i++) {
    auto bpi = bpis_[i];
    bpi->FlushAllPages();
  }
}

}  // namespace bustub
