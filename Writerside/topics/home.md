# Welcome to Browser Tamer!

## Pitch
Do you have multiple browsers or browser profiles on your machine? Do you want to open different links with different browsers or profiles without switching manually?

If yes, then you need Browser Tamer! Browser Tamer is a smart and powerful tool that acts as a browser proxy on your machine. It catches the links you click and redirects them to a browser or browser profile of your choice. You can set up rules based on the link’s domain, protocol, file extension and more.

![](one.png){border-effect=line}

## Features at a Glance

- Extremely lightweight on memory and resources, written in safe modern C++.
    - Self-contained single `.exe` **under 2 Mb in size** with no dependencies.
    - Available as `.msi` [installer](Installing.md).
    - [Portable mode](Installing.md) supported.
    - Fits on a floppy disk if you can find it in 2023! 💾
- Completely free and [open-source](https://github.com/aloneguid/bt).
- Intelligent detection of the most popular browsers.
    - **Firefox**, **Edge**, **Chrome**, **Brave** and so on.
    - Detection of browser profiles.
    - Support for [Firefox Containers](Installing.md).
    - Support for incognito mode / tor mode.
- Add your own, custom browser or application customised with any parameters you want.
- Rule-based redirect based on matching inside entire URL, domain, or path. This can be a simple case-insensitive substring or a regular expression.
- Assign rule priorities.
- **[Extensions](Installing.md)** to integrate with [Chrome](https://chrome.google.com/webstore/detail/browser-tamer/oggcljknmiiomjekepdoindjcpnpglnd), [Edge](https://microsoftedge.microsoft.com/addons/detail/browser-tamer/gofjagaghddmjloaecpnldjmjlplicin), [Firefox](https://addons.mozilla.org/en-GB/firefox/addon/browser-tamer/), or any Chromium-based or Firefox-based browser (Opera, Vivaldi, Brave, Waterfox etc.).
- Open links in "chromeless (frameless)" window.
- Optional [audit](https://www.aloneguid.uk/posts/2023/07/bt-log-to-file/) of rule hits to a csv file.
- Beautiful UI based on [ImGui](https://github.com/ocornut/imgui) (GPU-accelerated UI engine used in games) with support for [themes](Installing.md).

## Write content
%product% supports two types of markup: Markdown and XML.
When you create a new help article, you can choose between two topic types, but this doesn't mean you have to stick to a single format.
You can author content in Markdown and extend it with semantic attributes or inject entire XML elements.

## Inject XML
For example, this is how you inject a procedure:

<procedure title="Inject a procedure" id="inject-a-procedure">
    <step>
        <p>Start typing and select a procedure type from the completion suggestions:</p>
        <img src="completion_procedure.png" alt="completion suggestions for procedure" border-effect="line"/>
    </step>
    <step>
        <p>Press <shortcut>Tab</shortcut> or <shortcut>Enter</shortcut> to insert the markup.</p>
    </step>
</procedure>

## Add interactive elements

### Tabs
To add switchable content, you can make use of tabs (inject them by starting to type `tab` on a new line):

<tabs>
    <tab title="Markdown">
        <code-block lang="plain text">![Alt Text](new_topic_options.png){ width=450 }</code-block>
    </tab>
    <tab title="Semantic markup">
        <code-block lang="xml">
            <![CDATA[<img src="new_topic_options.png" alt="Alt text" width="450px"/>]]></code-block>
    </tab>
</tabs>

### Collapsible blocks
Apart from injecting entire XML elements, you can use attributes to configure the behavior of certain elements.
For example, you can collapse a chapter that contains non-essential information:

#### Supplementary info {collapsible="true"}
Content under such header will be collapsed by default, but you can modify the behavior by adding the following attribute:
`default-state="expanded"`

### Convert selection to XML
If you need to extend an element with more functions, you can convert selected content from Markdown to semantic markup.
For example, if you want to merge cells in a table, it's much easier to convert it to XML than do this in Markdown.
Position the caret anywhere in the table and press <shortcut>Alt+Enter</shortcut>:

<img src="convert_table_to_xml.png" alt="Convert table to XML" width="706" border-effect="line"/>

## Feedback and support
Please report any issues, usability improvements, or feature requests to our 
<a href="https://youtrack.jetbrains.com/newIssue?project=WRS">YouTrack project</a>
(you will need to register).

You are welcome to join our
<a href="https://join.slack.com/t/writerside/shared_invite/zt-1hnvxnl0z-Nc6RWXTppRI2Oc566vumYw">public Slack workspace</a>.
Before you do, please read our [Code of conduct](https://plugins.jetbrains.com/plugin/20158-writerside/docs/writerside-code-of-conduct.html).
We assume that you’ve read and acknowledged it before joining.

You can also always send an email to [writerside@jetbrains.com](mailto:writerside@jetbrains.com).

<seealso>
    <category ref="wrs">
        <a href="https://plugins.jetbrains.com/plugin/20158-writerside/docs/markup-reference.html">Markup reference</a>
        <a href="https://plugins.jetbrains.com/plugin/20158-writerside/docs/manage-table-of-contents.html">Reorder topics in the TOC</a>
        <a href="https://plugins.jetbrains.com/plugin/20158-writerside/docs/local-build.html">Build and publish</a>
        <a href="https://plugins.jetbrains.com/plugin/20158-writerside/docs/configure-search.html">Configure Search</a>
    </category>
</seealso>
