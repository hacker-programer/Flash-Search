files = {}
text = ""
with open("src/common/flash_search.h", "r") as f:
    files[f.name] = f.read()
with open("src/common/flash_search.cpp", "r") as f:
    files[f.name] = f.read()
with open("src/common/load.h", "r") as f:
    files[f.name] = f.read()
with open("src/common/load.cpp", "r") as f:
    files[f.name] = f.read()
with open("src/analistics.cpp", "r") as f:
    files[f.name] = f.read()
with open("src/benchmarks.cpp", "r") as f:
    files[f.name] = f.read()
with open("src/unit-test.cpp", "r") as f:
    files[f.name] = f.read()
with open("CMakeLists.txt", "r") as f:
    files[f.name] = f.read()

for k, v in files.items():
    text += f"\n\n\n{k}:\n"
    text += f"{v}\n"

import pyperclip
pyperclip.copy(text)