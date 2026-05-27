/* eslint-disable */
// This file is auto-generated from api/fermentbox.openapi.yaml
// Do not edit manually.

const operations = {
  "updateNetworkConfig": {
    "path": "networkConfig",
    "method": "POST",
    "queryParameters": [],
    "hasBody": true
  },
  "getConfig": {
    "path": "getConfig",
    "method": "GET",
    "queryParameters": [],
    "hasBody": false
  },
  "getMeasurement": {
    "path": "getMeasurement",
    "method": "GET",
    "queryParameters": [],
    "hasBody": false
  },
  "getStatus": {
    "path": "getStatus",
    "method": "GET",
    "queryParameters": [],
    "hasBody": false
  },
  "listSchedules": {
    "path": "schedule/list",
    "method": "GET",
    "queryParameters": [],
    "hasBody": false
  },
  "loadSchedule": {
    "path": "schedule/load",
    "method": "GET",
    "queryParameters": [
      "name"
    ],
    "hasBody": false
  },
  "saveSchedule": {
    "path": "schedule/save",
    "method": "POST",
    "queryParameters": [
      "name"
    ],
    "hasBody": true
  },
  "deleteSchedule": {
    "path": "schedule/delete",
    "method": "GET",
    "queryParameters": [
      "name"
    ],
    "hasBody": false
  },
  "startSchedule": {
    "path": "schedule/start",
    "method": "GET",
    "queryParameters": [
      "name"
    ],
    "hasBody": false
  },
  "stopSchedule": {
    "path": "schedule/stop",
    "method": "GET",
    "queryParameters": [],
    "hasBody": false
  }
};

export default operations;
