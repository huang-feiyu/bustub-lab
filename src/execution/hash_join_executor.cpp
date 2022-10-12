//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.cpp
//
// Identification: src/execution/hash_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/hash_join_executor.h"
#include "execution/expressions/abstract_expression.h"

namespace bustub {

HashJoinExecutor::HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&left_child,
                                   std::unique_ptr<AbstractExecutor> &&right_child)
    : AbstractExecutor(exec_ctx), plan_(plan), left_(std::move(left_child)), right_(std::move(right_child)) {}

void HashJoinExecutor::Init() {
  left_->Init();
  right_->Init();

  // build hash table on right table
  Tuple r_tuple;
  RID r_rid;
  while (right_->Next(&r_tuple, &r_rid)) {
    auto val = plan_->RightJoinKeyExpression()->Evaluate(&r_tuple, right_->GetOutputSchema());
    auto hash = HashValue(val);
    hash_table_[hash].push_back(r_tuple);
  }
}

bool HashJoinExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple l_tuple;
  RID l_rid;

  if (!prev_tuples_.empty()) {
    auto l_tuple = prev_tuples_.back();
    prev_tuples_.pop_back();
    *tuple = l_tuple;
    *rid = l_tuple.GetRid();
    return true;
  }

  while (left_->Next(&l_tuple, &l_rid)) {
    auto val = plan_->LeftJoinKeyExpression()->Evaluate(&l_tuple, right_->GetOutputSchema());
    auto hash = HashValue(val);
    if (hash_table_.count(hash) != 0) {
      for (auto r_tuple : hash_table_[hash]) {
        std::vector<Value> vals;
        for (auto &col : GetOutputSchema()->GetColumns()) {
          vals.emplace_back(
              col.GetExpr()->EvaluateJoin(&l_tuple, left_->GetOutputSchema(), &r_tuple, right_->GetOutputSchema()));
        }
        prev_tuples_.push_back(Tuple(vals, GetOutputSchema()));
      }
      return Next(tuple, rid);
    }
  }

  return false;
}

uint32_t HashJoinExecutor::HashValue(Value val) {
  // Naive/Stupid version: will this make sense?
  auto str = val.ToString();
  return murmur3::MurmurHash3_x64_128(str, GetOutputSchema()->GetColumnCount());
}

}  // namespace bustub
