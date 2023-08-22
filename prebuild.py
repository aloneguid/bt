import json
import requests

HEADER_FILE_PATH = "bt/app/security/clearurls_rules.inl"

def download_latest_clanurl_data() -> dict:
    response = requests.get("https://gitlab.com/ClearURLs/rules/-/raw/master/data.min.json?ref_type=heads")
    return response.json()


# %%

jd = download_latest_clanurl_data()

# %%

code = [
    "#pragma once",
    "#include <vector>",
    "#include \"clearurls.h\"",
    "",
    "namespace bt::security {",
    "    const std::vector<clearurl_provider> clearurl_providers = {"]

for name, data in jd["providers"].items():
    url_pattern: str = data["urlPattern"]
    # url_pattern = url_pattern.lstrip("^")

    qps = [f"\"{x}\"" for x in data["rules"]]
    qps = "{" + ", ".join(qps) + "}"
    code.append(f"        {{\"{name}\", \"{url_pattern}\", {{")
    for rule in data["rules"]:
        code.append(f"            \"{rule}\"")
    code.append("        }, {")
    for x in data["exceptions"]:
        code.append(f"            \"{x}\"")
    code.append("        }},")

code.append("    };")
code.append("}")

# dump to file
with open(HEADER_FILE_PATH, "w") as inl:
    code_feeds = [line + "\n" for line in code]
    inl.writelines(code_feeds)
