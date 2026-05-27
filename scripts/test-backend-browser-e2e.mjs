#!/usr/bin/env node

import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { mkdir, writeFile } from "node:fs/promises";
import path from "node:path";
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

async function request(pathname, init = {}) {
    return fetch(`${baseUrl}${pathname}`, {
        ...init,
        signal: AbortSignal.timeout(requestTimeoutMs)
    });
}

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

async function killStaleHostApps() {
    try {
        await run("pkill", ["-f", "fermentbox-backend/out/Host/debug/firmware/app"]);
    } catch {
        // No stale process matched, continue.
    }
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

function getAssetPathsFromHtml(html) {
    const references = Array.from(html.matchAll(/(?:src|href)=["']([^"']+)["']/gi), ([, assetPath]) => assetPath);
    const assets = references
        .filter((assetPath) => assetPath.startsWith("/") && !assetPath.startsWith("//"))
        .filter((assetPath) => assetPath.endsWith(".js") || assetPath.endsWith(".css"));
    return [...new Set(assets)];
}

async function verifyFrontendAssets() {
    const response = await request("/");
    assert.equal(response.ok, true, `Expected 2xx response for /, got ${response.status}`);
    const html = await response.text();

    const assets = getAssetPathsFromHtml(html);
    assert.equal(assets.length > 0, true, "Expected index page to reference at least one JS/CSS asset");

    for (const assetPath of assets) {
        const assetResponse = await request(assetPath);
        assert.equal(assetResponse.ok, true, `Expected 2xx response for asset ${assetPath}, got ${assetResponse.status}`);
    }
}

async function main() {
    await ensureTap();
    await killStaleHostApps();

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

    let browser;
    let page;
    const pageErrors = [];
    const consoleMessages = [];
    const failedRequests = [];

    try {
        await waitForServerReady();
        await verifyFrontendAssets();

        browser = await chromium.launch({ headless: true });
        page = await browser.newPage();
        page.setDefaultTimeout(browserUiTimeoutMs);
        page.setDefaultNavigationTimeout(browserNavigationTimeoutMs);
        page.on("pageerror", (error) => {
            const message = error instanceof Error ? error.stack || error.message : String(error);
            pageErrors.push(message);
            console.error(`[playwright:pageerror] ${message}`);
        });
        page.on("console", (message) => {
            const formatted = `[${message.type()}] ${message.text()}`;
            consoleMessages.push(formatted);
            console.log(`[playwright:console] ${formatted}`);
        });
        page.on("requestfailed", (request) => {
            const failure = request.failure();
            const formatted = `${request.method()} ${request.url()} (${failure?.errorText || "unknown failure"})`;
            failedRequests.push(formatted);
            console.error(`[playwright:requestfailed] ${formatted}`);
        });

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

        console.log("Browser e2e checks passed.");
    } catch (error) {
        if (page) {
            const artifactDir = "/tmp/fermentbox-browser-e2e-artifacts";
            const artifactPrefix = `failure-${Date.now()}`;
            const screenshotPath = path.join(artifactDir, `${artifactPrefix}.png`);
            const htmlPath = path.join(artifactDir, `${artifactPrefix}.html`);

            await mkdir(artifactDir, { recursive: true });
            try {
                await page.screenshot({ path: screenshotPath, fullPage: true });
                console.error(`Saved failure screenshot: ${screenshotPath}`);
            } catch (screenshotError) {
                console.error(`Failed to capture screenshot: ${screenshotError instanceof Error ? screenshotError.message : String(screenshotError)}`);
            }
            try {
                const html = await page.content();
                await writeFile(htmlPath, html, "utf8");
                console.error(`Saved failure page HTML: ${htmlPath}`);
            } catch (htmlError) {
                console.error(`Failed to capture page HTML: ${htmlError instanceof Error ? htmlError.message : String(htmlError)}`);
            }
        }

        if (pageErrors.length > 0) {
            console.error(`Collected page errors:\n${pageErrors.join("\n")}`);
        }
        if (failedRequests.length > 0) {
            console.error(`Collected failed requests:\n${failedRequests.join("\n")}`);
        }
        if (consoleMessages.length > 0) {
            console.error(`Collected console messages:\n${consoleMessages.join("\n")}`);
        }
        throw error;
    } finally {
        if (browser) {
            await browser.close();
        }
        await stopApp();
    }
}

main().catch((error) => {
    console.error(error instanceof Error ? error.message : String(error));
    process.exit(1);
});
