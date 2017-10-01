#include <stdio.h>

#define SIZE	4000

int buff[SIZE];
int in=0, out=-1, cnt=0, col=0;

char hexchar(int val) {
	if (val<10) return '0'+val;
	else return 'A'+val-10;
}

void puthex(int val) {
	putchar(hexchar(val/16));
	putchar(hexchar(val%16));
}

main(int argc, char *argv[]) {
	int i;
	putcmd(0x3f);
	for (;;) {
		while (i=getdata(), (i!=-1) && (in!=out)) {
			buff[in++] = i;
			putdata(i);
			cnt++;
			if (in==SIZE) in=0;
		}
		while (cnt--) {
			if (buff[++out]!=0xFE) {
				puthex(buff[out]);
				putchar(' ');
				col++;
				if ((col%16)==0) putchar('\n');
				fflush(stdout);
			}
		}
		cnt=0;
	}
}
