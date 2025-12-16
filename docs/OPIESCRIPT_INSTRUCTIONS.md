# OpieScript Instruction Set

| Mnemonic | Opcode | Args | Arg Types |
|----------|--------|------|-----------|
| `NOP` | `0x00` | 0 | — |
| `LOG` | `0x01` | 1 | str |
| `LOADI` | `0x10` | 2 | reg, imm |
| `LOADP` | `0x11` | 2 | reg, stat |
| `STOREP` | `0x12` | 2 | stat, reg |
| `LOGR` | `0x13` | 1 | reg |
| `CMP` | `0x20` | 2 | reg, reg |
| `JMP` | `0x21` | 1 | rel8 |
| `JNZ` | `0x22` | 1 | rel8 |
| `JZ` | `0x23` | 1 | rel8 |
| `CMPI` | `0x24` | 2 | reg, imm |
| `MOV` | `0x25` | 2 | reg, reg |
| `CLR` | `0x26` | 1 | reg |
| `TEST` | `0x27` | 1 | reg |
| `INC` | `0x30` | 1 | reg |
| `DEC` | `0x31` | 1 | reg |
| `ADD` | `0x32` | 2 | reg, imm |
| `SUB` | `0x33` | 2 | reg, imm |
| `MUL` | `0x34` | 2 | reg, imm |
| `DIV` | `0x35` | 2 | reg, imm |
| `END_SCRIPT` | `0xFF` | 0 | — |
