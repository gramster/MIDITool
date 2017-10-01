/* ------------- config.c ------------- */

#include <conio.h>
#include <string.h>
#include "dflat.h"

/* ----- default colors for color video system ----- */
unsigned char color[CLASSCOUNT] [4] [2] = {
    /* ------------ NORMAL ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ---------- APPLICATION --------- */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {LIGHTGRAY, BLUE},  /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */

    /* ------------ TEXTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LISTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- MENUBAR ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, CYAN},      /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, RED}},   /* HILITE_COLOR
                          Inactive, Shortcut (both FG) */

    /* ---------- POPDOWNMENU --------- */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {DARKGRAY, RED}},   /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

#ifdef INCLUDE_DIALOG_BOXES
    /* ------------ BUTTON ------------ */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {WHITE, CYAN},      /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {DARKGRAY, RED}},   /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

    /* ------------- DIALOG ----------- */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {LIGHTGRAY, BLUE},  /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */
#endif

    /* ----------- ERRORBOX ----------- */
   {{YELLOW, RED},      /* STD_COLOR    */
    {YELLOW, RED},      /* SELECT_COLOR */
    {YELLOW, RED},      /* FRAME_COLOR  */
    {YELLOW, RED}},     /* HILITE_COLOR */

    /* ----------- MESSAGEBOX --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

#ifdef INCLUDE_HELP
    /* ----------- HELPBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLUE},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */
#endif

#ifdef INCLUDE_STATUSBAR
    /* ---------- STATUSBAR ------------- */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {BLACK, CYAN},      /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {BLACK, CYAN}},     /* HILITE_COLOR */
#endif

    /* ---------- TITLEBAR ------------ */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {BLACK, CYAN},      /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {WHITE, CYAN}},     /* HILITE_COLOR */

    /* ------------ DUMMY ------------- */
   {{GREEN, LIGHTGRAY}, /* STD_COLOR    */
    {GREEN, LIGHTGRAY}, /* SELECT_COLOR */
    {GREEN, LIGHTGRAY}, /* FRAME_COLOR  */
    {GREEN, LIGHTGRAY}} /* HILITE_COLOR */
};

/* ----- default colors for mono video system ----- */
unsigned char bw[CLASSCOUNT] [4] [2] = {
    /* ------------ NORMAL ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ---------- APPLICATION --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ------------ TEXTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LISTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITBOX ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- MENUBAR ------------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive, Shortcut (both FG) */

    /* ---------- POPDOWNMENU --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

#ifdef INCLUDE_DIALOG_BOXES
    /* ------------ BUTTON ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {WHITE, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

    /* ------------- DIALOG ----------- */
   {{LIGHTGRAY, BLACK},  /* STD_COLOR    */
    {LIGHTGRAY, BLACK},  /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}}, /* HILITE_COLOR */
#endif

    /* ----------- ERRORBOX ----------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ----------- MESSAGEBOX --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

#ifdef INCLUDE_HELP
    /* ----------- HELPBOX ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {WHITE, BLACK},     /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */
#endif

#ifdef INCLUDE_STATUSBAR
    /* ---------- STATUSBAR ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */
#endif

    /* ---------- TITLEBAR ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ DUMMY ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}} /* HILITE_COLOR */
};
/* ----- default colors for reverse mono video ----- */
unsigned char reverse[CLASSCOUNT] [4] [2] = {
    /* ------------ NORMAL ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- APPLICATION --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ TEXTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LISTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- MENUBAR ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive, Shortcut (both FG) */

    /* ---------- POPDOWNMENU --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

#ifdef INCLUDE_DIALOG_BOXES
    /* ------------ BUTTON ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {WHITE, BLACK},     /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

    /* ------------- DIALOG ----------- */
   {{BLACK, LIGHTGRAY},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}}, /* HILITE_COLOR */
#endif

    /* ----------- ERRORBOX ----------- */
   {{BLACK, LIGHTGRAY},      /* STD_COLOR    */
    {BLACK, LIGHTGRAY},      /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},      /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},     /* HILITE_COLOR */

    /* ----------- MESSAGEBOX --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

#ifdef INCLUDE_HELP
    /* ----------- HELPBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */
#endif

#ifdef INCLUDE_STATUSBAR
    /* ---------- STATUSBAR ------------- */
   {{LIGHTGRAY, BLACK},      /* STD_COLOR    */
    {LIGHTGRAY, BLACK},      /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},      /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},     /* HILITE_COLOR */
#endif

    /* ---------- TITLEBAR ------------ */
   {{LIGHTGRAY, BLACK},      /* STD_COLOR    */
    {LIGHTGRAY, BLACK},      /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},      /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},     /* HILITE_COLOR */

    /* ------------ DUMMY ------------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}} /* HILITE_COLOR */
};

#define SIGNATURE DFLAT_APPLICATION " " VERSION

/* ------ default configuration values ------- */
CONFIG cfg = {
    SIGNATURE,
    0,               /* Color                       */
    TRUE,            /* Editor Insert Mode          */
    4,               /* Editor tab stops            */
    TRUE,            /* Editor word wrap            */
    TRUE,            /* Application Border          */
    TRUE,            /* Application Title           */
    TRUE,            /* Status Bar                  */
    TRUE,            /* Textured application window */
    25               /* Number of screen lines      */
};

/* ------ load a configuration file from disk ------- */
int LoadConfig(void)
{
    FILE *fp = fopen(DFLAT_APPLICATION ".cfg", "rb");
    if (fp != NULL)    {
        fread(cfg.version, sizeof cfg.version+1, 1, fp);
        if (strcmp(cfg.version, SIGNATURE) == 0)    {
            fseek(fp, 0L, SEEK_SET);
            fread(&cfg, sizeof(CONFIG), 1, fp);
        }
        else
            strcpy(cfg.version, SIGNATURE);
        fclose(fp);
    }
    return fp != NULL;
}

/* ------ save a configuration file to disk ------- */
void SaveConfig(void)
{
    FILE *fp = fopen(DFLAT_APPLICATION ".cfg", "wb");
    if (fp != NULL)    {
        cfg.InsertMode = GetCommandToggle(MainMenu, ID_INSERT);
        cfg.WordWrap = GetCommandToggle(MainMenu, ID_WRAP);
        fwrite(&cfg, sizeof(CONFIG), 1, fp);
        fclose(fp);
    }
}
