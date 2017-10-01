#include <dos.h>
#include "mpu401.h"

#define DATAPORT	0x330
#define STATPORT	0x331
#define MPUACK		0xFE
#define TRIES		0xFF

#if 1
int putcmd(int n) {
	int tries = TRIES;
	while (tries--) {
		if ((inportb(STATPORT)&0x40)==0) {
			disable();
			outportb(STATPORT,n);
			if (n==RESET) delay(100);
			tries = TRIES;
			while (tries--) {
				if ((inportb(STATPORT)&0x80)==0) {
					if ((n=inportb(DATAPORT))==MPUACK) {
						enable();
						return MPUACK;
					}// else break;
				}
			}
			break;
		}
	}
	enable();
	return -1;
}
#endif

#if 1
int getdata(void) {
	int tries = TRIES;
	while (tries--) {
		if ((inportb(STATPORT)&0x80)==0)
			return inportb(DATAPORT);
	}
	return -1;
}
#endif

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
