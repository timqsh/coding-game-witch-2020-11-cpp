import sys

import requests

from env import email, pw, userID

game_id = sys.argv[1]

# the session object saves cookies
with requests.Session() as s:
    p = s.post(
        "https://www.codingame.com/services/CodingamerRemoteService/loginSiteV2",
        json=[email, pw, True],
    )
    p.raise_for_status()
    r = s.post(
        "https://www.codingame.com/services/gameResultRemoteService/findByGameId",
        json=[str(game_id), userID],
    )
    r.raise_for_status()
    replay = r.json()

stderr = []
for frame in replay["success"]["frames"]:
    if "stderr" not in frame.keys():
        continue
    for err in frame["stderr"].split("\n"):
        # some of my stderr lines aren't referee input. I marked them with '#' to filter
        if not err.startswith("#"):
            stderr.append(err)

# write the errorstream to the file 'input.txt'
with open("input.txt", "w+") as f:
    f.write("\n".join(stderr))
