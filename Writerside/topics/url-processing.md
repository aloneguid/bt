# URL processing

%product% can optionally pre-process URLs before applying the rules.

## Un-shortening

<warning>
Internet access is required for this feature to work.
</warning>

Since v3.5.0 URL un-shortening is enabled by default due to the fact shortened URLs mask target link and make it impossible to apply a rule to it.

For instance pressing [https://bit.ly/47EZHSl](https://bit.ly/47EZHSl) will actually open [https://github.com/aloneguid/bt](https://github.com/aloneguid/bt), allowing potential attackers to evade configured rules. 

%product% supports the most popular URL shorteners, the list of which you can find [here](https://github.com/aloneguid/bt/blob/master/bt/app/pipeline/unshortener.cpp). If it's not in the list, fire up an issue or a PR.

If for some reason you don't like to un-shorten URLs, the tool can be disabled from `Tools`->`Enable URL Un-Shortener` menu.

## Office 365 link unwrapping

Since v3.5.1 **Office 365** links are unwrapped for rule matching, but for security reasons wrapped URL is open. For instance,

`https://eur02.safelinks.protection.outlook.com/?url=https%3A%2F%2Fwww.google.com`

will apply rules to `http://www.google.com` but still open the original URL.

<tip>
Link unwrapping is applied to any URL of which domain name ends with <code>.safelinks.protection.outlook.com</code>.
</tip>

### Testing

You can test the un-shortener by using the "URL Tester" tool, which can be called from `Tools`->`URL Tester` menu:

<img src="unshorten.png" width="500" alt="URL Tester window"/>
