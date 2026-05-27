#include "nn_controller.h"

#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const char *message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << std::endl;
    ++failures;
  }
}

TemperatureControllerInput baseInput() {
  return TemperatureControllerInput{
      true, 25.0f, 25.0f, false, false, false, 0, 0,
  };
}

} // namespace

int main() {
  expect(experimentalTemperatureControllerEnabled(),
         "experimental controller should be enabled for this test binary");

  resetExperimentalTemperatureController();
  {
    auto input = baseInput();
    input.temperature_active = false;

    TemperatureControllerOutput output{true, true};
    expect(runExperimentalTemperatureController(input, output),
           "controller should run when compiled with experimental flag");
    expect(!output.heater_on && !output.cooler_on,
           "inactive control should disable both heater and cooler");
  }

  resetExperimentalTemperatureController();
  {
    auto input = baseInput();
    input.measurement_error = true;

    TemperatureControllerOutput output{true, true};
    runExperimentalTemperatureController(input, output);
    expect(!output.heater_on && !output.cooler_on,
           "measurement errors should disable both heater and cooler");
  }

  resetExperimentalTemperatureController();
  {
    auto input = baseInput();
    input.current_temperature = 23.5f;

    TemperatureControllerOutput output{false, false};
    runExperimentalTemperatureController(input, output);
    expect(output.heater_on && !output.cooler_on,
           "controller should heat when temperature is far below target");
  }

  resetExperimentalTemperatureController();
  {
    auto input = baseInput();
    input.current_temperature = 26.5f;

    TemperatureControllerOutput output{false, false};
    runExperimentalTemperatureController(input, output);
    expect(!output.heater_on && output.cooler_on,
           "controller should cool when temperature is far above target");
  }

  resetExperimentalTemperatureController();
  {
    auto input = baseInput();
    input.current_temperature = 23.5f;
    input.heater_cooldown = 3;

    TemperatureControllerOutput output{false, false};
    runExperimentalTemperatureController(input, output);
    expect(!output.heater_on,
           "controller should respect heater cooldown when heater is off");
  }

  if (failures != 0) {
    std::cerr << failures << " test(s) failed" << std::endl;
  }
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
