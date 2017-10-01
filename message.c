/* --------- message.c ---------- */

#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include "dflat.h"

static int px = -1, py = -1;
static int pmx = -1, pmy = -1;
static int mx, my;

static int CriticalError;

/* ---------- event queue ---------- */
static struct events    {
    MESSAGE event;
    int mx;
    int my;
} EventQueue[MAXMESSAGES];

/* ---------- message queue --------- */
static struct msgs {
    WINDOW wnd;
    MESSAGE msg;
    PARAM p1;
    PARAM p2;
} MsgQueue[MAXMESSAGES];

static int EventQueueOnCtr;
static int EventQueueOffCtr;
static int EventQueueCtr;

static int MsgQueueOnCtr;
static int MsgQueueOffCtr;
static int MsgQueueCtr;

static int lagdelay = FIRSTDELAY;

static void (interrupt far *oldtimer)(void) = NULL;
static void (interrupt far *oldkeyboard)(void) = NULL;

static int keyportvalue;	/* for watching for key release */

WINDOW CaptureMouse = NULLWND;
WINDOW CaptureKeyboard = NULLWND;
static int NoChildCaptureMouse = FALSE;
static int NoChildCaptureKeyboard = FALSE;

static int doubletimer = -1;
static int delaytimer  = -1;
#ifdef INCLUDE_CLOCK
static int clocktimer  = -1;
#endif

WINDOW Cwnd = NULLWND;

static void interrupt far newkeyboard(void)
{
	keyportvalue = inp(KEYBOARDPORT);
	oldkeyboard();
}

/* ------- timer interrupt service routine ------- */
static void interrupt far newtimer(void)
{
    if (timer_running(doubletimer))
        countdown(doubletimer);
    if (timer_running(delaytimer))
        countdown(delaytimer);
#ifdef INCLUDE_CLOCK
    if (timer_running(clocktimer))
        countdown(clocktimer);
#endif
    oldtimer();
}

static char ermsg[] = "Error accessing drive x";

/* -------- test for critical errors --------- */
int TestCriticalError(void)
{
    int rtn = 0;
    if (CriticalError)    {
        rtn = 1;
        CriticalError = FALSE;
        if (TestErrorMessage(ermsg) == FALSE)
            rtn = 2;
    }
    return rtn;
}

/* ------ critical error interrupt service routine ------ */
static void interrupt far newcrit(IREGS ir)
{
    if (!(ir.ax & 0x8000))     {
        ermsg[sizeof(ermsg) - 2] = (ir.ax & 0xff) + 'A';
        CriticalError = TRUE;
    }
    ir.ax = 0;
}

/* ------------ initialize the message system --------- */
void init_messages(void)
{
    resetmouse();
	savecursor();
	hidecursor();
    px = py = -1;
    pmx = pmy = -1;
    mx = my = 0;
    CaptureMouse = CaptureKeyboard = NULLWND;
    NoChildCaptureMouse = FALSE;
    NoChildCaptureKeyboard = FALSE;
    MsgQueueOnCtr = MsgQueueOffCtr = MsgQueueCtr = 0;
    EventQueueOnCtr = EventQueueOffCtr = EventQueueCtr = 0;
    if (oldtimer == NULL)    {
        oldtimer = getvect(TIMER);
        setvect(TIMER, newtimer);
    }
    if (oldkeyboard == NULL)    {
        oldkeyboard = getvect(KEYBOARDVECT);
        setvect(KEYBOARDVECT, newkeyboard);
    }
    setvect(CRIT, newcrit);
    PostMessage(NULLWND,START,0,0);
    lagdelay = FIRSTDELAY;
}

/* ----- post an event and parameters to event queue ---- */
static void PostEvent(MESSAGE event, int p1, int p2)
{
    if (EventQueueCtr != MAXMESSAGES)    {
        EventQueue[EventQueueOnCtr].event = event;
        EventQueue[EventQueueOnCtr].mx = p1;
        EventQueue[EventQueueOnCtr].my = p2;
        if (++EventQueueOnCtr == MAXMESSAGES)
            EventQueueOnCtr = 0;
        EventQueueCtr++;
    }
}

/* ------ collect mouse, clock, and keyboard events ----- */
static void near collect_events(void)
{
    static int ShiftKeys = 0;
	int sk;
#ifdef INCLUDE_CLOCK
    struct tm *now;
    static int flipflop = FALSE;
    static char timestr[9];
    int hr;

    /* -------- test for a clock event (one/second) ------- */
    if (timed_out(clocktimer))    {
        /* ----- get the current time ----- */
        time_t t = time(NULL);
        now = localtime(&t);
        hr = now->tm_hour > 12 ?
             now->tm_hour - 12 :
             now->tm_hour;
        if (hr == 0)
            hr = 12;
        sprintf(timestr, "%2d:%02d", hr, now->tm_min);
        strcpy(timestr+5, now->tm_hour > 11 ? "pm " : "am ");
        /* ------- blink the : at one-second intervals ----- */
        if (flipflop)
            *(timestr+2) = ' ';
        flipflop ^= TRUE;
        /* -------- reset the timer -------- */
        set_timer(clocktimer, 1);
        /* -------- post the clock event -------- */
        PostEvent(CLOCKTICK, FP_SEG(timestr), FP_OFF(timestr));
    }
#endif

    /* --------- keyboard events ---------- */
    if ((sk = getshift()) != ShiftKeys)    {
        ShiftKeys = sk;
        /* ---- the shift status changed ---- */
        PostEvent(SHIFT_CHANGED, sk, 0);
    }

    /* ---- build keyboard events for key combinations that
        BIOS doesn't report --------- */
    if (sk & ALTKEY)
        if (inp(0x60) == 14)    {
			waitforkeyboard();
            PostEvent(KEYBOARD, ALT_BS, sk);
        }
    if (sk & CTRLKEY)
        if (inp(0x60) == 82)    {
            while (!(inp(0x60) & 0x80))
			waitforkeyboard();
            PostEvent(KEYBOARD, CTRL_INS, sk);
        }

    /* ----------- test for keystroke ------- */
    if (keyhit())    {
        static int cvt[] = {SHIFT_INS,END,DN,PGDN,BS,'5',
                        FWD,HOME,UP,PGUP};
        int c = getkey();

        /* -------- convert numeric pad keys ------- */
        if (sk & (LEFTSHIFT | RIGHTSHIFT))    {
            if (c >= '0' && c <= '9')
                c = cvt[c-'0'];
            else if (c == '.' || c == DEL)
                c = SHIFT_DEL;
            else if (c == INS)
                c = SHIFT_INS;
        }
		if (c != '\r' && (c < ' ' || c > 127))
			clearBIOSbuffer();
        /* ------ post the keyboard event ------ */
        PostEvent(KEYBOARD, c, sk);
    }

    /* ------------ test for mouse events --------- */
    if (button_releases())    {
        /* ------- the button was released -------- */
        doubletimer = DOUBLETICKS;
        PostEvent(BUTTON_RELEASED, mx, my);
        disable_timer(delaytimer);
    }
    get_mouseposition(&mx, &my);
    if (mx != px || my != py)  {
        px = mx;
        py = my;
        PostEvent(MOUSE_MOVED, mx, my);
    }
    if (rightbutton())
        PostEvent(RIGHT_BUTTON, mx, my);
    if (leftbutton())    {
        if (mx == pmx && my == pmy)    {
            /* ---- same position as last left button ---- */
            if (timer_running(doubletimer))    {
                /* -- second click before double timeout -- */
                disable_timer(doubletimer);
                PostEvent(DOUBLE_CLICK, mx, my);
            }
            else if (!timer_running(delaytimer))    {
                /* ---- button held down a while ---- */
                delaytimer = lagdelay;
                lagdelay = DELAYTICKS;
                /* ---- post a typematic-like button ---- */
                PostEvent(LEFT_BUTTON, mx, my);
            }
        }
        else    {
            /* --------- new button press ------- */
            disable_timer(doubletimer);
            delaytimer = FIRSTDELAY;
            lagdelay = DELAYTICKS;
            PostEvent(LEFT_BUTTON, mx, my);
            pmx = mx;
            pmy = my;
        }
    }
    else
        lagdelay = FIRSTDELAY;
}

/* ----- post a message and parameters to msg queue ---- */
void PostMessage(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    if (MsgQueueCtr != MAXMESSAGES)    {
        MsgQueue[MsgQueueOnCtr].wnd = wnd;
        MsgQueue[MsgQueueOnCtr].msg = msg;
        MsgQueue[MsgQueueOnCtr].p1 = p1;
        MsgQueue[MsgQueueOnCtr].p2 = p2;
        if (++MsgQueueOnCtr == MAXMESSAGES)
            MsgQueueOnCtr = 0;
        MsgQueueCtr++;
    }
}

/* --------- send a message to a window ----------- */
int SendMessage(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn = TRUE, x, y;

#ifdef INCLUDE_LOGGING
	LogMessages(wnd, msg, p1, p2);
#endif
    if (wnd != NULLWND)
        switch (msg)    {
            case PAINT:
            case BORDER:
                /* ------- don't send these messages unless the
                    window is visible -------- */
                if (isVisible(wnd))
	                rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
            case RIGHT_BUTTON:
            case LEFT_BUTTON:
            case DOUBLE_CLICK:
            case BUTTON_RELEASED:
                /* --- don't send these messages unless the
                    window is visible or has captured the mouse -- */
                if (isVisible(wnd) || wnd == CaptureMouse)
	                rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
            case KEYBOARD:
            case SHIFT_CHANGED:
                /* ------- don't send these messages unless the
                    window is visible or has captured the keyboard -- */
                if (isVisible(wnd) || wnd == CaptureKeyboard)
	                rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
            default:
                rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
        }
    /* ----- window processor returned true or the message was sent
        to no window at all (NULLWND) ----- */
    if (rtn != FALSE)    {
        /* --------- process messages that a window sends to the
            system itself ---------- */
        switch (msg)    {
            case STOP:
                if (oldtimer != NULL)    {
                    setvect(TIMER, oldtimer);
                    oldtimer = NULL;
                }
                if (oldkeyboard != NULL)    {
                    setvect(KEYBOARDVECT, oldkeyboard);
                    oldkeyboard = NULL;
                }
                break;
#ifdef INCLUDE_CLOCK
            /* ------- clock messages --------- */
            case CAPTURE_CLOCK:
                Cwnd = wnd;
                set_timer(clocktimer, 0);
                break;
            case RELEASE_CLOCK:
                Cwnd = NULLWND;
                disable_timer(clocktimer);
                break;
#endif
            /* -------- keyboard messages ------- */
            case KEYBOARD_CURSOR:
                if (wnd == NULLWND)
                    cursor((int)p1, (int)p2);
                else
                    cursor(GetClientLeft(wnd)+(int)p1,
                                GetClientTop(wnd)+(int)p2);
                break;
            case CAPTURE_KEYBOARD:
                if (p2)
                    ((WINDOW)p2)->PrevKeyboard=CaptureKeyboard;
                else
                    wnd->PrevKeyboard = CaptureKeyboard;
                CaptureKeyboard = wnd;
                NoChildCaptureKeyboard = (int)p1;
                break;
            case RELEASE_KEYBOARD:
				if (CaptureKeyboard == wnd || (int)p1)
	                CaptureKeyboard = wnd->PrevKeyboard;
				else	{
					WINDOW twnd = CaptureKeyboard;
					while (twnd != NULLWND)	{
						if (twnd->PrevKeyboard == wnd)	{
							twnd->PrevKeyboard = wnd->PrevKeyboard;
							break;
						}
						twnd = twnd->PrevKeyboard;
					}
				}
                wnd->PrevKeyboard = NULLWND;
                NoChildCaptureKeyboard = FALSE;
                break;
            case CURRENT_KEYBOARD_CURSOR:
                curr_cursor(&x, &y);
                *(int*)p1 = x;
                *(int*)p2 = y;
                break;
            case SAVE_CURSOR:
                savecursor();
                break;
            case RESTORE_CURSOR:
                restorecursor();
                break;
            case HIDE_CURSOR:
                normalcursor();
                hidecursor();
                break;
            case SHOW_CURSOR:
                if (p1)
                    set_cursor_type(0x0106);
                else
                    set_cursor_type(0x0607);
                unhidecursor();
                break;
			case WAITKEYBOARD:
				waitforkeyboard();
				break;
            /* -------- mouse messages -------- */
            case MOUSE_INSTALLED:
                rtn = mouse_installed();
                break;
            case SHOW_MOUSE:
                show_mousecursor();
                break;
            case HIDE_MOUSE:
                hide_mousecursor();
                break;
            case MOUSE_CURSOR:
                set_mouseposition((int)p1, (int)p2);
                break;
            case CURRENT_MOUSE_CURSOR:
                get_mouseposition((int*)p1,(int*)p2);
                break;
            case WAITMOUSE:
                waitformouse();
                break;
            case TESTMOUSE:
                rtn = mousebuttons();
                break;
            case CAPTURE_MOUSE:
                if (p2)
                    ((WINDOW)p2)->PrevMouse = CaptureMouse;
                else
                    wnd->PrevMouse = CaptureMouse;
                CaptureMouse = wnd;
                NoChildCaptureMouse = (int)p1;
                break;
            case RELEASE_MOUSE:
				if (CaptureMouse == wnd || (int)p1)
	                CaptureMouse = wnd->PrevMouse;
				else	{
					WINDOW twnd = CaptureMouse;
					while (twnd != NULLWND)	{
						if (twnd->PrevMouse == wnd)	{
							twnd->PrevMouse = wnd->PrevMouse;
							break;
						}
						twnd = twnd->PrevMouse;
					}
				}
                wnd->PrevMouse = NULLWND;
                NoChildCaptureMouse = FALSE;
                break;
            default:
                break;
        }
    }
    return rtn;
}

/* ---- dispatch messages to the message proc function ---- */
int dispatch_message(void)
{
    WINDOW Mwnd, Kwnd;
    /* -------- collect mouse and keyboard events ------- */
    collect_events();
    /* --------- dequeue and process events -------- */
    while (EventQueueCtr > 0)  {
        struct events ev;
			
		ev = EventQueue[EventQueueOffCtr];
        if (++EventQueueOffCtr == MAXMESSAGES)
            EventQueueOffCtr = 0;
        --EventQueueCtr;

        /* ------ get the window in which a
                        mouse event occurred ------ */
        Mwnd = inWindow(ev.mx, ev.my);

        /* ---- process mouse captures ----- */
        if (CaptureMouse != NULLWND)
            if (Mwnd == NULLWND ||
                    NoChildCaptureMouse ||
                        GetParent(Mwnd) != CaptureMouse)
                Mwnd = CaptureMouse;

        /* ------ get the window in which a
                        keyboard event occurred ------ */
        Kwnd = inFocus;

        /* ---- process keyboard captures ----- */
        if (CaptureKeyboard != NULLWND)
            if (Kwnd == NULLWND ||
                    NoChildCaptureKeyboard ||
                        GetParent(Kwnd) != CaptureKeyboard)
                Kwnd = CaptureKeyboard;

        /* -------- send mouse and keyboard messages to the
            window that should get them -------- */
        switch (ev.event)    {
            case SHIFT_CHANGED:
            case KEYBOARD:
                SendMessage(Kwnd, ev.event, ev.mx, ev.my);
                break;
            case LEFT_BUTTON:
                if (!CaptureMouse ||
                        (!NoChildCaptureMouse &&
                            GetParent(Mwnd) == CaptureMouse))
                    if (Mwnd != inFocus)
                        SendMessage(Mwnd, SETFOCUS, TRUE, 0);
            case BUTTON_RELEASED:
            case DOUBLE_CLICK:
            case RIGHT_BUTTON:
            case MOUSE_MOVED:
                SendMessage(Mwnd, ev.event, ev.mx, ev.my);
                break;
#ifdef INCLUDE_CLOCK
            case CLOCKTICK:
                SendMessage(Cwnd, ev.event,
                    (PARAM) MK_FP(ev.mx, ev.my), 0);
				break;
#endif
            default:
                break;
        }
    }
    /* ------ dequeue and process messages ----- */
    while (MsgQueueCtr > 0)  {
        struct msgs mq;

		mq = MsgQueue[MsgQueueOffCtr];
        if (++MsgQueueOffCtr == MAXMESSAGES)
            MsgQueueOffCtr = 0;
        --MsgQueueCtr;
        SendMessage(mq.wnd, mq.msg, mq.p1, mq.p2);
        if (mq.msg == ENDDIALOG)
			return FALSE;
        if (mq.msg == STOP)	{
			restorecursor();	
			unhidecursor();
			return FALSE;
		}
    }
    return TRUE;
}

