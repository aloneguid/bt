importScripts("core.js");

// context menu item click handler
chrome.contextMenus.onClicked.addListener((info, tab) => {
    openInBT(tab.id, info.linkUrl);
});

// add context menu item for a hyperlink (contexts: link)
chrome.runtime.onInstalled.addListener(() => {

    chrome.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });
});