/* ---------- window.c ------------- */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "dflat.h"

WINDOW inFocus = NULLWND;

int foreground, background;   /* current video colors */

static void TopLine(WINDOW, int, RECT);

/* --------- create a window ------------ */
WINDOW CreateWindow(
    CLASS class,              /* class of this window       */
    char *ttl,                /* title or NULL              */
    int left, int top,        /* upper left coordinates     */
    int height, int width,    /* dimensions                 */
    void *extension,          /* pointer to additional data */
    WINDOW parent,            /* parent of this window      */
    int (*wndproc)(struct window *,enum messages,PARAM,PARAM),
    int attrib)               /* window attribute           */
{
    WINDOW wnd = calloc(1, sizeof(struct window));
    get_videomode();
    if (wnd != NULLWND)    {
        int base;
        /* ----- height, width = -1: fill the screen ------- */
        if (height == -1)
            height = SCREENHEIGHT;
        if (width == -1)
            width = SCREENWIDTH;
        /* ----- coordinates -1, -1 = center the window ---- */
        if (left == -1)
            wnd->rc.lf = (SCREENWIDTH-width)/2;
        else
            wnd->rc.lf = left;
        if (top == -1)
            wnd->rc.tp = (SCREENHEIGHT-height)/2;
        else
            wnd->rc.tp = top;
        wnd->attrib = attrib;
        if (ttl != NULL)
            AddAttribute(wnd, HASTITLEBAR);
        if (wndproc == NULL)
            wnd->wndproc = classdefs[class].wndproc;
        else
            wnd->wndproc = wndproc;
        /* ---- derive attributes of base classes ---- */
        base = class;
        while (base != -1)    {
            AddAttribute(wnd, classdefs[base].attrib);
            base = classdefs[base].base;
        }
        if (parent && !TestAttribute(wnd, NOCLIP))    {
            /* -- keep upper left within borders of parent - */
            wnd->rc.lf = max(wnd->rc.lf,GetClientLeft(parent));
            wnd->rc.tp = max(wnd->rc.tp,GetClientTop(parent));
        }
        wnd->class = class;
        wnd->extension = extension;
        wnd->rc.rt = GetLeft(wnd)+width-1;
        wnd->rc.bt = GetTop(wnd)+height-1;
        wnd->ht = height;
        wnd->wd = width;
        if (ttl != NULL)
            InsertTitle(wnd, ttl);
        wnd->nextfocus = wnd->prevfocus = wnd->dFocus = NULLWND;
        wnd->parent = parent;
        wnd->condition = ISRESTORED;
        wnd->RestoredRC = wnd->rc;
        wnd->PrevKeyboard = wnd->PrevMouse = NULL;
        SendMessage(wnd, CREATE_WINDOW, 0, 0);
        if (isVisible(wnd))
            SendMessage(wnd, SHOW_WINDOW, 0, 0);
    }
    return wnd;
}

/* -------- add a title to a window --------- */
void AddTitle(WINDOW wnd, char *ttl)
{
    InsertTitle(wnd, ttl);
    SendMessage(wnd, BORDER, 0, 0);
}

/* ----- insert a title into a window ---------- */
void InsertTitle(WINDOW wnd, char *ttl)
{
    if ((wnd->title=realloc(wnd->title,strlen(ttl)+1)) != NULL)
        strcpy(wnd->title, ttl);
}

int CharInView(WINDOW wnd, int x, int y)
{
    int x1 = GetLeft(wnd)+x;
    int y1 = GetTop(wnd)+y;

    if (isVisible(wnd))    {
        if (!TestAttribute(wnd, NOCLIP))    {
            WINDOW wnd1 = GetParent(wnd);
            while (wnd1 != NULLWND)    {
                /* --- clip character to parent's borders -- */
                if (x1 < GetClientLeft(wnd1)   ||
                    x1 > GetClientRight(wnd1)  ||
                    y1 > GetClientBottom(wnd1) ||
                    y1 < GetClientTop(wnd1))
                        return FALSE;
                wnd1 = GetParent(wnd1);
            }
        }
	}
    return (x1 < SCREENWIDTH && y1 < SCREENHEIGHT);
}

/* ------- write a character to a window area at x,y ------- */
void PutWindowChar(WINDOW wnd, int x, int y, int c)
{
	if (CharInView(wnd, x, y))
        wputch(wnd, c, x, y);
}

static unsigned char line[161];

#ifdef INCLUDE_SYSTEM_MENUS

/* ----- clip line if it extends below the bottom of the
             parent window ------ */
static int clipbottom(WINDOW wnd, int y)
{
    if (!TestAttribute(wnd, NOCLIP))    {
        WINDOW wnd1 = GetParent(wnd);
        while (wnd1 != NULLWND)    {
            if (GetClientTop(wnd)+y > GetClientBottom(wnd1)+1)
                return TRUE;
            wnd1 = GetParent(wnd1);
        }
    }
    return GetTop(wnd)+y > SCREENHEIGHT;
}

/* ------ clip the portion of a line that extends past the
                     right margin of the parent window ----- */
void clipline(WINDOW wnd, int x, char *ln)
{
    WINDOW pwnd = GetParent(wnd);
    int x1 = strlen(ln);
    int i = 0;

    if (!TestAttribute(wnd, NOCLIP))    {
        while (pwnd != NULLWND)    {
            x1 = GetClientRight(pwnd) - GetLeft(wnd) - x + 1;
            pwnd = GetParent(pwnd);
        }
    }
    else if (GetLeft(wnd) + x > SCREENWIDTH)
        x1 = SCREENWIDTH-GetLeft(wnd) - x;
    /* --- adjust the clipping offset for color controls --- */
    if (x1 < 0)
        x1 = 0;
    while (i < x1)    {
        if ((unsigned char) ln[i] == CHANGECOLOR)
            i += 3, x1 += 3;
        else if ((unsigned char) ln[i] == RESETCOLOR)
            i++, x1++;
        else 
            i++;
    }
    ln[x1] = '\0';
}
#else
#define clipbottom(w,y) FALSE
#endif

/* ------ write a line to video window client area ------ */
void writeline(WINDOW wnd, char *str, int x, int y, int pad)
{
    static char wline[120];

    if (!clipbottom(wnd, y))
    {
        char *cp;
        int len;
        int dif;

        memset(wline, 0, sizeof wline);
        len = LineLength(str);
        dif = strlen(str) - len;
        strncpy(wline, str, ClientWidth(wnd) + dif);
        if (pad)    {
            cp = wline+strlen(wline);
            while (len++ < ClientWidth(wnd)-x)
                *cp++ = ' ';
        }
        clipline(wnd, x, wline);
        wputs(wnd, wline, x, y);
    }
}

RECT AdjustRectangle(WINDOW wnd, RECT rc)
{
    /* -------- adjust the rectangle ------- */
    if (TestAttribute(wnd, HASBORDER))    {
        if (RectLeft(rc) == 0)
            --rc.rt;
        else if (RectLeft(rc) < RectRight(rc) &&
                RectLeft(rc) < WindowWidth(wnd)+1)
            --rc.lf;
    }
    if (TestAttribute(wnd, HASBORDER | HASTITLEBAR))    {
        if (RectTop(rc) == 0)
            --rc.bt;
        else if (RectTop(rc) < RectBottom(rc) &&
                RectTop(rc) < WindowHeight(wnd)+1)
            --rc.tp;
    }
    RectRight(rc) = max(RectLeft(rc),
                        min(RectRight(rc),WindowWidth(wnd)));
    RectBottom(rc) = max(RectTop(rc),
                        min(RectBottom(rc),WindowHeight(wnd)));
    return rc;
}

/* -------- display a window's title --------- */
void DisplayTitle(WINDOW wnd, RECT *rcc)
{
	if (GetTitle(wnd) != NULL)	{
    	int tlen = min(strlen(GetTitle(wnd)), WindowWidth(wnd)-2);
    	int tend = WindowWidth(wnd)-3-BorderAdj(wnd);
    	RECT rc;

    	if (rcc == NULL)
        	rc = RelativeWindowRect(wnd, WindowRect(wnd));
    	else
        	rc = *rcc;
    	rc = AdjustRectangle(wnd, rc);

    	if (SendMessage(wnd, TITLE, (PARAM) rcc, 0))    {
        	if (wnd == inFocus)    {
            	foreground = cfg.clr[TITLEBAR] [HILITE_COLOR] [FG];
            	background = cfg.clr[TITLEBAR] [HILITE_COLOR] [BG];
        	}
        	else    {
            	foreground = cfg.clr[TITLEBAR] [STD_COLOR] [FG];
            	background = cfg.clr[TITLEBAR] [STD_COLOR] [BG];
        	}
        	memset(line,' ',WindowWidth(wnd));
        	if (wnd->condition != ISMINIMIZED)
            	strncpy(line + ((WindowWidth(wnd)-2 - tlen) / 2),
                	wnd->title, tlen);
        	if (TestAttribute(wnd, CONTROLBOX))
            	line[2-BorderAdj(wnd)] = CONTROLBOXCHAR;
#ifdef INCLUDE_SYSTEM_MENUS
        	if (TestAttribute(wnd, MINMAXBOX))    {
            	switch (wnd->condition)    {
                	case ISRESTORED:
                    	line[tend+1] = MAXPOINTER;
                    	line[tend]   = MINPOINTER;
                    	break;
                	case ISMINIMIZED:
                    	line[tend+1] = MAXPOINTER;
                    	break;
                	case ISMAXIMIZED:
                    	line[tend]   = MINPOINTER;
                    	line[tend+1] = RESTOREPOINTER;
                    	break;
                	default:
                    	break;
            	}
        	}
#endif
        	line[RectRight(rc)+1] = line[tend+3] = '\0';
        	writeline(wnd, line+RectLeft(rc),
                       	RectLeft(rc)+BorderAdj(wnd),
                       	0,
                       	FALSE);
    	}
	}
}

#ifdef INCLUDE_SHADOWS
/* --- display right border shadow character of a window --- */
static void near shadow_char(WINDOW wnd, int y)
{
    int fg = foreground;
    int bg = background;
    int x = WindowWidth(wnd);
    int c = videochar(GetLeft(wnd)+x, GetTop(wnd)+y);

    if (TestAttribute(wnd, SHADOW) == 0)
        return;
    foreground = DARKGRAY;
    background = BLACK;
    PutWindowChar(wnd, x, y, c);
    foreground = fg;
    background = bg;
}

/* --- display the bottom border shadow line for a window -- */
static void near shadowline(WINDOW wnd, RECT rc)
{
    int i;
    int y = GetBottom(wnd)+1;

    if ((TestAttribute(wnd, SHADOW)) == 0)
        return;
    if (!clipbottom(wnd, WindowHeight(wnd)))    {
        int fg = foreground;
        int bg = background;
        for (i = 0; i < WindowWidth(wnd)+1; i++)
            line[i] = videochar(GetLeft(wnd)+i, y);
        line[i] = '\0';
        foreground = DARKGRAY;
        background = BLACK;
        clipline(wnd, 0, line);
        line[RectRight(rc)+1] = '\0';
        if (RectLeft(rc) == 0)
            rc.lf++;
        wputs(wnd, line+RectLeft(rc), RectLeft(rc),
            WindowHeight(wnd));
        foreground = fg;
        background = bg;
    }
}
#endif

/* ------- display a window's border ----- */
void RepaintBorder(WINDOW wnd, RECT *rcc)
{
    int y;
    unsigned int lin, side, ne, nw, se, sw;
    RECT rc, clrc;

    if (!TestAttribute(wnd, HASBORDER))
        return;
    if (rcc == NULL)    {
        rc = RelativeWindowRect(wnd, WindowRect(wnd));
#ifdef INCLUDE_SHADOWS
	    if (TestAttribute(wnd, SHADOW))    {
    	    rc.rt++;
        	rc.bt++;
	    }
#endif
    }
    else
        rc = *rcc;
    clrc = AdjustRectangle(wnd, rc);

    if (wnd == inFocus)    {
        lin  = FOCUS_LINE;
        side = FOCUS_SIDE;
        ne   = FOCUS_NE;
        nw   = FOCUS_NW;
        se   = FOCUS_SE;
        sw   = FOCUS_SW;
    }
    else    {
        lin  = LINE;
        side = SIDE;
        ne   = NE;
        nw   = NW;
        se   = SE;
        sw   = SW;
    }
    line[WindowWidth(wnd)] = '\0';
    /* ---------- window title ------------ */
    if (TestAttribute(wnd, HASTITLEBAR))
        if (RectTop(rc) == 0)
            if (RectLeft(rc) < WindowWidth(wnd)-BorderAdj(wnd))
                DisplayTitle(wnd, &rc);
    foreground = FrameForeground(wnd);
    background = FrameBackground(wnd);
    /* -------- top frame corners --------- */
    if (RectTop(rc) == 0)    {
        if (RectLeft(rc) == 0)
            PutWindowChar(wnd, 0, 0, nw);
        if (RectLeft(rc) < WindowWidth(wnd))    {
            if (RectRight(rc) >= WindowWidth(wnd)-1)
                PutWindowChar(wnd, WindowWidth(wnd)-1, 0, ne);
            TopLine(wnd, lin, clrc);
        }
    }

    /* ----------- window body ------------ */
    for (y = RectTop(rc); y <= RectBottom(rc); y++)    {
        int ch;
        if (y == 0 || y >= WindowHeight(wnd)-1)
            continue;
        if (RectLeft(rc) == 0)
            PutWindowChar(wnd, 0, y, side);
        if (RectLeft(rc) < WindowWidth(wnd) &&
                RectRight(rc) >= WindowWidth(wnd)-1)    {
#ifdef INCLUDE_SCROLLBARS
            if (TestAttribute(wnd, VSCROLLBAR))
                ch = (    y == 1 ? UPSCROLLBOX      :
                          y == WindowHeight(wnd)-2  ?
                                DOWNSCROLLBOX       :
                          y-1 == wnd->VScrollBox    ?
                                SCROLLBOXCHAR       :
                          SCROLLBARCHAR );
            else
#endif
                ch = side;
            PutWindowChar(wnd, WindowWidth(wnd)-1, y, ch);
        }
#ifdef INCLUDE_SHADOWS
        if (RectRight(rc) == WindowWidth(wnd))
            shadow_char(wnd, y);
#endif
    }

    if (RectTop(rc) <= WindowHeight(wnd)-1 &&
            RectBottom(rc) >= WindowHeight(wnd)-1)    {
        /* -------- bottom frame corners ---------- */
        if (RectLeft(rc) == 0)
            PutWindowChar(wnd, 0, WindowHeight(wnd)-1, sw);
        if (RectLeft(rc) < WindowWidth(wnd) &&
                RectRight(rc) >= WindowWidth(wnd)-1)
            PutWindowChar(wnd, WindowWidth(wnd)-1,
                WindowHeight(wnd)-1, se);


		if (wnd->StatusBar == NULLWND)	{
        	/* ----------- bottom line ------------- */
        	memset(line,lin,WindowWidth(wnd)-1);
#ifdef INCLUDE_SCROLLBARS
        	if (TestAttribute(wnd, HSCROLLBAR))    {
            	line[0] = LEFTSCROLLBOX;
            	line[WindowWidth(wnd)-3] = RIGHTSCROLLBOX;
            	memset(line+1, SCROLLBARCHAR, WindowWidth(wnd)-4);
            	line[wnd->HScrollBox] = SCROLLBOXCHAR;
        	}
#endif
        	line[WindowWidth(wnd)-2] = line[RectRight(rc)] = '\0';
        	if (RectLeft(rc) != RectRight(rc) ||
        	(RectLeft(rc) && RectLeft(rc) < WindowWidth(wnd)-1))
            	writeline(wnd,
                	line+(RectLeft(clrc)),
                	RectLeft(clrc)+1,
                	WindowHeight(wnd)-1,
                	FALSE);
		}
#ifdef INCLUDE_SHADOWS
        if (RectRight(rc) == WindowWidth(wnd))
            shadow_char(wnd, WindowHeight(wnd)-1);
#endif
    }
#ifdef INCLUDE_SHADOWS
    if (RectBottom(rc) == WindowHeight(wnd))
        /* ---------- bottom shadow ------------- */
        shadowline(wnd, rc);
#endif
}

static void TopLine(WINDOW wnd, int lin, RECT rc)
{
    if (TestAttribute(wnd, HASMENUBAR))
        return;
    if (TestAttribute(wnd, HASTITLEBAR) && GetTitle(wnd))
        return;
	if (RectLeft(rc) == 0)	{
		RectLeft(rc) += BorderAdj(wnd);
		RectRight(rc) += BorderAdj(wnd);
	}
	if (RectRight(rc) < WindowWidth(wnd)-1)
		RectRight(rc)++;

    if (RectLeft(rc) < RectRight(rc))    {
        /* ----------- top line ------------- */
        memset(line,lin,WindowWidth(wnd)-1);
        line[RectRight(rc)] = '\0';
        writeline(wnd, line+RectLeft(rc),
            RectLeft(rc), 0, FALSE);
    }
}

/* ------ clear the data space of a window -------- */
void ClearWindow(WINDOW wnd, RECT *rcc, int clrchar)
{
    if (isVisible(wnd))    {
        int y;
        RECT rc;

        if (rcc == NULL)
            rc = RelativeWindowRect(wnd, WindowRect(wnd));
        else
            rc = *rcc;

        if (RectLeft(rc) == 0)
            RectLeft(rc) = BorderAdj(wnd);
        if (RectRight(rc) > WindowWidth(wnd)-1)
            RectRight(rc) = WindowWidth(wnd)-1;
        SetStandardColor(wnd);
        memset(line, clrchar, sizeof line);
        line[RectRight(rc)+1] = '\0';
        for (y = RectTop(rc); y <= RectBottom(rc); y++)    {
            if (y < TopBorderAdj(wnd) ||
                    y > ClientHeight(wnd)+1)
                continue;
            writeline(wnd,
                line+(RectLeft(rc)),
                RectLeft(rc),
                y,
                FALSE);
        }
    }
}

/* -- adjust a window's rectangle to clip it to its parent - */
static RECT near ClipRect(WINDOW wnd)
{
    RECT rc;
    rc = wnd->rc;
#ifdef INCLUDE_SHADOWS
    if (TestAttribute(wnd, SHADOW))    {
        RectBottom(rc)++;
        RectRight(rc)++;
    }
#endif
	return ClipRectangle(wnd, rc);
}

/* -- get the video memory that is to be used by a window -- */
void GetVideoBuffer(WINDOW wnd)
{
    RECT rc;
    int ht;
    int wd;

    rc = ClipRect(wnd);
    ht = RectBottom(rc) - RectTop(rc) + 1;
    wd = RectRight(rc) - RectLeft(rc) + 1;
    wnd->videosave = realloc(wnd->videosave, (ht * wd * 2));
    get_videomode();
    if (wnd->videosave != NULL)
        getvideo(rc, wnd->videosave);
}

/* --- restore the video memory that was used by a window -- */
void RestoreVideoBuffer(WINDOW wnd)
{
    if (wnd->videosave != NULL)    {
        RECT rc;
        rc = ClipRect(wnd);
        storevideo(rc, wnd->videosave);
        free(wnd->videosave);
        wnd->videosave = NULL;
    }
}

/* ------ compute the logical line length of a window ------ */
int LineLength(char *ln)
{
    int len = strlen(ln);
    char *cp = ln;
    while ((cp = strchr(cp, CHANGECOLOR)) != NULL)    {
        cp++;
        len -= 3;
    }
    cp = ln;
    while ((cp = strchr(cp, RESETCOLOR)) != NULL)    {
        cp++;
        --len;
    }
    return len;
}

