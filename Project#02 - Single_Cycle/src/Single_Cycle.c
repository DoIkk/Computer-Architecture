#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 10000000 

int I_count, J_count, R_count = 0; //type count

// 명령어
typedef struct {
    int opcode;
    int rs;
    int rt;
    int rd;
    int shamt;
    int funct;
    int constant;
    int address;
} inst;

// control unit
typedef struct {
    int RegDst;
    int RegWrite;
    int ALUSrc;
    int ALUop;
    int MemRead;
    int MemWrite;
    int MemtoReg;
    int PCSrc;
} control;

control CU = { 0 }; //구조체 0 으로 초기화

int Reg[32]; // 모든 레지스터를 0으로 초기화
int LO;
int HI;

unsigned int memory[MEMORY_SIZE];
int pc = 0; // 프로그램 카운터
char* func; // func 표시를 위한 변수

char type; // 전역 변수로 type 선언
int ALUresult = 0; //ALUresult 값

//레지스터 이름설정
const char* register_names[32] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$s8", "$ra"
};

//pc 값 계산 ALU
int Adder(int pc, int input) {
    return pc + (input << 2); // 4바이트 단위로 증가시키기
}

//branch pc값 ALU
int branchAdder(int pc, int constant) {
    return pc + 4 + (constant << 2); //branch address 계산
}

//jump pc값 계산 ALU
int jumpAdder(int pc, int address) {
    return ((pc + 4) & 0xf0000000 | (address << 2)); //jump address 계산
}

void init_Reg(int* Reg) {
    for (int i = 0; i < 32; i++) {
        Reg[i] = 0; // 모든 레지스터를 0으로 초기화
        if (i == 29) Reg[i] = 0x1000000; // 스택 포인터($sp)를 0x1000000으로 초기화
        else if (i == 31) Reg[i] = 0xFFFFFFFF; // 반환 주소($ra)를 0xFFFFFFFF로 초기화
    }
}

// 빅 엔디안 변환 함수
unsigned int convertToBigEndian(unsigned int value) {
    unsigned char* bytes = (unsigned char*)&value;
    return ((unsigned int)bytes[3]) | ((unsigned int)bytes[2] << 8) |
        ((unsigned int)bytes[1] << 16) | ((unsigned int)bytes[0] << 24);
}

// 비트 필드에 값을 설정하는 함수
void setField(int* field, int value) {
    *field = value;
}

// 명령어를 가져오는 함수
void fetch(unsigned int* IR) {
    *IR = memory[pc / 4];
    printf("    [Instruction Fetch] 0x%08x (pc=0x%07x)\n", *IR, pc);
}

// 부호 확장 함수
int SignExtend(short value) {
    return (int)value;
}

//control signal
void CU_signal(control* CU, inst* decodedInst, char type) {
    CU->RegDst = 0;
    CU->RegWrite = 0;
    CU->ALUSrc = 0;
    CU->ALUop = 0;
    CU->MemRead = 0;
    CU->MemWrite = 0;
    CU->MemtoReg = 0;
    CU->PCSrc = 0;

    if (type == 'R') {
        CU->RegDst = 1;
        CU->RegWrite = 1;
        CU->ALUSrc = 0;
        CU->ALUop = 2; // R형 명령어 ALUOp 설정
    }
    else if (type == 'I') {
        CU->RegWrite = 1;
        CU->ALUSrc = 1;
        switch (decodedInst->opcode) {
        case 0x23: // lw
        case 0x20: // lb
        case 0x24: // lbu
        case 0x21: // lh
        case 0x25: // lhu
            CU->MemRead = 1;
            CU->MemtoReg = 1;
            CU->ALUop = 0; // ADD
            break;
        case 0x2b: // sw
            CU->MemWrite = 1;
            CU->ALUop = 0; // ADD
            break;
        case 0x04: // beq
            CU->PCSrc = 1;
            CU->ALUop = 1; // SUB
            break;
        case 0x08: // addi
        case 0x09: // addiu
            CU->ALUop = 0; // ADD
            break;
        case 0x0c: // andi
            CU->ALUop = 3; // AND
            break;
        case 0x0d: // ori
            CU->ALUop = 4; // OR
            break;
        case 0x0a: // slti
        case 0x0b: // sltiu
            CU->ALUop = 5; // SLT
            break;
        }
    }
    else if (type == 'J') {
        // J형 명령어는 ALU 연산을 사용하지 않으므로 설정하지 않음
    }
}

// 명령어 디코드 함수
void decode(unsigned int instruction, inst* decodedInst) {
    // opcode: 상위 6비트 추출
    setField(&decodedInst->opcode, (instruction >> 26) & 0x3F);

    if ((instruction >> 26) == 0x00) { // R 타입 명령어
        setField(&decodedInst->rs, (instruction >> 21) & 0x1F);
        setField(&decodedInst->rt, (instruction >> 16) & 0x1F);
        setField(&decodedInst->rd, (instruction >> 11) & 0x1F);
        setField(&decodedInst->shamt, (instruction >> 6) & 0x1F);
        setField(&decodedInst->funct, instruction & 0x3F);
        type = 'R'; // 전역 변수 type 설정
        R_count++;
    }
    else if ((instruction >> 26) == 0x02 || (instruction >> 26) == 0x03) { // J 타입 명령어
        setField(&decodedInst->address, instruction & 0x3FFFFFF);
        type = 'J'; // 전역 변수 type 설정
        J_count++;
    }
    else { // I 타입 명령어
        setField(&decodedInst->rs, (instruction >> 21) & 0x1F);
        setField(&decodedInst->rt, (instruction >> 16) & 0x1F);
        setField(&decodedInst->constant, SignExtend(instruction & 0xFFFF));
        type = 'I'; // 전역 변수 type 설정
        I_count++;
    }

    CU_signal(&CU, decodedInst, type); // control unit 값 설정
}

// 명령어 분류 및 실행
int execute(inst* decodedInst) {
    int validALUresult = 0; // ALUresult 유효성 확인 변수

    if (type == 'R') {
        switch (decodedInst->funct) {
        case 0x20: // ADD
            func = "add";
            ALUresult = Reg[decodedInst->rs] + Reg[decodedInst->rt];
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x21: // ADDU
            func = "addu";
            ALUresult = (unsigned int)(Reg[decodedInst->rs] + Reg[decodedInst->rt]);
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x24: // AND
            func = "and";
            ALUresult = Reg[decodedInst->rs] & Reg[decodedInst->rt];
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x08: // JR
            func = "jr";
            pc = Reg[decodedInst->rs];
            return ALUresult; // 조기 반환하여 pc가 다시 변경되지 않도록 함
        case 0x27: // NOR
            func = "nor";
            ALUresult = ~(Reg[decodedInst->rs] | Reg[decodedInst->rt]);
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x25: // OR
            func = "or";
            ALUresult = Reg[decodedInst->rs] | Reg[decodedInst->rt];
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x2a: // SLT
            func = "slt";
            ALUresult = (Reg[decodedInst->rs] < Reg[decodedInst->rt]) ? 1 : 0;
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x2b: // SLTU
            func = "sltu";
            ALUresult = (unsigned int)(Reg[decodedInst->rs] < Reg[decodedInst->rt]) ? 1 : 0;
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x00: // SLL
            func = "sll";
            ALUresult = (Reg[decodedInst->rt] << decodedInst->shamt);
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x02: // SRL
            func = "srl";
            ALUresult = (Reg[decodedInst->rt] >> decodedInst->shamt);
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x22: // SUB
            func = "sub";
            ALUresult = (int)(Reg[decodedInst->rs] - Reg[decodedInst->rt]);
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x23: // SUBU
            func = "subu";
            ALUresult = (unsigned int)(Reg[decodedInst->rs] - Reg[decodedInst->rt]);
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        case 0x1a: // DIV
            func = "div";
            LO = (int)(Reg[decodedInst->rs] / Reg[decodedInst->rt]);
            HI = (int)(Reg[decodedInst->rs] % Reg[decodedInst->rt]);
            validALUresult = 0;
            break;
        case 0x1b: // DIVU
            func = "diviu";
            LO = (unsigned int)(Reg[decodedInst->rs] / Reg[decodedInst->rt]);
            HI = (unsigned int)(Reg[decodedInst->rs] % Reg[decodedInst->rt]);
            validALUresult = 0;
            break;
        case 0x19: // MUL
            func = "mul";
            ALUresult = Reg[decodedInst->rs] * Reg[decodedInst->rt];
            Reg[decodedInst->rd] = ALUresult;
            validALUresult = 1;
            break;
        }
    }
    else if (type == 'I') {
        switch (decodedInst->opcode) {
        case 0x08: // ADDI
            func = "addi";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            Reg[decodedInst->rt] = ALUresult;
            validALUresult = 1;
            break;
        case 0x09: // ADDI unsigned
            func = "addiu";
            ALUresult = (unsigned int)(Reg[decodedInst->rs] + SignExtend(decodedInst->constant));
            Reg[decodedInst->rt] = ALUresult;
            validALUresult = 1;
            break;
        case 0x0c: // ANDI
            func = "andi";
            ALUresult = Reg[decodedInst->rs] & (decodedInst->constant & 0xFFFF); // 16비트 0으로 채움
            Reg[decodedInst->rt] = ALUresult;
            validALUresult = 1;
            break;
        case 0x04: // beq
            func = "beq";
            if (Reg[decodedInst->rs] == Reg[decodedInst->rt]) {
                pc = branchAdder(pc, decodedInst->constant);
            }
            else {
                pc = Adder(pc, 1);
            }
            validALUresult = 0;
            break;
        case 0x05: // bne
            func = "bne";
            if (Reg[decodedInst->rs] != Reg[decodedInst->rt]) {
                pc = branchAdder(pc, decodedInst->constant);
            }
            else {
                pc = Adder(pc, 1);
            }
            validALUresult = 0;
            break;
        case 0x24: // lbu
            func = "lbu";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            Reg[decodedInst->rt] = memory[ALUresult / 4] & 0xFF;
            validALUresult = 1;
            break;
        case 0x25: // lhu
            func = "lhu";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            Reg[decodedInst->rt] = memory[ALUresult / 4] & 0xFFFF;
            validALUresult = 1;
            break;
        case 0x30: // ll
            func = "ll";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            Reg[decodedInst->rt] = memory[ALUresult / 4];
            validALUresult = 1;
            break;
        case 0x0f: // lui
            func = "lui";
            Reg[decodedInst->rt] = decodedInst->constant << 16;
            validALUresult = 1;
            break;
        case 0x23: // lw
            func = "lw";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            Reg[decodedInst->rt] = memory[ALUresult / 4];
            validALUresult = 1;
            break;
        case 0x0d: // ori
            func = "ori";
            ALUresult = Reg[decodedInst->rs] | (decodedInst->constant & 0xFFFF);
            Reg[decodedInst->rt] = ALUresult;
            validALUresult = 1;
            break;
        case 0x0a: // slti
            func = "slti";
            ALUresult = (int)((Reg[decodedInst->rs] < SignExtend(decodedInst->constant)) ? 1 : 0);
            Reg[decodedInst->rt] = ALUresult;
            validALUresult = 1;
            break;
        case 0x0b: // sltiu
            func = "sltiu";
            ALUresult = (unsigned int)((Reg[decodedInst->rs] < SignExtend(decodedInst->constant)) ? 1 : 0);
            Reg[decodedInst->rt] = ALUresult;
            validALUresult = 1;
            break;
        case 0x28: // sb
            func = "sb";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            memory[ALUresult / 4] = Reg[decodedInst->rt] & 0xFF;
            validALUresult = 1;
            break;
        case 0x38: { // sc
            func = "sc";
            int addr = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            if (1) { // Replace with actual condition check
                memory[addr / 4] = Reg[decodedInst->rt];
                Reg[decodedInst->rt] = 1;  // Store successful
            }
            else {
                Reg[decodedInst->rt] = 0;  // Store failed
            }
            validALUresult = 1;
            break;
        }
        case 0x29: // sh
            func = "sh";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            memory[ALUresult / 4] = Reg[decodedInst->rt] & 0xFFFF;
            validALUresult = 1;
            break;
        case 0x2b: // sw
            func = "sw";
            ALUresult = Reg[decodedInst->rs] + SignExtend(decodedInst->constant);
            memory[ALUresult / 4] = Reg[decodedInst->rt];
            validALUresult = 1;
            break;
        }
    }
    else if (type == 'J') {
        switch (decodedInst->opcode) {
        case 0x02: // j
            func = "j";
            pc = (pc & 0xF0000000) | (decodedInst->address << 2);
            return ALUresult; // 조기 반환하여 pc가 다시 변경되지 않도록 함
        case 0x03: // jal
            func = "jal";
            Reg[31] = pc + 8; // $ra 레지스터 설정
            pc = (pc & 0xF0000000) | (decodedInst->address << 2);
            return ALUresult; // 조기 반환하여 pc가 다시 변경되지 않도록 함
        }
    }

    // PC 업데이트
    if (type != 'J' && decodedInst->opcode != 0x04 && decodedInst->opcode != 0x05 && (type != 'R' || decodedInst->funct != 0x08)) {
        pc = Adder(pc, 1);
    }


    return ALUresult;
}

//execute print
void printExecute(int validALUresult) {
    if (validALUresult) {
        printf("    [Execute] ALU result: %08x\n", ALUresult);
    }
    else {
        printf("    [Execute] Pass\n");
    }
}

///memory access
void memoryAccess(inst* decodedInst, control* CU) {
    printf("    [Memory Access] ");
    unsigned int address;
    unsigned int value;

    switch (decodedInst->opcode) {
    case 0x23: // lw
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = memory[address / 4];
        printf("Load, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    case 0x20: // lb
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = memory[address / 4] & 0xFF; // 8비트만 가져옴
        printf("Load, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    case 0x24: // lbu
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = memory[address / 4] & 0xFF; // 8비트만 가져옴
        printf("Load, Address: 0x%08x, Value: %02xx\n", address, value);
        break;
    case 0x21: // lh
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = memory[address / 4] & 0xFFFF; // 16비트만 가져옴
        printf("Load, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    case 0x25: // lhu
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = memory[address / 4] & 0xFFFF; // 16비트만 가져옴
        printf("Load, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    case 0x2b: // sw
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = Reg[decodedInst->rt];
        memory[address / 4] = value;
        printf("Store, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    case 0x28: // sb
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = Reg[decodedInst->rt] & 0xFF; // 8비트만 저장
        memory[address / 4] = (memory[address / 4] & 0xFFFFFF00) | value;
        printf("Store, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    case 0x29: // sh
        address = Reg[decodedInst->rs] + decodedInst->constant;
        value = Reg[decodedInst->rt] & 0xFFFF; // 16비트만 저장
        memory[address / 4] = (memory[address / 4] & 0xFFFF0000) | value;
        printf("Store, Address: 0x%08x, Value: %02x\n", address, value);
        break;
    default:
        printf("Pass\n");
        break;
    }
}

//writeback 
void writeback(inst* decodedInst) {
    switch (type) {
    case 'R':
        printf("    [Write Back] Target: %s, Value: 0x%08x / newPC: 0x%08x\n", register_names[decodedInst->rd], Reg[decodedInst->rd], pc);
        break;
    case 'I':
        printf("    [Write Back] Target: %s, Value: 0x%08x / newPC: 0x%08x\n", register_names[decodedInst->rt], Reg[decodedInst->rt], pc);
        break;
    case 'J':
        printf("    [Write Back] / newPC: 0x%08x\n", pc);
        break;
    default:
        printf("    [Write Back] / newPC: 0x%08x\n", pc);
        break;
    }
}


// 명령어를 출력하는 함수
void printDecode(inst* decodedInst) {
    printf("    [Instruction Decode] ");
    switch (type) { // 전역 변수 type 사용
    case 'R': // R타입 출력
        printf("Type: %c inst: %s %s %s %s \n", type, func, register_names[decodedInst->rd], register_names[decodedInst->rs], register_names[decodedInst->rt]);
        printf("        Opcode: %d ", decodedInst->opcode);
        printf("RS: %d (0x%x) ", decodedInst->rs, Reg[decodedInst->rs]);
        printf("RT: %d (0x%x) ", decodedInst->rt, Reg[decodedInst->rt]);
        printf("RD: %d (0x%x) ", decodedInst->rd, Reg[decodedInst->rd]);
        printf("Shamt: %d ", decodedInst->shamt);
        printf("Funct: %d\n", decodedInst->funct);
        printf("        RegDst: %d ", CU.RegDst);
        printf("RegWrite: %d ", CU.RegWrite);
        printf("ALUSrc: %d ", CU.ALUSrc);
        printf("PCSrc: %d ", CU.PCSrc);
        printf("MemWrite: %d ", CU.MemWrite);
        printf("MemtoReg: %d ", CU.MemtoReg);
        printf("ALUOp: %d\n", CU.ALUop);

        break;

    case 'J': // J타입 출력
        printf("Type: %c inst: %s %d  \n", type, func, decodedInst->address);
        printf("        Opcode: %d ", decodedInst->opcode);
        printf("Imm: %d\n", decodedInst->address);
        printf("        RegDst: %d ", CU.RegDst);
        printf("RegWrite: %d ", CU.RegWrite);
        printf("ALUSrc: %d ", CU.ALUSrc);
        printf("PCSrc: %d ", CU.PCSrc);
        printf("MemWrite: %d ", CU.MemWrite);
        printf("MemtoReg: %d ", CU.MemtoReg);
        printf("ALUOp: %d\n", CU.ALUop);

        break;

    case 'I': // I타입 출력
        printf("Type: %c inst: %s ", type, func);
        if (decodedInst->opcode == 0x23) { // lw
            printf("%s, %d(%s) \n", register_names[decodedInst->rt], decodedInst->constant, register_names[decodedInst->rs]);
        }
        else if (decodedInst->opcode == 0x2b) { // sw
            printf("%s, %d(%s) \n", register_names[decodedInst->rt], decodedInst->constant, register_names[decodedInst->rs]);
        }
        else {
            printf("%s, %s, %d \n", register_names[decodedInst->rs], register_names[decodedInst->rt], decodedInst->constant);
        }
        printf("        Opcode: %d ", decodedInst->opcode);
        printf("RS: %d (0x%x) ", decodedInst->rs, Reg[decodedInst->rs]);
        printf("RT: %d (0x%x) ", decodedInst->rt, Reg[decodedInst->rt]);
        printf("Imm: %d\n", SignExtend((short)decodedInst->constant));
        printf("        RegDst: %d ", CU.RegDst);
        printf("RegWrite: %d ", CU.RegWrite);
        printf("ALUSrc: %d ", CU.ALUSrc);
        printf("PCSrc: %d ", CU.PCSrc);
        printf("MemWrite: %d ", CU.MemWrite);
        printf("MemtoReg: %d ", CU.MemtoReg);
        printf("ALUOp: %d\n", CU.ALUop);

        break;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];

    init_Reg(Reg); // 레지스터 초기화
    int cycle = 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    size_t bytesRead;
    int i = 0;

    // 4바이트 단위로 읽기
    while ((bytesRead = fread(&memory[i], sizeof(unsigned int), 1, file)) == 1 && i < MEMORY_SIZE) {
        memory[i] = convertToBigEndian(memory[i]); // 빅 엔디안 변환
        i++;
    }

    fclose(file);

    // 메모리에서 명령어를 가져오고 디코드 및 출력
    while (1) {
        if (pc == 0xFFFFFFFF) {
            printf("32217072> Final Result\n");
            printf("    Cycles: %d, R-type instructions: %d, I-type instructions: %d, J-type instructions: %d\n", cycle, R_count, I_count, J_count);
            printf("    Return value (v0): %d\n", Reg[2]);
            break;  // pc가 0xFFFFFFFF이면 루프를 종료
        }

        unsigned int IR;
        int validALUresult = 0;
        inst decodedInst = { 0 }; // 구조체 초기화

        cycle++;
        printf("32217072> cycle: %d\n", cycle);
        fetch(&IR);
        decode(IR, &decodedInst);
        validALUresult = execute(&decodedInst);
        printDecode(&decodedInst); // 명령어 출력
        printExecute(validALUresult);
        memoryAccess(&decodedInst, &CU);
        writeback(&decodedInst);
    }

    return 0;
}

