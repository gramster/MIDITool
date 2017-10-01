/* ----------------- dialbox.c -------------- */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef MSC
#include <direct.h>
#else
#include <dir.h>
#endif
#include <dos.h>
#include <io.h>
#include "dflat.h"

static char path[MAXPATH];
static char drive[MAXDRIVE] = " :";
static char dir[MAXDIR];
static char name[MAXFILE];
static char ext[MAXEXT];

#ifdef INCLUDE_DIALOG_BOXES

static int inFocusCommand(DBOX *);
static void SetRadioButton(WINDOW, DBOX *, CTLWINDOW *);
static void InvertCheckBox(WINDOW, CTLWINDOW *);
static void dbShortcutKeys(WINDOW, DBOX *, int);
static int ControlProc(WINDOW, MESSAGE, PARAM, PARAM);
static void ChangeFocus(WINDOW, int);

#ifdef INCLUDE_SYSTEM_MENUS
static int SysMenuOpen = FALSE;
#endif

int DialogProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn;
	DBOX *db;
	CTLWINDOW *ct;

	db = wnd->extension;

	switch (msg)	{
		case SHIFT_CHANGED:
			return TRUE;
		case LEFT_BUTTON:
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowSizing || WindowMoving)
				return FALSE;
#endif
			if (HitControlBox(wnd, p1-GetLeft(wnd), p2-GetTop(wnd)))	{
				PostMessage(wnd, KEYBOARD, ' ', ALTKEY);
				return TRUE;
			}
			ct = db->ctl;
			while (ct->class)	{
				int mx = (int) p1 - GetClientLeft(wnd);
				int my = (int) p2 - GetClientTop(wnd);
				if (ct->class == RADIOBUTTON)	{
					if (my == ct->dwnd.y && mx == ct->dwnd.x+1)
						SetRadioButton(wnd, db, ct);
				}
				else if (ct->class == CHECKBOX)	{
					if (my == ct->dwnd.y && mx == ct->dwnd.x+1)
						InvertCheckBox(wnd, ct);
				}
				ct++;
			}
			break;
		case PAINT:
			if (!isVisible(wnd))
				break;
			if (!p2)
				BaseWndProc(DIALOG, wnd, msg, p1, p2);
			ct = p2 ? (CTLWINDOW *) p2 : db->ctl;
			while (ct->class)	{
				if (ct->class == RADIOBUTTON)	{
					char rb[] = "( )";
					if (ct->setting)
						rb[1] = 7;
					SetClassColors(DIALOG);
					writeline(wnd, rb, ct->dwnd.x+1, ct->dwnd.y+1, FALSE);
				}
				else if (ct->class == CHECKBOX)	{
					char rb[] = "[ ]";
					if (ct->setting)
						rb[1] = 'X';
					SetClassColors(DIALOG);
					writeline(wnd, rb, ct->dwnd.x+1, ct->dwnd.y+1, FALSE);
				}
				else if (ct->class == TEXT && ct->vtext != NULL)	{
					char *cp = ct->vtext;
					int len = min(ct->dwnd.h, MsgHeight(cp));
					int i;
					for (i = 0; i < len; i++)	{
						int mlen;
						char *txt = cp;
						char *cp1 = cp;
						char *np = strchr(cp, '\n');
						if (np != NULL)
							*np = '\0';
						mlen = strlen(cp);
						while ((cp1 = strchr(cp1, SHORTCUTCHAR)) != NULL)	{
							mlen += 3;
							cp1++;
						}
						if (np != NULL)
							*np = '\n';
						if ((txt = malloc(mlen+1)) != NULL)	{
			 				CopyCommand(txt, cp, FALSE, WndBackground(wnd));
							txt[mlen] = '\0';
							SetStandardColor(wnd);
							writeline(wnd, txt, ct->dwnd.x+1, ct->dwnd.y+i+1, FALSE);
							if ((cp = strchr(cp, '\n')) != NULL)
								cp++;
							free(txt);
						}
					}
				}
				ct++;
				if (p2)
					break;
			}
			return TRUE;
		case KEYBOARD:
			switch ((int)p1)	{
				case CTRL_FIVE:		/* same as SHIFT-TAB */
					if ((int)p2 & (LEFTSHIFT | RIGHTSHIFT))
						ChangeFocus(wnd, FALSE);
					break;
				case '\t':
					ChangeFocus(wnd, TRUE);
					break;
#ifdef INCLUDE_SYSTEM_MENUS
				case ' ':
					if (((int)p2 & ALTKEY) && TestAttribute(wnd, CONTROLBOX))	{
						wnd->dFocus = inFocus;
						SysMenuOpen = TRUE;
						BuildSystemMenu(wnd);
					}
					break;
#endif
				case CTRL_F4:
				case ESC:
#ifdef INCLUDE_SYSTEM_MENUS
					if (!(WindowMoving || WindowSizing))
#endif
						SendMessage(wnd, COMMAND, ID_CANCEL, 0);
#ifdef INCLUDE_SYSTEM_MENUS
				case UP:
				case DN:
				case BS:
				case FWD:
				case '\r':
					if (WindowMoving || WindowSizing)
						return BaseWndProc(DIALOG, wnd, msg, p1, p2);
					break;
#endif
				default:
					/* ------ search all the shortcut keys ----- */
					dbShortcutKeys(wnd, db, (int) p1);
					break;
			}
			if (wnd->Modal)
				return TRUE;
			break;
#ifdef INCLUDE_SYSTEM_MENUS
		case CLOSE_POPDOWN:
			SysMenuOpen = FALSE;
			break;
#endif
		case BUTTON_RELEASED:
#ifdef INCLUDE_SYSTEM_MENUS
			if (!WindowMoving && !WindowSizing)
				break;
#endif
			rtn = BaseWndProc(DIALOG, wnd, msg, p1, p2);
			SendMessage(wnd->dFocus, SETFOCUS, TRUE, 0);
			return rtn;
		case LB_SELECTION:
		case LB_CHOOSE:
#ifdef INCLUDE_SYSTEM_MENUS
			if (SysMenuOpen)
				return TRUE;
#endif
			SendMessage(wnd, COMMAND, inFocusCommand(db), msg);
			break;
		case COMMAND:
			switch ((int) p1)	{
				case ID_OK:
				case ID_CANCEL:
					wnd->ReturnCode = (int) p1;
					if (wnd->Modal)
						PostMessage(wnd, ENDDIALOG, 0, 0);
					else
						SendMessage(wnd, CLOSE_WINDOW, TRUE, 0);
					return TRUE;
#ifdef INCLUDE_HELP
				case ID_HELP:
					return DisplayHelp(wnd, db->HelpName);
#endif
				default:
					break;
			}
			break;
		case CLOSE_WINDOW:
			if (!p1)	{
				SendMessage(wnd, COMMAND, ID_CANCEL, 0);
				return TRUE;
			}
			break;
		default:
			break;
	}
	return BaseWndProc(DIALOG, wnd, msg, p1, p2);
}

int DialogBox(WINDOW wnd, DBOX *db, int Modal,
	int (*wndproc)(struct window *, enum messages, PARAM, PARAM))
{
	int rtn;
	WINDOW Dwnd;
	CTLWINDOW *ct;
	WINDOW oldFocus = inFocus;
	Dwnd = CreateWindow(DIALOG,
						db->dwnd.title,
						db->dwnd.x,
						db->dwnd.y,
						db->dwnd.h,
						db->dwnd.w,
						db,
						wnd,
						wndproc,
						Modal ? SAVESELF : 0);
	Dwnd->Modal = Modal;
	ct = db->ctl;
	while (ct->class)	{
		if (ct->class != TEXT &&
				ct->class != RADIOBUTTON &&
					ct->class != CHECKBOX)	{
			WINDOW cwnd;
			int attrib = VISIBLE | NOCLIP;
			ct->vtext = ct->itext;
			ct->setting = ct->isetting;
			if (ct->class == EDITBOX && ct->dwnd.h > 1)
				attrib |= (MULTILINE | HASBORDER);
			else if (ct->class == LISTBOX || ct->class == TEXTBOX)
				attrib |= HASBORDER;
			cwnd = CreateWindow(ct->class,
								ct->dwnd.title,
								ct->dwnd.x+GetClientLeft(Dwnd),
								ct->dwnd.y+GetClientTop(Dwnd),
								ct->dwnd.h,
								ct->dwnd.w,
								NULL,
								Dwnd,
								ControlProc,
								attrib);
			ct->wnd = cwnd;
			if (Dwnd->dFocus == NULLWND)
				Dwnd->dFocus = cwnd;
			if (ct->itext != NULL)	{
				unsigned char txt[SCREENWIDTH] = "";
				memset(txt, '\0', sizeof txt);
				if (ct->class == BUTTON && ct->setting == OFF)	{
					txt[0] = CHANGECOLOR;
					txt[1] = cfg.clr[BUTTON] [HILITE_COLOR] [FG] | 0x80;
					txt[2] = cfg.clr[BUTTON] [STD_COLOR] [BG] | 0x80;
				}
				CopyCommand(txt+strlen(txt), ct->itext, !ct->setting,
					WndBackground(cwnd));
				SendMessage(cwnd, ADDTEXT, (PARAM) txt, 0);
			}
		}
		else if (ct->class == TEXT && ct->itext != NULL)	{
			int len = strlen(ct->itext)+1;
			if ((ct->vtext = calloc(1, len)) != NULL)
				strncpy(ct->vtext, ct->itext, len);
		}
		else if (ct->class == RADIOBUTTON || ct->class == CHECKBOX)
			ct->setting = ct->isetting;
		ct++;
	}
	SendMessage(Dwnd->dFocus, SETFOCUS, TRUE, 0);
	SendMessage(Dwnd, SHOW_WINDOW, 0, 0);
	SendMessage(Dwnd, INITIATE_DIALOG, 0, 0);
	if (Modal)	{
		SendMessage(Dwnd, CAPTURE_MOUSE, 0, 0);
		SendMessage(Dwnd, CAPTURE_KEYBOARD, 0, 0);
		while (dispatch_message())
			;
		rtn = Dwnd->ReturnCode == ID_OK;
		SendMessage(Dwnd, RELEASE_MOUSE, 0, 0);
		SendMessage(Dwnd, RELEASE_KEYBOARD, 0, 0);
		SendMessage(Dwnd, CLOSE_WINDOW, TRUE, 0);
		SendMessage(oldFocus, SETFOCUS, TRUE, TRUE);
		if (rtn)	{
			ct = db->ctl;
			while (ct->class)	{
				if (ct->class == RADIOBUTTON || ct->class == CHECKBOX)
					ct->isetting = ct->setting;
				ct++;
			}
		}
		return rtn;
	}
	return FALSE;
}

static int inFocusCommand(DBOX *db)
{
	CTLWINDOW *ct = db->ctl;
	while (ct->class)	{
		if (ct->wnd == inFocus)
			return ct->command;
		ct++;
	}
	return -1;
}

static CTLWINDOW *FindCommand(DBOX *db, enum commands cmd, int class)
{
	CTLWINDOW *ct = db->ctl;
	while (ct->class)	{
		if (ct->class == class)
			if (cmd == ct->command)
				return ct;
		ct++;
	}
	return NULL;
}

WINDOW ControlWindow(DBOX *db, enum commands cmd)
{
	CTLWINDOW *ct = db->ctl;
	while (ct->class)	{
		if (ct->class != TEXT && cmd == ct->command)
			return ct->wnd;
		ct++;
	}
	return NULLWND;
}

CTLWINDOW *ControlBox(DBOX *db, WINDOW wnd)
{
	CTLWINDOW *ct = db->ctl;
	while (ct->class)	{
		if (ct->class != TEXT && wnd == ct->wnd)
			return ct;
		ct++;
	}
	return NULL;
}

void PushRadioButton(DBOX *db, enum commands cmd)
{
	CTLWINDOW *ct = FindCommand(db, cmd, RADIOBUTTON);
	if (ct != NULL)	{
		SetRadioButton(NULLWND, db, ct);
		ct->isetting = ON;
	}
}

#define MAXRADIOS 50

static struct {
	CTLWINDOW *rct;
} rbs[MAXRADIOS];
#ifdef ORIG
static void SetRadioButton(WINDOW wnd, DBOX *db, CTLWINDOW *ct)
{
	CTLWINDOW *ctt = db->ctl;
	int i;

	/* --- clear all the radio buttons
				in this group on the dialog box --- */

	/* -------- build a table of all radio buttons at the
			same x vector ---------- */
	for (i = 0; i < MAXRADIOS; i++)
		rbs[i].rct = NULL;
	while (ctt->class)	{
		if (ctt->class == RADIOBUTTON)
			if (ct->dwnd.x == ctt->dwnd.x)
				rbs[ctt->dwnd.y].rct = ctt;
		ctt++;
	}

	/* ----- find the start of the radiobutton group ---- */
	i = ct->dwnd.y;
	while (i >= 0 && rbs[i].rct != NULL)
		--i;
	/* ---- ignore everthing before the group ------ */
	while (i >= 0)
		rbs[i--].rct = NULL;

	/* ----- find the end of the radiobutton group ---- */
	i = ct->dwnd.y;
	while (i < MAXRADIOS && rbs[i].rct != NULL)
		i++;
	/* ---- ignore everthing past the group ------ */
	while (i < MAXRADIOS)
		rbs[i++].rct = NULL;

	for (i = 0; i < MAXRADIOS; i++)	{
		if (rbs[i].rct != NULL)	{
			rbs[i].rct->setting = OFF;
			if (wnd != NULLWND)
				SendMessage(wnd, PAINT, 0, (PARAM) rbs[i].rct);
		}
	}
	ct->setting = ON;
	if (wnd != NULLWND)
		SendMessage(wnd, PAINT, 0, (PARAM) ct);
}
#else

/* Grams version. Clears all radio buttons that exist in the array
   surrounding the target button. Does not look at coordinates */

static void SetRadioButton(WINDOW wnd, DBOX *db, CTLWINDOW *ct)
{
	CTLWINDOW *ctt = db->ctl, *first = NULL, *last = NULL;
	int i;

	/* --- clear all the radio buttons
	in this group on the dialog box --- */

	while (ctt->class)	{
		if (ctt->class == RADIOBUTTON) {
			if (first==NULL) first = ctt;
			if (ct==ctt) {
				while (ctt->class==RADIOBUTTON) {
					last = ctt;
					ctt++;
				}
				break;
			}
		} else first = NULL;
		ctt++;
	}
 	ctt = first;
	for (;;) {
		ctt->setting = OFF;
		if (wnd != NULLWND)
			SendMessage(wnd, PAINT, 0, (PARAM) ctt);
		if (ctt==last) break;
		ctt++;
	}
	ct->setting = ON;
	if (wnd != NULLWND)
		SendMessage(wnd, PAINT, 0, (PARAM) ct);
}
#endif
int RadioButtonSetting(DBOX *db, enum commands cmd)
{
	CTLWINDOW *ct = FindCommand(db, cmd, RADIOBUTTON);
	if (ct != NULL)
		return (ct->setting == ON);
	return FALSE;
}

static void ControlSetting(DBOX *db, enum commands cmd,
									int class, int setting)
{
	CTLWINDOW *ct = FindCommand(db, cmd, class);
	if (ct != NULL)
		ct->isetting = setting;
}

void SetCheckBox(DBOX *db, enum commands cmd)
{
	ControlSetting(db, cmd, CHECKBOX, ON);
}

void ClearCheckBox(DBOX *db, enum commands cmd)
{
	ControlSetting(db, cmd, CHECKBOX, OFF);
}

void EnableButton(DBOX *db, enum commands cmd)
{
	ControlSetting(db, cmd, BUTTON, ON);
}

void DisableButton(DBOX *db, enum commands cmd)
{
	ControlSetting(db, cmd, BUTTON, OFF);
}

int CheckBoxSetting(DBOX *db, enum commands cmd)
{
	CTLWINDOW *ct = FindCommand(db, cmd, CHECKBOX);
	if (ct != NULL)
		return (ct->isetting == ON);
	return FALSE;
}

static void InvertCheckBox(WINDOW wnd, CTLWINDOW *ct)
{
	ct->setting ^= ON;
	SendMessage(wnd, PAINT, 0, (PARAM) ct);
}

void PutItemText(WINDOW wnd, enum commands cmd, char *text)
{
	CTLWINDOW *ct = FindCommand(wnd->extension, cmd, EDITBOX);

	if (ct == NULL)
		ct = FindCommand(wnd->extension, cmd, TEXTBOX);
	if (ct == NULL)
		ct = FindCommand(wnd->extension, cmd, TEXT);
	if (ct != NULL)		{
		WINDOW cwnd = (WINDOW) (ct->wnd);
		MESSAGE msg = ADDTEXT;
		switch (ct->class)	{
			case EDITBOX:
				msg = EB_PUTTEXT;
			case TEXTBOX:
				SendMessage(cwnd, msg, (PARAM) text, 0);
				if (ct->class == EDITBOX && !isMultiLine(cwnd))
					SendMessage(cwnd, PAINT, 0, 0);
				if (cwnd->wlines > ClientHeight(cwnd) &&
						!TestAttribute(cwnd, VSCROLLBAR))	{
					AddAttribute(cwnd, VSCROLLBAR);
					SendMessage(cwnd, BORDER, 0, 0);
				}
				break;
			case TEXT:	{
				ct->vtext = realloc(ct->vtext, strlen(text)+1);
				if (ct->vtext != NULL)
					strcpy(ct->vtext, text);
				SendMessage(wnd, PAINT, 0, (PARAM) ct);
				break;
			}
			default:
				break;
		}
	}
}

char *GetEditBoxText(DBOX *db, enum commands cmd)
{
	CTLWINDOW *ct = FindCommand(db, cmd, EDITBOX);
	if (ct != NULL)
		return ct->itext;
	else
		return NULL;
}

void SetEditBoxText(DBOX *db, enum commands cmd, char *text)
{
	CTLWINDOW *ct = FindCommand(db, cmd, EDITBOX);
	if (ct != NULL)	{
		ct->itext = realloc(ct->itext, strlen(text)+1);
		if (ct->itext != NULL)
			strcpy(ct->itext, text);
	}
}

void GetItemText(WINDOW wnd, enum commands cmd, char *text, int len)
{
	CTLWINDOW *ct = FindCommand(wnd->extension, cmd, EDITBOX);

	if (ct == NULL)
		ct = FindCommand(wnd->extension, cmd, TEXTBOX);
	if (ct == NULL)
		ct = FindCommand(wnd->extension, cmd, TEXT);
	if (ct != NULL)	{
		WINDOW cwnd = (WINDOW) (ct->wnd);
		switch (ct->class)	{
			case TEXTBOX:
				if (cwnd != NULL)	{
					char *cp = GetText(cwnd);
					if (cp != NULL)
						strncpy(text, cp, len);
				}
				break;
			case EDITBOX:
				if (cwnd != NULL)
					SendMessage(cwnd, EB_GETTEXT, (PARAM) text, len);
				break;
			case TEXT:
				strncpy(text, ct->vtext, len);
				break;
			default:
				break;
		}
	}
}

void GetDlgListText(WINDOW wnd, char *text, enum commands cmd)
{
	CTLWINDOW *ct = FindCommand(wnd->extension, cmd, LISTBOX);
	int sel = SendMessage(ct->wnd, LB_CURRENTSELECTION, 0, 0);
	SendMessage(ct->wnd, LB_GETTEXT, (PARAM) text, sel);
}

static int dircmp(const void *c1, const void *c2)
{
	return stricmp(*(char **)c1, *(char **)c2);
}

int DlgDirList(WINDOW wnd, char *fspec,
				enum commands nameid, enum commands pathid,
				unsigned attrib)
{
	int ax, i = 0, criterr = 1;
	struct ffblk ff;
	CTLWINDOW *ct = FindCommand(wnd->extension, nameid, LISTBOX);
	WINDOW lwnd;
	char **dirlist = NULL;

	CreatePath(path, fspec, TRUE, TRUE);

	if (ct != NULL)	{
		lwnd = ct->wnd;
		SendMessage(ct->wnd, CLEARTEXT, 0, 0);

		if (attrib & 0x8000)	{
			union REGS regs;
			char drname[15];
			unsigned int cd, dr;

#ifdef MSC
			_dos_getdrive(&cd);
			cd -= 1;
#else
			cd = getdisk();
#endif
			for (dr = 0; dr < 26; dr++)	{
				unsigned ndr;
				setdisk(dr);
#ifdef MSC
				_dos_getdrive(&ndr);
				ndr -= 1;
#else
				ndr = getdisk();
#endif
				if (ndr == dr)	{
					/* ------- test for remapped B drive ------- */
					if (dr == 1)	{
						regs.x.ax = 0x440e;		/* IOCTL function 14 */
						regs.h.bl = dr+1;
						int86(DOS, &regs, &regs);
						if (regs.h.al != 0)
							continue;
					}

					sprintf(drname, "[%c:]", dr+'A');

					/* ------ test for network or RAM disk ---- */
					regs.x.ax = 0x4409;		/* IOCTL function 9 */
					regs.h.bl = dr+1;
					int86(DOS, &regs, &regs);
					if (!regs.x.cflag)	{
						if (regs.x.dx & 0x1000)
							strcat(drname, " (Network)");
						else if (regs.x.dx == 0x0800)
							strcat(drname, " (RAMdisk)");
					}
					SendMessage(lwnd, ADDTEXT, (PARAM) drname, 0);
				}
			}
			setdisk(cd);
		}

		while (criterr == 1)	{
			ax = findfirst(path, &ff, attrib & 0x3f);
			criterr = TestCriticalError();
		}
		if (criterr)
			return FALSE;
		while (ax == 0)	{
			if (!((attrib & 0x4000) &&
					(ff.ff_attrib & (attrib & 0x3f)) == 0) &&
						strcmp(ff.ff_name, "."))	{
				char fname[15];
				sprintf(fname, (ff.ff_attrib & 0x10) ?
								"[%s]" : "%s" , ff.ff_name);
				dirlist = realloc(dirlist, sizeof(char *)*(i+1));
				if (dirlist != NULL)	{
					dirlist[i] = malloc(strlen(fname)+1);
					if (dirlist[i] != NULL)
						strcpy(dirlist[i], fname);
					i++;
				}
			}
			ax = findnext(&ff);
		}
		if (dirlist != NULL)	{
			int j;
			/* -- sort file/drive/directory list box data -- */
			qsort(dirlist, i, sizeof(void *), dircmp);

			/* ---- send sorted list to list box ---- */
			for (j = 0; j < i; j++)	{
				SendMessage(lwnd, ADDTEXT, (PARAM) dirlist[j], 0);
				free(dirlist[j]);
			}
			free(dirlist);
		}
#ifdef INCLUDE_SCROLLBARS
		if (lwnd->wlines > ClientHeight(lwnd))
			AddAttribute(lwnd, VSCROLLBAR);
		else
			ClearAttribute(lwnd, VSCROLLBAR);
#endif
		SendMessage(lwnd, SHOW_WINDOW, 0, 0);
	}

	if (pathid)	{
		fnmerge(path, drive, dir, NULL, NULL);
		PutItemText(wnd, pathid, path);
	}

	return TRUE;
}

static void dbShortcutKeys(WINDOW wnd, DBOX *db, int ky)
{
	CTLWINDOW *ct;
	int ch = AltConvert(ky);

	if (ch != 0)	{
		ct = db->ctl;
		while (ct->class)	{
			char *cp = ct->vtext;
			while (cp && *cp)	{
				if (*cp == SHORTCUTCHAR && tolower(*(cp+1)) == ch)	{
					if (ct->class == TEXT)	{
						enum commands Tcmd = ct->command;
						/* --- find the associated control --- */
						ct = db->ctl;
						while (ct->class)	{
							if (ct->class != TEXT)
								if (ct->command == Tcmd)
									break;
							ct++;
						}
					}
					if (ct->class == RADIOBUTTON)
						SetRadioButton(wnd, db, ct);
					else if (ct->class == CHECKBOX)
						InvertCheckBox(wnd, ct);
					else if (ct->class)	{
						SendMessage(ct->wnd, SETFOCUS, TRUE, 0);
						if (ct->class == BUTTON)
							SendMessage(ct->wnd, KEYBOARD, '\r', 0);
					}
					return;
				}
				cp++;
			}
			ct++;
		}
	}
}

/* generic window processor used by all dialog box control windows */
static int ControlProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	DBOX *db;
	CTLWINDOW *ct;

	if (wnd == NULLWND)
		return FALSE;
	db = GetParent(wnd) ? GetParent(wnd)->extension : NULL;
	ct = db ? ControlBox(db, wnd) : NULL;
	switch (msg)	{
		case KEYBOARD:
			switch ((int) p1)	{
				case '\r':
					if (GetClass(wnd) == EDITBOX && !isMultiLine(wnd))	{
						SendMessage(GetParent(wnd), COMMAND, ID_OK, 0);
						return TRUE;
					}
					break;
#ifdef INCLUDE_HELP
				case F1:
#ifdef INCLUDE_SYSTEM_MENUS
					if (WindowMoving || WindowSizing)
						break;
#endif
					if (!DisplayHelp(wnd, ct->help))
						SendMessage(GetParent(wnd), COMMAND, ID_HELP, 0);
					return TRUE;
#endif
				default:
					break;
			}
			break;
		case SETFOCUS:
			if (GetClass(wnd) != BUTTON)	{
				if (p1)	{
					DefaultWndProc(wnd, msg, p1, p2);
					SendMessage(GetParent(wnd), COMMAND,
						inFocusCommand(db), ENTERFOCUS);
					return TRUE;
				}
				else 
					SendMessage(GetParent(wnd), COMMAND,
						inFocusCommand(db), LEAVEFOCUS);
			}
			break;
		case CLOSE_WINDOW:
			if (db != NULL && ct != NULL)	{
				WINDOW pwnd = GetParent(wnd);
				if (ct->vtext != NULL && ct->class == TEXT)	{
					free(ct->vtext);
					ct->vtext = NULL;
				}
				if (pwnd && pwnd->ReturnCode == ID_OK &&
								ct->class == EDITBOX) {
					if (wnd->TextChanged)	{
						int len = strlen(wnd->text);
						ct->itext = realloc(ct->itext, len+1);
						if (ct->itext != NULL)
							strcpy(ct->itext, wnd->text);
					}
				}
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

#endif

/* ----- Create unambiguous path from file spec, filling in the
     drive and directory if incomplete. Optionally change to
	 the new drive and subdirectory ------ */
void CreatePath(char *path, char *fspec, int InclName, int Change)
{
	int cm = 0;
	unsigned currdrive;
	char currdir[64];
	char *cp;

	if (!Change)	{
		/* ----- save the current drive and subdirectory ----- */
#ifdef MSC
		_dos_getdrive(&currdrive);
		currdrive -= 1;
#else
		currdrive = getdisk();
#endif
		getcwd(currdir, sizeof currdir);
		memmove(currdir, currdir+2, strlen(currdir+1));
		cp = currdir+strlen(currdir)-1;
		if (*cp == '\\')
			*cp = '\0';
	}

	*drive = *dir = *name = *ext = '\0';
#ifndef MSC
	cm =
#endif
	fnsplit(fspec, drive, dir, name, ext);
	if (!InclName)
		*name = *ext = '\0';
	*drive = toupper(*drive);

#ifdef MSC
	if (*ext)
		cm |= EXTENSION;
	if (InclName && *name)
		cm |= FILENAME;
	if (*dir)
		cm |= DIRECTORY;
	if (*drive)
		cm |= DRIVE;
#endif

	if (cm & DRIVE)
		setdisk(*drive - 'A');
	else 	{
#ifdef MSC
		_dos_getdrive((unsigned *)drive);
		*drive -= 1;
#else
		*drive = getdisk();
#endif
		*drive += 'A';
	}

	if (cm & DIRECTORY)	{
		cp = dir+strlen(dir)-1;
		if (*cp == '\\')
			*cp = '\0';
		chdir(dir);
	}
	getcwd(dir, sizeof dir);
	memmove(dir, dir+2, strlen(dir+1));

	if (InclName)	{
		if (!(cm & FILENAME))
			strcpy(name, "*");
		if (!(cm & EXTENSION))
			strcpy(ext, ".*");
	}
	else
		*name = *ext = '\0';
	if (dir[strlen(dir)-1] != '\\')
		strcat(dir, "\\");
	memset(path, 0, sizeof path);
	fnmerge(path, drive, dir, name, ext);

	if (!Change)	{
		setdisk(currdrive);
		chdir(currdir);
	}
}

static void ChangeFocus(WINDOW wnd, int direc)
{
	DBOX *db = wnd->extension;
 	CTLWINDOW *ct = db->ctl;
 	CTLWINDOW *ctt;

	while (ct->class)	{
		if (ct->wnd == inFocus)
			break;
		ct++;
	}
	if (ct->class)	{
		ctt = ct;
		do	{
			if (direc)	{
				ct++;
				if (ct->class == 0)
					ct = db->ctl;
			}
			else	{
				if (ct == db->ctl)
					while (ct->class)
						ct++;
				--ct;
			}
			if (ct->wnd != NULLWND)	{
				SendMessage(ct->wnd, SETFOCUS, TRUE, 0);
				break;
			}
		} while (ct != ctt);
	}
}

