# URL Pipeline

URL Pipeline was introduced in v3.7.0 as a way to customize URL processing even further and streamline the process of URL un-shortening and unwrapping that already existed. 

In addition to that, URL Pipeline allows to perform additional actions on URLs, the first additional one being **find/replace**.

To access URL Pipeline, go to `Settings`->`Configure URL pipeline` menu:

<img src="url-pipeline-menu.png" width="400" alt="URL Pipeline window"/>

By default, it will only have two steps - **Unwrap Office 365** and **Un-shorten**. You can add additional steps by clicking `Add step` button, or reorder existing steps by using `Move up` and `Move down` buttons. It's that simple.

<video src="url-pipeline-start.mp4" preview-src="url-pipeline-start-preview.png" width="800"/>

The rest of this page will describe each step in detail.

## Un-shortening

<warning>
Internet access is required for this feature to work.
</warning>

Since v3.5.0 URL un-shortening is enabled by default due to the fact shortened URLs mask target link and make it impossible to apply a rule to it.

For instance pressing [https://bit.ly/47EZHSl](https://bit.ly/47EZHSl) will actually open [https://github.com/aloneguid/bt](https://github.com/aloneguid/bt), allowing potential attackers to evade configured rules.

%product% supports the most popular URL shorteners, the list of which you can find [here](https://github.com/aloneguid/bt/blob/master/bt/app/pipeline/unshortener.cpp). If it's not in the list, fire up an issue or a PR.

If for some reason you don't like to un-shorten URLs, the tool can be disabled from `Tools`->`Enable URL Un-Shortener` menu.

## Office 365 link unwrapping

**Office 365** links are unwrapped for rule matching, but for security reasons wrapped URL is open. For instance,

`https://eur02.safelinks.protection.outlook.com/?url=https%3A%2F%2Fwww.google.com`

will apply rules to `http://www.google.com` but still open the original URL.

<tip>
Link unwrapping is applied to any URL of which domain name ends with <code>.safelinks.protection.outlook.com</code>.
</tip>

## Find/replace

Find/replace works as you would expect - finds a substring in the incoming URL and replaces it with another substring.

For instance, if you want to replace `google.com` with `bing.com`, you would do the following:

1. Select "find/replace" in the step type dropdown.
2. Add a new step by clicking `Add step` button.
3. Enter `google.com` in the `Find` field.
4. Enter `bing.com` in the `Replace` field.

For more advanced scenarios, you can use regular expressions. For instance, if you want to replace all URLs that start with `http://` with `https://`, you would do the following:

1. Select "find/replace" in the step type dropdown.
2. Add a new step by clicking `Add step` button.
3. Enter `^http://` in the `Find` field.
4. Enter `https://` in the `Replace` field.

<img src="pipeline-regex.png" alt="Regex replacement"/>

## Testing

You can test the un-shortener by using the "URL Tester" tool, which can be called from `Tools`->`URL Tester` menu:

<img src="unshorten.png" width="500" alt="URL Tester window"/>


