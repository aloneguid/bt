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

/*
async function beforeNav(details){

  let activeTabs = await chrome.tabs.query({currentWindow: true, active: true});
  let tab = activeTabs[0];

  // to read tab.url you need "tabs" permission

  console.log("from:");
  console.log(tab.url);
  console.log("to:")
  console.log(details);

  // await chrome.tabs.update(details.tabId, {url: "http://microsoft.com"});

  // openInBT(details.tabId, details.url);
}

chrome.webNavigation.onBeforeNavigate.addListener(beforeNav);
*/

// add context menu item for a hyperlink (contexts: link)
chrome.runtime.onInstalled.addListener(() => {

  chrome.contextMenus.create({
    id: "bt-menu-item",
    title: "Open with Browser Tamer",
    contexts: ["link"]
  });
});