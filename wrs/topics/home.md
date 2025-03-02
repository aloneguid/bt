# Welcome to %product%!

## Pitch
Do you have multiple browsers or browser profiles on your machine? Do you want to open different links with different browsers or profiles without switching manually?

If yes, then you need %product%! It's a smart and powerful tool that acts as a browser proxy on your machine. It catches the links you click and redirects them to a browser or browser profile of your choice. You can set up rules based on the link’s domain, protocol, file extension and more.

![](one.png){border-effect=line}

## Features at a Glance

- Extremely lightweight on memory and resources, written in safe modern C++.
  - Self-contained single `.exe` under 2 Mb in size with no dependencies.
  - Available as `.msi` [installer](https://aloneguid.github.io/bt/install-msi.html).
  - [Portable mode](https://aloneguid.github.io/bt/portable-mode.html) supported.
  - Fits on a floppy disk if you can find it in 2023! 💾
- Completely free and [open-source](https://github.com/aloneguid/bt).
- Intelligent detection of the most popular browsers.
  - **Firefox**, **Edge**, **Chrome**, **Brave** and so on.
  - Detection of browser profiles.
  - Support for [Firefox Containers](https://aloneguid.github.io/bt/firefox-containers.html).
  - Support for incognito mode / tor mode.
- Supports Microsoft Store apps.
- Special support for [Arc browser.](https://arc.net/)
- Add your own, custom browser or application customised with any parameters you want.
- Rule-based redirect based on matching inside entire URL, domain, or path. This can be a simple case-insensitive substring or a regular expression.
- Rule-based redirect based on [window title and process name](https://aloneguid.github.io/bt/rules.html#matching-locations).
- Assign rule priorities.
- Supports [URL processing](https://aloneguid.github.io/bt/url-proc.html), which allows for custom URL pre-processing with
  - [URL un-shortening](https://aloneguid.github.io/bt/url-proc.html#un-shortening).
  - [Office 365 URL unwrapping](https://aloneguid.github.io/bt/url-proc.html#office-365-link-unwrapping).
  - [Find/replace](http://localhost:63342/wrs/preview/url-proc.html#substitutions) functionality, including regular expression support.

- **[Extensions](https://aloneguid.github.io/bt/browser-extensions.html)** to integrate with Chrome, Edge, Firefox, or any Chromium-based or Firefox-based browser (Opera, Vivaldi, Brave, Waterfox, LibreWolf etc.).
  [![Chrome Web Store Version](https://img.shields.io/chrome-web-store/v/oggcljknmiiomjekepdoindjcpnpglnd)](https://chrome.google.com/webstore/detail/browser-tamer/oggcljknmiiomjekepdoindjcpnpglnd)  [![Mozilla Add-on Version](https://img.shields.io/amo/v/browser-tamer)](https://addons.mozilla.org/eu/firefox/addon/browser-tamer/)
- Open links in ["chromeless (frameless)" window](https://aloneguid.github.io/bt/rules.html#frameless-windows).
- Optional [audit](https://aloneguid.github.io/bt/config-basic.html#audit) of rule hits to a csv file.
- Beautiful UI based on [ImGui](https://github.com/ocornut/imgui) (GPU-accelerated UI engine used in games) with support for themes.

Some technical details on how %product% works:

<video src="https://youtu.be/S9aVpDSSOWM" preview-src="intro-video-preview.png"/>

## Feedback and support
Please report any issues, usability improvements, or feature requests to my 
<a href="https://github.com/aloneguid/bt">GitHub project</a> (you need to have a GitHub account).

You can also always send an email to [c5t6bnyu7@mozmail.com](mailto:c5t6bnyu7@mozmail.com).
