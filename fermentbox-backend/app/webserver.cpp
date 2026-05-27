#include <FlashString/Map.hpp>
#include <FlashString/Stream.hpp>
#include <JsonObjectStream.h>

#include "configuration.h"
#include "schedule.h"
#include "sensors.h"
#include "status.h"
#include "webserver.h"

static const String RES_OK = String("{\"res\": \"ok\"}");

static bool serverStarted = false;
static HttpServer server;

static bool sendFile(const String &fileName, HttpResponse &response) {
  debug_i("File '%s' requested", fileName.c_str());

  return response.sendFile(fileName);
}

void onIndex(HttpRequest &request, HttpResponse &response) {
  response.setCache(
      86400, true); // It's important to use cache for better performance.
  sendFile("index.html", response);
}

void onNetworkConfig(HttpRequest &request, HttpResponse &response) {
  if (request.method != HTTP_POST) {
    response.code = HTTP_STATUS_BAD_REQUEST;
    return;
  }

  debugf("Update config");
  String body = request.getBody();
  if (!body) {
    debug_w("NULL bodyBuf");
    return;
  }

  if (body.length() == 0) {
    debug_w("Empty request body");
    return;
  }

  FermentboxConfig &activeConfig = FermentboxConfig::get();

  StaticJsonDocument<ConfigJsonBufferSize> root;

  if (!Json::deserialize(root, body)) {
    debug_w("Invalid JSON to un-serialize");
    return;
  }

  Json::serialize(root, Serial, Json::Pretty); // For debugging

  if (root.containsKey("SSID")) // Settings
  {
    activeConfig.Wifi.SSID = String((const char *)root["SSID"]);
    activeConfig.Wifi.Password = String((const char *)root["Password"]);

    WifiStation.enable(true);
    WifiAccessPoint.enable(false);
    WifiStation.config(activeConfig.Wifi.SSID, activeConfig.Wifi.Password);
  }

  activeConfig.save();

  response.setContentType(MIME_JSON);
  response.sendString(RES_OK);
}

void onGetConfig(HttpRequest &request, HttpResponse &response) {
  JsonObjectStream *stream = new JsonObjectStream();
  JsonObject json = stream->getRoot();

  FermentboxConfig &activeConfig = FermentboxConfig::get();

  auto network = json.createNestedObject("Wifi");

  network["SSID"] = activeConfig.Wifi.SSID;
  network["Password"] = activeConfig.Wifi.Password;

  response.sendDataStream(stream, MIME_JSON);
}

void onFile(HttpRequest &request, HttpResponse &response) {
  String file = request.uri.getRelativePath();

  if (file[0] == '.')
    response.code = HTTP_STATUS_FORBIDDEN;
  else {
    // It's important to use cache for better performance.
    response.setCache(86400, true);
    sendFile(file, response);
  }
}

void onGetMeasurement(HttpRequest &request, HttpResponse &response) {
  JsonObjectStream *stream = new JsonObjectStream();
  JsonObject json = stream->getRoot();

  Sensors &sensors = getSensors();
  Sensors::Measurement &measurement = sensors.getLastMeasurement();

  json["date"] = measurement.date.toUnixTime();
  json["temperature"] = measurement.temperature;
  json["humidity"] = measurement.humidity;

  response.sendDataStream(stream, MIME_JSON);
}

void onScheduleLoad(HttpRequest &request, HttpResponse &response) {
  String name = request.getQueryParameter("name");
  String filename = getScheduleFileName(name);

  sendFile(filename, response);
}

void onScheduleSave(HttpRequest &request, HttpResponse &response) {
  if (request.method != HTTP_POST) {
    response.code = HTTP_STATUS_BAD_REQUEST;
    return;
  }

  debugf("Save schedule");
  String body = request.getBody();
  if (!body) {
    debugf("NULL bodyBuf");
    return;
  }

  String name = request.getQueryParameter("name");
  String filename = getScheduleFileName(name);
  if (filename.length() > 31) {
    debugf("Filename to long");
    return;
  }
  FileStream stream(filename, IFS::OpenFlag::Write | IFS::OpenFlag::Create |
                                  IFS::OpenFlag::Truncate);
  if (!stream.isValid()) {
    debugf("Unable to open schedule file");
    response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    return;
  }

  debugf("Result: %s", body.c_str());

  auto pos = body.begin();
  while (pos != body.end()) {
    int count = stream.write((uint8_t *)pos, body.end() - pos);
    if (count <= 0) {
      response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
      return;
    }

    pos += count;
  }

  stream.close();

  response.setContentType(MIME_JSON);
  response.sendString(RES_OK);
}

void onScheduleDelete(HttpRequest &request, HttpResponse &response) {
  String name = request.getQueryParameter("name");
  String filename = getScheduleFileName(name);

  fileDelete(filename);
  response.setContentType(MIME_JSON);
  response.sendString(RES_OK);
}

void onScheduleStart(HttpRequest &request, HttpResponse &response) {
  String name = request.getQueryParameter("name");
  String error;
  if (!startSchedule(name, &error)) {
    response.code = HTTP_STATUS_BAD_REQUEST;
    response.setContentType(MIME_JSON);
    response.sendString(String("{\"error\":\"") + error + String("\"}"));
    return;
  }

  response.setContentType(MIME_JSON);
  response.sendString(RES_OK);
}

void onScheduleStop(HttpRequest &request, HttpResponse &response) {
  stopSchedule();

  response.setContentType(MIME_JSON);
  response.sendString(RES_OK);
}

void onScheduleList(HttpRequest &request, HttpResponse &response) {
  String result;
  result += String("[");

  Directory directory;
  if (!directory.open()) {
    response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    return;
  }

  int count = 0;
  while (directory.next()) {
    if (directory.stat().isDir()) {
      continue;
    }

    String filename = directory.stat().name.c_str();
    if (filename.startsWith(".sch-")) {
      if (count > 0) {
        result += String(",");
      }
      result += String("\"");
      filename.trim();
      result += filename.substring(5, filename.length() - 5);
      result += String("\"");

      count += 1;
    }
  }
  result += "]";
  response.setContentType(MIME_JSON);
  response.sendString(result);
}

void onStatus(HttpRequest &request, HttpResponse &response) {
  JsonObjectStream *stream = new JsonObjectStream();
  JsonObject json = stream->getRoot();

  FermentboxStatus status = get_status();
  FermentboxConfig &activeConfig = FermentboxConfig::get();
  bool networkAvailable = WifiStation.isEnabled() || WifiAccessPoint.isEnabled();
  bool configPresent = activeConfig.Wifi.SSID.length() > 0;
  bool scheduleActive = isScheduleActive();
  ControlStatePublic controlState = getControlState();
  bool controlActive = controlState.temperature_active || controlState.humidity_active;

  if (!networkAvailable) {
    status = static_cast<FermentboxStatus>(status | STATUS_NETWORK_UNAVAILABLE);
  }
  if (!configPresent) {
    status = static_cast<FermentboxStatus>(status | STATUS_CONFIG_MISSING);
  }
  if (!scheduleActive) {
    status = static_cast<FermentboxStatus>(status | STATUS_SCHEDULE_INACTIVE);
  }
  if (!controlActive) {
    status = static_cast<FermentboxStatus>(status | STATUS_CONTROL_IDLE);
  }

  FermentboxStatus summaryStatus = static_cast<FermentboxStatus>(
      status & ~(STATUS_SCHEDULE_INACTIVE | STATUS_CONTROL_IDLE));

  json["ok"] = status_is_ok(summaryStatus);
  json["message"] = get_status_message(summaryStatus);
  json["code"] = get_status_code(summaryStatus);

  JsonArray codes = json.createNestedArray("codes");
  if (status == STATUS_NORMAL) {
    codes.add("normal");
  } else {
    if (status & STATUS_SENSOR_FAILED) {
      codes.add("sensor_failed");
    }
    if (status & STATUS_NETWORK_UNAVAILABLE) {
      codes.add("network_unavailable");
    }
    if (status & STATUS_CONFIG_MISSING) {
      codes.add("config_missing");
    }
    if (status & STATUS_SCHEDULE_INACTIVE) {
      codes.add("schedule_inactive");
    }
    if (status & STATUS_CONTROL_IDLE) {
      codes.add("control_idle");
    }
  }

  JsonObject dimensions = json.createNestedObject("dimensions");
  auto sensor = dimensions.createNestedObject("sensor");
  sensor["code"] =
      (status & STATUS_SENSOR_FAILED) ? "sensor_failed" : "sensor_ok";
  sensor["message"] =
      (status & STATUS_SENSOR_FAILED) ? "Temperature/Humidity sensor failed"
                                      : "Sensor readings are healthy";
  sensor["ok"] = (status & STATUS_SENSOR_FAILED) == 0;

  auto network = dimensions.createNestedObject("network");
  network["code"] =
      (status & STATUS_NETWORK_UNAVAILABLE) ? "network_unavailable"
                                            : "network_available";
  network["message"] = (status & STATUS_NETWORK_UNAVAILABLE)
                           ? "Network connection unavailable"
                           : "Network is available";
  network["ok"] = (status & STATUS_NETWORK_UNAVAILABLE) == 0;

  auto config = dimensions.createNestedObject("config");
  config["code"] = (status & STATUS_CONFIG_MISSING) ? "config_missing"
                                                     : "config_present";
  config["message"] = (status & STATUS_CONFIG_MISSING)
                          ? "Wi-Fi configuration missing"
                          : "Wi-Fi configuration loaded";
  config["ok"] = (status & STATUS_CONFIG_MISSING) == 0;

  auto schedule = dimensions.createNestedObject("schedule");
  schedule["code"] = (status & STATUS_SCHEDULE_INACTIVE) ? "schedule_inactive"
                                                          : "schedule_active";
  schedule["message"] = (status & STATUS_SCHEDULE_INACTIVE)
                            ? "No schedule running"
                            : "Schedule is running";
  schedule["ok"] = true;

  auto control = dimensions.createNestedObject("control");
  control["code"] =
      (status & STATUS_CONTROL_IDLE) ? "control_idle" : "control_active";
  control["message"] = (status & STATUS_CONTROL_IDLE) ? "Control loop is idle"
                                                       : "Control loop active";
  control["ok"] = true;

  response.sendDataStream(stream, MIME_JSON);
};

void startWebServer() {
  if (serverStarted)
    return;

  server.listen(80);
  server.paths.set("/", onIndex);
  server.paths.set("/networkConfig", onNetworkConfig);
  server.paths.set("/getConfig", onGetConfig);
  server.paths.set("/getMeasurement", onGetMeasurement);
  server.paths.set("/getStatus", onStatus);

  server.paths.set("/schedule/load", onScheduleLoad);
  server.paths.set("/schedule/save", onScheduleSave);
  server.paths.set("/schedule/delete", onScheduleDelete);
  server.paths.set("/schedule/list", onScheduleList);
  server.paths.set("/schedule/start", onScheduleStart);
  server.paths.set("/schedule/stop", onScheduleStop);

  server.paths.setDefault(onFile);
  server.setBodyParser(MIME_JSON, bodyToStringParser);
  serverStarted = true;

  if (WifiStation.isEnabled())
    debugf("STA: %s", WifiStation.getIP().toString().c_str());
  if (WifiAccessPoint.isEnabled())
    debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}
