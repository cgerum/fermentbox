function encodeQueryData(data) {
  const ret = [];
  for (let d in data)
    ret.push(encodeURIComponent(d) + "=" + encodeURIComponent(data[d]));
  return ret.join("&");
}

function sendRequest(url, params = {}, body = "") {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    xhr.timeout = 1000
    xhr.onreadystatechange = () => {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          if (xhr.responseText !== "") {
            resolve(JSON.parse(xhr.responseText));
          } else {
            resolve(null);
          }
        } else {
          reject(xhr.statusText);
        }
      }
    };
    let type = "GET";
    if (body !== "") {
      type = "POST";
    }

    if (params !== {}) {
      const query_data = encodeQueryData(params);
      url = url + "?" + query_data;
    }

    xhr.open(type, url, true);

    if (body !== "") {
      xhr.setRequestHeader("Content-Type", "application/json");
    }
    xhr.send(body);
  });
}

export default sendRequest;
