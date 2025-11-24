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
tools_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, tools_dir)

from api_generator import parser

SETGET_PAIRS_TEMPLATE = """
typedef void (**FnT)(void);
struct FnPair {
    const char* name;
    FnT function;
};

// TODO: generate with API Generator
const struct FnPair pairs[] = {
PAIRS
};
"""

SIGNATURE_HEADER = """
#ifndef SIGNATURE_PREFIX
#define SIGNATURE_PREFIX static
#endif

"""


def main(parse_result):
    for server in parse_result.servers:
        path = pathlib.Path(server.filename)
        pairs = ['{"_init", (FnT) &backend->_init}', '{"_quit", (FnT) &backend->_quit}']
        signatures = SIGNATURE_HEADER
        for f in server.methods:
            pairs.append(f'{{"{f.name}", (FnT) &backend->{f.name}}}')

            args = ", ".join([f"{arg._type}" for arg in f.args])
            if args == "":
                args = "void"
            signatures += f"SIGNATURE_PREFIX {f.return_type} {f.name} ({args});\n"

        out = SETGET_PAIRS_TEMPLATE.replace("PAIRS", ",\n".join(pairs))
        with open(path.parent.joinpath("setget-pairs-table.h.gen"), "w") as f:
            f.write(out)

        with open(path.parent.joinpath("methods-signatures.h.gen"), "w") as f:
            f.write(signatures)
    # print(json.dumps(dict(parse_result)["servers"], indent=4))
    #


if __name__ == "__main__":
    parse_result = parser.parse()
    main(parse_result)
