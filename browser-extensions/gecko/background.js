const BtProtoPrefix = "x-bt://";

async function openInBT(url) {
    const destUrl = BtProtoPrefix + url;
    const newTab = await browser.tabs.create({ url: destUrl });
    // Gecko will no close x-bt:// tabs automatically
    await browser.tabs.remove(newTab.id);
}

// https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/API/browserAction/onClicked#addlistener_syntax
browser.browserAction.onClicked.addListener((activeTab) => {
    const url = activeTab.url;
    // as we are sending the url to BT, we can close the current tab
    browser.tabs.remove(activeTab.id);

    openInBT(url);
});

browser.contextMenus.onClicked.addListener((info, tab) => {
    const url = info.linkUrl;
    openInBT(url);
});

// install context menu (
browser.runtime.onInstalled.addListener(() => {
    browser.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });
});