//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), left_(std::move(left_executor)), right_(std::move(right_executor)) {}

void NestedLoopJoinExecutor::Init() {
  left_->Init();
  right_->Init();

  RID rid;
  Tuple tuple;
  if (left_->Next(&tuple, &rid)) {
    prev_tuple_ = std::make_unique<Tuple>(tuple);
  } else {
    prev_tuple_ = nullptr;
  }
  if (right_->Next(&tuple, &rid)) {
    right_->Init();
  } else {
    prev_tuple_ = nullptr;
  }
}

bool NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple l_tuple;
  Tuple r_tuple;
  RID l_rid;
  RID r_rid;
  auto pred = plan_->Predicate();

  while (prev_tuple_ != nullptr) {
    if (!right_->Next(&r_tuple, &r_rid)) {
      // another cycle
      right_->Init();
      if (left_->Next(&l_tuple, &l_rid)) {
        prev_tuple_ = std::make_unique<Tuple>(l_tuple);
      } else {
        prev_tuple_ = nullptr;  // for reading consistency
      }
      continue;
    }
    // right table not empty
    if (pred == nullptr ||
        pred->EvaluateJoin(prev_tuple_.get(), left_->GetOutputSchema(), &r_tuple, right_->GetOutputSchema())
            .GetAs<bool>()) {
      std::vector<Value> vals;
      for (auto &col : GetOutputSchema()->GetColumns()) {
        vals.emplace_back(col.GetExpr()->EvaluateJoin(prev_tuple_.get(), left_->GetOutputSchema(), &r_tuple,
                                                      right_->GetOutputSchema()));
      }
      *tuple = Tuple(vals, GetOutputSchema());
      *rid = prev_tuple_->GetRid();
      return true;
    }
  }
  return false;
}

}  // namespace bustub
