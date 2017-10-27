#include "Simulation.h"
using namespace std;

extern void read_elf();
extern unsigned int cadr;
extern unsigned int csize;
extern unsigned int vadr;
extern unsigned long long gp;
extern unsigned int madr;
extern unsigned int msize;
extern unsigned int snum;
extern unsigned int sadr;
extern unsigned int ssize;
extern unsigned int endPC;
extern unsigned int entry;
extern FILE *file;

bool garrulous = true;
//指令运行数
long long inst_num=0;

//系统调用退出指示
int exit_flag=0;

//加载代码段
//初始化PC
void load_memory() {
	Elf64_Shdr elf64_shdr;
	#ifdef DEBUG
		printf("sum:%d ssize:%d sadr:%d\n", snum, ssize, sadr);
	#endif
	for(int i=0;i<snum;i++) {
		fseek(file, sadr + i*ssize, SEEK_SET);
		fread(&elf64_shdr,1,sizeof(elf64_shdr),file);
		fseek(file, elf64_shdr.sh_offset, SEEK_SET);
		fread(&memory[elf64_shdr.sh_addr],1,elf64_shdr.sh_size, file);
	}
}

int main(int argc, char *argv[])
{
	memset(reg,0,sizeof(reg));
	if(argc < 2)
		return 0;
	char *filename = argv[1];
	file = fopen(filename, "rb");
	if(file == NULL) {
		printf("No such file !!!\n");
		exit(1);
	}
	//解析elf文件
	read_elf();
	
	//加载内存
	load_memory(); 

	//设置入口地址
	PC=madr;
	
	//设置全局数据段地址寄存器
	reg[3]=gp;
	
	reg[2]=MAX/2;//栈基址 （sp寄存器）

	simulate();

	cout <<"simulate over!"<<endl;
	return 0;
}

void simulate()
{
	//结束PC的设置
	endPC = madr + msize -3;//madr + msize -3
	while(PC!=endPC) {
		//运行
		IF();
		ID();
		EX();
		MEM();
		WB();

		//更新中间寄存器
		// IF_ID=IF_ID_old;
		// ID_EX=ID_EX_okd;
		// EX_MEM=EX_MEM_old;
		// MEM_WB=MEM_WB_old;

        if(exit_flag==1)
            break;

        reg[0]=0;//一直为零
	}
}

//取指令
void IF()
{
	//write IF_ID_old
	memcpy(&IF_ID.inst,&memory[PC],4);
	PC=PC+4;
	IF_ID.PC=PC;
#ifdef DEBUG
	printf("PC%d %d %x\n", PC, madr, IF_ID.inst);
#endif
}

//译码
void ID()
{
	//Read IF_ID
	unsigned int inst=IF_ID.inst;
	int EXTop=0;
	unsigned int EXTsrc=0; // extend extop length 
	//
	int RegDst,ALUop,ALUSrc_A, ALUSrc_B;
	int Branch,MemRead,MemWrite;
	int RegWrite,MemtoReg;

	Branch = 0, MemRead = 0, MemWrite = 0;
	RegWrite = 0, MemtoReg = 0, EXTop = 0, EXTsrc = 0;
	OP = getbit(inst, 0, 6);
	rd=getbit(inst,7,11);
	fun3 = getbit(inst, 12, 14);
	rs1 = getbit(inst, 15, 19);
	rs2 = getbit(inst, 20, 24);
	shamt = getbit(inst, 20, 25);
	fun7 = getbit(inst, 25, 31);
	imm = 0;
#ifdef DEBUG
	printf("inst %x OP%d\n", inst, OP);
#endif
	RegDst = rd;
	ALUSrc_A = rs1;
	ALUSrc_B = rs2;
	// type R

	if(OP==OP_R) {
		RegWrite=1;
		if(fun3==0x0&&fun7==0x0) {// add
			ALUop=0;
			if (garrulous) printf("add ");
		}
		else if(fun3==0x0&&fun7==0x1) {// mull
			ALUop=1;
			if (garrulous) printf("mull ");
		}
		else if(fun3==0x0&&fun7==0x20) {//sub
			ALUop=2;
			if (garrulous) printf("sub ");
		}
		else if(fun3==0x1&&fun7==0x0) {//sll
			ALUop=3;
			if (garrulous) printf("sll ");
		}
		else if(fun3=0x1&&fun7==0x1) {//mulh
			ALUop=4;
			if (garrulous) printf("mulh ");
		}
		else if(fun3==0x2&&fun7==0x0) {//slt
			ALUop=5;
			if (garrulous) printf("slt ");
		}
		else if (fun3==0x4&&fun7==0x0) {//xor
			ALUop=6;
			if (garrulous) printf("xor ");
		}
		else if (fun3 == 0x4&& fun7 == 0x1) {//div
			ALUop = 7;
			if (garrulous) printf("div ");
		}
		else if (fun3 == 0x5&& fun7 == 0x0) {//srl
			ALUop = 8;
			if (garrulous) printf("srl ");
		}
		else if (fun3 == 0x5&& fun7 == 0x20) {//sra
			ALUop = 9;
			if (garrulous) printf("sra ");
		}
		else if (fun3 == 0x6&& fun7 == 0x0) {//or
			ALUop = 10;
			if (garrulous) printf("or ");
		}
		else if (fun3 == 0x6&& fun7 == 0x1) {//rem
			ALUop = 11;
			if (garrulous) printf("rem ");
		}
		else if (fun3 == 0x7&& fun7 == 0x0) {//and
			ALUop = 12;
			if (garrulous) printf("and ");
		}
	} // type I
	else if(OP==OP_LB || OP==OP_ADDI || OP==OP_ADDIW || OP==OP_JALR) {
		imm = getbit(inst, 20, 31);
		if(OP==OP_LB) { //0x03
			MemtoReg = 1;
			MemRead = 1;
			EXTop = 1;
			EXTsrc = 0xc; // offset length
			ALUop = 13;
			if (garrulous) printf("lb ");
			switch(fun3) {
				case 0: // byte
					MemRead = 1;
					break;
				case 1: // half
					MemRead = 2;
					break;
				case 2: // word
					MemRead = 3;
					break;
				case 3: // doubleword
					MemRead = 4;
					break;
			}
		} else if (OP==OP_ADDI) { // 0x13
			RegWrite = 1;
			switch(fun3){
			case F3_ADDI:
        		ALUop = 14;
        		EXTsrc = 0xc;
        		EXTop = 1;
				if (garrulous) printf("addi ");
        		break;
        	case F3_SLLI:
        		if(fun7 = F7_SLLI) {
        			EXTsrc = 0xc;
        			ALUop = 15;
        			EXTop = 1;
					if (garrulous) printf("slli ");
        		}
        		break;
        	case F3_SLTI:
        		ALUop = 16;
				if (garrulous) printf("slti ");
        		break;
        	case F3_XORI:
        		ALUop = 17;
				if (garrulous) printf("xori ");
        		break;
        	case F3_SRLI:
        		if (fun7 == F7_SRLI) {
        			ALUop = 18;
					if (garrulous) printf("srli ");
        		}
        		else if(fun7 == F7_SRAI) {
        			ALUop = 19;
					if (garrulous) printf("srai ");
        		}
        		break;
        	case F3_ORI: 
        		ALUop = 20;
				if (garrulous) printf("ori ");
        		break;
        	case F3_ANDI:
        		ALUop = 21;
				if (garrulous) printf("andi ");
        		break;
			}
		} else if (OP==OP_ADDIW) { // 0x1b
			RegWrite = 1; 
			if(fun3 == F3_ADDIW) {
    			EXTop = 1;
    			EXTsrc = 0xc;
    			ALUop = 22;
				if (garrulous) printf("addiw ");
				#ifdef DEBUG
				printf("imm: %ld %llx\n", imm, ext_signed(imm, EXTop, EXTsrc));
				#endif
			}
		} else if(OP==OP_JALR && fun3==F3_JALR) { // 0x67
			EXTop = 1;
			EXTsrc = 0xc;
			RegWrite = 1;
			ALUop = 23;
			imm = (imm>>1) << 1;// last bit is zero
			if (garrulous) printf("jalr ");
		}
    } // type S
    else if(OP==OP_S) {
    	EXTop = 1;
		EXTsrc = 0xc;
		MemWrite = 1;
		imm = getbit(inst,7,11) + (getbit(inst,25,31)<<5);
		if (garrulous) printf("sb ");
		switch(fun3){
			case F3_SB:
				ALUop = 24;
				break;
			case F3_SH:
				ALUop = 25;
				break;
			case F3_SW:
				ALUop = 26;
				break;
			case F3_SD:
				ALUop = 27;
				break;
		}
    } // type SB
  	else if (OP == OP_SB) { // branch type
		imm = (getbit(inst, 7, 7)<< 11) + ((getbit(inst, 8, 11)<<1) + (getbit(inst, 25, 30)<< 5) +  (getbit(inst, 31, 31)<< 12));//SB
		EXTop = 1;
    	EXTsrc = 0xd;
    	ALUop = 28;
		switch(fun3){
			case F3_BEQ:
				Branch = 2;
				if (garrulous) printf("beq ");
				break;
			case F3_BNE:
				Branch = 3;
				if (garrulous) printf("bne ");
				break;
			case F3_BLT:
				Branch = 4;
				if (garrulous) printf("blt ");
				break;
			case F3_BGE:
				Branch = 5;
				if (garrulous) printf("bge ");
				break;
		}
    }
    else if(OP == OP_AUIPC) {
    	// R[rd] = pc + imm
    	imm = (getbit(inst,12,31)<<12);
		RegWrite = 1;
		ALUop = 29;
		if (garrulous) printf("auipc ");
    } else if (OP == OP_LUI) {
		EXTop = 1;
		imm = (getbit(inst,12,31)<<12);
		RegWrite = 1;
		ALUop = 30;
		if (garrulous) printf("lui ");
	} else if (OP == OP_JAL) {
		EXTop = 1;
		EXTsrc = 0x15;
		imm = (getbit(inst,12,19)<<12) + (getbit(inst,20,20)<<11) + (getbit(inst,21,30)<<1) + (getbit(inst,31,31)<<20);
		ALUop = 31;
		RegWrite = 1;
		if (garrulous) printf("jal ");
	}

	ID_EX.Rd = rd;
	ID_EX.Rs1 = rs1;
	ID_EX.Rs2 = rs2;
	ID_EX.PC = PC;
	ID_EX.Imm = ext_signed(imm, EXTop, EXTsrc);
	ID_EX.Reg_Rd = reg[rd];
	ID_EX.Reg_Rs1 = reg[rs1];
	ID_EX.Reg_Rs2 = reg[rs2];

	ID_EX.Ctrl_EX_ALUOp = ALUop;
	ID_EX.Ctrl_EX_RegDst = rd;

	ID_EX.Ctrl_M_Branch = Branch;
	ID_EX.Ctrl_M_MemWrite = MemWrite;
	ID_EX.Ctrl_M_MemRead = MemRead;

	ID_EX.Ctrl_WB_RegWrite = RegWrite;
	ID_EX.Ctrl_WB_MemtoReg = MemtoReg;
	ID_EX.PC = PC;
	if (garrulous) printf("rs1=%d rs2=%d rd=%d imm=%lld\n", rs1, rs2, rd, ID_EX.Imm);
}
bool cao_ = false;
//执行
void EX()
{
	//read ID_EX
	int Rd = ID_EX.Rd;
	int Rs1 = ID_EX.Rs1;
	int Rs2 = ID_EX.Rs2;
	int temp_PC = ID_EX.PC;
	long long Imm = ID_EX.Imm;


	int RegDst=ID_EX.Ctrl_EX_RegDst;
	int ALUOp=ID_EX.Ctrl_EX_ALUOp;
	int ALUSrc = ID_EX.Ctrl_EX_ALUSrc;

	int Branch = ID_EX.Ctrl_M_Branch;
	int MemWrite = ID_EX.Ctrl_M_MemWrite;
	int MemRead = ID_EX.Ctrl_M_MemRead;
	int Zero = reg[0];
	int RegWrite = ID_EX.Ctrl_WB_RegWrite;
	int MemtoReg = ID_EX.Ctrl_WB_MemtoReg;	
	//Branch PC calulate

	//choose ALU input number

	//alu calculate
	REG ALUout;
	switch(ALUOp){
		case 0: // add
			ALUout = reg[Rs1] + reg[Rs2];
			break;
		case 1: // mull
			ALUout = (reg[Rs1] * reg[Rs2]);
			break;
		case 2: // sub
			ALUout = reg[Rs1] - reg[Rs2];
			break;
		case 3: // sll
			ALUout = reg[Rs1] << (unsigned int)reg[Rs2];
			break;
		case 4:{	// mulh
			unsigned long long a, b;
			a = (unsigned long long)reg[Rs1], b = (unsigned long long)reg[Rs2];
			unsigned long long a1 = ((a & MASK_H) >> 32);
			unsigned long long a0 = a & MASK_L;
			unsigned long long b1 = ((b & MASK_H) >> 32);
			unsigned long long b0 = b & MASK_L;
			unsigned long long a_x_b_low = a0 * b0;
			unsigned long long a_x_b_mid = a1 * b0;
			unsigned long long b_x_a_mid = b1 * a0;
			unsigned long long a_x_b_high = a1 * b1;
			unsigned long long carry = (a_x_b_low >> 32) + (a_x_b_mid & MASK_L) + (b_x_a_mid & MASK_L);
			carry = carry >> 32;
			ALUout = (a_x_b_high & MASK_H) + (a_x_b_mid >> 32) + (b_x_a_mid >> 32) + (a_x_b_high & MASK_L) + carry;
			break;
		}
		case 5://slt
			ALUout = ((long long)reg[Rs1]<(long long)reg[Rs2]) ? 1 : 0;
			break;
		case 6://xor
			ALUout = reg[Rs1] ^ reg[Rs2];
			break;
		case 7://div
			ALUout = (unsigned long long)(reg[Rs1] / reg[Rs2]);
			break;
		case 8://srl
			ALUout = ((reg[Rs1] >> (unsigned int)reg[Rs2]));
			break;
		case 9://sra
			ALUout = (long long)reg[Rs1] >> (unsigned int)reg[Rs2];
			break;
		case 10://or
			ALUout = reg[Rs1] | reg[Rs2];
			break;
		case 11:
			ALUout = reg[Rs1] % reg[Rs2];
			break;
		case 12:
			ALUout = reg[Rs1] & reg[Rs2];
			break;
		case 13: // load type
			ALUout = reg[Rs1] + Imm;
			break;
		case 14: //addi
			ALUout = reg[Rs1] + Imm;
		#ifdef DEBUG
			printf("addi %d %lx %d %lx \n\n", Rs1, reg[Rs1], Imm, ALUout);
		#endif
			break;
		case 15: //slli
			ALUout = reg[Rs1] << (unsigned int)Imm;
			break;
		case 16: // slti
			ALUout = ((long long)reg[Rs1]<Imm) ? 1:0;
			break;
		case 17://xori
			ALUout = reg[Rs1] ^ Imm;
			break;
		case 18: // srli
			ALUout = (unsigned long long)((reg[Rs1] >> imm));
			break;
		case 19: // srai
			ALUout = (long long)((reg[Rs1] >> Imm));
			break;
		case 20: // ori
			ALUout = reg[Rs1] | Imm;
			break;
		case 21: // andi
			ALUout = reg[Rs1] & Imm;
			break;
		case 22: // addiw
			ALUout = ext_signed((int)(reg[Rs1] + Imm), 1, 32);
			#ifdef DEBUG
			printf("Addiw %d res1 %d imm %d %d !!!!!!!!\n", Rs1, reg[Rs1], Imm, ALUout);
			#endif
			break;
		case 23: // jalr
			ALUout = temp_PC; // to R[rd]
			temp_PC = reg[Rs1] + Imm; // already extend 0
			break;
		case 24: // sb
			ALUout = reg[Rs1] + Imm;
			break;
		case 25: // sh
			ALUout = reg[Rs1] + Imm;
			break;
		case 26: // sw
			ALUout = reg[Rs1] + Imm;
			break;
		case 27: // sd
			ALUout = reg[Rs1] + Imm;
			break;
		case 28: { // sb type
			long long a, b;
			a = (long long)reg[Rs1], b = (long long)reg[Rs2];
			if(Branch == 2 && a == b) // beq
				temp_PC = temp_PC + Imm - 4;
			else if(Branch == 3 && a != b) // bne
				temp_PC = temp_PC + Imm - 4;
			else if(Branch == 4 && a < b) // blt
				temp_PC = temp_PC + Imm - 4;
			else if(Branch == 5 && a >= b) {// bge
				temp_PC = temp_PC + Imm - 4;
			#ifdef DEBUG
				printf("PC %d ", temp_PC);
			#endif
				cao_ = true;
			}
			break;
		}
		case 29: // auipc
			ALUout = temp_PC - 4 + Imm;
			break;
		case 30: // lui
			ALUout = temp_PC;
			break;
		case 31: // jal
			ALUout = temp_PC;
			temp_PC = temp_PC - 4 + Imm;
			break;			
		default:
			ALUout = 0;
	}
	//choose reg dst address
	// int Reg_Dst;
	// if(RegDst)
	// {

	// }
	// else
	// {

	// }
#ifdef DEBUG
reg_Output();
#endif
	//write EX_MEM_old
	EX_MEM.ALU_out=ALUout;
	EX_MEM.PC=temp_PC;
    EX_MEM.Reg_Dst = Rd;
	EX_MEM.Zero = Zero;
	EX_MEM.ALU_op = ALUOp;
	EX_MEM.Reg_Rt = Rs2;

	EX_MEM.Ctrl_M_Branch = ID_EX.Ctrl_M_Branch;
	EX_MEM.Ctrl_M_MemWrite = ID_EX.Ctrl_M_MemWrite;
	EX_MEM.Ctrl_M_MemRead = ID_EX.Ctrl_M_MemRead;

	EX_MEM.Ctrl_WB_RegWrite = ID_EX.Ctrl_WB_RegWrite;
	EX_MEM.Ctrl_WB_MemtoReg = ID_EX.Ctrl_WB_MemtoReg;
}

//访问存储器
void MEM()
{
	//read EX_MEM
	//complete Branch instruction PC change
	PC = EX_MEM.PC;
	int Reg_Dst = EX_MEM.Reg_Dst;
	REG ALUout = EX_MEM.ALU_out;
	int ALUop = EX_MEM.ALU_op;
	int Zero = EX_MEM.Zero;
	int Reg_Rs2 = EX_MEM.Reg_Rt;

	int Branch = EX_MEM.Ctrl_M_Branch;
	int MemWrite = EX_MEM.Ctrl_M_MemWrite;
	int MemRead = EX_MEM.Ctrl_M_MemRead;

	int RegWrite = EX_MEM.Ctrl_WB_RegWrite;
	int MemtoReg = EX_MEM.Ctrl_WB_MemtoReg;
	unsigned long long Mem_read = 0;
	switch(MemRead) {
		case 1: {// lb 
			char tmp; 
			memcpy(&tmp, &memory[ALUout], 1);
			Mem_read = ext_signed((int)tmp, 1, 8);
			break;}
		case 2: {// lh
			short tmp;
			memcpy(&tmp, &memory[ALUout], 2);
			Mem_read = ext_signed((int)tmp, 1, 16);
			break;}
		case 3: {// lw
			int tmp;
			memcpy(&tmp, &memory[ALUout], 4);
			Mem_read = ext_signed(tmp, 1, 32);
			break;}
		case 4:{// ld
			long long tmp;
			memcpy(&tmp, &memory[ALUout], 8);
			Mem_read = tmp;
			break;}
	}
		
	if (MemWrite) {
		switch(ALUop) {
			case 24: {// sb
				unsigned char tmp = (unsigned char)reg[Reg_Rs2];
				memcpy(&memory[ALUout], &tmp, 1);
				break;}
			case 25: {// sh
				short tmp = (short)reg[Reg_Rs2];
				memcpy(&memory[ALUout], &tmp, 2);
				break;}
			case 26: {// sw
				unsigned int tmp = (unsigned int)reg[Reg_Rs2];
				memcpy(&memory[ALUout], &tmp, 4);
				break;}
			case 27: {// sd
				unsigned long long tmp = (unsigned long long)reg[Reg_Rs2];
				memcpy(&memory[ALUout], &tmp, 8);
				break;}
		}
	}
	//write MEM_WB_old
	MEM_WB.Mem_read = Mem_read;
	MEM_WB.ALU_out = ALUout;
	MEM_WB.Reg_Dst = Reg_Dst;

	MEM_WB.Ctrl_WB_RegWrite = RegWrite;
	MEM_WB.Ctrl_WB_MemtoReg = MemtoReg;

}

//写回
void WB()
{
	//read MEM_WB
	unsigned long long Mem_read = MEM_WB.Mem_read;
	REG ALUout = MEM_WB.ALU_out;
	int Reg_Dst = MEM_WB.Reg_Dst;

	int RegWrite = MEM_WB.Ctrl_WB_RegWrite;
	int MemtoReg = MEM_WB.Ctrl_WB_MemtoReg;

	if (RegWrite == 1)
		reg[Reg_Dst] = ALUout;
	if (MemtoReg) {
		#ifdef DEBUG
		printf("\nMemtoReg %d %lx\n", Reg_Dst, Mem_read);
		#endif
		reg[Reg_Dst] = Mem_read;
	}

#ifdef DEBUG
		reg_Output();
		// exit(1);
#endif
}