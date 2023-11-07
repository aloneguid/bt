importScripts("core.js");

// context menu item click handler
chrome.contextMenus.onClicked.addListener((info, tab) => {
    openInBT(tab.id, info.linkUrl);
});

// chrome.tabs.onUpdated.addListener((tabId, changeInfo, tab) => {
//     if (changeInfo.status === "complete") {
//         // requires "scripting" permission and "activeTab" permission, plus host permissions for the tab URL (patterns)
//         console.log(`injecting into ${tab.url}`);
//         chrome.scripting
//             .executeScript({
//                 target: {tabId: tabId},
//                 func: interceptLinkClicks,
//             })
//             .then(
//                 () => console.log("injected."),
//                 (err) => console.log(`failed to inject: ${err}`));
//     }
// });

// add context menu item for a hyperlink (contexts: link)
chrome.runtime.onInstalled.addListener(() => {

    chrome.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });
});