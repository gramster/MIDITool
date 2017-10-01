/* ------------- dflat.h ----------- */
#ifndef WINDOW_H
#define WINDOW_H

#define VERSION "Version 1.0"

#define TRUE 1
#define FALSE 0

#define DFLAT_APPLICATION "MIDITOOL"

#define INCLUDE_SYSTEM_MENUS
#define INCLUDE_MULTIDOCS

#define INCLUDE_SHADOWS

#define INCLUDE_DIALOG_BOXES
#define INCLUDE_STATUSBAR
#define INCLUDE_CLOCK
#define INCLUDE_HELP
#define INCLUDE_COMPRESS_HELPFILE

#define PUSHBUTTON_DEPRESS
#define INCLUDE_SCROLLBARS
#define INCLUDE_MULTILINE
#define INCLUDE_CLIPBOARD

#ifdef TESTING_DFLAT
#define INCLUDE_LOGGING
#define INCLUDE_RELOADHELP
#endif

#define MAXMESSAGES 50
#define DELAYTICKS 1
#define FIRSTDELAY 7
#define DOUBLETICKS 5

typedef enum messages {
	#undef DFlatMsg
	#define DFlatMsg(m) m,
	#include "dflatmsg.h"
	MESSAGECOUNT
} MESSAGE;

typedef enum window_class    {
	#define ClassDef(c,b,p,a) c,
	#include "classes.h"
	CLASSCOUNT
} CLASS;

#include "system.h"
#include "config.h"
#include "rect.h"
#include "menu.h"
#include "keys.h"
#include "commands.h"
#include "dialbox.h"

/* ------ integer type for message parameters ----- */
typedef long PARAM;

typedef struct window {
    CLASS class;           /* window class                  */
    char *title;           /* window title                  */
    struct window *parent; /* parent window                 */
    int (*wndproc)
        (struct window *, enum messages, PARAM, PARAM);
    /* ---------------- window dimensions ----------------- */
    RECT rc;               /* window coordinates
                                            (0/0 to 79/24)  */
    int ht, wd;            /* window height and width       */
    RECT RestoredRC;       /* restored condition rect       */
    /* -------------- linked list pointers ---------------- */
    struct window *nextfocus;   /* next window on screen    */
    struct window *prevfocus;   /* previous window on screen*/
    struct window *nextbuilt;   /* next window built        */
    struct window *prevbuilt;   /* previous window built    */

    int attrib;                 /* Window attributes        */
    char *videosave;            /* video save buffer        */
    int condition;              /* Restored, Maximized,
                                   Minimized, Closing       */
    int restored_attrib;        /* attributes when restored */
    void *extension;      /* menus, dialogs, documents, etc */
    struct window *PrevMouse;
    struct window *PrevKeyboard;
	struct window *MenuBar;   /* menu bar                   */
	struct window *StatusBar; /* status bar                 */
    /* ----------------- text box fields ------------------ */
    int wlines;     /* number of lines of text              */
    int wtop;       /* text line that is on the top display */
    unsigned char *text;   /* window text                   */
    unsigned int textlen;  /* text length                   */
    int wleft;      /* left position in window viewport     */
    int textwidth;  /* width of longest line in textbox     */
    int BlkBegLine; /* beginning line of marked block       */
    int BlkBegCol;  /* beginning column of marked block     */
    int BlkEndLine; /* ending line of marked block          */
    int BlkEndCol;  /* ending column of marked block        */
    int HScrollBox; /* position of horizontal scroll box    */
    int VScrollBox; /* position of vertical scroll box      */
	unsigned int *TextPointers; /* -> list of line offsets	*/
    /* ----------------- list box fields ------------------ */
    int selection;  /* current selection                    */
    int AddMode;    /* adding extended selections mode      */
    int AnchorPoint;/* anchor point for extended selections */
    int SelectCount;/* count of selected items              */
    /* ----------------- edit box fields ------------------ */
    int CurrCol;    /* Current column                       */
    int CurrLine;   /* Current line                         */
    int WndRow;     /* Current window row                   */
    int TextChanged; /* TRUE if text has changed            */
    unsigned char *DeletedText; /* for undo                 */
    int DeletedLength; /*  "   "                            */
    /* ---------------- dialog box fields ----------------- */
    struct window *dFocus; /* control that has the focus    */
    int ReturnCode;        /* return code from a dialog box */
	int Modal;		       /* True if a modeless dialog box */
	/* -------------- popdownmenu fields ------------------ */
	MENU *mnu;		/* points to menu structure             */
	MENU *holdmenu; /* previous active menu                 */
	/* --------------- help box fields -------------------- */
	void *firstword; /* -> first in list of key words       */
	void *lastword;  /* -> last in list of key words        */
	void *thisword;  /* -> current in list of key words     */
	/* -------------- status bar fields ------------------- */
	int TimePosted;  /* True if time has been posted        */
} * WINDOW;

#include "classdef.h"
#include "video.h"

enum Condition     {
    ISRESTORED, ISMINIMIZED, ISMAXIMIZED, ISCLOSING
};

void LogMessages (WINDOW, MESSAGE, PARAM, PARAM);
void MessageLog(WINDOW);
/* ------- window methods ----------- */
#define ICONHEIGHT 3
#define ICONWIDTH  10
#define WindowHeight(w)      ((w)->ht)
#define WindowWidth(w)       ((w)->wd)
#define BorderAdj(w)         (TestAttribute(w,HASBORDER)?1:0)
#define TopBorderAdj(w)      ((TestAttribute(w,HASTITLEBAR) &&   \
                              TestAttribute(w,HASMENUBAR)) ?  \
                              2 : (TestAttribute(w,HASTITLEBAR | \
                              HASMENUBAR | HASBORDER) ? 1 : 0))
#define ClientWidth(w)       (WindowWidth(w)-BorderAdj(w)*2)
#define ClientHeight(w)      (WindowHeight(w)-TopBorderAdj(w)-\
                              BorderAdj(w))
#define WindowRect(w)        ((w)->rc)
#define GetTop(w)            (RectTop(WindowRect(w)))
#define GetBottom(w)         (RectBottom(WindowRect(w)))
#define GetLeft(w)           (RectLeft(WindowRect(w)))
#define GetRight(w)          (RectRight(WindowRect(w)))
#define GetClientTop(w)      (GetTop(w)+TopBorderAdj(w))
#define GetClientBottom(w)   (GetBottom(w)-BorderAdj(w))
#define GetClientLeft(w)     (GetLeft(w)+BorderAdj(w))
#define GetClientRight(w)    (GetRight(w)-BorderAdj(w))
#define GetParent(w)         ((w)->parent)
#define GetTitle(w)          ((w)->title)
#define NextWindow(w)        ((w)->nextfocus)
#define PrevWindow(w)        ((w)->prevfocus)
#define NextWindowBuilt(w)   ((w)->nextbuilt)
#define PrevWindowBuilt(w)   ((w)->prevbuilt)
#define GetClass(w)          ((w)->class)
#define GetAttribute(w)      ((w)->attrib)
#define AddAttribute(w,a)    (GetAttribute(w) |= a)
#define ClearAttribute(w,a)  (GetAttribute(w) &= ~(a))
#define TestAttribute(w,a)   (GetAttribute(w) & (a))
#define isWndVisible(w)      (GetAttribute(w) & VISIBLE)
#define SetVisible(w)        (GetAttribute(w) |= VISIBLE)
#define ClearVisible(w)      (GetAttribute(w) &= ~VISIBLE)
#define gotoxy(w,x,y) cursor(w->rc.lf+(x)+1,w->rc.tp+(y)+1)
WINDOW CreateWindow(CLASS,char *,int,int,int,int,void*,WINDOW,
       int (*)(struct window *,enum messages,PARAM,PARAM),int);
void AddTitle(WINDOW, char *);
void InsertTitle(WINDOW, char *);
void DisplayTitle(WINDOW, RECT *);
void RepaintBorder(WINDOW, RECT *);
void ClearWindow(WINDOW, RECT *, int);
#ifdef INCLUDE_SYSTEM_MENUS
void clipline(WINDOW, int, char *);
#else
#define clipline(w,x,c) /**/
#endif
void writeline(WINDOW, char *, int, int, int);

void SetNextFocus(WINDOW);
void SetPrevFocus(WINDOW);
void RemoveFocusWindow(WINDOW);
void AppendFocusWindow(WINDOW);
void RemoveBuiltWindow(WINDOW);
void AppendBuiltWindow(WINDOW);
WINDOW SearchFocusNext(WINDOW, WINDOW);
WINDOW GetFirstChild(WINDOW);
WINDOW GetNextChild(WINDOW, WINDOW);
WINDOW GetLastChild(WINDOW);
WINDOW GetPrevChild(WINDOW, WINDOW);
WINDOW GetFirstFocusChild(WINDOW);
WINDOW GetNextFocusChild(WINDOW, WINDOW);

int isVisible(WINDOW);
int CharInView(WINDOW, int, int);
void PutWindowChar(WINDOW, int, int, int);
void GetVideoBuffer(WINDOW);
void RestoreVideoBuffer(WINDOW);
void CreatePath(char *, char *, int, int);
int LineLength(char *);
RECT AdjustRectangle(WINDOW, RECT);
#define DefaultWndProc(wnd,msg,p1,p2)    \
    (*classdefs[wnd->class].wndproc)(wnd,msg,p1,p2)
#define BaseWndProc(class,wnd,msg,p1,p2)    \
    (*classdefs[DerivedClass(class)].wndproc)(wnd,msg,p1,p2)
#define NULLWND ((WINDOW) 0)
struct LinkedList    {
    WINDOW FirstWindow;
    WINDOW LastWindow;
};
extern struct LinkedList Focus;
extern struct LinkedList Built;
extern WINDOW inFocus;
extern WINDOW CaptureMouse;
extern WINDOW CaptureKeyboard;
extern int foreground, background;
extern int WindowMoving;
extern int WindowSizing;
extern int HScrolling;
extern int VScrolling;
extern int TextMarking;
extern char *Clipboard;
/* --------- space between menubar labels --------- */
#define MSPACE 2
/* --------------- border characters ------------- */
#define FOCUS_NW      (unsigned char) '\xc9'
#define FOCUS_NE      (unsigned char) '\xbb'
#define FOCUS_SE      (unsigned char) '\xbc'
#define FOCUS_SW      (unsigned char) '\xc8'
#define FOCUS_SIDE    (unsigned char) '\xba'
#define FOCUS_LINE    (unsigned char) '\xcd'
#define NW            (unsigned char) '\xda'
#define NE            (unsigned char) '\xbf'
#define SE            (unsigned char) '\xd9'
#define SW            (unsigned char) '\xc0'
#define SIDE          (unsigned char) '\xb3'
#define LINE          (unsigned char) '\xc4'
#define LEDGE         (unsigned char) '\xc3'
#define REDGE         (unsigned char) '\xb4'
/* ------------- scroll bar characters ------------ */
#define UPSCROLLBOX    (unsigned char) '\x1e'
#define DOWNSCROLLBOX  (unsigned char) '\x1f'
#define LEFTSCROLLBOX  (unsigned char) '\x11'
#define RIGHTSCROLLBOX (unsigned char) '\x10'
#define SCROLLBARCHAR  (unsigned char) 176 
#define SCROLLBOXCHAR  (unsigned char) 178
#define CHECKMARK      (unsigned char) 251 /* menu toggle   */
/* ----------------- title bar characters ----------------- */
#define CONTROLBOXCHAR (unsigned char) '\xf0'
#define MAXPOINTER     24      /* maximize token            */
#define MINPOINTER     25      /* minimize token            */
#define RESTOREPOINTER 18      /* restore token             */
/* --------------- text control characters ---------------- */
#define APPLCHAR     (unsigned char) 176 /* fills application window */
#define SHORTCUTCHAR '~'    /* prefix: shortcut key display */
#define CHANGECOLOR  (unsigned char) 174 /* prefix to change colors  */
#define RESETCOLOR   (unsigned char) 175 /* reset colors to default  */
#define LISTSELECTOR   4    /* selected list box entry      */
/* --------- message prototypes ----------- */
void init_messages(void);
void PostMessage(WINDOW, MESSAGE, PARAM, PARAM);
int SendMessage(WINDOW, MESSAGE, PARAM, PARAM);
int dispatch_message(void);
int TestCriticalError(void);
/* ---- standard window message processing prototypes ----- */
int ApplicationProc(WINDOW, MESSAGE, PARAM, PARAM);
int NormalProc(WINDOW, MESSAGE, PARAM, PARAM);
int TextBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int ListBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int EditBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int MenuBarProc(WINDOW, MESSAGE, PARAM, PARAM);
int PopDownProc(WINDOW, MESSAGE, PARAM, PARAM);
int ButtonProc(WINDOW, MESSAGE, PARAM, PARAM);
int DialogProc(WINDOW, MESSAGE, PARAM, PARAM);
int SystemMenuProc(WINDOW, MESSAGE, PARAM, PARAM);
int HelpBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int MessageBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int ErrorBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int YesNoBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int StatusBarProc(WINDOW, MESSAGE, PARAM, PARAM);
/* ------------- normal box prototypes ------------- */
int isWindow(WINDOW);
WINDOW inWindow(int, int);
int WndForeground(WINDOW);
int WndBackground(WINDOW);
int FrameForeground(WINDOW);
int FrameBackground(WINDOW);
int SelectForeground(WINDOW);
int SelectBackground(WINDOW);
void SetStandardColor(WINDOW);
void SetReverseColor(WINDOW);
void SetClassColors(CLASS);
#define HitControlBox(wnd, p1, p2)     \
    (TestAttribute(wnd, HASTITLEBAR)   && \
     TestAttribute(wnd, CONTROLBOX) && \
     p1 == 2 && p2 == 0)
#define WndForeground(wnd) 		\
	(cfg.clr [GetClass(wnd)] [STD_COLOR] [FG])
#define WndBackground(wnd) 		\
	(cfg.clr [GetClass(wnd)] [STD_COLOR] [BG])
#define FrameForeground(wnd) 	\
	(cfg.clr [GetClass(wnd)] [FRAME_COLOR] [FG])
#define FrameBackground(wnd) 	\
	(cfg.clr [GetClass(wnd)] [FRAME_COLOR] [BG])
#define SelectForeground(wnd) 	\
	(cfg.clr [GetClass(wnd)] [SELECT_COLOR] [FG])
#define SelectBackground(wnd) 	\
	(cfg.clr [GetClass(wnd)] [SELECT_COLOR] [BG])
/* -------- text box prototypes ---------- */
#define TextLine(wnd, sel) \
      (wnd->text + *((wnd->TextPointers) + sel))
void WriteTextLine(WINDOW, RECT *, int, int);
void SetAnchor(WINDOW, int, int);
#define BlockMarked(wnd) (  wnd->BlkBegLine ||    \
                            wnd->BlkEndLine ||    \
                            wnd->BlkBegCol  ||    \
                            wnd->BlkEndCol)
#define ClearBlock(wnd) wnd->BlkBegLine = wnd->BlkEndLine =  \
                        wnd->BlkBegCol  = wnd->BlkEndCol = 0;
#define GetText(w)        ((w)->text)
void ClearTextPointers(WINDOW);
void BuildTextPointers(WINDOW);
/* --------- menu prototypes ---------- */
int CopyCommand(unsigned char *, unsigned char *, int, int);
void PrepFileMenu(void *, struct Menu *);
void PrepEditMenu(void *, struct Menu *);
void PrepSearchMenu(void *, struct Menu *);
void PrepWindowMenu(void *, struct Menu *);
void BuildSystemMenu(WINDOW);
int isActive(MENU *, int);
void ActivateCommand(MENU *,int);
void DeactivateCommand(MENU *,int);
int GetCommandToggle(MENU *,int);
void SetCommandToggle(MENU *,int);
void ClearCommandToggle(MENU *,int);
void InvertCommandToggle(MENU *,int);
int BarSelection(int);
/* ------------- list box prototypes -------------- */
int ItemSelected(WINDOW, int);
/* ------------- edit box prototypes ----------- */
#ifdef INCLUDE_MULTILINE
#define isMultiLine(wnd)     TestAttribute(wnd, MULTILINE)
#else
#define isMultiLine(wnd)     FALSE
#endif
/* --------- message box prototypes -------- */
int GenericMessage(char *, char *, int,
	int (*)(struct window *, enum messages, PARAM, PARAM),
	char *, char *);
#define TestErrorMessage(msg)	\
	GenericMessage("Error", msg, 2, ErrorBoxProc,   Ok, Cancel)
#define ErrorMessage(msg) \
	GenericMessage("Error", msg, 1, ErrorBoxProc,   Ok, NULL)
#define MessageBox(ttl, msg) \
	GenericMessage(ttl,     msg, 1, MessageBoxProc, Ok, NULL)
#define YesNoBox(msg)	\
	GenericMessage(NULL,    msg, 2, YesNoBoxProc,  Yes, No)
WINDOW MomentaryMessage(char *);
int MsgHeight(char *);
int MsgWidth(char *);

#ifdef INCLUDE_DIALOG_BOXES
/* ------------- dialog box prototypes -------------- */
int DialogBox(WINDOW, DBOX *, int,
       int (*)(struct window *, enum messages, PARAM, PARAM));
int DlgOpenFile(char *, char *);
int DlgSaveAs(char *);
void GetDlgListText(WINDOW, char *, enum commands);
int DlgDirList(WINDOW, char *, enum commands,
                            enum commands, unsigned);
int RadioButtonSetting(DBOX *, enum commands);
void PushRadioButton(DBOX *, enum commands);
void PutItemText(WINDOW, enum commands, char *);
void GetItemText(WINDOW, enum commands, char *, int);
char *GetEditBoxText(DBOX *, enum commands);
void SetEditBoxText(DBOX *, enum commands, char *);
void SetCheckBox(DBOX *, enum commands);
void ClearCheckBox(DBOX *, enum commands);
int CheckBoxSetting(DBOX *, enum commands);
WINDOW ControlWindow(DBOX *, enum commands);
CTLWINDOW *ControlBox(DBOX *, WINDOW);
void EnableButton(DBOX *, enum commands);
void DisableButton(DBOX *, enum commands);
#endif

/* ------------- help box prototypes ------------- */
void LoadHelpFile(void);
void UnLoadHelpFile(void);
int DisplayHelp(WINDOW, char *);

#ifdef INCLUDE_HELP
extern char *ClassNames[];
#endif

#endif

