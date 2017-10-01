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

DBOX *ISDB=NULL, *CCDB = NULL;

/*******************************************/
/* Build an OK/Cancel/Help or other button */
/*******************************************/

static void buildBasic(CTLWINDOW *w) {
	w->dwnd.title = NULL;
	w->vtext = NULL;
	w->setting = OFF;
	w->wnd = NULL;
	w->dwnd.h = 1;
}

static void buildButton(CTLWINDOW *w, char *name, int x, int y, int cmd, char *hlp) {
	buildBasic(w);
	w->dwnd.x = x;
	w->dwnd.y = y;
	w->dwnd.w = 8;
	w->class = BUTTON;
	w->itext = name;
	w->command = cmd;
	w->help = hlp;
	w->isetting = ON;
}

static void buildRadioButton(CTLWINDOW *w, int x, int y, int cmd, char *hlp) {
	buildBasic(w);
	w->dwnd.x = x;
	w->dwnd.y = y;
	w->dwnd.w = 4;
	w->class = RADIOBUTTON;
	w->itext = NULL;
	w->command = cmd;
	w->help = hlp;
	w->isetting = OFF;
}

static void buildCheckbox(CTLWINDOW *w, int x, int y, int cmd, char *hlp, int set) {
	buildBasic(w);
	w->dwnd.x = x;
	w->dwnd.y = y;
	w->dwnd.w = 4;
	w->class = CHECKBOX;
	w->itext = NULL;
	w->command = cmd;
	w->help = hlp;
	w->isetting = set;
}

static void buildTextBox(CTLWINDOW *w, char *txt, int x, int y, int wdth, int cmd, char *hlp) {
	buildBasic(w);
	w->dwnd.x = x;
	w->dwnd.y = y;
	w->dwnd.w = wdth;
	w->class = TEXT;
	w->itext = txt;
	w->command = cmd;
	w->help = hlp;
	w->isetting = OFF;
}

static void buildEditBox(CTLWINDOW *w, char *txt, int x, int y, int wdth, int cmd, char *hlp) {
	buildBasic(w);
	w->dwnd.x = x;
	w->dwnd.y = y;
	w->dwnd.w = wdth;
	w->class = EDITBOX;
	w->itext = txt;
	w->command = cmd;
	w->help = hlp;
	w->isetting = OFF;
}

/* -------------- the Add Instrument dialog box --------------- */
DIALOGBOX( InstNameDB )
    DB_TITLE(        "Add Instrument",    -1,-1,9,62)
    CONTROL(TEXT,    "~Name:",          2, 1, 1, 11, ID_INSTNAME)
    CONTROL(EDITBOX, NULL,                   14, 1, 1, 29, ID_INSTNAME)

    CONTROL(BUTTON, "   ~OK   ",    7, 5, 1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ",   19, 5, 1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",   31, 5, 1,  8, ID_HELP)
ENDDB

/* -------------- the Channel selector dialog box ------------- */

DIALOGBOX( ChSelectDB )
    DB_TITLE(        "Select Channel",    -1,-1,9,62)
    CONTROL(TEXT,         "1",      6, 1, 1, 23, ID_CH0)
    CONTROL(TEXT,         "2",     13, 1, 1, 23, ID_CH1)
    CONTROL(TEXT,         "3",     20, 1, 1, 23, ID_CH2)
    CONTROL(TEXT,         "4",     27, 1, 1, 23, ID_CH3)
    CONTROL(TEXT,         "5",     34, 1, 1, 23, ID_CH4)
    CONTROL(TEXT,         "6",     41, 1, 1, 23, ID_CH5)
    CONTROL(TEXT,         "7",     48, 1, 1, 23, ID_CH6)
    CONTROL(TEXT,         "8",     55, 1, 1, 23, ID_CH7)
    CONTROL(TEXT,         "9",      6, 3, 1, 23, ID_CH8)
    CONTROL(TEXT,         "10",     13, 3, 1, 23, ID_CH9)
    CONTROL(TEXT,         "11",    20, 3, 1, 23, ID_CH10)
    CONTROL(TEXT,         "12",    27, 3, 1, 23, ID_CH11)
    CONTROL(TEXT,         "13",    34, 3, 1, 23, ID_CH12)
    CONTROL(TEXT,         "14",    41, 3, 1, 23, ID_CH13)
    CONTROL(TEXT,         "15",    48, 3, 1, 23, ID_CH14)
    CONTROL(TEXT,         "16",    55, 3, 1, 23, ID_CH15)

    CONTROL(RADIOBUTTON,  NULL,     2, 1, 1,  3, ID_CH0)
    CONTROL(RADIOBUTTON,  NULL,     9, 1, 1,  3, ID_CH1)
    CONTROL(RADIOBUTTON,  NULL,    16, 1, 1,  3, ID_CH2)
    CONTROL(RADIOBUTTON,  NULL,    23, 1, 1,  3, ID_CH3)
    CONTROL(RADIOBUTTON,  NULL,    30, 1, 1,  3, ID_CH4)
    CONTROL(RADIOBUTTON,  NULL,    37, 1, 1,  3, ID_CH5)
    CONTROL(RADIOBUTTON,  NULL,    44, 1, 1,  3, ID_CH6)
    CONTROL(RADIOBUTTON,  NULL,    51, 1, 1,  3, ID_CH7)
    CONTROL(RADIOBUTTON,  NULL,     2, 3, 1,  3, ID_CH8)
    CONTROL(RADIOBUTTON,  NULL,     9, 3, 1,  3, ID_CH9)
    CONTROL(RADIOBUTTON,  NULL,    16, 3, 1,  3, ID_CH10)
    CONTROL(RADIOBUTTON,  NULL,    23, 3, 1,  3, ID_CH11)
    CONTROL(RADIOBUTTON,  NULL,    30, 3, 1,  3, ID_CH12)
    CONTROL(RADIOBUTTON,  NULL,    37, 3, 1,  3, ID_CH13)
    CONTROL(RADIOBUTTON,  NULL,    44, 3, 1,  3, ID_CH14)
    CONTROL(RADIOBUTTON,  NULL,    51, 3, 1,  3, ID_CH15)

    CONTROL(BUTTON, "   ~OK   ",    7, 5, 1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ",   19, 5, 1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",   31, 5, 1,  8, ID_HELP)
ENDDB


static int AddInstDo(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case COMMAND:
			if (p1==ID_OK) {
				GetItemText(wnd,ID_INSTNAME,InstName,40);
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

int getChSelection() {
	int i, rtn_chan=-1;
	for (i=ID_CH0;i<=ID_CH15;i++) {
		if (RadioButtonSetting(&ChSelectDB,i)) {
			rtn_chan = i - ID_CH0;
			break;
		}
	}
	return rtn_chan;
}

void AddInstBox(WINDOW wnd)
{
	if (num_inst<MAXCHAN) {
		if (DialogBox(wnd, &InstNameDB, TRUE, AddInstDo)) {
			/* Add a new instrument using InstName */
			if (*InstName) 
				strcpy(InstTbl[num_inst++], InstName);
		}
	}
}

/*******************************************************/
/* Destroy/Build a dialog box for instrument selection */
/*******************************************************/

void free_ISDB(void) {
	int i;
	if (ISDB) {
		for (i=0;i<num_inst;i++)
			if (ISDB->ctl[i].class==RADIOBUTTON) {
				if (ISDB->ctl[i].help)
					Mem_Free(ISDB->ctl[i].help);
			}
		Mem_Free(ISDB);
	}
	ISDB = NULL;
}

void build_ISDB(int redirect) {
	int i, j;
	char *hlp;
	free_ISDB();
	ISDB = Mem_Calloc(1,sizeof(DBOX),30);
	ISDB->HelpName="ISDB";
	ISDB->dwnd.title = "Select Instrument";
	ISDB->dwnd.x = -1;
	ISDB->dwnd.y = -1;
	ISDB->dwnd.w = 50;
	i = 0;
	while (i<num_inst) {
		hlp = Mem_Calloc(10,1,40);
		sprintf(hlp,"ID_INST%d",i);
		buildRadioButton(&ISDB->ctl[i],1,i+1,ID_INST0+i,hlp);
		i++;
	}
	if (redirect) buildRadioButton(&ISDB->ctl[i],1,i+1,ID_NOREDIRECT,"ID_NOREDIRECT");
	i = 0;
	j = num_inst + redirect;
	while (i<num_inst) {
	 	buildTextBox(&ISDB->ctl[i+j], InstTbl[i],5,i+1,40,ID_INST0+i,ISDB->ctl[i].help);
	 	i++;
	}
 	if (redirect) buildTextBox(&ISDB->ctl[i+j], "Play as recorded",5,i+1,40,ID_NOREDIRECT,"ID_NOREDIRECT");
	ISDB->dwnd.h = num_inst+6+redirect;
	j *= 2;
	if (chosen_inst[curtrack]!=-1) {
		ISDB->ctl[chosen_inst[curtrack]].isetting = 1;
	}
	buildButton(&ISDB->ctl[j+0],"~Ok",7,num_inst+3,ID_OK,"ID_OK");
	buildButton(&ISDB->ctl[j+1],"~Cancel",19,num_inst+3,ID_CANCEL,"ID_CANCEL");
	buildButton(&ISDB->ctl[j+2],"~Help",31,num_inst+3,ID_HELP,"ID_HELP");
}
	
/**************************************************/
/* Destroy/Build a dialog box for changing config */
/**************************************************/

int inchTmp, OutChansTmp[MAXCHAN];

void free_CCDB(void) {
	int i;
	if (CCDB) {
		for (i=0;i<num_inst;i++)
			if (CCDB->ctl[i].class==TEXT) {
				if (CCDB->ctl[i].help)
					Mem_Free(CCDB->ctl[i].help);
				if (CCDB->ctl[i].itext)
					Mem_Free(CCDB->ctl[i].itext);
				if (CCDB->ctl[num_inst+i].itext)
					Mem_Free(CCDB->ctl[num_inst+i].itext);
			}
		if (CCDB->ctl[2*num_inst].itext)
			Mem_Free(CCDB->ctl[2*num_inst].itext);
		if (CCDB->ctl[2*num_inst+2].itext)
			Mem_Free(CCDB->ctl[2*num_inst+2].itext);
		if (CCDB->ctl[2*num_inst+4].itext)
			Mem_Free(CCDB->ctl[2*num_inst+4].itext);
		if (CCDB->ctl[2*num_inst+6].itext)
			Mem_Free(CCDB->ctl[2*num_inst+6].itext);
		Mem_Free(CCDB);
	}
	CCDB = NULL;
}

static void build_CCDB(void) {
	int i;
	char *snp, *vel, *bpm, *met;
	free_CCDB();
	CCDB = Mem_Calloc(1,sizeof(DBOX),30);
	CCDB->HelpName="CCDB";
	CCDB->dwnd.title = "Change Config";
	CCDB->dwnd.x = -1;
	CCDB->dwnd.y = -1;
	CCDB->dwnd.w = 52;
	/* Build the list of instruments & channel assignments. If the
		user clicks on one of these, the channel selector must
		be popped up (and the text subsequently changed). */
	i = 0;
	while (i<num_inst) {
		char *hlp = Mem_Calloc(10,1,40),
			*nam = Mem_Calloc(5,1,41);
		sprintf(hlp,"ID_INST%d",i);
		if (OutChans[i]==-1)
			sprintf(nam,"(  )");
		else sprintf(nam,"(%2d)",OutChans[i]+1);
		OutChansTmp[i] = OutChans[i];
		buildTextBox(&CCDB->ctl[i],nam,5,i+1,4,ID_INST0+i,hlp);
		i++;
	}
	i = 0;
	while (i<num_inst) {
		char *nam = Mem_Calloc(40,1,41);
		sprintf(nam,"%-39s",InstTbl[i]);
		buildTextBox(&CCDB->ctl[num_inst+i],nam,10,i+1,40,ID_INST0+i,CCDB->ctl[i].help);
		i++;
	}
//	inchTmp = in_chan;
//	ich = Mem_Calloc(20,1,42);
//	sprintf(ich,"(%02d) Input Channel",in_chan+1);
//	buildTextBox(&CCDB->ctl[2*num_inst+2],ich,5,num_inst+2,14,ID_INCH,"ID_INCH");
	snp = Mem_Calloc(6,1,42);
	sprintf(snp,"%03d",timesnap);
	buildEditBox(&CCDB->ctl[2*num_inst],snp,5,num_inst+2,4,ID_TIMESNAP,"ID_TIMESNAP");
	buildTextBox(&CCDB->ctl[2*num_inst+1],"~Time Snap",10,num_inst+2,14,ID_TIMESNAP,"ID_TIMESNAP");

	vel = Mem_Calloc(6,1,43);
	sprintf(vel,"%03d",velocity);
	buildEditBox(&CCDB->ctl[2*num_inst+2],vel,5,num_inst+3,4,ID_VELOCITY,"ID_VELOCITY");
	buildTextBox(&CCDB->ctl[2*num_inst+3],"~Default Velocity",10,num_inst+3,14,ID_VELOCITY,"ID_VELOCITY");

	bpm = Mem_Calloc(6,1,44);
	sprintf(bpm,"%03d",meter);
	buildEditBox(&CCDB->ctl[2*num_inst+4],bpm,5,num_inst+4,4,ID_BPM,"ID_BPM");
	buildTextBox(&CCDB->ctl[2*num_inst+5],"~Beats per Measure",10,num_inst+4,14,ID_BPM,"ID_BPM");

	met = Mem_Calloc(6,1,45);
	sprintf(met,"%03d",metrate);
	buildEditBox(&CCDB->ctl[2*num_inst+6],met,5,num_inst+5,4,ID_METRATE,"ID_METRATE");
	buildTextBox(&CCDB->ctl[2*num_inst+7],"~Metronome Rate",10,num_inst+5,14,ID_METRATE,"ID_METRATE");

	buildTextBox(&CCDB->ctl[2*num_inst+8],"~Use Repetitions",10,num_inst+6,14,ID_USEREPS,"ID_USEREPS");
	buildCheckbox(&CCDB->ctl[2*num_inst+9], 5, num_inst+6, ID_USEREPS, "ID_USEREPS", usereps);

	buildTextBox(&CCDB->ctl[2*num_inst+10],"~Flush data",10,num_inst+7,14,ID_FLUSHMODE,"ID_FLUSHMODE");
	buildCheckbox(&CCDB->ctl[2*num_inst+11], 5, num_inst+7, ID_FLUSHMODE, "ID_FLUSHMODE", flushmode);

	buildButton(&CCDB->ctl[2*num_inst+12],"~Ok",7,num_inst+9,ID_OK,"ID_OK");
	buildButton(&CCDB->ctl[2*num_inst+13],"~Cancel",19,num_inst+9,ID_CANCEL,"ID_CANCEL");
	buildButton(&CCDB->ctl[2*num_inst+14],"~Help",31,num_inst+9,ID_HELP,"ID_HELP");
	CCDB->dwnd.h = num_inst+14;
}


static int CCDProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2) {
	int x,y;
	x = p1-wnd->rc.lf;
	y = p2-wnd->rc.tp-1;
	switch (msg)	{
		case COMMAND:
			if (p1==ID_OK) {
				char tmp[4];
				GetItemText(wnd,ID_TIMESNAP,tmp,4);
				timesnap = atoi(tmp);
				GetItemText(wnd,ID_VELOCITY,tmp,4);
				velocity = atoi(tmp);
				GetItemText(wnd,ID_BPM,tmp,4);
				meter = atoi(tmp);
				GetItemText(wnd,ID_METRATE,tmp,4);
				metrate = atoi(tmp);
			}
			break;
		case LEFT_BUTTON:
			if (x>5 && x<10 && y>0 && y<=num_inst) {
				int ln, oldch, newch;
				ln = y-1;
				oldch = OutChansTmp[ln];
				PushRadioButton(&ChSelectDB, oldch);
				if (DialogBox(wnd, &ChSelectDB, TRUE, NULL)) {
					if ((newch = getChSelection()) != -1) {
						OutChansTmp[ln] = newch;
						sprintf(CCDB->ctl[ln].itext,"(%2d)",newch+1);
						wputs(wnd,CCDB->ctl[ln].itext,6,ln+2);
					}
				}
				return TRUE;
//			} else if (x>5 && x<10 && y==(num_inst+2)) {
//				PushRadioButton(&ChSelectDB, inchTmp);
//				if (DialogBox(wnd, &ChSelectDB, TRUE, NULL))	{
//					int newch = getChSelection();
//					if (newch!=-1) {
//						inchTmp = newch;
//						sprintf(CCDB->ctl[2*num_inst].itext,"(%2d)",newch+1);
//						wputs(wnd,CCDB->ctl[2*num_inst].itext,6,num_inst+3);
//					}
//  /				}
//				return TRUE;
			} else break;
		default: break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

void CCDBox(WINDOW wnd) {
	build_CCDB();
	if (DialogBox(wnd, CCDB, TRUE, CCDProc))	{
		int i;
		usereps = CheckBoxSetting(CCDB,ID_USEREPS);
		flushmode = CheckBoxSetting(CCDB,ID_FLUSHMODE);
//		in_chan = inchTmp;
		for (i=0;i<MAXCHAN;i++) OutChans[i] = OutChansTmp[i];
	}
}
