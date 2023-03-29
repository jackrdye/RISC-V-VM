/*RISK_TYPES.h*/

#ifndef RISK_TYPES_H
#define RISK_TYPES_H

// Risk5 Type structs - Use bitfields to optimise memory
struct Risk_R {
    unsigned int func7 : 7;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int func3 : 3;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};

struct RISK_I {
    unsigned int imm : 12;
    unsigned int rs1 : 5;
    unsigned int funct3 : 3;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};
struct RISK_S {
    unsigned int imm : 12;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int funct3 : 3;
    unsigned int opcode : 7;
};

struct RISK_SB {
    unsigned int imm : 12;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int funct3 : 3;
    unsigned int opcode : 7;
};

struct RISK_U {
    unsigned int imm : 20;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};

struct RISK_UJ {
    unsigned int imm : 20;
    unsigned int rd : 5;
    unsigned int opcode : 7;
};
#endif /* RISK_TYPES_H */