import sendRequest from "../../src/requests";

class MockXHR {
    constructor() {
        this.readyState = 0;
        this.status = 0;
        this.responseText = "";
        this.timeout = 0;
        this.statusText = "";
        this.onreadystatechange = null;
        this.headers = {};
    }

    open(method, url) {
        this.method = method;
        this.url = url;
    }

    setRequestHeader(key, value) {
        this.headers[key] = value;
    }

    send(body) {
        this.body = body;
    }
}

describe("sendRequest", () => {
    let xhr;

    beforeEach(() => {
        global.XMLHttpRequest = jest.fn(() => {
            xhr = new MockXHR();
            return xhr;
        });
    });

    it("resolves parsed JSON response when status is 200", async () => {
        const promise = sendRequest("getStatus");

        xhr.status = 200;
        xhr.responseText = '{"ok":true}';
        xhr.readyState = 4;
        xhr.onreadystatechange();

        await expect(promise).resolves.toEqual({ ok: true });
    });

    it("resolves null on empty 200 response", async () => {
        const promise = sendRequest("schedule/start");

        xhr.status = 200;
        xhr.responseText = "";
        xhr.readyState = 4;
        xhr.onreadystatechange();

        await expect(promise).resolves.toBeNull();
    });

    it("rejects on non-200 response", async () => {
        const promise = sendRequest("getStatus");

        xhr.status = 500;
        xhr.statusText = "Internal Error";
        xhr.readyState = 4;
        xhr.onreadystatechange();

        await expect(promise).rejects.toBe("Internal Error");
    });
});
