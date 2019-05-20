#include "6502.h"
#include "nes_file_read.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signalHandler(int sigint){
	puts("\nERROR: SEGFAULT\n");
	puts("REGISTER DUMP");
	printf("OPCODE: %x, A: %x, X: %x, Y: %x, P: %x\n",opcode, A, X, Y, P);
	printf("Program Counter: %x, Stack Pointer: %x\n", PC, SP);
	printf("DEBUG INFO: Variables - address: %x, opcode: %x, operand: %x, cycles: %d\n\n",address, opcode, operand,cycles); 
	
	printf("Interrupt Vector: %x %x\n\n", mem[0xFFFE], mem[0xFFFF]);
	
	puts("ZERO PAGE DUMP");
	    for (int i = 0x0; i <= 0xFF; i += 0x10){
                printf("0x%02x - 0x%02x: ",i, i + 0xF);
                for (int c = 0x0; c < 16; c++){
                        printf("%02x ", mem[c+i]);
                }
                puts(" ");
}

	
	puts("STACK DUMP");
	for (int i = 0x0; i <= 0xFF; i += 0x10){
		printf("0x1%02x - 0x1%02x: ",i, i + 0xF);
		for (int c = 0x0; c < 16; c++){
			printf("%02x ", mem[0x100+c+i]);
		}
		puts(" ");
}
	int index = 0;
	exit(sigint);
}
int main(int argc, char* argv[]){
signal(SIGSEGV, signalHandler);
signal(SIGINT, signalHandler);
puts("START");
loadINESFile(argv[1]);

void (*opcodes[4][8])() = {{NULL, BIT, JMP, NULL, STY, LDY, CPY, CPX},
			   {ORA, AND, EOR, ADC, STA, LDA, CMP, SBC},
			   {ASL, ROL, LSR, ROR, STX, LDX, DEC, INC},
			   {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

void (*singleByte0[16])() = {BRK, BRANCH, JSR, BRANCH, RTI, BRANCH, RTS, BRANCH, NULL, BRANCH, NULL, BRANCH, NULL, BRANCH, NULL, BRANCH};
void (*singleByte8[16])() = {PHP, CLC, PLP, SEC, PHA, CLI, PLA, SEI, DEY, TYA, TAY, CLV, INY, CLD, INX, SED};
void (*singleByteA[7])() = {TXA, TXS, TAX, TSX, DEX, NULL, NOP};
while (1){
cycles = 400;
while (cycles >= 0){

loadOpcode();
if (argc == 3){
	if (PC - 1 == strtol(argv[2],NULL, 16)){
	signalHandler(0);
}
}
printf("OPCODE: %2x, A: %2x, X: %2x, Y: %2x, P: %2x            PC = %4x, SP = %2x | ",opcode, A, X, Y, P,PC - 1, SP);
int test = (opcode & 0xF0) >> 4;
test -= 8;
if ((opcode & 0x0F) == 0x0 && singleByte0[(opcode & 0xF0)>> 4]){
        singleByte0[(opcode & 0xF0)>> 4]();
}
else if ((opcode & 0x0F) == 0x8){
        singleByte8[(opcode & 0xF0) >> 4]();
}
else if ((opcode & 0x0F) == 0xa && test >= 0){
        singleByteA[((opcode & 0xF0) >> 4) - 8]();
}
else if (opcodes[cc][aaa] != NULL && addressingMode[cc][bbb] != NULL){
        opcodes[cc][aaa]();
}
else{
	printf(" Unrecognized OPCODE: %x\n",opcode);
}
puts("");
}
sleep(1);
}
}
