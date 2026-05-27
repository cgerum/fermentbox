# Fermentbox Architecture

## 1. System Overview

Fermentbox is an embedded fermentation controller with two tightly-coupled parts:

- **Backend (firmware):** C++ application built with the Sming framework for ESP8266, responsible for sensing, control, persistence, and HTTP serving.
- **Frontend (web UI):** Vue 2 + Vuetify single-page app, compiled into static assets and embedded into the firmware SPIFFS image.

At runtime, the ESP8266 hosts both:

- the control logic (sensor sampling + actuator switching), and
- the web UI + JSON API on port 80.

## 2. Repository Structure

- `fermentbox-backend/`
  - `app/`: firmware implementation
  - `include/`: firmware public headers
  - `files/`: SPIFFS-served static files (frontend build output)
- `fermentbox-frontend/`
  - `src/`: Vue application source
  - `public/`: HTML template
- `Makefile` (root): orchestrates frontend build and backend image/flash workflow

## 3. Backend Architecture (ESP8266 / Sming)

### 3.1 Startup and Boot Sequence

Boot entrypoint is `fermentbox-backend/app/application.cpp` (`init()`). Startup order:

1. Initialize serial/debug and mount SPIFFS.
2. Configure MCU runtime (160 MHz CPU, disable Wi-Fi sleep).
3. Load persisted configuration (`FermentboxConfig::load()`).
4. If Wi-Fi credentials exist, configure station mode.
5. Register Wi-Fi event handlers:
   - disconnect handler can enable fallback AP,
   - got-IP handler disables fallback AP and starts mDNS.
6. Start subsystems:
   - `startWebServer()`
   - `startSensors()`
   - `startControlLoop()`
   - `startLogger()`

This means HTTP/UI and control both live in one firmware process, coordinated with timer callbacks.

### 3.2 Configuration Subsystem

Files:

- `include/configuration.h`
- `app/configuration.cpp`

Responsibilities:

- Load/save Wi-Fi config from SPIFFS file `.therm.conf`.
- JSON format with `Wifi.SSID` and `Wifi.Password`.
- If config missing, use compile-time defaults via `WIFI_SSID` / `WIFI_PWD` macros.

The configuration object is effectively singleton-like (`static FermentboxConfig Config`).

### 3.3 Sensor Subsystem

Files:

- `include/sensors.h`
- `app/sensors.cpp`

Responsibilities:

- Read DHT22 through `DHTesp` on GPIO16 (`D0`).
- Schedule periodic reads with a Sming timer at sensor minimum sampling period.
- Store the most recent `Measurement` (timestamp, temperature, humidity, error flag).
- Update global status to `STATUS_SENSOR_FAILED` on read errors.

Only the latest sample is retained in-memory for control and API responses.

### 3.4 Control Loop Subsystem

Files:

- `include/control_loop.h`
- `app/control_loop.cpp`

Responsibilities:

- Execute every second via timer (`onControlStep`).
- Drive four active-low outputs:
  - heater: GPIO14 (D5)
  - cooler: GPIO12 (D6)
  - ventilator: GPIO13 (D7)
  - humidifier: GPIO2
- Read latest sensor measurement.
- Apply hysteresis-based threshold control for temperature and humidity.
- Enforce cooldown counters to avoid rapid switching conflicts between opposite actuators.
- Expose simplified public state (`getControlState`) for logger and potential API use.

Current defaults in `startControlLoop()`:

- temperature control enabled, target 28.0 C
- humidity control disabled, target 50.0%

### 3.5 Status Subsystem

Files:

- `include/status.h`
- `app/status.cpp`

Responsibilities:

- Maintain global service status enum:
  - `STATUS_NORMAL`
  - `STATUS_SENSOR_FAILED`
  - `STATUS_NETWORK_UNAVAILABLE`
  - `STATUS_CONFIG_MISSING`
  - `STATUS_SCHEDULE_INACTIVE`
  - `STATUS_CONTROL_IDLE`
- Provide human-readable status text for API consumers.

### 3.6 Schedule Subsystem

Files:

- `include/schedule.h`
- `app/schedule.cpp`

Responsibilities:

- Build schedule filename convention: `.sch-{name}.json`.
- Validate schedule JSON steps on start.
- Execute schedule steps once per second and update control loop targets.
- Interpolate target values between `*_start` and `*_end` over step duration.
- Provide `startSchedule(name)` and `stopSchedule()` lifecycle entrypoints.

### 3.7 Logging Subsystem

Files:

- `app/logger.cpp`

Responsibilities:

- Every 30 seconds, sample current measurement + control targets into binary `LogEntry` records.
- Buffer records in-memory and flush to `fermentbox.log`.
- Rotate to `fermentbox.log.old` when size limit is reached.

Log format is binary, scaled fixed-point for temperature/humidity (factor 256).

### 3.8 Network Discovery

Files:

- `app/mdns.cpp`

Responsibilities:

- Register mDNS host `fermentbox` on successful station IP acquisition.

### 3.9 HTTP Server / API Layer

Files:

- `include/webserver.h`
- `app/webserver.cpp`

Responsibilities:

- Serve static files from SPIFFS (including UI).
- Expose JSON endpoints for monitoring/config/schedules.

Routes:

- `GET /` -> `index.html`
- `POST /networkConfig` -> update Wi-Fi config, persist, reconfigure station
- `GET /getConfig` -> returns saved Wi-Fi config
- `GET /getMeasurement` -> latest sample `{date, temperature, humidity}`
- `GET /getStatus` -> `{ok, message, code, codes, dimensions}`
- `GET /schedule/load?name=...` -> raw schedule JSON file
- `POST /schedule/save?name=...` -> write schedule file
- `GET /schedule/delete?name=...` -> delete schedule file
- `GET /schedule/list` -> list schedule names
- `GET /schedule/start?name=...` -> trigger `startSchedule`
- `GET /schedule/stop` -> trigger `stopSchedule`

Unknown paths are served as static assets through the default handler.

## 4. Frontend Architecture (Vue 2 + Vuetify)

### 4.1 App Composition

Files:

- `src/main.js`
- `src/App.vue`
- `src/plugins/vuetify.js`

`main.js` mounts `App.vue` with Vuetify theme setup.

`App.vue` owns shell state:

- navigation drawer and current page (`controls`, `schedule`, `settings`)
- polling timers:
  - measurement polling every 2500 ms
  - status polling every 2000 ms

### 4.2 Data Acquisition and Local Eventing

Files:

- `src/requests.js`
- `src/event-bus.js`

`sendRequest()` wraps `XMLHttpRequest` and is used across components.

`App.vue` fetches `/getMeasurement` and broadcasts samples via global Vue event bus (`new-measurement`) so graph components update independently.

### 4.3 Feature Components

- `src/components/TemperatureControl.vue`
  - listens to `new-measurement`
  - keeps rolling temperature history for sparkline display
- `src/components/HumidityControl.vue`
  - same pattern for humidity
- `src/components/Schedule.vue`
  - CRUD UI for named schedule JSON files
  - uses draggable task list (`vuedraggable`)
  - calls schedule endpoints (`list/load/save/delete/start`)
- `src/components/Settings.vue`
  - reads Wi-Fi config via `/getConfig`
  - UI for password fields (save actions are not yet wired)

## 5. End-to-End Runtime Data Flow

### 5.1 Telemetry Path

1. DHT22 sampled by sensor timer.
2. Latest measurement stored in memory.
3. Frontend polls `/getMeasurement`.
4. App emits `new-measurement` event.
5. Temperature/Humidity components append datapoints and redraw sparklines.

### 5.2 Control Path

1. Sensor sample updates shared measurement.
2. Control timer runs each second.
3. Control loop compares current values against targets with hysteresis.
4. GPIO outputs are toggled (active-low), with cooldown counters for anti-thrashing.

### 5.3 Configuration Path

1. Frontend loads `/getConfig` into settings UI.
2. On config update, backend receives `POST /networkConfig` JSON body.
3. New Wi-Fi credentials are persisted to `.therm.conf` and station mode reconfigured.

### 5.4 Schedule Path

1. Frontend edits/creates schedule JSON and saves via `/schedule/save`.
2. Backend stores schedule file in SPIFFS with `.sch-{name}.json` naming.
3. `/schedule/start` validates and starts timed schedule execution.
4. Every second, schedule engine advances the active step and updates control loop targets.
5. `/schedule/stop` stops execution and restores pre-schedule control targets.

## 6. Build and Deployment Pipeline

### 6.1 Frontend -> Firmware Asset Packaging

Root `Makefile` target `frontend`:

1. Builds Vue app (`yarn build`).
2. Copies `dist` output into `fermentbox-backend/files`.
3. Removes source maps.
4. Gzips JS/CSS assets.
5. Flattens nested `js/` and `css/` output folders.
6. Rewrites `index.html` asset URLs to match firmware file serving layout.

### 6.2 Firmware Build / Flash

- `make backend` builds backend + updates SPIFFS image.
- `make flash` builds all and flashes device.

Backend project uses Sming (`project.mk`) and Arduino libraries:

- `ArduinoJson6`
- `DHTesp`

## 7. API Contract Workflow

The API contract is defined in `api/fermentbox.openapi.yaml`.

Contract ownership rules:

- The OpenAPI file is the source of truth for endpoint and schema changes.
- Backend and frontend updates should be made after the contract is updated.
- Frontend contract artifacts are generated from the OpenAPI file.

Current generation and validation commands (frontend workspace):

- `yarn api:validate`
- `yarn api:generate`
- `yarn api:check`
- `yarn api:verify-fresh`

Rollout notes:

- Root `Makefile` targets `frontend` and `test-frontend` enforce contract validation and generated artifact freshness checks before build/tests.
- Frontend CI runs `yarn api:check` and `yarn api:verify-fresh` before lint/test/build.
- API changes must include updates to both generated files:
  - `fermentbox-frontend/src/services/api-contract.d.ts`
  - `fermentbox-frontend/src/services/api-generated.js`

## 7. Current Architectural Constraints and Gaps

- Schedule engine is only partially implemented (load/parse without timed execution).
- Settings UI has save buttons not connected to request handlers.
- API lacks authentication/authorization; Wi-Fi credentials are exposed by `/getConfig`.
- Status model is minimal (normal vs sensor failed only).
- `nn_controller.cpp` appears experimental/unintegrated and is not part of runtime wiring.

## 8. Suggested Next Architecture Steps

1. Implement executable schedule state machine tied into control targets.
2. Add authenticated configuration endpoints and avoid returning plaintext credentials.
3. Add explicit actuator state API to support richer frontend diagnostics.
4. Introduce integration tests for endpoint contracts and control-loop edge cases.
