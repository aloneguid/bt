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

## Terminal commands

You can also use terminal commands to control %product%, this does not pop up any UI.

### List browsers and profiles

```shell
bt browser list
```

Returns a long list of all currently configured browsers and profiles.

```
browsers: 3

msedge
  name:     Microsoft Edge
  cmd:      C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe
  system:   yes
  hidden:   no
  features: system chromium
  profiles: 3
    > id:        Default
      name:      Personal
      args:      "url" "--profile-directory=Default" --no-default-browser-check
      hidden:    no
      icon:      C:\Users\alone\AppData\Local\Microsoft\Edge\User Data\Default\Edge Profile Picture.png
      incognito: no
    > id:        Profile 1
      name:      Work
      args:      "url" "--profile-directory=Profile 1" --no-default-browser-check
      hidden:    no
      icon:      C:\Users\alone\AppData\Local\Microsoft\Edge\User Data\Profile 1\Edge Profile Picture.png
      incognito: no
    > id:        InPrivate
      name:      InPrivate
      args:      "url" --inprivate
      hidden:    no
      incognito: yes

firefox
  name:     Mozilla Firefox
  cmd:      C:\Program Files\Mozilla Firefox\firefox.exe
  system:   yes
  hidden:   no
  features: system firefox
  profiles: 4
    > id:        Profile0
      name:      No container
      args:      "url" -P "default-release"
      hidden:    no
      incognito: no
    > id:        Profile0+c_2
      name:      Isoline
      args:      "ext+bt:container=Isoline&url=url" -P "default-release"
      hidden:    no
      incognito: no
    > id:        private
      name:      Private
      args:      -private-window "url"
      hidden:    no
      icon:      C:\Program Files\Mozilla Firefox\firefox.exe
      incognito: yes

aadb3a96-6f1b-444b-955d-ac5f7d451f7f
  name:     Tor Browser
  cmd:      C:\software\tor\Browser\firefox.exe
  system:   no
  hidden:   no
  profiles: 1
    > id:        default
      name:      Tor Browser
      hidden:    no
      incognito: no
```
{collapsible="true" collapsed-title="Example output"}

### Get default browser

```shell
bt browser get default
```

Will print the default browser in format `<browser_id>.<profile_id>`. You can view these ids using `bt browser list` commands.

### Set default browser

```shell
bt browser set default <browser_id>.<profile_id>
```

Will set the default browser to the specified browser and profile. You need to specify correct browser and profile ids, which again can be retreived from `bt browser list` command.