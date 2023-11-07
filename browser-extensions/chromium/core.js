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

    console.log("opening " + destUrl);

    await chrome.tabs.update(tabId, {
        url: destUrl
    });
}

// gets current active tab of this browser by querying chrome.tabs
async function getCurrentTab() {
    const tabs = await chrome.tabs.query({
        active: true,
        currentWindow: true
    });
    return tabs[0];
}