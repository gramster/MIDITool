/* record.c   MIDI track recorder */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <alloc.h>

//#include "chain.h"
//#include "writscrn.h"
//#include "standard.h"

#ifdef SOLO
#  include "mpu401.h"
#else
#  include "miditool.h"
#endif

struct event *store();
struct event *record();
struct event *play();

struct event *startevent[8] = {NULL}; /* first node for eight tracks */
struct event *lastevent[8];	/* current node for eight tracks */

#define isTiming(b)	(((uchar)b)<0xF0)
#define isNote(b) 	((((uchar)b)>=0x80) && (((uchar)b)<=0x9F))
#define isNoteOff(b) 	((((uchar)b)>=0x80) && (((uchar)b)<=0x8F))
#define isNoteOn(b) 	((((uchar)b)>=0x90) && (((uchar)b)<=0x9F))
#define isPoly(b) 	((((uchar)b)>=0xA0) && (((uchar)b)<=0xAF))
#define isControl(b) 	((((uchar)b)>=0xB0) && (((uchar)b)<=0xBF))
#define isProgram(b) 	((((uchar)b)>=0xC0) && (((uchar)b)<=0xCF))
#define isAfterTch(b) 	((((uchar)b)>=0xD0) && (((uchar)b)<=0xDF))
#define isPitch(b) 	((((uchar)b)>=0xE0) && (((uchar)b)<=0xEF))

int
	metrate = 120,
	meter = 4,
	curtrack = 1,			/* this program only uses track 1 */
	hasdata[TRACKS] = {0},
	flushmode=0,
	midiwinline = 0,
	alltracks = 0,
	timesnap = 1,
	wndpos,
	meton = 0;

WINDOW midiwin=NULL, grafikwin=NULL, editwin = NULL;

/* Must write routines to configure the meter and metronome rate */

void toggleMetronome(void) {
    void setMet();
    setMet(metrate);
    if (meton){
	sendcmd(MET_OFF);
	meton = 0;
    }
    else{
	sendcmd(MET_ON_WOUT);
	meton = 1;
    }
}

void clearTrack(int t) {
	if (hasdata[t]) {
		clear(startevent[t]);
		startevent[t] = NULL;
		hasdata[t] = 0;
	}
}

void clearTracks(void) {
	int i;
	for (i=0;i<TRACKS;i++) clearTrack(i);
}

int busy=0;

#ifndef SOLO

char *AddLine2Buf(char *buf, char *line) {
	int len, i;
	if (line==NULL) return buf;
	i = len=buf?strlen(buf):0;
	len += strlen(line)+2;
	buf = (char *)Mem_Realloc(buf,len,555);
	if (buf) {
		strcpy(buf+i,line);
		buf[len-2] = '\n';
		buf[len-1] = 0;
	}
	return buf;
}


static void ShowPosition(WINDOW wnd)
{
	char status[30];
	sprintf(status, "Line:%4d  Column: %2d",
		wnd->CurrLine, wnd->CurrCol);
	SendMessage(GetParent(wnd), ADDSTATUS, (PARAM) status, 0);
}

char *NameComponent(char *FileName)
{
	char *Fname;
	if ((Fname = strrchr(FileName, '\\')) == NULL)
		if ((Fname = strrchr(FileName, ':')) == NULL)
			Fname = FileName-1;
	return Fname + 1;
}

void SaveFile(WINDOW wnd, int Saveas)
{
	FILE *fp;
	if (wnd->extension == NULL || Saveas)	{
		char FileName[64];
		if (DlgSaveAs(FileName))	{
			if (wnd->extension != NULL)
				free(wnd->extension);
			if ((wnd->extension = malloc(strlen(FileName)+1)) != NULL)	{
				strcpy(wnd->extension, FileName);
				AddTitle(wnd, NameComponent(FileName));
				SendMessage(wnd, BORDER, 0, 0);
			}
		}
		else
			return;
	}
	if (wnd->extension != NULL)	{
		WINDOW mwnd = MomentaryMessage("Saving the file");
		if ((fp = fopen(wnd->extension, "wt")) != NULL)	{
			fwrite(GetText(wnd), strlen(GetText(wnd)), 1, fp);
			fclose(fp);
			wnd->TextChanged = FALSE;
		}
		SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
	}
}

static int editGrafikProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn;
	if (busy && rightbutton()) busy = 0;
	switch (msg)	{
		case COMMAND:
			switch ((int)p1) {
				case ID_SAVE:
					SaveFile(inFocus, FALSE);
					read_track(TrkFile[curtrack]);
					return TRUE;
				default: break;
			}
			break;
		case SETFOCUS:
			rtn = DefaultWndProc(wnd, msg, p1, p2);
			if ((int)p1 == FALSE)
				SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
			else 
				ShowPosition(wnd);
			return rtn;
		case MOVE:
		case SIZE:
			SendMessage(midiwin, PAINT, 0, 0);
			SendMessage(midiwin, BORDER, 0, 0);
			if (grafikwin) {
				SendMessage(grafikwin, PAINT, 0, 0);
				SendMessage(grafikwin, BORDER, 0, 0);
			}
			break;
		case KEYBOARD_CURSOR:
			rtn = DefaultWndProc(wnd, msg, p1, p2);
			ShowPosition(wnd);
			return rtn;
		case CLOSE_WINDOW:
			if (wnd->TextChanged)	{
				char *cp = malloc(25+strlen(GetTitle(wnd)));
				SendMessage(wnd, SETFOCUS, TRUE, 0);
				if (cp != NULL)	{
					strcpy(cp, GetTitle(wnd));
					strcat(cp, "\nText changed. Save it?");
					if (YesNoBox(cp))
						SendMessage(GetParent(wnd), COMMAND, ID_SAVE, 0);
					free(cp);
				}
			}
			wndpos = 0;
			if (wnd->extension != NULL)	{
				Mem_Free(wnd->extension);
				wnd->extension = NULL;
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

void makeWaitWin(void) {
	midiwin = CreateWindow(EDITBOX,TrkFile[curtrack],
		-1,-1,14,70,
		NULL,Appwnd,editGrafikProc,
	        CONTROLBOX|MINMAXBOX|SAVESELF|HASBORDER|
		MULTILINE|SHADOW|VSCROLLBAR|HSCROLLBAR|
		MOVEABLE|SIZEABLE);
	SendMessage(midiwin, SETFOCUS, TRUE, 0);
	editwin = midiwin;
}

void startRecord(void) {
	busy=1;
	clearTrack(curtrack);
	SendMessage(midiwin,CLEARTEXT,(PARAM)0,(PARAM)0);
	SendMessage(midiwin,PAINT,(PARAM)0,(PARAM)0);
	init_MPU401();
#ifdef SOLO
	puts("Recording...");
#else
	if (Appwnd->StatusBar) {
		SendMessage(Appwnd->StatusBar, CLEARTEXT, (PARAM)0,(PARAM)0);
		SendMessage(Appwnd->StatusBar, SETTEXT, (PARAM)"Recording - Right Mouse Button stops",(PARAM)0);
		SendMessage(Appwnd->StatusBar, PAINT, (PARAM)0,(PARAM)0);
	}
#endif
	toggleMetronome();
	toggleMetronome();
	startevent[curtrack] = record();
	hasdata[curtrack] = 1;
}

void activateTrack(int t) {
	sendcmd(ACT_TRACK);
	putdata(t);
}

void startPlay(void) {
	int i, trackmask = 0;
	if (!hasdata[curtrack]){
#ifdef SOLO
		puts("Nothing to play!");
#else
		ErrorMessage("No data in memory to play.");
#endif
	} else{	/* always start at begining */
		busy = 1;
		init_MPU401();
#ifdef SOLO
		puts("Playing...");
#else
		if (Appwnd->StatusBar) {
			SendMessage(Appwnd->StatusBar, CLEARTEXT, (PARAM)0,(PARAM)0);
			SendMessage(Appwnd->StatusBar, SETTEXT, (PARAM)"Playing - Right Mouse Button stops",(PARAM)0);
			SendMessage(Appwnd->StatusBar, PAINT, (PARAM)0,(PARAM)0);
		}
#endif
		if (alltracks)
			for (i = 0; i < TRACKS; i++){		
				lastevent[i] = startevent[i];
				if (hasdata[i]) trackmask |= (1<<i);
			}
		else {
			for (i = 0; i < 8; i++){		
				lastevent[i] = startevent[i];
			}
			lastevent[curtrack] = startevent[curtrack];
			trackmask = (1<<curtrack);
		}
		activateTrack(trackmask);
 		toggleMetronome();
		toggleMetronome();
		play(lastevent);
	    }
}


#define BUFSIZE		100
int cmdbuf[BUFSIZE];		/* global buffer for pending MPU commands */
int cmdbufp = 0;		/* next free position in cmdbuf */

getnext()	/* get a possibly pushed back command from MPU or cmdbuf */
{
    return((cmdbufp > 0) ? cmdbuf[--cmdbufp] : getdata());
}

void ungetnext(n)	/* push a command back on input */
int n;
{
    if (cmdbufp > BUFSIZE)
#ifdef SOLO
	puts("ungetc ran out of buffer space");
#else
	ErrorMessage("ungetnext ran out of buffer space.");
#endif
    else
	cmdbuf[cmdbufp++] = n;
}

int get401()		/* get next byte from mpu401 (or pending buffer) */		
{			/* try forever, stop on keypress */
    int i;
    
    do	{
	i = getnext();
//	if (kbhit() ) break;
	if (rightbutton()) break;
	if (i != -1) return(i&0xFF);
    } while (busy);
    return(-1);
}

void setMet(int tempo) { /* set metronome */
	sendcmd(SET_TEMPO);
	putdata(tempo/2);
}

void setBPM(int meter) { /* set beats per measure */
	sendcmd(METRO_MEAS);
	putdata(meter);
}

void init_MPU401(void) {
#ifdef SOLO
	puts("Initialising...");
#endif
	putcmd(RESET);			/* clear MPU-401 */
	delay(500);
//	sendcmd(BEND_ON);			/* allow pitch bend data to pass */
	setMet(metrate);
	setBPM(meter);
}

void sendcmd(int n)	/* send a command, check for ACK, if not save MPU data */
		/* until it stops sending */
{
    int ans;

    ans = putcmd(n);
    if (ans == ACK)
	return;
    else if (ans != -1){
	ungetnext(ans);		/* put pending data on stack */
	while (1){		/* check for more incoming data */
	    ans = getdata();
	    if (ans == ACK || ans == -1)
		return;
	    else
		ungetnext(ans);
	}
    }
}

void winPutLine(WINDOW w, char *msg, int *line, int flushmode) {
	SendMessage(w,ADDTEXT,(PARAM)msg,(PARAM)NULL);
	(*line)++;
	if (flushmode && ((*line)>=w->ht)) {
		SendMessage(w,SCROLL,(PARAM)1,(PARAM)0);
		SendMessage(w, PAINT, (PARAM)0, (PARAM)0);
	}
}

#ifdef SOLO
#  define wwput(n)	puts(n)
#else
#  define wwput(n)	winPutLine(midiwin,n,&midiwinline,flushmode)
#endif

void addline(char *line, int trk, int nbytes, uchar b1, uchar b2, uchar b3, uchar b4) {
    if (nbytes==0) sprintf(line,"              ; ");
    else if (nbytes==1) sprintf(line,"%02X           ; ",(int)b1);
    else if (nbytes==2) sprintf(line,"%02X %02X        ; ",(int)b1, (int)b2);
    else if (nbytes==3) sprintf(line,"%02X %02X %02X     ; ",(int)b1, (int)b2, (int)b3);
    else if (nbytes==4) sprintf(line,"%02X %02X %02X %02X  ; ",(int)b1, (int)b2, (int)b3, (int)b4);
    sprintf(line+15,"Trk %-2d ",trk+1);
}

char *noteNames[12] = {
	"C",	"C#",	"D",	"D#",	"E",	"F",
	"F#",	"G",	"G#",	"A",	"A#",	"B",
};

char *note(int val, int *oct) {
	*oct = (val/12)+1;
	return noteNames[val%12];
}

void buildline(char *line, int trk, int n, uchar b1, uchar b2, uchar b3, uchar b4) {
	char *cmt;
	cmt = line+21;
	addline(line, trk, n, (uchar)b1, (uchar)b2, (uchar)b3, (uchar)b4);
	if (isTiming(b1)){
	    if (isNote(b2)) {
		char *n; int o;
		n = note(b3,&o);
		if (b4==0 || isNoteOff(b2))
			sprintf(cmt,"Tcks: %-3d Ch: %-3d Note: %-2s (%d) Off",
				b1,(b2&0xF)+1,n,o);
		else
			sprintf(cmt,"Tcks: %-3d Ch: %-3d Note: %-2s (%d) Velocity %-3d",
				b1,(b2&0xF)+1,n,o,b4);
	    } else if (isPoly(b2))
		sprintf(cmt,"ch %2d Polyphonic key pressure",(b2%16)+1);
	    else if (isControl(b2))
		sprintf(cmt,"ch %2d Control change",(b2%16)+1);
	    else if (isProgram(b2))
		sprintf(cmt,"ch %2d Program change",(b2%16)+1);
	    else if (isAfterTch(b2))
		sprintf(cmt,"ch %2d After touch",(b2%16)+1);
	    else if (isPitch(b2))
		sprintf(cmt,"ch %2d Pitch wheel",(b2%16)+1);
	    else if (b2 == 0xF8)
		sprintf(cmt,"NOP");
	    else if (b2 == 0xF9)
		sprintf(cmt,"Measure end");
	    else if (b2 == 0xFC) 	/* data end for track */
		sprintf(cmt,"Data end");
	    else sprintf(cmt,"Unknown b2 value");
	} else if (b1<0xF8) {
		sprintf(cmt,"Trk %2d Request",(b1%16)+1);
	} else switch(b1) {
	    case 0xF8: sprintf(cmt,"Time overflow"); break;
	    case 0xF9: sprintf(cmt,"Conductor request"); break;
	    case 0xFC: sprintf(cmt,"All end"); break;
	    case 0xFD: sprintf(cmt,"Clock signal"); break;
	    case 0xFE: sprintf(cmt,"ACK"); break;
	    case 0xFF: sprintf(cmt,"Sys message"); break;
	    default:   sprintf(cmt,"Cmd %02X",b1); break;
	}
}

void PutPacket(int trk,int n, uchar b1, uchar b2, uchar b3, uchar b4) {
	char line[80];
	buildline(line, trk, n, b1, b2, b3, b4);
	wwput(line);		
}

struct event *record(void)	/* record a track, stop on any keypress */
{
    char tmp[40];
    int first, second, third, fourth, mstatus;
    struct event *firstnode;
    struct event *nextnode;

    mstatus = 0;
    firstnode = nextnode = eventalloc();	/* start memory allocation with first node */
//    while (kbhit()) getch();	/* clear any stray keypress */
    sendcmd(START_REC);
    while (busy) {
	first = get401();
	if (first == -1) {		/* a keypress */
//	    while (kbhit()) getch();
	    break;
        }
	if (first == 0xFE){		/* ignore acknowledge */
		PutPacket(curtrack, 1,first,0,0,0);
	} else if (isTiming(first)){
	    second = get401();
	    if (second <= 0x7F){		/* MIDI data, running status assumed */
		nextnode = store(nextnode,4,first,mstatus,second,get401());
	    }
	    else if (second <= 0xBF){	/* MIDI message, note on/off */	
		mstatus = second;	/* after touch or control change */
		third = get401();
		nextnode = store(nextnode,4,first,second,third,get401());
	    }
	    else if (second <= 0xDF){	/* prog change or chan after touch */
		mstatus = second;
		nextnode = store(nextnode,3,first,second,get401(),0);
	    }
	    else if (second <= 0xEF){	/* pitch wheel */
		mstatus = second;
		third = get401();
		nextnode = store(nextnode,4,first,second,third,get401());
	    }
	    else if (second == 0xF9){	/* measure end */
		nextnode = store(nextnode,2,first,second,0,0);
	    }
	    else if (second == 0xFC){	/* data end for track */
		nextnode = store(nextnode,2,first,second,0,0);
		break;	/* only one track recording, so done */
	    }
	    else{
		sprintf(tmp,"Unrecognized data %x %x",first,second);
#ifdef SOLO
		puts(tmp);
#else
		ErrorMessage(tmp);
#endif
	    }
	} else if (first <= 0xF7 || first==0xF9 || first==0xFD) {
		PutPacket(curtrack,1,first,second,third,fourth);
	} else if (first == 0xF8) {		/* time out */
	    nextnode = store(nextnode,1,first,0,0,0);
	} else if (first == 0xFC) {
		PutPacket(curtrack,1,first,second,third,fourth);
		break;	/* all end */
	} else{
#ifdef SOLO
	    puts("Record stopped.");
#else
	    ErrorMessage("Record stopped.");
#endif
	    break;
	}
	if (nextnode == NULL){
#ifdef SOLO
	    puts("Could not allocate more memory (full?), record stopped.");
#else
	    ErrorMessage("Could not allocate more memory (full?), record stopped.");
#endif
	    break;
	}
    }
    /* must trash the last node; make ACK */
    nextnode->b[0] = 0xFE;
    nextnode->nbytes = 1;
    sendcmd(STOP_REC);
    get401();
    if (!flushmode)
    	SendMessage(midiwin, PAINT, (PARAM)0, (PARAM)0);
    return(firstnode);
}

struct event *eventalloc()		/* returns pointer to memory area */
{					/* sized for one event. */
    return((struct event *)Mem_Calloc(1,sizeof(struct event),100));
}

struct event *store(node,nbytes,b1,b2,b3,b4)
int nbytes;
struct event *node;		/* store an event, return pointer to next */
int b1,b2,b3,b4;
{
    PutPacket(curtrack,nbytes, (uchar)b1, (uchar)b2, (uchar)b3, (uchar)b4);
    node->next = eventalloc();
    (node->next)->next = NULL;	/* null pointer in next node to mark end */
    node->nbytes = nbytes;
    node->b[0] = b1;
    node->b[1] = b2;
    node->b[2] = b3;
    node->b[3] = b4;
    return(node->next);
}

void PutMIDIData(int trk) {
	struct event *e = startevent[trk];
	SendMessage(midiwin, CLEARTEXT, (PARAM)0, (PARAM)0);
	while (e) {
	    PutPacket(trk,e->nbytes, e->b[0], e->b[1], e->b[2], e->b[3]);
	    e = e->next;
	}
	SendMessage(midiwin, ADDTEXT, (PARAM)" ", (PARAM)0);
	AddTitle(midiwin,TrkFile[trk]);
	SendMessage(midiwin, PAINT, (PARAM)0, (PARAM)0);
}

struct event *play(events)		/* play tracks */	
struct event *events[];	
{
    int i, cmd, trk, firstcmd=1;
#ifdef SOLO
    while (kbhit()) getch();		/* clear any stray keypress */
#endif
    sendcmd(CLEAR_PCOUNT);		/* clear play counters */
    sendcmd(START_PLAY);
    while (busy){
	cmd = get401();			/* get next mpu-401 request */
	if (cmd == -1) break;			/* quit on keypress/mouse */
	if (cmd >= REQ_T1 && cmd <= REQ_T8){		/* track req */
	    struct event *e;
	    firstcmd = 0;
	    trk = cmd - REQ_T1;				/* track number */
	    e = events[trk];	
	    if (flushmode) PutPacket(trk,e->nbytes, e->b[0], e->b[1], e->b[2], events[trk]->b[3]);
	    for (i = 0; i < e->nbytes; i++){
		if (i==1) {
			uchar c = e->b[1];
			if (OutChans[chosen_inst[trk]]!=-1) {
				if (isNoteOff(c))
					putdata(0x80+OutChans[chosen_inst[trk]]);
				else if (isNoteOn(c))
					putdata(0x90+OutChans[chosen_inst[trk]]);
				else putdata(c);/* send data bytes */
			} else putdata(c);		/* send data bytes */
		} else putdata(e->b[i]);		/* send data bytes */
	    }
	    if (events[trk]->next == NULL)
		break;
	    else
		events[trk] = events[trk]->next;	/* move track counter */	
	     						/* forward one event */
	}
	else if (cmd == ALL_END){
	    if (firstcmd)		/* don't quit if received at start */
		firstcmd = 0;
	    else
		break;
	}
    }
    sendcmd(STOP_PLAY);
    sendcmd(CLEAR_PMAP);
    return(events[trk]);
}

void clear(struct event *event)			/* clear a track from memory */
{
    struct event *nextevent;
    
    if (event) do{
	nextevent = event->next;
	Mem_Free(event);
	event = nextevent;
    }while (event != NULL);
}

#ifdef SOLO

main() {
	init_MPU401();
	startRecord();
	startPlay();
}

#endif

void snap(struct event *event, int *Min, int *Max) {
	int tm = 0, tmp, lasttm = 0;
	if (Min) *Min = 0xFF;
	if (Max) *Max = 0;
	SendMessage(midiwin,CLEARTEXT,(PARAM)0,(PARAM)0);
	SendMessage(midiwin,PAINT,(PARAM)0,(PARAM)0);
	while (event) {
		if (isTiming(event->b[0])) {
			if (isNote(event->b[1])) {
				if (Min) {
					if (((uchar)event->b[2])<*Min)
						*Min = (int)((uchar)event->b[2]);
				}
				if (Max) {
					if (((uchar)event->b[2])>*Max)
						*Max = (int)((uchar)event->b[2]);
				}
			}
			tm += (uchar)event->b[0];
			tmp = tm - lasttm; /* elapsed time since last snapped event */
			tmp /= timesnap;
			tmp*= timesnap; /* snap this time */
			event->b[0] = (char)tmp;
			lasttm += tmp;
		}
		PutPacket(curtrack,event->nbytes, event->b[0], event->b[1], event->b[2], event->b[3]);
		if (((uchar)event->b[1])==0xF9)
			lasttm = tm = 0;/* Measure end */
		event = event->next;
	}
	SendMessage(midiwin,PAINT,(PARAM)0,(PARAM)0);
}

void makeGrafikWin(void) {
	grafikwin = CreateWindow(EDITBOX,TrkFile[curtrack],
		4,4,10,30,
		NULL,Appwnd,editGrafikProc,
	        MOVEABLE|CONTROLBOX|SIZEABLE|MINMAXBOX|SAVESELF|HASBORDER|
		MULTILINE|SHADOW|VSCROLLBAR|HSCROLLBAR);
	SetName(grafikwin,TrkFile[curtrack]);
	SendMessage(grafikwin, SETFOCUS, TRUE, 0);
	AddAttribute(midiwin,READONLY);
	editwin = grafikwin;
}

void closeGrafik(void) {
	if (grafikwin) {
		if (grafikwin->extension) Mem_Free(grafikwin->extension);
		SendMessage(grafikwin, CLOSE_WINDOW, 0, 0);
		if (midiwin) ClearAttribute(midiwin,READONLY);
		grafikwin=NULL;
	}
	editwin = midiwin;
}

#endif

char stateline[257]={0};
char oldstate[257]={0};
char titleline1[261]={0};
char titleline2[261]={0};
int overflow=0;
char *grafikbuf;

struct event *skip(struct event *e) {
	while (e && (((uchar)e->b[0])>=0xF0)) {
		if (((uchar)e->b[0])==0xF8)
			overflow++;
		e=e->next;
	}
	return e;
}

int minimise = 1;
int reps, usereps = 1;

// catch up to the last line

int catchUp(int tm, int target, int flush) {
	while (tm<=(target-timesnap)) {
		tm += timesnap;
		if (usereps) {
			if (strcmp(stateline,oldstate)) {
				char tmp[260];
				if (reps) {
					sprintf(tmp,"%-3d %s",reps,oldstate);
					grafikbuf = AddLine2Buf(grafikbuf,tmp);
				}
				strcpy(oldstate,stateline);
				reps = 1;
			} else reps++;
		} else grafikbuf = AddLine2Buf(grafikbuf,stateline);
	}
	if (usereps && flush) {
		char tmp[260];
		if (reps) {
			sprintf(tmp,"%-3d %s",reps,oldstate);
			grafikbuf = AddLine2Buf(grafikbuf,tmp);
		}
		strcpy(oldstate,stateline);
		reps = 0;
	}
	return tm;
}

void buildGrafik(struct event *e) {
	int tm=0;
	int i, j, Min,Max, width;
	reps=0;
	grafikbuf = NULL;
	snap(e, &Min, &Max);
	if (Min>Max) {
		Min = Max;
		Max += 12;
		if (Max>255) Max = 255;
	}
	if (!minimise) { Min = 0; Max = 255; }
	else {
		if (Min) Min--;
		Min/=12;
		Min*=12;
		Max += 11;
		Max /= 12;
		Max *= 12;
	}
	width = Max-Min+1;
	makeGrafikWin();
	overflow=0;
	sprintf(titleline1,"@%d %d Leftmost note value and timesnap",Min,timesnap);
	grafikbuf = AddLine2Buf(grafikbuf,titleline1);
	if (usereps) {
		strcpy(titleline1,"    ");
		strcpy(titleline2,"Rep ");
	}
	for (i=Min, j=usereps?4:0;i<=Max;i++, j++)
		titleline1[j] = noteNames[i%12][0];
	titleline1[j] = 0;
	for (i=Min, j=usereps?4:0; i<=Max;i++,j++)
		if (noteNames[i%12][1])
			titleline2[j] = noteNames[i%12][1];
		else titleline2[j] = ' ';
	titleline2[j] = 0;
	for (i=0;i<width;i++) stateline[i] = '.';
	stateline[i] = 0;
	strcpy(oldstate,stateline);
	grafikbuf = AddLine2Buf(grafikbuf,titleline1);
	grafikbuf = AddLine2Buf(grafikbuf,titleline2);
	while (e) {
		if (((uchar)e->b[0])==0xF8) overflow = 1;
		else if (((uchar)e->b[1])==0xF9) {  /* Measure end */
			int cnt;
			if (overflow && ((uchar)e->b[0])==0) {
				catchUp(0,240,TRUE);
				overflow=0;
			} else {
				catchUp(tm,240,TRUE);
			}
			grafikbuf = AddLine2Buf(grafikbuf,titleline1);
			grafikbuf = AddLine2Buf(grafikbuf,titleline2);
			tm = 0;
		} else if (isTiming(e->b[0])) {
			tm = catchUp(tm,tm+(uchar)e->b[0],FALSE);
			if (isNote(e->b[1])) {
				/* Note on/off */
				if (isNoteOff(e->b[1]) || (((uchar)e->b[3])==0))
					stateline[(uchar)e->b[2]-Min] = '.'; /* note off */
				else stateline[(uchar)e->b[2]-Min] = 'O';
			}
		}
		e = e->next;
	}
 	catchUp(tm,tm,TRUE);
	SendMessage(grafikwin,SETTEXT,(PARAM)grafikbuf,(PARAM)0);
	Mem_Free(grafikbuf);
	SendMessage(grafikwin,PAINT,(PARAM)0,(PARAM)0);
}
