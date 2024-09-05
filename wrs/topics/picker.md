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

## Keyboard navigation

You can dismiss the picker dialog after it has opened by pressing <shortcut>Esc</shortcut>. This will only work when you haven't selected a browser or profile yet.

In addition to that, you can select the first `9` browsers and `9` profiles using the keyboard. For instance, pressing <shortcut>1</shortcut> will select the first browser, pressing <shortcut>2</shortcut> will select the second browser, and so on. The same applies to profiles. For instance, to select the second profile or the third browser, presss <shortcut>3 2</shortcut>.

If you have selected a browser with multiple profiles, but then changed your mind, you can press <shortcut>Esc</shortcut> to go back to the browser selection and choose another browser. Pressing <shortcut>Esc Esc</shortcut> will close the picker dialog.




