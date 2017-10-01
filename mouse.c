/* ------------- mouse.c ------------- */

#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"

static union REGS regs;

static void near mouse(int m1,int m2,int m3,int m4)
{
    regs.x.dx = m4;
    regs.x.cx = m3;
    regs.x.bx = m2;
    regs.x.ax = m1;
    int86(MOUSE, &regs, &regs);
}

/* ---------- reset the mouse ---------- */
void resetmouse(void)
{
    mouse(0,0,0,0);
}

/* ----- test to see if the mouse driver is installed ----- */
int mouse_installed(void)
{
    unsigned char far *ms;
    ms = MK_FP(peek(0, MOUSE*4+2), peek(0, MOUSE*4));
    return (ms != NULL && *ms != 0xcf);
}

/* ------ return true if mouse buttons are pressed ------- */
int mousebuttons(void)
{
    if (mouse_installed())
        mouse(3,0,0,0);
    return regs.x.bx & 3;
}

/* ---------- return mouse coordinates ---------- */
void get_mouseposition(int *x, int *y)
{
    if (mouse_installed())    {
        mouse(3,0,0,0);
        *x = regs.x.cx/8;
        *y = regs.x.dx/8;
    }
}

/* -------- position the mouse cursor -------- */
void set_mouseposition(int x, int y)
{
    if(mouse_installed())
        mouse(4,0,x*8,y*8);
}

/* --------- display the mouse cursor -------- */
void show_mousecursor(void)
{
    if(mouse_installed())
        mouse(1,0,0,0);
}

/* --------- hide the mouse cursor ------- */
void hide_mousecursor(void)
{
    if(mouse_installed())
        mouse(2,0,0,0);
}

/* --- return true if a mouse button has been released --- */
int button_releases(void)
{
    if(mouse_installed())
        mouse(6,0,0,0);
    return regs.x.bx;
}
