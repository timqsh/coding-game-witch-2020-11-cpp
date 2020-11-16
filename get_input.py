import json
import requests

game_id = 503086305
r = requests.post(
    "https://www.codingame.com/services/gameResultRemoteService/findByGameId",
    json=[str(game_id), None],
)
replay = r.json()
print(replay)
with open(f"{game_id}.json", "w+") as f:
    f.write(json.dumps(replay))
