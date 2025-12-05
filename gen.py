#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.13"
# dependencies = [
#     #"libclang==12.0.0",
#     "libclang>=18.0.0",
# ]
# ///


import sys
import os
import pathlib
import json

# Добавляем tools в Python path
tools_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tools/")
sys.path.insert(0, tools_dir)

apg = __import__("api-generator")
server_gen = __import__("server-setget-pairs-generator")

from tools.api_generator import parser


def main():
    parse_result = parser.parse()
    apg.main(parse_result)
    server_gen.main(parse_result)


if __name__ == "__main__":
    main()
