# Rules

All the links that do not match any configured rules will open in the [default browser](config-basic.md#default-browser), however, that's not very interesting and not why you installed this utility in the first place.

To open a link in another browser, you need to create a **rule**.

<note>
A rule is just a text. It simply says that if a link (or application title, or process name) contains the text you've typed in, then open the specified browser. It is case-insensitive so a line <strong>mydomain</strong> will match <strong>mydomain.com</strong>, <strong>MyDomain.com</strong> or <strong>https://blabla.mydomain.com</strong> just the same.
</note>

To add a rule, simply press `add` button in browser parameters, and then type the rule text.

<img height="90" src="rule-type.png" alt="rule type"/>

You can keep adding as many rules as you want, they are always **case-insensitive** and by default match **any part or the URL**.

<note>
You can test rules as you type with the <a href="debugger.md">Pipeline debugger</a> utility.
</note>

## Scoping

You can also match a part of the URL with scoping restriction:

<img height="35" src="rule-scope.png" alt="scope"/>

There are three matching scopes you can select from.

<deflist type="full">

<def title="Anywhere">
As implied, matches anywhere in the url, regardless it's a domain, path, query or anything else at all.
</def>

<def title="Domain">
Only matches inside domain part. For instance, rule <code>github</code> will match <code>http://github.com/user</code> but will not match <code>http://domesite.com/github</code>. 
</def>

<def title="Path">
Only match in path part. For instance, rule <code>github</code> will <strong>not</strong> match <code>http://github.com/user</code> but <strong>will</strong> match <code>http://domesite.com/github</code>. 
</def>

</deflist>

## Regular expressions

Regular expressions are for advanced use case only and deserve a long topic by themselves. If you have never heard of [Regular Expressions](https://en.wikipedia.org/wiki/Regular_expression), I'd strongly discourage you from using them here.

To turn on regular expression, switch on the appropriate button next to the rule text:

<img height="50" src="rule-regex-check.png" alt="turn on"/>

<tip>
Internally %product% uses <a href="https://en.cppreference.com/w/cpp/regex"><code>std::regex</code></a> to validate URL against expressions.
</tip>

When typing in an expression, it has to match an **entire input** and not just a substring inside it. Here is an example.

| Input              | Expression               | Match? |
|--------------------|--------------------------|-------|
| https://github.com | `hub`                    | ❌     |
| https://github.com | `.*hub.*`                | ✅     |
| https://github.com | `http?://github\.com/.*` |  ✅    |

By default, a regular expression has to match an entire URL, however, [scoping](#scoping) applies during matching stage as well.

## Matching locations

By default, rules are matched based on input URL. However, since v3.6.0 you can also optionally match on application title and process name. [Pipeline debugger](debugger.md) and [auditing](config-basic.md#auditing) supports these options as well.

<img height="100" src="rule-loc.png"/>

When selecting title or process (a good example [here](rules-process-name.md)), you can, just like URL matching, use regular expressions or plain substring. Note that scoping is only applicable to URL matching.

"Lua script" is a special case and is covered in [Scripting](scripting.md) topic.

## Frameless windows

Part of rule definition is attempt to open an URL as "frameless window". Frameless windows have no browser toolbar, url, or any other controls and try to behave like dump apps. If your browser supports frameless windows (currently Chromium-based only) you will have a button to test this behavior (1) and also rule definitions will have a button to enable frameless window triggers (2).

![](rule-frameless.png)

You can also open a link in a frameless window with [Picker](picker.md).

## Reordering

Rules can be reordered using <shortcut>Up</shortcut> and <shortcut>Down</shortcut> keyboard arrow keys. Focus on rule text and press the keys to move the field up or down the list. 

<video src="reorder.mp4" preview-src="reorder.png"/>

## Navigation

When inside rule text, you can press <shortcut>Tab</shortcut> to move to the next rule's text, or <shortcut>Shift+Tab</shortcut> to move the the previous rule's text. 
