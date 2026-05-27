import Ajv from "ajv";
import scheduleTaskSchema from "../../../api/schemas/schedule-task.json";

describe("API contract fixtures", () => {
    const ajv = new Ajv({ allErrors: true });

    it("validates schedule task fixtures against schema", () => {
        const validateTask = ajv.compile(scheduleTaskSchema);

        const validTask = {
            duration: 12,
            temperature_active: true,
            humidity_active: false,
            temperature_start: 20,
            temperature_end: 22
        };

        const invalidTask = {
            duration: 4,
            temperature_active: true,
            humidity_active: false,
            unknown_field: 10
        };

        expect(validateTask(validTask)).toBe(true);
        expect(validateTask(invalidTask)).toBe(false);
    });

    it("validates key response examples", () => {
        const measurementSchema = {
            type: "object",
            required: ["date", "temperature", "humidity"],
            additionalProperties: false,
            properties: {
                date: { type: "integer" },
                temperature: { type: "number" },
                humidity: { type: "number" }
            }
        };

        const statusSchema = {
            type: "object",
            required: ["ok", "message", "code", "codes", "dimensions"],
            additionalProperties: false,
            properties: {
                ok: { type: "boolean" },
                message: { type: "string" },
                code: { type: "string" },
                codes: {
                    type: "array",
                    items: { type: "string" }
                },
                dimensions: {
                    type: "object",
                    required: ["sensor", "network", "config", "schedule", "control"],
                    additionalProperties: false,
                    properties: {
                        sensor: {
                            type: "object",
                            required: ["code", "message", "ok"],
                            additionalProperties: false,
                            properties: {
                                code: { type: "string" },
                                message: { type: "string" },
                                ok: { type: "boolean" }
                            }
                        },
                        network: {
                            type: "object",
                            required: ["code", "message", "ok"],
                            additionalProperties: false,
                            properties: {
                                code: { type: "string" },
                                message: { type: "string" },
                                ok: { type: "boolean" }
                            }
                        },
                        config: {
                            type: "object",
                            required: ["code", "message", "ok"],
                            additionalProperties: false,
                            properties: {
                                code: { type: "string" },
                                message: { type: "string" },
                                ok: { type: "boolean" }
                            }
                        },
                        schedule: {
                            type: "object",
                            required: ["code", "message", "ok"],
                            additionalProperties: false,
                            properties: {
                                code: { type: "string" },
                                message: { type: "string" },
                                ok: { type: "boolean" }
                            }
                        },
                        control: {
                            type: "object",
                            required: ["code", "message", "ok"],
                            additionalProperties: false,
                            properties: {
                                code: { type: "string" },
                                message: { type: "string" },
                                ok: { type: "boolean" }
                            }
                        }
                    }
                }
            }
        };

        const validateMeasurement = ajv.compile(measurementSchema);
        const validateStatus = ajv.compile(statusSchema);

        expect(
            validateMeasurement({ date: 1711000000, temperature: 22.3, humidity: 55.2 })
        ).toBe(true);
        expect(
            validateStatus({
                ok: true,
                message: "No errors",
                code: "normal",
                codes: ["normal"],
                dimensions: {
                    sensor: { code: "sensor_ok", message: "Sensor readings are healthy", ok: true },
                    network: { code: "network_available", message: "Network is available", ok: true },
                    config: { code: "config_present", message: "Wi-Fi configuration loaded", ok: true },
                    schedule: { code: "schedule_inactive", message: "No schedule running", ok: true },
                    control: { code: "control_active", message: "Control loop active", ok: true }
                }
            })
        ).toBe(true);
    });
});
