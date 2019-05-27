
uint8_t bytes;
void mapper_0(uint8_t* data){
	int index = 0; 
	while (index < 16 * 1024 * bytes){
		mem[0x8000 + index] = data[index + 16];
		mem[0xC000 + index] = data[index + 16];
		index++;
	}
	//PC = 0xc000;
	PC = (mem[0xFFFD] << 8 | mem[0xFFFC]);
}


void (*mappers[256])(uint8_t*) = {mapper_0, mapper_0};

