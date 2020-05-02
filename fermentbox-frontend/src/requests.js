function sendRequest(url) {
    return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = () => {
            if (xhr.readyState === 4) {
                if( xhr.status === 200) {
                    resolve(JSON.parse(xhr.responseText))
                } else {
                    reject(xhr.statusText)
                }
            } 
        }
        xhr.open('GET', url, true)
        xhr.send()
    })        
}

export default sendRequest;