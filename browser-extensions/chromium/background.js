const BtProtoPrefix = "x-bt://";

async function openInBT(tabId, url) {

  const destUrl = BtProtoPrefix + url;

  console.log("opening " + destUrl);

  await chrome.tabs.update(tabId, {
    url: destUrl
  });
}

chrome.action.onClicked.addListener((tab) => {
  openInBT(tab.id, tab.url);
});

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