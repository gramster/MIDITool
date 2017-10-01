/* ------------- normal.c ------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "dflat.h"

#ifdef INCLUDE_MULTIDOCS
static void near PaintOverLappers(WINDOW wnd);
static void near PaintUnderLappers(WINDOW wnd);
#endif
static int InsideWindow(WINDOW, int, int);
#ifdef INCLUDE_SYSTEM_MENUS
static void TerminateMoveSize(void);
static void SaveBorder(RECT);
static void RestoreBorder(RECT);
static RECT PositionIcon(WINDOW);
static void near dragborder(WINDOW, int, int);
static void near sizeborder(WINDOW, int, int);
static int px = -1, py = -1;
static int diff;
static int conditioning = FALSE;
static struct window dwnd = {DUMMY, NULL, NULL, NormalProc,
                                {-1,-1,-1,-1}};
int WindowMoving = FALSE;
int WindowSizing = FALSE;
#endif
/* -------- array of class definitions -------- */
CLASSDEFS classdefs[] = {
    #undef ClassDef
    #define ClassDef(c,b,p,a) {b,p,a},
    #include "classes.h"
};

int NormalProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int mx = (int) p1 - GetLeft(wnd);
    int my = (int) p2 - GetTop(wnd);
    int DoneClosing = FALSE;
    switch (msg)    {
        case CREATE_WINDOW:
            AppendBuiltWindow(wnd);  /* add to the lists */
            AppendFocusWindow(wnd);
#ifdef INCLUDE_SCROLLBARS
            if (!SendMessage(NULLWND, MOUSE_INSTALLED, 0, 0))
                ClearAttribute(wnd, VSCROLLBAR | HSCROLLBAR);
#endif
            if (TestAttribute(wnd, SAVESELF) && isVisible(wnd))
                GetVideoBuffer(wnd);
            break;
        case SHOW_WINDOW:
            if ((GetParent(wnd) == NULLWND ||
                        isVisible(GetParent(wnd)))
#ifdef INCLUDE_SYSTEM_MENUS
                            && !conditioning
#endif
                                                    )    {
                WINDOW cwnd = Focus.FirstWindow;
                if (TestAttribute(wnd, SAVESELF) &&
                                wnd->videosave == NULL)
                    GetVideoBuffer(wnd);
                SetVisible(wnd);
                SendMessage(wnd, PAINT, 0, 0);
                SendMessage(wnd, BORDER, 0, 0);
                /* --- show the children of this window --- */
                while (cwnd != NULLWND)    {
                    if (GetParent(cwnd) == wnd &&
                            cwnd->condition != ISCLOSING)
                        SendMessage(cwnd, msg, p1, p2);
                    cwnd = NextWindow(cwnd);
                }
            }
            break;
        case HIDE_WINDOW:
            if (isVisible(wnd)
#ifdef INCLUDE_SYSTEM_MENUS
                && !conditioning
#endif
                                )    {
                WINDOW cwnd = Focus.LastWindow;
                /* --- hide the children of this window --- */
                while (cwnd != NULLWND)    {
                    if (GetParent(cwnd) == wnd)
                        ClearVisible(cwnd);
                    cwnd = PrevWindow(cwnd);
                }
                /* --- paint what this window covered --- */
                if (wnd->videosave != NULL)
                    RestoreVideoBuffer(wnd);
#ifdef INCLUDE_MULTIDOCS
                else if (isVisible(GetParent(wnd)))
                    PaintOverLappers(wnd);
#endif
                ClearVisible(wnd);
            }
            break;
#ifdef INCLUDE_HELP
        case DISPLAY_HELP:
            DisplayHelp(wnd, (char *)p1);
            break;
#endif
        case INSIDE_WINDOW:
            return InsideWindow(wnd, (int) p1, (int) p2);
        case KEYBOARD:
#ifdef INCLUDE_HELP
            if ((int)p1 == F1)    {
#ifdef INCLUDE_SYSTEM_MENUS
                if (WindowMoving || WindowSizing)
                    break;
#endif
                SendMessage(wnd, COMMAND, ID_HELP, 0);
                break;
            }
#endif
#ifdef INCLUDE_SYSTEM_MENUS
            if (WindowMoving || WindowSizing)    {
                /* -- move or size a window with keyboard -- */
                int x, y;
                x=WindowMoving?GetLeft(&dwnd):GetRight(&dwnd);
                y=WindowMoving?GetTop(&dwnd):GetBottom(&dwnd);
                switch ((int)p1)    {
                    case ESC:
                        TerminateMoveSize();
                        return TRUE;
                    case UP:
                        if (y)
                            --y;
                        break;
                    case DN:
                        if (y < SCREENHEIGHT-1)
                            y++;
                        break;
                    case FWD:
                        if (x < SCREENWIDTH-1)
                            x++;
                        break;
                    case BS:
                        if (x)
                            --x;
                        break;
                    case '\r':
                        SendMessage(wnd,BUTTON_RELEASED,x,y);
                    default:
                        return TRUE;
                }
                /* -- use the mouse functions to move/size - */
                SendMessage(wnd, MOUSE_CURSOR, x, y);
                SendMessage(wnd, MOUSE_MOVED, x, y);
                break;
            }
#endif
        /* --- unprocessed keystrokes fall through here --- */
        case ADDSTATUS:
        case SHIFT_CHANGED:
            PostMessage(GetParent(wnd), msg, p1, p2);
            break;
        case PAINT:
            if (isVisible(wnd))    
                ClearWindow(wnd, (RECT *)p1, ' ');
            break;
        case BORDER:
            if (isVisible(wnd))    {
                if (TestAttribute(wnd, HASBORDER))
                    RepaintBorder(wnd, (RECT *)p1);
                else if (TestAttribute(wnd, HASTITLEBAR))
                    DisplayTitle(wnd, (RECT *)p1);
                if (wnd->StatusBar != NULLWND)
                    SendMessage(wnd->StatusBar, PAINT, p1, 0);
            }
            break;
#ifdef INCLUDE_SYSTEM_MENUS
        case COMMAND:
            switch ((int)p1)    {
#ifdef INCLUDE_HELP
                case ID_HELP:
                    DisplayHelp(wnd, ClassNames[GetClass(wnd)]);
                    break;
#endif
                case ID_SYSRESTORE:
                    SendMessage(wnd, RESTORE, 0, 0);
                    break;
                case ID_SYSMOVE:
                    SendMessage(wnd, CAPTURE_MOUSE, TRUE,
                        (PARAM) &dwnd);
                    SendMessage(wnd, CAPTURE_KEYBOARD, TRUE,
                        (PARAM) &dwnd);
                    SendMessage(wnd, MOUSE_CURSOR,
                        GetLeft(wnd), GetTop(wnd));
                    WindowMoving = TRUE;
                    dragborder(wnd, GetLeft(wnd), GetTop(wnd));
                    break;
                case ID_SYSSIZE:
                    SendMessage(wnd, CAPTURE_MOUSE, TRUE,
                        (PARAM) &dwnd);
                    SendMessage(wnd, CAPTURE_KEYBOARD, TRUE,
                        (PARAM) &dwnd);
                    SendMessage(wnd, MOUSE_CURSOR,
                        GetRight(wnd), GetBottom(wnd));
                    WindowSizing = TRUE;
                    dragborder(wnd, GetLeft(wnd), GetTop(wnd));
                    break;
                case ID_SYSMINIMIZE:
                    SendMessage(wnd, MINIMIZE, 0, 0);
                    break;
                case ID_SYSMAXIMIZE:
                    SendMessage(wnd, MAXIMIZE, 0, 0);
                    break;
                case ID_SYSCLOSE:
                    SendMessage(wnd, CLOSE_WINDOW, 0, 0);
                    break;
                default:
                    break;
            }
            break;
#endif
        case SETFOCUS:
            if (p1 && inFocus != wnd)    {
                WINDOW pwnd = GetParent(wnd);
                   int Redraw = isVisible(wnd) &&
                       !TestAttribute(wnd, SAVESELF);
                if (GetClass(pwnd) == APPLICATION)    {
                    WINDOW cwnd = Focus.FirstWindow;
                    /* -- if no children, do not need selective
                        redraw --- */
                    while (cwnd != NULLWND)    {
                        if (GetParent(cwnd) == wnd)
                            break;
                        cwnd = NextWindow(cwnd);
                    }
                    if (cwnd == NULLWND)
                        Redraw = FALSE;
                }
                /* ---- setting focus ------ */
                SendMessage(inFocus, SETFOCUS, FALSE, 0);
#ifdef INCLUDE_MULTIDOCS
                if (Redraw)
                    PaintUnderLappers(wnd);
#endif
                /* remove window from list */
                RemoveFocusWindow(wnd);
                /* move window to end of list */
                AppendFocusWindow(wnd);
                inFocus = wnd;

                if (Redraw)
                    SendMessage(wnd, BORDER, 0, 0);
                else    {
                    if (pwnd == NULLWND ||
#ifdef INCLUDE_DIALOGS
                        GetClass(pwnd) == DIALOG ||
                            DerivedClass(GetClass(pwnd)) ==
                                DIALOG ||
#endif
				GetClass(pwnd) ==APPLICATION)
                        SendMessage(wnd, SHOW_WINDOW, 0, 0);
                    else
                        SendMessage(pwnd, SHOW_WINDOW, 0, 0);
                }
            }
            else if (!p1 && inFocus == wnd)    {
                /* -------- clearing focus --------- */
                inFocus = NULL;
                SendMessage(wnd, BORDER, 0, 0);
            }
            break;
        case DOUBLE_CLICK:
            if (!SendMessage(wnd, INSIDE_WINDOW, p1, p2) &&
                        CaptureMouse == NULLWND)    {
                PostMessage(GetParent(wnd), msg, p1, p2);
                break;
            }
#ifdef INCLUDE_SYSTEM_MENUS
            if (!WindowSizing && !WindowMoving)
#endif
                if (HitControlBox(wnd, mx, my))
                    PostMessage(wnd, CLOSE_WINDOW, 0, 0);
            break;
        case LEFT_BUTTON:
#ifdef INCLUDE_SYSTEM_MENUS
            if (WindowSizing || WindowMoving)
                return FALSE;
#endif
            if ((!SendMessage(wnd, INSIDE_WINDOW, p1, p2) &&
                    CaptureMouse == NULLWND) ||
                        HitControlBox(wnd, mx, my))    {
                PostMessage(GetParent(wnd), msg, p1, p2);
                break;
            }
#ifdef INCLUDE_SYSTEM_MENUS
            if (my == 0 && TestAttribute(wnd, HASTITLEBAR))   {
                /* ---------- hit the title bar -------- */
                if (TestAttribute(wnd, MINMAXBOX))        {
                    if (mx == WindowWidth(wnd)-2)    {
                        if (wnd->condition == ISRESTORED)
                            /* --- hit the maximize box --- */
                            SendMessage(wnd, MAXIMIZE, 0, 0);
                        else
                            /* --- hit the restore box --- */
                            SendMessage(wnd, RESTORE, 0, 0);
                        break;
                    }
                    if (mx == WindowWidth(wnd)-3)    {
                        /* --- hit the minimize box --- */
                        if (wnd->condition != ISMINIMIZED)
                            SendMessage(wnd, MINIMIZE, 0, 0);
                        break;
                    }
                }
                if (wnd->condition != ISMAXIMIZED &&
                            TestAttribute(wnd, MOVEABLE))    {
                    WindowMoving = TRUE;
                    px = mx;
                    py = my;
                    diff = (int) mx;
                    SendMessage(wnd, CAPTURE_MOUSE, TRUE,
                        (PARAM) &dwnd);
                    dragborder(wnd, GetLeft(wnd), GetTop(wnd));
                }
                break;
            }
            if (mx == WindowWidth(wnd)-1 &&
                    my == WindowHeight(wnd)-1)    {
                /* ------- hit the resize corner ------- */
                if (wnd->condition == ISMINIMIZED ||
                        !TestAttribute(wnd, SIZEABLE))
                    break;
                if (wnd->condition == ISMAXIMIZED)    {
                    if (TestAttribute(GetParent(wnd),HASBORDER))
                        break;
                    /* ----- resizing a maximized window over a
                            borderless parent ----- */
                    wnd = GetParent(wnd);
                }
                WindowSizing = TRUE;
                SendMessage(wnd, CAPTURE_MOUSE,
                    TRUE, (PARAM) &dwnd);
                dragborder(wnd, GetLeft(wnd), GetTop(wnd));
            }
#endif
            break;
#ifdef INCLUDE_SYSTEM_MENUS
        case MOUSE_MOVED:
            if (WindowMoving)    {
                int leftmost = 0, topmost = 0,
                    bottommost = SCREENHEIGHT-2,
                    rightmost = SCREENWIDTH-2;
                int x = (int) p1 - diff;
                int y = (int) p2;
                if (GetParent(wnd) != NULLWND &&
                        !TestAttribute(wnd, NOCLIP))    {
                    WINDOW wnd1 = GetParent(wnd);
                    topmost    = GetClientTop(wnd1);
                    leftmost   = GetClientLeft(wnd1);
                    bottommost = GetClientBottom(wnd1);
                    rightmost  = GetClientRight(wnd1);
                }
                if (x < leftmost || x > rightmost ||
                        y < topmost || y > bottommost)    {
                    x = max(x, leftmost);
                    x = min(x, rightmost);
                    y = max(y, topmost);
                    y = min(y, bottommost);
                    SendMessage(NULLWND,MOUSE_CURSOR,x+diff,y);
                }
                if (x != px || y != py)    {
                    px = x;
                    py = y;
                    dragborder(wnd, x, y);
                }
                return TRUE;
            }
            if (WindowSizing)    {
                sizeborder(wnd, (int) p1, (int) p2);
                return TRUE;
            }
            break;
        case BUTTON_RELEASED:
            if (WindowMoving || WindowSizing)    {
                if (WindowMoving)
                    PostMessage(wnd,MOVE,dwnd.rc.lf,dwnd.rc.tp);
                else
                    PostMessage(wnd,SIZE,dwnd.rc.rt,dwnd.rc.bt);
                TerminateMoveSize();
            }
            break;
        case MAXIMIZE:
            if (wnd->condition != ISMAXIMIZED)    {
                RECT rc = {0, 0, 0, 0};
                RECT holdrc;
                holdrc = wnd->RestoredRC;
                rc.rt = SCREENWIDTH-1;
                rc.bt = SCREENHEIGHT-1;
                if (GetParent(wnd))
                    rc = ClientRect(GetParent(wnd));
                wnd->condition = ISMAXIMIZED;
                SendMessage(wnd, HIDE_WINDOW, 0, 0);
                conditioning = TRUE;
                SendMessage(wnd, MOVE,
                    RectLeft(rc), RectTop(rc));
                SendMessage(wnd, SIZE,
                    RectRight(rc), RectBottom(rc));
                conditioning = FALSE;
                if (wnd->restored_attrib == 0)
                    wnd->restored_attrib = wnd->attrib;
#ifdef INCLUDE_SHADOWS
                ClearAttribute(wnd,    SHADOW);
#endif
                SendMessage(wnd, SHOW_WINDOW, 0, 0);
                wnd->RestoredRC = holdrc;
            }
            break;
        case MINIMIZE:
            if (wnd->condition != ISMINIMIZED)    {
                RECT rc;
                RECT holdrc;

                holdrc = wnd->RestoredRC;
                rc = PositionIcon(wnd);
                wnd->condition = ISMINIMIZED;
                SendMessage(wnd, HIDE_WINDOW, 0, 0);
                conditioning = TRUE;
                SendMessage(wnd, MOVE,
                    RectLeft(rc), RectTop(rc));
                SendMessage(wnd, SIZE,
                    RectRight(rc), RectBottom(rc));
                SetPrevFocus(wnd);
                conditioning = FALSE;
                if (wnd->restored_attrib == 0)
                    wnd->restored_attrib = wnd->attrib;
                ClearAttribute(wnd,
                    SHADOW | SIZEABLE | HASMENUBAR |
                    VSCROLLBAR | HSCROLLBAR);
                SendMessage(wnd, SHOW_WINDOW, 0, 0);
                wnd->RestoredRC = holdrc;
            }
            break;
        case RESTORE:
            if (wnd->condition != ISRESTORED)    {
                RECT holdrc;
                holdrc = wnd->RestoredRC;
                wnd->condition = ISRESTORED;
                SendMessage(wnd, HIDE_WINDOW, 0, 0);
                wnd->attrib = wnd->restored_attrib;
                wnd->restored_attrib = 0;
                conditioning = TRUE;
                SendMessage(wnd, MOVE, wnd->RestoredRC.lf,
                    wnd->RestoredRC.tp);
                wnd->RestoredRC = holdrc;
                SendMessage(wnd, SIZE, wnd->RestoredRC.rt,
                    wnd->RestoredRC.bt);
                SendMessage(wnd, SETFOCUS, TRUE, 0);
                conditioning = FALSE;
                SendMessage(wnd, SHOW_WINDOW, 0, 0);
            }
            break;
        case MOVE:    {
            WINDOW wnd1 = Focus.FirstWindow;
            int wasVisible = isVisible(wnd);
            int xdif = (int) p1 - wnd->rc.lf;
            int ydif = (int) p2 - wnd->rc.tp;

            if (xdif == 0 && ydif == 0)
                return FALSE;
            if (wasVisible)
                SendMessage(wnd, HIDE_WINDOW, 0, 0);
            wnd->rc.lf = (int) p1;
            wnd->rc.tp = (int) p2;
            wnd->rc.rt = GetLeft(wnd)+WindowWidth(wnd)-1;
            wnd->rc.bt = GetTop(wnd)+WindowHeight(wnd)-1;
            if (wnd->condition == ISRESTORED)
                wnd->RestoredRC = wnd->rc;
            while (wnd1 != NULLWND)    {
                if (GetParent(wnd1) == wnd)
                    SendMessage(wnd1, MOVE,
                        wnd1->rc.lf+xdif, wnd1->rc.tp+ydif);
                wnd1 = NextWindow(wnd1);
            }
            if (wasVisible)
                SendMessage(wnd, SHOW_WINDOW, 0, 0);
            break;
        }
        case SIZE:    {
            int wasVisible = isVisible(wnd);
            WINDOW wnd1 = Focus.FirstWindow;
            RECT rc;
            int xdif = (int) p1 - wnd->rc.rt;
            int ydif = (int) p2 - wnd->rc.bt;

            if (xdif == 0 && ydif == 0)
                return FALSE;
            if (wasVisible)
                SendMessage(wnd, HIDE_WINDOW, 0, 0);
            wnd->rc.rt = (int) p1;
            wnd->rc.bt = (int) p2;
            wnd->ht = GetBottom(wnd)-GetTop(wnd)+1;
            wnd->wd = GetRight(wnd)-GetLeft(wnd)+1;

            if (wnd->condition == ISRESTORED)
                wnd->RestoredRC = WindowRect(wnd);

            rc = ClientRect(wnd);
            while (wnd1 != NULLWND)    {
                if (GetParent(wnd1) == wnd &&
                        wnd1->condition == ISMAXIMIZED)
                    SendMessage(wnd1, SIZE, RectRight(rc),
                                            RectBottom(rc));
                wnd1 = NextWindow(wnd1);
            }

            if (wasVisible)
                SendMessage(wnd, SHOW_WINDOW, 0, 0);
            break;
        }
#endif
        case CLOSE_WINDOW:
            wnd->condition = ISCLOSING;
            if (wnd->PrevMouse != NULLWND)
                SendMessage(wnd, RELEASE_MOUSE, 0, 0);
            if (wnd->PrevKeyboard != NULLWND)
                SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
            /* ----------- hide this window ------------ */
            SendMessage(wnd, HIDE_WINDOW, 0, 0);
            /* --- close the children of this window --- */
            while (!DoneClosing)    {
                WINDOW wnd1 = Focus.LastWindow;
                DoneClosing = TRUE;
                while (wnd1 != NULLWND)    {
                    WINDOW prwnd = PrevWindow(wnd1);
                    if (GetParent(wnd1) == wnd)    {
                        if (inFocus == wnd1)    {
                            RemoveFocusWindow(wnd);
                            AppendFocusWindow(wnd);
                            inFocus = wnd;
                        }
                        SendMessage(wnd1,CLOSE_WINDOW,0,0);
                        DoneClosing = FALSE;
                        break;
                    }
                    wnd1 = prwnd;
                }
            }
            /* --- change focus if this window had it -- */
            SetPrevFocus(wnd);
            /* ------- remove this window from the
                    list of open windows ------------- */
            RemoveBuiltWindow(wnd);
            /* ------- remove this window from the
                    list of in-focus windows ---------- */
            RemoveFocusWindow(wnd);
            /* -- free memory allocated to this window - */
            if (wnd->title != NULL)
                free(wnd->title);
            if (wnd->videosave != NULL)
                free(wnd->videosave);
            free(wnd);
            break;
        default:
            break;
    }
    return TRUE;
}
/* ---- compute lower left icon space in a rectangle ---- */
#ifdef INCLUDE_SYSTEM_MENUS
static RECT LowerLeft(RECT prc)
{
    RECT rc;
    RectLeft(rc) = RectRight(prc) - ICONWIDTH;
    RectTop(rc) = RectBottom(prc) - ICONHEIGHT;
    RectRight(rc) = RectLeft(rc)+ICONWIDTH-1;
    RectBottom(rc) = RectTop(rc)+ICONHEIGHT-1;
    return rc;
}
/* ----- compute a position for a minimized window icon ---- */
static RECT PositionIcon(WINDOW wnd)
{
    RECT rc;
    RectLeft(rc) = SCREENWIDTH-ICONWIDTH;
    RectTop(rc) = SCREENHEIGHT-ICONHEIGHT;
    RectRight(rc) = SCREENWIDTH-1;
    RectBottom(rc) = SCREENHEIGHT-1;
    if (GetParent(wnd))    {
        WINDOW wnd1 = (WINDOW) -1;
        RECT prc;
        prc = WindowRect(GetParent(wnd));
        rc = LowerLeft(prc);
        /* - search for icon available location - */
        while (wnd1 != NULLWND)    {
            wnd1 = GetFirstChild(GetParent(wnd));
            while (wnd1 != NULLWND)    {
                if (wnd1->condition == ISMINIMIZED)    {
                    RECT rc1;
                    rc1 = WindowRect(wnd1);
                    if (RectLeft(rc1) == RectLeft(rc) &&
                            RectTop(rc1) == RectTop(rc))    {
                        RectLeft(rc) -= ICONWIDTH;
                        RectRight(rc) -= ICONWIDTH;
                        if (RectLeft(rc) < RectLeft(prc)+1)   {
                            RectLeft(rc) =
                                RectRight(prc)-ICONWIDTH;
                            RectRight(rc) =
                                RectLeft(rc)+ICONWIDTH-1;
                            RectTop(rc) -= ICONHEIGHT;
                            RectBottom(rc) -= ICONHEIGHT;
                            if (RectTop(rc) < RectTop(prc)+1)
                                return LowerLeft(prc);
                        }
                        break;
                    }
                }
                wnd1 = GetNextChild(GetParent(wnd), wnd1);
            }
        }
    }
    return rc;
}
/* ----- terminate the move or size operation ----- */
static void TerminateMoveSize(void)
{
    px = py = -1;
    diff = 0;
    SendMessage(&dwnd, RELEASE_MOUSE, TRUE, 0);
    SendMessage(&dwnd, RELEASE_KEYBOARD, TRUE, 0);
    RestoreBorder(dwnd.rc);
    WindowMoving = WindowSizing = FALSE;
}
/* ---- build a dummy window border for moving or sizing --- */
static void near dragborder(WINDOW wnd, int x, int y)
{
    RestoreBorder(dwnd.rc);
    /* ------- build the dummy window -------- */
    dwnd.rc.lf = x;
    dwnd.rc.tp = y;
    dwnd.rc.rt = dwnd.rc.lf+WindowWidth(wnd)-1;
    dwnd.rc.bt = dwnd.rc.tp+WindowHeight(wnd)-1;
    dwnd.ht = WindowHeight(wnd);
    dwnd.wd = WindowWidth(wnd);
    dwnd.parent = GetParent(wnd);
    dwnd.attrib = VISIBLE | HASBORDER | NOCLIP;
    SaveBorder(dwnd.rc);
    RepaintBorder(&dwnd, NULL);
}
/* ---- write the dummy window border for sizing ---- */
static void near sizeborder(WINDOW wnd, int rt, int bt)
{
    int leftmost = GetLeft(wnd)+10;
    int topmost = GetTop(wnd)+3;
    int bottommost = SCREENHEIGHT-1;
    int rightmost  = SCREENWIDTH-1;
    if (GetParent(wnd))    {
        bottommost = min(bottommost,
            GetClientBottom(GetParent(wnd)));
        rightmost  = min(rightmost,
            GetClientRight(GetParent(wnd)));
    }
    rt = min(rt, rightmost);
    bt = min(bt, bottommost);
    rt = max(rt, leftmost);
    bt = max(bt, topmost);
    SendMessage(NULLWND, MOUSE_CURSOR, rt, bt);

    if (rt != px || bt != py)
        RestoreBorder(dwnd.rc);

    /* ------- change the dummy window -------- */
    dwnd.ht = bt-dwnd.rc.tp+1;
    dwnd.wd = rt-dwnd.rc.lf+1;
    dwnd.rc.rt = rt;
    dwnd.rc.bt = bt;
    if (rt != px || bt != py)    {
        px = rt;
        py = bt;
        SaveBorder(dwnd.rc);
        RepaintBorder(&dwnd, NULL);
    }
}
#endif
/* ----- adjust a rectangle to include the shadow ----- */
#ifdef INCLUDE_SHADOWS
static RECT adjShadow(WINDOW wnd)
{
    RECT rc;
    rc = wnd->rc;
    if (TestAttribute(wnd, SHADOW))    {
        if (RectRight(rc) < SCREENWIDTH-1)
            RectRight(rc)++;           
        if (RectBottom(rc) < SCREENHEIGHT-1)
            RectBottom(rc)++;
    }
    return rc;
}
#endif
/* --- repaint a rectangular subsection of a window --- */
#ifdef INCLUDE_MULTIDOCS
static void near PaintOverLap(WINDOW wnd, RECT rc)
{
    int isBorder, isTitle, isData;
    isBorder = isTitle = FALSE;
    isData = TRUE;
    if (TestAttribute(wnd, HASBORDER))    {
        isBorder =  RectLeft(rc) == 0 &&
                    RectTop(rc) < WindowHeight(wnd);
        isBorder |= RectLeft(rc) < WindowWidth(wnd) &&
                    RectRight(rc) >= WindowWidth(wnd)-1 &&
                    RectTop(rc) < WindowHeight(wnd);
        isBorder |= RectTop(rc) == 0 &&
                    RectLeft(rc) < WindowWidth(wnd);
        isBorder |= RectTop(rc) < WindowHeight(wnd) &&
                    RectBottom(rc) >= WindowHeight(wnd)-1 &&
                    RectLeft(rc) < WindowWidth(wnd);
    }
    else if (TestAttribute(wnd, HASTITLEBAR))
        isTitle = RectTop(rc) == 0 &&
                  RectLeft(rc) > 0 &&
                  RectLeft(rc)<WindowWidth(wnd)-BorderAdj(wnd);

    if (RectLeft(rc) >= WindowWidth(wnd)-BorderAdj(wnd))
        isData = FALSE;
    if (RectTop(rc) >= WindowHeight(wnd)-BorderAdj(wnd))
        isData = FALSE;
    if (TestAttribute(wnd, HASBORDER))    {
        if (RectRight(rc) == 0)
            isData = FALSE;
        if (RectBottom(rc) == 0)
            isData = FALSE;
    }
#ifdef INCLUDE_SHADOWS
    if (TestAttribute(wnd, SHADOW))
        isBorder |= RectRight(rc) == WindowWidth(wnd) ||
                    RectBottom(rc) == WindowHeight(wnd);
#endif
    if (isData)
        SendMessage(wnd, PAINT, (PARAM) &rc, 0);
    if (isBorder)
        SendMessage(wnd, BORDER, (PARAM) &rc, 0);
    else if (isTitle)
        DisplayTitle(wnd, &rc);
}
WINDOW HiddenWindow;
/* ------ paint the part of a window that is overlapped
            by another window that is being hidden ------- */
static void PaintOver(WINDOW wnd)
{
        RECT wrc, rc;
#ifdef INCLUDE_SHADOWS
        wrc = adjShadow(HiddenWindow);
        rc = adjShadow(wnd);
#else
        wrc = HiddenWindow->rc;
        rc = wnd->rc;
#endif
        rc = subRectangle(rc, wrc);
        if (ValidRect(rc))
            PaintOverLap(wnd, RelativeWindowRect(wnd, rc));
}
/* --- paint the overlapped parts of all children --- */
static void PaintOverChildren(WINDOW pwnd)
{
    WINDOW cwnd = GetFirstFocusChild(pwnd);
    while (cwnd != NULLWND)    {
        if (cwnd != HiddenWindow)    {
            PaintOver(cwnd);
            PaintOverChildren(cwnd);
        }
        cwnd = GetNextFocusChild(pwnd, cwnd);
    }
}
/* -- recursive overlapping paint of parents -- */
static void PaintOverParents(WINDOW wnd)
{
    WINDOW pwnd = GetParent(wnd);
    if (pwnd != NULL)    {
        PaintOverParents(pwnd);
        PaintOver(pwnd);
        PaintOverChildren(pwnd);
    }
}
/* - paint the parts of all windows that a window is over - */
static void near PaintOverLappers(WINDOW wnd)
{
    if (isVisible(wnd))    {
        HiddenWindow = wnd;
        PaintOverParents(wnd);
    }
}
/* --- paint those parts of a window that are overlapped --- */
static void near PaintUnder(WINDOW wnd)
{
    WINDOW hwnd = Focus.FirstWindow;
    while (hwnd != NULLWND)    {
        /* ---- don't bother testing self ----- */
        if (hwnd != wnd)    {
            /* --- see if other window is descendent --- */
            WINDOW pwnd = GetParent(hwnd);
            while (pwnd != NULLWND)    {
                if (pwnd == wnd)
                    break;
                pwnd = GetParent(pwnd);
            }
            /* ----- don't test descendent overlaps ----- */
            if (pwnd == NULLWND)    {
                /* -- see if other window is ancestor --- */
                pwnd = GetParent(wnd);
                while (pwnd != NULLWND)    {
                    if (pwnd == hwnd)
                        break;
                    pwnd = GetParent(pwnd);
                }
                /* --- don't test ancestor overlaps --- */
                if (pwnd == NULLWND)    {
                    /* ---- other window must be ahead in
                        focus chain ----- */
                    WINDOW fwnd = NextWindow(wnd);
                    while (fwnd != NULLWND)    {
                        if (fwnd == hwnd)
                            break;
                        fwnd = NextWindow(fwnd);
                    }
                    if (fwnd != NULLWND)    {
                        HiddenWindow = hwnd;
                        PaintOver(wnd);
                    }
                }
            }
        }
        hwnd = NextWindow(hwnd);
    }
    /* --------- repaint all children of this window
        the same way ----------- */
    hwnd = Focus.FirstWindow;
    while (hwnd != NULLWND)    {
        if (GetParent(hwnd) == wnd)
            PaintUnder(hwnd);
        hwnd = NextWindow(hwnd);
    }
}
/* paint the parts of a window that are under other windows */
static void near PaintUnderLappers(WINDOW wnd)
{
    WINDOW pwnd = wnd;
    /* find oldest ancestor younger than application window */
    while (pwnd != NULLWND && GetClass(pwnd) != APPLICATION) {
        if (TestAttribute(wnd, SAVESELF))
            break;
        wnd = pwnd;
        pwnd = GetParent(pwnd);
    }
    PaintUnder(wnd);
}
#endif

#ifdef INCLUDE_SYSTEM_MENUS

static int *Bsave = NULL;
static int Bht, Bwd;
/* --- save video area to be used by dummy window border --- */
static void SaveBorder(RECT rc)
{
    Bht = RectBottom(rc) - RectTop(rc) + 1;
    Bwd = RectRight(rc) - RectLeft(rc) + 1;
    if ((Bsave = realloc(Bsave, (Bht + Bwd) * 4)) != NULL)    {
        RECT lrc;
        int i;
        int *cp;

        lrc = rc;
        RectBottom(lrc) = RectTop(lrc);
        getvideo(lrc, Bsave);
        RectTop(lrc) = RectBottom(lrc) = RectBottom(rc);
        getvideo(lrc, Bsave + Bwd);
        cp = Bsave + Bwd * 2;
        for (i = 1; i < Bht-1; i++)    {
            *cp++ = GetVideoChar(RectLeft(rc),RectTop(rc)+i);
            *cp++ = GetVideoChar(RectRight(rc),RectTop(rc)+i);
        }
    }
}
/* ---- restore video area used by dummy window border ---- */
static void RestoreBorder(RECT rc)
{
    if (Bsave != NULL)    {
        RECT lrc;
        int i;
        int *cp;
        lrc = rc;
        RectBottom(lrc) = RectTop(lrc);
        storevideo(lrc, Bsave);
        RectTop(lrc) = RectBottom(lrc) = RectBottom(rc);
        storevideo(lrc, Bsave + Bwd);
        cp = Bsave + Bwd * 2;
        for (i = 1; i < Bht-1; i++)    {
            PutVideoChar(RectLeft(rc),RectTop(rc)+i, *cp++);
            PutVideoChar(RectRight(rc),RectTop(rc)+i, *cp++);
        }
        free(Bsave);
        Bsave = NULL;
    }
}
#endif
/* ----- test if screen coordinates are in a window ---- */
static int InsideWindow(WINDOW wnd, int x, int y)
{
    RECT rc;
    rc = WindowRect(wnd);
    if (!TestAttribute(wnd, NOCLIP))    {
        WINDOW pwnd = GetParent(wnd);
        while (pwnd != NULL)    {
            rc = subRectangle(rc, ClientRect(pwnd));
            pwnd = GetParent(pwnd);
        }
    }
    return InsideRect(x, y, rc);
}
/* ----- find window that screen coordinates are in --- */
WINDOW inWindow(int x, int y)
{
    WINDOW wnd = Focus.LastWindow;
    while (wnd != NULLWND)    {
        if (SendMessage(wnd, INSIDE_WINDOW, x, y))    {
            WINDOW wnd1 = GetLastChild(wnd);
            while (wnd1 != NULLWND)    {
                if (SendMessage(wnd1, INSIDE_WINDOW, x, y)) {
                    if (isVisible(wnd))  {
                        wnd = wnd1;
                        break;
                    }
                }
                wnd1 = GetPrevChild(wnd, wnd1);
            }
            break;
        }
        wnd = PrevWindow(wnd);
    }
    return wnd;
}
/* --------- set window colors --------- */
void SetStandardColor(WINDOW wnd)
{
    foreground = WndForeground(wnd);
    background = WndBackground(wnd);
}

void SetReverseColor(WINDOW wnd)
{
    foreground = SelectForeground(wnd);
    background = SelectBackground(wnd);
}

void SetClassColors(CLASS class)
{
    foreground = cfg.clr [class] [STD_COLOR] [FG];
    background = cfg.clr [class] [STD_COLOR] [BG];
}
/* ---- test window visible through chain of ancestors ---- */
int isVisible(WINDOW wnd)
{
    while (wnd != NULLWND)    {
        if (!isWndVisible(wnd))
            return FALSE;
        wnd = GetParent(wnd);
    }
    return TRUE;
}

