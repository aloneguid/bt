# Configuration

## Default browser

By default, %product% takes the first browser it finds and opens all the links in it.

You can change the default browser by clicking the "heart" button on the tab of the profile you want to set as default. This will also be reflected in the browser list:

<img height="150" src="default.png"/>

Additionally, status bar will show the default browser:

![](default-sb.png)

## Custom arguments

Both user-defined and system (auto-detected) browsers have an additional argument added. This is "extra arg". %product% does **not** allow modifying `arg` to avoid breaking basic functionality.

<warning>
Most browsers will not apply settings like <a href="https://peter.sh/experiments/chromium-command-line-switches/">chromium flags</a> until you restart the browser completely.
</warning>

![](extra-params.png)

## Bring your own browser

You can add your own browser definition by clicking the **Add** button above the browser list.

<img height="100" src="addcustom-button.png"/>

%product% will ask you for the path to the browser executable, and then go to the customization screen.

<img height="100" src="addcustom-params.png"/>

You can change display name, arguments, icon and whether to hide the UI when launching this "browser". The last option is useful if you want to add a custom script or application that does not have a UI, like in the example above, which simply launches powershell to execute a command to copy URL to clipboard. Without "hide user interface", a console window would be shown briefly.

## Auditing

%product% can now optionally log rule hits to a `.csv` file. It can be enabled by checking the **General -> Log clicks to hit_log.csv**.

And the file path can be found in "File" menu.

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