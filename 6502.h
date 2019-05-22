#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define C 0
#define Z 1
#define I 2
#define D 3
#define B 4
#define B2 5
#define V 6
#define N 7
int time = 100000;

uint8_t A;
uint8_t X, Y;

uint8_t P = 0x24;

uint16_t PC;
uint8_t SP = 0xFD;

uint8_t mem[0xFFFFFF];

uint8_t opcode, operand;
uint8_t aaa, bbb, cc;
uint8_t lo, hi;

uint16_t address;
uint8_t* target;
uint8_t acc;
uint8_t modify = 0x0;

int32_t cycles_count = 0;
/* Atomic Cycles */ 
inline static void loadOpcode(){
	opcode = mem[PC];
	PC++;
	aaa = (opcode & 0b11100000) >> 5;
	bbb = (opcode & 0b00011100) >> 2;
	cc =  (opcode & 0b00000011);
	cycles_count++;
}
inline static void loadOperand(){
	//printf("LOAD OPERAND | ");
	operand = mem[PC];
	PC++;
	cycles_count++;
}

inline static void writeAddress(){
	operand = mem[address];
	address = (hi << 8) | lo;
	//printf("READ ADDRESS | Address: %x\n",address);
}

uint8_t getFlag(uint8_t FLAG){return (P >> FLAG ) & 1;}
inline static void setFlag(uint8_t FLAG){P |= (1 << FLAG);}
inline static void clearFlag(uint8_t FLAG){P &= ~(1 << FLAG);}
inline static void setNZ(){
	if (operand == 0x0){
		setFlag(Z);
	}else {clearFlag(Z);};
	if (((operand >> 7) & 1) == 1){
		setFlag(N);
	} else { clearFlag(N);};
}

/* Addressing modes */
void immediateAddress(){                    // 1 cycle
	loadOperand();			    // Cycle 1: Load next byte
	printf(" Immediate Addressing |");
}

void accumulatorMode(){
	printf(" Accumulator Mode");
	operand = A; target = &A;
	cycles_count++;
}
void zeroPageAddressRW(){              
	printf(" Zero Page Address |");     // 2 cycles
	loadOperand();                      // Cycle 1: Load next byte
	printf(" Operand: %x", operand);
	address = operand & 0xFF;                 // Cycle 2: Fetch zero Address
	cycles_count++;
	operand = mem[address]; target = mem+address;
	if (modify){cycles_count++;}
}
void zeroPageAddressY();
void zeroPageAddressX(){                    // 3 cycles
	if (( aaa == 0b100 || aaa == 0b101 ) && cc == 0b10){
		zeroPageAddressY();
		return;
	}
	printf(" Zero Page Address X");
	loadOperand();			    // Cycle 1: Load next byte
	                		    // Cycle 2: Fetch Zero Address
	address += X;			    // Cycle 3: Get Address, add Index register
	address &= 0x00FF;		    // Cycle 4: Get Effective Address
	cycles_count += 2;
	operand = mem[address]; target = mem+address;
	if (modify){cycles_count++;}
}

void zeroPageAddressY(){               
	printf(" Zero Page Address Y");     // 3 cycles
        loadOperand();                      // Cycle 1: Load next byte
                                            // Cycle 2: Fetch Zero Address
        address += Y;                       // Cycle 3: Get Address, add Index register
        address &= 0x00FF;                  // Cycle 4: Get Effective Address
	cycles_count += 2;
	operand = mem[address]; target = mem+address;
	if (modify){cycles_count++;}
}

void absoluteAddressRW(){                   // 3 cycles
	printf(" Absolute Addressing");
	loadOperand();lo = operand;         // Cycle 1: Fetch high byte of PC
	loadOperand();hi = operand;         // Cycle 2: Fetch low byte of PC
	writeAddress();                     // Cycle 3: Fetch Address
	operand = mem[address]; target = mem+address;
	cycles_count++;
	if (modify){cycles_count++;}
}
void absoluteAddressY();
void absoluteAddressX(){
	  if ((aaa = 0b101) && cc == 0b10){
                absoluteAddressY();
                return;
        }
                    											     // 3 + 1* cycles
	printf(" Absolute Addressing X");
	loadOperand();lo = operand;         											     // Cycle 1: Fetch low byte of PC
        loadOperand();hi = operand;												     // Cycle 2: Fetch high byte of PC
	address = hi << 8; // ; puts("FETCH ADDRESS");							     	     // Cycle 3: Write high byte
	cycles_count++;
	if ( (uint16_t) (lo + X) <= 0xFF){address = hi << 8 |lo;}              
	else { address = ((hi + 0x1) << 8) | ((lo + X) & 0xFF); puts("RECALCULATE ADDRESS"); cycles_count++;}				     // Cycle 4*: Will happen if page boundary is exceeded
	operand = mem[address]; target = mem+address;
	if (modify){cycles_count++;}
}

void absoluteAddressY(){
	printf(" Absolute Addressing Y");                                                                        // 3 + 1* cycles
        loadOperand();lo = operand;                                                             // Cycle 1: Fetch low byte of PC
        loadOperand();hi = operand;                                                             // Cycle 2: Fetch high byte of PC
        address = hi << 8; // puts("FETCH ADDRESS");;	
	cycles_count++;						// Cycle 3: Write high byte
	if ( (uint16_t) (lo + Y) <= 0xFF){  address = hi << 8 | lo;}               
        else { address = ((hi + 0x1) << 8) | ((lo + Y) & 0xFF) ; puts("RECALCULATE ADDRESS"); cycles_count++;}        
	operand = mem[address]; target = mem+address;                       // Cycle 4*: Will happen if page boundary is exceeded
	if (modify){cycles_count++;}
}


void indexedIndirect(){
	printf(" Indexed Indirect"); 
	loadOperand(); address = operand & 0xFF;
	address += X;
	lo = mem[address];
	hi = mem[address+1];
	address = (hi << 8) | lo;
	cycles_count += 4;
	operand = mem[address]; target = mem+address;
	if (modify){cycles_count++;}
}

void indirectIndexed(){
	printf(" Indirect Indexed");
	loadOperand(); address = operand & 0xFF;
	lo = mem[address];
	hi = mem[address+1];
	if ( lo + Y > 0xFF ) { cycles_count -= 1; }
	address = (hi << 8) | lo;
	operand = mem[address]; target = mem+address;
	cycles_count += 2;
	if (modify){cycles_count++;}
}

void(* addressingMode[3][8])() = {{immediateAddress, zeroPageAddressRW, NULL, absoluteAddressRW, NULL, zeroPageAddressX, NULL, absoluteAddressX},
				  {indexedIndirect, zeroPageAddressRW, immediateAddress, absoluteAddressRW, indirectIndexed, zeroPageAddressX, absoluteAddressY, absoluteAddressX},
				  {immediateAddress, zeroPageAddressRW, accumulatorMode, absoluteAddressRW, indirectIndexed, zeroPageAddressX, NULL, absoluteAddressX}};

/* Stack Instructions */
void BRK(){							// Force Break{
loadOperand();
setFlag(B);
mem[SP+0x100] = PC >> 8; SP--;
mem[SP+0x100] = PC & 0x00FF; SP--;
mem[SP+0x100] = P; SP--;

PC = mem[0xFFFE]; puts("Write HI Address");
PC = (mem[0xFFFF] << 8) | PC; puts("Write LO Address");
cycles_count += 5;
}

void RTS(){
printf(" RTS");
loadOperand();
SP++;
lo = mem[0x100+SP]; SP++;
hi = mem[0x100+SP];
PC = (hi << 8) | lo;
PC++;
}

void RTI(){
printf(" RTI");
loadOperand();
SP++;
P = mem[0x100+SP]; SP++;
lo = mem[0x100+SP]; SP++;
hi = mem[0x100+SP];
address = hi << 8 | lo;
PC = address;
cycles_count += 4;
}

void BRANCH(){					// 3 + 1* + 1! cycles
	puts(" BRANCH");
	short xx = (opcode & 0b11000000) >> 6;  
	short y = (opcode & 0b00100000) >> 5;
	loadOperand();				// Cycle 1: Load Operand
	int8_t offset = operand;
	uint8_t flag = 0;
	cycles_count++;
	if (xx == 0){ flag = N;}
	else if (xx == 1){ flag = V;}
	else if (xx == 2){ flag = C;}
	else if (xx == 3){ flag = Z;}
	if (getFlag(flag) == y){
		cycles_count++;
		PC += operand;	
	}
}

void JMP(){
	loadOperand(); lo = operand;
	loadOperand(); hi = operand;  
	PC = hi << 8 | lo;
	printf(" PC: %d\n",PC);
}

void JMPindirect(){
	loadOperand(); lo = operand;
	loadOperand(); hi = operand;
	address = hi << 8 | lo;
	uint8_t addr2 = hi << 8 | ((lo + 1) & 0xFF);
	lo = mem[address];
	hi = mem[addr2];
	PC = hi << 8 | lo;
	cycles_count += 2;
}
void JSR(){
	printf(" JSR");
	loadOperand(); lo = operand;
	loadOperand(); hi = operand;
	PC--;
	mem[0x100+SP] = PC >> 8; SP--;
	mem[0x100+SP] = PC & 0x00FF; SP--;
	
	PC = (hi << 8) | (lo);
	cycles_count += 4;
}
/* Flags */
void CLD(){clearFlag(D); cycles_count++; printf(" CLEAR DECIMAL");}
void SED(){setFlag(D); cycles_count++; printf(" SET DECIMAL (non-functional)");}
void CLC(){clearFlag(C); cycles_count++; printf(" CLEAR CARRY");}
void SEC(){setFlag(C); cycles_count++; printf(" SET CARRY");}
void CLI(){clearFlag(I); cycles_count++; printf(" CLEAR INTERRUPT");}
void SEI(){setFlag(I); cycles_count++; printf(" SET INTERRUPT");}
void CLV(){clearFlag(V); cycles_count++; printf(" CLEAR OVERFLOW");}

/* LDx Instructions */
void LDA(){addressingMode[cc][bbb](); A = operand; setNZ(); printf(" Instruction: LDA");}			// LDA Immediate Addressing
void LDX(){addressingMode[cc][bbb](); X = operand; setNZ(); printf(" Instruction: LDX");}			// LDX Immediate Addressing
void LDY(){printf(" Instruction: LDY");addressingMode[cc][bbb](); Y = operand; setNZ();}
/* STx Instructions */
void STA(){addressingMode[cc][bbb]();mem[address] = A;}		// STA Absolute Addressing
void STX(){addressingMode[cc][bbb]();mem[address] = X;}
void STY(){addressingMode[cc][bbb]();mem[address] = Y;}
/* Arithmetic Instructions */
void LSR(){modify = 1;addressingMode[cc][bbb]();
	 *target = operand; int carry =operand & 1;operand >>= 1; puts("WRITE BACK");*target = operand; puts("RIGHT SHIFT"); if(carry){setFlag(C);} else {clearFlag(C);}; setNZ();}
void ASL(){modify = 1; addressingMode[cc][bbb]();
	 *target = operand; int carry =(operand >> 7) & 1;operand <<= 1; puts("WRITE BACK");*target = operand; puts("LEFT SHIFT"); if(carry){setFlag(C);} else {clearFlag(C);}; setNZ();}

void DEC(){modify = 1; addressingMode[cc][bbb](); *target = operand; operand--; puts("WRITE BACK");*target = operand; puts("DECREMENT"); setNZ();}
void INC(){modify = 1; addressingMode[cc][bbb](); *target = operand; operand++; puts("WRITE BACK");*target = operand; puts("INCREMENT"); setNZ();}

void ROR(){modify = 1; addressingMode[cc][bbb](); int carry = operand & 1;*target = operand; puts("WRITE BACK");*target = operand >> 1 | ((P >> C)&1) << 7; operand = *target; if(carry){setFlag(C);} else {clearFlag(C);};puts(" ROTATE RIGHT"); setNZ();}
void ROL(){modify = 1; addressingMode[cc][bbb](); int carry = (operand >> 7) & 1;*target = operand; puts("WRITE BACK");*target = operand << 1 | ((P >> C)&1); operand = *target; if(carry){setFlag(C);} else {clearFlag(C);};puts(" ROTATE LEFT"); setNZ();}

//void ROR(){addressingMode[cc][bbb](); *target = operand; puts("WRITE BACK");*target = operand << 1; operand = *target; setNZ();}   // Purposefully emulate the ROR bug...

void AND(){addressingMode[cc][bbb]() ;operand &= A; setNZ(); A= operand; printf(" AND");}
void ORA(){addressingMode[cc][bbb]() ;operand |= A; setNZ(); A = operand; printf(" ORA");}
void EOR(){addressingMode[cc][bbb]() ;operand ^= A; setNZ(); A = operand;  printf(" EOR");}

void BIT(){
	addressingMode[cc][bbb](); 
		if ((operand & A) == 0){setFlag(Z);} else {clearFlag(Z);}
		if ((((operand) >> 6) & 1)){setFlag(V);} else {clearFlag(V);}
		if ((((operand) >> 7) & 1)){setFlag(N);} else {clearFlag(N);}
	}
void ADC(){
	uint8_t carry = P &  1;
	uint16_t result = A;	
	addressingMode[cc][bbb]();
	result += operand;
	result += carry;
	uint8_t overflow = ~( A ^ operand) & (A ^ ( (uint8_t) A+operand)) & 0x80;
	printf(" Operand: %x, Overflow result!: %x\n", operand, overflow);
	if ((( result >> 8 ) & 1) == 1){setFlag(C); puts("CARRY FLAG SET");} else {clearFlag(C);}
	if (overflow) { setFlag(V); } else  {clearFlag(V);};
	
	A= result;
	operand = A; setNZ();
	puts(" ADD WITH CARRY"); 
}

void SBC(){
        uint8_t carry = P &  1;
        uint16_t result = A;
        addressingMode[cc][bbb]();
        result -= operand + !(carry);
        if ((( result >> 8 ) & 1) == 1){setFlag(C); puts("CARRY FLAG SET");} else {clearFlag(C);}
        if ( ~(A ^ operand) & (A ^ (A+operand)) & 0x80) { setFlag(V); } else  {clearFlag(V);};

	A = result;
	operand = A; setNZ();
        puts("SUBTRACT WITH CARRY");
}

void CMP(){
	addressingMode[cc][bbb]();
	printf(" CMP");
	if (A >= operand){ 
	setFlag(C); clearFlag(Z);
	if (A == operand){ setFlag(Z);} 
	else {clearFlag(Z);}
	} else {clearFlag(C); clearFlag(Z);}
	
	uint8_t result = A - operand;
	if ((result >> 7) & 1) {setFlag(N);} else {clearFlag(N);}
}

void CPX(){
        addressingMode[cc][bbb]();
	printf(" CPX");
	if (X >= operand){
        setFlag(C); clearFlag(Z);
        if (X == operand){ setFlag(Z);}
        else {clearFlag(Z);}
        } else {clearFlag(C); clearFlag(Z);}

        uint8_t result = X - operand;
        if ((result >> 7) & 1) {setFlag(N);} else {clearFlag(N);}

}

void CPY(){
        addressingMode[cc][bbb]();
	printf(" CPY");
        if (Y >= operand){
        setFlag(C); clearFlag(Z);
        if (Y == operand){ setFlag(Z);}
        else {clearFlag(Z);}
        } else {clearFlag(C); clearFlag(Z);}

        uint8_t result = Y - operand;
        if ((result >> 7) & 1) {setFlag(N);} else {clearFlag(N);}

}

/* Single Byte Instructions */
void DEX(){ X--; puts("DECREMENT X"); operand = X; setNZ(); cycles_count++;}
void DEY(){ Y--; puts("DECREMENT Y"); operand = Y; setNZ(); cycles_count++;} 
void INX(){ X++; puts("INCREMENT X"); operand = X; setNZ(); cycles_count++;}
void INY(){ Y++; puts("INCREMENT Y"); operand = Y; setNZ(); cycles_count++;}

void PHP(){ mem[0x100+SP] = P; SP--; cycles_count += 2;}
void PHA(){ mem[0x100+SP] = A; SP--; operand = A;cycles_count += 2; printf(" PUSH A TO S");}

void PLP(){ SP++; P= mem[0x100+SP]; cycles_count+=2; setFlag(B2); clearFlag(B);}
void PLA(){ SP++; A=mem[0x100+SP]; operand = A;  setNZ();cycles_count += 2; printf(" PULL A FROM S");}

void NOP(){ cycles_count++; printf(" NOP");}

void TXA(){ cycles_count++; A = X; operand =A; setNZ();}
void TXS(){ cycles_count++; SP = X;}
void TAX(){ cycles_count++; X = A; operand = X; setNZ();}
void TSX(){ cycles_count++; X = SP; operand = X; setNZ();}
void TYA(){ cycles_count++; A = Y; operand = A; setNZ();}
void TAY(){ cycles_count++; Y = A; operand = Y; setNZ();}
