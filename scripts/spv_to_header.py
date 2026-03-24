#!/usr/bin/env python3
"""Convert a compiled SPIR-V .spv file into a C++ uint32_t array header."""
import sys
import os


def spv_to_header(spv_path: str, symbol: str, out_path: str) -> None:
    with open(spv_path, "rb") as f:
        data = f.read()

    if len(data) % 4 != 0:
        raise ValueError(f"{spv_path}: size {len(data)} is not a multiple of 4")

    words = []
    for i in range(0, len(data), 4):
        word = int.from_bytes(data[i : i + 4], "little")
        words.append(f"0x{word:08x}")

    word_count = len(words)
    src_name = os.path.basename(spv_path)

    lines = [
        "#pragma once",
        f"// Auto-generated from {src_name} \xe2\x80\x94 do not edit by hand.",
        "#include <cstdint>",
        "// clang-format off",
        f"static constexpr uint32_t {symbol}[] = {{",
    ]

    for i in range(0, word_count, 8):
        chunk = ", ".join(words[i : i + 8])
        lines.append(f"    {chunk},")

    lines += [
        "};",
        "// clang-format on",
        f"static constexpr uint32_t {symbol}Size = {word_count}u;",
        "",
    ]

    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    print(f"  Written {out_path}  ({word_count} words)")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: spv_to_header.py <input.spv> <symbol_name> <output.h>")
        sys.exit(1)
    spv_to_header(sys.argv[1], sys.argv[2], sys.argv[3])
