# URL processing

%product% can optionally pre-process URLs before applying the rules.

## Un-shortening

Since v3.5.0 URL un-shortening is enabled by default due to the fact shortened URLs mask target link and make it impossible to apply a rule to it.

For instance pressing [https://bit.ly/47EZHSl](https://bit.ly/47EZHSl) will actually open [https://github.com/aloneguid/bt](https://github.com/aloneguid/bt), allowing potential attackers to evade configured rules. 

%product% supports the most popular URL shorteners, the list of which you can find [here](https://github.com/aloneguid/bt/blob/master/bt/app/pipeline/unshortener.cpp). If it's not in the list, fire up an issue or a PR.

If for some reason you don't like to un-shorten URLs, the tool can be disabled from `Tools`->`Enable URL Un-Shortener` menu.

### Testing

You can test the un-shortener by using the "URL Tester" tool, which can be called from `Tools`->`URL Tester` menu:

<img src="unshorten.png" width="500"/>