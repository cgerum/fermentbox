jest.mock("../../src/services/api.js", () => ({
    __esModule: true,
    default: {
        getConfig: jest.fn(),
        updateNetworkConfig: jest.fn()
    }
}));

import Settings from "../../src/components/Settings.vue";
import apiService from "../../src/services/api.js";

describe("Settings", () => {
    beforeEach(() => {
        apiService.getConfig.mockReset();
        apiService.updateNetworkConfig.mockReset();
    });

    it("getConfig loads wifi settings", async () => {
        apiService.getConfig.mockResolvedValue({
            Wifi: {
                SSID: "TestWifi",
                Password: "secret"
            }
        });

        const vm = {
            appPassword: null,
            wifiSSID: null,
            wifiPassword: null
        };

        Settings.methods.getConfig.call(vm);
        await Promise.resolve();

        expect(apiService.getConfig).toHaveBeenCalledTimes(1);
        expect(vm.appPassword).toBe("secret");
        expect(vm.wifiSSID).toBe("TestWifi");
        expect(vm.wifiPassword).toBe("secret");
    });

    it("storeWifiConfig validates required fields", async () => {
        const vm = {
            wifiSSID: "",
            wifiPassword: "",
            wifiSaveSuccess: "",
            wifiSaveError: "",
            isSavingWifiConfig: false
        };

        await Settings.methods.storeWifiConfig.call(vm);

        expect(apiService.updateNetworkConfig).not.toHaveBeenCalled();
        expect(vm.wifiSaveError).toBe("SSID and password are required.");
    });

    it("storeWifiConfig saves and refreshes values", async () => {
        apiService.updateNetworkConfig.mockResolvedValue({ res: "ok" });

        const vm = {
            wifiSSID: "  TestWifi  ",
            wifiPassword: "  secret  ",
            wifiSaveSuccess: "",
            wifiSaveError: "",
            isSavingWifiConfig: false,
            getConfig: jest.fn(() => Promise.resolve())
        };

        await Settings.methods.storeWifiConfig.call(vm);

        expect(apiService.updateNetworkConfig).toHaveBeenCalledWith({
            SSID: "TestWifi",
            Password: "secret"
        });
        expect(vm.getConfig).toHaveBeenCalledTimes(1);
        expect(vm.wifiSaveSuccess).toBe("Wi-Fi credentials saved.");
        expect(vm.wifiSaveError).toBe("");
        expect(vm.isSavingWifiConfig).toBe(false);
    });

    it("storeAppPassword surfaces API failure", async () => {
        apiService.updateNetworkConfig.mockRejectedValue(new Error("network"));

        const vm = {
            wifiSSID: "MyWifi",
            appPassword: "secret",
            appSaveSuccess: "",
            appSaveError: "",
            isSavingAppPassword: false,
            getConfig: jest.fn(() => Promise.resolve())
        };

        await Settings.methods.storeAppPassword.call(vm);

        expect(vm.appSaveSuccess).toBe("");
        expect(vm.appSaveError).toBe("Unable to save password. Please try again.");
        expect(vm.isSavingAppPassword).toBe(false);
    });

    it("created hook fetches config", () => {
        const vm = {
            getConfig: jest.fn()
        };

        Settings.created.call(vm);

        expect(vm.getConfig).toHaveBeenCalledTimes(1);
    });
});
