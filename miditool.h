#include "dflat.h"
#include "mpu401.h"

#define PTR_TO_LONG(p)	((((ulong)(p)) >> 16l)*16l+ (ulong)((ushort)(p)))

#define MAXCHAN	16
#define MAXNAME	40
#define TRACKS	8

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

struct event {
    struct event *next;
    char nbytes;
    char b[4]; 
};

extern int
	metrate,
	meter,
	curtrack,			/* this program only uses track 1 */
	hasdata[],
//	in_chan,
	flushmode,
	redirect,
	alltracks,
	out_chan,
	num_inst,
	chosen_inst[],
	timesnap,
	usereps,
	velocity,
	meton;
extern struct event *startevent[8];
extern WINDOW Appwnd, midiwin, grafikwin, editwin;
extern DBOX *ISDB, *CCDB, ChSelectDB, InstNameDB;
extern char InstTbl[MAXCHAN][MAXNAME],	// Instrument names
	InstName[], InstChan[], InChan[], TrkFile[TRACKS][64];
extern int OutChans[MAXCHAN];	// Instrument allocations to channels

void SaveFile(WINDOW wnd, int Saveas);
void init_MPU401(void);
void sendcmd(int n);
void startRecord(void);
void startPlay(void);
void clear(struct event *event);
void clearTrack(int t);
void clearTracks(void);
int putcmd(int n);
int getdata(void);
int putdata(int n);
char *NameComponent(char *FileName);
void SetName(WINDOW wnd, char *name);
void makeWaitWin(void);
void free_ISDB(void);
void build_ISDB(int redirect);
void AddInstBox(WINDOW wnd);
void free_CCDB(void);
void CCDBox(WINDOW wnd);
void PrepFileMenu(void *wnd, struct Menu *mnu);
void PrepSearchMenu(void *wnd, struct Menu *mnu);
void PrepEditMenu(void *wnd, struct Menu *mnu);
struct event *eventalloc();
void buildGrafik(struct event *e);
void Mem_Check(int free_table);
void Mem_Init(void);
void *Mem_Malloc(int nbytes, int who);
void *Mem_Calloc(int nelts, int size, int who);
void *Mem_Realloc(void *old, int nbytes, int who);
void Mem_Free(void *p);
