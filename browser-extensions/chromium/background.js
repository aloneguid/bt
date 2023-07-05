
chrome.action.onClicked.addListener((tab) => {
  var url = tab.url;

  console.log("clicked from toolbar: " + url);

  chrome.tabs.update(tab.id, {
    url: "x-bt://" + url
  });
});

chrome.contextMenus.onClicked.addListener((info, tab) => {
  var url = info.linkUrl;

  console.log("clicked from context menu: " + url);

  chrome.tabs.update(tab.id, {
    url: "x-bt://" + url
  });
});

chrome.runtime.onInstalled.addListener(() => {

  chrome.contextMenus.create({
    id: "bt-menu-item",
    title: "Open with Browser Tamer",
    contexts: ["link"]
  });

});