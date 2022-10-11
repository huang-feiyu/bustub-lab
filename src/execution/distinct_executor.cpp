//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// distinct_executor.cpp
//
// Identification: src/execution/distinct_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/distinct_executor.h"

namespace bustub {

DistinctExecutor::DistinctExecutor(ExecutorContext *exec_ctx, const DistinctPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void DistinctExecutor::Init() { child_executor_->Init(); }

bool DistinctExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple d_tuple;
  if (!child_executor_->Next(&d_tuple, rid)) {
    return false;
  }

  auto hash = HashValue(d_tuple);
  if (hash_table_.find(hash) != hash_table_.end()) {
    return false;
  }
  hash_table_[hash] = d_tuple;
  *tuple = d_tuple;
  return true;
}

uint32_t DistinctExecutor::HashValue(const Tuple &tuple) {
  // Naive/Stupid version: will this make sense?
  auto o_schema = GetOutputSchema();
  auto str = tuple.ToString(o_schema);
  return murmur3::MurmurHash3_x64_128(str, o_schema->GetColumnCount());
}

}  // namespace bustub
