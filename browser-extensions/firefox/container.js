const ProtocolPrefix = 'ext+bt:'

function error(e) {
    console.error(e)

    document.getElementById('internalErrorBody').textContent = e
    document.getElementById('internalErrorContainer').classList.remove('hidden')
}

function parseParams(hash) {
    if(hash.length < ProtocolPrefix.length + 1 || hash[0] !== '#') {
        throw new Error('invalid location hash');
    }

    const uri = decodeURIComponent(hash.substring(1));

    console.log(uri);

    if (!uri.startsWith(ProtocolPrefix)) {
        throw new Error('unknown URI protocol')
    }

    const qs = new URLSearchParams(uri.substring(ProtocolPrefix.length));
    console.log(qs);

    // checking qs: qs.has("...") -> true/false; qs.get("...") -> value

    if(!qs.has("url")) {
        throw new Error("missing url");
    }

    if(!qs.has("container")) {
        throw new Error("missing container name");
    }

    return {
        "url": qs.get("url"),
        "container": qs.get("container")
    }
}

async function getContainerByName(name) {
    const containers = await browser.contextualIdentities.query({
        name: name,
    })

    if (containers.length >= 1) {
        return containers[0]
    }

    return null
}

async function newTab(container, url){

    let currentTab = await browser.tabs.getCurrent();

    let createTabParams = {
        cookieStoreId: container.cookieStoreId,
        url: url,
        index: currentTab.index + 1
    }

    await browser.tabs.create(createTabParams);
    await browser.tabs.remove(currentTab.id);
}

async function openTabInContainer(url, containerName){
    const container = await getContainerByName(containerName);
    if(!container) {
        throw new Error("there is no container called '" + containerName + "'");
    }

    await newTab(container, url);
}

async function main() {

    // URL looks like:
    // moz-extension://11371b7e-1db3-4930-a223-ac0d0bd3b50f/container.html#ext%2Bbt%3Aname%3DMyContainer%26url%3Dhttps%3A%2F%2Fmozilla.org

    try {
        // console.log(window.location)
        const params = parseParams(window.location.hash);
        console.log(params);

        await openTabInContainer(params.url, params.container);

    } catch (e) {
        error(e)
    }
}

main()