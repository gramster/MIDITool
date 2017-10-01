#include <dos.h>

#define DATAPORT	0x330
#define STATPORT	0x331
#define MPUACK		0xFE
#define TRIES		0xFF

int putcmd(int n) {
	int tries = TRIES;
	while (tries--) {
		if ((inportb(STATPORT)&0x40)==0) {
			disable();
			outportb(STATPORT,n);
			tries = TRIES;
			while (tries--) {
				if ((inportb(STATPORT)&0x80)==0x80) {
					if (inportb(DATAPORT)==MPUACK) {
						enable();
						delay(10);
						return MPUACK;
					} else break;
				}
			}
			break;
		}
	}
	enable();
	return -1;
}

#if 0
int getdata(void) {
	int tries = TRIES;
	while (tries--) {
		if ((inportb(STATPORT)&0x80)==0x80)
			return inportb(DATAPORT);
	}
	return -1;
}

int putdata(int n) {
	int tries = TRIES;
	while (tries--) {
		if ((inportb(STATPORT)&0x40)==0) {
			outportb(DATAPORT,n);
			return n;
		}
	}
	return -1;
}
#endif
