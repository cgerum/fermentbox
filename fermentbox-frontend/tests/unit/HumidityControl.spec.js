import HumidityControl from "../../src/components/HumidityControl.vue";

describe("HumidityControl", () => {
    it("updates log with new humidity data", () => {
        const vm = {
            log: [],
            start: 1000
        };

        HumidityControl.methods.updateLog.call(vm, {
            date: 1010,
            humidity: 48
        });

        expect(vm.log).toEqual([{ label: 10, value: 48 }]);
    });

    it("keeps only latest 180 measurements", () => {
        const vm = {
            log: Array.from({ length: 180 }, (_, i) => ({ label: i, value: i })),
            start: 1000
        };

        HumidityControl.methods.updateLog.call(vm, {
            date: 2000,
            humidity: 55
        });

        expect(vm.log).toHaveLength(180);
        expect(vm.log[0]).toEqual({ label: 1, value: 1 });
        expect(vm.log[179]).toEqual({ label: 1000, value: 55 });
    });

    it("returns '--' current value when log is empty", () => {
        const result = HumidityControl.computed.currentValue.call({ log: [] });
        expect(result).toBe("--");
    });

    it("returns latest value when log has entries", () => {
        const result = HumidityControl.computed.currentValue.call({
            log: [{ value: 40 }, { value: 44 }]
        });
        expect(result).toBe(44);
    });
});
