// load and display configuration
chrome.storage.sync.get(["interceptAll"]).then((result) => {
    document.getElementById("interceptAll").checked = result.interceptAll || false;
});

// save configuration when checkbox is clicked
document.getElementById("interceptAll").addEventListener("click", function () {
    const value = document.getElementById("interceptAll").checked;
    chrome.storage.sync.set({"interceptAll": value}).then(() => {
        console.log("interceptAll -> " + value);
    });
});

// open current tab in BT
document.getElementById("openPage").addEventListener("click", function () {
    getCurrentTab()
        .then((tab) => {
           openInBT(tab.id, tab.url);
        });
});

