#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctime>
using namespace std;

/*Instruction*/
class instruction {
public:
	bitset<32> instr;//instruction
	instruction(bitset<32> fetch); // constructor

};

/*CPU*/
class CPU {
	public:

	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc 
	long reg_in1;
	long reg_in2;
	long cntrl_in;
	long reg_wr;
	

	CPU();
	unsigned long readPC();
	bitset<32> Fetch(bitset<8> *instmem);
	bool Decode(instruction* instr);
	/*helper*/
	void printMem();
	
};

/*Register File*/
class RegFile
{
	
	public:
		long x[32]; //32 registers
		long rs1; // source reg 1
		long rs2; //source reg 2
		long rd; //dest reg 
		long data; //data to write


		RegFile();
		void readReg (unsigned long, unsigned long); //read values of rs1, rs2
		long getReg1 ();
		long getReg2 ();
		void writeReg (); 	// Set rd to the writeData
		void writeData (long); // Data to write ?? maybe not needed


		/*Tests*/
		void printReg();

};



/*Immidiate generator*/
class ImmGen{
	/*construct for s type, i type, b type*/
	public:
		//long immediate;
		long immediate;

		ImmGen();
		void set_imm_i(instruction*);
		void set_imm_s(instruction*);
		void set_imm_b(instruction*);

	/*helper*/
		void printImm();

};

/*Controller*/
class controller{
	public:
		/*control unit signals */
		
		unsigned long op;
		bool Jump;
		bool Branch;
		bool MemRead;
		bool MemtoReg;
		bool ALUop;
		bool MemWrite;
		bool ALUSrc;
		bool RegWrite;

		controller();
		void setFlags(RegFile*, CPU*, ImmGen*, instruction*); // set the flags

		/*helpers*/
		void printFlags();

};

/*ALU*/
class ALU 
{

	/*ALUop:
		ADD/ ADDI -> 0010 (2)
		SUB -> 0110 (6)
		AND/ ANDI -> 0000 (0)
		XOR -> 1110	(14)
		SRA -> 1010 (10)
	*/

	public:
	bool Negative = 0;
	long ALUres;

	void Execute(controller*, RegFile*, ImmGen*); // do the execution based on the opcode
	long ADD(long, long); //R-type
	long SUB(long, long); //R-type
	long ANDI(long, long); //I-type
	long SRA(long, long); //R-type
	long XOR(long, long); //R-type
	/*helpers*/
	void printOps();
};


/*Data Memory*/
class DataMem {
	public:

	void memReadWrite(controller*, ALU*, RegFile*, CPU*);	//read from or wirte to memory
	int Read(int*); //Read the memory
	void Write(int*, int); //Write to mmemory
};

/*Update cpu*/
void updatePC(CPU*, ALU*, controller*, ImmGen*, RegFile*);