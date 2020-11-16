import json, os

# read the replay from file
with open("replay.json", "r") as f:
    replay = json.loads(f.read())

stderr = []
for frame in replay["success"]["frames"]:
    if not "stderr" in frame.keys():
        continue
    for err in frame["stderr"].split("\n"):
        # some of my stderr lines aren't referee input. I marked them with '#' to filter them
        if not err.startswith("#"):
            stderr.append(err)

# write the errorstream to the file 'input.txt'
with open("input.txt", "w+") as f:
    f.write("\n".join(stderr))
print("\n".join(stderr))
