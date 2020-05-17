#include <JsonObjectStream.h>
#include <FlashString/Map.hpp>
#include <FlashString/Stream.hpp>

#include "webserver.h"
#include "configuration.h"
#include "sensors.h"

static bool serverStarted = false;
static HttpServer server;

static bool sendFile(const String& fileName, HttpResponse& response)
{
	debug_i("File '%s' requested", fileName.c_str());

	return response.sendFile(fileName);
}

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.setCache(86400, true); // It's important to use cache for better performance.
	sendFile("index.html", response);
}

void onNetworkConfig(HttpRequest& request, HttpResponse& response)
{
	if(request.method != HTTP_POST) {
		response.code = HTTP_STATUS_BAD_REQUEST;
		return;
	}

	debugf("Update config");
	// Update config
	if(request.getBody() == nullptr) {
		debugf("NULL bodyBuf");
		return;
	}


	FermentboxConfig &activeConfig = FermentboxConfig::get();

	StaticJsonDocument<ConfigJsonBufferSize> root;

	if(!Json::deserialize(root, request.getBodyStream())) {
		debug_w("Invalid JSON to un-serialize");
		return;
	}

	Json::serialize(root, Serial, Json::Pretty); // For debugging

	if(root.containsKey("SSID")) // Settings
	{
		activeConfig.Wifi.SSID = String((const char*)root["SSID"]);
		activeConfig.Wifi.Password = String((const char*)root["Password"]);
		
		
		WifiStation.enable(true);
		WifiAccessPoint.enable(false);
		WifiStation.config(activeConfig.Wifi.SSID, activeConfig.Wifi.Password);
	}

	activeConfig.save();
}

void onGetConfig(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();

	FermentboxConfig& activeConfig = FermentboxConfig::get();

	auto network = json.createNestedObject("Wifi");

	network["SSID"] = activeConfig.Wifi.SSID;
	network["Password"] = activeConfig.Wifi.Password;


	response.sendDataStream(stream, MIME_JSON);
}
void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		sendFile(file, response);
	}
}

void onGetMeasurement(HttpRequest& request, HttpResponse& response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject json = stream->getRoot();

	Sensors& sensors = getSensors();
	Sensors::Measurement &measurement = sensors.getLastMeasurement();

    json["date"] = measurement.date.toISO8601();
	json["temperature"] = measurement.temperature;
	json["humidity"] = measurement.humidity; 

	response.sendDataStream(stream, MIME_JSON);
}

void startWebServer()
{
	if(serverStarted)
		return;

	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/networkConfig", onNetworkConfig);
	server.paths.set("/getConfig", onGetConfig);
	server.paths.set("/getMeasurement", onGetMeasurement);
	server.paths.setDefault(onFile);
	server.setBodyParser(MIME_JSON, bodyToStringParser);
	serverStarted = true;

	if(WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if(WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}
