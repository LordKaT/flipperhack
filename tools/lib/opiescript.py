import sys

opiescript_instructions = {
    "LOG": {
        "argc": 1,
        "opcode": 0x01,
        "argtypes": ["str"],
    },
    "LOADI": {
        "argc": 2,
        "opcode": 0x10,
        "argtypes": ["reg", "imm"],
    },
    "LOADP": {
        "argc": 2,
        "opcode": 0x11,
        "argtypes": ["reg", "stat"],
    },
    "STOREP": {
        "argc": 2,
        "opcode": 0x12,
        "argtypes": ["stat", "reg"],
    },
    "LOGR": {
        "argc": 1,
        "opcode": 0x13,
        "argtypes": ["reg"],
    },
    "END_SCRIPT": {
        "argc": 0,
        "opcode": 0xFF,
    },
}

opiescript_register_symbols = {
    "R0": 0,
    "R1": 1,
    "R2": 2,
    "R3": 3
}

opiescript_stats_symbols = {
    "STAT_HP":      0,
    "STAT_MAX_HP":  1,
    "STAT_STR":     2,
    "STAT_DEX":     3,
    "STAT_CON":     4,
    "STAT_INTL":    5,
    "STAT_WIS":     6,
    "STAT_CHA":     7,
    "STAT_SP":      8,
    "STAT_SP_MAX":  9,
}

'''
OpieScript bytecode format:

[ opcode ] [ operand 1 ] [ operand 2 ] ... [ operand N ]
'''

def opiescript_error(lineno: int, err: str):
    print(f"ERROR: {lineno}: {err}")
    sys.exit(1)

def opiescript_tokenizer(source: str):
    for lineno, line in enumerate(source.splitlines(), start=1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue # ignore comments

        yield lineno, line.split()

def opiescript_assembler(source: str, instr_table: dict, string_symbols: dict):
    bytecode = bytearray()

    for lineno, tokens in opiescript_tokenizer(source):
        mnemonic = tokens[0]

        if mnemonic not in instr_table:
            opiescript_error(lineno, f"unknown instruction: {mnemonic}")

        instr = instr_table[mnemonic]
        argc = instr["argc"]

        if len(tokens) - 1 != argc:
            opiescript_error(lineno, f"{mnemonic} expects {argc} args, got {len(tokens) - 1}")

        bytecode.append(instr["opcode"])
        argtypes = instr.get("argtypes", [])

        for arg, argtype in zip(tokens[1:], argtypes):
            if argtypes and len(argtypes) != argc:
                opiescript_error(lineno, f"{mnemonic} argtypes mismatch (argc={argc}, argtypes={len(argtypes)})")
            if argtype == "reg":
                if arg not in opiescript_register_symbols:
                    opiescript_error(lineno, f"unknown register: {arg}")
                bytecode.append(opiescript_register_symbols[arg])

            elif argtype == "imm":
                value = int(arg, 0)
                if not (0 <= value <= 255):
                    opiescript_error(lineno, f"immediate out of range")
                bytecode.append(value)

            elif argtype == "stat":
                if arg not in opiescript_stats_symbols:
                    opiescript_error(lineno, f"unknown stat: {arg}")
                bytecode.append(opiescript_stats_symbols[arg])

            elif argtype == "str":
                if arg not in string_symbols:
                    opiescript_error(lineno, f"unknown string: {arg}")
                bytecode.append(string_symbols[arg])

    # We are very opinionated
    end_opcode = instr_table["END_SCRIPT"]["opcode"]
    if not bytecode or bytecode[-1] != end_opcode:
        print("Warning: No END_SCRIPT detected.")
        bytecode.append(end_opcode)

    return bytes(bytecode)
