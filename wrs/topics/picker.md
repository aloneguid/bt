# Picker

Picker is a visual prompt that asks which browser you want to open a link in specifically, instead of matching a rule.

It can be invoked either manually or automatically, or both. To set this behavior, invoke the "Picker" menu from the main window:

<img height="150" src="picker-on.png"/>

## Manual invocation

For manual invocation, one of the following key combinations can be used, if configured.
- `Ctrl` + `Shift` + left mouse click.
- `Ctrl` + `Alt` + left mouse click.
- `Alt` + `Shift` + left mouse click.
- `CAPS LOCKS` is on when clicking.

## Automatic invocation

For automatic invocation, picker will be displayed in the following situations:
- **Always** - picker will always be displayed, regardless of any rules configured.
- **On conflict** - picker will be displayed if there are multiple rules that match the URL.
- **On no rule** - picker will be displayed if there are no rules that match the URL.

<note>
You can also invoke picker from the <a href="commandline.md">command line</a>.
</note>

## Making a choice

The picker interface is dead simple - it's a radial menu that displays all the browsers and profiles you have configured in %product%, excluding the ones you have hidden explicitly.

You can make a selection by clicking on the profile you want the current link to be opened in. 

<img height="300" src="picker.png"/>

Before making the choice, you can also change the URL that will be opened by the browser. This is useful if you want to open a different URL than the one you clicked on, or if you want to modify the URL in some way. To change the URL, simply modify it in the input field above the radial menu.

There are also a couple of "micro apps" available when clicking on the "9 dots" icon to the right top of the menu:

<img height="200" src="picker2.png"/>

Currently the options are:
- **Back** - go back to the browser selection menu.
- **Copy** - copy the URL to the clipboard and close the picker dialog.
- **Email** - open the default email client with the URL in the body of the email.

## Keyboard navigation

Coming soon...

## Other customisations

The top of the Picker menu has more customisation options that are worth mentioning:

Close on focus loss
: If enabled, the picker dialog will close when it loses focus. This is useful if you want to quickly switch to another window without having to close the picker dialog manually. This essentially cancels picker dialog if you switch to another window.

Always on top
: If enabled, the picker dialog will always be on top of other windows, regardless which application is active. This is useful if some of your applications prevent Picker from being on top.






