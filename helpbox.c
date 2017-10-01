/* ------------ helpbox.c ----------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <ctype.h>
#include "dflat.h"
#include "htree.h"

#ifdef INCLUDE_HELP

extern DBOX HelpBox;


/* -------- strings of D-Flat classes for calling default
      help text collections -------- */
char *ClassNames[] = {
	#undef ClassDef
	#define ClassDef(c,b,p,a) #c,
	#include "classes.h"
	NULL
};

#define MAXHEIGHT (SCREENHEIGHT-10)

/* --------- linked list of help text collections -------- */
struct helps {
    char *hname;
	char *NextName;
	char *PrevName;
    long hptr;
	int bit;
	int hheight;
	int hwidth;
	WINDOW hwnd;
	struct helps *NextHelp;
};
static struct helps *FirstHelp = NULL;
static struct helps *LastHelp = NULL;
static struct helps *ThisHelp = NULL;

/* --- linked stack of help windows that have beed used --- */
struct HelpStack {
	char *hname;
	struct HelpStack *PrevStack;
};
static struct HelpStack *LastStack;
static struct HelpStack *ThisStack;

/* --- linked list of keywords in the current help
           text collection (listhead is in window) -------- */
struct keywords {
	char *hname;
	int lineno;
	int off1, off2, off3;
	int isDefinition;
	struct keywords *nextword;
	struct keywords *prevword;
};

static FILE *helpfp;
static char hline [160];
static int Helping;

static void SelectHelp(WINDOW, char *);
static void ReadHelp(WINDOW);
static void FindHelp(char *);
static void FindHelpWindow(WINDOW);
static void DisplayDefinition(WINDOW, char *);
static void BestFit(WINDOW, DIALOGWINDOW *);

int HelpBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	DBOX *db = wnd->extension;
	WINDOW cwnd;
	struct keywords *thisword;
	static char HelpName[50];

	switch (msg)	{
		case CREATE_WINDOW:
			Helping = TRUE;
			GetClass(wnd) = HELPBOX;
			if (ThisHelp != NULL)
				ThisHelp->hwnd = wnd;
			break;
		case INITIATE_DIALOG:
			ReadHelp(wnd);
			break;
		case COMMAND:
			switch ((int)p1)	{
				case ID_CANCEL:
					ThisStack = LastStack;
					while (ThisStack != NULL)	{
						LastStack = ThisStack->PrevStack;
						if (ThisStack->hname != NULL)
							free(ThisStack->hname);
						free(ThisStack);
						ThisStack = LastStack;
					}
					break;
				case ID_PREV:
					FindHelpWindow(wnd);
					if (ThisHelp != NULL)
						SelectHelp(wnd, ThisHelp->PrevName);
					return TRUE;
				case ID_NEXT:
					FindHelpWindow(wnd);
					if (ThisHelp != NULL)
						SelectHelp(wnd, ThisHelp->NextName);
					return TRUE;
				case ID_BACK:
					if (LastStack != NULL)	{
						if (LastStack->PrevStack != NULL)	{
							ThisStack = LastStack->PrevStack;
							if (LastStack->hname != NULL)
								free(LastStack->hname);
							free(LastStack);
							LastStack = ThisStack;
							SelectHelp(wnd, ThisStack->hname);
						}
					}
					return TRUE;
				default:
					break;
			}
			break;
		case KEYBOARD:
			if (WindowMoving)
				break;
			cwnd = ControlWindow(wnd->extension, ID_HELPTEXT);
			if (cwnd == NULLWND || inFocus != cwnd)
				break;
			thisword = cwnd->thisword;
			switch ((int)p1)	{
				case '\r':
					if (thisword != NULL)	{
						if (thisword->isDefinition)
							DisplayDefinition(GetParent(wnd), thisword->hname);
						else	{
							strncpy(HelpName, thisword->hname,
								sizeof HelpName);
							SelectHelp(wnd, HelpName);
						}
					}
					return TRUE;
				case '\t':
					if (thisword == NULL)
						thisword = cwnd->firstword;
					else {
						if (thisword->nextword == NULL)
							thisword = cwnd->firstword;
						else
							thisword = thisword->nextword;
					}
					break;
				case SHIFT_HT:
					if (thisword == NULL)
						thisword = cwnd->lastword;
					else {
						if (thisword->prevword == NULL)
							thisword = cwnd->lastword;
						else
							thisword = thisword->prevword;
					}
					break;
				default:
					thisword = NULL;
					break;
			}
			if (thisword != NULL)	{
				cwnd->thisword = thisword;
				if (thisword->lineno < cwnd->wtop ||
						thisword->lineno >= cwnd->wtop + ClientHeight(cwnd))	{
					int distance = ClientHeight(cwnd)/2;
					do	{
						cwnd->wtop = thisword->lineno - distance;
						distance /= 2;
					}
					while (cwnd->wtop < 0);
				}
				SendMessage(cwnd, PAINT, 0, 0);
				return TRUE;
			}
			break;
		case CLOSE_WINDOW:
			if (db != NULL)	{
				if (db->dwnd.title != NULL)	{
					free(db->dwnd.title);
					db->dwnd.title = NULL;
				}
			}
			FindHelpWindow(wnd);
			if (ThisHelp != NULL)
				ThisHelp->hwnd = NULLWND;
			Helping = FALSE;
			break;
		default:
			break;
	}
	return BaseWndProc(HELPBOX, wnd, msg, p1, p2);
}

static void SelectHelp(WINDOW wnd, char *hname)
{
	if (hname != NULL)	{
		WINDOW pwnd = GetParent(wnd);
		PostMessage(wnd, ENDDIALOG, 0, 0);
		PostMessage(pwnd, DISPLAY_HELP, (PARAM) hname, 0);
	}
}

int HelpTextProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	struct keywords *thisword;
	int rtn, mx, my;
	switch (msg)	{
		case PAINT:
			if (wnd->thisword != NULL)	{
				char *cp;
				thisword = wnd->thisword;
				cp = TextLine(wnd, thisword->lineno);
				cp += thisword->off1;
				*(cp+1) = (cfg.clr[HELPBOX] [SELECT_COLOR] [FG] & 255) | 0x80;
				*(cp+2) = (cfg.clr[HELPBOX] [SELECT_COLOR] [BG] & 255) | 0x80;
				rtn = DefaultWndProc(wnd, msg, p1, p2);
				*(cp+1) = (cfg.clr[HELPBOX] [HILITE_COLOR] [FG] & 255) | 0x80;
				*(cp+2) = (cfg.clr[HELPBOX] [HILITE_COLOR] [BG] & 255) | 0x80;
				return rtn;
			}
			break;
		case LEFT_BUTTON:
			rtn = DefaultWndProc(wnd, msg, p1, p2);
			mx = (int)p1 - GetClientLeft(wnd);
			my = (int)p2 - GetClientTop(wnd);
			my += wnd->wtop;
			thisword = wnd->firstword;
			while (thisword != NULL)	{
				if (my == thisword->lineno)	{
					if (mx >= thisword->off2 && mx < thisword->off3)	{
						wnd->thisword = thisword;
						SendMessage(wnd, PAINT, 0, 0);
						if (thisword->isDefinition)	{
							WINDOW pwnd = GetParent(wnd);
							if (pwnd != NULLWND)
								DisplayDefinition(GetParent(pwnd),
									thisword->hname);
						}
						break;
					}
				}
				thisword = thisword->nextword;
			}
			return rtn;
		case DOUBLE_CLICK:
			PostMessage(wnd, KEYBOARD, '\r', 0);
			break;
		case CLOSE_WINDOW:
			thisword = wnd->firstword;
			while (thisword != NULL)	{
				struct keywords *nextword = thisword->nextword;
				if (thisword->hname != NULL)
					free(thisword->hname);
				free(thisword);
				thisword = nextword;
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

static void *fgetshelp(void)
{
	void *fg;
	do
		if ((fg = GetHelpLine(hline)) == NULL)
			break;
	while (*hline == ';');	/* bypassing all comments */
	return fg;
}

static void ReadHelp(WINDOW wnd)
{
	WINDOW cwnd = ControlWindow(wnd->extension, ID_HELPTEXT);
	int linectr = 0;
	if (cwnd == NULLWND)
		return;
	cwnd->wndproc = HelpTextProc;
	/* ----- read the help text ------- */
	while (TRUE)	{
		unsigned char *cp = hline, *cp1;
		int colorct = 0;
        if (fgetshelp() == NULL)
			break;
		if (*hline == '<')
			break;
		hline[strlen(hline)-1] = '\0';
		/* --- add help text to the help window --- */
		while (cp != NULL)	{
			if ((cp = strchr(cp, '[')) != NULL)	{
				/* ----- hit a new key word ----- */
				struct keywords *thisword;
				if (*(cp+1) != '.' && *(cp+1) != '*')	{
					cp++;
					continue;
				}
				thisword = calloc(1, sizeof(struct keywords));
				if (thisword != NULL)	{
					if (cwnd->firstword == NULL)
						cwnd->firstword = thisword;
					if (cwnd->lastword != NULL)	{
						((struct keywords *)
							(cwnd->lastword))->nextword = thisword;
						thisword->prevword = cwnd->lastword;
					}
					cwnd->lastword = thisword;
					thisword->lineno = cwnd->wlines;
					thisword->off1 = (int) (cp - hline);
					thisword->off2 = thisword->off1 - colorct * 4;
					thisword->isDefinition = *(cp+1) == '*';
				}
				colorct++;
				*cp++ = CHANGECOLOR;
				*cp++ = (cfg.clr[HELPBOX] [HILITE_COLOR] [FG] & 255) | 0x80;
				*cp++ = (cfg.clr[HELPBOX] [HILITE_COLOR] [BG] & 255) | 0x80;
				cp1 = cp;
				if ((cp = strchr(cp, ']')) != NULL)	{
					if (thisword != NULL)
						thisword->off3 = thisword->off2 + (int) (cp - cp1);
					*cp++ = RESETCOLOR;
				}
				if ((cp = strchr(cp, '<')) != NULL)	{
					char *cp1 = strchr(cp, '>');
					if (cp1 != NULL)	{
						int len = (int) (cp1 - cp);
						if ((thisword->hname = calloc(1, len)) != NULL)	
							strncpy(thisword->hname, cp+1, len-1);
						memmove(cp, cp1+1, strlen(cp1));
					}
				}
			}
		}
		PutItemText(wnd, ID_HELPTEXT, hline);
		if (++linectr == ClientHeight(cwnd))
			SendMessage(cwnd, PAINT, 0, 0);
	}
}

static int HelpLength(char *s)
{
	int len = strlen(s);
	char *cp = strchr(s, '[');
	while (cp != NULL)	{
		len -= 4;
		cp = strchr(cp+1, '[');
	}
	cp = strchr(s, '<');
	while (cp != NULL)	{
		char *cp1 = strchr(cp, '>');
		if (cp1 != NULL)
			len -= (int) (cp1-cp)+1;
		cp = strchr(cp1, '<');
	}
	return len;
}

/* ----------- load the help text file ------------ */
void LoadHelpFile()
{
    char *cp;

	if (Helping)
		return;
	UnLoadHelpFile();
    if ((helpfp = OpenHelpFile()) == NULL)
        return;
	*hline = '\0';
	while (*hline != '<')	{
	    if (fgetshelp() == NULL)	{
			fclose(helpfp);
   	    	return;
		}
	}
    while (*hline == '<')   {
        if (strncmp(hline, "<end>", 5) == 0)
            break;

		if ((ThisHelp = calloc(1, sizeof(struct helps))) == NULL)
			break;

		if (FirstHelp == NULL)
			FirstHelp = ThisHelp;

		/* -------- parse the help window's text name ----- */
		if ((cp = strchr(hline, '>')) == NULL)
			continue;
		*cp = '\0';
		if ((ThisHelp->hname=malloc(strlen(hline+1)+1))==NULL)
			break;
        strcpy(ThisHelp->hname, hline+1);

		HelpFilePosition(&ThisHelp->hptr, &ThisHelp->bit);

	    if (fgetshelp() == NULL)
			break;

		/* ------- build the help linked list entry --- */
		while (*hline == '[')	{
			HelpFilePosition(&ThisHelp->hptr, &ThisHelp->bit);
			/* ------ parse the <<prev button pointer ------- */
			if (strncmp(hline, "[<<]", 4) == 0)	{
				char *cp = strchr(hline+4, '<');
				if (cp != NULL)	{
					char *cp1 = strchr(cp, '>');
					if (cp1 != NULL)	{
						int len = (int) (cp1-cp);
						ThisHelp->PrevName = calloc(1, len);
						if (ThisHelp->PrevName != NULL)
							strncpy(ThisHelp->PrevName, cp+1, len-1);
					}
				}
			    if (fgetshelp() == NULL)
					break;
				continue;
			}
			/* ------ parse the next>> button pointer ------- */
			else if (strncmp(hline, "[>>]", 4) == 0)	{
				char *cp = strchr(hline+4, '<');
				if (cp != NULL)	{
					char *cp1 = strchr(cp, '>');
					if (cp1 != NULL)	{
						int len = (int) (cp1-cp);
						ThisHelp->NextName = calloc(1, len);
						if (ThisHelp->NextName != NULL)
							strncpy(ThisHelp->NextName, cp+1, len-1);
					}
				}
			    if (fgetshelp() == NULL)
					break;
				continue;
			}
			else
				break;
		}
		ThisHelp->hheight = 0;
		ThisHelp->hwidth = 0;
		ThisHelp->NextHelp = NULL;

		/* ------ append entry to the linked list ------ */
		if (LastHelp != NULL)
			LastHelp->NextHelp = ThisHelp;
		LastHelp = ThisHelp;

		/* -------- move to the next <> token ------ */
        if (fgetshelp() == NULL)
            strcpy(hline, "<end>");
        while (*hline != '<')	{
			ThisHelp->hwidth =
				max(ThisHelp->hwidth, HelpLength(hline));
			ThisHelp->hheight++;
	        if (fgetshelp() == NULL)
    	        strcpy(hline, "<end>");
		}
    }
	fclose(helpfp);
}

/* ------ free the memory used by the help file table ------ */
void UnLoadHelpFile(void)
{
	while (FirstHelp != NULL)	{
		ThisHelp = FirstHelp;
		if (ThisHelp->hname != NULL)
			free(ThisHelp->hname);
		if (ThisHelp->PrevName != NULL)
			free(ThisHelp->PrevName);
		if (ThisHelp->NextName != NULL)
			free(ThisHelp->NextName);
		FirstHelp = ThisHelp->NextHelp;
		free(ThisHelp);
	}
	ThisHelp = LastHelp = NULL;
}

/* ------------ display help text ----------- */
int DisplayHelp(WINDOW wnd, char *Help)
{
	if (Helping)
		return TRUE;
	FindHelp(Help);
    if (ThisHelp != NULL)    {
		if (LastStack == NULL || stricmp(Help, LastStack->hname))	{
			ThisStack = calloc(1,sizeof(struct HelpStack));
			if (ThisStack != NULL)	{
				ThisStack->hname = malloc(strlen(Help)+1);
				if (ThisStack->hname != NULL)
					strcpy(ThisStack->hname, Help);
				ThisStack->PrevStack = LastStack;
				LastStack = ThisStack;
			}
		}
	    if ((helpfp = OpenHelpFile()) != NULL)	{
			DBOX *db;
			int offset, i;

			if ((db = calloc(1,sizeof HelpBox)) != NULL)	{
				memcpy(db, &HelpBox, sizeof HelpBox);
				SeekHelpLine(ThisHelp->hptr, ThisHelp->bit);
       			fgetshelp();
				hline[strlen(hline)-1] = '\0';
				if ((db->dwnd.title = malloc(strlen(hline)+1)) != NULL)
					strcpy(db->dwnd.title, hline);
				db->dwnd.h = min(ThisHelp->hheight, MAXHEIGHT)+7;
				db->dwnd.w = max(45, ThisHelp->hwidth+6);
				BestFit(wnd, &db->dwnd);
				db->ctl[0].dwnd.w = max(40, ThisHelp->hwidth+2);
				db->ctl[0].dwnd.h = min(ThisHelp->hheight, MAXHEIGHT)+2;
				offset = (db->dwnd.w-40) / 2;
				for (i = 1; i < 5; i++)	{
					db->ctl[i].dwnd.y = min(ThisHelp->hheight, MAXHEIGHT)+3;
					db->ctl[i].dwnd.x += offset;
				}
				if (ThisStack != NULL)
					if (ThisStack->PrevStack == NULL)
						DisableButton(db, ID_BACK);
				if (ThisHelp->NextName == NULL)
					DisableButton(db, ID_NEXT);
				if (ThisHelp->PrevName == NULL)
					DisableButton(db, ID_PREV);
				DialogBox(wnd, db, TRUE, HelpBoxProc);
			}
			fclose(helpfp);
			return TRUE;
		}
    }
	return FALSE;
}

static void DisplayDefinition(WINDOW wnd, char *def)
{
	WINDOW dwnd;
	WINDOW hwnd = wnd;
	int y;

	if (GetClass(wnd) == POPDOWNMENU)
		hwnd = GetParent(wnd);
	y = GetClass(hwnd) == MENUBAR ? 2 : 1;
	FindHelp(def);
	if (ThisHelp != NULL)	{
		clearBIOSbuffer();
	    if ((helpfp = OpenHelpFile()) != NULL)	{
			clearBIOSbuffer();
			dwnd = CreateWindow(TEXTBOX,
							   NULL,
							   GetClientLeft(hwnd),
							   GetClientTop(hwnd)+y,
							   min(ThisHelp->hheight, MAXHEIGHT)+3,
							   ThisHelp->hwidth+2,
							   NULL,
							   wnd,
							   NULL,
							   HASBORDER | NOCLIP | SAVESELF);
			if (dwnd != NULLWND)	{
				clearBIOSbuffer();
				/* ----- read the help text ------- */
				SeekHelpLine(ThisHelp->hptr, ThisHelp->bit);
				while (TRUE)	{
					clearBIOSbuffer();
        			if (fgetshelp() == NULL)
						break;
					if (*hline == '<')
						break;
					hline[strlen(hline)-1] = '\0';
					SendMessage(dwnd, ADDTEXT, (PARAM) hline, 0);
				}
				SendMessage(dwnd, SHOW_WINDOW, 0, 0);
				SendMessage(NULLWND, WAITKEYBOARD, 0, 0);
				SendMessage(NULLWND, WAITMOUSE, 0, 0);
				SendMessage(dwnd, CLOSE_WINDOW, 0, 0);
			}
			fclose(helpfp);
		}
	}
}

static int wildcmp(char *s1, char *s2)
{
	while (*s1 || *s2)	{
		if (tolower(*s1) != tolower(*s2))
			if (*s1 != '?' && *s2 != '?')
				return 1;
		s1++, s2++;
	}
	return 0;
}

static void FindHelp(char *Help)
{
	ThisHelp = FirstHelp;
	while (ThisHelp != NULL)	{
        if (wildcmp(Help, ThisHelp->hname) == 0)
            break;
		ThisHelp = ThisHelp->NextHelp;
	}
}

static void FindHelpWindow(WINDOW wnd)
{
	ThisHelp = FirstHelp;
	while (ThisHelp != NULL)	{
        if (wnd == ThisHelp->hwnd)
            break;
		ThisHelp = ThisHelp->NextHelp;
	}
}

static int OverLap(int a, int b)
{
	int ov = a - b;
	if (ov < 0)
		ov = 0;
	return ov;
}

/* ------ compute the best location for a dialog box ----- */
static void BestFit(WINDOW wnd, DIALOGWINDOW *dwnd)
{
	int above, below, right, left;
	if (GetClass(wnd) == MENUBAR || GetClass(wnd) == APPLICATION)	{
		dwnd->x = dwnd->y = -1;
		return;
	}
	/* --- compute above overlap ---- */
	above = OverLap(dwnd->h, GetTop(wnd));
	/* --- compute below overlap ---- */
	below = OverLap(GetBottom(wnd), SCREENHEIGHT-dwnd->h);
	/* --- compute right overlap ---- */
	right = OverLap(GetRight(wnd), SCREENWIDTH-dwnd->w);
	/* --- compute left  overlap ---- */
	left = OverLap(dwnd->w, GetLeft(wnd));

	if (above < below)
		dwnd->y = max(0, GetTop(wnd)-dwnd->h-2);
	else
		dwnd->y = min(SCREENHEIGHT-dwnd->h, GetBottom(wnd)+2);
	if (right < left)
		dwnd->x = min(GetRight(wnd)+2, SCREENWIDTH-dwnd->w);
	else
		dwnd->x = max(0, GetLeft(wnd)-dwnd->w-2);

	if (dwnd->x == GetRight(wnd)+2 || dwnd->x == GetLeft(wnd)-dwnd->w-2)
		dwnd->y = -1;
	if (dwnd->y ==GetTop(wnd)-dwnd->h-2 || dwnd->y == GetBottom(wnd)+2)
		dwnd->x = -1;
}

#endif
