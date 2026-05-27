import sendRequest from "../requests.js";

const apiService = {
    getMeasurement() {
        return sendRequest("getMeasurement");
    },

    getStatus() {
        return sendRequest("getStatus");
    },

    getConfig() {
        return sendRequest("getConfig");
    },

    schedule: {
        list() {
            return sendRequest("schedule/list");
        },

        load(name) {
            return sendRequest("schedule/load", { name });
        },

        save(name, body = "[]") {
            return sendRequest("schedule/save", { name }, body);
        },

        remove(name) {
            return sendRequest("schedule/delete", { name });
        },

        start(name) {
            return sendRequest("schedule/start", { name });
        }
    }
};

export default apiService;
