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
}

bool NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple l_tuple;
  Tuple r_tuple;
  RID l_rid;
  RID r_rid;

  right_->Init();
  if (!left_->Next(&l_tuple, &l_rid)) {
    return false;
  }

  auto pred = plan_->Predicate();
  while (right_->Next(&r_tuple, &r_rid)) {
    auto res = pred->EvaluateJoin(&l_tuple, left_->GetOutputSchema(), &r_tuple, right_->GetOutputSchema());
    if (res.GetAs<bool>()) {
      std::vector<Value> vals;
      for (auto &col : GetOutputSchema()->GetColumns()) {
        vals.emplace_back(col.GetExpr()->Evaluate(&l_tuple, left_->GetOutputSchema()));
      }
      *tuple = Tuple(vals, GetOutputSchema());
      *rid = l_tuple.GetRid();
      return true;
    }
  }

  return false;
}

}  // namespace bustub
