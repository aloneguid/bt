# Rules

All the links that do not match any configured rules will open in the [default browser](config-basic.md#default-browser), however, that's not very interesting and not why you installed this utility in the first place.

To open a link in another browser, you need to create a **rule**.

<note>
A rule is just a text. It simply says that if a link contains the text you've typed in, then open the specified browser. It is case-insensitive so a line <strong>mydomain</strong> will match <strong>mydomain.com</strong>, <strong>MyDomain.com</strong> or <strong>https://blabla.mydomain.com</strong> just the same.
</note>

To add a rule, simply press `add` button in browser parameters, and then type the rule text.

![](rule-type.png)

You can keep adding as many rules as you want, they are always **case-insensitive** and by default match **any part or the URL**.

## Testing rules

Browser Tamer has a "URL Tester" utility which can be invoked either from "Rules" toolbar's `Test` button, or by going to `Tools`->`URL Tester` menu. URL Tester checks all the configured rules as you type, splits them in separate parts and calculates a match on where a particular URL will open.

![](rules-tester.png)

This is an excellent tool if you are not sure how configured rules will trigger.

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

To turn on regular expression, click "express delivery" icon next to the rule text.

<img height="50" src="rule-regex-check.png" alt="turn on"/>

<tip>
Internally Browser Tamer uses <a href="https://en.cppreference.com/w/cpp/regex"><code>std::regex</code></a> to validate URL against expressions.
</tip>

When typing in an expression, it has to match an **entire input** and not just a substring inside it. Here is an example.

| Input              | Expression               | Match? |
|--------------------|--------------------------|-------|
| https://github.com | `hub`                    | ❌     |
| https://github.com | `.*hub.*`                | ✅     |
| https://github.com | `http?://github\.com/.*` |  ✅    |

By default, a regular expression has to match an entire URL, however, [scoping](#scoping) applies during matching stage as well.

## Conflicting rules

Sometimes you might have a conflict in rule resolution. For instance URL `http://github.com` will match both `github` and `github.com` rule. By default, BT will take the first random rule and apply it, but you can override it by setting rule priority. The higher priority is, the higher chance the rule will be picked. To set the rule priority, first click the "priority" button, which will make a text field appear where you can type the priority number:

![](rule-priority.png)

## Frameless windows

Part of rule definition is attempt to open an URL as "frameless window". Frameless windows have no browser toolbar, url, or any other controls and try to behave like dump apps. If your browser supports frameless windows (currently Chromium-based only) you will have a button to test this behavior (1) and also rule definitions will have a button to enable frameless window triggers (2).

![](rule-frameless.png)

You can also open a link in a frameless window with [Picker](picker.md).