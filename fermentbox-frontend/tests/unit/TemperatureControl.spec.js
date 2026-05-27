import TemperatureControl from "../../src/components/TemperatureControl.vue";

describe("TemperatureControl", () => {
    it("updates log with new temperature data", () => {
        const vm = {
            log: [],
            start: 1000
        };

        TemperatureControl.methods.updateLog.call(vm, {
            date: 1010,
            temperature: 21.5
        });

        expect(vm.log).toEqual([{ label: 10, value: 21.5 }]);
    });

    it("keeps only latest 180 measurements", () => {
        const vm = {
            log: Array.from({ length: 180 }, (_, i) => ({ label: i, value: i })),
            start: 1000
        };

        TemperatureControl.methods.updateLog.call(vm, {
            date: 2000,
            temperature: 99
        });

        expect(vm.log).toHaveLength(180);
        expect(vm.log[0]).toEqual({ label: 1, value: 1 });
        expect(vm.log[179]).toEqual({ label: 1000, value: 99 });
    });

    it("returns '--' current value when log is empty", () => {
        const result = TemperatureControl.computed.currentValue.call({ log: [] });
        expect(result).toBe("--");
    });

    it("returns latest value when log has entries", () => {
        const result = TemperatureControl.computed.currentValue.call({
            log: [{ value: 15 }, { value: 18 }]
        });
        expect(result).toBe(18);
    });
});
