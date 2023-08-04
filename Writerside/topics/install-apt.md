# Package Managers

Installing with package managers is unfortunately counterproductive. %product% used to be in Scoop and WinGet, but I have requested to remove it in both [WinGet](https://github.com/microsoft/winget-pkgs/pull/115266) and [Scoop](https://github.com/ScoopInstaller/Extras/pull/11695).

## Reasons

1. %product% is a free and open-source system utility made by one person in spare time. This means I don't have time to properly support external package managers.
2. Community efforts so far have left package managers with ancient, outdated versions.
3. %product% very often triggers antivirus false-positives, because:
   - it scans your system to autodetect the browsers installed, and this is apparently considered dangerous by robots or mindless individuals.
   - %product% does not have a code signing certificate, because I can't afford one.
4. Package managers don't like system utilities and block submissions if any antivirus triggers a warning. I don't have time to convince them it's not. Again it's a personal hobby project, not a business.
