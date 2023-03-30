#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// #include <RISK_TYPES.h>

#define MAX_INSTRUCTION_BYTES 1024
#define MAX_REGISTER_BYTES 1024

// Helper functions
int read_binary_file(const char* filename, unsigned char* buffer) {
    FILE* file;
    int size;

    file = fopen(filename, "rb"); // open the binary file for reading
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // read up to MAX_BYTES bytes from the file into the buffer
    size = fread(buffer, 1, MAX_INSTRUCTION_BYTES, file);
    if (ferror(file)) {
        perror("Error reading file");
        size = -1;
    }

    fclose(file); // close the file
    return size; // return the number of bytes read
}


void print_buffer(unsigned char* buffer, int size) {
    for (int i = 0; i < size / 4; i++) {
        printf("%02x ", buffer[i]); // print each byte as a two-digit hexadecimal value
    }
    printf("\n");
}


unsigned int combine_four_bytes(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4) {
    unsigned int instruction = 0;
    instruction |= (unsigned int)b1;
    instruction |= (unsigned int)b2 << 8;
    instruction |= (unsigned int)b3 << 16;
    instruction |= (unsigned int)b4 << 24;
    return instruction;
}
uint16_t combine_two_bytes(unsigned char b1, unsigned char b2) {
    uint16_t instruction = 0;
    instruction |= (unsigned int)b1;
    instruction |= (unsigned int)b2 << 8;
    return instruction;
}

void print_bits(unsigned int instruction, unsigned int length) {
    for (int i = length - 1; i >= 0; i--) {
        printf("%c", (instruction & (1 << i)) ? '1' : '0');
    }
    printf("\n");
}

void print_registers(unsigned int registers[32]) {
    for (int i = 0; i < 32; i++) {
        printf("Register %d: %u\n", i, registers[i]);
    }
}

void print_memory(unsigned char memory[1024]) {
    for (int i = 0; i < 1024; i += 4) {
        printf("Byte: %d -- %02x %02x %02x %02x\n", i, memory[i], memory[i+1], memory[i+2], memory[i+3]);
    }
}

// Register Dump
void register_dump(unsigned int *pc, unsigned int *registers) {
    printf("PC = 0x%08X\n", *pc);
    for (int i = 0; i < 32; i ++) {
        printf("R[%d] = 0x%08X\n", i, registers[i]);
    }
}

// Instruction not Implemented Helper
void not_implemented(unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    printf("Instruction Not Implemented: 0x%08X\n", *instruction);
    register_dump(pc, registers);
    exit(0);
}

// Illegal Operation Helper
void illegal_operation(unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    printf("Illegal Operation: 0x%08X\n", *instruction);
    register_dump(pc, registers);
    exit(0);
}

// Ensure PC not out of bounds
bool valid_pc(unsigned int *pc) {
    if (*pc < 0 || *pc >1024) {
        return false;
    } else {
        return true;
    }
}

// Store in register helper
void store_in_register(unsigned int *registers, unsigned char store_in, int set_to) {
    if (store_in == 0) {
        // Cannot set zero register - Ignore any write
    } else if (store_in > 31) {
        // Only 31 registers should be prevented anyway since rd, rs1, rs2 only 5 bits
    } else {
        // Set register 
        registers[store_in] = set_to;
    }
}
// Read Regsiter - Register always specified in instruction not by program thus cannot exceed 5 bits. (Do not need helper)

// Store in memory helper
void store_in_memory(unsigned char *memory, unsigned int address, unsigned int value, unsigned int num_bytes, unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    // address = address - 0x0400;
    if (address < 0x0400) {
        // Throw error, Cannot overwrite instruction memory 
        // Illegal operation
        printf("Can't write to instructions\n");
        illegal_operation(pc, registers, instruction);
    } 
    else if (address > 0x8FF) {
        // Exceeds Memory & Virtual Routine bounds
        // Illegal operation
        printf("Exceeds memory bounds\n");
        illegal_operation(pc, registers, instruction);
    }

    // -------------------Virtual Routines-------------------------
    // Write to 0x80C - HALT - print "CPU halt requested"
    else if (address == 0x80C) {
        printf("CPU Halt Requested\n");
        exit(0);
    }
    // Write to 0x800 - Print to ASCII character of number to stdout
    else if (address == 0x800) {
        // printf("PRINT ASCII 0x800 -(%c)\n", value);
        printf("%c", value);
    }
    // 0x804
    else if (address == 0x804) {
        // Console write signed integer
        printf("%d", value);
    } 
    // 0x808
    else if (address == 0x808) {    
        // Console write unsigned integer
        printf("%x", value);
    }
    // 0x820
    else if (address == 0x820) {
        // Dump PC
    }
    // 0x824
    else if (address == 0x824) {
        // Dump Register Banks
    }
    // 0x828
    else if (address == 0x828) {
        // Dump Memory Word
    } 
    // 0x830
    else if (address == 0x830) {
        // Malloc - Implement linked list
        // Request a chunck of memory with the size of the value being stored 
        // Pointer to the allocated memory (starting address) will be stored in R[28]
        // If the memory cannot be allocated R[28] should be set to 0.
    } 
    // 0x834
    else if (address == 0x834) {
        // Free a chunk of memory starting at the value being stored.
        // If the value being stored was not an allocated address raise an illegal operation. 
    }
    // ------------------ Normal Memory Storage ----------------
    else if (address >= 0x0400 && address <= 0x7FF) {
        // Store in memory address
        address = address - 0x0400;
        // memory[address] = value;
        memcpy(&memory[address - 1], &value, num_bytes);
    } else {
        // Throw error 
        // Not implemented - Call to unimplemented Virtual Routine
        not_implemented(pc, registers, instruction);
    }
}

// Load from memory helper
unsigned int read_memory(unsigned char *memory, unsigned char *instructions, unsigned int address, unsigned int num_bytes, unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    // address = address - 0x0400;
    // printf("%u", address);
    if (address < 0x0400) {
        // Throw error, Cannot read from instruction memory 
        // Illegal operation
        // printf("Can't read from instructions\n");
        // print_bits(*instruction, 32);
        // illegal_operation(pc, registers, instruction);
        if (num_bytes == 1) {
            return instructions[address];
        } else if (num_bytes == 2) {
            return combine_two_bytes(instructions[address], instructions[address + 1]);
        } else if (num_bytes == 4) {
            return combine_four_bytes(instructions[address], instructions[address+1], instructions[address+2], instructions[address+3]);
        } else {
            printf("Why are you trying to return %d number of bytes from memory?", num_bytes);
            illegal_operation(pc, registers, instruction);
        }

    } 
    else if (address > 0x8FF) {
        // Exceeds Memory & Virtual Routine bounds
        // Illegal operation
        printf("Exceeds memory bounds\n");
        illegal_operation(pc, registers, instruction);
    }

    // -------------------Virtual Routines-------------------------
    else if (address == 0x0812) {
        // Console Read Character
        char input;
        scanf("%c", &input);
        return input;
    }
    else if (address == 0x0816) {
        // Console read signed integer
        int num;
        scanf("%d", &num);
        return num;
    }   

    // ------------------ Normal Memory Storage ----------------
    else if (address >= 0x0400 && address <= 0x7FF) {
        // Store in memory address
        address = address - 0x0400;
        // memory[address] = value;
        if (num_bytes == 1) {
            return memory[address];
        } else if (num_bytes == 2) {
            return combine_two_bytes(memory[address], memory[address + 1]);
        } else if (num_bytes == 4) {
            return combine_four_bytes(memory[address], memory[address+1], memory[address+2], memory[address+3]);
        } else {
            printf("Why are you trying to return %d number of bytes from memory?", num_bytes);
            illegal_operation(pc, registers, instruction);
        }

    } else {
        // Throw error 
        // Not implemented - Call to unimplemented Virtual Routine
        not_implemented(pc, registers, instruction);
    }
    printf("Should hit this - end of read memory\n");
    return -1;
}

// RiskV Type structs - Use bitfields to optimise memory
// Decode Instructions & Load into struct
// Type R
struct RISK_R {
    unsigned int func7 : 7;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int func3 : 3;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};
struct RISK_R decode_r(unsigned int instruction) {
    struct RISK_R r;
    r.opcode = instruction & 0b1111111;
    r.rd = (instruction >> 7) & 0b11111;
    r.func3 = (instruction >> 12) & 0b111;
    r.rs1 = (instruction >> 15) & 0b11111;
    r.rs2 = (instruction >> 20) & 0b11111;
    r.func7 = (instruction >> 25) & 0b111;
    return r;
}

// Type I
struct RISK_I {
    int imm : 12;
    unsigned int rs1 : 5;
    unsigned int func3 : 3;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};
struct RISK_I decode_i(unsigned int instruction) {
    struct RISK_I i;
    i.opcode = instruction & 0b1111111;
    i.rd = (instruction >> 7) & 0b11111;
    i.func3 = (instruction >> 12) & 0b111;
    i.rs1 = (instruction >> 15) & 0b11111;
    i.imm = (instruction >> 20) & 0b111111111111;
    
    // // Sign extension
    // if (i.imm & (1 < 11)) {
    //     i.imm |= 0xFFFFF000;
    // }
    // printf("I func3 = (%u)\n", i.func3);


    return i;
}

// Type S
struct RISK_S {
    int imm : 12;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int func3 : 3;
    unsigned int opcode : 7;
};
struct RISK_S decode_s(unsigned int instruction) {
    struct RISK_S s;
    s.opcode = instruction & 0b1111111;
    s.func3 = (instruction >> 12) & 0b111;
    s.rs1 = (instruction >> 15) & 0b11111;
    s.rs2 = (instruction >> 20) & 0b11111;
    unsigned int imm4_0 = ((instruction >> 7) & 0b11111);
    unsigned int imm11_5 = ((instruction >> 25) & 0b1111111) ;
    s.imm = (imm11_5 << 5) | imm4_0;

    return s;
}

// Type SB
struct RISK_SB {
    int imm : 13;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int func3 : 3;
    unsigned int opcode : 7;
};
struct RISK_SB decode_sb(unsigned int instruction) {
    struct RISK_SB sb;
    sb.opcode = instruction & 0b1111111;
    sb.func3 = (instruction >> 12) & 0b111;
    sb.rs1 = (instruction >> 15) & 0b11111;
    sb.rs2 = (instruction >> 20) & 0b11111;
    unsigned int imm11 = (instruction >> 7) & 0b1;
    unsigned int imm4_1 = (instruction >> 8) & 0b1111; // before 0x1E
    unsigned int imm10_5 = (instruction >> 25) & 0b111111; // Before 0x7F0
    unsigned int imm12 = (instruction >> 31) & 0b1;
    sb.imm = ((imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | imm4_1 << 1);
    // printf("SB IMM = (%d)\n", sb.imm);
    // print_bits(sb.imm, 13);
    return sb;
}

// Type U
struct RISK_U {
    unsigned int imm : 32;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};
struct RISK_U decode_u(unsigned int instruction) {
    struct RISK_U u;
    u.opcode = instruction & 0b1111111;
    u.rd = (instruction >> 7) & 0b11111;
    u.imm = (instruction) & 0xFFFFF000; // Bits 31-12
    
    return u;
}

// Type UJ
struct RISK_UJ {
    int imm : 21;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};
struct RISK_UJ decode_uj(unsigned int instruction) {
    struct RISK_UJ uj;
    uj.opcode = instruction & 0b1111111;
    uj.rd = (instruction >> 7) & 0b11111;
    unsigned int imm20 = (instruction >> 31) & 0b1;
    unsigned int imm10_1 = (instruction >> 21) & 0b1111111111;
    unsigned int imm11 = (instruction >> 20) & 0b1;
    unsigned int imm19_12 = (instruction >> 12) & 0b11111111;

    int imm = ((imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1));

    uj.imm = imm;
    // printf("unsigned (%u), signed (%d): ", uj.imm, uj.imm);
    // print_bits(uj.imm, 21);
    return uj;
}
// Function to get the imm of UJ



int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Please use 1 command line argument");
        return 0;
    }
    // printf("%s\n", argv[1]);

    // Setup VM architecture 
    // Note - unsigned char = 1 byte, unsigned int = 4 bytes.
    unsigned char instructions[1024] = {0}; // store 1024 bytes of instructions
    unsigned int registers[32] = { 0 }; // store 32 registers each containing 4 bytes
    unsigned char memory[1024] = { 0 };
    // unsigned char virtual_routines[256]; 
    // unsigned char heap_bank[128][64]; // should use linked list!!!
    unsigned int pc = 0;
    bool running = true;
    bool jump = false;


    // Read the instructions from file into the instructions array
    int instructions_length = read_binary_file(argv[1], instructions);
    if (instructions_length == -1) {
        exit(0);
    }
    int i = 0;
    // Run the VM
    while (running) {
        // Fetch instruction
        unsigned int instruction = combine_four_bytes(instructions[pc], instructions[pc+1], instructions[pc+2], instructions[pc+3]);
        
        // Decode instruction
        // printf("PC: %u\n", pc);
        // printf("Instruction: ");print_bits(instruction, 32);
        
        // Extract opcode
        unsigned char opcode = (unsigned char)(instruction & 0b1111111); 
        // printf("Opcode Number: %hhu\n", opcode);
        
        // Execution 
        // Type R
        if (opcode == 0b0110011) {
            // Decode Type R instruction
            struct RISK_R R = decode_r(instruction);
            if (R.func3 == 0b000 && R.func7 == 0b0000000) {
                // 1. add
                store_in_register(registers, R.rd, registers[R.rs1] + registers[R.rs2]);
            } else if (R.func3 == 0b000 && R.func7 == 0b0100000) {
                // 3. sub
                store_in_register(registers, R.rd, registers[R.rs1] - registers[R.rs2]);
            } else if (R.func3 == 0b100 && R.func7 == 0b0000000) {
                // 5. xor
                store_in_register(registers, R.rd, registers[R.rs1] ^ registers[R.rs2]);
            } else if (R.func3 == 0b110 && R.func7 == 0b0000000) {
                // 7. or
                store_in_register(registers, R.rd, registers[R.rs1] | registers[R.rs2]);
            } else if (R.func3 == 0b111 && R.func7 == 0b0000000) {
                // 9. and
                store_in_register(registers, R.rd, registers[R.rs1] & registers[R.rs2]);
            } else if (R.func3 == 0b001 && R.func7 == 0b0000000) {
                // 11. sll
                store_in_register(registers, R.rd, registers[R.rs1] << registers[R.rs2]);
            } else if (R.func3 == 0b101 && R.func7 == 0b0000000) {
                // 12. srl
                store_in_register(registers, R.rd, registers[R.rs1] >> registers[R.rs2]);
            } else if (R.func3 == 0b101 && R.func7 == 0b0100000) {
                // 13. sra
                unsigned int store = registers[R.rs1] >> registers[R.rs2];
                store_in_register(registers, R.rd, (store >> 1) | (store << 31));
            } else if (R.func3 == 0b010 && R.func7 == 0b0000000) {
                // 22. slt
                store_in_register(registers, R.rd, ((int) registers[R.rs1] < (int) registers[R.rs2]) ? 1 : 0);
            } else if (R.func3 == 0b011 && R.func7 == 0b0000000) {
                // 24. sltu
                store_in_register(registers, R.rd, (((unsigned int) registers[R.rs1]) >> ((unsigned int) registers[R.rs2])) ? 1 : 0);
            } else {
                // func3 & func7 not detected -  not implemented
                not_implemented(&pc, registers, &instruction);
            }
            
        } 
        // Type I
        else if (opcode == 0b0010011) {
            // Decode Type I instruction
            struct RISK_I I = decode_i(instruction);
            if (I.func3 == 0b000) {
                // 2. addi
                // registers[I.rd] = registers[I.rs1] + I.imm;
                // Bitfields dont allow memory address, therefore cannot pass a pointer - must pass a copy
                store_in_register(registers, I.rd, registers[I.rs1] + I.imm);
            } else if (I.func3 == 0b100) {
                // 6. xori
                store_in_register(registers, I.rd, registers[I.rs1] ^ I.imm);
            } else if (I.func3 == 0b110) {
                // 8. ori
                store_in_register(registers, I.rd, registers[I.rs1] | I.imm);
            } else if (I.func3 == 0b111) {
                // 10. andi
                store_in_register(registers, I.rd, registers[I.rs1] & I.imm);
            } else if (I.func3 == 0b010) {
                // 23. slti
                store_in_register(registers, I.rd, ((int) registers[I.rs1] < (int) I.imm) ? 1 : 0);
            } else if (I.func3 == 0b011) {
                // 25. sltiu
                store_in_register(registers, I.rd, ((unsigned int) registers[I.rs1] < (unsigned int) I.imm) ? 1 : 0);
            } else {
                // func3 not detected -  not implemented
                not_implemented(&pc, registers, &instruction);
            }
        } 
        else if (opcode == 0b1100111) {
            // Decode Type I instruction
            struct RISK_I I = decode_i(instruction);
            // 33. jalr
            store_in_register(registers, I.rd, pc + 4);
            unsigned int new_pc = registers[I.rs1] + I.imm;
            if (new_pc < 0 || new_pc > 1019) {
                // Throw error - setting pc to a value which exceeds instruction memory
                illegal_operation(&pc, registers, &instruction);
            } else {
                pc = new_pc;
            }
            jump = true;
        }
        else if (opcode == 0b0000011) {
            // Decode Type I instruction
            struct RISK_I I = decode_i(instruction);
            // Memory Access Operations
            if (I.func3 == 0b000) {
                // 14. lb
                unsigned int address = registers[I.rs1] + I.imm;
                unsigned char value_in_memory = read_memory(memory, instructions, address, 1, &pc, registers, &instruction);
                store_in_register(registers, I.rd, (int) value_in_memory);
            } else if (I.func3 == 0b001) {
                // 15. lh
                unsigned int address = registers[I.rs1] + I.imm;
                uint16_t value_in_memory = read_memory(memory, instructions, address, 2, &pc, registers, &instruction);
                store_in_register(registers, I.rd, (int) value_in_memory);
            } else if (I.func3 == 0b010) {
                // 16. lw
                // print_registers(registers);
                // printf("Reg + Imm = (%u)\n", registers[I.rs1] + I.imm);
                unsigned int address = registers[I.rs1] + I.imm;
                unsigned int value_in_memory = read_memory(memory, instructions, address, 4, &pc, registers, &instruction);
                store_in_register(registers, I.rd, (int) value_in_memory);
                // print_registers(registers);
                // printf("We have finished lw\n");
            } else if (I.func3 == 0b100) {
                // 17. lbu
                unsigned int address = registers[I.rs1] + I.imm;
                unsigned char value_in_memory = read_memory(memory, instructions, address, 1, &pc, registers, &instruction);
                store_in_register(registers, I.rd, value_in_memory);
            } else if (I.func3 == 0b101) {
                // 18. lhu
                unsigned int address = registers[I.rs1] + I.imm;
                uint16_t value_in_memory = read_memory(memory, instructions, address, 2, &pc, registers, &instruction);
                store_in_register(registers, I.rd, value_in_memory);
            } else {
                // func3 not detected -  not implemented
                // printf("No func3 detected ?? Func 3 = (%u)\n", I.func3);
                not_implemented(&pc, registers, &instruction);
            }
        }
        // Type S
        else if (opcode == 0b0100011) {
            // Decode Type S instruction
            struct RISK_S S = decode_s(instruction);
            
            if (S.func3 == 0b000) {
                // 19. sb
                // memory[registers[S.rs1] + S.imm] = registers[S.rs2];
                // printf("Store in memory. memory addr-(%d). Value-(%d)\n", registers[S.rs1] + S.imm, registers[S.rs2]);
                // printf("Register value=(%d). S IMM value=(%d)\n", registers[S.rs1], S.imm);
                store_in_memory(memory, registers[S.rs1] + S.imm, registers[S.rs2], 1, &pc, registers, &instruction);
            } else if (S.func3 == 0b001) {
                // 20. sh
                store_in_memory(memory, registers[S.rs1] + S.imm, registers[S.rs2], 2, &pc, registers, &instruction);
            } else if (S.func3 == 0b010) {
                // 21. sw
                store_in_memory(memory, registers[S.rs1] + S.imm, registers[S.rs2], 4, &pc, registers, &instruction);
            } else {
                // func3 not detected -  not implemented
                not_implemented(&pc, registers, &instruction);
            }

        }
        // Type SB
        else if (opcode == 0b1100011) {
            // Decode Type SB instruction
            struct RISK_SB SB = decode_sb(instruction);

            if (SB.func3 == 0b000) {
                // 26. beq
                if (registers[SB.rs1] == registers[SB.rs2]) {
                    unsigned int new_pc = pc + SB.imm;
                    if (valid_pc(&new_pc)) {
                        pc = new_pc;
                        jump = true;
                    } else {
                        illegal_operation(&pc, registers, &instruction);
                    }
                }
            } else if (SB.func3 == 0b001) {
                // 27. bne
                if (registers[SB.rs1] != registers[SB.rs2]) {
                    unsigned int new_pc = pc + SB.imm;
                    if (valid_pc(&new_pc)) {
                        pc = new_pc;
                        jump = true;
                    } else {
                        illegal_operation(&pc, registers, &instruction);
                    }
                }
            } else if (SB.func3 == 0b100) {
                // 28. blt
                if (registers[SB.rs1] < registers[SB.rs2]) {
                    unsigned int new_pc = pc + SB.imm;
                    if (valid_pc(&new_pc)) {
                        pc = new_pc;
                        jump = true;
                    } else {
                        illegal_operation(&pc, registers, &instruction);
                    }
                }
            } else if (SB.func3 == 0b110) {
                // 29. bltu
                if (((unsigned int) registers[SB.rs1]) < ((unsigned int) registers[SB.rs2])) {
                    unsigned int new_pc = pc + SB.imm;
                    if (valid_pc(&new_pc)) {
                        pc = new_pc;
                        jump = true;
                    } else {
                        illegal_operation(&pc, registers, &instruction);
                    }
                }
            } else if (SB.func3 == 0b101) {
                // 30. bge
                if (registers[SB.rs1] >= registers[SB.rs2]) {
                    unsigned int new_pc = pc + SB.imm;
                    if (valid_pc(&new_pc)) {
                        pc = new_pc;
                        jump = true;
                    } else {
                        illegal_operation(&pc, registers, &instruction);
                    }
                }
            } else if (SB.func3 == 0b111) {
                // 31. bgeu
                if (((unsigned int) registers[SB.rs1]) >= ((unsigned int) registers[SB.rs2])) {
                    unsigned int new_pc = pc + SB.imm;
                    if (valid_pc(&new_pc)) {
                        pc = new_pc;
                        jump = true;
                    } else {
                        illegal_operation(&pc, registers, &instruction);
                    }
                }
            } else {
                // func3 not detected -  not implemented
                not_implemented(&pc, registers, &instruction);
            }
        }
        // Type U
        else if (opcode == 0b0110111) {
            // Decode Type U instruction
            struct RISK_U U = decode_u(instruction);

            // 4. lui
            store_in_register(registers, U.rd, U.imm);
        }
        // Type UJ
        else if (opcode == 0b1101111) {
            // Decode Type UJ instruction
            struct RISK_UJ UJ = decode_uj(instruction);
            // 32. jal
            store_in_register(registers, UJ.rd, pc + 4);
            unsigned int new_pc = pc + UJ.imm;
            if (valid_pc(&new_pc)) {
                pc = pc + UJ.imm;
            } else {
                illegal_operation(&pc, registers, &instruction);
            }
            jump = true;
            // print_registers(registers);
        }
        // Instruction Not Implemented
        else {
            // Opcode not detected -  not implemented
            not_implemented(&pc, registers, &instruction);
        }
        // print_registers(registers);
        // print_memory(memory);
        // Increment Program Counter

        // If jumped to another address don't increment PC, else increment
        if (jump == true) {
            jump = false;
        } else {
            pc += 4;
        }
        // if (pc > 36 || i > 11) {
        //     break;
        // }
        i++;
    }







    // print_buffer(instructions, length);
    // printf("\nChar: (%u)\n", instructions[0]);
    // printf("\nChar: (%u)\n", instructions[1]);
    // printf("\nChar: (%u)\n", instructions[2]);
    // printf("\nChar: (%u)\n", instructions[3]);
    // printf("\nChar: (%u)\n", instructions[4]);
    // printf("\nChar: (%u)\n", instructions[5]);
    // unsigned int instruction1 = combine_bytes(instructions[4], instructions[5], instructions[6], instructions[7]);
    
    // printf("---(%u)---\n", instruction1);
    // printf("Instruction: 0x%08x\n", instruction1);
    // print_bits(instruction1);

    return 0;
}



