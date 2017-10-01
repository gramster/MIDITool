/* ---------------- statbar.c -------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include "dflat.h"

#ifdef INCLUDE_STATUSBAR

int StatusBarProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	char *statusbar;
	switch (msg)	{
		case CREATE_WINDOW:
		case MOVE:
			SendMessage(wnd, CAPTURE_CLOCK, 0, 0);
			break;
		case PAINT:	
			if (!isVisible(wnd))
				break;
			if ((statusbar = calloc(1, WindowWidth(wnd)+1)) == NULL)
				break;
			memset(statusbar, ' ', WindowWidth(wnd));
			*(statusbar+WindowWidth(wnd)) = '\0';
			strncpy(statusbar+1, "F1=Help", 7);
			if (wnd->text)	{
				int len = min(strlen(wnd->text), WindowWidth(wnd)-17);
				if (len > 0)	{
					int off=(WindowWidth(wnd)-len)/2;
					strncpy(statusbar+off, wnd->text, len);
				}
			}
#ifdef INCLUDE_CLOCK
			if (wnd->TimePosted)
				*(statusbar+WindowWidth(wnd)-8) = '\0';
#endif
			SetStandardColor(wnd);
			ClearAttribute(wnd, NOCLIP);
	        clipline(wnd, 0, statusbar);
    	    wputs(wnd, statusbar, 0, 0);
			free(statusbar);
			AddAttribute(wnd, NOCLIP);
			return TRUE;
		case BORDER:
			return TRUE;
#ifdef INCLUDE_CLOCK
		case CLOCKTICK:	{
			WINDOW wnd1 = Focus.LastWindow;
			int x = GetLeft(wnd)+WindowWidth(wnd)-9;
			while (wnd1 != NULLWND && wnd1 != wnd)	{
				if (wnd1 != GetParent(wnd))	{
					if (SendMessage(wnd1, INSIDE_WINDOW, x, GetTop(wnd)))
						return TRUE;
					if (SendMessage(wnd1, INSIDE_WINDOW, x+5, GetTop(wnd)))
						return TRUE;
				}
				wnd1 = PrevWindow(wnd1);
			}
			SetStandardColor(wnd);
			wputs(wnd, (char *)p1, WindowWidth(wnd)-8, 0);
			wnd->TimePosted = TRUE;
			return TRUE;
		}
#endif
		case CLOSE_WINDOW:
#ifdef INCLUDE_CLOCK
			SendMessage(NULLWND, RELEASE_CLOCK, 0, 0);
#endif
			if (GetText(wnd) != NULL)	{
				free(GetText(wnd));
				GetText(wnd) = NULL;
			}
			break;
		default:
			break;
	}
	return BaseWndProc(STATUSBAR, wnd, msg, p1, p2);
}

#endif
