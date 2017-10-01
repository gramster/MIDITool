/* -------------- button.c -------------- */

#include <conio.h>
#include "dflat.h"

#ifdef INCLUDE_DIALOG_BOXES

int ButtonProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	DBOX *db = GetParent(wnd)->extension;
	CTLWINDOW *ct = ControlBox(db, wnd);
	int x;
	switch (msg)	{
		case SETFOCUS:
			BaseWndProc(BUTTON, wnd, msg, p1, p2);
			/* ------- fall through ------- */
		case PAINT:
			if (isVisible(wnd))	{
				if (TestAttribute(wnd, SHADOW))	{
					/* -------- draw the button's shadow ------- */
					background = WndBackground(GetParent(wnd));
					foreground = BLACK;
					PutWindowChar(wnd, WindowWidth(wnd), 0, 220);
					for (x = 0; x < WindowWidth(wnd); x++)
						PutWindowChar(wnd, x+1, 1, 223);
				}
				/* --------- write the button's text ------- */
				WriteTextLine(wnd, NULL, 0, wnd == inFocus);
			}
			return TRUE;
		case KEYBOARD:
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			if (p1 != '\r')
				break;
			/* ---- fall through ---- */
		case LEFT_BUTTON:
#ifdef PUSHBUTTON_DEPRESS
			/* --------- draw a pushed button -------- */
			background = WndBackground(GetParent(wnd));
			foreground = WndBackground(wnd);
			PutWindowChar(wnd, 0, 0, ' ');
			for (x = 0; x < WindowWidth(wnd); x++)	{
				PutWindowChar(wnd, x+1, 0, 220);
				PutWindowChar(wnd, x+1, 1, 223);
			}
			if (msg == LEFT_BUTTON)
				SendMessage(NULLWND, WAITMOUSE, 0, 0);
			else
				SendMessage(NULLWND, WAITKEYBOARD, 0, 0);
			SendMessage(wnd, PAINT, 0, 0);
#endif
			if (ct->setting == ON)
				PostMessage(GetParent(wnd), COMMAND, ct->command, 0);
			else
				beep();
			return TRUE;
		case HORIZSCROLL:
			return TRUE;
		default:
			break;
	}
	return BaseWndProc(BUTTON, wnd, msg, p1, p2);
}

#endif
