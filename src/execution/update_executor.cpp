//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void UpdateExecutor::Init() {
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  index_infos_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);

  child_executor_->Init();
}

bool UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  auto old_tuple = std::make_unique<Tuple>();
  auto updated = false;

  if (!child_executor_->Next(old_tuple.get(), rid)) {
    LOG_DEBUG("No more next from child");
    return false;
  }

  auto u_tuple = GenerateUpdatedTuple(*old_tuple.get());
  std::cout << "Old tuple " << old_tuple->ToString(&table_info_->schema_) << std::endl;
  std::cout << "U tuple " << u_tuple.ToString(&table_info_->schema_) << std::endl;
  updated = table_info_->table_->UpdateTuple(u_tuple, *rid, exec_ctx_->GetTransaction());

  // if updated, need to insert into indexes
  if (updated && !index_infos_.empty()) {
    for (auto index : index_infos_) {
      index->index_->DeleteEntry(*old_tuple.get(), *rid, exec_ctx_->GetTransaction());
      index->index_->InsertEntry(u_tuple, *rid, exec_ctx_->GetTransaction());
    }
  }

  return updated;
}

Tuple UpdateExecutor::GenerateUpdatedTuple(const Tuple &src_tuple) {
  const auto &update_attrs = plan_->GetUpdateAttr();
  Schema schema = table_info_->schema_;
  uint32_t col_count = schema.GetColumnCount();
  std::vector<Value> values;
  for (uint32_t idx = 0; idx < col_count; idx++) {
    if (update_attrs.find(idx) == update_attrs.cend()) {
      values.emplace_back(src_tuple.GetValue(&schema, idx));
    } else {
      const UpdateInfo info = update_attrs.at(idx);
      Value val = src_tuple.GetValue(&schema, idx);
      switch (info.type_) {
        case UpdateType::Add:
          values.emplace_back(val.Add(ValueFactory::GetIntegerValue(info.update_val_)));
          break;
        case UpdateType::Set:
          values.emplace_back(ValueFactory::GetIntegerValue(info.update_val_));
          break;
      }
    }
  }
  return Tuple{values, &schema};
}

}  // namespace bustub
