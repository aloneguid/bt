# Configuration

## Default browser

By default, BT takes the first browser it finds and opens all the links in it.

You can change this from `Settings->Default Browser` menu which lists all the detected browsers and profiles hierarchically.

![](default.png)

## Custom arguments

Both user-defined and system (autodetected) browsers have an additional argument added. For system browsers, expand "Parameters" to add this "extra arg". %product% does **not** allow modifying `arg` to avoid breaking basic functionality.

<warning>
Most browsers will not apply settings like <a href="https://peter.sh/experiments/chromium-command-line-switches/">chromium flags</a> until you restart the browser completely.
</warning>

![](extra-params.png)

## Auditing

%product% can now optionally log rule hits to a `.csv` file. It can be enabled in **Settings**:

<img height="150" src="audit00.png"/>

and the file path can be found in `File` menu:

<img height="120" src="audit01.png"/>

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