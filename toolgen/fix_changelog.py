import re

with open("CHANGELOG.md", "r", encoding="utf-8") as f:
    text = f.read()

# We'll just define the specific sections we want and write a clean CHANGELOG.md.
# I will extract the blocks and order them correctly.

