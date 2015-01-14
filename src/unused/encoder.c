#include <stdio.h>

static char bitptr=0;
static unsigned int byte=0;
static char str[8]="00000000";

void print(int komma) {
				printf("0x%02X",byte);
				if (komma) 
						printf(","); 
				else
						printf("\n");
				bitptr=0;
				byte=0;
				for(int i=0;i<7;i++)  str[i]='0'; 
}

void putBit(int bit) {
		if (bitptr>7) {
				print(1);
		}
		byte |= (bit<<bitptr++);
		str[8-bitptr]=0x30+bit;
}



void encode_rom(int* rom) {
		// Reset
		putBit(0);
		putBit(1);

		// Rom with Complement
		for(int i=0;i<8;i++) {
				for (int j=0;j<8;j++) {
					int v=(rom[i] >> j) & 1;
					if (v == 1) {
							putBit(1);
							putBit(0);
					} else {
							putBit(0);
							putBit(1);
					}
				}
		}

}

void printInts(unsigned int* array, int length) {
		for (int i=0;i<length;i++)
				printf("%x ",array[i]);
}

typedef struct  {
		int a;
		int*b;
} s_T;

void mdo(s_T* s) {
		printf("%d %d\n",s->a,s->b[0]);
}

int main() {
		int a[]={1,2};
		mdo(&(s_T){
			2,
			(int[]){1.2}
		});

		unsigned int roms[2][8] ={
			   	{ 0x28,0x61,0xb6,0xce,0x01,0x00,0x00,0xbe },
				{ 0x28,0xd7,0xbd,0xce,0x01,0x00,0x00,0x7b }
		};

		printInts((int[]){1,2,3},3);

		for(int i=0;i<2;i++)
			encode_rom(roms[i]);
		
		print(0);
}

