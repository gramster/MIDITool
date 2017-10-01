/* ----------- dialogs.c --------------- */

#include "dflat.h"

#ifdef INCLUDE_DIALOG_BOXES

/* -------------- the File Open dialog box --------------- */
DIALOGBOX( FileOpen )
    DB_TITLE(        "Open File",    -1,-1,19,48)
    CONTROL(TEXT,    "~Filename",     2, 1, 1, 8, ID_FILENAME)
    CONTROL(EDITBOX, NULL,           13, 1, 1,29, ID_FILENAME)
    CONTROL(TEXT,    "Directory:",    2, 3, 1,10, 0 )
    CONTROL(TEXT,    NULL,           13, 3, 1,28, ID_PATH ) 
    CONTROL(TEXT,    "F~iles",        2, 5, 1, 5, ID_FILES )
    CONTROL(LISTBOX, NULL,            2, 6,11,16, ID_FILES )
    CONTROL(TEXT,    "~Directories", 19, 5, 1,11, ID_DRIVE )
    CONTROL(LISTBOX, NULL,           19, 6,11,16, ID_DRIVE ) 
    CONTROL(BUTTON,  "   ~OK   ",    36, 7, 1, 8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ",    36,10, 1, 8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ",    36,13, 1, 8, ID_HELP)
ENDDB


/* -------------- the Save As dialog box --------------- */
DIALOGBOX( SaveAs )
    DB_TITLE(        "Save As",    -1,-1,19,48)
    CONTROL(TEXT,    "~Filename",   2, 1, 1, 8, ID_FILENAME)
    CONTROL(EDITBOX, NULL,         13, 1, 1,29, ID_FILENAME)
    CONTROL(TEXT,    "Directory:",  2, 3, 1,10, 0 )
    CONTROL(TEXT,    NULL,         13, 3, 1,28, ID_PATH ) 
    CONTROL(TEXT,    "~Directories",2, 5, 1,11, ID_DRIVE )
    CONTROL(LISTBOX, NULL,          2, 6,11,16, ID_DRIVE ) 
    CONTROL(BUTTON,  "   ~OK   ",  36, 7, 1, 8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ",  36,10, 1, 8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ",  36,13, 1, 8, ID_HELP)
ENDDB

/* -------------- the Search Text dialog box --------------- */
DIALOGBOX( SearchText )
    DB_TITLE(        "Search Text",    -1,-1,9,48)
    CONTROL(TEXT,    "~Search for:",          2, 1, 1, 11, ID_SEARCHFOR)
    CONTROL(EDITBOX, NULL,                   14, 1, 1, 29, ID_SEARCHFOR)
    CONTROL(TEXT, "~Match upper/lower case:", 2, 3, 1, 23, ID_MATCHCASE)
	CONTROL(CHECKBOX,  NULL,                 26, 3, 1,  3, ID_MATCHCASE)
    CONTROL(BUTTON, "   ~OK   ",              7, 5, 1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ",             19, 5, 1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",             31, 5, 1,  8, ID_HELP)
ENDDB

/* -------------- the Replace Text dialog box --------------- */
DIALOGBOX( ReplaceText )
    DB_TITLE(        "Replace Text",    -1,-1,12,50)
    CONTROL(TEXT,    "~Search for:",          2, 1, 1, 11, ID_SEARCHFOR)
    CONTROL(EDITBOX, NULL,                   16, 1, 1, 29, ID_SEARCHFOR)
    CONTROL(TEXT,    "~Replace with:",        2, 3, 1, 13, ID_REPLACEWITH)
    CONTROL(EDITBOX, NULL,                   16, 3, 1, 29, ID_REPLACEWITH)
    CONTROL(TEXT, "~Match upper/lower case:", 2, 5, 1, 23, ID_MATCHCASE)
	CONTROL(CHECKBOX,  NULL,                 26, 5, 1,  3, ID_MATCHCASE)
    CONTROL(TEXT, "Replace ~Every Match:",    2, 6, 1, 23, ID_REPLACEALL)
	CONTROL(CHECKBOX,  NULL,                 26, 6, 1,  3, ID_REPLACEALL)
    CONTROL(BUTTON, "   ~OK   ",              7, 8, 1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ",             20, 8, 1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",             33, 8, 1,  8, ID_HELP)
ENDDB

/* -------------- generic message dialog box --------------- */
DIALOGBOX( MsgBox )
    DB_TITLE(       NULL,  -1,-1, 0, 0)
    CONTROL(TEXT,   NULL,   1, 1, 0, 0, 0)
    CONTROL(BUTTON, NULL,   0, 0, 1, 8, ID_OK)
    CONTROL(0,      NULL,   0, 0, 1, 8, ID_CANCEL)
ENDDB

#ifdef INCLUDE_MULTIDOCS
#define offset 4
#else
#define offset 0
#endif

/* ------------ VGA Display dialog box -------------- */
DIALOGBOX( DisplayVGA )
    DB_TITLE(     "Display", -1, -1, 14+offset, 34)
#ifdef INCLUDE_MULTIDOCS
    CONTROL(CHECKBOX,    NULL,     9, 1, 1, 3, ID_TITLE)
    CONTROL(TEXT,     "~Title",   15, 1, 1, 5, ID_TITLE)
    CONTROL(CHECKBOX,    NULL,     9, 2, 1, 3, ID_BORDER)
    CONTROL(TEXT,     "~Border",  15, 2, 1, 6, ID_BORDER)
    CONTROL(CHECKBOX,    NULL,     9, 3, 1, 3, ID_STATUSBAR)
    CONTROL(TEXT,   "~Status bar",15, 3, 1,10, ID_STATUSBAR)
    CONTROL(CHECKBOX,    NULL,     9, 4, 1, 3, ID_TEXTURE)
    CONTROL(TEXT,     "Te~xture", 15, 4, 1, 7, ID_TEXTURE)
#endif
    CONTROL(RADIOBUTTON, NULL,     9, 2+offset,1,3,ID_COLOR)
    CONTROL(TEXT,     "Co~lor",   15, 2+offset,1,5,ID_COLOR)
    CONTROL(RADIOBUTTON, NULL,     9, 3+offset,1,3,ID_MONO)
    CONTROL(TEXT,     "~Mono",    15, 3+offset,1,4,ID_MONO)
    CONTROL(RADIOBUTTON, NULL,     9, 4+offset,1,3,ID_REVERSE)
    CONTROL(TEXT,     "~Reverse", 15, 4+offset,1,7,ID_REVERSE)
    CONTROL(RADIOBUTTON, NULL,     9, 6+offset,1,3,ID_25LINES)
    CONTROL(TEXT,     "~25 Lines",15, 6+offset,1,8,ID_25LINES)
    CONTROL(RADIOBUTTON, NULL,     9, 7+offset,1,3,ID_43LINES)
    CONTROL(TEXT,     "~43 Lines",15, 7+offset,1,8,ID_43LINES)
    CONTROL(RADIOBUTTON, NULL,     9, 8+offset,1,3,ID_50LINES)
    CONTROL(TEXT,     "~50 Lines",15, 8+offset,1,8,ID_50LINES)
    CONTROL(BUTTON, "   ~OK   ",   2,10+offset,1,8,ID_OK)
    CONTROL(BUTTON, " ~Cancel ",  12,10+offset,1,8,ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",  22,10+offset,1,8,ID_HELP)
ENDDB


/* ------------ EGA Display dialog box -------------- */
DIALOGBOX( DisplayEGA )
    DB_TITLE(     "Display", -1, -1, 13+offset, 34)
#ifdef INCLUDE_MULTIDOCS
    CONTROL(CHECKBOX,    NULL,     9, 1, 1, 3, ID_TITLE)
    CONTROL(TEXT,     "~Title",   15, 1, 1, 5, ID_TITLE)
    CONTROL(CHECKBOX,    NULL,     9, 2, 1, 3, ID_BORDER)
    CONTROL(TEXT,     "~Border",  15, 2, 1, 6, ID_BORDER)
    CONTROL(CHECKBOX,    NULL,     9, 3, 1, 3, ID_STATUSBAR)
    CONTROL(TEXT,   "~Status bar",15, 3, 1,10, ID_STATUSBAR)
    CONTROL(CHECKBOX,    NULL,     9, 4, 1, 3, ID_TEXTURE)
    CONTROL(TEXT,     "Te~xture", 15, 4, 1, 7, ID_TEXTURE)
#endif
    CONTROL(RADIOBUTTON, NULL,     9, 2+offset,1,3,ID_COLOR)
    CONTROL(TEXT,     "Co~lor",   15, 2+offset,1,5,ID_COLOR)
    CONTROL(RADIOBUTTON, NULL,     9, 3+offset,1,3,ID_MONO)
    CONTROL(TEXT,     "~Mono",    15, 3+offset,1,4,ID_MONO)
    CONTROL(RADIOBUTTON, NULL,     9, 4+offset,1,3,ID_REVERSE)
    CONTROL(TEXT,     "~Reverse", 15, 4+offset,1,7,ID_REVERSE)
    CONTROL(RADIOBUTTON, NULL,     9, 6+offset,1,3,ID_25LINES)
    CONTROL(TEXT,     "~25 Lines",15, 6+offset,1,8,ID_25LINES)
    CONTROL(RADIOBUTTON, NULL,     9, 7+offset,1,3,ID_43LINES)
    CONTROL(TEXT,     "~43 Lines",15, 7+offset,1,8,ID_43LINES)
    CONTROL(BUTTON, "   ~OK   ",   2, 9+offset,1,8,ID_OK)
    CONTROL(BUTTON, " ~Cancel ",  12, 9+offset,1,8,ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",  22, 9+offset,1,8,ID_HELP)
ENDDB

/* ------------ CGA/MDA Display dialog box -------------- */
DIALOGBOX( DisplayCGA )
    DB_TITLE(     "Display", -1, -1, 10+offset, 34)
#ifdef INCLUDE_MULTIDOCS
    CONTROL(CHECKBOX,    NULL,     9, 1, 1, 3, ID_TITLE)
    CONTROL(TEXT,     "~Title",   15, 1, 1, 5, ID_TITLE)
    CONTROL(CHECKBOX,    NULL,     9, 2, 1, 3, ID_BORDER)
    CONTROL(TEXT,     "~Border",  15, 2, 1, 6, ID_BORDER)
    CONTROL(CHECKBOX,    NULL,     9, 3, 1, 3, ID_STATUSBAR)
    CONTROL(TEXT,   "~Status bar",15, 3, 1,10, ID_STATUSBAR)
    CONTROL(CHECKBOX,    NULL,     9, 4, 1, 3, ID_TEXTURE)
    CONTROL(TEXT,     "Te~xture", 15, 4, 1, 7, ID_TEXTURE)
#endif
    CONTROL(RADIOBUTTON, NULL,     9, 2+offset,1,3,ID_COLOR)
    CONTROL(TEXT,     "Co~lor",   15, 2+offset,1,5,ID_COLOR)
    CONTROL(RADIOBUTTON, NULL,     9, 3+offset,1,3,ID_MONO)
    CONTROL(TEXT,     "~Mono",    15, 3+offset,1,4,ID_MONO)
    CONTROL(RADIOBUTTON, NULL,     9, 4+offset,1,3,ID_REVERSE)
    CONTROL(TEXT,     "~Reverse", 15, 4+offset,1,7,ID_REVERSE)
    CONTROL(BUTTON, "   ~OK   ",   2, 6+offset,1,8,ID_OK)
    CONTROL(BUTTON, " ~Cancel ",  12, 6+offset,1,8,ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",  22, 6+offset,1,8,ID_HELP)
ENDDB

#define TS2 "~2  컨컨컨컨컨컨컨컨컨컨"
#define TS4 "~4  컴컨컴컨컴컨컴컨컴컨"
#define TS6 "~6  컴컴컨컴컴컨컴컴컨컴"
#define TS8 "~8  컴컴컴컨컴컴컴컨컴컴"

/* ------------ Tab Stops dialog box -------------- */
DIALOGBOX( TabStops )
    DB_TITLE(      "Editor Tab Stops", -1,-1, 10, 35)
    CONTROL(RADIOBUTTON,  NULL,    2, 1, 1,  3, ID_TAB2)
    CONTROL(TEXT,         TS2,     7, 1, 1, 23, ID_TAB2)
    CONTROL(RADIOBUTTON,  NULL,    2, 2, 1, 11, ID_TAB4)
    CONTROL(TEXT,         TS4,     7, 2, 1, 23, ID_TAB4)
    CONTROL(RADIOBUTTON,  NULL,    2, 3, 1, 11, ID_TAB6)
    CONTROL(TEXT,         TS6,     7, 3, 1, 23, ID_TAB6)
    CONTROL(RADIOBUTTON,  NULL,    2, 4, 1, 11, ID_TAB8)
    CONTROL(TEXT,         TS8,     7, 4, 1, 23, ID_TAB8)
    CONTROL(BUTTON,  "   ~OK   ",  1, 6, 1,  8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ", 12, 6, 1,  8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ", 23, 6, 1,  8, ID_HELP)
ENDDB

/* ------------ Windows dialog box -------------- */
#ifdef INCLUDE_MULTIDOCS
DIALOGBOX( Windows )
    DB_TITLE(     "Windows", -1, -1, 19, 24)
    CONTROL(LISTBOX, NULL,        1,  1,11, 20, ID_WINDOWLIST)
    CONTROL(BUTTON,  "   ~OK   ",  2, 13, 1, 8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ", 12, 13, 1, 8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ",  7, 15, 1, 8, ID_HELP)
ENDDB
#endif

#ifdef INCLUDE_LOGGING
/* ------------ Message Log dialog box -------------- */
DIALOGBOX( Log )
    DB_TITLE(    "D-Flat Message Log", -1, -1, 18, 41)
    CONTROL(TEXT,  "~Messages",   10,   1,  1,  8, ID_LOGLIST)
    CONTROL(LISTBOX,    NULL,     1,    2, 14, 26, ID_LOGLIST)
    CONTROL(TEXT,    "~Logging:", 29,   4,  1, 10, ID_LOGGING)
    CONTROL(CHECKBOX,    NULL,    31,   5,  1,  3, ID_LOGGING)
    CONTROL(BUTTON,  "   ~OK   ", 29,   7,  1,  8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ", 29,  10,  1,  8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ", 29,  13, 1,   8, ID_HELP)
ENDDB
#endif

#ifdef INCLUDE_HELP
/* ------------ the Help window dialog box -------------- */
DIALOGBOX( HelpBox )
    DB_TITLE(         NULL,       -1, -1, 0, 45)
    CONTROL(TEXTBOX, NULL,         1,  1, 0, 40, ID_HELPTEXT)
    CONTROL(BUTTON,  "  ~Close ",  0,  0, 1,  8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Back  ", 10,  0, 1,  8, ID_BACK)
    CONTROL(BUTTON,  "<< ~Prev ", 20,  0, 1,  8, ID_PREV)
    CONTROL(BUTTON,  " ~Next >>", 30,  0, 1,  8, ID_NEXT)
ENDDB
#endif
#endif
