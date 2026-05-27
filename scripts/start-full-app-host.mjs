#!/usr/bin/env node

import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";
import { setTimeout as delay } from "node:timers/promises";

const repoDir = new URL("../", import.meta.url);
const backendDir = new URL("../fermentbox-backend/", import.meta.url);
const appPath = new URL("./out/Host/debug/firmware/app", backendDir);

const repoDirPath = fileURLToPath(repoDir);
const backendDirPath = fileURLToPath(backendDir);
const appPathValue = fileURLToPath(appPath);

const tapIfname = process.env.SMING_HOST_TAP_IFNAME || "tap0";
const hostIp = process.env.SMING_HOST_IP || "192.168.13.2";
const tapGateway = process.env.SMING_HOST_GATEWAY || "192.168.13.1";
const tapNetmask = process.env.SMING_HOST_NETMASK || "255.255.255.0";
const baseUrl = process.env.SMING_HOST_BASE_URL || `http://${hostIp}`;

async function run(command, args, options = {}) {
  const child = spawn(command, args, {
    stdio: ["ignore", "pipe", "pipe"],
    ...options
  });

  child.stdout.on("data", (data) => process.stdout.write(data));
  child.stderr.on("data", (data) => process.stderr.write(data));

  const code = await new Promise((resolve, reject) => {
    child.on("error", reject);
    child.on("close", resolve);
  });

  if (code !== 0) {
    throw new Error(`Command failed (${command} ${args.join(" ")}) with code ${code}`);
  }
}

async function ensureTap() {
  try {
    await run("ip", ["link", "show", tapIfname]);
  } catch {
    await run("sudo", ["ip", "tuntap", "add", "dev", tapIfname, "mode", "tap", "user", process.env.USER || "node"]);
    await run("sudo", ["ip", "address", "add", `${tapGateway}/24`, "dev", tapIfname]);
    await run("sudo", ["ip", "link", "set", tapIfname, "up"]);
  }
}

async function waitForServerReady(timeoutMs = 30000) {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    try {
      const response = await fetch(`${baseUrl}/getStatus`);
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

async function openBrowser(url) {
  const browserCommand = process.env.BROWSER || "xdg-open";
  const browser = spawn(browserCommand, [url], { stdio: "inherit", detached: true });
  browser.unref();
}

async function main() {
  await ensureTap();
  await run("make", ["frontend"], { cwd: repoDirPath });
  await run("make", ["-C", "fermentbox-backend", "-j4", "SMING_ARCH=Host"], { cwd: repoDirPath });
  await run("make", ["-C", "fermentbox-backend", "SMING_ARCH=Host", "flash"], { cwd: repoDirPath });

  const app = spawn(appPathValue, [
    "--ifname", tapIfname,
    "--ipaddr", hostIp,
    "--gateway", tapGateway,
    "--netmask", tapNetmask,
    "--debug=1"
  ], {
    cwd: backendDirPath,
    stdio: ["ignore", "pipe", "pipe"]
  });

  app.stdout.on("data", (data) => process.stdout.write(data));
  app.stderr.on("data", (data) => process.stderr.write(data));

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
    console.log(`Full app is ready at ${baseUrl}`);
    await openBrowser(baseUrl);
    console.log("Leave this process running while you inspect the UI. Press Ctrl+C to stop.");

    await new Promise(() => {
      process.on("SIGINT", async () => {
        await stopApp();
        process.exit(0);
      });
      process.on("SIGTERM", async () => {
        await stopApp();
        process.exit(0);
      });
    });
  } finally {
    await stopApp();
  }
}

main().catch((error) => {
  console.error(error instanceof Error ? error.message : String(error));
  process.exit(1);
});