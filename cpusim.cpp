#include "CPU.h"

#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
#include<fstream>
#include <sstream>
using namespace std;

int main(int argc, char* argv[])
{
	bitset<8> instMem[4096];
	if (argc < 2) {
		return -1;
	}

	ifstream infile(argv[1]); //open the file
	if (!(infile.is_open() && infile.good())) {
		cout<<"error opening file\n";
		return 0; 
	}
	string line; 
	int i = 0;
	while (infile) {
			infile>>line;
			stringstream line2(line);
			int x; 
			line2>>x;
			instMem[i] = bitset<8>(x);
			
			i++;
		}
	
	int maxPC= i; 
	CPU myCPU;  // call the approriate constructor here to initialize the processor...  

	RegFile regs; //instantiate registers
	

	ALU myAlu;	//instatiate ALU
	ImmGen myImm;	//instantiate Immediate generator
	controller myCntlr;	//instantiate Contrller
	DataMem myMem;	//Instantiate Data memory
	

	bitset<32> curr;
	instruction instr = instruction(curr);
	bool done = true;

	/*report section*/
	int numCycles = 0;
	int numRtype = 0;

	while (done == true) // processor's main loop. Each iteration is equal to one clock cycle.  
	{
		//fetch
		curr = myCPU.Fetch(instMem); // fetching the instruction
		
		instr = instruction(curr);
		// decode
		done = myCPU.Decode(&instr);
		if (done == false) // break from loop so stats are not mistakenly updated
			break;

		if(myCPU.cntrl_in == 51) numRtype++;	//increment num of R-type instructions

		regs.readReg(myCPU.reg_in1, myCPU.reg_in2);
		myCntlr.setFlags(&regs, &myCPU, &myImm, &instr);

		myAlu.Execute(&myCntlr, &regs, &myImm);
		myMem.memReadWrite(&myCntlr, &myAlu, &regs, &myCPU);
		

		updatePC(&myCPU, &myAlu, &myCntlr, &myImm, &regs); //update pc
		// sanity check
		if (myCPU.readPC() > maxPC)
			break;

		numCycles++; //increment the num cycles
	}
	int a0 = regs.x[10];
	int a1 = regs.x[11]; 
	
	  cout << "(" << a0 << "," << a1 << ")" << endl;
	
	return 0;

}