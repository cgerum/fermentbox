jest.mock("../../src/services/api.js", () => ({
    __esModule: true,
    default: {
        getConfig: jest.fn()
    }
}));

import Settings from "../../src/components/Settings.vue";
import apiService from "../../src/services/api.js";

describe("Settings", () => {
    it("getConfig loads wifi settings", async () => {
        apiService.getConfig.mockResolvedValue({
            Wifi: {
                SSID: "TestWifi",
                Password: "secret"
            }
        });

        const vm = {
            wifiSSID: null,
            wifiPassword: null
        };

        Settings.methods.getConfig.call(vm);
        await Promise.resolve();

        expect(apiService.getConfig).toHaveBeenCalledTimes(1);
        expect(vm.wifiSSID).toBe("TestWifi");
        expect(vm.wifiPassword).toBe("secret");
    });

    it("created hook fetches config", () => {
        const vm = {
            getConfig: jest.fn()
        };

        Settings.created.call(vm);

        expect(vm.getConfig).toHaveBeenCalledTimes(1);
    });
});
