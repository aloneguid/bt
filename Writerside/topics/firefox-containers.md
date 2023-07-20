# Firefox Containers

Firefox Containers is an amazing extension by Mozilla itself (it's even [open-source](https://github.com/mozilla/multi-account-containers#readme)), which "lets you keep parts of your online life separated into colour-coded tabs.  Cookies are separated by container, allowing you to use the web with  multiple accounts". This extension is extremely popular amongst Firefox users today, and in a way that's one of the reasons I use Firefox myself. Fair enough, Chrom(e/ium) browsers do allow you to create separate profiles, but visually they always run in separate windows, and this is not something to everyone's liking.

Therefore, containers are fully supported since v3.3 in both Firefox and Waterfox browsers. By default, container support is switched off to stay compatible with older versions, and you have to enable it. Also, Firefox does not provide means to open a link in a container, therefore at the moment this is possible only by installing a supporting extension.

To enable containers, go to `Settings->Firefox Container Mode` and choose either:

- **Browser Tamer** - container switching will be handled by the Browser Tamer extension.
- **open-url-in-container** - via [Danys H's awesome "Open external links in a container" extension](https://addons.mozilla.org/en-GB/firefox/addon/open-url-in-container/).

The choice is yours. The second extension is more versatile when it comes to handling containers, and the first one is native to BT, so it not only handles containers but can forward links to BT as explained later on this page.

In terms of container support, there is no difference for %product% which extension to use in terms of functionality.

Once you switch the mode, go to `Tools->Rediscover Browsers` in order to detect configured containers.

![](ff-containers-123.png)

You can do that as many times as you want, or any time you add, delete, or rename a container in Firefox/Waterfox itself. BT will show the list of available containers as tabs.

![](ff-containers-list.png)

Then it's configuring rules as usual.

And the last thing to do is install the extension you have chosen. Depending on your choice, BT will give you a shortcut in Firefox options - just press the toolbar button which will open addon store in the correct browser to install it.

the last thing to do is install the extension you have chosen. Depending on your choice, BT will give you a shortcut in Firefox options - just press the toolbar button which will open addon store in the correct browser to install it.

![](ff-containers-xcmd.png)