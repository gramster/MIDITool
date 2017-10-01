#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#ifndef MSC
#include <dir.h>
#endif
#include "miditool.h"

char InstTbl[MAXCHAN][MAXNAME];

int OutChans[MAXCHAN];	// Channel allocations to instruments

int /*in_chan,*/ out_chan, chosen_inst[TRACKS]={0}, num_inst = 0;
int velocity = 50;
char CFGFile[64], InstName[40], InstChan[10], InChan[10],
     TrkFile[TRACKS][64];
static WINDOW oldFocus;
WINDOW Appwnd;
int inDflat = 0;

/* Forward declarations */

static int PlayProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2);

/****************************************************************/

void SetName(WINDOW wnd, char *name) {
	if (wnd->extension) Mem_Free(wnd->extension);
	wnd->extension = Mem_Calloc(strlen(name)+1,1,935);
	strcpy(wnd->extension, name);
	AddTitle(wnd, NameComponent(name));
	SendMessage(wnd, BORDER, 0, 0);
}

void clear_cfg(void) {
	int i = MAXCHAN;
	while (i--) {
		InstTbl[i][0]=0; /* Clear names */
		OutChans[i] = -1;
		//ChanLines[i] = -1;
	}
	num_inst = 0;
}
int read_cfg(char *name) {
	FILE *fp;
	int i, magic;
	clear_cfg(); /* Clear old config */
	if ((fp=fopen(name,"rb"))!=NULL) {
		int i;
		fread(&magic,1,sizeof(magic),fp);
		if (magic!=0x1234) {
			fclose(fp);
			return 0;
		}
		fread(&num_inst,1,sizeof(num_inst),fp);
		fread(InstTbl,1,sizeof(InstTbl),fp);
		fread(OutChans,1,sizeof(OutChans),fp);
//		fread(ChanLines,1,sizeof(OutChans),fp);
		fread(&flushmode,1,sizeof(flushmode),fp);
		fread(&timesnap,1,sizeof(timesnap),fp);
		fread(&velocity,1,sizeof(velocity),fp);
		fread(&usereps,1,sizeof(usereps),fp);
		fread(&meter,1,sizeof(meter),fp);
		fread(&metrate,1,sizeof(metrate),fp);
		fclose(fp);
		return 1;
	} else return 0;
}

int write_cfg(char *name) {
	FILE *fp;
	if ((fp=fopen(name,"wb"))!=NULL) {
		int magic = 0x1234;
		fwrite(&magic,1,sizeof(magic),fp);
//		fwrite(&in_chan,1,sizeof(in_chan),fp);
		fwrite(&num_inst,1,sizeof(num_inst),fp);
		fwrite(InstTbl,1,sizeof(InstTbl),fp);
		fwrite(OutChans,1,sizeof(OutChans),fp);
//		fwrite(ChanLines,1,sizeof(OutChans),fp);
		fwrite(&flushmode,1,sizeof(flushmode),fp);
		fwrite(&timesnap,1,sizeof(timesnap),fp);
		fwrite(&velocity,1,sizeof(velocity),fp);
		fwrite(&usereps,1,sizeof(usereps),fp);
		fwrite(&meter,1,sizeof(meter),fp);
		fwrite(&metrate,1,sizeof(metrate),fp);
		fclose(fp);
		return 1;
	} else return 0;
}

/**************************************************/

void skipline(FILE *fp) {
	while (!feof(fp) && fgetc(fp)!='\n');
}

struct event *addNode(struct event *e,int nbytes, int b0, int b1, int b2, int b3) {
	e -> nbytes = nbytes;
	e -> b[0] = b0;
	e -> b[1] = b1;
	e -> b[2] = b2;
	e -> b[3] = b3;
	e -> next = eventalloc();
	return e->next;
}

struct event *addNoteChange(struct event *e, struct event **prev,
		int tcks, int minNote, char *new, char *old) {
	int i = 0;
	while (new[i]=='.' || new[i]=='O') {
		if (new[i]!=old[i]) {
			if (new[i]=='O') { // note on?
				*prev = e;
				e = addNode(e,4,tcks,0x90,minNote+i,velocity);
			} else { // note off
				*prev = e;
				e = addNode(e,4,tcks,0x80,minNote+i,0);
			}
		}
		i++;
	}
	return e;
}

void readGrafikFile(FILE *fp, char *buf) {
	struct event *nextnode = startevent[curtrack],
		*prevnode = NULL;
	char oldbuf[258];
	int Min, i, change=0, reps=0, tm = 0, lasttm = 0, first=1;
	for (i=0;i<256;i++) oldbuf[i] = '.';
	oldbuf[i] = 0;
	/* The first line contains the min note and time snap value */
	/* If a line begins with a C or R it is a new measure
		(the latter in the case of reps) */
	/* If a line begins with a . or digit it is time entry 
		(the latter in the case of reps) */
	/* All other lines are trashed */
	sscanf(buf+1,"%d %d",&Min,&timesnap);
	while (!feof(fp)) {
		fgets(buf,300,fp);
		if (buf[0]=='C' || buf[0]=='R') {
			// we allow mixing of two types
			if (buf[0] == 'R') reps = 1;
			else reps = 0;
			if (first) first=0;
			else if (!change) {
				nextnode = addNode(nextnode,1,0xF8,0,0,0); // timer overflow
				prevnode = nextnode;
				nextnode = addNode(nextnode,2,0x0,0xF9,0,0); // measure end
			} else {
				change = 0;
				prevnode = nextnode;
				nextnode = addNode(nextnode,2,240-tm,0xF9,0,0); // measure end
			}
			tm = lasttm = 0;
		} else if (!reps && (buf[0]=='.' || buf[0]=='O')) {
			if (strcmp(oldbuf,buf)==0) { // No change?
			} else {
				change = 1;
				nextnode = addNoteChange(nextnode,&prevnode,tm-lasttm,Min,buf,oldbuf);
				strcpy(oldbuf,buf); // Comments are thus disallowed on note lines
				lasttm = tm;
			}
			tm += timesnap;
		} else if (reps && (buf[0]>='1' && buf[0]<='9')) {
			int repcnt, pos;
			sscanf(buf,"%d",&repcnt);
			pos = 0;
			while (buf[pos++]!= ' ');
			while (buf[pos] != 'O' && buf[pos]!='.') pos++;
			change = 1;
			nextnode = addNoteChange(nextnode,&prevnode,tm-lasttm,Min,buf+pos,oldbuf);
			strcpy(oldbuf,buf+pos);
			lasttm = tm;
			tm += repcnt*timesnap;
		}
	}
	fclose(fp);
	i = 0;
	while (oldbuf[i]=='.' || oldbuf[i]=='O') buf[i++]= '.';
	buf[i] = 0;
	nextnode = addNoteChange(nextnode,&prevnode,tm-lasttm,Min,buf,oldbuf);
	Mem_Free(nextnode);
	prevnode->next = NULL;
}

void readTextFile(FILE *fp, char *buf) {
	struct event *nextnode = startevent[curtrack],
		*prevnode = NULL;
	do	{
		int b1=0,b2=0,b3=0,b4=0;
		nextnode->nbytes=sscanf(buf,"%x %x %x %x",&b1,&b2,&b3,&b4);
		if (nextnode->nbytes>0) {
			nextnode->b[0] = (uchar)b1;
			nextnode->b[1] = (uchar)b2;
			nextnode->b[2] = (uchar)b3;
			nextnode->b[3] = (uchar)b4;
			prevnode = nextnode;
			nextnode->next = eventalloc();
			nextnode = nextnode->next;
		}
		fgets(buf,300,fp);
	} while (!feof(fp));
	fclose(fp);
	Mem_Free(nextnode);
	if (prevnode) prevnode->next = NULL;
}

int read_track(char *name) {
	FILE *fp;
	char buf[300];
	struct event *nextnode;
	clearTrack(curtrack);
	if ((fp=fopen(name,"rt"))!=NULL) {
		WINDOW mwnd = MomentaryMessage("Reading track file");
		startevent[curtrack] = eventalloc();	/* start memory allocation with first node */
		fgets(buf,80,fp);
		if (buf[0]=='@')
			readGrafikFile(fp,buf);
		else
			readTextFile(fp,buf);
		SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
		hasdata[curtrack] = 1;
		PutMIDIData(curtrack);
		if (midiwin)
			SetName(midiwin,name);
		return 1;
	} else return 0;
}

int write_track(char *name) {
	FILE *fp;
	struct event *nextnode = startevent[curtrack];
	if ((fp=fopen(name,"wt"))!=NULL) {
		char line[80];
		while (nextnode && !feof(fp)) {
			buildline(line,curtrack,nextnode->nbytes,nextnode->b[0],
				nextnode->b[1],nextnode->b[2],nextnode->b[3]);
			fprintf(fp,"%s\n",line);
			nextnode = nextnode->next;
		}
		fclose(fp);
		return 1;
	} else return 0;
}

/**************************************************/

static void SetOldFocus(void)
{
	oldFocus = ( (GetClass(inFocus) == MENUBAR) ?
		PrevWindow(inFocus) : inFocus );
}

char **Argv;

static int tryReadFile(int (*reader)(), char *base, char *fname, char *sufx) {
	if ((*reader)(base)) {
		strcpy(fname, base);
		goto OK;
	} else if (sufx) {
		char tmp[64];
		strcpy(tmp,base);
		strcat(tmp,sufx);
		if ((*reader)(tmp)) {
			strcpy(fname,tmp);
			goto OK;
		}
	} else {
		strcpy(fname,base);
		goto OK;
	}
	return 0;
OK:
	return 1;
}

void main(int argc, char *argv[])
{
	int i;
	Argv = argv;
	Mem_Init();
	init_messages();
	clear_cfg();
	init_MPU401();
	tryReadFile(read_cfg,"MYPLAY.CFG",CFGFile,NULL);
	Appwnd = CreateWindow(APPLICATION,
			"MIDITOOL " VERSION " by Graham Wheeler (c) 1992",
			0, 0, -1, -1,
			MainMenu,
			NULL,
			PlayProc,
			MOVEABLE  |
			SIZEABLE  |
			HASBORDER |
			HASSTATUSBAR
			);
	SendMessage(Appwnd, SETFOCUS, TRUE, 0);
	inDflat = TRUE;
	makeWaitWin();
	i = 1;
	curtrack = 0;
	if (argc>1)
		while (i<argc) {
			if (tryReadFile(read_track,argv[i],TrkFile[curtrack],".TRK"))
				curtrack++;
			i++;
		}
	else tryReadFile(read_track,"NONAME.TRK",TrkFile[curtrack],NULL);
	curtrack = 0;
	PutMIDIData(curtrack);
	while (dispatch_message());
	inDflat = FALSE;
	clearTracks();
	clear_cfg();
	Mem_Check(TRUE);
}

/* ----------------------------- */

static void MySaveFile(int Saveas, char *name, int (*saver)())
{
	FILE *fp;
	if (*name == 0 || Saveas) {
		char FileName[64];
		if (DlgSaveAs(FileName))	{
			 strcpy(name, FileName);
		}
		else return;
	}
	if (*name)	{
		WINDOW mwnd = MomentaryMessage("Saving the file");
		(*saver)(name);
		SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
	}
}

static void ISDBox(WINDOW wnd, int redirect)
{
	int i;
	build_ISDB(redirect);
	if (DialogBox(wnd, ISDB, TRUE, NULL))	{
		if (redirect && RadioButtonSetting(ISDB,ID_NOREDIRECT))
			chosen_inst[curtrack]=num_inst;
		else for (i=0;i<MAXCONTROLS;i++) {
			if (RadioButtonSetting(ISDB,ID_INST0+i)) {
				chosen_inst[curtrack] = i;
				break;
			}
		}
	}
}

/* Menus */

/* --------------------- the main menu --------------------- */
DEFMENU(MainMenu)
    /* --------------- the File popdown menu ----------------*/
    POPDOWN( "~File",  PrepFileMenu, "Read/write configuration files. Go to DOS" )
        SELECTION( "~Clear All Tracks",		ID_ALLNEW,          0, 0 )
        SELECTION( "~New Track",		ID_NEW,          0, 0 )
        SELECTION( "~Open...",		ID_OPEN,         0, 0 )
        SEPARATOR
        SELECTION( "~Save Track",	ID_SAVE,     ALT_S, 0)
        SELECTION( "Save ~As...",	ID_SAVEAS,     0, 0)
        SELECTION( "~Print",	ID_PRINT,     0, 0)
        SEPARATOR
        SELECTION( "~DOS",        ID_DOS,          0, 0 )
        SELECTION( "E~xit",       ID_EXIT,     ALT_X, 0 )
    ENDPOPDOWN

    /* --------------- the Edit popdown menu ----------------*/
    POPDOWN( "~Edit", PrepEditMenu, "Change instruments/channels" )
        SELECTION( "~Undo",      ID_UNDO,  ALT_BS,    INACTIVE)
#ifdef INCLUDE_CLIPBOARD
        SEPARATOR
        SELECTION( "Cu~t",       ID_CUT,   SHIFT_DEL, INACTIVE)
        SELECTION( "~Copy",      ID_COPY,  CTRL_INS,  INACTIVE)
        SELECTION( "~Paste",     ID_PASTE, SHIFT_INS, INACTIVE)
        SEPARATOR
        SELECTION( "Cl~ear",     ID_CLEAR, 0,         INACTIVE)
#endif
        SELECTION( "~Delete",    ID_DELETETEXT, DEL,  INACTIVE)
        SEPARATOR
        SELECTION( "~Time Snap",      ID_TIMESNAP,  0,    0)
        SELECTION( "~Graphic Editor",      ID_GRAFIK,  0,    0)
    ENDPOPDOWN

    /* --------------- the Run popdown menu ----------------*/
    POPDOWN( "~Run", NULL, "Play/record MIDI data" )
        SELECTION( "~Record",    ID_RECORD,F9,    0)
        SELECTION( "~Play",      ID_PLAY,  CTRL_F9,    0)
        SELECTION( "play all ~Tracks",      ID_PLAYALL,  ALT_F9,    0)
        SELECTION( "~Metronome On/Off",      ID_METRO,  CTRL_F10,    0)
    ENDPOPDOWN

    /* --------------- the Search popdown menu ----------------*/
    POPDOWN( "~Search", PrepSearchMenu, "Search and replace" )
        SELECTION( "~Search...", ID_SEARCH,      0,    INACTIVE)
        SELECTION( "~Replace...",ID_REPLACE,     0,    INACTIVE)
        SELECTION( "~Next",      ID_SEARCHNEXT,  F3,   INACTIVE)
    ENDPOPDOWN

    /* --------------- the Options popdown menu ----------------*/
    POPDOWN( "~Options", NULL, "Change settings" )
        SELECTION( "~Change Config",	ID_CHANGECFG,   0,	0)
        SELECTION( "~Delete Instrument",ID_DELINST,	DEL,	0)
        SELECTION( "~Add Instrument",   ID_ADDINST,	0,	0)
        SELECTION( "~Insert",       ID_INSERT,     INS, TOGGLE)
        SEPARATOR
        SELECTION( "~New Config",	ID_NEWCFG,	0, 0 )
        SELECTION( "~Open Config...",	ID_OPENCFG,	0, 0 )
        SEPARATOR
        SELECTION( "~Save Config",	ID_SAVECFG,	0, 0)
        SELECTION( "Save Config ~As...",ID_SAVECFGAS,	0, 0)
    ENDPOPDOWN
    /* --------------- the Help popdown menu ----------------*/
    POPDOWN( "~Help", NULL, "Get help" )
#ifdef INCLUDE_HELP
        SELECTION(  "~Help for help...",  ID_HELPHELP,  0, 0 )
        SELECTION(  "~Extended help...",  ID_EXTHELP,   0, 0 )
        SELECTION(  "~Keys help...",      ID_KEYSHELP,  0, 0 )
        SELECTION(  "Help ~index...",     ID_HELPINDEX, 0, 0 )
        SEPARATOR
#endif
        SELECTION(  "~About...",          ID_ABOUT,     0, 0 )
#ifdef INCLUDE_HELP
#ifdef INCLUDE_RELOADHELP
        SEPARATOR
        SELECTION(  "~Reload Help Database",ID_LOADHELP,0, 0 )
#endif
#endif
    ENDPOPDOWN

ENDMENU


#ifdef INCLUDE_SYSTEM_MENUS
/* ------------- the System Menu --------------------- */

DEFMENU(SystemMenu)
    POPDOWN("System Menu", NULL, NULL)
        SELECTION("~Restore",  ID_SYSRESTORE,  0,         0 )
        SELECTION("~Move",     ID_SYSMOVE,     0,         0 )
        SELECTION("~Size",     ID_SYSSIZE,     0,         0 )
        SELECTION("Mi~nimize", ID_SYSMINIMIZE, 0,         0 )
        SELECTION("Ma~ximize", ID_SYSMAXIMIZE, 0,         0 )
        SEPARATOR
        SELECTION("~Close",    ID_SYSCLOSE,    CTRL_F4,   0 )
    ENDPOPDOWN
ENDMENU

#endif

static int PlayProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	WINDOW mwnd;
	char FileName[64];
	switch (msg)	{
		case STOP:
			if (midiwin->extension) Mem_Free(midiwin->extension);
			SendMessage(midiwin, CLOSE_WINDOW, 0, 0);
			closeGrafik();
			return TRUE;
		case KEYBOARD:
			if (p1==ALT_F1) curtrack=0;
			else if (p1==ALT_F2) curtrack=1;
			else if (p1==ALT_F3) curtrack=2;
			else if (p1==ALT_F4) curtrack=3;
			else if (p1==ALT_F5) curtrack=4;
			else if (p1==ALT_F6) curtrack=5;
			else if (p1==ALT_F7) curtrack=6;
			else if (p1==ALT_F8) curtrack=7;
			else break;
			closeGrafik();
			PutMIDIData(curtrack);
//			SendMessage(midiwin,PAINT,(PARAM)0,(PARAM)0);
			{ char tmp[60];
			  sprintf(tmp,"Track %d active",curtrack+1);
			  mwnd = MomentaryMessage(tmp);
			  sleep(1);
			  SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
			}
			return TRUE;
		case COMMAND:
			switch ((int)p1)	{
				case ID_ALLNEW:
					clearTracks();
					closeGrafik();
					return TRUE;
				case ID_NEW:
					clearTrack(curtrack);
					closeGrafik();
					return TRUE;
				case ID_GRAFIK:
					buildGrafik(startevent[curtrack]);
					return TRUE;
				case ID_METRO:
					toggleMetronome();
					return TRUE;
		                case ID_NEWCFG:
					clear_cfg();
					return TRUE;
				case ID_PLAY:ISDBox(wnd,TRUE);
					if (chosen_inst[curtrack]!=-1) {
						alltracks = 0;
						startPlay();
					}
					return TRUE;
				case ID_PLAYALL:
					alltracks = 1;
					startPlay();
					return TRUE;
				case ID_TIMESNAP:
					closeGrafik();
					snap(startevent[curtrack],NULL,NULL);
					return TRUE;
				case ID_OPEN:
					closeGrafik();
					if (DlgOpenFile("*.TRK", FileName)) {
						strcpy(TrkFile[curtrack],FileName);
						read_track(TrkFile[curtrack]);
						if (grafikwin->extension) Mem_Free(grafikwin->extension);
						grafikwin->extension = Mem_Calloc(strlen(TrkFile[curtrack])+1,1,935);
						strcpy(grafikwin->extension, TrkFile[curtrack]);
						AddTitle(grafikwin, NameComponent(TrkFile[curtrack]));
						SendMessage(grafikwin, BORDER, 0, 0);
					} else TrkFile[curtrack][0]=0;
					return TRUE;
		                case ID_OPENCFG:
					if (DlgOpenFile("*.CFG", FileName)) {
						strcpy(CFGFile,FileName);
						read_cfg(CFGFile);
					} else *CFGFile=0;
				 	return TRUE;
				case ID_RECORD:
					closeGrafik();
					startRecord();
					return TRUE;
				case ID_SAVE:
//					MySaveFile(FALSE, TrkFile[curtrack], write_track);
					if (editwin) SaveFile(editwin,FALSE);
					return TRUE;
				case ID_SAVECFG: MySaveFile(FALSE, CFGFile, write_cfg);
					return TRUE;
				case ID_SAVEAS:
//					MySaveFile(TRUE, TrkFile[curtrack], write_track);
					if (editwin) SaveFile(editwin,TRUE);
					return TRUE;
		                case ID_SAVECFGAS:
					MySaveFile(TRUE, CFGFile, write_cfg);
					return TRUE;
				case ID_ADDINST: AddInstBox(wnd); return TRUE;
				case ID_DELINST: ISDBox(wnd, FALSE);
					if (chosen_inst[curtrack]!=-1) {
						num_inst--;
						strcpy(InstTbl[chosen_inst[curtrack]],InstTbl[num_inst]);
						InstTbl[num_inst][0] = 0;
						OutChans[chosen_inst[curtrack]] = OutChans[num_inst];
						OutChans[num_inst] = -1;
					}
					return TRUE;
				case ID_CHANGECFG: CCDBox(wnd);
					return TRUE;
				case ID_ABOUT:
					MessageBox(
						"About Grams MIDI Player",
						"This program allows you to\n"
						"connect a MIDI input device\n"
						"to a MIDI output device.\n"
						"You specify the input channel\n"
						"and the output channels and\n"
						"instruments. You can then send\n"
						"the input to any instrument.");
					return TRUE;
				default:
					break;
				}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

