#define PPUCTRL 0x2000
#define PPUMASK 0x2001
#define PPUSTATUS 0x2002
#define OAMADDR 0x2003
uint64_t ppu_cycles_count;
uint8_t ppu_cycles = 0;
int i = 0;

uint8_t odd_frame = 0;

uint16_t t_addr, v_addr, v, w;

// Sprite registers

uint8_t oam_primary[64][4];
uint8_t oam_secondary[8][4];

uint8_t fg_shift_bitmap[16];

uint8_t latches[8];
uint8_t counters[8];

uint8_t nametable,attr_table;
int n, m, scanline, oamsec = 0;
int state;
#define incrementPPUClock(val) 			\
	if (i < ppu_cycles_count){   		\
	ppu_cycles = (ppu_cycles + val)%340; 	\
	i += val;  				\
	}else{    				\
		printf("PPU CONTEXT OVER\n");	\
		return;  			\
	}   					\


// The PPU works as a state machine so that, switching between CPU and PPU is simple without causing timing issues...


void spriteEvaluation(){ 
                                        if (ppu_cycles & 1){                                            
                                                latches[5] = oam_primary[n][0];
                                        }
                                        else{                                                           
                                                if (oam_primary[n][0] <scanline+1 && scanline + 1< oam_primary[n][0] + 8 && (n < 64)){          // If scanline falls in range of tile, push to secondary OAM
                                                        m = 0;
                                                        while (m < 4){
                                                                if (oamsec >= 8){
                                                                        break;
                                                                }
                                                                oam_secondary[oamsec][m] = oam_primary[n][m];
                                                                m++;
                                                        }
                                                        oamsec++;
                                                n++;
                                                }
                                        }
                                }


void ppu_emulate(){
	ppu_cycles_count = cycles_count * 3;
	for (;;){
		if (ppu_cycles == 0){	
			state = -1;
			puts("PPU IDLE...");
			t_addr = v_addr;
			incrementPPUClock(1);
			odd_frame ^= 1;
		}
		while (ppu_cycles >= 1 && ppu_cycles <= 256){
			switch (state){
				case 0:
				state = 0;
				latches[0] = mem[(0x2000 | ((t_addr & 0xc00) << 1)) + (t_addr & 0x1F)]; incrementPPUClock(2);       			    // Fetch Nametable byte
				case 1:
				state = 1;
				latches[1] = mem[0x23C0 | (t_addr & 0x0C00) | ((t_addr >> 4) & 0x38) | ((t_addr >> 2) & 0x07)]; incrementPPUClock(2);       // Fetch attribute table
				spriteEvaluation();
				case 2:
				state = 2;
				latches[2] = mem[0x1000 | (v & 0x0FFF)]; incrementPPUClock(2);           						    // Fetch upper portion of tile
				case 3:
				state = 3;
				latches[3] = mem[(0x1000 | (v & 0x0FFF)) + 0x8]; incrementPPUClock(2);           					    // Fetch lower portion of tile
				spriteEvaluation();
				}
				
			}
				if ((t_addr & 0x01F) == 31){                              // Increment Fine X Scroll every 8 cycles
					t_addr &= ~(0x1F);
					t_addr ^= 0x400;
				}else { t_addr += 1 ;};
		 	
		}
	     while (ppu_cycles >= 257 && ppu_cycles <= 320){
			switch (state){
				case 6:
					state = 6;
					latches[0] = mem[0xBEEF];
				case 7:
					state = 7;
					latches[1] = mem[0xBEEF];
				case 8:
					state = 8;
					latches[2] = mem[0xBEEF];
				case 9:
					state = 9;
					latches[3] = mem[0xBEEF];
			}
	   while (ppu_cycles >= 320 && ppu_cycles <= 336){
			
		}
	   while (ppu_cycles >= 337 && ppu_cycles <= 340){

		}	
	}
}
	
void ppu_init(){
	mem[0x2000] = 0x0;
	mem[0x2001] = 0x0;
	mem[0x2003] = 0x0;
	odd_frame = 0;
}


