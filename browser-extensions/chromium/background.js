/*
 * Permissions:
 * - contextMenus: add context menu item "open with bt"
 * - tabs: read current tab URL to send to BT.
 * - activeTab: update current tab URL to send to BT.
 * - storage: store settings.
 */

const BtProtoPrefix = "x-bt://";

async function openInBT(tabId, url) {
    const destUrl = BtProtoPrefix + url;
    await chrome.tabs.update(tabId, { url: destUrl });
}

chrome.action.onClicked.addListener((activeTab) => {
    const url = activeTab.url;
    // as we are sending the url to BT, we can close the current tab
    openInBT(activeTab.id, url).then(() => {
        // using a timeout because I can't find a reliable way to wait for the custom protocol to be handled
        setTimeout(() => {
            chrome.tabs.remove(activeTab.id);
        }, 250);
    });
});

// context menu item click handler
chrome.contextMenus.onClicked.addListener((info, tab) => {
    const url = info.linkUrl;
    openInBT(tab.id, url);
});

// add context menu item for a hyperlink (contexts: link)
chrome.runtime.onInstalled.addListener(() => {
    chrome.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });
});