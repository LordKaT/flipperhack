import sys

opiescript_instructions = {
    "LOG": {
        "argc": 1,
        "opcode": 0x01,
    },
    "END_SCRIPT": {
        "argc": 0,
        "opcode": 0xFF,
    },
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
            opiescript_error(lineno, f"Unknown instruction {mnemonic}")
            #raise ValueError(f"Line {lineno}: unknown instruction {mnemonic}")

        instr = instr_table[mnemonic]
        argc = instr["argc"]

        if len(tokens) - 1 != argc:
            opiescript_error(lineno, f"{mnemonic} expects {argc} args, got {len(tokens) - 1}")
            #raise ValueError(f"[{lineno}]: {mnemonic} expects {argc} args, {len(tokens) -1} given.")
        
        bytecode.append(instr["opcode"])

        for arg in tokens[1:]:
            if arg not in string_symbols:
                raise ValueError(f"[{lineno}]: unknown symbol {arg}")
            bytecode.append(string_symbols[arg])

    # We are very opinionated
    end_opcode = instr_table["END_SCRIPT"]["opcode"]
    if not bytecode or bytecode[-1] != end_opcode:
        print("Warning: No END_SCRIPT detected.")
        bytecode.append(end_opcode)

    return bytes(bytecode)
