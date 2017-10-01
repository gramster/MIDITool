/* ------------- listbox.c ------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dflat.h"

#ifdef INCLUDE_MULTILINE
static int ExtendSelections(WINDOW, int, int);
static void TestExtended(WINDOW, PARAM);
static void ClearAllSelections(WINDOW);
static void SetSelection(WINDOW, int);
static void FlipSelection(WINDOW, int);
static void ClearSelection(WINDOW, int);
#endif
static void near writeselection(WINDOW, int, int, RECT *);
static void near change_selection(WINDOW, int, int);
static int near selection_in_window(WINDOW, int);

int ListBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn, sel;
	static int py = -1;
	int my = (int) p2 - GetTop(wnd);
	RECT rc;

	if (my >= wnd->wlines-wnd->wtop)
		my = wnd->wlines - wnd->wtop;

	switch (msg)	{
		case CREATE_WINDOW:
			rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
			wnd->selection = -1;
			wnd->AnchorPoint = -1;
			return rtn;
#ifdef INCLUDE_MULTILINE
		case BORDER:
			if (isMultiLine(wnd))	{
				char ttl[80] = "";
				WINDOW wnd1 = wnd;
				char *cp;
				if (!TestAttribute(wnd1, HASTITLEBAR))	{
					if ((wnd1 = GetParent(wnd)) == NULLWND)
						break;
					if (!TestAttribute(wnd1, HASTITLEBAR))
						break;
				}
				if (wnd1->title)
					strcpy(ttl, wnd1->title);
				if ((cp = strstr(ttl, " (Add)")) != NULL)
					*cp = '\0';
				if (wnd->AddMode)
					/* ---- in Add mode ---- */
					strcat(ttl, " (Add)");
				InsertTitle(wnd1, ttl);
				if (wnd != wnd1)
					SendMessage(wnd1, BORDER, 0, 0);
			}
			break;
#endif
		case ADDTEXT:
			if (wnd->selection == -1)
				PostMessage(wnd, LB_SETSELECTION, 0, 0);
			if (*(char *)p1 == LISTSELECTOR)
				wnd->SelectCount++;
			break;
		case CLEARTEXT:
			wnd->selection = -1;
			wnd->AnchorPoint = -1;
			wnd->SelectCount = 0;
			break;
		case SCROLL:
			rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
			if ((int)p2 == FALSE)
				writeselection(wnd, wnd->selection, TRUE, NULL);
			return rtn;
		case KEYBOARD:
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			switch ((int) p1)	{
#ifdef INCLUDE_MULTILINE
				case SHIFT_F8:
					if (isMultiLine(wnd))	{
						wnd->AddMode ^= TRUE;
						SendMessage(GetParent(wnd), ADDSTATUS,
							wnd->AddMode ? ((PARAM) "Add Mode") : 0, 0);
					}
					return TRUE;
#endif
				case UP:
#ifdef INCLUDE_MULTILINE
					TestExtended(wnd, p2);
#endif
					if (wnd->selection > 0)	{
						if (wnd->selection == wnd->wtop)	{
							writeselection(wnd, wnd->selection, FALSE, NULL);
							rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
							PostMessage(wnd, LB_SELECTION, wnd->selection-1,
								isMultiLine(wnd) ? p2 : FALSE);
						}
						else	{
							int newsel = wnd->selection-1;
							if (wnd->wlines == ClientHeight(wnd))
								while (*TextLine(wnd, newsel) == LINE)
									--newsel;
							PostMessage(wnd, LB_SELECTION, newsel,
								isMultiLine(wnd) ? p2 : FALSE);
						}
					}
					return TRUE;
				case DN:
#ifdef INCLUDE_MULTILINE
					TestExtended(wnd, p2);
#endif
					if (wnd->selection < wnd->wlines-1)	{
						if (wnd->selection == wnd->wtop+ClientHeight(wnd)-1)	{
							writeselection(wnd, wnd->selection, FALSE, NULL);
							rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
							PostMessage(wnd, LB_SELECTION, wnd->selection+1,
								isMultiLine(wnd) ? p2 : FALSE);
						}
						else	{
							int newsel = wnd->selection+1;
							if (wnd->wlines == ClientHeight(wnd))
								while (*TextLine(wnd, newsel) == LINE)
									newsel++;
							PostMessage(wnd, LB_SELECTION, newsel,
								isMultiLine(wnd) ? p2 : FALSE);
						}
					}
					return TRUE;
				case PGUP:
				case HOME:
#ifdef INCLUDE_MULTILINE
					TestExtended(wnd, p2);
#endif
					rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
					PostMessage(wnd, LB_SELECTION, wnd->wtop,
						isMultiLine(wnd) ? p2 : FALSE);
					return rtn;
				case PGDN:
				case END:
#ifdef INCLUDE_MULTILINE
					TestExtended(wnd, p2);
#endif
					rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
					PostMessage(wnd, LB_SELECTION,
						wnd->wtop+ClientHeight(wnd)-1,
						isMultiLine(wnd) ? p2 : FALSE);
					return rtn;
#ifdef INCLUDE_MULTILINE
				case ' ':
					if (!isMultiLine(wnd))
						break;
					sel = SendMessage(wnd, LB_CURRENTSELECTION, 0, 0);
					if (sel == -1)
						break;
					if (wnd->AddMode)
						FlipSelection(wnd, sel);
					else 	{
						ClearAllSelections(wnd);
						SetSelection(wnd, sel);
					}
					if (!((int) p2 & (LEFTSHIFT | RIGHTSHIFT)))
						wnd->AnchorPoint = sel;
					if (ItemSelected(wnd, sel))
						ExtendSelections(wnd, sel, (int) p2);
					SendMessage(wnd, PAINT, 0, 0);
					break;
#endif
				case '\r':
					if (wnd->selection != -1)	{
						int sel = wnd->selection;
						SendMessage(wnd, LB_SELECTION, sel, TRUE);
						SendMessage(wnd, LB_CHOOSE, sel, 0);
					}
					return TRUE;
				default:	{
					int sel = wnd->selection+1;
					while (sel < wnd->wlines)	{
						char *cp = TextLine(wnd, sel);
						if (cp == NULL)
							break;
						if (isMultiLine(wnd))
							cp++;
						if (*cp == '[')
							cp++;
						if (tolower(*cp) == (int)p1)	{
							SendMessage(wnd, LB_SELECTION, sel,
								isMultiLine(wnd) ? p2 : FALSE);
							if (!selection_in_window(wnd, sel))	{
								wnd->wtop = sel-ClientHeight(wnd)+1;
								SendMessage(wnd, PAINT, 0, 0);
							}
							break;
						}
						sel++;
					}
					break;
				}
			}
			break;
		case BUTTON_RELEASED:
			py = -1;
			return TRUE;
		case LEFT_BUTTON:
#ifdef INCLUDE_SCROLLBARS
			if (HScrolling || VScrolling)
				break;
#endif
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			rc = ClientRect(wnd);
			if (!InsideRect(p1, p2, rc))
				break;
			if (my != py)	{
				int sel = wnd->wtop+my-1;
#ifdef INCLUDE_MULTILINE
				int sh = getshift();
				if (!(sh & (LEFTSHIFT | RIGHTSHIFT)))	{
					if (!(sh & CTRLKEY))
						ClearAllSelections(wnd);
					wnd->AnchorPoint = sel;
					SendMessage(wnd, PAINT, 0, 0);
				}
#endif
				if (*TextLine(wnd, sel) != LINE)
					SendMessage(wnd, LB_SELECTION, sel, TRUE);
				py = my;
			}
			return TRUE;
		case DOUBLE_CLICK:
			BaseWndProc(LISTBOX, wnd, msg, p1, p2);
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			rc = ClientRect(wnd);
			if (InsideRect(p1, p2, rc))
				SendMessage(wnd, LB_CHOOSE,
					wnd->selection, 0);
			return TRUE;
		case LB_CHOOSE:
			SendMessage(GetParent(wnd), msg, p1, p2);
			break;
		case LB_SETSELECTION:
			change_selection(wnd, (int) p1, 0);
			return TRUE;
		case LB_SELECTION:	{
			WINDOW pwnd = GetParent(wnd);
			CLASS class = GetClass(pwnd);
			change_selection(wnd, (int) p1, (int) p2);
			if (class != LISTBOX && DerivedClass(class) != LISTBOX)
				SendMessage(GetParent(wnd), LB_SELECTION, wnd->selection, 0);
			return TRUE;
		}
		case LB_CURRENTSELECTION:
			return wnd->selection;
		case LB_GETTEXT:
			if ((int)p2 != -1)	{
				char *cp1 = (char *)p1;
				char *cp2 = TextLine(wnd, (int)p2);
				while (cp2 && *cp2 && *cp2 != '\n')
					*cp1++ = *cp2++;
				*cp1 = '\0';
			}
			return TRUE;
		case PAINT:
			if (isVisible(wnd))	{
				rtn = BaseWndProc(LISTBOX, wnd, msg, p1, p2);
#ifdef INCLUDE_MULTILINE
				if (isMultiLine(wnd))	{
					int sel = 0;
					for (sel = 0; sel < wnd->wlines; sel++)	{
						if (ItemSelected(wnd, sel))
							writeselection(wnd, sel, TRUE, (RECT *)p1);
					}
				}
#endif
				writeselection(wnd, wnd->selection, TRUE, (RECT *)p1);
				return rtn;
			}
			break;
		case HORIZSCROLL:
			return TRUE;
		case CLOSE_WINDOW:
			if (isMultiLine(wnd) && wnd->AddMode)	{
				wnd->AddMode = FALSE;
				SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
			}
			break;
		default:
			break;
	}
	return BaseWndProc(LISTBOX, wnd, msg, p1, p2);
}

#ifdef INCLUDE_MULTILINE

static void TestExtended(WINDOW wnd, PARAM p2)
{
	if (isMultiLine(wnd) && !wnd->AddMode &&
			!((int) p2 & (LEFTSHIFT | RIGHTSHIFT)))	{
		if (wnd->SelectCount > 1)	{
			ClearAllSelections(wnd);
			SendMessage(wnd, PAINT, 0, 0);
		}
	}
}

static void ClearAllSelections(WINDOW wnd)
{
	if (isMultiLine(wnd) && wnd->SelectCount > 0)	{
		int sel;
		for (sel = 0; sel < wnd->wlines; sel++)
			ClearSelection(wnd, sel);
	}
}

static void FlipSelection(WINDOW wnd, int sel)
{
	if (isMultiLine(wnd))	{
		if (ItemSelected(wnd, sel))
			ClearSelection(wnd, sel);
		else
			SetSelection(wnd, sel);
	}
}

static int ExtendSelections(WINDOW wnd, int sel, int shift)
{	
	if (shift & (LEFTSHIFT | RIGHTSHIFT) &&
						wnd->AnchorPoint != -1)	{
		int i = sel;
		int j = wnd->AnchorPoint;
		int rtn;
		if (j > i)
			swap(i,j);
		rtn = i - j;
		while (j <= i)
			SetSelection(wnd, j++);
		return rtn;
	}
	return 0;
}

static void SetSelection(WINDOW wnd, int sel)
{
	if (isMultiLine(wnd) && !ItemSelected(wnd, sel))	{
		char *lp = TextLine(wnd, sel);
		*lp = LISTSELECTOR;
		wnd->SelectCount++;
	}
}

static void ClearSelection(WINDOW wnd, int sel)
{
	if (isMultiLine(wnd) && ItemSelected(wnd, sel))	{
		char *lp = TextLine(wnd, sel);
		*lp = ' ';
		--wnd->SelectCount;
	}
}

int ItemSelected(WINDOW wnd, int sel)
{
	if (isMultiLine(wnd) && sel < wnd->wlines)	{
		char *cp = TextLine(wnd, sel);
		return (int)((*cp) & 255) == LISTSELECTOR;
	}
	return FALSE;
}

#endif

static int near selection_in_window(WINDOW wnd, int sel)
{
	return (wnd->wlines && sel >= wnd->wtop &&
			sel < wnd->wtop+ClientHeight(wnd));
}

static void near writeselection(WINDOW wnd, int sel, int reverse, RECT *rc)
{
	if (selection_in_window(wnd, sel))
		WriteTextLine(wnd, rc, sel, reverse);
}

static void near change_selection(WINDOW wnd, int sel, int shift)
{
	if (sel != wnd->selection)	{
#ifdef INCLUDE_MULTILINE
		if (isMultiLine(wnd))		{
			int sels;
			if (!wnd->AddMode)
				ClearAllSelections(wnd);
			sels = ExtendSelections(wnd, sel, shift);
			if (sels > 1)
				SendMessage(wnd, PAINT, 0, 0);
			if (sels == 0 && !wnd->AddMode)	{
				ClearSelection(wnd, wnd->selection);
				SetSelection(wnd, sel);
				wnd->AnchorPoint = sel;
			}
		}
#endif
		writeselection(wnd, wnd->selection,
#ifdef INCLUDE_MULTILINE
				isMultiLine(wnd) ?
				ItemSelected(wnd, wnd->selection) :
#endif
				FALSE,
				NULL);
		wnd->selection = sel;
		writeselection(wnd, sel,
#ifdef INCLUDE_MULTILINE
				(isMultiLine(wnd) && wnd->AddMode) ?
				!ItemSelected(wnd, sel) :
#endif
				TRUE,
				NULL);
		
	}
}
