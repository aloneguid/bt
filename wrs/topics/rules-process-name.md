# Match by process (application) name

To open a link when it's invoked by a specific application, you need to create a **rule** with "Process" matching location:

![](rule-process.png)

In this field, you need to type name of the process you want to watch. Let's say, for example, I want to open all the links from [Thunderbird](https://www.thunderbird.net/en-GB/) in a specific browser.

Considering that Thunderbird is running, I can find out the process name by opening Task Manager (Ctrl+Shift+Esc) and looking for the application in the list. Then, expand the process and choose "Go to details" to see the process name:

![](rules-thunderbird.png)

In this case, the process name is `thunderbird.exe`. So, I can create a rule with this process name:

![](rule-process-filled.png)

Close %product% settings, or press "File" -> "Save" to apply the changes.