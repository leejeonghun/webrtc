/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
  */

#include "modules/audio_processing/aec3/matched_filter_lag_aggregator.h"

#include <sstream>
#include <string>
#include <vector>

#include "api/array_view.h"
#include "modules/audio_processing/aec3/aec3_common.h"
#include "modules/audio_processing/logging/apm_data_dumper.h"
#include "test/gtest.h"

namespace webrtc {
namespace {

void VerifyNoAggregateOutputForRepeatedLagAggregation(
    size_t num_repetitions,
    rtc::ArrayView<const MatchedFilter::LagEstimate> lag_estimates,
    MatchedFilterLagAggregator* aggregator) {
  for (size_t k = 0; k < num_repetitions; ++k) {
    EXPECT_FALSE(aggregator->Aggregate(lag_estimates));
  }
}

constexpr size_t kThresholdForRequiredLagUpdatesInARow = 10;
constexpr size_t kThresholdForRequiredIdenticalLagAggregates = 100;

}  // namespace

// Verifies that the most accurate lag estimate is chosen.
// TODO(peah): Modify and reenable according to new scheme.
TEST(MatchedFilterLagAggregator, DISABLED_MostAccurateLagChosen) {
  constexpr size_t kArtificialLag1 = 5;
  constexpr size_t kArtificialLag2 = 10;
  ApmDataDumper data_dumper(0);
  std::vector<MatchedFilter::LagEstimate> lag_estimates(2);
  MatchedFilterLagAggregator aggregator(&data_dumper);
  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag1, true);
  lag_estimates[1] =
      MatchedFilter::LagEstimate(0.5f, true, kArtificialLag2, true);

  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredLagUpdatesInARow +
          kThresholdForRequiredIdenticalLagAggregates,
      lag_estimates, &aggregator);
  rtc::Optional<size_t> aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag1, *aggregated_lag);

  lag_estimates[0] =
      MatchedFilter::LagEstimate(0.5f, true, kArtificialLag1, true);
  lag_estimates[1] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag2, true);

  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredIdenticalLagAggregates, lag_estimates, &aggregator);
  aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag2, *aggregated_lag);
}

// Verifies that varying lag estimates causes lag estimates to not be deemed
// reliable.
// TODO(peah): Modify and reenable according to new scheme.
TEST(MatchedFilterLagAggregator,
     DISABLED_LagEstimateInvarianceRequiredForAggregatedLag) {
  constexpr size_t kArtificialLag1 = 5;
  constexpr size_t kArtificialLag2 = 10;
  ApmDataDumper data_dumper(0);
  std::vector<MatchedFilter::LagEstimate> lag_estimates(1);
  MatchedFilterLagAggregator aggregator(&data_dumper);
  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag1, true);
  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredLagUpdatesInARow +
          kThresholdForRequiredIdenticalLagAggregates,
      lag_estimates, &aggregator);
  rtc::Optional<size_t> aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag1, *aggregated_lag);

  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag2, true);

  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredIdenticalLagAggregates, lag_estimates, &aggregator);
  aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag2, *aggregated_lag);
}

// Verifies that lag estimate updates are required to produce an updated lag
// aggregate.
// TODO(peah): Modify and reenable according to new scheme.
TEST(MatchedFilterLagAggregator,
     DISABLED_LagEstimateUpdatesRequiredForAggregatedLag) {
  constexpr size_t kArtificialLag1 = 5;
  constexpr size_t kArtificialLag2 = 10;
  ApmDataDumper data_dumper(0);
  std::vector<MatchedFilter::LagEstimate> lag_estimates(1);
  MatchedFilterLagAggregator aggregator(&data_dumper);
  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag1, true);
  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredLagUpdatesInARow +
          kThresholdForRequiredIdenticalLagAggregates,
      lag_estimates, &aggregator);
  rtc::Optional<size_t> aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag1, *aggregated_lag);

  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag2, false);

  for (size_t k = 0; k < kThresholdForRequiredLagUpdatesInARow +
                             kThresholdForRequiredIdenticalLagAggregates + 1;
       ++k) {
    aggregated_lag = aggregator.Aggregate(lag_estimates);
    EXPECT_TRUE(aggregated_lag);
    EXPECT_EQ(kArtificialLag1, *aggregated_lag);
  }

  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag2, true);
  for (size_t k = 0; k < kThresholdForRequiredLagUpdatesInARow; ++k) {
    aggregated_lag = aggregator.Aggregate(lag_estimates);
    EXPECT_TRUE(aggregated_lag);
    EXPECT_EQ(kArtificialLag1, *aggregated_lag);
  }

  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredIdenticalLagAggregates, lag_estimates, &aggregator);

  aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag2, *aggregated_lag);
}

// Verifies that an aggregated lag is persistent if the lag estimates do not
// change and that an aggregated lag is not produced without gaining lag
// estimate confidence.
// TODO(peah): Modify and reenable according to new scheme.
TEST(MatchedFilterLagAggregator, DISABLED_PersistentAggregatedLag) {
  constexpr size_t kArtificialLag = 5;
  ApmDataDumper data_dumper(0);
  std::vector<MatchedFilter::LagEstimate> lag_estimates(1);
  MatchedFilterLagAggregator aggregator(&data_dumper);
  lag_estimates[0] =
      MatchedFilter::LagEstimate(1.f, true, kArtificialLag, true);
  VerifyNoAggregateOutputForRepeatedLagAggregation(
      kThresholdForRequiredLagUpdatesInARow +
          kThresholdForRequiredIdenticalLagAggregates,
      lag_estimates, &aggregator);
  rtc::Optional<size_t> aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag, *aggregated_lag);

  aggregated_lag = aggregator.Aggregate(lag_estimates);
  EXPECT_TRUE(aggregated_lag);
  EXPECT_EQ(kArtificialLag, *aggregated_lag);
}

#if RTC_DCHECK_IS_ON && GTEST_HAS_DEATH_TEST && !defined(WEBRTC_ANDROID)

// Verifies the check for non-null data dumper.
TEST(MatchedFilterLagAggregator, NullDataDumper) {
  EXPECT_DEATH(MatchedFilterLagAggregator(nullptr), "");
}

#endif

}  // namespace webrtc
