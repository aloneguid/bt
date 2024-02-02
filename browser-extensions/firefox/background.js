function makeXbtUrl(url) {
    return "x-bt://" + url;
}

browser.browserAction.onClicked.addListener((tab) => {
    var url = tab.url;

    console.log("clicked from toolbar: " + url);

    browser.tabs.update(tab.id, {
        url: makeXbtUrl(url)
    });
});

browser.contextMenus.onClicked.addListener((info, tab) => {
    var url = info.linkUrl;

    console.log("clicked from context menu: " + url);

    browser.tabs.update(tab.id, {
        url: makeXbtUrl(url)
    });
});

browser.runtime.onInstalled.addListener(() => {

    browser.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });

});