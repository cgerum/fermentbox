function encodeQueryData(data) {
  const ret = [];
  for (let d in data)
    ret.push(encodeURIComponent(d) + "=" + encodeURIComponent(data[d]));
  return ret.join("&");
}

function sendRequest(url, params = {}, body = "") {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    xhr.onreadystatechange = () => {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          resolve(JSON.parse(xhr.responseText));
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
    xhr.send(body);
  });
}

export default sendRequest;
