# Picker

Picker is a visual prompt that asks which browser you want to open a link in specifically, instead of matching a rule. It's invoked in the following situations:

- There is more than one browser matching a specific rule. You may have a duplicated rule in several browsers, or an URL is matching different rules in different browsers. Picker will be displayed to resolve this conflict.
- Buttons `ctrl` and `shift` were held when clicking a link (this can be configured in settings). In this case, the picker will display all of the browsers registered on the system to pick from.

![](picker.png)

Simply clicking the browser you want closes this window and opens the desired browser.

The list of browsers is sorted by popularity i.e. how many times you have chosen this particular browser in the past (red square).

As a convenience you can add a rule by tapping the `+` button before a browser is selected - this will persist without the need to open the configuration dialogue.

You can also just press "copy & cancel" to close this window without opening any browsers and copying the URL into the clipboard instead.

There's also ability to try to open a browser in "frameless" window.

Picker can be switched off completely, or forced to always appear:

![](picker-on.png)