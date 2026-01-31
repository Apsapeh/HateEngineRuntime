#!/usr/bin/env python

import sys
import re

test_text = """
u8 RenderServerTextureMipmap

Nearest
Linear
Cubic    =        1000  // Comment  oeuoe oe u oe
Bicubic

"""

ALLOWED_TYPES = ("u8", "u16", "u32", "u64", "i8", "i16", "i32", "i64")


API_TEMPLATE = """/*
API ENUM {{
        "name": "{}",
        "type": "{}",
        "values": [
{}
        ]
}}
*/"""

API_VALUE_TEMPLATE = """                ["{}", {}]"""


class Value:
    def __init__(self, name, value, comment="") -> None:
        self.name = name
        self.value = value
        self.comment = comment


def main():
    data = sys.stdin.read()
    parser(data)


def parser(text):
    # Split text by lines and remove empty
    lines = text.splitlines()
    lines = list(filter(lambda x: x.strip(), lines))

    # Text should be contains header and one value
    if len(lines) < 2:
        err()

    # "   u8   Name  " -> "u8", "Name"
    type_name_line = [w for w in lines[0].split(" ") if w.strip()]

    if len(type_name_line) < 2:
        err()

    # I'm to lazy to add the checks
    enum_type = type_name_line[0]
    enum_name = type_name_line[1]

    counter = 0
    values = []
    for l in lines[1:]:
        # Split by comment "val // comm" -> "val ", " comm"
        splitted_line = l.split("//", 1)

        # Normalize comment " comm" -> "comm"
        comment = ""
        if len(splitted_line) == 2:
            comment = splitted_line[1].strip()

        # Normalize value part "  val    =   1000   " -> "val", "=", "1000"
        value_stripped_part = [
            w for w in splitted_line[0].strip().split(" ") if w.strip()
        ]

        if len(value_stripped_part) == 1:  # "val"
            values.append(Value(value_stripped_part[0], counter, comment))

        elif len(value_stripped_part) == 3:  # "value = 1000"
            if value_stripped_part[1] != "=":  # Shit
                err()

            counter = int(value_stripped_part[2])
            values.append(Value(value_stripped_part[0], counter, comment))
        else:  # Shit
            err()

        # Should be PaskalCase[
        if value_stripped_part[0].islower():
            err("Should be PaskalCase: {}".format(value_stripped_part))

        counter += 1

    values_api_str = ""
    for i, v in enumerate(values):
        values_api_str += API_VALUE_TEMPLATE.format(v.name, v.value)
        if i < len(values) - 1:
            values_api_str += ",\n"

    result = ""
    # result += API_TEMPLATE.format(enum_name, enum_type, values_api_str) + "\n\n"
    #
    # # Defines
    # result += "// clang-format off\n"
    # # for v in values:
    # TEMPLATE = "#define {} {}"
    # result += "\n".join(
    #     map(
    #         lambda v: TEMPLATE.format(camel_to_uppersnake(enum_name + v.name), v.value),
    #         values,
    #     )
    # )
    # result += "\n"
    # result += "// clang-format on\n\n"

    result += "/**\n"
    result += " * @api\n"
    result += " */\n"
    result += "enum {\n"
    for v in values:
        if v.comment != "":
            result += f"    // {v.comment}\n"
        result += f"    {camel_to_uppersnake(enum_name + v.name)} = {v.value},\n"
    result += "};\n\n"

    # Typedef
    result += "/**\n"
    for v in values:
        result += " * {} - {}".format(v.value, v.name)
        if v.comment != "":
            result += ": " + v.comment
        result += "\n\n"
    result += " * @api\n"
    result += " */\n"
    result += "typedef {} {};".format(enum_type, enum_name)
    result += "\n"

    print(result)


def camel_to_uppersnake(name):
    name = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub("([a-z0-9])([A-Z])", r"\1_\2", name).upper()


def err(msg=None):
    example = """enum_type enum_name

Value_1     // 0
Value_2     // 1
Value_3 = 5 // 5
Value_4     // 6"""

    eprint(example)

    exit(1)


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


if __name__ == "__main__":
    main()
