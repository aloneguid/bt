/*
 * Permissions:
 * - contextMenus: add context menu item "open with bt"
 * - tabs: read current tab URL to send to BT.
 * - activeTab: update current tab URL to send to BT.
 * - storage: store settings.
 */

const BtProtoPrefix = "x-bt://";

async function openInBT(url) {
    const destUrl = BtProtoPrefix + url;
    await chrome.tabs.create({ url: destUrl });
}

chrome.action.onClicked.addListener((activeTab) => {
    const url = activeTab.url;
    // as we are sending the url to BT, we can close the current tab
    chrome.tabs.remove(activeTab.id);

    openInBT(url);
});

// context menu item click handler
chrome.contextMenus.onClicked.addListener((info, tab) => {
    const url = info.linkUrl;
    openInBT(url);
});

// add context menu item for a hyperlink (contexts: link)
chrome.runtime.onInstalled.addListener(() => {
    chrome.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });
});