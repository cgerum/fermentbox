jest.mock("../../src/services/api.js", () => ({
    __esModule: true,
    default: {
        schedule: {
            list: jest.fn(),
            load: jest.fn(),
            save: jest.fn(),
            remove: jest.fn(),
            start: jest.fn()
        }
    }
}));

import Schedule from "../../src/components/Schedule.vue";
import apiService from "../../src/services/api.js";

describe("Schedule", () => {
    it("loadSchedules populates schedules and sets current schedule", async () => {
        apiService.schedule.list.mockResolvedValue(["B", "A"]);
        const vm = {
            schedules: [],
            currentSchedule: "",
            tasks: [],
            loadTasks: jest.fn()
        };

        Schedule.methods.loadSchedules.call(vm);
        await Promise.resolve();

        expect(apiService.schedule.list).toHaveBeenCalledTimes(1);
        expect(vm.schedules).toEqual(["A", "B"]);
        expect(vm.currentSchedule).toBe("A");
        expect(vm.loadTasks).toHaveBeenCalledTimes(1);
    });

    it("getTitle and getSubTitle format task labels", () => {
        const task = {
            controlTemperature: true,
            controlHumidity: true,
            temperature: 20,
            humidity: 65,
            duration: 24
        };

        const title = Schedule.methods.getTitle.call({}, task);
        const subtitle = Schedule.methods.getSubTitle.call({}, task);

        expect(title).toBe("20°C Temperature, 65% Humidity");
        expect(subtitle).toBe("Duration: 24 hours");
    });

    it("removeTask removes matching task id", () => {
        const vm = {
            tasks: [{ id: "1" }, { id: "2" }]
        };

        Schedule.methods.removeTask.call(vm, { id: "1" });

        expect(vm.tasks).toEqual([{ id: "2" }]);
    });
});
