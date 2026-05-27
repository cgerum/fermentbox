#include "nn_controller.h"

#include <cmath>
#include <cstddef>

#ifndef FERMENTBOX_EXPERIMENTAL_RL_CONTROLLER
#define FERMENTBOX_EXPERIMENTAL_RL_CONTROLLER 0
#endif

namespace {

enum TemperatureAction {
  TemperatureActionIdle = 0,
  TemperatureActionHeat = 1,
  TemperatureActionCool = 2,
};

constexpr std::size_t ErrorBucketCount = 5;
constexpr std::size_t TrendBucketCount = 3;
constexpr std::size_t ActionCount = 3;

class ExperimentalTemperatureController {
public:
  ExperimentalTemperatureController() { reset(); }

  void reset() {
    previous_temperature = 0.0f;
    has_previous_temperature = false;
    has_previous_transition = false;
    for (std::size_t error = 0; error < ErrorBucketCount; ++error) {
      for (std::size_t trend = 0; trend < TrendBucketCount; ++trend) {
        initializeActionValues(error, trend, q_values[error][trend]);
      }
    }
  }

  TemperatureControllerOutput step(const TemperatureControllerInput &input) {
    TemperatureControllerOutput output = {false, false};
    if (!input.temperature_active || input.measurement_error) {
      has_previous_transition = false;
      has_previous_temperature = false;
      return output;
    }

    const float delta =
        has_previous_temperature ? (input.current_temperature - previous_temperature)
                                 : 0.0f;
    const std::size_t error_bucket =
        classifyError(input.target_temperature - input.current_temperature);
    const std::size_t trend_bucket = classifyTrend(delta);

    if (has_previous_transition) {
      updateQValue(input, error_bucket, trend_bucket);
    }

    const TemperatureAction action =
        selectAction(input, error_bucket, trend_bucket);
    output = toOutput(action, input);

    previous_error_bucket = error_bucket;
    previous_trend_bucket = trend_bucket;
    previous_action = action;
    has_previous_transition = true;
    previous_temperature = input.current_temperature;
    has_previous_temperature = true;

    return output;
  }

private:
  void initializeActionValues(std::size_t error_bucket, std::size_t trend_bucket,
                              float *values) {
    values[TemperatureActionIdle] = 0.0f;
    values[TemperatureActionHeat] = -0.5f;
    values[TemperatureActionCool] = -0.5f;

    switch (error_bucket) {
    case 0:
      values[TemperatureActionHeat] = 2.0f;
      values[TemperatureActionIdle] = -0.8f;
      values[TemperatureActionCool] = -2.0f;
      break;
    case 1:
      values[TemperatureActionHeat] = 1.0f;
      values[TemperatureActionIdle] = 0.2f;
      values[TemperatureActionCool] = -1.0f;
      break;
    case 2:
      values[TemperatureActionIdle] = 1.5f;
      values[TemperatureActionHeat] = -0.3f;
      values[TemperatureActionCool] = -0.3f;
      break;
    case 3:
      values[TemperatureActionCool] = 1.0f;
      values[TemperatureActionIdle] = 0.2f;
      values[TemperatureActionHeat] = -1.0f;
      break;
    case 4:
      values[TemperatureActionCool] = 2.0f;
      values[TemperatureActionIdle] = -0.8f;
      values[TemperatureActionHeat] = -2.0f;
      break;
    }

    if (trend_bucket == 0 && error_bucket <= 1) {
      values[TemperatureActionHeat] += 0.3f;
    }
    if (trend_bucket == 2 && error_bucket >= 3) {
      values[TemperatureActionCool] += 0.3f;
    }
    if ((trend_bucket == 2 && error_bucket <= 1) ||
        (trend_bucket == 0 && error_bucket >= 3)) {
      values[TemperatureActionIdle] += 0.2f;
    }
  }

  std::size_t classifyError(float error) const {
    if (error >= 0.8f) {
      return 0;
    }
    if (error >= 0.2f) {
      return 1;
    }
    if (error > -0.2f) {
      return 2;
    }
    if (error > -0.8f) {
      return 3;
    }
    return 4;
  }

  std::size_t classifyTrend(float delta) const {
    if (delta <= -0.05f) {
      return 0;
    }
    if (delta >= 0.05f) {
      return 2;
    }
    return 1;
  }

  float rewardForInput(const TemperatureControllerInput &input) const {
    const float absolute_error =
        std::fabs(input.target_temperature - input.current_temperature);
    float reward = 1.0f - absolute_error;
    if (absolute_error <= 0.15f) {
      reward += 1.0f;
    } else if (absolute_error >= 1.0f) {
      reward -= 1.0f;
    }

    if (previous_action != TemperatureActionIdle) {
      reward -= 0.05f;
    }
    if ((previous_action == TemperatureActionHeat &&
         input.current_temperature > input.target_temperature) ||
        (previous_action == TemperatureActionCool &&
         input.current_temperature < input.target_temperature)) {
      reward -= 1.5f;
    }

    return reward;
  }

  void updateQValue(const TemperatureControllerInput &input,
                    std::size_t next_error_bucket,
                    std::size_t next_trend_bucket) {
    float next_max = q_values[next_error_bucket][next_trend_bucket][0];
    for (std::size_t action = 1; action < ActionCount; ++action) {
      const float candidate = q_values[next_error_bucket][next_trend_bucket][action];
      if (candidate > next_max) {
        next_max = candidate;
      }
    }

    float &value =
        q_values[previous_error_bucket][previous_trend_bucket][previous_action];
    const float reward = rewardForInput(input);
    value += LearningRate * (reward + DiscountFactor * next_max - value);
  }

  TemperatureAction selectAction(const TemperatureControllerInput &input,
                                 std::size_t error_bucket,
                                 std::size_t trend_bucket) const {
    float best_value = -1000000.0f;
    TemperatureAction best_action = TemperatureActionIdle;

    for (std::size_t action = 0; action < ActionCount; ++action) {
      const TemperatureAction candidate =
          static_cast<TemperatureAction>(action);
      if (!actionAllowed(candidate, input)) {
        continue;
      }
      const float value = q_values[error_bucket][trend_bucket][action];
      if (value > best_value) {
        best_value = value;
        best_action = candidate;
      }
    }

    return best_action;
  }

  bool actionAllowed(TemperatureAction action,
                     const TemperatureControllerInput &input) const {
    if (action == TemperatureActionHeat) {
      return input.heater_on || input.heater_cooldown <= 0;
    }
    if (action == TemperatureActionCool) {
      return input.cooler_on || input.cooler_cooldown <= 0;
    }
    return true;
  }

  TemperatureControllerOutput
  toOutput(TemperatureAction action,
           const TemperatureControllerInput &input) const {
    TemperatureControllerOutput output = {false, false};

    switch (action) {
    case TemperatureActionHeat:
      output.heater_on = input.heater_on || input.heater_cooldown <= 0;
      break;
    case TemperatureActionCool:
      output.cooler_on = input.cooler_on || input.cooler_cooldown <= 0;
      break;
    case TemperatureActionIdle:
      break;
    }

    return output;
  }

  static constexpr float LearningRate = 0.15f;
  static constexpr float DiscountFactor = 0.85f;

  float q_values[ErrorBucketCount][TrendBucketCount][ActionCount];
  float previous_temperature;
  std::size_t previous_error_bucket;
  std::size_t previous_trend_bucket;
  TemperatureAction previous_action;
  bool has_previous_temperature;
  bool has_previous_transition;
};

#if FERMENTBOX_EXPERIMENTAL_RL_CONTROLLER
ExperimentalTemperatureController experimental_controller;
#endif

} // namespace

bool experimentalTemperatureControllerEnabled() {
#if FERMENTBOX_EXPERIMENTAL_RL_CONTROLLER
  return true;
#else
  return false;
#endif
}

void resetExperimentalTemperatureController() {
#if FERMENTBOX_EXPERIMENTAL_RL_CONTROLLER
  experimental_controller.reset();
#endif
}

bool runExperimentalTemperatureController(const TemperatureControllerInput &input,
                                          TemperatureControllerOutput &output) {
#if FERMENTBOX_EXPERIMENTAL_RL_CONTROLLER
  output = experimental_controller.step(input);
  return true;
#else
  (void)input;
  (void)output;
  return false;
#endif
}
