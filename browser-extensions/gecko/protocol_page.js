const BtProtoPrefix = "x-bt://";
const url = window.location.hash.slice(1);

console.log(`Navigating to ${BtProtoPrefix + url}`);

window.location.href = BtProtoPrefix + url;

document.addEventListener("visibilitychange", () => {
    // Close the tab on any focus change
    // User confirmed protocol => page is visible => closed
    // User canceled protocol => nothing happens => page remains open
    // Protocol was confirmed permanently => page is visible => closed
    window.close()
});