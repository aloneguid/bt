# Picker

Picker is a visual prompt that asks which browser you want to open a link in specifically, instead of matching a rule. It's invoked in the following situations.

It can be invoked either manually or automatically, or both. To set this behavior, invoke the "Picker" menu from the main window:

<img height="150" src="picker-on.png"/>

For manual invocation, one of the following key combinations can be used, if configured.
- `Ctrl` + `Shift` + left mouse click.
- `Ctrl` + `Alt` + left mouse click.
- `Alt` + `Shift` + left mouse click.

You can also invoke picker from the [command line](commandline.md).

For automatic invocation, picker will be displayed in the following situations:
- **Always** - picker will always be displayed, regardless of any rules configured.
- **On conflict** - picker will be displayed if there are multiple rules that match the URL.
- **On no rule** - picker will be displayed if there are no rules that match the URL.

## Making a choice

The picker interface is dead simple - it display address bar and the list of browsers you can choose from:

<img height="150" src="picker.png"/>

If a browser has multiple profiles, the picker window will display a list of profiles as well:

<img height="200" src="picker2.png"/>




