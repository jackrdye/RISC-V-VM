/*RISK_TYPES.h*/

#ifndef RISK_TYPES_H
#define RISK_TYPES_H

// RiskV Type structs - Use bitfields to optimise memory
// Decode Instructions & Load into struct
// Type R
struct RISK_R {
    unsigned char func7 : 7;
    unsigned char rs2 : 5;
    unsigned char rs1 : 5;
    unsigned char func3 : 3;
    unsigned char rd : 5;
    unsigned char opcode : 7;
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
    return uj;
}

#endif /* RISK_TYPES_H */