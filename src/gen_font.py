#!/usr/bin/env python3
from os import getenv
from os.path import dirname

from fontTools.subset import Subsetter
from fontTools.ttLib import TTFont

font = TTFont(getenv("iosevka"))

REPL = 0xFFFD  # unicode replacement character "�"

subsetter = Subsetter()
subsetter.populate(unicodes=(list(range(32, 128)) + [REPL]))  # printable ASCII
subsetter.subset(font)

glyphs = font["glyf"].glyphs
for table in font["cmap"].tables:
    if table.isUnicode():
        repl = table.cmap.get(REPL)
        if repl != None:
            glyphs[".notdef"] = glyphs[repl]
            break

font.save(dirname(__file__) + "/../res/iosevka.ttf")
