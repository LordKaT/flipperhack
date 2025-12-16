#!/usr/bin/env python3
from pathlib import Path
from lib.opiescript import opiescript_instructions

def generate_markdown(instr_table: dict) -> str:
    lines = []
    lines.append("# OpieScript Instruction Set\n")
    lines.append("| Mnemonic | Opcode | Args | Arg Types |")
    lines.append("|----------|--------|------|-----------|")

    for mnemonic, instr in sorted(
        instr_table.items(), key=lambda x: x[1]["opcode"]
    ):
        opcode = f"0x{instr['opcode']:02X}"
        argc = instr.get("argc", 0)
        argtypes = instr.get("argtypes", [])
        argtypes_str = ", ".join(argtypes) if argtypes else "—"

        lines.append(
            f"| `{mnemonic}` | `{opcode}` | {argc} | {argtypes_str} |"
        )

    return "\n".join(lines) + "\n"


def main():
    # Path of this script
    script_dir = Path(__file__).resolve().parent

    # ../docs relative to this script
    docs_dir = (script_dir / ".." / "docs").resolve()
    docs_dir.mkdir(parents=True, exist_ok=True)

    out_path = docs_dir / "OPIESCRIPT_INSTRUCTIONS.md"

    markdown = generate_markdown(opiescript_instructions)
    out_path.write_text(markdown)

    print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
