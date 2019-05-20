#include <stdlib.h>
#include "mappers.h"
void loadINESFile(const char* filename){
puts("parsing");
char data[0xFFFF];

FILE* f = fopen(filename,"rb");
if (!f){
        puts("File not found");
        exit(-1);
}
unsigned char ch = getc(f);
int count = 0;
while (count < 0xFFFF){
        data[count] = ch;
        ch = getc(f);
        count += 1;
}
puts("FILE LOADED");

if (data[0] == 0x4E && data[1] == 0x45 && data[2] == 0x53 && data[3] == 0x1A){
	puts("INES MAGIC NUMBER VALIDATED");
}
else{
	puts("INVALID INES HEADER");
	exit(0);
}

uint8_t mapper_val = (data[7] >> 4) << 8 | (data[6]  >> 4);
printf("Size of PRG ROM: %u KB\n",data[4] * 16);
printf("Size of CHR ROM: %u KB\n",data[5] * 16);
printf("Mapper: %u\n",(data[7] >> 4) << 8 | (data[6]  >> 4));

bytes = data[4];

if (mappers[mapper_val]){
	mappers[mapper_val](data);
}
else{
	puts("UNSUPPORTED MAPPER");
	exit(-1);
}
}

