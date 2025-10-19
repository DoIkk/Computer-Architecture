#define _CRT_SECURE_NO_WARNINGS
#define zero 0 //제로 레지스터

#include <ctype.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 100 //메모리의 크기
#define S_REG_SIZE 8 //s레지스터 길이
#define T_REG_SIZE 10 //t레지스터의 길이
#define TOKEN_SIZE 5 // token의 최대길이

typedef struct {
    int data;
} Register; //레지스터 구조체

typedef struct { // opcode, operand1~operand3의 구조체 
    char opcode[5]; //각 단어의 크기는 5을 넘어가지 않기 때문에 5으로 지정
    char operand1[5];
    char operand2[5];
    char operand3[5];
}inst;

char memory[MEMORY_SIZE][MEMORY_SIZE]; //파일저장 2차원 배열
char IR[MEMORY_SIZE]; //fetch후 IR에 한instruction저장
int pc;

Register t[10]; //t레지스터
Register s[8]; //s레지스터
Register v[1]; //v레지스터

int decode(char* instruction, inst* decoded_instruction);
void execute(inst decoded_instruction);
void execute_LW(inst decoded_instruction);
void execute_ADD(inst decoded_instruction);
void execute_SUB(inst decoded_instruction);
void execute_MUL(inst decoded_instruction);
void execute_DIV(inst decoded_instruction);
void execute_NOP();
void execute_JMP(inst decoded_instruction);
void execute_BEQ(inst decoded_instruction);
void execute_BNE(inst decoded_instruction);
void execute_SLT(inst decoded_instruction);


int main(int argc, char* argv[]) {
    // 파일명을 명령행 인자로 받아옴
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    char* filename = argv[1];

    char buffer[MEMORY_SIZE]; // 각 줄을 읽을 버퍼

    //파일 열기
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    int row = 0; // 행 인덱스
    int col = 0; // 열 인덱스

    //파일에서 한 줄씩 읽어서 2차원 배열에 저장
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        col = 0; // 열 인덱스 초기화
        //읽은 문자열을 2차원 배열에 복사
        for (int i = 0; buffer[i] != '\0' && buffer[i] != '\n'; ++i) {
            memory[row][col++] = buffer[i];
        }
        memory[row][col] = '\0'; // 문자열 끝에 널 문자 추가
        row++; // 다음 행으로 이동
    }

    //파일 닫기
    fclose(file);

    inst decoded_instruction;

    // load 단계
    pc = 0;
    while (pc < row) {
        strcpy(IR, memory[pc]);  // fetch 단계
        decode(IR, &decoded_instruction); // decode 단계 
        printf("(line %d) %s\n", pc + 1, memory[pc]);
        printf("32217072> ");
        pc++;
        execute(decoded_instruction);
    }

    return 0;
}

int decode(char* instruction, inst* decoded_instruction) {
    // 공백을 기준으로 토큰화하여 분석
    char* token = strtok(instruction, " ");
    if (token == NULL) {
        printf("Error: The instruction is empty.\n");
        return 1; // 토큰이 없으면 종료
    }

    // opcode 복사
    if (strlen(token) >= TOKEN_SIZE) {
        printf("Error: Opcode length exceeds the allowed limit.\n"); // opcode 길이 오류
        return 1;
    }
    strcpy(decoded_instruction->opcode, token);

    // operand1 복사
    token = strtok(NULL, " ");
    if (token != NULL) {
        if (strlen(token) >= TOKEN_SIZE) {
            printf("Error: Operand1 length exceeds the allowed limit.\n"); // operand1 길이 오류
            return 1;
        }
        strcpy(decoded_instruction->operand1, token);
    }
    else {
        strcpy(decoded_instruction->operand1, "");
    }

    // operand2 복사
    token = strtok(NULL, " ");
    if (token != NULL) {
        if (strlen(token) >= TOKEN_SIZE) {
            printf("Error: Operand2 length exceeds the allowed limit.\n"); //operand2 길이 오류
            return 1;
        }
        strcpy(decoded_instruction->operand2, token);
    }
    else {
        strcpy(decoded_instruction->operand2, "");
    }

    // operand3 복사
    token = strtok(NULL, " ");
    if (token != NULL) {
        if (strlen(token) >= TOKEN_SIZE) {
            printf("Error: Operand3 length exceeds the allowed limit.\n"); // operand3 길이 오류
            return 1;
        }
        strcpy(decoded_instruction->operand3, token);
    }
    else {
        strcpy(decoded_instruction->operand3, "");
    }

    // 추가 토큰이 있는지 확인
    token = strtok(NULL, " ");
    if (token != NULL) {
        printf("Error: There are more than four tokens.\n"); // 단어가 5개 이상이면 오류
        return 1;
    }


    return 0; // 성공적으로 분석 완료
}

void execute(inst decoded_instruction) {
    // opcode에 따라서 실행
    if (strcmp(decoded_instruction.opcode, "LW") == 0) {
        //LW 연산 수행
        execute_LW(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "ADD") == 0) {
        // ADD 연산 수행
        execute_ADD(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "SUB") == 0) {
        // SUB 연산 수행
        execute_SUB(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "MUL") == 0) {
        // MUL 연산 수행
        execute_MUL(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "DIV") == 0) {
        //DIV 연산 수행
        execute_DIV(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "NOP") == 0) {
        // NOP 연산 수행
        execute_NOP();
    }
    else if (strcmp(decoded_instruction.opcode, "JMP") == 0) {
        // JMP 연산 수행
        execute_JMP(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "BEQ") == 0) {
        // BEQ 연산 수행
        execute_BEQ(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "BNE") == 0) {
        // BNE 연산 수행
        execute_BNE(decoded_instruction);
    }
    else if (strcmp(decoded_instruction.opcode, "SLT") == 0) {
        // SLT 연산 수행
        execute_SLT(decoded_instruction);
    }
    else {
        printf("Error: Unknown instruction\n"); // 적절치 않은 opcode 오류
    }
}

void execute_LW(inst decoded_instruction) {
    // Operand1에 저장할 레지스터의 인덱스
    char reg_type = decoded_instruction.operand1[0]; // 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // 레지스터 인덱스

    // Operand2에 있는 16진수 값 가져오기
    int data;
    data = strtol(decoded_instruction.operand2, NULL, 0);

    // Operand1에 저장할 레지스터의 유형 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    // 레지스터에 데이터 로드
    switch (reg_type) {
    case 's':
        s[reg_index].data = data;
        printf("Loaded %d to s%d\n", data, reg_index);
        break;
    case 'v':
        v[reg_index].data = data;
        printf("Loaded %d to v%d\n", data, reg_index);
        break;
    case 't':
        t[reg_index].data = data;
        printf("Loaded %d to t%d\n", data, reg_index);
        break;
    default:
        printf("Unknown register type\n");
    }
}


void execute_ADD(inst decoded_instruction) {

    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스
    char reg2_type = decoded_instruction.operand3[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg2_index = atoi(&decoded_instruction.operand3[1]); // operand3의 레지스터 인덱스

    // Operand1, Operand2, Operand3에 저장할 레지스터의 유효성 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg2_type != 's' && reg2_type != 't' && reg2_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand3에 저장할 레지스터의 인덱스 범위 검사
    if (reg2_type == 's' && (reg2_index < 0 || reg2_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 't' && (reg2_index < 0 || reg2_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 'v' && (reg2_index < 0 || reg2_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    int result = 0; // 결과를 저장할 변수 초기화

    // 각 레지스터 타입에 맞게 연산 수행
    if (reg1_type == 's' && reg2_type == 's') {
        result = s[reg1_index].data + s[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 't') {
        result = s[reg1_index].data + t[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 'z') {
        result = s[reg1_index].data + zero;
    }
    else if (reg1_type == 't' && reg2_type == 's') {
        result = t[reg1_index].data + s[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 't') {
        result = t[reg1_index].data + t[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 'z') {
        result = t[reg1_index].data + zero;
    }
    else if (reg1_type == 'z' && reg2_type == 's') {
        result = zero + s[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 't') {
        result = zero + t[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 'z') {
        result = zero + zero;
    }

    // 결과를 Operand1에 저장된 레지스터에 저장
    if (reg_type == 's') {
        s[reg_index].data = result;
    }
    else if (reg_type == 't') {
        t[reg_index].data = result;
    }
    else if (reg_type == 'v') {
        v[reg_index].data = result;
    }

    // 출력 
    printf("Added %c%d(%d) to %c%d(%d) and changed %c%d to %d\n",
        reg1_type, reg1_index, (reg1_type == 's') ? s[reg1_index].data : (reg1_type == 't') ? t[reg1_index].data : zero,
        reg2_type, reg2_index, (reg2_type == 's') ? s[reg2_index].data : (reg2_type == 't') ? t[reg2_index].data : zero,
        reg_type, reg_index, (reg_type == 's') ? s[reg_index].data : (reg_type == 't') ? t[reg_index].data : v[reg_index].data);
}



void execute_SUB(inst decoded_instruction) {

    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스
    char reg2_type = decoded_instruction.operand3[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg2_index = atoi(&decoded_instruction.operand3[1]); // operand3의 레지스터 인덱스

    int result = 0; // 결과를 저장할 변수 초기화

    // Operand1, Operand2, Operand3에 대한 레지스터 타입 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg2_type != 's' && reg2_type != 't' && reg2_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }

    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand3에 저장할 레지스터의 인덱스 범위 검사
    if (reg2_type == 's' && (reg2_index < 0 || reg2_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 't' && (reg2_index < 0 || reg2_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 'v' && (reg2_index < 0 || reg2_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }


    // 각 레지스터 타입에 맞게 연산 수행
    if (reg1_type == 's' && reg2_type == 's') {
        result = s[reg1_index].data - s[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 't') {
        result = s[reg1_index].data - t[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 'z') {
        result = s[reg1_index].data - zero;
    }
    else if (reg1_type == 't' && reg2_type == 's') {
        result = t[reg1_index].data - s[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 't') {
        result = t[reg1_index].data - t[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 'z') {
        result = t[reg1_index].data - zero;
    }
    else if (reg1_type == 'z' && reg2_type == 's') {
        result = zero - s[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 't') {
        result = zero - t[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 'z') {
        result = zero - zero;
    }


    // 결과를 Operand1에 저장된 레지스터에 저장
    if (reg_type == 's') {
        s[reg_index].data = result;
    }
    else if (reg_type == 't') {
        t[reg_index].data = result;
    }
    else if (reg_type == 'v') {
        v[reg_index].data = result;
    }

    // 출력 (테스트용)
    printf("Subtracted %c%d(%d) to %c%d(%d) and changed %c%d to %d\n",
        reg1_type, reg1_index, (reg1_type == 's') ? s[reg1_index].data : (reg1_type == 't') ? t[reg1_index].data : zero,
        reg2_type, reg2_index, (reg2_type == 's') ? s[reg2_index].data : (reg2_type == 't') ? t[reg2_index].data : zero,
        reg_type, reg_index, (reg_type == 's') ? s[reg_index].data : (reg_type == 't') ? t[reg_index].data : v[reg_index].data);
}


void execute_MUL(inst decoded_instruction) {

    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스
    char reg2_type = decoded_instruction.operand3[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg2_index = atoi(&decoded_instruction.operand3[1]); // operand3의 레지스터 인덱스


    // Operand1, Operand2, Operand3 에 저장할 레지스터의 유효성 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg2_type != 's' && reg2_type != 't' && reg2_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }

    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand3에 저장할 레지스터의 인덱스 범위 검사
    if (reg2_type == 's' && (reg2_index < 0 || reg2_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 't' && (reg2_index < 0 || reg2_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 'v' && (reg2_index < 0 || reg2_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    int result = 0; // 결과를 저장할 변수 초기화

    // 각 레지스터 타입에 맞게 연산 수행
    if (reg1_type == 's' && reg2_type == 's') {
        result = s[reg1_index].data * s[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 't') {
        result = s[reg1_index].data * t[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 'z') {
        result = s[reg1_index].data * zero;
    }
    else if (reg1_type == 't' && reg2_type == 's') {
        result = t[reg1_index].data * s[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 't') {
        result = t[reg1_index].data * t[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 'z') {
        result = t[reg1_index].data * zero;
    }
    else if (reg1_type == 'z' && reg2_type == 's') {
        result = zero * s[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 't') {
        result = zero * t[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 'z') {
        result = zero * zero;
    }

    // 결과를 Operand1에 저장된 레지스터에 저장
    if (reg_type == 's') {
        s[reg_index].data = result;
    }
    else if (reg_type == 't') {
        t[reg_index].data = result;
    }
    else if (reg_type == 'v') {
        v[reg_index].data = result;
    }

    // 출력 
    printf("Multiplied %c%d(%d) to %c%d(%d) and changed %c%d to %d\n",
        reg1_type, reg1_index, (reg1_type == 's') ? s[reg1_index].data : (reg1_type == 't') ? t[reg1_index].data : zero,
        reg2_type, reg2_index, (reg2_type == 's') ? s[reg2_index].data : (reg2_type == 't') ? t[reg2_index].data : zero,
        reg_type, reg_index, (reg_type == 's') ? s[reg_index].data : (reg_type == 't') ? t[reg_index].data : v[reg_index].data);
}

void execute_DIV(inst decoded_instruction) {

    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스
    char reg2_type = decoded_instruction.operand3[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg2_index = atoi(&decoded_instruction.operand3[1]); // operand3의 레지스터 인덱스

    // 예외 처리: 0으로 나누는 경우
    if (reg2_type == 's' && s[reg2_index].data == 0) {
        printf("Error: Division by zero\n");
        return;
    }
    else if (reg2_type == 'v' && t[reg2_index].data == 0) {
        printf("Error: Division by zero\n");
        return;
    }
    else if (reg2_type == 't' && t[reg2_index].data == 0) {
        printf("Error: Division by zero\n");
        return;
    }

    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type.\n");
        return;
    }

    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand3에 저장할 레지스터의 인덱스 범위 검사
    if (reg2_type == 's' && (reg2_index < 0 || reg2_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 't' && (reg2_index < 0 || reg2_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 'v' && (reg2_index < 0 || reg2_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    // Operand1, Operand2, Operand3 에 저장할 레지스터의 유효성 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg2_type != 's' && reg2_type != 't' && reg2_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }

    int result = 0;
    // 각 레지스터 타입에 맞게 연산 수행
    if (reg1_type == 's' && reg2_type == 's') {
        result = s[reg1_index].data / s[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 't') {
        result = s[reg1_index].data / t[reg2_index].data;
    }
    else if (reg1_type == 's' && reg2_type == 'z') {
        printf("Error: Division by zero\n");
        return;
    }
    else if (reg1_type == 't' && reg2_type == 's') {
        result = t[reg1_index].data / s[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 't') {
        result = t[reg1_index].data / t[reg2_index].data;
    }
    else if (reg1_type == 't' && reg2_type == 'z') {
        printf("Error: Division by zero\n");
        return;
    }
    else if (reg1_type == 'z' && reg2_type == 's') {
        result = zero / s[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 't') {
        result = zero / t[reg2_index].data;
    }
    else if (reg1_type == 'z' && reg2_type == 'z') {
        printf("Error: Division by zero\n");
        return;
    }


    // 결과를 Operand1에 저장된 레지스터에 저장
    if (reg_type == 's') {
        s[reg_index].data = result;
    }
    else if (reg_type == 't') {
        t[reg_index].data = result;
    }
    else if (reg_type == 'v') {
        v[reg_index].data = result;
    }

    // 출력
    printf("Diveded %c%d(%d) to %c%d(%d) and changed %c%d to %d\n",
        reg1_type, reg1_index, (reg1_type == 's') ? s[reg1_index].data : (reg1_type == 't') ? t[reg1_index].data : zero,
        reg2_type, reg2_index, (reg2_type == 's') ? s[reg2_index].data : (reg2_type == 't') ? t[reg2_index].data : zero,
        reg_type, reg_index, (reg_type == 's') ? s[reg_index].data : (reg_type == 't') ? t[reg_index].data : v[reg_index].data);
}

void execute_NOP() {
    printf("No Operation\n");
}

void execute_JMP(inst decoded_instruction) {
    int line; // line number
    if (isdigit(decoded_instruction.operand1[0])) {
        // 만약 operand1이 숫자로 시작하면 직접 숫자로 해석
        line = strtol(decoded_instruction.operand1, NULL, 0); //line number
    }
    else if (decoded_instruction.operand1[0] == 's' ||
        decoded_instruction.operand1[0] == 't' ||
        decoded_instruction.operand1[0] == 'v') {
        // 만약 operand1이 레지스터인 경우, 오류 처리
        printf("Error: The JMP instruction cannot directly use a register.\n");
        return;
    }
    else {
        // 그렇지 않으면 예외 처리
        printf("Error: JMP invalid operand.\n");
        return;
    }


    printf("Jumped to line %d\n", line); //출력
    pc = line - 1; // pc는 메모리의 행을 세기 때문에 line에서 1을 뺌

}



void execute_BEQ(inst decoded_instruction) {
    // Operand1에 저장할 레지스터의 인덱스
    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스

    int line; // line number
    line = strtol(decoded_instruction.operand3, NULL, 0);
    //line number

    // Operand1, Operand2에 저장할 레지스터의 유효성 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    if (reg_type == 's' && reg1_type == 's') {

        if (s[reg_index].data == s[reg1_index].data) { //같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
        }
        else if (s[reg_index].data != s[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 's' && reg1_type == 't') {

        if (s[reg_index].data == t[reg1_index].data) { //같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (s[reg_index].data != t[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }


    }
    else if (reg_type == 's' && reg1_type == 'z') {

        if (s[reg_index].data == zero) { // 같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, zero, line);
        }
        else if (s[reg_index].data != zero) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, zero, line);
            return;
        }

    }
    else if (reg_type == 't' && reg1_type == 's') {
        if (t[reg_index].data == s[reg1_index].data) { // 같으면 점프 
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
        }
        else if (t[reg_index].data != s[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 't' && reg1_type == 't') {

        if (t[reg_index].data == t[reg1_index].data) { // 같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (t[reg_index].data != t[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }
    }
    else if (reg_type == 't' && reg1_type == 'z') {
        if (t[reg_index].data == zero) { // 같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, zero, line);

        }
        else if (t[reg_index].data != zero) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, zero, line);
            return;
        }
    }
    else if (reg_type == 'z' && reg1_type == 's') {
        if (zero == s[reg1_index].data) { // 같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, s[reg1_index].data, line);
        }
        else if (zero != s[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, s[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 'z' && reg1_type == 't') {
        if (zero == t[reg1_index].data) { // 같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (zero != t[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 'z' && reg1_type == 'z') {
        if (zero == t[reg1_index].data) { // 같으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (zero != t[reg1_index].data) { // 같지 않으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }

    }

}

void execute_BNE(inst decoded_instruction) {

    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스

    int line; //line number
    line = strtol(decoded_instruction.operand3, NULL, 0);
    //line number

        // Operand1, Operand2에 저장할 레지스터의 유효성 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    if (reg_type == 's' && reg1_type == 's') {
        if (s[reg_index].data != s[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
        }
        else if (s[reg_index].data == s[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 's' && reg1_type == 't') {

        if (s[reg_index].data != t[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (s[reg_index].data == t[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }


    }
    else if (reg_type == 's' && reg1_type == 'z') {

        if (s[reg_index].data != zero) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, zero, line);
        }
        else if (s[reg_index].data == zero) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, s[reg_index].data,
                reg1_type, reg1_index, zero, line);
            return;
        }

    }
    else if (reg_type == 't' && reg1_type == 's') {

        if (t[reg_index].data != s[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
        }
        else if (t[reg_index].data == s[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, s[reg1_index].data, line);
            return;
        }


    }
    else if (reg_type == 't' && reg1_type == 't') {

        if (t[reg_index].data != t[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (t[reg_index].data == t[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not  equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 't' && reg1_type == 'z') {

        if (t[reg_index].data != zero) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, zero, line);

        }
        else if (t[reg_index].data == zero) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, t[reg_index].data,
                reg1_type, reg1_index, zero, line);
            return;
        }

    }
    else if (reg_type == 'z' && reg1_type == 's') {

        if (zero != s[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, s[reg1_index].data, line);
        }
        else if (zero == s[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, s[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 'z' && reg1_type == 't') {

        if (zero != t[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, t[reg1_index].data, line);
        }
        else if (zero == t[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, t[reg1_index].data, line);
            return;
        }

    }
    else if (reg_type == 'z' && reg1_type == 'z') {

        if (zero != t[reg1_index].data) { // 같지 않으면 점프
            pc = line - 1;
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and jumped to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, zero, line);
        }
        else if (zero == t[reg1_index].data) { // 같으면 다음 명령어
            printf("checked if %c%d(%d) is not equal to %c%d(%d) and didn't jump to line %d\n", reg_type, reg_index, zero,
                reg1_type, reg1_index, zero, line);
            return;
        }

    }

}

void execute_SLT(inst decoded_instruction) {

    char reg_type = decoded_instruction.operand1[0]; // operand1의 레지스터 타입 (s, v, t)
    int reg_index = atoi(&decoded_instruction.operand1[1]); // operand1의 레지스터 인덱스
    char reg1_type = decoded_instruction.operand2[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg1_index = atoi(&decoded_instruction.operand2[1]); // operand2의 레지스터 인덱스
    char reg2_type = decoded_instruction.operand3[0]; // operand2의 레지스터 타입 (s, v, t)
    int reg2_index = atoi(&decoded_instruction.operand3[1]); // operand3의 레지스터 인덱스

    int result = 0; // 결과를 저장할 변수 초기화

    // Operand1, Operand2, Operand3에 저장할 레지스터의 유효성 검사
    if (reg_type != 's' && reg_type != 't' && reg_type != 'v') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg1_type != 's' && reg1_type != 't' && reg1_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    if (reg2_type != 's' && reg2_type != 't' && reg2_type != 'z') {
        printf("Error: invalid register type\n");
        return;
    }
    // Operand1에 저장할 레지스터의 인덱스 범위 검사
    if (reg_type == 's' && (reg_index < 0 || reg_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 't' && (reg_index < 0 || reg_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg_type == 'v' && (reg_index < 0 || reg_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand2에 저장할 레지스터의 인덱스 범위 검사
    if (reg1_type == 's' && (reg1_index < 0 || reg1_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 't' && (reg1_index < 0 || reg1_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg1_type == 'v' && (reg1_index < 0 || reg1_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }
    // Operand3에 저장할 레지스터의 인덱스 범위 검사
    if (reg2_type == 's' && (reg2_index < 0 || reg2_index > S_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 't' && (reg2_index < 0 || reg2_index > T_REG_SIZE - 1)) {
        printf("Error: invalid register index\n");
        return;
    }
    if (reg2_type == 'v' && (reg2_index < 0 || reg2_index > 0)) {
        printf("Error: invalid register index\n");
        return;
    }

    // 각 레지스터 타입에 맞게 연산 수행
    if (reg1_type == 's' && reg2_type == 's') {
        if (s[reg1_index].data < s[reg2_index].data) { // operand2가 operand3보다 작으면 result = 1
            result = 1;
        }
        else if (s[reg1_index].data >= s[reg2_index].data) { // operand2가 operand3 보다 작지 않으면 result = 0
            result = 0;
        }
    }
    else if (reg1_type == 's' && reg2_type == 't') { // operand2가 operand3보다 작으면 result = 1
        if (s[reg1_index].data < t[reg2_index].data) {
            result = 1;
        }
        else if (s[reg1_index].data >= t[reg2_index].data) { // operand2가 operand3 보다 작지 않으면 result = 0
            result = 0;
        }
    }
    else if (reg1_type == 't' && reg2_type == 's') { // operand2가 operand3보다 작으면 result = 1
        if (t[reg1_index].data < s[reg2_index].data) {
            result = 1;
        }
        else if (t[reg1_index].data >= s[reg2_index].data) { // operand2가 operand3 보다 작지 않으면 result = 0
            result = 0;
        }
    }
    else if (reg1_type == 't' && reg2_type == 't') { // operand2가 operand3보다 작으면 result = 1
        if (t[reg1_index].data < t[reg2_index].data) {
            result = 1;
        }
        else if (t[reg1_index].data >= t[reg2_index].data) { // operand2가 operand3 보다 작지 않으면 result = 0
            result = 0;
        }
    }

    // 결과를 Operand1에 저장된 레지스터에 저장
    if (reg_type == 's') {
        s[reg_index].data = result;
    }
    else if (reg_type == 't') {
        t[reg_index].data = result;
    }
    else if (reg_type == 'v') {
        v[reg_index].data = result;
    }

    // 출력
    printf("checked if %c%d(%d) is less than %c%d(%d) and set %d to %c%d\n",
        reg1_type, reg1_index, (reg1_type == 's') ? s[reg1_index].data : t[reg1_index].data,
        reg2_type, reg2_index, (reg2_type == 's') ? s[reg2_index].data : t[reg2_index].data,
        (reg_type == 's') ? s[reg_index].data : (reg_type == 't') ? t[reg_index].data : v[reg_index].data, reg_type, reg_index);
}