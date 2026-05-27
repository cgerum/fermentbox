jest.mock("../../src/services/api.js", () => ({
    __esModule: true,
    default: {
        getMeasurement: jest.fn(),
        getStatus: jest.fn()
    }
}));

jest.mock("../../src/event-bus.js", () => ({
    __esModule: true,
    default: {
        $emit: jest.fn()
    }
}));

import App from "../../src/App.vue";
import apiService from "../../src/services/api.js";
import EventBus from "../../src/event-bus.js";

describe("App", () => {
    it("setPage updates page and closes drawer", () => {
        const vm = {
            page: "controls",
            drawer: true
        };

        App.methods.setPage.call(vm, "schedule");

        expect(vm.page).toBe("schedule");
        expect(vm.drawer).toBe(false);
    });

    it("startMeasurement polls and emits measurements", async () => {
        jest.useFakeTimers();
        apiService.getMeasurement.mockResolvedValue({ date: 111, temperature: 22 });

        const vm = {
            measurements: [],
            measurementTimer: 0
        };

        App.methods.startMeasurement.call(vm);

        jest.advanceTimersByTime(2500);
        await Promise.resolve();

        expect(apiService.getMeasurement).toHaveBeenCalledTimes(1);
        expect(vm.measurements).toEqual([{ date: 111, temperature: 22 }]);
        expect(EventBus.$emit).toHaveBeenCalledWith("new-measurement", {
            date: 111,
            temperature: 22
        });
    });

    it("startMonitor updates status and handles failures", async () => {
        jest.useFakeTimers();
        apiService.getStatus
            .mockResolvedValueOnce({ ok: true, message: "all good" })
            .mockRejectedValueOnce(new Error("network"));

        const vm = {
            status: { ok: true, message: "" },
            statusTimer: 0
        };

        App.methods.startMonitor.call(vm);

        jest.advanceTimersByTime(2000);
        await Promise.resolve();
        expect(vm.status).toEqual({ ok: true, message: "all good" });

        jest.advanceTimersByTime(2000);
        await Promise.resolve();
        await Promise.resolve();
        expect(vm.status).toEqual({ ok: false, message: "Could not get Status!" });
    });

    it("created starts both monitor loops", () => {
        const vm = {
            startMeasurement: jest.fn(),
            startMonitor: jest.fn()
        };

        App.created.call(vm);

        expect(vm.startMeasurement).toHaveBeenCalledTimes(1);
        expect(vm.startMonitor).toHaveBeenCalledTimes(1);
    });

    it("beforeDestroy clears active timers", () => {
        const clearSpy = jest.spyOn(global, "clearInterval");
        const vm = {
            measurementTimer: 11,
            statusTimer: 22
        };

        App.beforeDestroy.call(vm);

        expect(clearSpy).toHaveBeenCalledWith(11);
        expect(clearSpy).toHaveBeenCalledWith(22);
        clearSpy.mockRestore();
    });
});
