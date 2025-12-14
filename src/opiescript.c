#include "opiescript.h"

uint32_t opiescript_regs[4];
bool opiescript_zero_flag;

static inline void opiescript_init() {
    for (uint8_t i = 0; i < 4; i++)
        opiescript_regs[i] = 0;
    opiescript_zero_flag = false;
}

static inline void opiescript_cleanup(File* file) {
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void opiescript_run(GameState* state, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if (!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E("OpieScript", rom_read_string(STR_OPEN_FAIL));
        opiescript_cleanup(file);
    }

    uint8_t buffer[OPIESCRIPT_MAX_SIZE];
    size_t size = storage_file_read(file, buffer, sizeof(buffer));

    uint8_t* pc = buffer;
    uint8_t* end = buffer + size;

    while (pc < end) {
        uint8_t opcode = *pc++;
        uint8_t reg = 0;
        uint8_t imm = 0;
        int32_t offset = 0;

        switch (opcode) {
            case OPIESCRIPT_OP_LOG:
                if (pc >= end)
                    break;
                imm = *pc++;
                log_msg(state, rom_read_string(imm));
                break;

            case OPIESCRIPT_OP_LOADI:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4)
                    opiescript_regs[reg] = imm;
                break;

            case OPIESCRIPT_OP_LOADP:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                //if (reg < 4)
                //    opiescript_regs[reg] = player_get_stat(state, imm);
                break;

            case OPIESCRIPT_OP_STOREP:
                if (pc + 2 > end)
                    break;
                imm = *pc++;
                reg  = *pc++;
                //if (reg < 4)
                //    player_set_stat(state, imm, opiescript_regs[reg]);
                break;

            case OPIESCRIPT_OP_LOGR:
                if (pc >= end)
                    break;
                reg = *pc++;
                if (reg < 4)
                    FURI_LOG_I("OpieScript", "R%d: %lu", reg, opiescript_regs[reg]);
                break;
            
            case OPIESCRIPT_OP_CMP:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4 && imm < 4)
                    opiescript_zero_flag = (opiescript_regs[reg] == opiescript_regs[imm]);
                break;
            
            case OPIESCRIPT_OP_JMP:
                if (pc >= end)
                    break;
                offset = *pc++;
                pc += offset;
                break;
            
            case OPIESCRIPT_OP_JNZ:
                if (pc >= end)
                    break;
                offset = *pc++;
                if (!opiescript_zero_flag)
                    pc += offset;
                break;
            
            case OPIESCRIPT_OP_JZ:
                if (pc >= end)
                    break;
                offset = *pc++;
                if (opiescript_zero_flag)
                    pc += offset;
                break;

            case OPIESCRIPT_OP_CMPI:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4)
                    opiescript_zero_flag = (opiescript_regs[reg] == imm);
                break;
            
            case OPIESCRIPT_OP_MOV:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4 && imm < 4)
                    opiescript_regs[reg] = opiescript_regs[imm];
                break;
            
            case OPIESCRIPT_OP_CLR:
                if (pc >= end)
                    break;
                reg = *pc++;
                if (reg < 4)
                    opiescript_regs[reg] = 0;
                break;
            
            case OPIESCRIPT_OP_INC:
                if (pc >= end)
                    break;
                reg = *pc++;
                if (reg < 4)
                    opiescript_regs[reg]++;
                break;
            
            case OPIESCRIPT_OP_DEC:
                if (pc >= end)
                    break;
                reg = *pc++;
                if (reg < 4)
                    opiescript_regs[reg]--;
                break;
            
            case OPIESCRIPT_OP_ADD:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4)
                    opiescript_regs[reg] += imm;
                break;
            
            case OPIESCRIPT_OP_SUB:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4)
                    opiescript_regs[reg] -= imm;
                break;
            
            case OPIESCRIPT_OP_MUL:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4)
                    opiescript_regs[reg] *= imm;
                break;
            
            case OPIESCRIPT_OP_DIV:
                if (pc + 2 > end)
                    break;
                reg = *pc++;
                imm = *pc++;
                if (reg < 4)
                    opiescript_regs[reg] /= imm;
                break;

            case OPIESCRIPT_OP_END_SCRIPT:
                break;

            default:
                FURI_LOG_E("OpieScript", "%s%d", rom_read_string(STR_INVALID_OPCODE), opcode);
                break;
        }
    }
    opiescript_cleanup(file);
}
