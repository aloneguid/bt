# Configuration

## Default browser

By default, %product% takes the first browser it finds and opens all the links in it.

You can change the default browser by clicking the "heart" button on the tab of the profile you want to set as default. This will also be reflected in the browser list:

<img height="200" src="default.png"/>

Additionally, status bar will show the default browser:

![](default-sb.png)

## Custom arguments

Both user-defined and system (auto-detected) browsers have an additional argument added. This is "extra arg". %product% does **not** allow modifying `arg` to avoid breaking basic functionality.

<warning>
Most browsers will not apply settings like <a href="https://peter.sh/experiments/chromium-command-line-switches/">chromium flags</a> until you restart the browser completely.
</warning>

![](extra-params.png)

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