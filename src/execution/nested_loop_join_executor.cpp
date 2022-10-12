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

  Tuple l_tuple;
  Tuple r_tuple;
  RID l_rid;
  RID r_rid;
  auto pred = plan_->Predicate();

  while (left_->Next(&l_tuple, &l_rid)) {
    while (right_->Next(&r_tuple, &r_rid)) {
      if (pred == nullptr ||
          pred->EvaluateJoin(&l_tuple, left_->GetOutputSchema(), &r_tuple, right_->GetOutputSchema()).GetAs<bool>()) {
        std::vector<Value> vals;
        for (auto &col : GetOutputSchema()->GetColumns()) {
          vals.emplace_back(
              col.GetExpr()->EvaluateJoin(&l_tuple, left_->GetOutputSchema(), &r_tuple, right_->GetOutputSchema()));
        }
        auto tuple = Tuple(vals, GetOutputSchema());
        tuples_.emplace_back(tuple);
      }
    }
    right_->Init();
  }

  cur_ = tuples_.begin();
  end_ = tuples_.end();
}

bool NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) {
  if (cur_ == end_) {
    return false;
  }
  *tuple = *cur_.base();
  *rid = tuple->GetRid();
  cur_++;
  return true;
}

}  // namespace bustub
