#!/usr/bin/env python3
"""Writes data/SFSE/Plugins/CinematicConversationCameraSFStrings.json, the translations template.

Every text the mod shows through N_(), T(), Fmt() or Literal() in src/*.cpp, with its English as
the value (src/Strings.h). Translators replace the values; the keys stay English.

    python tools/strings.py           write the file
    python tools/strings.py --check   fail if the file isn't current (the build runs this)
"""
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "data" / "SFSE" / "Plugins" / "CinematicConversationCameraSFStrings.json"
ABOUT = (
    "Translations of Cinematic Conversation Camera SF's settings page and listen prompt. "
    "Translate the values, never the keys; keep {} and other placeholders as they are. "
    "Anything left out or empty stays English. Save as UTF-8, and turn on your language's "
    "glyphs in SFSE Menu Framework's Fonts settings. Details: docs/TRANSLATING.md on GitHub."
)

CALL = re.compile(r'(?<![A-Za-z0-9_])(?:N_|T|Fmt|Literal)\(\s*"')
ESCAPES = {"n": "\n", "t": "\t", "r": "\r", '"': '"', "\\": "\\", "'": "'", "0": "\0"}


def literals_after(text: str, at: int) -> tuple[str, int]:
    """The C++ string literal starting at `at` (a quote), with any literals right after it
    joined, as C++ joins them; and where they end."""
    out = []
    i = at
    while True:
        assert text[i] == '"'
        i += 1
        while text[i] != '"':
            if text[i] == "\\":
                out.append(ESCAPES.get(text[i + 1], text[i + 1]))
                i += 2
            else:
                out.append(text[i])
                i += 1
        i += 1
        j = i
        while j < len(text) and text[j] in " \t\r\n":
            j += 1
        if j < len(text) and text[j] == '"':
            i = j
            continue
        return "".join(out), i


def collect() -> list[str]:
    seen: dict[str, None] = {}
    for path in sorted((ROOT / "src").glob("*.cpp")):
        if path.name == "Strings.cpp":
            continue
        text = path.read_text(encoding="utf-8")
        for match in CALL.finditer(text):
            line_start = text.rfind("\n", 0, match.start()) + 1
            if text[line_start:match.start()].lstrip().startswith("//"):
                continue
            value, _ = literals_after(text, match.end() - 1)
            seen.setdefault(value, None)
    return list(seen)


def render(texts: list[str]) -> str:
    table = {"_about": ABOUT}
    table.update({t: t for t in texts})
    return json.dumps(table, ensure_ascii=False, indent=2) + "\n"


def main() -> int:
    wanted = render(collect())
    if "--check" in sys.argv:
        current = OUT.read_text(encoding="utf-8").replace("\r\n", "\n") if OUT.exists() else ""
        if current != wanted:
            print(f"{OUT.relative_to(ROOT)} is out of date: run python tools/strings.py", file=sys.stderr)
            return 1
        print(f"{OUT.relative_to(ROOT)} is current ({len(json.loads(wanted)) - 1} texts)")
        return 0
    OUT.write_text(wanted, encoding="utf-8", newline="\n")
    print(f"wrote {OUT.relative_to(ROOT)} ({len(json.loads(wanted)) - 1} texts)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
