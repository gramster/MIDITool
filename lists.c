/* --------------- focus.c -------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "dflat.h"

struct LinkedList Focus = {NULLWND, NULLWND};
struct LinkedList Built = {NULLWND, NULLWND};

void SetPrevFocus(WINDOW wnd)
{
	if (wnd != NULLWND && wnd == inFocus)	{
		WINDOW wnd1 = wnd;
		while (TRUE)	{
			if ((wnd1 = PrevWindow(wnd1)) == NULLWND)
				wnd1 = Focus.LastWindow;
			if (wnd1 == wnd)
				return;
			if (wnd1 != NULLWND)
				break;
		}
		if (wnd1 != NULLWND)
			SendMessage(wnd1, SETFOCUS, TRUE, 0);
	}
}

/* this function assumes that wnd is in the Focus linked list */
WINDOW SearchFocusNext(WINDOW wnd, WINDOW pwnd)
{
	WINDOW wnd1 = wnd;

	if (wnd != NULLWND)	{
		while (TRUE)	{
			if ((wnd1 = NextWindow(wnd1)) == NULLWND)
				wnd1 = Focus.FirstWindow;
			if (wnd1 == wnd)
				return NULLWND;
			if (wnd1 != NULLWND)
				if (pwnd == NULLWND || pwnd == GetParent(wnd1))
					break;
		}
	}
	return wnd1;
}

void SetNextFocus(WINDOW wnd)
{
	WINDOW wnd1;

	if (wnd != inFocus)
		return;
	if ((wnd1 = SearchFocusNext(wnd, GetParent(wnd))) == NULLWND)
		wnd1 = SearchFocusNext(wnd, NULLWND);
	if (wnd1 != NULLWND)
		SendMessage(wnd1, SETFOCUS, TRUE, 0);
}

void RemoveBuiltWindow(WINDOW wnd)
{
	if (wnd != NULLWND)	{
		if (PrevWindowBuilt(wnd) != NULLWND)
			NextWindowBuilt(PrevWindowBuilt(wnd)) = NextWindowBuilt(wnd);
		if (NextWindowBuilt(wnd) != NULLWND)
			PrevWindowBuilt(NextWindowBuilt(wnd)) = PrevWindowBuilt(wnd);
		if (wnd == Built.FirstWindow)
			Built.FirstWindow = NextWindowBuilt(wnd);
		if (wnd == Built.LastWindow)
			Built.LastWindow = PrevWindowBuilt(wnd);
	}
}

void RemoveFocusWindow(WINDOW wnd)
{
	if (wnd != NULLWND)	{
		if (PrevWindow(wnd) != NULLWND)
			NextWindow(PrevWindow(wnd)) = NextWindow(wnd);
		if (NextWindow(wnd) != NULLWND)
			PrevWindow(NextWindow(wnd)) = PrevWindow(wnd);
		if (wnd == Focus.FirstWindow)
			Focus.FirstWindow = NextWindow(wnd);
		if (wnd == Focus.LastWindow)
			Focus.LastWindow = PrevWindow(wnd);
	}
}

void AppendBuiltWindow(WINDOW wnd)
{
	if (wnd != NULLWND)	{
		if (Built.FirstWindow == NULLWND)
			Built.FirstWindow = wnd;
		if (Built.LastWindow != NULLWND)
			NextWindowBuilt(Built.LastWindow) = wnd;
		PrevWindowBuilt(wnd) = Built.LastWindow;
		NextWindowBuilt(wnd) = NULLWND;
		Built.LastWindow = wnd;
	}
}

void AppendFocusWindow(WINDOW wnd)
{
	if (wnd != NULLWND)	{
		if (Focus.FirstWindow == NULLWND)
			Focus.FirstWindow = wnd;
		if (Focus.LastWindow != NULLWND)
			NextWindow(Focus.LastWindow) = wnd;
		PrevWindow(wnd) = Focus.LastWindow;
		NextWindow(wnd) = NULLWND;
		Focus.LastWindow = wnd;
	}
}

WINDOW GetFirstChild(WINDOW wnd)
{
	WINDOW ThisWindow = Built.FirstWindow;
	while (ThisWindow != NULLWND)	{
		if (GetParent(ThisWindow) == wnd)
			break;
		ThisWindow = NextWindowBuilt(ThisWindow);
	}
	return ThisWindow;
}

WINDOW GetNextChild(WINDOW wnd, WINDOW ThisWindow)
{
	if (ThisWindow != NULLWND)	{
		do	{
			if ((ThisWindow = NextWindowBuilt(ThisWindow)) != NULLWND)
				if (GetParent(ThisWindow) == wnd)
					break;
		}	while (ThisWindow != NULLWND);
	}
	return ThisWindow;
}

WINDOW GetFirstFocusChild(WINDOW wnd)
{
	WINDOW ThisWindow = Focus.FirstWindow;
	while (ThisWindow != NULLWND)	{
		if (GetParent(ThisWindow) == wnd)
			break;
		ThisWindow = NextWindow(ThisWindow);
	}
	return ThisWindow;
}

WINDOW GetNextFocusChild(WINDOW wnd, WINDOW ThisWindow)
{
	while (ThisWindow != NULLWND)	{
		ThisWindow = NextWindow(ThisWindow);
		if (ThisWindow != NULLWND)
			if (GetParent(ThisWindow) == wnd)
				break;
	}
	return ThisWindow;
}

WINDOW GetLastChild(WINDOW wnd)
{
	WINDOW ThisWindow = Built.LastWindow;
	while (ThisWindow != NULLWND)	{
		if (GetParent(ThisWindow) == wnd)
			break;
		ThisWindow = PrevWindowBuilt(ThisWindow);
	}
	return ThisWindow;
}

WINDOW GetPrevChild(WINDOW wnd, WINDOW ThisWindow)
{
	if (ThisWindow != NULLWND)	{
		do	{
			if ((ThisWindow = PrevWindowBuilt(ThisWindow)) != NULLWND)
				if (GetParent(ThisWindow) == wnd)
					break;
		}	while (ThisWindow != NULLWND);
	}
	return ThisWindow;
}


