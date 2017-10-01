/* ------------------ msgbox.c ------------------ */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include "dflat.h"

extern DBOX MsgBox;

static int ReturnValue;

int MessageBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case CREATE_WINDOW:
			GetClass(wnd) = MESSAGEBOX;
			ClearAttribute(wnd, CONTROLBOX);
			break;
#ifdef INCLUDE_DIALOG_BOXES
		case KEYBOARD:
			if (p1 == '\r' || p1 == ESC)
				ReturnValue = (int)p1;
			break;
#endif
		default:
			break;
	}
	return BaseWndProc(MESSAGEBOX, wnd, msg, p1, p2);
}

int YesNoBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case CREATE_WINDOW:
			GetClass(wnd) = MESSAGEBOX;
			ClearAttribute(wnd, CONTROLBOX);
			break;
		case KEYBOARD:	{
			int c = tolower((int)p1);
			if (c == 'y')
				SendMessage(wnd, COMMAND, ID_OK, 0);
			else if (c == 'n')
				SendMessage(wnd, COMMAND, ID_CANCEL, 0);
			break;
		}
#ifndef INCLUDE_DIALOG_BOXES
		case COMMAND:
			if (p1 == ID_OK || p1 == ID_CANCEL)	{
				ReturnValue = (int)p1;
				return TRUE;
			}
			break;
#endif
		default:
			break;
	}
	return BaseWndProc(MESSAGEBOX, wnd, msg, p1, p2);
}

int ErrorBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case CREATE_WINDOW:
			GetClass(wnd) = ERRORBOX;
			ClearAttribute(wnd, MOVEABLE | CONTROLBOX);
			break;
#ifdef INCLUDE_DIALOG_BOXES
		case KEYBOARD:
			if (p1 == '\r' || p1 == ESC)
				ReturnValue = (int)p1;
			break;
#endif
		default:
			break;
	}
	return BaseWndProc(ERRORBOX, wnd, msg, p1, p2);
}

int GenericMessage(char *ttl, char *msg, int buttonct,
	int (*wndproc)(struct window *, enum messages, PARAM, PARAM),
	char *b1, char *b2)
{
#ifdef INCLUDE_DIALOG_BOXES
	int rtn;
	MsgBox.dwnd.title = ttl;
	MsgBox.ctl[0].dwnd.h = MsgHeight(msg);
	MsgBox.ctl[0].dwnd.w = max(MsgWidth(msg),
			buttonct*8 + buttonct + 2);
	MsgBox.dwnd.h = MsgBox.ctl[0].dwnd.h+6;
	MsgBox.dwnd.w = MsgBox.ctl[0].dwnd.w+4;
	if (buttonct == 1)
		MsgBox.ctl[1].dwnd.x = (MsgBox.dwnd.w - 10) / 2;
	else	{
		MsgBox.ctl[1].dwnd.x = (MsgBox.dwnd.w - 20) / 2;
		MsgBox.ctl[2].dwnd.x = MsgBox.ctl[1].dwnd.x + 10;
		MsgBox.ctl[2].class = BUTTON;
	}
	MsgBox.ctl[1].dwnd.y = MsgBox.dwnd.h - 4;
	MsgBox.ctl[2].dwnd.y = MsgBox.dwnd.h - 4;
	MsgBox.ctl[0].itext = msg;
	MsgBox.ctl[1].itext = b1;
	MsgBox.ctl[2].itext = b2;
	MsgBox.ctl[1].isetting = ON;
	MsgBox.ctl[2].isetting = ON;
	rtn = DialogBox(NULLWND, &MsgBox, TRUE, wndproc);
	MsgBox.ctl[2].class = 0;
	return rtn;
#else
	WINDOW wnd;

	wnd = CreateWindow(TEXTBOX,ttl,
			-1,-1,MsgHeight(msg)+3,MsgWidth(msg)+2,
			NULL,NULL,
			wndproc,
			HASBORDER | SAVESELF);
	SendMessage(wnd, SETTEXT, (PARAM) msg, 0);
	SendMessage(wnd, SETFOCUS, TRUE, 0);
	SendMessage(wnd, CAPTURE_KEYBOARD, 0, 0);
	SendMessage(wnd, CAPTURE_MOUSE, 0, 0);
	ReturnValue = 0;
	while (ReturnValue == 0)
		dispatch_message();
	SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
	SendMessage(wnd, RELEASE_MOUSE, 0, 0);
	SendMessage(wnd, CLOSE_WINDOW, 0, 0);
	return ReturnValue == ID_OK || ReturnValue == '\r';
#endif
}

WINDOW MomentaryMessage(char *msg)
{
	WINDOW wnd = CreateWindow(
					TEXTBOX,
					NULL,
					-1,-1,MsgHeight(msg)+2,MsgWidth(msg)+2,
					NULL,NULL,NULL,
					HASBORDER | VISIBLE | SAVESELF);
	SendMessage(wnd, SETTEXT, (PARAM) msg, 0);
	SendMessage(wnd, PAINT, 0, 0);
	return wnd;
}

int MsgHeight(char *msg)
{
	int h = 1;
	while ((msg = strchr(msg, '\n')) != NULL)	{
		h++;
		msg++;
	}
	return min(h, SCREENHEIGHT-10);
}

int MsgWidth(char *msg)
{
	int w = 0;
	char *cp = msg;
	while ((cp = strchr(msg, '\n')) != NULL)	{
		w = max(w, (int) (cp-msg));
		msg = cp+1;
	}
	return min(max(strlen(msg),w), SCREENWIDTH-10);
}


