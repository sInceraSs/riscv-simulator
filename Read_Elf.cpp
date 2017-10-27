#include"Read_Elf.h"

FILE *elf=NULL;
Elf64_Ehdr elf64_hdr;

//代码段的长度
unsigned int csize=0;
unsigned int cadr=0;

//代码段在内存中的虚拟地址
unsigned int vadr=0;

//全局数据段在内存的地址
unsigned long long gp=0;

//main函数在内存中地址
unsigned int madr=0;
unsigned int msize = 0;

//程序结束时的PC
unsigned int endPC=0;

//程序的入口地址
unsigned int entry=0;

FILE *file=NULL;

unsigned int padr=0;
unsigned int psize=0;
unsigned int pnum=0;

//Section Headers
unsigned int sadr=0;
unsigned int ssize=0;
unsigned int snum=0;

//Symbol table
unsigned int symnum=0;
unsigned int symadr=0;
unsigned int symsize=0;

//unsigned int index=0;

unsigned int stradr=0;
unsigned int shstrndx = 0;
unsigned int symstradr = 0;

bool open_file()
{	
	elf = fopen("elfres.txt","w");
	if(!elf)
		return false;
	return true;
}

void read_elf()
{
	if(!open_file())
		return ;

	fprintf(elf,"ELF Header:\n");
	read_Elf_header();

	fprintf(elf,"\n\nSection Headers:\n");
	read_elf_sections();

	fprintf(elf,"\n\nProgram Headers:\n");
	read_Phdr();

	fprintf(elf,"\n\nSymbol table:\n");
	read_symtable();

	fclose(elf);
}

void read_Elf_header(){
	//file should be relocated
	fread(&elf64_hdr,1,sizeof(elf64_hdr),file);

	pnum = elf64_hdr.e_phnum;
	psize = elf64_hdr.e_phentsize;
	padr = elf64_hdr.e_phoff;

	snum = elf64_hdr.e_shnum;
	ssize = elf64_hdr.e_shentsize;
	sadr = elf64_hdr.e_shoff;

	entry = (unsigned int)elf64_hdr.e_entry;
	shstrndx = elf64_hdr.e_shstrndx;

	fprintf(elf," magic number:  ");
	for(int i = 0;i<16;i++){
		fprintf(elf, "%2x ", elf64_hdr.e_ident[i]);
	}
	fprintf(elf,"\n");

	switch(elf64_hdr.e_ident[4]){
		case 0:
			fprintf(elf," Class:  ILLEGAL ELFCLASS\n");
			break;
		case 1:
			fprintf(elf," Class:  ELFCLASS32\n");
			break;
		case 2:
			fprintf(elf," Class:  ELFCLASS64\n");
			break;
		default: break;
	}
	
	switch(elf64_hdr.e_ident[5]){
		case 0:
			fprintf(elf," Data:  ILLEGAL\n");
			break;
		case 1:
			fprintf(elf," Data:  little-endian\n");
			break;
		case 2:
			fprintf(elf," Data:  big-endian\n");
			break;
		default: break;
	}
		
	fprintf(elf," Version:  %d current\n",elf64_hdr.e_ident[EI_VERSION]);

	fprintf(elf," OS/ABI:	 System V ABI\n");
	
	fprintf(elf," ABI Version:  %d\n",elf64_hdr.e_ident[EI_OSABI]);
	
	fprintf(elf," Type:  %x",elf64_hdr.e_type);

	fprintf(elf," Machine:  %x\n",elf64_hdr.e_machine);

	fprintf(elf," Version:  %x\n",elf64_hdr.e_version);
	
	fprintf(elf," Entry point address:  0x%x\n",elf64_hdr.e_entry);

	fprintf(elf," Start of program headers:  %d  bytes into  file\n",elf64_hdr.e_phoff);

	fprintf(elf," Start of section headers:  %d  bytes into  file\n",elf64_hdr.e_shoff);

	fprintf(elf," Flags:  0x%x\n",elf64_hdr.e_flags);

	fprintf(elf," Size of this header:  %d Bytes\n",elf64_hdr.e_ehsize);

	fprintf(elf," Size of program headers:  %d Bytes\n",elf64_hdr.e_phentsize);

	fprintf(elf," Number of program headers:  %d \n",elf64_hdr.e_phnum);

	fprintf(elf," Size of section headers:  %d  Bytes\n",elf64_hdr.e_shentsize);

	fprintf(elf," Number of section headers: %d \n",elf64_hdr.e_shnum);

	fprintf(elf," Section header string table index:  %d \n",elf64_hdr.e_shstrndx);
}
	
void read_elf_sections() {
	Elf64_Shdr elf64_shdr;
	fseek(file, sadr + elf64_hdr.e_shstrndx * sizeof(Elf64_Shdr), 0);
	fread(&elf64_shdr, 1, sizeof(elf64_shdr), file);
	fseek(file, elf64_shdr.sh_offset, 0);
	unsigned char name[10000];
	// symbol table all in 
	fread(name, 1, elf64_shdr.sh_size, file);
	fseek(file, sadr, 0);
	for (int c = 0; c < snum; c++)	{
		fprintf(elf, " [%3d]\n", c);
		fread(&elf64_shdr, 1, sizeof(elf64_shdr), file);
		fprintf(elf, " Name: ");
		fprintf(elf, "%-15s", name + elf64_shdr.sh_name);
		
		if (!strcmp((const char*)(name + elf64_shdr.sh_name), ".symtab")) {
			symadr = elf64_shdr.sh_offset;
			symsize = elf64_shdr.sh_entsize;
			symnum=(elf64_shdr.sh_size)/symsize;
		} else if (!strcmp((const char*)(name + elf64_shdr.sh_name), ".text")) {
			cadr = elf64_shdr.sh_offset;
			csize = elf64_shdr.sh_size;
		} else if (!strcmp((const char*)(name + elf64_shdr.sh_name), ".strtab")) {
			symstradr = elf64_shdr.sh_offset;
		}

		fprintf(elf, " Type: ");

		fprintf(elf, " Address:  %x", elf64_shdr.sh_addr);

		fprintf(elf, " Offest:  %d", elf64_shdr.sh_offset);

		fprintf(elf, " Size:  %d", elf64_shdr.sh_size);

		fprintf(elf, " Entsize:  %d", elf64_shdr.sh_entsize);

		fprintf(elf, " Flags:   ");

		fprintf(elf, " Link:  %d", elf64_shdr.sh_link);

		fprintf(elf, " Info:  %d", elf64_shdr.sh_info);

		fprintf(elf, " Align: %d\n", elf64_shdr.sh_addralign);

	}
}

void read_Phdr()
{
	Elf64_Phdr elf64_phdr;

	for(int c=0;c<pnum;c++)	{
		fprintf(elf," [%3d]\n",c);
		fseek(file, padr + c*psize, SEEK_SET);
			
		//file should be relocated
		fread(&elf64_phdr,1,sizeof(elf64_phdr),file);

		fprintf(elf," Type:  %d ",elf64_phdr.p_type);
		
		fprintf(elf," Flags:  %d ",elf64_phdr.p_flags);
		
		fprintf(elf," Offset:  %d ",elf64_phdr.p_offset);
		
		fprintf(elf," VirtAddr: %x ",elf64_phdr.p_vaddr);
		
		fprintf(elf," PhysAddr:  %x ",elf64_phdr.p_paddr);

		fprintf(elf," FileSiz:  %d ",elf64_phdr.p_filesz);

		fprintf(elf," MemSiz:  %d ",elf64_phdr.p_memsz);
		
		fprintf(elf," Align:  %d \n",elf64_phdr.p_align);
	}
}


void read_symtable()
{
	Elf64_Sym elf64_sym;
	char name_i;
	int name_idx;
	for(int c=0;c<symnum;c++)	{
		fprintf(elf," [%3d]   ",c);
		fseek(file, symadr + c*symsize, SEEK_SET);
		//file should be relocated
		fread(&elf64_sym,1,sizeof(elf64_sym),file);
		//read section name
		fseek(file, elf64_sym.st_name + symstradr, SEEK_SET);
		char name[40];
		name_i = 's', name_idx = 0;
		while(name_i != '\0') {
			fread(&name_i,1,1,file);
			name[name_idx++]=name_i;
		}

		if(strcmp(name,"main")==0) {
			madr = elf64_sym.st_value;
			msize = elf64_sym.st_size;
		}
		else if(strcmp(name,"__global_pointer$")==0)
			gp = elf64_sym.st_value;

		fprintf(elf," Name:  %40s   ",name);

		fprintf(elf," Bind:  %d ",elf64_sym.st_info);
		
		fprintf(elf," Type:  %d ",elf64_sym.st_other);
		
		fprintf(elf," NDX:   %d",elf64_sym.st_shndx);
		
		fprintf(elf," Size:  %d ",elf64_sym.st_size);

		fprintf(elf," Value:  %d \n",elf64_sym.st_value);

	}
}


