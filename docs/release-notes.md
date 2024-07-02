## 4.0.0

- Multiple picker hotkeys can be configured, you can even turn them all on at the same time.
- "URL tester" is now built into main configuration window and highlights matched browsers, profiles and rules. This is to avoid confusion and make it more ergonomic.
- URL Picker is completely rewritten. Browsers and profiles are now listed in 2D grid, removing the need to sort flat list by some means like popularity or user-defined order.
- Updates to latest version of ImGui and MSVC compiler.

## 3.7.3

### New feature

- Added support for opening links in "no container" in Firefox container mode. Thanks to @benaduggan in #69.
- [LibreWolf](https://librewolf.net/) is now recognized as a Firefox-compatible browser.

## 3.7.2

### Bugs fixed
- Latest version of Firefox omits ':' character from the URL when it's passed to the browser. This was causing BT to not be able to open URLs from Firefox. Thanks to @jnv in #68.
- Chrome incognito mode was not working due to a bug in the way BT was passing arguments to the browser. Thanks to @magesh-memorres in #66.

## 3.7.1

### Improvements

- Hopefully fixed browser being backgrounded for some users (#39).

## 3.7.0

### New features

- Introducing **URL Pipeline**. BT now allows you to to define custom find and replace rules on URLs before applying rules to them. Simple strings and regular expressions are supported. Read more about this in the [official docs](https://aloneguid.github.io/bt/url-pipeline.html).

### Improvements

- Upgraded dependencies to latest.
- Un-shortener supports `vrch.at` shortlinks, by @Bluscream in #62.

### Bugs fixed

- BT crashed with unhandled exception when regular expression rule was executing a regex with invalid syntax (#59). Thanks @egerlach.
- Pipe `|` and colon `:` in rule values were screwing up configuration (#59, 67). Thanks to @liny01-nbsa and @egerlach.
- Empty rule would make URL Tester hang.

> As you may have noticed, the number of releases went down in frequency. This is mostly due to the fact I wanted to sort out AV false-positives by contacting AV vendors one by one, and this is a long and tedious process which has nothing to do with technology.
>
> Remember, if you like this project and wish and can to support it, you can always star :star: it, which helps a lot in terms of spreading the word. Optionally, if you can, you can :coffee: [buy me a coffee](https://www.buymeacoffee.com/alonecoffee) as well.

## 3.6.2

🐞Fixed: BT hangs waiting for browser proxy process to terminate. Thanks to @cjs1976 and @fixator10 in #51.

## 3.6.1

🐞Fixed: when external monitor is set as primary, picker is shown on a wrong monitor. Thanks to @ianzhuo in #48.

## 3.6.0

### New

- You can create rules based on [application title and process name](https://aloneguid.github.io/bt/rules.html#matching-locations).
- URL picker displays extra metadata if known: [process name and window title](https://aloneguid.github.io/bt/picker.html).
- Added support for [Thorium Browser](https://thorium.rocks/) (#43 for @paz)

### Improvements

- Picker is shown on the monitor 💻 of the source application instead of always defaulting to main monitor, thanks to @mahoromax in #42.
- Picker allows selecting first 10 browsers with a 🔢 number key, thanks to @coromd for the idea in #38!
- Theming improvements:
  - Better dark theme.
  - Theme switching was leaving some colours from the old theme.
  - Button colours on lite theme do not look ugly anymore.
- Update checks removed to avoid bugging users. You can still check from them from "Help" menu.


### Bugs Fixed

- do not show hidden browser's profiles in picker (#41 for @mahoromax).
- crash on update check when no internet available (#44 for @neoOpus).

## 3.5.2

Resolves an issue when Chromium browsers are not brought to foreground. Thanks to @tallen1983 in #39.

> Not using `ShellExecute` and waiting for successful process handle seems to be making Chromium happy.

## 3.5.1

**New feature!** [Office 365 link unwrapping](https://aloneguid.github.io/bt/url-processing.html#office-365-link-unwrapping) implemented, thanks to @maluht in #36.

## 3.5.0

### New

- Added support for [URL un-shortening](https://aloneguid.github.io/bt/url-processing.html#un-shortening).
- Tray menu has "URL Tester" shortcut.
- "buy me a coffee" link removed from the tray icon and status bar, because no one is donating. Everything irrelevant removed too, trying to be more ergonomic.

### Improvements

- Slightly redesigned "URL Tester" window for better user workflow.

### Bugs Fixed

- Clicking on update notification was opening home page, but it should just go to [GitHub Releases page](https://github.com/aloneguid/bt/releases).

## 3.4.2

Bug fixed - picker was always defaulting to "chrome-less window". Thanks to @danarnold in #20.

## 3.4.1

Bug fixed - when editing rule, you could not jump to next edit box with `tab` or previous with `shift`+`tab`.

## 3.4.0

### New

- When editing browser profiles, you can optionally add your own custom arguments to pass to the browser (see `extra args`). Thanks to @AJolly for the idea.
- URL Picker has an option to turn off recording of "popularity".
- [Documentation website](https://aloneguid.github.io/bt/home.html) is now live. It's small and simple, but allows to add more structured content, unlike blog pages.
- Ability to hide browsers from configuration window was added. This affects both configuration and picker, so you won't see browsers you are not interested in.
- Rules can be reordered by simply pressing ⬆️ or ⬇️ keys on the keyboard. Thanks to @maluht for the idea in #13.
- When editing rule, you can jump to next edit box with `tab` or previous with `shift`+`tab`.
- Status bar shows number of browsers, profiles, and configured rules.
- GitHub releases publishes SHA-256 checksums.

### Improvements

- Updating custom browser name will also update it in browser list as you type.
- Other cosmetic pedantic improvements.

### Bugs Fixed

- Picker would not work if you enable "frameless window" but pick a browser that does not support this feature. Browser list now also indicates if browser supports frameless feature. Thanks to @D3XX3R in #20.
- Tray icon tooltip was not displaying. It should have shown application name and version number.

## 3.3.1

- Rule hits can can be optionally recorded to a CSV file (off by default). To enable, check `Settings`->`Log Rule Hits to File`. This creates `hit_log.csv` which you can open from the `File` menu. Feature is useful for recording your activity for further analysis to exporting to some analytical engine.

### Bugs Fixed

- Config window was not deleted from memory after closing it.
- Update check was happening on every start instead of every 24 hours.

## 3.3.0

### New

- Match rules support regular expressions syntax.
- Support for Firefox Containers added! Firefox containers link redirection works with [Open external links in a container by Denys H](https://addons.mozilla.org/en-GB/firefox/addon/open-url-in-container/?utm_source=addons.mozilla.org&utm_medium=referral&utm_content=search) extension or my own [Browser Tamer extension](https://addons.mozilla.org/en-GB/firefox/addon/browser-tamer/).
- Added [Firefox integration extension](https://addons.mozilla.org/en-GB/firefox/addon/browser-tamer/) (link available from `Help` menu) and published source code for Chromium and Firefox extensions.
- Added native support for [Waterfox](https://www.waterfox.net/). This means BT will treat Waterfox just like Firefox, including support for profiles, private mode, containers and the rest.

### Improvements

- `config.ini` is slightly simpler - removed `open_cmd` and `vdf` keys as they are not required anymore.
- Browser profiles are sorted by the order they appear in the browser itself instead of alphabetically.
- Chromium browsers have a convenience toolbar button that links to browser store to download optional integration extension.
- Visual: "incognito" modes are marked with a visual icon.
- Firefox: profile discovery skips `default` profile which is intended for some kind of alpha testing and has no meaning to a normal user. `default-release` profiles is now displayed as `Primary` as this is exactly what it is.
- Infrastructure: release notes are removed from application code and replaced by a [link to GitHub](https://github.com/aloneguid/bt/releases) to avoid duplication and gradually increasing application size as they grow.

### Bugs Fixed

- health check: if BT is installed as browser but to a different directory than you are launching from, validation was still succeeding.
- health check: status icon was not updating after health is fixed from dashboard.


## 3.2.0

### New Features

- Rules can trigger opening browser in 'app mode' - borderless, frameless window. Only available for Chromium browsers.


### Improvements

- Visual improvement of the rule editor, making it look less busy.
- URL Tester visual improvement
- URL Picker visual improvement - buttons moved to the top, and added option to open in frameless mode
- Added shortcut button to open URL tester next to rule definitions


### Bugs Fixed

- URL matching by domain only would sometimes detect wrong protocol.
- Adding rule from URL picker was ignored.


## 3.1.7

### New Features

- You can run in truly portable mode (where config.ini is stored alongside bt.exe) by creating an empty '.portable' file in the same directory. Thanks to @maluht in #6.


### Improvements

- Various code stability improvements
- Added 'Add' button to the main UI, to emplasise you can add your custom browser. Previously it was available only from the File menu.


### Bugs Fixed

- BT would crash on start of configuration window for new installations due to expecting at least one browser to be registered.


## 3.1.6

### New Features

- Rules can have priorities in case multiple browsers match with similar rules.


### Improvements

- Added automation test suite based on Google Test. For you it means the app gets more rock-solid.


### Bugs Fixed

- Invalid behavior when matching empty URL or empty rule.


## 3.1.5

### Improvements

- BT does not check if browser process is running anymore, to avoid listing system processes which AVs don't like.
- Some UI icons updated


## 3.1.4

### Bugs Fixed

- Configuration editor did not properly read scope of matching.


## 3.1.3

### New Features

- Added ability to restrict rule matching to everything (default), domain, or path.


## 3.1.2

### New Features

- "Autoshutdown" feature removed as not an important function of this app


## 3.1.1

### New Features

- This is the first fully open-source version, where all the code and releases are published in the open on GitHub.


### Improvements

- To avoid distractions, CPU/Memory/Framerate debug info moved to about window.


### Bugs Fixed

- update check will only occur every 24 hours.


## 3.1.0

### New Features

- Custom browser's profiles are now just custom browsers, making custom-registered browser experience more ergonomic. They can be added from File menu.
- Metrics window is completely removed. I've decided this app should do one thing but do it well and metrics is just outside of the scope of Browser Tamer core functionality. There are plenty of tools to investigate performance, including ones built into Windows itself.
- File menu has new option - 'copy path to config.ini' which copies it to clipboard.


### Improvements

- Browser list in config window is sorted - system browsers alphabetically, then custom browsers alphabetically. Profiles are sorted alphabetically to, incognito being at the end. All of the sorting is case-insensitive.
- Freed up configuration real estate by moving 'test' button to the rest of browser control buttons.
- Window scaling errors fixed for certain monitors.
- Config window status bar text doesn't jump around when width change.
- All of the browsers have a running icon indicator, including user-defined.


### Bugs Fixed

- If custom defined browser didn't specify url argument nothing was passed to the target browser


## 3.0.2

### New Features

- License terms updated so that you don't have to buy me a cake (including vanilla) or assign your first-born child. You are obliged to buy me a coffee though, which is cheaper than cake.


### Bugs Fixed

- CPU usage metric is unreliable and is temporarily removed.


## 3.0.1

### New Features

- Status bar displays current CPU load of your system.
- Metrics window is available from browser right panel, showing realtime graphs of CPU, Memory and number of processes of the browser in general.


## 3.0.0

### New Features

- Browser discovery is only triggered by user request, so we don't re-discover on every launch. It makes antivirus software more happy
- All the browser metadata is saved inside the .ini file
- Configuration file location is now at LocalAppData\bt\config.ini
- Installers are now .msi based
- Show notification when link is intercepted (off by default)


### Improvements

- URL Tester shows additional column with browser and profile.


## 2.9.14

### New Features

- Added a button to kill all browser processes


### Improvements

- update to latest release of ImGui and MSVC++


## 2.9.13

### Improvements

- New version check on start


### Bugs Fixed

- Fixed configuration window shake when dragging.


## 2.9.12

### New Features

- In addition to appearing on hot key press, Picker supports three modes. Silent - never pop up. Conflict - pop up if there are multiple matches (current and default mode). Always - always pop up regardless of any conditions.


### Improvements

- Autoshutdown is now off by default by popular demands.


### Bugs Fixed

- Corporate Grey theme was always turning on Cherry.


## 2.9.11

### Improvements

- RAM usage and process count now also display microplots.


### Bugs Fixed

- Only allow single instance of the configuration dialog.


## 2.9.10

### Bugs Fixed

- Autoshutdown was invoked in 2 minutes instead of 30.
- I didn't have coffee like everyone else did.


## 2.9.9

### Bugs Fixed

- There were a lot of visually identical notification icons in Windows tasbar history due to re-registering a new one on startup.


## 2.9.8

### New Features

- BT will automatically shut-down if not used for 30 minutes to consume even less resources. You can disable that in settings, but I'm not sure why you would do that'.


### Improvements

- Browser uptime is displayed as a part of browser stats
- Browser profile overlays in the picker are rounded similar to how Chromium paints them.
- Multi-window interface


### Bugs Fixed

- Change Log was trimming older entries so you couldn't see them at all.
- Picker Enabled setting was always ignored and picker was shown regardless
- Uninstaller completely removes all traces of BT from the system


## 2.9.7

### Improvements

- PDF association has a different icon


## 2.9.6

### New Features

- Brave Browser profiles are fully supported
- Brave Tor Mode is supported. Unfortunately due to the bug in Brave itself it opens duplicate instances for Tor URL. This shoudl be fixed in Brave soon.
- BT registers itself as a handler for popular file extensions browsers can handle (pdf, html, html, svg etc.). If you already have BT installed, go to Tools->Troubleshooting->Re-register as Browser to enable this.


### Improvements

- More modern looking installer (if you are using it)
- Menu look more aligned to icons


## 2.9.5

### Improvements

- windows and controls have rounded corners
- selection rectangles change backgrounds
- picker has proper spacing
- picker window has title and close button


## 2.9.4

### Improvements

- URL tester window is not modal and looks slightly better


### Bugs Fixed

- change log window could not be closed by X button
- URLs containing exclamation mark were trimmed


## 2.9.3

### New Features

- completely redesigned URL picker - more modern, scrollable and adjustable
- URL picker now allows to copy URL to clipboard and cancel


## 2.9.2

### Bugs Fixed

- picker was always triggered


## 2.9.1

### Bugs Fixed

- picker was not triggered at all


## 2.9.0

### New Features

- more modern user interface layout
- custom browser location is now selectable with standard Windows file dialog instead of manual input
- tools->rediscover browsers menu item rescans your OS for browser changes instantly
- number of processes and amount of RAM used is displayed next to browser item
- file menu has a new option to open bt.ini configuration file
- almost zero tracking: I only record the fact the app is started, and the app version, nothing else.


### Improvements

- Chromium profile discovery is now much faster and much more secure
- browser rules visual editor replaces multiline textbox - this is to get ready for more complicated rules
- small pedantic UI improvements


## 2.8.7

### New Features

- BT follows OS application mode (Windows 10 & Windows 11) for theming. Light and Dark mode are supported. You can additionally pin theme in settings, as well as choose a theme other than Light and Dark.


## 2.8.6

### Improvements

- Firefox profile discovery has changed - instead of trying to enumerate profile folders in AppData/Local BT now reads profiles.ini which is way more reliable.
- New button in BT UI allows to open Firefox profile manager in a system window or inside the Firefox itself.


## 2.8.5

### Improvements

- UI shows which hot keys are pressed down at the moment
- change log can be viewer from the website as well as the app itself


### Bugs Fixed

- UI is more responsive (was trying to be as effective as possible too much).
- hot key detection fixed, hopefully forever!


## 2.8.3

### Improvements

- Chromium profile discovery is now much faster and more secure
- change log is now embedded into BT program itself. The website won't show it.
- very minor UI polishes.


### Bugs Fixed

- shutdown was opening a new instance instead of actually shutting down


## 2.8.2

### Improvements

- notification icon displays tooltip with version number
- BT is now rendering UI in high resolution, the difference is immense!
- picker hot keys are customizable.


## 2.8.1

### Bugs Fixed

- detection for x-bt protocol was sometimes showing BT is not registered as a handler.
- picker hot key detection was often wrongly thiking picker should be called.


## 2.8.0

### New Features

- Fully support Firefox Profiles.


## 2.7.9

### New Features

- Vivaldi brower profiles are supported.


## 2.7.7

### Improvements

- bt.exe now runs in "efficiency mode" which makes it absolutely non existant in terms of system resource usage.


### Bugs Fixed

- occasional crash on urls with "%".


## 2.7.6

### Improvements

- window header displays version.
- by popular requests more items added to context menu.


### Bugs Fixed

- pressing "exit" in config window did nothing.


## 2.7.5

### Improvements

- Installer does not require admin privileges to install anymore.


## 2.7.4

### Bugs Fixed

- search queries issued by "PowerToys Run" (starting with ?) were launching new instance of BT rather than launching search.


## 2.7.3

### Bugs Fixed

- installer is now putting files into user profile instead of installing machine-wide. You might need to uninstall 2.7.2 manually.


## 2.7.2

### Improvements

- added proper Windows application metadata to the output .exe
- configuration is now stored alongside .exe in bt.ini file and can be moved around. Windows Registry is not used at all.


## 2.7.1

### Improvements

- general stability


## 2.7.0

### New Features

- added system notification icon - click to open configuration, right-click for more options. Bug fixed: Sometimes URL picker will show only when “ctrl” is pressed. System improvements: using windows messaging instead of named pipes which would trigger antivirus false positive sometimes. Also executable size is now even smaller.


## 2.6.2

### New Features

- Added ability to delete custom profiles. Installer now shuts down bt.exe before upgrading to avoid conflicts.


## 2.6.1

### New Features

- redesigned user interface in the configuration editor
- browser picker displays icons graphically and sorts them based on frequency of use
- if there is more than one rule match in different browsers, UI will ask which one you want instead of using the first one


### Improvements

- even thought there are more features in this release, it’s considerably smaller in size and faster.


## 2.5.6

### Improvements

- Performance and security


## 2.5.5

### Improvements

- Performance and security


## 2.5.4

### Improvements

- Performance and security


## 2.5.3

### Improvements

- added auto update check and link to Chrome extension


## 2.5.2

### Bugs Fixed

- URL picker would sometimes hang in the background.


## 2.5.1

### Bugs Fixed

- URL picker was opening in the background so you couldn’t see it sometimes.


## 2.5

### New Features

- add your own browser
- support for Firefox private mode
- support for x-bt protocol


### Improvements

- picker launcher keys can be configured!
- user interface is slightly more polished.


## 2.4

### New Features

- When holding ctrl and pressing a link, you can pick which browser to open.


## 2.3

### Improvements

- Performance and security


## 2.2

### Improvements

- Performance and security


## 2.1

### Improvements

- Still called Oh Dear! due to the fact I love ImGui so much.


## 2.0

### New Features

- Complete UI rewrite and new features. Due to the fact Microsoft cannot decide on what is the UI framework for programming on Windows, I’ve decided to use a completely independent third-party framework ImGui which is usually used in game development but also good for small utilities like this. It’s super small and super fast.


## 1.0

### New Features

- Sort of a first release.