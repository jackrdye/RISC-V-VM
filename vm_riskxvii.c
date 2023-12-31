#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
// #include <stdint.h>

#include "heap_bank.h"
#include "riscv_types.h"

// Helper functions
// void read_binary_file(const char* filename, unsigned char *instructions, unsigned char *memory) {
//     FILE* file;
//     // short size;
//     // short size2;

//     file = fopen(filename, "rb"); // open the binary file for reading
//     if (file == NULL) {
//         // perror("Error opening file");
//         exit(1);
//         // return -1;
//     }

//     // read up to MAX_BYTES bytes from the file into the buffer
//     // size = 
//     fread(instructions, 1, 1024, file);
//     fread(memory, 1, 1024, file);
//     if (ferror(file)) {
//         // perror("Error reading file");
//         exit(1);
//         // size = -1;
//     }
//     // if (size != 1024) {

//     // }

//     // size2 = 
    
//     fclose(file); // close the file
//     // return (size == -1 || size2 == -1)? 1 : 2048; // return the number of bytes read
// }


// void print_buffer(unsigned char* buffer, int size) {
//     for (int i = 0; i < size / 4; i++) {
//         printf("%02x ", buffer[i]); // print each byte as a two-digit hexadecimal value
//     }
//     printf("\n");
// }


// void print_bits(unsigned int instruction, unsigned int length) {
//     for (int i = length - 1; i >= 0; i--) {
//         printf("%c", (instruction & (1 << i)) ? '1' : '0');
//     }
//     printf("\n");
// }

// void print_registers(unsigned int registers[32]) {
//     for (int i = 0; i < 32; i++) {
//         printf("Register %d: %u\n", i, registers[i]);
//     }
// }

// void print_register(unsigned int registers[32], unsigned int i) {
//     printf("Register %d: %u\n", i, registers[i]);
// }

// void print_memory(unsigned char memory[1024]) {
//     for (int i = 0; i < 1024; i += 4) {
//         printf("Byte: %d -- %02x %02x %02x %02x\n", i, memory[i], memory[i+1], memory[i+2], memory[i+3]);
//     }
// }


unsigned int combine_four_bytes(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4) {
    unsigned int instruction = 0;
    instruction |= (unsigned int)b1;
    instruction |= (unsigned int)b2 << 8;
    instruction |= (unsigned int)b3 << 16;
    instruction |= (unsigned int)b4 << 24;
    return instruction;
}
unsigned short combine_two_bytes(unsigned char b1, unsigned char b2) {
    unsigned short instruction = 0;
    instruction |= (unsigned int)b1;
    instruction |= (unsigned int)b2 << 8;
    return instruction;
}

// Instruction not Implemented Helper
void not_implemented(unsigned short *pc, unsigned int *registers, unsigned int *instruction) {
    printf("Instruction Not Implemented: 0x%x\n", *instruction);
    register_dump(pc, registers);
    exit(1);
}


// Ensure PC not out of bounds
bool valid_pc(unsigned short *pc) {
    if (*pc < 0 || *pc >1020) {
        return false;
    } else {
        return true;
    }
}

// Store in register helper
void store_in_register(unsigned int *registers, unsigned char store_in, int set_to) {
    if (store_in == 0 || store_in > 31) {
        // Cannot set zero register - Ignore any write
    // } else if (store_in > 31) {
        // Only 31 registers should be prevented anyway since rd, rs1, rs2 only 5 bits
    } else {
        // Set register 
        registers[store_in] = set_to;
    }
}
// Read Regsiter - Register always specified in instruction not by program thus cannot exceed 5 bits. (Do not need helper)

// Load from memory helper
unsigned int read_memory(unsigned char *memory, unsigned char *instructions, Node *head, unsigned int address, unsigned int num_bytes, unsigned short *pc, unsigned int *registers, unsigned int *instruction) {
    // address = address - 0x0400;
    if (address >= 0x000 && address + (num_bytes-1)< 0x0400) {
        if (num_bytes == 1) {
            return instructions[address];
        } else if (num_bytes == 2) {
            return combine_two_bytes(instructions[address], instructions[address+1]);
        } else if (num_bytes == 4) {
            return combine_four_bytes(instructions[address], instructions[address+1], instructions[address+2], instructions[address+3]);
        } else {
            illegal_operation(pc, registers, instruction);
        }
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
    else if (address >= 0x0400 && address+(num_bytes-1) <= 0x7FF) {
        // Store in memory address
        address = address - 0x0400;
        if (num_bytes == 1) {
            return memory[address];
        } else if (num_bytes == 2) {
            return combine_two_bytes(memory[address], memory[address+1]);
        } else if (num_bytes == 4) {
            return combine_four_bytes(memory[address], memory[address+1], memory[address+2], memory[address+3]);
        } else {
            // printf("Why are you trying to return %d number of bytes from memory?", num_bytes);
            illegal_operation(pc, registers, instruction);
        }
    // ------------------------ Heap Banks --------------------------
    } else if (address >= 0xB700 && address + (num_bytes-1) < 0xD700) {
        if (num_bytes == 1) {
            return read_byte_from_heap(head, address, pc, registers, instruction);
        } else if (num_bytes == 2) {
            return combine_two_bytes(
                read_byte_from_heap(head, address, pc, registers, instruction), 
                read_byte_from_heap(head, address + 1, pc, registers, instruction)
                );
        } else if (num_bytes == 4) {
            return combine_four_bytes(read_byte_from_heap(head, address, pc, registers, instruction), 
                read_byte_from_heap(head, address + 1, pc, registers, instruction), 
                read_byte_from_heap(head, address + 2, pc, registers, instruction), 
                read_byte_from_heap(head, address + 3, pc, registers, instruction) 
                );
        } else {
            illegal_operation(pc, registers, instruction);
        }
    } else {
        // Throw error 
        // Not implemented - Call to unimplemented Virtual Routine
        not_implemented(pc, registers, instruction);
    }
    return -1; // Shouldn't reach here
}

// Store in memory helper
void store_in_memory(unsigned char *memory, unsigned char *instructions, Node *head, unsigned int address, unsigned int value, unsigned int num_bytes, unsigned short *pc, unsigned int *registers, unsigned int *instruction) {
    if (address < 0x0400) {
        // Throw error, Cannot overwrite instruction memory 
        illegal_operation(pc, registers, instruction);
    } 
    else if (address > 0x8FF && address < 0xB700) {
        // Exceeds Memory & Virtual Routine bounds
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
        printf("%x", *pc);
    }
    // 0x824
    else if (address == 0x824) {
        // Dump Register Banks
        register_dump(pc, registers);
    }
    // 0x828
    else if (address == 0x828) {
        // Dump Memory Word
        printf("%x", read_memory(memory, instructions, head, value, 4, pc, registers, instruction));
    } 
    // 0x830
    else if (address == 0x830) {
        // Malloc - Implement linked list
        // Request a chunck of memory with the size of the value being stored 
        unsigned int allocated_address = allocate(head, &value);
        // Pointer to the allocated memory (starting address) will be stored in R[28]
            // If the memory cannot be allocated R[28] should be set to 0.
        registers[28] = ((allocated_address == 0) ? 0 : allocated_address);
        // printf("Bytes allocated at, Register 28: (%x)\n", registers[28]);
    } 
    // 0x834
    else if (address == 0x834) {
        // Free a chunk of memory starting at the value being stored.
        // If the value being stored was not an allocated address raise an illegal operation. 
        free_heap_bank(head, value, pc, registers, instruction); 
    }
    // ------------------ Normal Memory Storage ----------------
    else if (address >= 0x0400 && address <= 0x7FF) {
        // Store in memory address
        address = address - 0x0400;
        // memory[address] = value;
        memcpy(&memory[address], &value, num_bytes);
    } 

    // ---------------------- Heap Banks ----------------------
    else if (address >= 0xB700) {
        
        store_byte_in_heap(head, address, (unsigned char) value, pc, registers, instruction);
        
        if (num_bytes >= 2) {
            store_byte_in_heap(head, address+1, (unsigned char) (value >> 8), pc, registers, instruction);
        } 
        if (num_bytes >= 4) {
            store_byte_in_heap(head, address+2, (unsigned char) (value >> 16), pc, registers, instruction);
            store_byte_in_heap(head, address+3, (unsigned char) (value >> 24), pc, registers, instruction);
        }
    }
    else {
        // Throw error 
        not_implemented(pc, registers, instruction); // Call to unimplemented Virtual Routine
    }
}



int main(int argc, char *argv[]) {
    if (argc != 2) {
        // printf("Please use 1 command line argument");
        exit(1);
    }

    // Setup VM architecture 
    // Note - unsigned char = 1 byte, unsigned short = 2 bytes, unsigned int = 4 bytes.
    unsigned char instructions[1024] = {0}; // store 1024 bytes of instructions
    unsigned int registers[32] = { 0 }; // store 32 registers each containing 4 bytes
    unsigned char memory[1024] = { 0 };
    unsigned short pc = 0;

    // Create Heap Bank
    Node head_node;
    Node *head = create_heap_bank(&head_node);

    
    // Read the instructions from file into the instructions array
    // read_binary_file(argv[1], instructions, memory);
    FILE* file;

    file = fopen(argv[1], "rb"); // open the binary file for reading
    if (file == NULL) {
        exit(1);
    }
    fread(instructions, 1, 1024, file);
    fread(memory, 1, 1024, file);
    if (ferror(file)) {
        exit(1);
    }
    fclose(file);


    // Run the VM
    while (1) {
        if (!valid_pc(&pc)) {
            // printf("PC is out of bounds\n");
            exit(1);
        }
        bool jump = false;
        // Fetch instruction
        unsigned int instruction = combine_four_bytes(instructions[pc], instructions[pc+1], instructions[pc+2], instructions[pc+3]);
        
        // Decode instruction
        // printf("PC: %u. ", pc);
        // printf("Register 14 = (%u), Register 15 = (%u). ", registers[14], registers[15]);
        // printf("Instruction: ");print_bits(instruction, 32);
        
        // Extract opcode
        unsigned char opcode = (instruction & 0b1111111); 
        // printf("Opcode Number: %hhu\n", opcode);
        
        // Execution 
        // Type R
        if (opcode == 0b0110011) {
            // Decode Type R instruction
            const struct RISK_R R = decode_r(instruction);
            if (R.func3 == 0b000 && R.func7 == 0b0000000) {
                // 1. add
                if (registers[R.rs1] > (__UINT32_MAX__ - registers[R.rs2])) {
                    // Integer Overflow
                    store_in_register(registers, R.rd, registers[R.rs1] + registers[R.rs2]);
                    // printf("Integer overflow detected in 'add'\n");
                } else {
                    store_in_register(registers, R.rd, registers[R.rs1] + registers[R.rs2]);
                }
            } else if (R.func3 == 0b000 && R.func7 == 0b0100000) {
                // 3. sub
                if (registers[R.rs1] < registers[R.rs2]) {
                    // Integer Underflow
                    store_in_register(registers, R.rd, registers[R.rs1] - registers[R.rs2]);
                    // printf("Integer underflow detected in 'sub'\n");
                } else {
                    store_in_register(registers, R.rd, registers[R.rs1] - registers[R.rs2]);
                }
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
            const struct RISK_I I = decode_i(instruction);
            if (I.func3 == 0b000) {
                // 2. addi
                // Bitfields dont allow memory address, therefore cannot pass a pointer - must pass a copy
                if (I.imm > 0 && registers[I.rs1] > (__UINT32_MAX__ - I.imm)) {
                    // Integer Overflow
                    store_in_register(registers, I.rd, registers[I.rs1] + I.imm);
                    // printf("Integer overflow detected in 'addi'\n");
                } else if (I.imm < 0 && registers[I.rs1] < I.imm) {
                    store_in_register(registers, I.rd, registers[I.rs1] + I.imm);
                    // printf("Integer underflow detected in 'addi'\n");
                } else {
                    store_in_register(registers, I.rd, registers[I.rs1] + I.imm);
                }
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
            const struct RISK_I I = decode_i(instruction);
            // 33. jalr
            store_in_register(registers, I.rd, pc + 4);
            unsigned short new_pc = registers[I.rs1] + I.imm;
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
            const struct RISK_I I = decode_i(instruction);
            // Memory Access Operations
            if (I.func3 == 0b000) {
                // 14. lb
                unsigned int address = registers[I.rs1] + I.imm;
                unsigned char value_in_memory = read_memory(memory, instructions, head, address, 1, &pc, registers, &instruction);
                store_in_register(registers, I.rd, (int) value_in_memory);
            } else if (I.func3 == 0b001) {
                // 15. lh
                unsigned int address = registers[I.rs1] + I.imm;
                unsigned short value_in_memory = read_memory(memory, instructions, head, address, 2, &pc, registers, &instruction);
                store_in_register(registers, I.rd, (int) value_in_memory);
            } else if (I.func3 == 0b010) {
                // 16. lw
                unsigned int address = registers[I.rs1] + I.imm;
                unsigned int value_in_memory = read_memory(memory, instructions, head, address, 4, &pc, registers, &instruction);
                store_in_register(registers, I.rd, (int) value_in_memory);
            } else if (I.func3 == 0b100) {
                // 17. lbu
                unsigned int address = registers[I.rs1] + I.imm;
                store_in_register(registers, I.rd, read_memory(memory, instructions, head, address, 1, &pc, registers, &instruction));
            } else if (I.func3 == 0b101) {
                // 18. lhu
                unsigned int address = registers[I.rs1] + I.imm;
                store_in_register(registers, I.rd, read_memory(memory, instructions, head, address, 2, &pc, registers, &instruction));
            } else {
                // func3 not detected -  not implemented
                // printf("No func3 detected ?? Func 3 = (%u)\n", I.func3);
                not_implemented(&pc, registers, &instruction);
            }
        }
        // Type S
        else if (opcode == 0b0100011) {
            // Decode Type S instruction
            const struct RISK_S S = decode_s(instruction);
            
            if (S.func3 == 0b000) {
                // 19. sb
                store_in_memory(memory, instructions, head, registers[S.rs1] + S.imm, registers[S.rs2], 1, &pc, registers, &instruction);
            } else if (S.func3 == 0b001) {
                // 20. sh
                store_in_memory(memory, instructions, head, registers[S.rs1] + S.imm, registers[S.rs2], 2, &pc, registers, &instruction);
            } else if (S.func3 == 0b010) {
                // 21. sw
                store_in_memory(memory, instructions, head, registers[S.rs1] + S.imm, registers[S.rs2], 4, &pc, registers, &instruction);
            } else {
                // func3 not detected -  not implemented
                not_implemented(&pc, registers, &instruction);
            }

        }
        // Type SB
        else if (opcode == 0b1100011) {
            // Decode Type SB instruction
            const struct RISK_SB SB = decode_sb(instruction);

            if (SB.func3 == 0b000) {
                // 26. beq
                if (registers[SB.rs1] == registers[SB.rs2]) {
                    unsigned short new_pc = pc + SB.imm;
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
                    unsigned short new_pc = pc + SB.imm;
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
                    unsigned short new_pc = pc + SB.imm;
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
                    unsigned short new_pc = pc + SB.imm;
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
                    unsigned short new_pc = pc + SB.imm;
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
                    unsigned short new_pc = pc + SB.imm;
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
            const struct RISK_U U = decode_u(instruction);

            // 4. lui
            store_in_register(registers, U.rd, U.imm);
        }
        // Type UJ
        else if (opcode == 0b1101111) {
            // Decode Type UJ instruction
            struct RISK_UJ UJ = decode_uj(instruction);
            // 32. jal
            store_in_register(registers, UJ.rd, pc + 4);
            unsigned short new_pc = pc + UJ.imm;
            if (valid_pc(&new_pc)) {
                pc = pc + UJ.imm;
            } else {
                illegal_operation(&pc, registers, &instruction);
            }
            jump = true;
        }
        // Instruction Not Implemented
        else {
            not_implemented(&pc, registers, &instruction); // Opcode not detected -  not implemented
        }

        // If jumped to another address don't increment PC, else increment
        if (jump == true) {
            jump = false;
        } else {
            pc += 4;
        }
    }
}