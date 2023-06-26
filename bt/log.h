#pragma once
#include <string>
#include <vector>

struct change_version {
    std::string number;
    std::string date;
    std::vector<std::string> news;
    std::vector<std::string> improvements;
    std::vector<std::string> bugs;
};

const std::vector<change_version> ChangeLog {
    change_version {"3.1.0", "2023-06-08",
        {
            "Custom browser's profiles are now just custom browsers, making custom-registered browser experience more ergonomic. They can be added from File menu.",
            "Metrics window is completely removed. I've decided this app should do one thing but do it well and metrics is just outside of the scope of Browser Tamer core functionality. There are plenty of tools to investigate performance, including ones built into Windows itself.",
            "File menu has new option - 'copy path to config.ini' which copies it to clipboard.",
        },
        {
            "Browser list in config window is sorted - system browsers alphabetically, then custom browsers alphabetically. Profiles are sorted alphabetically to, incognito being at the end. All of the sorting is case-insensitive.",
            "Freed up configuration real estate by moving 'test' button to the rest of browser control buttons.",
            "Window scaling errors fixed for certain monitors.",
            "Config window status bar text doesn't jump around when width change.",
            "All of the browsers have a running icon indicator, including user-defined.",
        },
        {
            "If custom defined browser didn't specify url argument nothing was passed to the target browser",
        }},
    change_version {"3.0.2", "2023-05-26",
        {
            "License terms updated so that you don't have to buy me a cake (including vanilla) or assign your first-born child. You are obliged to buy me a coffee though, which is cheaper than cake.",
        },
        {},
        {
            "CPU usage metric is unreliable and is temporarily removed.",
        }},
    change_version {"3.0.1", "2023-05-23",
        {
            "Status bar displays current CPU load of your system.",
            "Metrics window is available from browser right panel, showing realtime graphs of CPU, Memory and number of processes of the browser in general.",
        },
        {},
        {}},
    change_version {"3.0.0", "2023-05-09",
        {
            "Browser discovery is only triggered by user request, so we don't re-discover on every launch. It makes antivirus software more happy",
            "All the browser metadata is saved inside the .ini file",
            "Configuration file location is now at LocalAppData/bt/config.ini",
            "Installers are now .msi based",
            "Show notification when link is intercepted (off by default)",
        },
        {
            "URL Tester shows additional column with browser and profile.",
        },
        {}},
    change_version {"2.9.14", "2023-05-02",
        {
            "Added a button to kill all browser processes",
        },
        {
            "update to latest release of ImGui and MSVC++",
        },
        {}},
    change_version {"2.9.13", "2023-03-27",
        {},
        {
            "New version check on start",
        },
        {
            "Fixed configuration window shake when dragging.",
        }},
    change_version {"2.9.12", "2023-03-13",
        {
            "In addition to appearing on hot key press, Picker supports three modes. Silent - never pop up. Conflict - pop up if there are multiple matches (current and default mode). Always - always pop up regardless of any conditions.",
        },
        {
            "Autoshutdown is now off by default by popular demands.",
        },
        {
            "Corporate Grey theme was always turning on Cherry.",
        }},
    change_version {"2.9.11", "2023-02-24",
        {},
        {
            "RAM usage and process count now also display microplots.",
        },
        {
            "Only allow single instance of the configuration dialog.",
        }},
    change_version {"2.9.10", "2023-02-23",
        {},
        {},
        {
            "Autoshutdown was invoked in 2 minutes instead of 30.",
            "I didn't have coffee like everyone else did.",
        }},
    change_version {"2.9.9", "2023-02-23",
        {},
        {},
        {
            "There were a lot of visually identical notification icons in Windows tasbar history due to re-registering a new one on startup.",
        }},
    change_version {"2.9.8", "2023-02-14",
        {
            "BT will automatically shut-down if not used for 30 minutes to consume even less resources. You can disable that in settings, but I'm not sure why you would do that'.",
        },
        {
            "Browser uptime is displayed as a part of browser stats",
            "Browser profile overlays in the picker are rounded similar to how Chromium paints them.",
            "Multi-window interface",
        },
        {
            "Change Log was trimming older entries so you couldn't see them at all.",
            "Picker Enabled setting was always ignored and picker was shown regardless",
            "Uninstaller completely removes all traces of BT from the system",
        }},
    change_version {"2.9.7", "2022-12-14",
        {},
        {
            "PDF association has a different icon",
        },
        {}},
    change_version {"2.9.6", "2022-12-14",
        {
            "Brave Browser profiles are fully supported",
            "Brave Tor Mode is supported. Unfortunately due to the bug in Brave itself it opens duplicate instances for Tor URL. This shoudl be fixed in Brave soon.",
            "BT registers itself as a handler for popular file extensions browsers can handle (pdf, html, html, svg etc.). If you already have BT installed, go to Tools->Troubleshooting->Re-register as Browser to enable this.",
        },
        {
            "More modern looking installer (if you are using it)",
            "Menu look more aligned to icons",
        },
        {}},
    change_version {"2.9.5", "2022-12-13",
        {},
        {
            "windows and controls have rounded corners",
            "selection rectangles change backgrounds",
            "picker has proper spacing",
            "picker window has title and close button",
        },
        {}},
    change_version {"2.9.4", "2022-12-12",
        {},
        {
            "URL tester window is not modal and looks slightly better",
        },
        {
            "change log window could not be closed by X button",
            "URLs containing exclamation mark were trimmed",
        }},
    change_version {"2.9.3", "2022-12-08",
        {
            "completely redesigned URL picker - more modern, scrollable and adjustable",
            "URL picker now allows to copy URL to clipboard and cancel",
        },
        {},
        {}},
    change_version {"2.9.2", "2022-12-08",
        {},
        {},
        {
            "picker was always triggered",
        }},
    change_version {"2.9.1", "2022-12-08",
        {},
        {},
        {
            "picker was not triggered at all",
        }},
    change_version {"2.9.0", "2022-12-07",
        {
            "more modern user interface layout",
            "custom browser location is now selectable with standard Windows file dialog instead of manual input",
            "tools->rediscover browsers menu item rescans your OS for browser changes instantly",
            "number of processes and amount of RAM used is displayed next to browser item",
            "file menu has a new option to open bt.ini configuration file",
            "almost zero tracking: I only record the fact the app is started, and the app version, nothing else.",
        },
        {
            "Chromium profile discovery is now much faster and much more secure",
            "browser rules visual editor replaces multiline textbox - this is to get ready for more complicated rules",
            "small pedantic UI improvements",
        },
        {}},
    change_version {"2.8.7", "2022-11-24",
        {
            "BT follows OS application mode (Windows 10 & Windows 11) for theming. Light and Dark mode are supported. You can additionally pin theme in settings, as well as choose a theme other than Light and Dark.",
        },
        {},
        {}},
    change_version {"2.8.6", "2022-11-21",
        {},
        {
            "Firefox profile discovery has changed - instead of trying to enumerate profile folders in AppData/Local BT now reads profiles.ini which is way more reliable.",
            "New button in BT UI allows to open Firefox profile manager in a system window or inside the Firefox itself.",
        },
        {}},
    change_version {"2.8.5", "2022-11-21",
        {},
        {
            "UI shows which hot keys are pressed down at the moment",
            "change log can be viewer from the website as well as the app itself",
        },
        {
            "UI is more responsive (was trying to be as effective as possible too much).",
            "hot key detection fixed, hopefully forever!",
        }},
    change_version {"2.8.3", "2022-11-18",
        {},
        {
            "Chromium profile discovery is now much faster and more secure",
            "change log is now embedded into BT program itself. The website won't show it.",
            "very minor UI polishes.",
        },
        {
            "shutdown was opening a new instance instead of actually shutting down",
        }},
    change_version {"2.8.2", "2022-11-17",
        {},
        {
            "notification icon displays tooltip with version number",
            "BT is now rendering UI in high resolution, the difference is immense!",
            "picker hot keys are customizable.",
        },
        {}},
    change_version {"2.8.1", "2022-11-16",
        {},
        {},
        {
            "detection for x-bt protocol was sometimes showing BT is not registered as a handler.",
            "picker hot key detection was often wrongly thiking picker should be called.",
        }},
    change_version {"2.8.0", "2022-11-10",
        {
            "Fully support Firefox Profiles.",
        },
        {},
        {}},
    change_version {"2.7.9", "2022-11-02",
        {
            "Vivaldi brower profiles are supported.",
        },
        {},
        {}},
    change_version {"2.7.7", "2022-10-05",
        {},
        {
            "bt.exe now runs in \"efficiency mode\" which makes it absolutely non existant in terms of system resource usage.",
        },
        {
            "occasional crash on urls with \"%\".",
        }},
    change_version {"2.7.6", "2022-09-26",
        {},
        {
            "window header displays version.",
            "by popular requests more items added to context menu.",
        },
        {
            "pressing \"exit\" in config window did nothing.",
        }},
    change_version {"2.7.5", "2022-09-21",
        {},
        {
            "Installer does not require admin privileges to install anymore.",
        },
        {}},
    change_version {"2.7.4", "2022-09-20",
        {},
        {},
        {
            "search queries issued by \"PowerToys Run\" (starting with ?) were launching new instance of BT rather than launching search.",
        }},
    change_version {"2.7.3", "2022-09-20",
        {},
        {},
        {
            "installer is now putting files into user profile instead of installing machine-wide. You might need to uninstall 2.7.2 manually.",
        }},
    change_version {"2.7.2", "2022-09-19",
        {},
        {
            "added proper Windows application metadata to the output .exe",
            "configuration is now stored alongside .exe in bt.ini file and can be moved around. Windows Registry is not used at all.",
        },
        {}},
    change_version {"2.7.1", "2022-09-14",
        {},
        {
            "general stability",
        },
        {}},
    change_version {"2.7.0", "2022-09-13",
        {
            "added system notification icon - click to open configuration, right-click for more options. Bug fixed: Sometimes URL picker will show only when “ctrl” is pressed. System improvements: using windows messaging instead of named pipes which would trigger antivirus false positive sometimes. Also executable size is now even smaller.",
        },
        {},
        {}},
    change_version {"2.6.2", "2022-07-06",
        {
            "Added ability to delete custom profiles. Installer now shuts down bt.exe before upgrading to avoid conflicts.",
        },
        {},
        {}},
    change_version {"2.6.1", "2022-07-04",
        {
            "redesigned user interface in the configuration editor",
            "browser picker displays icons graphically and sorts them based on frequency of use",
            "if there is more than one rule match in different browsers, UI will ask which one you want instead of using the first one",
        },
        {
            "even thought there are more features in this release, it’s considerably smaller in size and faster.",
        },
        {}},
    change_version {"2.5.6", "2022-06-13",
        {},
        {
            "Performance and security",
        },
        {}},
    change_version {"2.5.5", "2022-05-20",
        {},
        {
            "Performance and security",
        },
        {}},
    change_version {"2.5.4", "2022-05-12",
        {},
        {
            "Performance and security",
        },
        {}},
    change_version {"2.5.3", "2022-03-21",
        {},
        {
            "added auto update check and link to Chrome extension",
        },
        {}},
    change_version {"2.5.2", "2022-03-18",
        {},
        {},
        {
            "URL picker would sometimes hang in the background.",
        }},
    change_version {"2.5.1", "2022-03-16",
        {},
        {},
        {
            "URL picker was opening in the background so you couldn’t see it sometimes.",
        }},
    change_version {"2.5", "2022-03-15",
        {
            "add your own browser",
            "support for Firefox private mode",
            "support for x-bt protocol",
        },
        {
            "picker launcher keys can be configured!",
            "user interface is slightly more polished.",
        },
        {}},
    change_version {"2.4", "2022-02-11",
        {
            "When holding ctrl and pressing a link, you can pick which browser to open.",
        },
        {},
        {}},
    change_version {"2.3", "2022-01-20",
        {},
        {
            "Performance and security",
        },
        {}},
    change_version {"2.2", "2022-01-16",
        {},
        {
            "Performance and security",
        },
        {}},
    change_version {"2.1", "2021-11-16",
        {},
        {
            "Still called Oh Dear! due to the fact I love ImGui so much.",
        },
        {}},
    change_version {"2.0", "2021-11-13",
        {
            "Complete UI rewrite and new features. Due to the fact Microsoft cannot decide on what is the UI framework for programming on Windows, I’ve decided to use a completely independent third-party framework ImGui which is usually used in game development but also good for small utilities like this. It’s super small and super fast.",
        },
        {},
        {}},
    change_version {"1.0", "2021-09-06",
        {
            "Sort of a first release.",
        },
        {},
        {}},

};

