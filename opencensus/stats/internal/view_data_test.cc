// Copyright 2018, OpenCensus Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "opencensus/stats/view_data.h"

#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "opencensus/stats/aggregation.h"
#include "opencensus/stats/aggregation_window.h"
#include "opencensus/stats/bucket_boundaries.h"
#include "opencensus/stats/distribution.h"
#include "opencensus/stats/internal/view_data_impl.h"
#include "opencensus/stats/testing/test_utils.h"
#include "opencensus/stats/view_descriptor.h"

namespace opencensus {
namespace stats {
namespace {

TEST(ViewDataTest, CumulativeSum) {
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(Aggregation::Sum())
          .set_aggregation_window(AggregationWindow::Cumulative());
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 2.0}});
  EXPECT_EQ(Aggregation::Sum(), data.aggregation());
  EXPECT_EQ(AggregationWindow::Cumulative(), data.aggregation_window());
  ASSERT_EQ(ViewData::Type::kDouble, data.type());
  EXPECT_THAT(data.double_data(),
              ::testing::UnorderedElementsAre(
                  ::testing::Pair(::testing::ElementsAre(), 2.0)));
}

TEST(ViewDataTest, IntervalSum) {
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(Aggregation::Sum())
          .set_aggregation_window(
              AggregationWindow::Interval(absl::Minutes(1)));
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 2.0}});
  EXPECT_EQ(Aggregation::Sum(), data.aggregation());
  EXPECT_EQ(AggregationWindow::Type::kInterval,
            data.aggregation_window().type());
  EXPECT_EQ(absl::Minutes(1), data.aggregation_window().duration());
  ASSERT_EQ(ViewData::Type::kDouble, data.type());
  EXPECT_THAT(data.double_data(),
              ::testing::UnorderedElementsAre(
                  ::testing::Pair(::testing::ElementsAre(), 2.0)));
}

TEST(ViewDataTest, CumulativeCount) {
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(Aggregation::Count())
          .set_aggregation_window(AggregationWindow::Cumulative());
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 2.0}});
  EXPECT_EQ(Aggregation::Count(), data.aggregation());
  EXPECT_EQ(AggregationWindow::Cumulative(), data.aggregation_window());
  ASSERT_EQ(ViewData::Type::kInt64, data.type());
  EXPECT_THAT(data.int_data(), ::testing::UnorderedElementsAre(::testing::Pair(
                                   ::testing::ElementsAre(), 1)));
}

TEST(ViewDataTest, IntervalCount) {
  const auto window = AggregationWindow::Interval(absl::Minutes(1));
  const auto descriptor = ViewDescriptor()
                              .set_aggregation(Aggregation::Count())
                              .set_aggregation_window(window);
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 2.0}});
  EXPECT_EQ(Aggregation::Count(), data.aggregation());
  EXPECT_EQ(window, data.aggregation_window());
  ASSERT_EQ(ViewData::Type::kDouble, data.type());
  EXPECT_THAT(data.double_data(),
              ::testing::UnorderedElementsAre(
                  ::testing::Pair(::testing::ElementsAre(), 1.0)));
}

TEST(ViewDataTest, CumulativeDistribution) {
  const auto aggregation =
      Aggregation::Distribution(BucketBoundaries::Explicit({0}));
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(aggregation)
          .set_aggregation_window(AggregationWindow::Cumulative());
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 2.0}});
  EXPECT_EQ(aggregation, data.aggregation());
  EXPECT_EQ(AggregationWindow::Cumulative(), data.aggregation_window());
  ASSERT_EQ(ViewData::Type::kDistribution, data.type());
  EXPECT_EQ(data.distribution_data().size(), 1);
}

TEST(ViewDataTest, IntervalDistribution) {
  const auto aggregation =
      Aggregation::Distribution(BucketBoundaries::Explicit({0}));
  const auto window = AggregationWindow::Interval(absl::Minutes(1));
  const auto descriptor = ViewDescriptor()
                              .set_aggregation(aggregation)
                              .set_aggregation_window(window);
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 2.0}});
  EXPECT_EQ(aggregation, data.aggregation());
  EXPECT_EQ(window, data.aggregation_window());
  ASSERT_EQ(ViewData::Type::kDistribution, data.type());
  EXPECT_EQ(data.distribution_data().size(), 1);
}

TEST(ViewDataDeathTest, DoubleData) {
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(Aggregation::Sum())
          .set_aggregation_window(AggregationWindow::Cumulative());
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 1.0}});
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(data.int_data().empty()); }, "");
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(data.distribution_data().empty()); }, "");
}

TEST(ViewDataDeathTest, IntData) {
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(Aggregation::Count())
          .set_aggregation_window(AggregationWindow::Cumulative());
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 1.0}});
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(data.double_data().empty()); }, "");
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(data.distribution_data().empty()); }, "");
}

TEST(ViewDataDeathTest, DistributionData) {
  const auto descriptor =
      ViewDescriptor()
          .set_aggregation(
              Aggregation::Distribution(BucketBoundaries::Explicit({})))
          .set_aggregation_window(AggregationWindow::Cumulative());
  ViewData data = testing::TestUtils::MakeViewData(descriptor, {{{}, 1.0}});
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(data.double_data().empty()); }, "");
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(data.int_data().empty()); }, "");
}

}  // namespace
}  // namespace stats
}  // namespace opencensus
