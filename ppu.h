#define PPUCTRL 0x2000
#define PPUMASK 0x2001
#define PPUSTATUS 0x2002
#define OAMADDR 0x2003
uint64_t ppu_cycles_count;
uint8_t ppu_cycles = 0;
int i = 0;

uint8_t odd_frame;

int shift_count_bg;
int shift_count_fg;
// Background registers
uint16_t bg_shift_bitmap[2];
uint8_t bg_shift_attr[2];

// Sprite registers

uint8_t oam_primary[64][4];
uint8_t oam_secondary[8][4];

uint8_t fg_shift_bitmap[16];

uint8_t latches[8];
uint8_t counters[8];

int n, m;
int state;
#define incrementPPUClock(val) \
	if (i < ppu_cycles_count){   \
	ppu_cycles = (ppu_cycles + val)%340; \
	i += val;  \
	}else{    \
		return;  \
	}   \


// The PPU works as a state machine so that, switching between CPU and PPU is simple without causing timing issues...
//
void ppu_emulate(){
	ppu_cycles_count = cycles_count * 3;
	for (ppu_cycles, i; i < ppu_cycles_count;){
		if (ppu_cycles == 0){
			state = -1;
			puts("PPU IDLE...");
			incrementPPUClock(1);
		}
		while (ppu_cycles >= 1 && ppu_cycles <= 256){
			switch (state){
				case 0:
				state = 0;
				latches[0] = mem[0x2000 + i]; incrementPPUClock(2);       // Fetch Nametable byte
				case 1:
				state = 1;
				latches[1] = mem[0x23C0 + i]; incrementPPUClock(2);       // Fetch attribute table
				case 2:
				state = 2;
				latches[2] = mem[0x0000 + i]; incrementPPUClock(2);           // Fetch upper byte of tile
				case 3:
				state = 3;
				latches[3] = mem[0x1000 + i]; incrementPPUClock(2);            // Fetch lower byte of tile
				case 4:
				state = 4;
				if (ppu_cycles <= 64){
					mem[0x2004] = 0xFF;                                          // Between cycles 1 and 64, OAMADDR will be set as 0xFF
				}
				case 5:	
				state = 5;
				if (ppu_cycles >= 65 && ppu_cycles <= 256){
					
				}
			}
			state = 0;
		 	
		}
	     while (ppu_cycles >= 257){
			
		}	
	}
}
	
void ppu_init(){
	mem[0x2000] = 0x0;
	mem[0x2001] = 0x0;
	mem[0x2003] = 0x0;
	odd_frame = 0;
}


