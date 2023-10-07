#include "CPU.h"

instruction::instruction(bitset<32> fetch)
{
	instr = fetch;
}

CPU::CPU()
{
	PC = 0; //set PC to 0
	for (int i = 0; i < 4096; i++) //copy instrMEM
	{
		dmemory[i] = (0);
	}
}

bitset<32> CPU::Fetch(bitset<8> *instmem) {
	bitset<32> instr = ((((instmem[PC + 3].to_ulong()) << 24)) + ((instmem[PC + 2].to_ulong()) << 16) + 
			((instmem[PC + 1].to_ulong()) << 8) + (instmem[PC + 0].to_ulong()));  //get 32 bit instruction
	return instr;
	
}


bool CPU::Decode(instruction* curr)
{
	long maskOp = 127; // mask for opcode
	long maskSrc = 31; //mask for register sources

	if (!(curr->instr).to_ulong()){
		return false;
	}

	cntrl_in = curr->instr.to_ulong() & maskOp;
 	reg_in1 = (curr->instr.to_ulong() >> 15) & maskSrc;
	reg_in2 = (curr->instr.to_ulong() >> 20) & maskSrc;
	reg_wr = (curr->instr.to_ulong() >> 7) & maskSrc; 
	return true;
}

unsigned long CPU::readPC()
{
	return PC;
}

void CPU:: printMem(){
	for (int i = 0; i < 100; i++){
		cout << dmemory[i] << endl; 
	}
}

/*RegFile*/

RegFile:: RegFile(){

	srand(time(NULL));
	x[0] = 0; //set x0 to 0

	for(int i = 1; i < 32; i++){ //initialize the registers with random values
		x[i] = ((long long)(rand()));
	} 
}

void RegFile:: printReg() {
	for(int i = 0; i < 32; i++) 
		cout << x[i] << endl;
}

void RegFile::readReg (unsigned long r1, unsigned long r2 = 0){

	rs1 = x[r1];
	rs2 = x[r2];
}

long RegFile:: getReg1(){return rs1;} 
long RegFile:: getReg2(){return rs2;}

void RegFile:: writeData(long d){ data = d;}
void RegFile:: writeReg (){
	x[rd] = data; //write the data in register
} 

/*ALU*/
void ALU:: Execute(controller* cntrl, RegFile* reg, ImmGen* imm){
	unsigned long op = cntrl->op;
	long src1, src2;

	
	// select the operation
	if (op == 0) {
		src1 = reg->rs1;
		src2 = imm->immediate;

		ALUres = ANDI(src1, src2);
		reg->writeData(ALUres);
		reg->writeReg();
	}
	else if(op == 2) {	
		src1 = reg->rs1;
		if (cntrl->ALUSrc == 0){	//check the operation type R/I
			src2 = reg->rs2;		//R-type
		}
		else if (cntrl->ALUSrc == 1){
			src2 = imm->immediate;	//I-type
		}
		
		ALUres = ADD(src1, src2);

		if(cntrl->RegWrite){
			reg->writeData(ALUres);	//write the result in data
			reg->writeReg();	//write the result to rd
		}
	}
	else if(op == 6) {	
		src1 = reg->rs1;
		src2 = reg->rs2;

		ALUres = SUB(src1, src2);

		if(cntrl->Branch == 1){	//set the flag for branch
			Negative = (ALUres < 0);
		}
		if(cntrl->RegWrite){	//if not branch write the result to rd
			reg->writeData(ALUres);	//write the result in data
			reg->writeReg();	//write the result to rd
		}

		
	}
	else if(op == 14) {
		src1 = reg->rs1;
		src2 = reg->rs2;

		ALUres = XOR(src1, src2);

		reg->writeData(ALUres);	//write the result in data
		reg->writeReg();	//write the result to rd
	}
	else if (op == 10) {
		src1 = reg->rs1;
		src2 = reg->rs2;

		ALUres = SRA(src1, src2);

		reg->writeData(ALUres);	//write the result in data
		reg->writeReg();	//write the result to rd
	}

} 
 
long ALU:: ADD (long src1, long src2){ 
	
	return (src1 + src2);
}

long ALU:: SUB (long src1, long src2){ 
	
	long res = src1 - src2;
	return res;
}

long ALU:: ANDI (long src1, long src2){
	return (src1 & src2);
}

long ALU:: XOR (long src1, long src2){ 
	
	return (src1 ^ src2);
}

long ALU::SRA (long src1, long src2){
	return (src1 >> src2);
}


/*Immediate generator*/
ImmGen::ImmGen(){immediate = 0;} //set immediate to zero

void ImmGen:: set_imm_i (instruction* inst){
	//immediate for i type instructions
	immediate = int(inst->instr.to_ulong()) >> 20;		
}

void ImmGen::set_imm_s (instruction* inst){
	long lInst = inst->instr.to_ulong();
	
	long imm_11_5 = long(lInst & 0xfe000000); // imm 11_5
	
	long imm_4_0 = long((lInst & 0xf80) << 13); //imm 4_0

	immediate = (imm_11_5 + imm_4_0) >> 20;
}

void ImmGen::set_imm_b (instruction* inst){
	
	long lInst = inst->instr.to_ulong();

	long imm_11 = long(lInst & 0x80000000); //imm 12
	long imm_10 = long((lInst & 0x80) << 23); //imm 11
	long imm_9_4 = long((lInst & 0x7e000000) >> 1); //imm 9-4
	long imm_3_0 = long((lInst & 0xf00) << 12); //imm 3_0

	immediate = long(
		(imm_11 + imm_10 + imm_9_4 + imm_3_0) << 1
	) >> 20;  //immediate for B-type

}

void ImmGen:: printImm(){
	bitset<12> binImm;
	
	binImm = immediate;
	cout << "The immediate is: " << immediate << endl;
}

/*Controller*/
controller:: controller(){	//set the flags to zero
		Jump = 0;
		Branch = 0;
		MemRead = 0;
		MemtoReg = 0;
		ALUop = 0;
		MemWrite = 0;
		ALUSrc = 0;
		RegWrite = 0;
}
void controller:: setFlags (RegFile* regf, CPU* cpu, ImmGen* imm, instruction* inst){

	long cntrl = cpu->cntrl_in; //controller input
	unsigned long lInst = inst->instr.to_ulong();
	int mask3 = 7; //mask for func3
	int func3, func7;
	
	if(cntrl == 51 ) {		//R-type
		RegWrite = 1; ALUSrc = 0;
		Branch = 0; MemtoReg = 0;
		MemRead = 0; MemWrite = 0;
		Jump = 0;
		
		func3 = (lInst >> 12) & mask3;	
		func7 = lInst >> 25;

		if(func3 == 0 && func7 == 0) {
			op = 2;	//ADD
			
		}
		else if (func3 == 7 && func7 == 0){
			op = 0; //AND
		}
		else if (func3 == 0 && func7 == 32) op = 6; //SUB
		else if (func3 == 4 && func7 == 0) op = 14; //XOR
		else if (func3 == 5 && func7 == 32) op = 10; //SRA

		
		regf->readReg(cpu->reg_in1, cpu->reg_in2); //read values of rs1 and rs2
		regf->rd = cpu->reg_wr; 	//set rd to decoded value

	}
	else if(cntrl == 3){	// Load instruction
		RegWrite = 1; ALUSrc = 1;
		Branch = 0; MemtoReg = 1;
		MemRead = 1; MemWrite = 0;
		Jump = 0;
		
		imm->set_imm_i(inst);	//build the immediate of I-type
		regf->readReg(cpu->reg_in1);
		regf->rd = cpu->reg_wr;

	}

	else if(cntrl == 19 ) {		//I-type
		RegWrite = 1; ALUSrc = 1;
		Branch = 0; MemtoReg = 0;
		MemRead = 0; MemWrite = 0;
		Jump = 0;
		
		func3 = (lInst >> 12) & mask3;	

		if(func3 == 0 ) {
			op = 2;	//ADDI
		
		}
		else if (func3 == 7) {
			op = 0; //ANDI
		}
		imm->set_imm_i(inst);	//build the immediate of I-type
		regf->readReg(cpu->reg_in1);
		regf->rd = cpu->reg_wr;
	}

	else if(cntrl == 35){		//S-type
		RegWrite = 0; ALUSrc = 1;
		Branch = 0; MemtoReg = 0;
		MemRead = 0; MemWrite = 1;
		Jump = 0;
		
		op = 2;	//ADD
	
		imm->set_imm_s(inst);	//build the immediate or S-type
		regf->readReg(cpu->reg_in1, cpu->reg_in2);
	}
	
	else if(cntrl == 99){		//BLT
		RegWrite = 0; ALUSrc = 0;
		Branch = 1; MemtoReg = 0;
		MemRead = 0; MemWrite = 0;
		Jump = 0;
		
		op = 6; //SUB
		imm->set_imm_b(inst);	//build the immediate or B-type
		regf->readReg(cpu->reg_in1, cpu->reg_in2);
	}
	else if(cntrl == 103){		//JALR/ I-Type
		RegWrite = 1; ALUSrc = 1; 
		Branch = 0; MemtoReg = 0;
		MemRead = 0; MemWrite = 0;
		Jump = 1; 

		op = 2; //ADD

		imm->set_imm_i(inst);	//build immediate for i-type 
		
		regf->readReg(cpu->reg_in1);	//read rs1
		regf->rd = cpu->reg_wr;		//set rd to decoded value
		regf->writeData(cpu->PC + 4);	//save PC+4
		regf->writeReg();	//write the data in rd
	}

	else return;

}

void controller::printFlags(){
	cout << "RegWrite " << RegWrite << "\n" 
		<<" ALUSrc " << ALUSrc << " \n" 
		<< "Branch " << Branch << " \n" 
		<<"MemtoReg " << MemtoReg << " \n" 
		<< "MemRead " << MemRead << " \n" 
		<< "MemWrite " << MemWrite <<" \n"
		<< "Jump " << Jump << " \n";
}

/*Data Memory*/
void DataMem:: memReadWrite(controller* cntrl, ALU* alu, RegFile* reg, CPU* cpu){
	unsigned long addr; //address
	if(cntrl->MemRead && cntrl->MemtoReg){	//Load instruction
		addr = alu->ALUres;
		
		reg->writeData(cpu->dmemory[addr]);		//retrieve the data from memory
		reg->writeReg();	//write the data to rd
	}
	else if(cntrl->MemWrite) {	//save instruction
		addr = alu->ALUres;
		cpu->dmemory[addr] = reg->rs2;	//assign the value of rs2 to address
	}
}

/*Updade PC*/
void updatePC(CPU* cpu, ALU* alu, controller* cntrl, ImmGen* imm, RegFile* reg){
	if(alu->Negative && cntrl->Branch){	//if branch is taken
		 
		cpu->PC = cpu->PC + imm->immediate;	//set PC to label
		 
	}
	else if(cntrl->Jump){	// if the instruction is JALR
		unsigned long savedPC = cpu->PC + 4;	//save PC+4
		
		
		cpu->PC = reg->x[reg->rd];	//Jump to rd = rs1 + imm
		reg->x[reg->rd] = savedPC; //save the PC+4 in the rd
		
	}
	else{
		
		cpu->PC += 4;	//not jump/ branch
	}
}
