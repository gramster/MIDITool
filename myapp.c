/* ------------- applicat.c ------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <process.h>
#include "dflat.h"

static int ScreenHeight;

#ifdef INCLUDE_DIALOG_BOXES
extern DBOX TabStops;
extern DBOX DisplayCGA;
extern DBOX DisplayEGA;
extern DBOX DisplayVGA;
extern DBOX Windows;
extern DBOX Log;
static DBOX *Display;
#endif

static void ShellDOS(WINDOW);
static void CreateMenu(WINDOW);
void CreateStatusBar(WINDOW);
static void CloseAll(WINDOW);
static void SelectColors(void);
static void SetScreenHeight(int);
#ifdef INCLUDE_MULTIDOCS
static void ChooseWindow(WINDOW, int);
static void SelectTexture(void);
static void SelectBorder(WINDOW);
static void SelectTitle(WINDOW);
static void SelectStatusBar(WINDOW);
#endif
#ifdef INCLUDE_DIALOG_BOXES
static void SelectLines(WINDOW);
#endif

int ApplicationProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn, WasVisible;

	switch (msg)	{
		case CREATE_WINDOW:
			ScreenHeight = SCREENHEIGHT;
			if (!LoadConfig())
				cfg.ScreenLines = ScreenHeight;
			if (cfg.InsertMode)
				SetCommandToggle(MainMenu, ID_INSERT);
			if (cfg.WordWrap)
				SetCommandToggle(MainMenu, ID_WRAP);
#ifdef INCLUDE_DIALOG_BOXES
			if (isVGA())
				Display = &DisplayVGA;
			else if (isEGA())
				Display = &DisplayEGA;
			else 
				Display = &DisplayCGA;
			if (cfg.Border)
				SetCheckBox(Display, ID_BORDER);
			if (cfg.Title)
				SetCheckBox(Display, ID_TITLE);
			if (cfg.StatusBar)
				SetCheckBox(Display, ID_STATUSBAR);
			if (cfg.Texture)
				SetCheckBox(Display, ID_TEXTURE);
			if (cfg.mono == 1)
				PushRadioButton(Display, ID_MONO);
			else if (cfg.mono == 2)
				PushRadioButton(Display, ID_REVERSE);
			else
				PushRadioButton(Display, ID_COLOR);
			if (cfg.ScreenLines == 25)
				PushRadioButton(Display, ID_25LINES);
			else if (cfg.ScreenLines == 43)
				PushRadioButton(Display, ID_43LINES);
			else if (cfg.ScreenLines == 50)
				PushRadioButton(Display, ID_50LINES);
#endif
			if (SCREENHEIGHT != cfg.ScreenLines)	{
				SetScreenHeight(cfg.ScreenLines);
				if (WindowHeight(wnd) == ScreenHeight ||
						SCREENHEIGHT-1 < GetBottom(wnd))	{
					WindowHeight(wnd) = SCREENHEIGHT-1;
					GetBottom(wnd) = GetTop(wnd)+WindowHeight(wnd)-1;
					wnd->RestoredRC = WindowRect(wnd);
				}
			}
			SelectColors();
#ifdef INCLUDE_MULTIDOCS
			SelectBorder(wnd);
			SelectTitle(wnd);
			SelectStatusBar(wnd);
#endif
			rtn = BaseWndProc(APPLICATION, wnd, msg, p1, p2);
			if (wnd->extension != NULL)
				CreateMenu(wnd);
#ifdef INCLUDE_STATUSBAR
			CreateStatusBar(wnd);
#endif
#ifdef INCLUDE_HELP
			LoadHelpFile();
#endif
			SendMessage(NULLWND, SHOW_MOUSE, 0, 0);
			return rtn;
#ifdef INCLUDE_STATUSBAR
		case ADDSTATUS:
			if (wnd->StatusBar != NULLWND)	{
				if (p1 && *(char *)p1)
					SendMessage(wnd->StatusBar, SETTEXT, p1, p2);
				else 
					SendMessage(wnd->StatusBar, CLEARTEXT, 0, 0);
				SendMessage(wnd->StatusBar, PAINT, 0, 0);
			}
			return TRUE;
#endif
		case SETFOCUS:
			if (p1 && inFocus != wnd)	{
				/* ---- setting focus ------ */
				SendMessage(inFocus, SETFOCUS, FALSE, 0);
				/* remove window from list */
				RemoveFocusWindow(wnd);
				/* move window to end of list */
				AppendFocusWindow(wnd);
				inFocus = wnd;
				SendMessage(wnd, BORDER, 0, 0);
				return TRUE;
			}
			break;
		case SIZE:
			WasVisible = isVisible(wnd);
			if (WasVisible)
				SendMessage(wnd, HIDE_WINDOW, 0, 0);
			if (p1-GetLeft(wnd) < 30)
				p1 = GetLeft(wnd) + 30;
			BaseWndProc(APPLICATION, wnd, msg, p1, p2);
			CreateMenu(wnd);
#ifdef INCLUDE_STATUSBAR
			CreateStatusBar(wnd);
#endif
			if (WasVisible)
				SendMessage(wnd, SHOW_WINDOW, 0, 0);
			return TRUE;
		case KEYBOARD:
#ifdef INCLUDE_HELP
			if ((int)p1 == F1)
				break;
#endif
#ifdef INCLUDE_SYSTEM_MENUS
			if ((int)p1 == ALT_HYPHEN)	{
				BuildSystemMenu(wnd);
				return TRUE;
			}
			if (WindowSizing || WindowMoving)
				break;
#endif
		/* ----- fall through here ----- */
		case SHIFT_CHANGED:
			PostMessage(wnd->MenuBar, msg, p1, p2);
			return TRUE;
		case PAINT:
			if (isVisible(wnd))
				ClearWindow(wnd, (RECT *)p1, cfg.Texture ?
					APPLCHAR : ' ');
			return TRUE;
		case COMMAND:
			switch ((int)p1)	{
#ifdef INCLUDE_HELP
				case ID_HELP:
					DisplayHelp(wnd, DFLAT_APPLICATION);
					break;
				case ID_HELPHELP:
					DisplayHelp(wnd, "HelpHelp");
					break;
				case ID_EXTHELP:
					DisplayHelp(wnd, "ExtHelp");
					break;
				case ID_KEYSHELP:
					DisplayHelp(wnd, "KeysHelp");
					break;
				case ID_HELPINDEX:
					DisplayHelp(wnd, "HelpIndex");
					break;
#ifdef INCLUDE_RELOADHELP
				case ID_LOADHELP:
					LoadHelpFile();
					break;
#endif
#endif
#if 0
#ifdef INCLUDE_LOGGING
				case ID_LOG:
					MessageLog(wnd);
					if (CheckBoxSetting(&Log, ID_LOGGING))
						SetCommandToggle(MainMenu, ID_LOG);
					else
						ClearCommandToggle(MainMenu, ID_LOG);
					break;
#endif
#endif
				case ID_DOS:
					ShellDOS(wnd);
					return TRUE;
				case ID_EXIT:
				case ID_SYSCLOSE:
					PostMessage(wnd, CLOSE_WINDOW, 0, 0);
					break;
#if 0
#ifdef INCLUDE_DIALOG_BOXES
				case ID_DISPLAY:
					if (DialogBox(wnd, Display, TRUE, NULL))	{
						SendMessage(wnd, HIDE_WINDOW, 0, 0);
						SelectColors();
						SelectLines(wnd);
#ifdef INCLUDE_MULTIDOCS
						SelectBorder(wnd);
						SelectTitle(wnd);
						SelectStatusBar(wnd);
						SelectTexture();
#endif
						CreateMenu(wnd);
#ifdef INCLUDE_STATUSBAR
						CreateStatusBar(wnd);
#endif
						SendMessage(wnd, SHOW_WINDOW, 0, 0);
					}
					break;
				case ID_TABS:
					switch (cfg.Tabs)	{
						case 2:
							PushRadioButton(&TabStops, ID_TAB2);
							break;
						case 4:
							PushRadioButton(&TabStops, ID_TAB4);
							break;
						case 6:
							PushRadioButton(&TabStops, ID_TAB6);
							break;
						case 8:
							PushRadioButton(&TabStops, ID_TAB8);
							break;
						default:
							break;
					}
					if (DialogBox(wnd, &TabStops, TRUE, NULL))	{
						if (RadioButtonSetting(&TabStops, ID_TAB2))
							cfg.Tabs = 2;
						if (RadioButtonSetting(&TabStops, ID_TAB4))
							cfg.Tabs = 4;
						if (RadioButtonSetting(&TabStops, ID_TAB6))
							cfg.Tabs = 6;					
						if (RadioButtonSetting(&TabStops, ID_TAB8))
							cfg.Tabs = 8;
					}
					break;
#endif
				case ID_SAVEOPTIONS:
					SaveConfig();
					break;
#ifdef INCLUDE_MULTIDOCS
				case ID_WINDOW:
					ChooseWindow(wnd, (int)p2-2);
					break;
				case ID_CLOSEALL:
					CloseAll(wnd);
					break;
#endif
#endif /* 0 */
				case ID_SYSRESTORE:
				case ID_SYSMOVE:
				case ID_SYSSIZE:
				case ID_SYSMINIMIZE:
				case ID_SYSMAXIMIZE:
					return BaseWndProc(APPLICATION, wnd, msg, p1, p2);
				default:
					if (inFocus != wnd->MenuBar)
						PostMessage(inFocus, msg, p1, p2);
					break;
			}
			return TRUE;
#ifdef INCLUDE_SYSTEM_MENUS
		case LEFT_BUTTON:	{
			WINDOW wnd1 = wnd;
			int mx, my;
			if (WindowSizing || WindowMoving)
				return FALSE;
			if (SendMessage(wnd, INSIDE_WINDOW, p1, p2))	{
				if (inFocus && inFocus != wnd->MenuBar)
					if (SendMessage(inFocus, INSIDE_WINDOW, p1, p2))
						wnd1 = inFocus;
				mx = (int) p1 - GetLeft(wnd1);
				my = (int) p2 - GetTop(wnd1);
				if (HitControlBox(wnd1, mx, my))	{
					BuildSystemMenu(wnd1);
					return TRUE;
				}
			}
			break;
		}
#endif
		case CLOSE_WINDOW:
			if (!YesNoBox("Exit " DFLAT_APPLICATION "?"))
				return FALSE;
			CloseAll(wnd);
			free_ISDB();
			free_CCDB();
			PostMessage(NULLWND, STOP, 0, 0);
			rtn = BaseWndProc(APPLICATION, wnd, msg, p1, p2);
			if (ScreenHeight != SCREENHEIGHT)
				SetScreenHeight(ScreenHeight);
#ifdef INCLUDE_HELP
			UnLoadHelpFile();
#endif
			SendMessage(NULLWND, HIDE_MOUSE, 0, 0);
			return rtn;
		default:
			break;
	}
	return BaseWndProc(APPLICATION, wnd, msg, p1, p2);
}

static void SwitchCursor(void)
{
	SendMessage(NULLWND, SAVE_CURSOR, 0, 0);
	SwapCursorStack();
	SendMessage(NULLWND, RESTORE_CURSOR, 0, 0);
}

/* ------- Shell out to DOS ---------- */
static void ShellDOS(WINDOW wnd)
{
	SendMessage(wnd, HIDE_WINDOW, 0, 0);
	SwitchCursor();
	if (ScreenHeight != SCREENHEIGHT)
		SetScreenHeight(ScreenHeight);
	SendMessage(NULLWND, HIDE_MOUSE, 0, 0);
	printf("To return to " DFLAT_APPLICATION
		", execute the DOS exit command.");
	spawnl(P_WAIT, getenv("COMSPEC"), NULL);
	if (SCREENHEIGHT != cfg.ScreenLines)
		SetScreenHeight(cfg.ScreenLines);
	SwitchCursor();
	SendMessage(wnd, SHOW_WINDOW, 0, 0);
	SendMessage(NULLWND, SHOW_MOUSE, 0, 0);
}

static void CreateMenu(WINDOW wnd)
{
	AddAttribute(wnd, HASMENUBAR);
	if (wnd->MenuBar != NULLWND)
		SendMessage(wnd->MenuBar, CLOSE_WINDOW, 0, 0);
	wnd->MenuBar = CreateWindow(MENUBAR,
						NULL,
						GetClientLeft(wnd),
						GetClientTop(wnd)-1,
						1,
						ClientWidth(wnd),
						NULL,
						wnd,
						NULL,
						0);
	SendMessage(wnd->MenuBar, BUILDMENU, (PARAM) wnd->extension, 0);
}

#ifdef INCLUDE_STATUSBAR
void CreateStatusBar(WINDOW wnd)
{
	if (wnd->StatusBar != NULLWND)	{
		SendMessage(wnd->StatusBar, CLOSE_WINDOW, 0, 0);
		wnd->StatusBar = NULLWND;
	}
	if (TestAttribute(wnd, HASBORDER) &&
			TestAttribute(wnd, HASSTATUSBAR))	{
		wnd->StatusBar = CreateWindow(STATUSBAR,
							NULL,
							GetClientLeft(wnd),
							GetBottom(wnd),
							1,
							ClientWidth(wnd),
							NULL,
							wnd,
							NULL,
							SAVESELF);
	}
}
#endif

static WINDOW oldFocus;

static void SetOldFocus(void)
{
	if (GetClass(inFocus) == MENUBAR)
		oldFocus = PrevWindow(inFocus);
	else
		oldFocus = inFocus;
}

void PrepFileMenu(void *wnd, struct Menu *mnu)
{
	SetOldFocus();
}

void PrepSearchMenu(void *wnd, struct Menu *mnu)
{
	DeactivateCommand(MainMenu, ID_SEARCH);
	DeactivateCommand(MainMenu, ID_REPLACE);
	DeactivateCommand(MainMenu, ID_SEARCHNEXT);
	SetOldFocus();
	if (oldFocus != NULLWND && GetClass(oldFocus) == EDITBOX) {
		if (isMultiLine(oldFocus))	{
			ActivateCommand(MainMenu, ID_SEARCH);
			ActivateCommand(MainMenu, ID_REPLACE);
			ActivateCommand(MainMenu, ID_SEARCHNEXT);
		}
	}
}

void PrepEditMenu(void *wnd, struct Menu *mnu)
{
	DeactivateCommand(MainMenu, ID_CUT);
	DeactivateCommand(MainMenu, ID_COPY);
	DeactivateCommand(MainMenu, ID_CLEAR);
	DeactivateCommand(MainMenu, ID_DELETETEXT);
	DeactivateCommand(MainMenu, ID_PASTE);
	DeactivateCommand(MainMenu, ID_UNDO);
	DeactivateCommand(MainMenu, ID_SEARCH);
	DeactivateCommand(MainMenu, ID_SEARCHNEXT);
	SetOldFocus();
	if (oldFocus != NULLWND && GetClass(oldFocus) == EDITBOX) {
		if (isMultiLine(oldFocus))	{
			if (BlockMarked(oldFocus))	{
				ActivateCommand(MainMenu, ID_CUT);
				ActivateCommand(MainMenu, ID_COPY);
				ActivateCommand(MainMenu, ID_CLEAR);
				ActivateCommand(MainMenu, ID_DELETETEXT);
			}
			if (!TestAttribute(oldFocus, READONLY) &&
						Clipboard != NULL)
				ActivateCommand(MainMenu, ID_PASTE);
			if (oldFocus->DeletedText != NULL)
				ActivateCommand(MainMenu, ID_UNDO);
		}
	}
}

static char *Menus[9] = {
	"~1.                      ",
	"~2.                      ",
	"~3.                      ",
	"~4.                      ",
	"~5.                      ",
	"~6.                      ",
	"~7.                      ",
	"~8.                      ",
	"~9.                      "
};

#ifdef INCLUDE_MULTIDOCS

static int WindowSel;

void PrepWindowMenu(void *wnd, struct Menu *mnu)
{
	struct PopDown *p0 = mnu->Selections;
	struct PopDown *pd = mnu->Selections + 2;
	struct PopDown *ca = mnu->Selections + 13;
	int MenuNo = 0;
	WINDOW wnd1 = Built.FirstWindow;
	mnu->Selection = 0;
	oldFocus = PrevWindow(inFocus);
	while (wnd1 != NULLWND && MenuNo < 9)	{
		if (GetClass(wnd1) != MENUBAR &&
				GetClass(wnd1) != STATUSBAR &&
					GetParent(wnd1) == wnd)	{
			strncpy(Menus[MenuNo]+4, GetTitle(wnd1), 20);
			pd->SelectionTitle = Menus[MenuNo];
			if (wnd1 == oldFocus)	{
				pd->Attrib |= CHECKED;
				mnu->Selection = MenuNo+2;
			}
			else
				pd->Attrib &= ~CHECKED;
			pd++;
			MenuNo++;
		}
		wnd1 = NextWindowBuilt(wnd1);
	}
	if (MenuNo)
		p0->SelectionTitle = "~Close all";
	else
		p0->SelectionTitle = NULL;
	if (wnd1 != NULLWND)	{
		*pd++ = *ca;
		if (mnu->Selection == 0)
			mnu->Selection = 11;
	}
	pd->SelectionTitle = NULL;
}

#ifdef INCLUDE_DIALOG_BOXES
static int WindowPrep(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case INITIATE_DIALOG:	{
			WINDOW wnd1 = Built.FirstWindow;
			WINDOW cwnd = ControlWindow(&Windows, ID_WINDOWLIST);
			int sel = 0;
			if (cwnd == NULLWND)
				return FALSE;
			while (wnd1 != NULLWND)	{
				if (GetClass(wnd1) != MENUBAR &&
						GetClass(wnd1) != STATUSBAR &&
							GetParent(wnd1) == GetParent(wnd) &&
								wnd1 != wnd)	{
					if (wnd1 == oldFocus)
						WindowSel = sel;
					SendMessage(cwnd, ADDTEXT, (PARAM) GetTitle(wnd1), 0);
					sel++;
				}
				wnd1 = NextWindowBuilt(wnd1);
			}
			SendMessage(cwnd, LB_SETSELECTION, WindowSel, 0);
#ifdef INCLUDE_SCROLLBARS
			AddAttribute(cwnd, VSCROLLBAR);
#endif
			PostMessage(cwnd, SHOW_WINDOW, 0, 0);
			break;
		}
		case COMMAND:
			switch ((int) p1)	{
				case ID_OK:
					WindowSel = SendMessage(ControlWindow(&Windows,
										ID_WINDOWLIST),
											LB_CURRENTSELECTION, 0, 0);
					break;
				case ID_WINDOWLIST:
					if ((int) p2 == LB_CHOOSE)
						SendMessage(wnd, COMMAND, ID_OK, 0);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}
#endif

static void ChooseWindow(WINDOW wnd, int WindowNo)
{
	WINDOW wnd1 = GetFirstChild(wnd);
	if (WindowNo == 9)	{
#ifdef INCLUDE_DIALOG_BOXES
		if (DialogBox(wnd, &Windows, TRUE, WindowPrep))
			WindowNo = WindowSel;
		else
#endif
			return;
	}
	while (wnd1 != NULLWND)	{
		if (GetClass(wnd1) != MENUBAR && GetClass(wnd1) != STATUSBAR)
			if (WindowNo-- == 0)
				break;
		wnd1 = GetNextChild(wnd, wnd1);
	}
	SendMessage(wnd1, SETFOCUS, TRUE, 0);
	if (wnd1->condition == ISMINIMIZED)
		SendMessage(wnd1, RESTORE, 0, 0);
}

#endif

static void SelectColors(void)
{
#ifdef INCLUDE_DIALOG_BOXES
	if (RadioButtonSetting(Display, ID_MONO))
		cfg.mono = 1;
	else if (RadioButtonSetting(Display, ID_REVERSE))
		cfg.mono = 2;
	else
		cfg.mono = 0;
#endif
	if ((ismono() || video_mode == 2) && cfg.mono == 0)
		cfg.mono = 1;

	if (cfg.mono == 1)
		memcpy(cfg.clr, bw, sizeof bw);
	else if (cfg.mono == 2)
		memcpy(cfg.clr, reverse, sizeof reverse);
	else
		memcpy(cfg.clr, color, sizeof color);
}

#ifdef INCLUDE_DIALOG_BOXES
static void SelectLines(WINDOW wnd)
{
	cfg.ScreenLines = 25;
	if (isEGA() || isVGA())	{
		if (RadioButtonSetting(Display, ID_43LINES))
			cfg.ScreenLines = 43;
		else if (RadioButtonSetting(Display, ID_50LINES))
			cfg.ScreenLines = 50;
	}
	if (SCREENHEIGHT != cfg.ScreenLines)	{
		int FullScreen = WindowHeight(wnd) == SCREENHEIGHT;
		SetScreenHeight(cfg.ScreenLines);
		if (FullScreen || SCREENHEIGHT-1 < GetBottom(wnd))
			SendMessage(wnd, SIZE, (PARAM) GetRight(wnd),
				SCREENHEIGHT-1);
	}
}
#endif

static void SetScreenHeight(int height)
{
	if (isEGA() || isVGA())	{
		SendMessage(NULLWND, SAVE_CURSOR, 0, 0);
		switch (height)	{
			case 25:
				Set25();
				break;
			case 43:
				Set43();
				break;
			case 50:
				Set50();
				break;
			default:
				break;
		}
		SendMessage(NULLWND, RESTORE_CURSOR, 0, 0);
		resetmouse();
		SendMessage(NULLWND, SHOW_MOUSE, 0, 0);
	}
}

#ifdef INCLUDE_MULTIDOCS
static void SelectTexture(void)
{
#ifdef INCLUDE_DIALOG_BOXES
	cfg.Texture = CheckBoxSetting(Display, ID_TEXTURE);
#endif
}

static void SelectBorder(WINDOW wnd)
{
#ifdef INCLUDE_DIALOG_BOXES
	cfg.Border = CheckBoxSetting(Display, ID_BORDER);
#endif
	if (cfg.Border)
		AddAttribute(wnd, HASBORDER);
	else
		ClearAttribute(wnd, HASBORDER);
}

static void SelectStatusBar(WINDOW wnd)
{
#ifdef INCLUDE_DIALOG_BOXES
	cfg.StatusBar = CheckBoxSetting(Display, ID_STATUSBAR);
#endif
	if (cfg.StatusBar)
		AddAttribute(wnd, HASSTATUSBAR);
	else
		ClearAttribute(wnd, HASSTATUSBAR);
}

static void SelectTitle(WINDOW wnd)
{
#ifdef INCLUDE_DIALOG_BOXES
	cfg.Title = CheckBoxSetting(Display, ID_TITLE);
#endif
	if (cfg.Title)
		AddAttribute(wnd, HASTITLEBAR);
	else
		ClearAttribute(wnd, HASTITLEBAR);
}
#endif

static void CloseAll(WINDOW wnd)
{
	WINDOW wnd1 = GetLastChild(wnd);
	ClearAttribute(wnd, VISIBLE);
	while (wnd1 != NULLWND)	{
		if (GetClass(wnd1) == MENUBAR
#ifdef INCLUDE_STATUSBAR
				|| GetClass(wnd1) == STATUSBAR
#endif
						)
			wnd1 = GetPrevChild(wnd, wnd1);
		else	{
			SendMessage(wnd1, CLOSE_WINDOW, 0, 0);
			wnd1 = GetLastChild(wnd);
		}
	}
	AddAttribute(wnd, VISIBLE);
	SendMessage(wnd, SETFOCUS, TRUE, 0);
	SendMessage(wnd, PAINT, 0, 0);
}


