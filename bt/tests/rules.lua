-- initial payload for testing
p = {
    url = "https://microsoft.com/other",
    wt = "Slack",
    pn = "slack.exe"
}

function work_slack()

    -- extract domain part from p.url
    domain = string.match(p.url, "https://(.-)/")
    print("domain is " .. domain)

    -- check if domain is microsoft.com and process name is slack.exe, then return true
    if domain == "microsoft.com" and p.pn == "slack.exe" then
        p.url = p.url .. "?authuser=3"
        return true
    end

    return false
end

print("--- Before: ---")
print("url: " .. p.url)
print("wt:  " .. p.wt)
print("pn:  " .. p.pn)

matches = work_slack()
print("matches: " .. tostring(matches))

print("--- After: ---")
print("url: " .. p.url)
print("wt:  " .. p.wt)
print("pn:  " .. p.pn)