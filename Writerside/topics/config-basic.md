# Configuration

## Default browser

By default, BT takes the first browser it finds and opens all the links in it.

You can change this from `Settings->Default Browser` menu which lists all the detected browsers and profiles hierarchically.

![](default.png)

## Auditing

Browser Tamer can now optionally log rule hits to a `.csv` file. It can be enabled in **Settings**, and the file path can be found in `File` menu:

![](audit-menus.png)

The file itself has all the information about the rule hit event:

- timestamp
- browser id
- browser name
- profile name
- URL
- rule description
- calling process name (if available)
- calling process window title (if available)

![](audit-csv.png)