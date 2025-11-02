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
import json

# Добавляем tools в Python path
tools_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, tools_dir)

from api_generator import parser, c_src_generator, c_api_generator


def help():
    print("Usage: api_generator [--json]")
    print("  --json   Output the parsed API as JSON to stdout")


def main(parse_result):
    # aboba.main()
    # aboba.parse_file(sys.argv[1])
    # parser.parse(sys.argv[1])
    c_src_generator.run(parse_result)
    c_api_generator.run(parse_result)

    if "--json" in sys.argv:
        print(json.dumps(dict(parse_result), indent=4))

    return


if __name__ == "__main__":
    if "--help" in sys.argv or "-h" in sys.argv:
        help()
    else:
        parse_result = parser.parse()
        main(parse_result)
