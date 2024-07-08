# Command line

%product% can be controlled from the command line. This is useful for scripting, automation, or for advanced users.

## Launch configuration editor

Invoking %product% with no arguments will launch the configuration editor user interface.

```shell
bt.exe
```

## Open URL

The most basic case is to just invoke %product% with a URL as an argument. This will behave exactly as you would click a link in an external application and follow the usual process of handling rule, invoking picker and whatever else you have configured.

```shell
bt.exe https://www.google.com
```

## Open picker for any URL

If you want to open the picker for any URL, you can use the `pick` command.

```shell
bt.exe pick https://www.google.com
```
