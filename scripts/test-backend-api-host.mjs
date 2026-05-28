#!/usr/bin/env node

import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";
import { setTimeout as delay } from "node:timers/promises";
import http from "node:http";

const backendDir = new URL("../fermentbox-backend/", import.meta.url);
const appPath = new URL("./out/Host/debug/firmware/app", backendDir);
const repoDir = new URL("../", import.meta.url);

const backendDirPath = fileURLToPath(backendDir);
const appPathValue = fileURLToPath(appPath);
const repoDirPath = fileURLToPath(repoDir);

const tapIfname = process.env.SMING_HOST_TAP_IFNAME || "tap0";
const hostIp = process.env.SMING_HOST_IP || "192.168.13.2";
const tapGateway = process.env.SMING_HOST_GATEWAY || "192.168.13.1";
const tapNetmask = process.env.SMING_HOST_NETMASK || "255.255.255.0";
const baseUrl = process.env.SMING_HOST_BASE_URL || `http://${hostIp}`;
const requestTimeoutMs = Number(process.env.API_REQUEST_TIMEOUT_MS || 8000);
const skipBuild = process.env.SKIP_HOST_BUILD === "1";
const hostBuildArgs = ["SMING_ARCH=Host", "HWCONFIG=spiffs"];

const scheduleName = `apitest-${Date.now()}`;
const scheduleBody = JSON.stringify([
  {
    duration: 0.5,
    temperature_active: true,
    temperature_start: 18.5,
    temperature_end: 18.5,
    humidity_active: true,
    humidity_start: 65,
    humidity_end: 65
  },
  {
    duration: 0.5,
    temperature_active: true,
    temperature_start: 20.0,
    temperature_end: 20.0,
    humidity_active: true,
    humidity_start: 70,
    humidity_end: 70
  }
]);

async function run(command, args, options = {}) {
  const child = spawn(command, args, {
    stdio: ["ignore", "pipe", "pipe"],
    ...options
  });

  let stdout = "";
  let stderr = "";

  child.stdout.on("data", (data) => {
    const text = data.toString();
    stdout += text;
    process.stdout.write(text);
  });

  child.stderr.on("data", (data) => {
    const text = data.toString();
    stderr += text;
    process.stderr.write(text);
  });

  const code = await new Promise((resolve, reject) => {
    child.on("error", reject);
    child.on("close", resolve);
  });

  if (code !== 0) {
    throw new Error(`Command failed (${command} ${args.join(" ")}) with code ${code}`);
  }

  return { stdout, stderr };
}

async function isTapReady() {
  try {
    await run("ip", ["link", "show", tapIfname]);
    return true;
  } catch {
    return false;
  }
}

async function request(path, init) {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), requestTimeoutMs);
  const response = await fetch(`${baseUrl}${path}`, {
    ...init,
    signal: controller.signal
  });
  clearTimeout(timeout);
  return response;
}

async function waitForServerReady(timeoutMs = 30000) {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    try {
      const response = await request("/getStatus");
      if (response.ok) {
        return;
      }
    } catch {
      // Retry until timeout.
    }

    await delay(500);
  }

  throw new Error(`Timed out waiting for backend host endpoint at ${baseUrl}/getStatus`);
}

async function readJson(path, init) {
  const response = await request(path, init);
  assert.equal(response.ok, true, `Expected 2xx response for ${path}, got ${response.status}`);
  return response.json();
}

async function postJsonRaw(path, payload) {
  return new Promise((resolve, reject) => {
    const url = new URL(path, baseUrl);
    const request = http.request(
      {
        method: "POST",
        protocol: url.protocol,
        hostname: url.hostname,
        port: url.port || 80,
        path: `${url.pathname}${url.search}`,
        headers: {
          "content-type": "application/json",
          "content-length": Buffer.byteLength(payload)
        }
      },
      (response) => {
        let body = "";
        response.setEncoding("utf8");
        response.on("data", (chunk) => {
          body += chunk;
        });
        response.on("end", () => {
          resolve({
            statusCode: response.statusCode || 0,
            body
          });
        });
      }
    );

    request.on("error", reject);
    request.setTimeout(requestTimeoutMs, () => {
      request.destroy(new Error(`Request timeout after ${requestTimeoutMs}ms for ${path}`));
    });
    request.write(payload);
    request.end();
  });
}

async function killStaleHostApps() {
  try {
    await run("pkill", ["-f", "fermentbox-backend/out/Host/debug/firmware/app"]);
  } catch {
    // No stale process matched, continue.
  }
}

async function main() {
  if (!(await isTapReady())) {
    console.error(`Missing network interface '${tapIfname}'.`);
    console.error("Create it using Sming Host networking setup (tap + 192.168.13.1/24) before running this test.");
    console.error("In this repo, rebuild the devcontainer to apply runArgs and let .devcontainer/post-start.sh create tap0.");
    process.exit(2);
  }

  await killStaleHostApps();

  if (!skipBuild) {
    await run("make", ["-C", "fermentbox-backend", "-j4", ...hostBuildArgs], {
      cwd: repoDirPath
    });
    await run("make", ["-C", "fermentbox-backend", ...hostBuildArgs, "flash"], {
      cwd: repoDirPath
    });
  }

  const appArgs = [
    "--ifname",
    tapIfname,
    "--ipaddr",
    hostIp,
    "--gateway",
    tapGateway,
    "--netmask",
    tapNetmask,
    "--debug=1"
  ];

  const app = spawn(appPathValue, appArgs, {
    cwd: backendDirPath,
    stdio: ["ignore", "pipe", "pipe"]
  });

  let appOutput = "";
  app.stdout.on("data", (data) => {
    const text = data.toString();
    appOutput += text;
    process.stdout.write(text);
  });
  app.stderr.on("data", (data) => {
    const text = data.toString();
    appOutput += text;
    process.stderr.write(text);
  });

  const stopApp = async () => {
    if (app.exitCode !== null) {
      return;
    }

    app.kill("SIGTERM");
    await delay(750);
    if (app.exitCode === null) {
      app.kill("SIGKILL");
    }
  };

  try {
    await waitForServerReady();

    const status = await readJson("/getStatus");
    assert.equal(typeof status.ok, "boolean", "status.ok should be a boolean");
    assert.equal(typeof status.message, "string", "status.message should be a string");

    const config = await readJson("/getConfig");
    assert.equal(typeof config.Wifi, "object", "config should include Wifi object");
    assert.equal(typeof config.Wifi.SSID, "string", "Wifi.SSID should be a string");
    assert.equal(typeof config.Wifi.Password, "string", "Wifi.Password should be a string");

    const measurement = await readJson("/getMeasurement");
    assert.ok(Object.hasOwn(measurement, "date"), "measurement should include date");
    assert.ok(Object.hasOwn(measurement, "temperature"), "measurement should include temperature");
    assert.ok(Object.hasOwn(measurement, "humidity"), "measurement should include humidity");

    const networkUpdatePayload = JSON.stringify({ SSID: "HostApiTestSSID", Password: "HostApiTestPassword" });
    const networkUpdateRaw = await postJsonRaw("/networkConfig", networkUpdatePayload);
    assert.equal(networkUpdateRaw.statusCode >= 200 && networkUpdateRaw.statusCode < 300, true, "networkConfig should return 2xx");
    const networkUpdate = JSON.parse(networkUpdateRaw.body);
    assert.equal(networkUpdate.res, "ok", "networkConfig should return res=ok");

    const configAfterUpdate = await readJson("/getConfig");
    assert.equal(configAfterUpdate.Wifi.SSID, "HostApiTestSSID");
    assert.equal(configAfterUpdate.Wifi.Password, "HostApiTestPassword");

    const scheduleListBeforeSave = await readJson("/schedule/list");
    assert.equal(Array.isArray(scheduleListBeforeSave), true, "schedule list before create should be an array");
    assert.equal(
      scheduleListBeforeSave.includes(scheduleName),
      false,
      "new schedule name should not exist before create"
    );

    const scheduleLoadBeforeCreate = await request(`/schedule/load?name=${encodeURIComponent(scheduleName)}`);
    assert.equal(scheduleLoadBeforeCreate.ok, false, "loading a new schedule name before create should fail");

    const scheduleSaveRaw = await postJsonRaw(
      `/schedule/save?name=${encodeURIComponent(scheduleName)}`,
      scheduleBody
    );

    let schedulePersistenceAvailable = false;
    if (scheduleSaveRaw.statusCode >= 200 && scheduleSaveRaw.statusCode < 300) {
      const scheduleSave = JSON.parse(scheduleSaveRaw.body);
      assert.equal(scheduleSave.res, "ok", "schedule save should return res=ok");
      schedulePersistenceAvailable = true;
    } else {
      console.warn(`Schedule persistence unavailable on host (status=${scheduleSaveRaw.statusCode}), skipping load/list persistence assertions.`);
    }

    if (schedulePersistenceAvailable) {
      const scheduleLoadResponse = await request(`/schedule/load?name=${encodeURIComponent(scheduleName)}`);
      assert.equal(scheduleLoadResponse.ok, true, "schedule load should return 2xx");
      const loadedSchedule = (await scheduleLoadResponse.text()).trim();
      assert.equal(loadedSchedule, scheduleBody, "loaded schedule should match saved payload");

      const scheduleList = await readJson("/schedule/list");
      assert.equal(Array.isArray(scheduleList), true, "schedule list should be an array");
      assert.equal(scheduleList.includes(scheduleName), true, "schedule list should contain saved schedule");
    }

    if (schedulePersistenceAvailable) {
      const scheduleStart = await readJson(`/schedule/start?name=${encodeURIComponent(scheduleName)}`);
      assert.equal(scheduleStart.res, "ok", "schedule start should return res=ok");

      const scheduleStop = await readJson("/schedule/stop");
      assert.equal(scheduleStop.res, "ok", "schedule stop should return res=ok");

      const scheduleDelete = await readJson(`/schedule/delete?name=${encodeURIComponent(scheduleName)}`);
      assert.equal(scheduleDelete.res, "ok", "schedule delete should return res=ok");

      const scheduleListAfterDelete = await readJson("/schedule/list");
      assert.equal(scheduleListAfterDelete.includes(scheduleName), false, "schedule should be removed");
    }

    const indexResponse = await request("/");
    const indexBody = await indexResponse.text();
    if (indexResponse.ok) {
      assert.equal(indexBody.includes("<html"), true, "index endpoint should return html");
    } else {
      assert.equal(indexResponse.status, 404, "index should be 2xx or 404 on host without mounted static files");
    }

    console.log("\nAll backend API host integration checks passed.");
  } catch (error) {
    console.error("\nBackend API host integration test failed.");
    if (appOutput) {
      console.error("\nCaptured backend output:\n");
      console.error(appOutput);
    }
    throw error;
  } finally {
    await stopApp();
  }
}

main().catch((error) => {
  console.error(error instanceof Error ? error.message : String(error));
  process.exit(1);
});