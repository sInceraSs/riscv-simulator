#pragma once
#pragma once
typedef unsigned long long REG;

struct IFID {
	unsigned int inst;
	//inst means the instruction which is going to execute
	int PC;
	//PC means the index of the instruction
}IF_ID, IF_ID_old;


struct IDEX {
	int Rd, Rs1, Rs2;
	int PC;
	long long Imm;
	REG Reg_Rd, Reg_Rs1,Reg_Rs2;

	int Ctrl_EX_ALUSrc;
	int Ctrl_EX_ALUOp;
	int Ctrl_EX_RegDst;//controlÐÅºÅ

	int Ctrl_M_Branch;
	int Ctrl_M_MemWrite;
	int Ctrl_M_MemRead;

	int Ctrl_WB_RegWrite;
	int Ctrl_WB_MemtoReg;

}ID_EX, ID_EX_old;

struct EXMEM {
	int PC;
	int Reg_Dst;
	int Reg_Rt;
	REG ALU_out;
	int ALU_op;
	int Zero;
	REG Reg_Rs2;//used in writing to memory

	int Ctrl_M_Branch;
	int Ctrl_M_MemWrite;
	int Ctrl_M_MemRead;

	int Ctrl_WB_RegWrite;
	int Ctrl_WB_MemtoReg;

}EX_MEM, EX_MEM_old;

struct MEMWB {
	int PC;
	unsigned long long Mem_read;//the data from the memory
	REG ALU_out;
	int Reg_Dst;

	int Ctrl_WB_RegWrite;
	int Ctrl_WB_MemtoReg;

}MEM_WB, MEM_WB_old;