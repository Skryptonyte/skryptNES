#define PPU_CYCLES 50

#define PPUCTRL 0x2000
#define PPUMASK 0x2001
#define PPUSTATUS 0x2002
#define OAMADDR 0x2003
uint64_t ppu_cycles
uint8_t odd_frame;


uint8_t oam_mem[0x100];

void ppu_emulate(){

}
void ppu_init(){
	mem[0x2000] = 0x0;
	mem[0x2001] = 0x0;
	mem[0x2003] = 0x0;
	oddframe = 0;
}


