//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>
#include <vector>

#include "execution/executors/aggregation_executor.h"

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      child_(std::move(child)),
      aht_(SimpleAggregationHashTable(plan->GetAggregates(), plan->GetAggregateTypes())),
      aht_iterator_(aht_.End()) {}

void AggregationExecutor::Init() {
  // Build hash table
  child_->Init();

  Tuple tuple;
  RID rid;
  while (child_->Next(&tuple, &rid)) {
    aht_.InsertCombine(MakeAggregateKey(&tuple), MakeAggregateValue(&tuple));
  }
  aht_iterator_ = aht_.Begin();
}

bool AggregationExecutor::Next(Tuple *tuple, RID *rid) {
  if (aht_iterator_ == aht_.End()) {
    return false;
  }

  auto &key = aht_iterator_.Key();
  auto &val = aht_iterator_.Val();
  ++aht_iterator_;

  auto having = plan_->GetHaving();
  if (having == nullptr || having->EvaluateAggregate(key.group_bys_, val.aggregates_).GetAs<bool>()) {
    std::vector<Value> vals;
    for (auto &col : GetOutputSchema()->GetColumns()) {
      vals.emplace_back(col.GetExpr()->EvaluateAggregate(key.group_bys_, val.aggregates_));
    }
    *tuple = Tuple(vals, GetOutputSchema());
    *rid = tuple->GetRid();
    return true;
  }

  return Next(tuple, rid);
}

const AbstractExecutor *AggregationExecutor::GetChildExecutor() const { return child_.get(); }

}  // namespace bustub
