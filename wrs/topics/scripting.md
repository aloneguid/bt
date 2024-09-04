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

todo

## Combining URL processing and rule matching

todo

## Printing to the terminal

todo


