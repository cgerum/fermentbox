#!/usr/bin/env node

import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";
import { setTimeout as delay } from "node:timers/promises";

import { chromium } from "playwright";

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
const requestTimeoutMs = Number(process.env.API_REQUEST_TIMEOUT_MS || 8000);
const hostReadyTimeoutMs = Number(process.env.HOST_READY_TIMEOUT_MS || 60000);
const browserNavigationTimeoutMs = Number(process.env.BROWSER_NAVIGATION_TIMEOUT_MS || 90000);
const browserUiTimeoutMs = Number(process.env.BROWSER_UI_TIMEOUT_MS || 90000);
const skipBuild = process.env.SKIP_HOST_BUILD === "1";

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

async function ensureTap() {
    try {
        await run("ip", ["link", "show", tapIfname]);
        return;
    } catch {
        // Create on demand when running locally.
    }

    await run("sudo", ["ip", "tuntap", "add", "dev", tapIfname, "mode", "tap", "user", process.env.USER || "node"]);
    await run("sudo", ["ip", "address", "add", `${tapGateway}/24`, "dev", tapIfname]);
    await run("sudo", ["ip", "link", "set", tapIfname, "up"]);
}

async function waitForServerReady(timeoutMs = hostReadyTimeoutMs) {
    const deadline = Date.now() + timeoutMs;

    while (Date.now() < deadline) {
        try {
            const response = await fetch(`${baseUrl}/getStatus`, { signal: AbortSignal.timeout(requestTimeoutMs) });
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

async function main() {
    await ensureTap();

    if (!skipBuild) {
        await run("make", ["frontend"], { cwd: repoDirPath });
        await run("make", ["-C", "fermentbox-backend", "-j4", "SMING_ARCH=Host"], { cwd: repoDirPath });
        await run("make", ["-C", "fermentbox-backend", "SMING_ARCH=Host", "flash"], { cwd: repoDirPath });
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

        const browser = await chromium.launch({ headless: true });
        const page = await browser.newPage();
        page.setDefaultTimeout(browserUiTimeoutMs);
        page.setDefaultNavigationTimeout(browserNavigationTimeoutMs);

        await page.goto(baseUrl, { waitUntil: "domcontentloaded", timeout: browserNavigationTimeoutMs });
        await assert.doesNotReject(async () => page.getByText("Ferment Box").waitFor({ state: "visible", timeout: browserUiTimeoutMs }));
        await assert.doesNotReject(async () => page.getByText("Controls").waitFor({ state: "visible", timeout: browserUiTimeoutMs }));
        await assert.doesNotReject(async () => page.getByText("Temperature").waitFor({ state: "visible", timeout: browserUiTimeoutMs }));

        await page.getByTestId("nav-drawer-toggle").click();
        await page.getByText("Settings").click();
        await assert.doesNotReject(async () => page.getByText("Wi-Fi").waitFor({ state: "visible", timeout: browserUiTimeoutMs }));

        await page.getByLabel("SSID").fill("BrowserHostSSID");
        await page.getByLabel("Password").nth(1).fill("BrowserHostPassword");

        await page.getByRole("button", { name: "Save WIFI Credentials" }).click();
        await assert.doesNotReject(async () => page.getByText("Wi-Fi credentials saved.").waitFor({ state: "visible", timeout: browserUiTimeoutMs }));

        await page.getByTestId("nav-drawer-toggle").click();
        await page.getByText("Schedule").click();
        await assert.doesNotReject(async () => page.getByText("Schedule").waitFor({ state: "visible", timeout: browserUiTimeoutMs }));

        await browser.close();
        console.log("Browser e2e checks passed.");
    } finally {
        await stopApp();
    }
}

main().catch((error) => {
    console.error(error instanceof Error ? error.message : String(error));
    process.exit(1);
});
