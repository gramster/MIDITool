/* -------------- menus.c ------------- */

#include <stdio.h>
#include "dflat.h"

/* --------------------- the main menu --------------------- */
DEFMENU(MainMenu)
    /* --------------- the File popdown menu ----------------*/
    POPDOWN( "~File",  PrepFileMenu, "Read/write/print files. Go to DOS" )
        SELECTION( "~New",        ID_NEW,          0, 0 )
#ifdef INCLUDE_DIALOG_BOXES
        SELECTION( "~Open...",    ID_OPEN,         0, 0 )
        SEPARATOR
#endif
        SELECTION( "Sa~ve",       ID_SAVE,     ALT_V, INACTIVE)
#ifdef INCLUDE_DIALOG_BOXES
        SELECTION( "Save ~as...", ID_SAVEAS,       0, INACTIVE)
#endif
        SEPARATOR
        SELECTION( "~Print",      ID_PRINT,        0, INACTIVE)
        SEPARATOR
        SELECTION( "~DOS",        ID_DOS,          0, 0 )
        SELECTION( "E~xit",       ID_EXIT,     ALT_X, 0 )
    ENDPOPDOWN

    /* --------------- the Edit popdown menu ----------------*/
    POPDOWN( "~Edit", PrepEditMenu, "Clipboard, delete text, paragraph" )
        SELECTION( "~Undo",      ID_UNDO,  ALT_BS,    INACTIVE)
#ifdef INCLUDE_CLIPBOARD
        SEPARATOR
        SELECTION( "Cu~t",       ID_CUT,   SHIFT_DEL, INACTIVE)
        SELECTION( "~Copy",      ID_COPY,  CTRL_INS,  INACTIVE)
        SELECTION( "~Paste",     ID_PASTE, SHIFT_INS, INACTIVE)
        SEPARATOR
        SELECTION( "Cl~ear",     ID_CLEAR, 0,         INACTIVE)
#endif
        SELECTION( "~Delete",    ID_DELETETEXT, DEL,  INACTIVE)
        SEPARATOR
        SELECTION( "Pa~ragraph", ID_PARAGRAPH,  ALT_P,INACTIVE)
    ENDPOPDOWN

    /* --------------- the Search popdown menu ----------------*/
    POPDOWN( "~Search", PrepSearchMenu, "Search and replace" )
        SELECTION( "~Search...", ID_SEARCH,      0,    INACTIVE)
        SELECTION( "~Replace...",ID_REPLACE,     0,    INACTIVE)
        SELECTION( "~Next",      ID_SEARCHNEXT,  F3,   INACTIVE)
    ENDPOPDOWN

    /* ------------- the Options popdown menu ---------------*/
    POPDOWN( "~Options", NULL, "Editor and display options" )
        SELECTION( "~Insert",       ID_INSERT,     INS, TOGGLE)
        SELECTION( "~Word wrap",    ID_WRAP,        0,  TOGGLE)
#ifdef INCLUDE_DIALOG_BOXES
        SELECTION( "~Tabs...",      ID_TABS,        0,      0 )
        SEPARATOR
        SELECTION( "~Display...",   ID_DISPLAY,     0,      0 )
#ifdef INCLUDE_LOGGING
        SEPARATOR
        SELECTION( "~Log Messages       ",ID_LOG,   0,      0 )
#endif
#endif
        SEPARATOR
        SELECTION( "~Save Options", ID_SAVEOPTIONS, 0,      0 )
    ENDPOPDOWN

#ifdef INCLUDE_MULTIDOCS

    /* --------------- the Window popdown menu --------------*/
    POPDOWN( "~Window", PrepWindowMenu, "Select/close document windows" )
        SELECTION(  NULL,  ID_CLOSEALL, 0, 0)
        SEPARATOR
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  "~More Windows...", ID_WINDOW, 0, 0)
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
    ENDPOPDOWN
#endif

#ifdef INCLUDE_HELP
    /* --------------- the Help popdown menu ----------------*/
    POPDOWN( "~Help", NULL, "Get help" )
        SELECTION(  "~Help for help...",  ID_HELPHELP,  0, 0 )
        SELECTION(  "~Extended help...",  ID_EXTHELP,   0, 0 )
        SELECTION(  "~Keys help...",      ID_KEYSHELP,  0, 0 )
        SELECTION(  "Help ~index...",     ID_HELPINDEX, 0, 0 )
        SEPARATOR
        SELECTION(  "~About...",          ID_ABOUT,     0, 0 )
#ifdef INCLUDE_RELOADHELP
        SEPARATOR
        SELECTION(  "~Reload Help Database",ID_LOADHELP,0, 0 )
#endif
    ENDPOPDOWN
#endif

ENDMENU

#ifdef INCLUDE_SYSTEM_MENUS
/* ------------- the System Menu --------------------- */
DEFMENU(SystemMenu)
    POPDOWN("System Menu", NULL, NULL)
        SELECTION("~Restore",  ID_SYSRESTORE,  0,         0 )
        SELECTION("~Move",     ID_SYSMOVE,     0,         0 )
        SELECTION("~Size",     ID_SYSSIZE,     0,         0 )
        SELECTION("Mi~nimize", ID_SYSMINIMIZE, 0,         0 )
        SELECTION("Ma~ximize", ID_SYSMAXIMIZE, 0,         0 )
        SEPARATOR
        SELECTION("~Close",    ID_SYSCLOSE,    CTRL_F4,   0 )
    ENDPOPDOWN
ENDMENU

#endif
