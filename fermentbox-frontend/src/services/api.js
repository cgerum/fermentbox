import sendRequest from "../requests.js";
import operations from "./api-generated.js";

function callOperation(operationId, options = {}) {
    const operation = operations[operationId];
    if (!operation) {
        return Promise.reject(new Error(`Unknown API operation: ${operationId}`));
    }

    const params = options.params || {};
    const body = operation.hasBody ? (options.body || "") : "";

    return sendRequest(operation.path, params, body);
}

const apiService = {
    getMeasurement() {
        return callOperation("getMeasurement");
    },

    getStatus() {
        return callOperation("getStatus");
    },

    getConfig() {
        return callOperation("getConfig");
    },

    updateNetworkConfig(config) {
        return callOperation("updateNetworkConfig", {
            body: JSON.stringify(config || {})
        });
    },

    schedule: {
        list() {
            return callOperation("listSchedules");
        },

        load(name) {
            return callOperation("loadSchedule", { params: { name } });
        },

        save(name, body = "[]") {
            return callOperation("saveSchedule", { params: { name }, body });
        },

        remove(name) {
            return callOperation("deleteSchedule", { params: { name } });
        },

        start(name) {
            return callOperation("startSchedule", { params: { name } });
        },

        stop() {
            return callOperation("stopSchedule");
        }
    }
};

export default apiService;
