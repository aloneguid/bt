import json
import requests


def download_latest_clanurl_data() -> dict:
    response = requests.get("https://gitlab.com/ClearURLs/rules/-/raw/master/data.min.json?ref_type=heads")
    return response.json()


# %%

jd = download_latest_clanurl_data()




# %%
