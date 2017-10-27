#include<iostream>
#include<stdio.h>
#include<math.h>
// #include <io.h>
// #include <process.h>
#include<time.h>
#include<stdlib.h>
#include"Reg_def.h"
#include "Read_Elf.h"

#define OP_AUIPC 0x17
#define OP_LUI 0x37
#define OP_JAL 0x6f

#define OP_R 0x33
#define OP_ADDI 0x13

#define F3_ADD 0
#define F3_MUL 0
#define F7_MULH 0x01
#define F7_DIV 0x1

#define F7_MSE 1
#define F7_ADD 0
#define F7_MUL 0x01
#define F7_SUB 0x20
#define F3_ADDI 0

#define F3_SLL 0x1
#define F7_SLL 0
#define F3_SLT 0x2
#define F7_SLT 0
#define F3_SRL 0x5
#define F7_SRL 0
#define F7_SRA 0x20

#define F3_OR 0x6
#define F7_OR 0
#define F3_XOR 0x4
#define F7_XOR 0

#define F7_REM 0x1
#define F3_AND 0x7
#define F7_AND 0

#define F3_SLLI 1
#define F7_SLLI 0
#define F3_SLTI 2
#define F3_XORI 4
#define F3_SRLI 5
#define F7_SRLI 0
#define F7_SRAI 0x20
#define F3_ORI 6
#define F3_ANDI 7

#define OP_SW 35

#define OP_LW 3
#define F3_LB 0

#define OP_BEQ 99

#define OP_SB 0x63
#define F3_BEQ 0
#define F3_BNE 1
#define F3_BLT 4
#define F3_BGE 5

#define OP_IW 27
#define F3_ADDIW 0

#define OP_RW 59
#define F3_ADDW 0
#define F7_ADDW 0

#define OP_S 0x23
#define F3_SB 0
#define F3_SH 1
#define F3_SW 2
#define F3_SD 3

#define OP_ADDIW 0x1b
#define F3_ADDIW 0
#define OP_JALR 0x67
#define F3_JALR 0
#define F3_SCALL 0
#define F7_SCALL 0

#define OP_LB 3
#define F3_LB 0
#define F3_LH 1
#define F3_LW 2
#define F3_LD 3

#define MAX 400000000

// define for mulh
#define MASK_OP 0x7f
#define MASK_REG 0x1f
#define MASK_F3 0x7
#define MASK_F7 0x7f
#define MASK_H 0xffffffff00000000
#define MASK_L 0xffffffff
#define MASK_FSU 0x7fffffff
#define MASK_FS 0x80000000
#define MASK_FLU 0x7fffffffffffffff
#define MASK_FL 0x8000000000000000

//主存
unsigned char memory[MAX]={0};
//寄存器堆
REG reg[32]={0};
//PC
int PC=0;


//各个指令解析段
unsigned int OP=0;
unsigned int fun3 = 0, fun7 = 0;
int rs=0,rt=0,rd=0;
int rs1=0,rs2=0,shamt=0;
int imm=0;


//加载内存
void load_memory();

void simulate();

void IF();

void ID();

void EX();

void MEM();

void WB();

void reg_Output() {
	printf("reg info:\n");
	for(int i = 0; i < 32; i++)
		printf("%llx ", reg[i]);
	printf("\n");
}
//符号扩展
int ext_signed(unsigned int src,int bit);

//获取指定位
unsigned int getbit(int s,int e);

unsigned int getbit(unsigned inst,int s,int e) {
	int mask = 0;
	for(int i = s; i<=e; i++)
		mask |= (1<<i);
	return ((inst&mask)>>s);
}

int ext_signed(int src, int sign, int bit) {
	int mask;
	if(sign) {
		int shift = bit - 1;
		mask = (src&(1<<shift))>>shift;
		if(mask && bit < 32) 
			src -= (1<<bit);
	}
    return (long long)src;
}

