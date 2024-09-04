# Scripting

For total flexibility, %product% supports user scripting. Scripting allows you to customize two things:

1. **URL processing** - write your own custom scripts to process URLs before they are opened in the browser.
2. **Rule matching** - write your own custom rules to match URLs.

Scripts are written in [Lua](https://www.lua.org/), a lightweight, high-level, multi-paradigm programming language designed primarily for embedded use in applications. Lua is very fast, very low overhead language, and is very easy to learn. The sections below explain how to write scripts for %product%.

To write and test scripts, you should first open the Script editor from the **Tools** menu:

![](script-start.png)

## URL processing

Let's write a simple pipeline step that checks that if the URL starts with `https://meet.google.com/` it will append `authuser=1` to the query string. This is useful if you have multiple Google accounts and want to force a specific account to be used. Here is the Lua code for this functionality:

```
function ppl_gmeet()
    if string.find(p.url, "https://meet.google.com/") == 1 then
        return p.url .. "?authuser=1"
    end
    return p.url
end
```

The function `ppl_gmeet` is called by %product% for every URL that is about to be opened, after all the other built-in steps like unshortening, unwrapping, and substitutions.

This is how the function works:

1. It checks if the URL starts with `https://meet.google.com/` by using `string.find` function against `p.url` variable.
2. If it does, it appends `authuser=1` to the query string.
3. It returns the modified URL.
4. If the URL does not start with `https://meet.google.com/`, it returns the URL as is.

To use this script in %product%:

1. Paste the script into the "Lua script" editor.
2. Press "Save" to save the script.
3. Select the function name to run from the dropdown list. At this point you should have only one function, `ppl_gmeet`.
4. Type a URL to test, for instance `https://meet.google.com/mtg/`.
5. Press "Run".
6. The result will be displayed in the "Terminal" section.

![](script-fn1.png)

### The value of `p`

The `p` variable is always available from any function, and contains the following fields:

- `url` - the URL that is being processed.
- `wt` - the title of the window that is being processed.
- `pn` - the process name of the window that is being processed.

The variable `p` is read-only, and any modifications are ignored.

## Rule matching

Let's write a simple rule that matches any URL that starts with  `https://meet.google.com/` **and** is opened from [Slack](https://slack.com/) desktop application.

We already know how to check the first part, but for the second the solutions also simple - if the process name is `slack.exe`. Process name can be retreived from the `p.pn` variable. Here is the Lua code for this functionality:

```
function rule_gmeet_from_slack()
	if string.find(p.url, "https://meet.google.com/") == 1 and string.lower(p.pn) == "slack.exe" then
		return true
    end
	return false
end
```

This is how the function works:

1. It checks if the URL starts with `https://meet.google.com/` by using `string.find` function against `p.url` variable.
2. It checks if the process name is `slack.exe` by using `string.lower` function against `p.pn` variable.
3. If both conditions are met, it returns `true`.
4. If any of the conditions are not met, it returns `false`.

To use this script in %product%:

1. Paste the script into the "Lua script" editor.
2. Press "Save" to save the script.
3. Select the function name to run from the dropdown list. At this point you should have two functions, `ppl_gmeet` and `rule_gmeet_from_slack`.
4. Type a URL to test, for instance `https://meet.google.com/mtg/`.
5. Type process name to test, for instance `slack.exe`. You will notice that switching to a function prefixed with `rule_` will add extra fields for test input.
6. Press "Run".
7. The result will be displayed in the "Terminal" section.


![](script-fn2.png)

You might notice an interesting behavior in the terminal - message "pipeline changed URL to 'https://meet.google.com/?authuser=1'" - this is because the pipeline step `ppl_gmeet` was executed before the rule was checked. This is the expected behavior, as the pipeline steps are executed before the rule matching.

To use this rule in %product%, you need to add it to the rules list. This is done by pressing the "Add" button in the "Rules" section of an appropriate browser or prifile, selecting "Lua script" and then selecting function name in the dropdown list:

<img height="100" src="script-fn2-inrule.png"/>

To test, use [Pipeline debugger](debugger.md) and type a URL that starts with `https://meet.google.com/` and process name `slack.exe`. You will see that the URL goes through the pipeline step `ppl_gmeet`

![](script-fn2-dbg1.png)

and then the rule `rule_gmeet_from_slack` is matched.

![](script-fn2-dbg2.png)

## Printing to the terminal

%product% allows you to print messages to the terminal from the script. This is useful for debugging purposes. To print a message, use the built-in Lua `print` function, and the message will be displayed in the "Terminal" section.


