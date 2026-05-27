jest.mock("../../src/requests.js", () => jest.fn(() => Promise.resolve(null)));

import apiService from "../../src/services/api.js";
import sendRequest from "../../src/requests.js";

describe("apiService contract wiring", () => {
    beforeEach(() => {
        sendRequest.mockClear();
    });

    it("maps telemetry endpoints through generated operations", async () => {
        await apiService.getMeasurement();
        await apiService.getStatus();
        await apiService.getConfig();

        expect(sendRequest).toHaveBeenNthCalledWith(1, "getMeasurement", {}, "");
        expect(sendRequest).toHaveBeenNthCalledWith(2, "getStatus", {}, "");
        expect(sendRequest).toHaveBeenNthCalledWith(3, "getConfig", {}, "");
    });

    it("maps schedule endpoints with expected query and body", async () => {
        await apiService.schedule.list();
        await apiService.schedule.load("alpha");
        await apiService.schedule.save("alpha", "[]");
        await apiService.schedule.remove("alpha");
        await apiService.schedule.start("alpha");
        await apiService.schedule.stop();

        expect(sendRequest).toHaveBeenNthCalledWith(1, "schedule/list", {}, "");
        expect(sendRequest).toHaveBeenNthCalledWith(2, "schedule/load", { name: "alpha" }, "");
        expect(sendRequest).toHaveBeenNthCalledWith(3, "schedule/save", { name: "alpha" }, "[]");
        expect(sendRequest).toHaveBeenNthCalledWith(4, "schedule/delete", { name: "alpha" }, "");
        expect(sendRequest).toHaveBeenNthCalledWith(5, "schedule/start", { name: "alpha" }, "");
        expect(sendRequest).toHaveBeenNthCalledWith(6, "schedule/stop", {}, "");
    });
});
