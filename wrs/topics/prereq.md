# Prerequisites

Once launched first time, %product% will display a message that you have no browsers configured. This is expected, as you need to set up at least one browser to start using the tool.

![](no-browsers.png)

Press "discover system browsers" to populate the list of browsers installed on your system. This will also set the first browser found as the default one.

On every subsequent launch, %product% will
perform a quick self-check and warn you which actions need to be performed by you for links to be properly proxied by it.

<img alt="dashboard" height="150" src="dash-onered.png"/>

Health status is also available in the bottom status bar (green and red heart).

## Register as proxy browser

This is required in order for Windows to think that %product% is just another browser. It affects the OS dialogs allowing you to pick Browser Tamer from the list of various browsing options.

## Set as HTTP(S) protocol handler

Required in order for Windows to offer protocol handling (http and https links) by %product%. This also checks that Browser Tamer was indeed set as the default handler.

<warning>
For security reasons, %product% does not automatically fix this issue, but rather opens a Windows dialog with default handlers, where you need to do it yourself.
</warning>

The instructions are different for Windows 10 and 11, please choose your version here:

<tabs>
<tab title="Windows 11">
<procedure title="Steps">
<step>
Press Fix button, which will open Windows Defaults window.
</step>
<step>
Search for "HTTP" which should display the current default handler, like Microsoft Edge or Google Chrome.
</step>
<step>
Click on it, and select "%product%".
</step>
<step>
Go back to %product%, press "recheck". If you have done it correctly, check will pass.
</step>
</procedure>
Video demonstration:
<video src="default-win11.mp4" preview-src="default-win11.png"/>
</tab>
<tab title="Windows 10">
<procedure title="Steps">
<step>
Press Fix button, which will open Windows Defaults window.
</step>
<step>
In the search results, select <strong>Default apps</strong>.
</step>
<step>
Under <strong>Web browser</strong>, select the browser currently listed, and then select <strong>%product%</strong>. 
</step>
<step>
Go back to %product%, press "recheck". If you have done it correctly, check will pass.
</step>
</procedure>
</tab>
</tabs>
