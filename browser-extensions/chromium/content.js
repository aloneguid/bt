// This script is injected into a page when it is loaded, and it intercepts clicks on hyperlinks.

document.body.addEventListener("click", (event) => {
    if (event.target.tagName === "A") {

        chrome.storage.sync.get(["interceptAll"]).then((result) => {
            const doIntercept = result.interceptAll;
            if (doIntercept) {
                const url = event.target.href;

                if (url.includes("#")) {
                    console.log("skipping anchor link: " + url);
                } else {
                    console.log("blocking link: " + url);
                    event.preventDefault(); // Prevent the default link behavior (navigation)
                    openInBT(getCurrentTab().id, url);
                }
            }
        });
    }
});