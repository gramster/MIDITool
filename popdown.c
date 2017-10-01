/* ------------- popdown.c ----------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "dflat.h"

static int SelectionWidth(struct PopDown *);

int PopDownProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int wd, rtn, i;
	struct PopDown *pd1;
	unsigned char sep[80], *cp = sep;
	unsigned char sel[80];
	struct PopDown *ActivePopDown = NULL;
	WINDOW currFocus;

	switch (msg)	{
		case CREATE_WINDOW:
			ClearAttribute(wnd, HASTITLEBAR 	|
								VSCROLLBAR 	|
								MOVEABLE 	|
								SIZEABLE 	|
								HSCROLLBAR);
			rtn = BaseWndProc(POPDOWNMENU, wnd, msg, p1, FALSE);
			SendMessage(wnd, CAPTURE_MOUSE, 0, 0);
			SendMessage(wnd, CAPTURE_KEYBOARD, 0, 0);
			SendMessage(NULLWND, SAVE_CURSOR, 0, 0);
			SendMessage(NULLWND, HIDE_CURSOR, 0, 0);
			return rtn;
		case BUILD_SELECTIONS:
			wnd->mnu = (void *) p1;
			ActivePopDown = pd1 = wnd->mnu->Selections;
			wd = MenuWidth(ActivePopDown)-2;
			while (wd--)
				*cp++ = LINE;
			*cp = '\0';
			wnd->selection = wnd->mnu->Selection;
			while (pd1->SelectionTitle != NULL)	{
				if (*pd1->SelectionTitle == LINE)
					SendMessage(wnd, ADDTEXT, (PARAM) sep, 0);
				else	{
					memset(sel, '\0', sizeof sel);
					if (pd1->Attrib & INACTIVE)	{
						strcpy(sel, "   ");
						*sel = CHANGECOLOR;
						*(sel+1) = cfg.clr[POPDOWNMENU] [HILITE_COLOR] [FG] | 0x80;
						*(sel+2) = cfg.clr[POPDOWNMENU] [STD_COLOR] [BG] | 0x80;
					}
					strcat(sel, " ");
					CopyCommand(sel+strlen(sel), pd1->SelectionTitle,
							pd1->Attrib & INACTIVE, cfg.clr[POPDOWNMENU] [STD_COLOR] [BG]);
					if (pd1->Attrib & CHECKED)
						*sel = CHECKMARK;
					if (pd1->Accelerator)	{
						int i;
						int wd1 = 2+SelectionWidth(ActivePopDown) - strlen(pd1->SelectionTitle);
						for (i = 0; keys[i].keylabel; i++)
							if (keys[i].keycode == pd1->Accelerator)	{
								while (wd1--)
									strcat(sel, " ");
								strcat(sel, "[");
								strcat(sel, keys[i].keylabel);
								strcat(sel, "]");
								break;
							}
					}
					strcat(sel, "  ");
					sel[strlen(sel)-1] = RESETCOLOR;
					SendMessage(wnd, ADDTEXT, (PARAM) sel, 0);
				}
				pd1++;
			}
			break;
		case LEFT_BUTTON:
			if (SendMessage(wnd, INSIDE_WINDOW, p1, p2))
				break;
			if ((int)p2 == GetTop(GetParent(wnd)))
				if (GetClass(GetParent(wnd)) == MENUBAR)
					PostMessage(GetParent(wnd), msg, p1, p2);
			return FALSE;
		case DOUBLE_CLICK:
			return TRUE;
		case BUTTON_RELEASED:
			if (InsideRect((int)p1, (int)p2, ClientRect(wnd)))
				SendMessage(wnd, LB_CHOOSE, wnd->selection, 0);
			else	{
				WINDOW pwnd = GetParent(wnd);
				if (GetClass(pwnd) == MENUBAR && (int)p2 == GetTop(pwnd))
					break;
				if ((int)p1 == GetLeft(pwnd)+2)
					break;
				SendMessage(wnd, CLOSE_WINDOW, TRUE, 0);
				return TRUE;
			}
			break;
		case PAINT:
			if (wnd->mnu == NULL)
				return TRUE;
			break;
		case BORDER:
			if (wnd->mnu == NULL)
				return TRUE;
			currFocus = inFocus;
			inFocus = NULLWND;
			rtn = BaseWndProc(POPDOWNMENU, wnd, msg, p1, FALSE);
			inFocus = currFocus;

			for (i = 0; i < ClientHeight(wnd); i++)	{
				if (*TextLine(wnd, i) == LINE)	{
					wputch(wnd, LEDGE, 0, i+1);
					wputch(wnd, REDGE, WindowWidth(wnd)-1, i+1);
				}
			}
			return rtn;

		case LB_CHOOSE:
			ActivePopDown = wnd->mnu->Selections;
			if (ActivePopDown != NULL)	{
				int *attr = &(ActivePopDown+(int)p1)->Attrib;
				wnd->mnu->Selection = (int)p1;
				if (*attr & INACTIVE)
					beep();
				else if (*attr & TOGGLE)	{
					unsigned char *tl = TextLine(wnd, (int)p1);
					*attr ^= CHECKED;
					if (*attr & CHECKED)
						*tl = CHECKMARK;
					else
						*tl = ' ';
					SendMessage(wnd, PAINT, 0, 0);
				}
				else	{
					PostMessage(GetParent(wnd), COMMAND,
						(ActivePopDown+(int)p1)->ActionId, p1);
					SendMessage(wnd, CLOSE_WINDOW, 0, 0);
				}
			}
			return TRUE;
		case KEYBOARD:
			if (wnd->mnu != NULL)	{
				ActivePopDown = wnd->mnu->Selections;
				if (ActivePopDown != NULL)	{
					int c = tolower((int)p1);
					int sel = 0;
					struct PopDown *pd = ActivePopDown;
					int a = AltConvert(c);

					while (pd->SelectionTitle != NULL)	{
						char *cp = strchr(pd->SelectionTitle,
										SHORTCUTCHAR);
						int sc = tolower(*(cp+1));
						if ((cp && sc == c) ||
								(a && sc == a) ||
									pd->Accelerator == c)	{
							PostMessage(wnd, LB_CHOOSE, sel, TRUE);
							return TRUE;
						}
						pd++, sel++;
					}
				}
			}
			switch ((int)p1)	{
				case F1:
#ifdef INCLUDE_HELP
					if (ActivePopDown == NULL)
						SendMessage(GetParent(wnd), msg, p1, p2);
					else 
						DisplayHelp(wnd, (ActivePopDown+wnd->selection)->help);
					return TRUE;
#endif
				case ESC:
					SendMessage(wnd, CLOSE_WINDOW, TRUE, 0);
					return TRUE;
				case FWD:
				case BS:
					if (GetClass(GetParent(wnd)) == MENUBAR)	{
						PostMessage(GetParent(wnd), KEYBOARD, p1, p2);
						return TRUE;
					}
					break;
				case UP:
					if (wnd->selection == 0)	{
						if (wnd->wlines == ClientHeight(wnd))	{
							PostMessage(wnd, LB_SELECTION, wnd->wlines-1, FALSE);
							return TRUE;
						}
					}
					return BaseWndProc(POPDOWNMENU, wnd, msg, p1, p2);
				case DN:
					if (wnd->selection == wnd->wlines-1)	{
						if (wnd->wlines == ClientHeight(wnd))	{
							PostMessage(wnd, LB_SELECTION, 0, FALSE);
							return TRUE;
						}
					}
					return BaseWndProc(POPDOWNMENU, wnd, msg, p1, p2);
				case HOME:
				case END:
				case '\r':
					return BaseWndProc(POPDOWNMENU, wnd, msg, p1, p2);
			}
			return FALSE;
		case CLOSE_WINDOW:
			SendMessage(wnd, RELEASE_MOUSE, 0, 0);
			SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
			SendMessage(GetParent(wnd), CLOSE_POPDOWN, p1, 0);
			SendMessage(NULLWND, RESTORE_CURSOR, 0, 0);
			break;
		default:
			break;
	}
	return BaseWndProc(POPDOWNMENU, wnd, msg, p1, p2);
}

int MenuHeight(struct PopDown *pd)
{
	int ht = 0;
	while (pd[ht].SelectionTitle != NULL)
		ht++;
	return ht+2;
}

int MenuWidth(struct PopDown *pd)
{
	int wd = 0, i;
	int len = 0;

	wd = SelectionWidth(pd);
	while (pd->SelectionTitle != NULL)	{
		if (pd->Accelerator)	{
			for (i = 0; keys[i].keylabel; i++)
				if (keys[i].keycode == pd->Accelerator)	{
					len = max(len, 2+strlen(keys[i].keylabel));
					break;
				}
		}
		pd++;
	}
	return wd+5+len;
}

static int SelectionWidth(struct PopDown *pd)
{
	int wd = 0;
	while (pd->SelectionTitle != NULL)	{
		int len = strlen(pd->SelectionTitle)-1;
		wd = max(wd, len);
		pd++;
	}
	return wd;
}

int CopyCommand(unsigned char *dest, unsigned char *src,
										int skipcolor, int bg)
{
	unsigned char *d = dest;
	while (*src && *src != '\n')	{
		if (*src == SHORTCUTCHAR)	{
			src++;
			if (!skipcolor)	{
				*dest++ = CHANGECOLOR;
				*dest++ = cfg.clr[POPDOWNMENU] [HILITE_COLOR] [BG] | 0x80;
				*dest++ = bg | 0x80;
				*dest++ = *src++;
				*dest++ = RESETCOLOR;
			}
		}
		else
			*dest++ = *src++;
	}
	return (int) (dest - d);
}

