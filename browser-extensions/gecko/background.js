const ProtocolPage = browser.runtime.getURL("protocol_page.html");
const TabReturnDict = new Map();

async function openInBT(url, returnTab) {
    const destUrl = `${ProtocolPage}#${url}`;
    const newTab = await browser.tabs.create({ url: destUrl, active: true, windowId: returnTab.windowId });

    if (returnTab) {
        TabReturnDict.set(newTab.id, {
            returnTo: returnTab.id,
            tabCount: (await browser.tabs.query({
                windowId: returnTab.windowId
            })).length - 1
        });
    }
}

browser.tabs.onRemoved.addListener(async (tabId, _) => {
    if (TabReturnDict.has(tabId)) {
        const returnTabId = TabReturnDict.get(tabId).returnTo;
        const returnTab = await browser.tabs.get(returnTabId);

        const currentTabs = (await browser.tabs.query({
            windowId: returnTab.windowId
        })).length;

        if (currentTabs === TabReturnDict.get(tabId).tabCount) {
            // If no new tab was opened, return to the original tab
            await browser.tabs.update(TabReturnDict.get(tabId).returnTo, { active: true });
        }
        TabReturnDict.delete(tabId);
    }
});

// https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/API/browserAction/onClicked#addlistener_syntax
browser.browserAction.onClicked.addListener(async (activeTab) => {
    const url = activeTab.url;
    // as we are sending the url to BT, we can close the current tab
    await browser.tabs.remove(activeTab.id);

    await openInBT(url);
});

browser.contextMenus.onClicked.addListener(async(info, tab) => {
    const url = info.linkUrl;
    await openInBT(url, tab);
});

// install context menu (
browser.runtime.onInstalled.addListener(() => {
    browser.contextMenus.create({
        id: "bt-menu-item",
        title: "Open with Browser Tamer",
        contexts: ["link"]
    });
});