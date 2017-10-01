/* ---------------- menubar.c -------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include "dflat.h"

static void reset_menubar(WINDOW);
static int TestGlobalKeys(WINDOW, PARAM, PARAM);
static void SkipSystemWindows(void);

static struct {
	int x1, x2;		/* position in menu bar */
	char sc;		/* shortcut key value   */
} menu[10];
static int mctr;

MENU *ActiveMenu = NULL;
int ActiveSelection = -1;
WINDOW MenuBar = NULLWND;

static WINDOW mwnd = NULL;
static int Selecting = FALSE;

int MenuBarProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int offset = 3, wd, offset1, i, j;
	MENU *mnu;
	int rtn;
	static AltDown = FALSE;

	switch (msg)	{
		case CREATE_WINDOW:
			MenuBar = wnd;
			reset_menubar(wnd);
			break;
		case SETFOCUS:
			rtn = BaseWndProc(MENUBAR, wnd, msg, p1, p2);
			if ((int) p1)	{
				if (ActiveSelection == -1)
					ActiveSelection = 0;
				if (inFocus == wnd)
					SendMessage(wnd, PAINT, 0, 0);
			}
			else	{
				SendMessage(wnd, PAINT, 0, 0);
#ifdef INCLUDE_STATUSBAR
				SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
#endif
			}
			return rtn;
		case COMMAND:
			ActiveSelection = -1;
			SetPrevFocus(wnd);
			PostMessage(GetParent(wnd), msg, p1, p2);
			return TRUE;
		case BUILDMENU:
			reset_menubar(wnd);
			mctr = 0;
			ActiveMenu = (MENU *) p1;
			while (ActiveMenu->Title != NULL)	{
				char *cp;
				GetText(wnd) = realloc(GetText(wnd), strlen(GetText(wnd))+5);
				memmove(GetText(wnd) + offset+4, GetText(wnd) + offset, strlen(GetText(wnd))-offset+1);
				CopyCommand(GetText(wnd)+offset, ActiveMenu->Title, FALSE,
					cfg.clr [MENUBAR] [STD_COLOR] [BG]);
				menu[mctr].x1 = offset;
				offset += strlen(ActiveMenu->Title) + (3+MSPACE);
				menu[mctr].x2 = offset-MSPACE;
				cp = strchr(ActiveMenu->Title, SHORTCUTCHAR);
				if (cp)
					menu[mctr].sc = tolower(*(cp+1));
				ActiveMenu++;
				mctr++;
			}
			ActiveMenu = (MENU *) p1;
			break;
		case PAINT:	
			if (!isVisible(wnd))
				break;

#ifdef INCLUDE_STATUSBAR
			if (wnd == inFocus)
				SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
#endif
			SetStandardColor(wnd);
			ClearAttribute(wnd, NOCLIP);

	        clipline(wnd, 0, GetText(wnd));
    	    wputs(wnd, GetText(wnd), 0, 0);

			AddAttribute(wnd, NOCLIP);
			if (ActiveSelection != -1 &&
					(wnd == inFocus || mwnd != NULLWND))	{
				char sel[80], *cp;
				offset = menu[ActiveSelection].x1;
				offset1 = menu[ActiveSelection].x2;
				GetText(wnd)[offset1] = '\0';
				SetReverseColor(wnd);
				memset(sel, '\0', sizeof sel);
				strcpy(sel, GetText(wnd)+offset);
				cp = strchr(sel, CHANGECOLOR);
				if (cp != NULL)
					*(cp + 2) = background | 0x80;
				wputs(wnd, sel, offset-ActiveSelection*4, 0);
				GetText(wnd)[offset1] = ' ';
#ifdef INCLUDE_STATUSBAR
				if (!Selecting && mwnd == NULLWND && wnd == inFocus)	{
					char *st = ActiveMenu[ActiveSelection].StatusText;
					if (st != NULL)
						SendMessage(GetParent(wnd), ADDSTATUS, (PARAM)st, 0);
				}
#endif
			}
			return FALSE;
		case SHIFT_CHANGED:
			if (mwnd == NULLWND)	{
				if ((int)p1 & ALTKEY)
					AltDown = TRUE;
				else if (AltDown)
					SendMessage(wnd, KEYBOARD,
						wnd == inFocus ? ESC : F10, 0);
			}
			return TRUE;
		case KEYBOARD:
			AltDown = FALSE;
			if (mwnd == NULLWND)	{
				/* ----- search for menu bar shortcut keys ---- */
				int c = tolower((int)p1);
				int a = AltConvert((int)p1);
				for (j = 0; j < mctr; j++)	{
					if ((inFocus == wnd && menu[j].sc == c) ||
							(a && menu[j].sc == a))	{
						if (inFocus != wnd)
							SendMessage(wnd, SETFOCUS, TRUE, 0);
						SendMessage(wnd, SELECTION, j, 0);
						return FALSE;
					}
				}
			}
			/* -------- search for accelerator keys -------- */
			mnu = ActiveMenu;
			while (mnu->Title != NULL)	{
				struct PopDown *pd = mnu->Selections;
				if (mnu->PrepMenu)
					(*(mnu->PrepMenu))(GetParent(wnd), mnu);
				while (pd->SelectionTitle != NULL)	{
					if (pd->Accelerator == (int) p1)	{
						if (pd->Attrib & INACTIVE)
							beep();
						else	{
							if (GetClass(inFocus) == MENUBAR)
								SetPrevFocus(inFocus);
							PostMessage(GetParent(wnd),
								COMMAND, pd->ActionId, 0);
						}
						return TRUE;
					}
					pd++;
				}
				mnu++;
			}
			if (TestGlobalKeys(wnd, p1, p2))
				return TRUE;

			switch ((int)p1)	{
#ifdef INCLUDE_HELP
				case F1:
				  if (ActiveMenu != NULL &&
					  (mwnd == NULLWND ||
						(ActiveMenu+ActiveSelection)->
							Selections[0].SelectionTitle == NULL)) {
						DisplayHelp(wnd, (ActiveMenu+ActiveSelection)->Title+1);
						return TRUE;
					}
					break;
#endif
				case '\r':
					if (mwnd == NULLWND && ActiveSelection != -1)
						SendMessage(wnd, SELECTION, ActiveSelection, 0);
					break;
				case ESC:
					if (inFocus == wnd)	{
						ActiveSelection = -1;
						SetPrevFocus(wnd);
						SendMessage(wnd, PAINT, 0, 0);
					}
					break;
				case FWD:
					ActiveSelection++;
					if (ActiveSelection == mctr)
						ActiveSelection = 0;
					if (mwnd != NULLWND)
						SendMessage(wnd, SELECTION, ActiveSelection, 0);
					else 
						SendMessage(wnd, PAINT, 0, 0);
					break;
				case BS:
					if (ActiveSelection == 0)
						ActiveSelection = mctr;
					--ActiveSelection;
					if (mwnd != NULLWND)
						SendMessage(wnd, SELECTION, ActiveSelection, 0);
					else 
						SendMessage(wnd, PAINT, 0, 0);
					break;
				default:
					break;
			}
			return FALSE;
		case LEFT_BUTTON:
			i = BarSelection((int) p1 - GetLeft(wnd));
			if (i < mctr)
				if (i != ActiveSelection || mwnd == NULLWND)
					SendMessage(wnd, SELECTION, i, 0);
			return TRUE;
		case SELECTION:
			Selecting = TRUE;
			if (mwnd != NULLWND)
				SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
			mwnd = NULLWND;
			ActiveSelection = (int) p1;
			offset = menu[ActiveSelection].x1 -
						4 * ActiveSelection;
			mnu = ActiveMenu+ActiveSelection;
			if (mnu->PrepMenu != NULL)
				(*(mnu->PrepMenu))(GetParent(wnd), mnu);
			wd = MenuWidth(mnu->Selections);
			if (offset > WindowWidth(wnd)-wd)
				offset = WindowWidth(wnd)-wd;
			mwnd = CreateWindow(POPDOWNMENU, NULL,
						GetLeft(wnd)+offset, GetTop(wnd)+1,
						MenuHeight(mnu->Selections),
						wd,
						NULL,
						wnd,
						NULL,
						0);
#ifdef INCLUDE_SHADOWS
			AddAttribute(mwnd, SHADOW);
#endif
			if (mnu->Selections[0].SelectionTitle != NULL)	{
				SendMessage(mwnd, BUILD_SELECTIONS, (PARAM) mnu, 0);
				SendMessage(mwnd, SETFOCUS, TRUE, 0);
			}
			else
				SendMessage(wnd, PAINT, 0, 0);
			Selecting = FALSE;
			break;
		case BORDER:
			return TRUE;
		case INSIDE_WINDOW:
			return InsideRect(p1, p2, WindowRect(wnd));
		case CLOSE_POPDOWN:
			if ((int)p1)
				PostMessage(wnd, KEYBOARD, ESC, 0);
			mwnd = NULLWND;
			break;
		case CLOSE_WINDOW:
			rtn = BaseWndProc(MENUBAR, wnd, msg, p1, p2);
			if (GetText(wnd) != NULL)	{
				free(GetText(wnd));
				GetText(wnd) = NULL;
			}
			mctr = 0;
			ActiveSelection = -1;
			MenuBar = NULL;
			ActiveMenu = NULL;
			return rtn;
		default:
			break;
	}
	return BaseWndProc(MENUBAR, wnd, msg, p1, p2);
}

static void reset_menubar(WINDOW wnd)
{
	GetText(wnd) = realloc(GetText(wnd), SCREENWIDTH+5);
	memset(GetText(wnd), ' ', SCREENWIDTH);
	*(GetText(wnd)+SCREENWIDTH-
		(TestAttribute(GetParent(wnd), HASBORDER) ? 2 : 0)) = '\0';
}

static int TestGlobalKeys(WINDOW wnd, PARAM p1, PARAM p2)
{
	switch ((int)p1)	{
		case F10:
			if (MenuBar != NULLWND)
				SendMessage(MenuBar, SETFOCUS, TRUE, 0);
			return TRUE;
		case ALT_F6:
			if (GetClass(inFocus) == POPDOWNMENU)
				return TRUE;
			SetNextFocus(inFocus);
			SkipSystemWindows();
			return TRUE;
#ifdef INCLUDE_SYSTEM_MENUS
		case ALT_HYPHEN:
			if (GetClass(inFocus) == POPDOWNMENU)
				SendMessage(inFocus, CLOSE_WINDOW, 0, 0);
			if (GetClass(GetParent(inFocus)) == APPLICATION)
				BuildSystemMenu(GetParent(inFocus));
			return TRUE;
		case ' ':
			if ((int)p2 & ALTKEY)	{
				if (GetClass(inFocus) == POPDOWNMENU)
					SendMessage(inFocus, CLOSE_WINDOW, 0, 0);
				if (GetClass(inFocus) != MENUBAR &&
						TestAttribute(inFocus, HASTITLEBAR) &&
							TestAttribute(inFocus, CONTROLBOX))
					BuildSystemMenu(inFocus);
				return TRUE;
			}
			break;
#endif
		case CTRL_F4:
			if (GetClass(inFocus) != MENUBAR)
#ifdef INCLUDE_STATUSBAR
				if (GetClass(inFocus) != STATUSBAR)
#endif
					if (GetClass(inFocus) != APPLICATION)	{
						SendMessage(inFocus, CLOSE_WINDOW, 0, 0);
						SkipSystemWindows();
					}
			break;
		case ALT_F4:
			PostMessage(GetParent(wnd), CLOSE_WINDOW, 0, 0);
			break;
		default:
			break;
	}
	return FALSE;
}

static void SkipSystemWindows(void)
{
	int cl, ct = 0;
	while ((cl = GetClass(inFocus)) == MENUBAR ||
			cl == APPLICATION
#ifdef INCLUDE_STATUSBAR
		 || cl == STATUSBAR
#endif
								)	{
		SetNextFocus(inFocus);
		if (++ct == 3)
			break;
	}
}

int BarSelection(int mx)
{
	int i;
	for (i = 0; i < mctr; i++)
		if (mx >= menu[i].x1-4*i &&
				mx <= menu[i].x2-4*i-5)
			break;
	return i;
}


