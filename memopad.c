/* --------------- memopad.c ----------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#ifndef MSC
#include <dir.h>
#endif
#include "dflat.h"

static char Untitled[] = "Untitled";
static int wndpos = 0;

static int MemoPadProc(WINDOW, MESSAGE, PARAM, PARAM);
static void NewFile(WINDOW);
static void SelectFile(WINDOW);
static void PadWindow(WINDOW, char *);
static void OpenPadWindow(WINDOW, char *);
static void LoadFile(WINDOW, int);
static void PrintPad(WINDOW);
static void SaveFile(WINDOW, int);
static int EditorProc(WINDOW, MESSAGE, PARAM, PARAM);
static char *NameComponent(char *);

char **Argv;

void main(int argc, char *argv[])
{
	WINDOW wnd;
	init_messages();
	Argv = argv;
	wnd = CreateWindow(APPLICATION,
						"D-Flat MemoPad " VERSION,
						0, 0, -1, -1,
						MainMenu,
						NULL,
						MemoPadProc,
						MOVEABLE  |
						SIZEABLE  |
						HASBORDER |
						HASSTATUSBAR
						);

	SendMessage(wnd, SETFOCUS, TRUE, 0);
	while (argc > 1)	{
		PadWindow(wnd, argv[1]);
		--argc;
		argv++;
	}
	while (dispatch_message())
		;
}

static void PadWindow(WINDOW wnd, char *FileName)
{
	int ax, criterr = 1;
	struct ffblk ff;
	char path[64];
	char *cp;

	CreatePath(path, FileName, FALSE, FALSE);
	cp = path+strlen(path);
	CreatePath(path, FileName, TRUE, FALSE);
	while (criterr == 1)	{
		ax = findfirst(path, &ff, 0);
		criterr = TestCriticalError();
	}
	while (ax == 0 && !criterr)	{
		strcpy(cp, ff.ff_name);
		OpenPadWindow(wnd, path);
		ax = findnext(&ff);
	}
}

static int MemoPadProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case COMMAND:
			switch ((int)p1)	{
                case ID_NEW:
                    NewFile(wnd);
					return TRUE;
                case ID_OPEN:
                    SelectFile(wnd);
					return TRUE;
                case ID_SAVE:
                    SaveFile(inFocus, FALSE);
					return TRUE;
                case ID_SAVEAS:
                    SaveFile(inFocus, TRUE);
					return TRUE;
                case ID_PRINT:
                    PrintPad(inFocus);
					return TRUE;
				case ID_ABOUT:
					MessageBox(
 						"About D-Flat and the MemoPad",
						"   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n"
						"   ³    ÜÜÜ   ÜÜÜ     Ü    ³\n"
						"   ³    Û  Û  Û  Û    Û    ³\n"
						"   ³    Û  Û  Û  Û    Û    ³\n"
						"   ³    Û  Û  Û  Û Û  Û    ³\n"
						"   ³    ßßß   ßßß   ßß     ³\n"
						"   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n"
						"D-Flat implements the SAA/CUA\n"
						"interface in a public domain\n"
						"C language library originally\n"
						"published in Dr. Dobb's Journal\n"
						"    ------------------------ \n"
						"MemoPad is a multiple document\n"
						"editor that demonstrates D-Flat");
					MessageBox(
						"D-Flat Testers and Friends",
						"Jeff Ratcliff, David Peoples, Kevin Slater,\n"
						"Naor Toledo Pinto, Jeff Hahn, Jim Drash,\n"
						"Art Stricek, John Ebert, George Dinwiddie,\n"
						"Damaian Thorne, Wes Peterson, Thomas Ewald,\n"
						"Mitch Miller, Ray Waters, Jim Drash, Eric\n"
						"Silver, Russ Nelson, Elliott Jackson, Warren\n"
						"Master, H.J. Davey, Jim Kyle, Jim Morris,\n"
						"Andrew Terry, Michel Berube, Bruce Edmondson,\n"
						"Peter Baenziger, Phil Roberge, Willie Hutton,\n"
						"Randy Bradley, Tim Gentry, Lee Humphrey,\n"
						"Larry Troxler, Robert DiFalco, Carl Huff,\n"
						"Vince Rice, Michael Kaufman");
					return TRUE;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

/*
 *  The New command. Open an empty editor window
 */
static void NewFile(WINDOW wnd)
{
    OpenPadWindow(wnd, Untitled);
}

/*
 *  The Open... command. Select a file 
 */
static void SelectFile(WINDOW wnd)
{
    char FileName[64];
    if (DlgOpenFile("*.PAD", FileName))	{
		/* --- see if the document is already in a window --- */
		WINDOW wnd1 = GetFirstChild(wnd);
		while (wnd1 != NULLWND)	{
			if (stricmp(FileName, wnd1->extension) == 0)	{
				SendMessage(wnd1, SETFOCUS, TRUE, 0);
				SendMessage(wnd1, RESTORE, 0, 0);
				return;
			}
			wnd1 = GetNextChild(wnd, wnd1);
		}
        OpenPadWindow(wnd, FileName);
	}
}

/*
 *  open a document window and load a file
 */
static void OpenPadWindow(WINDOW wnd, char *FileName)
{
    static WINDOW wnd1 = NULLWND;
    struct stat sb;
	char *Fname = FileName;
	char *ermsg;
    if (strcmp(FileName, Untitled))	{
		if (stat(FileName, &sb))    {
			if ((ermsg = malloc(strlen(FileName)+20)) != NULL)	{
				strcpy(ermsg, "No such file as\n");
				strcat(ermsg, FileName);
		        ErrorMessage(ermsg);
				free(ermsg);
			}
    	    return;
	    }
		Fname = NameComponent(FileName);
	}
	wndpos += 2;
	if (wndpos == 20)
		wndpos = 2;
	wnd1 = CreateWindow(EDITBOX,
				Fname,
				(wndpos-1)*2, wndpos, 10, 40,
				NULL, wnd, EditorProc,
				SHADOW     |
				MINMAXBOX  |
				CONTROLBOX |
				VSCROLLBAR |
				HSCROLLBAR |
				MOVEABLE   |
				HASBORDER  |
				SIZEABLE   |
				MULTILINE
	);
    if (strcmp(FileName, Untitled))	{
		if ((wnd1->extension = malloc(strlen(FileName)+1)) != NULL)	{
			strcpy(wnd1->extension, FileName);
    	    LoadFile(wnd1, (int) sb.st_size);
		}
	}
	SendMessage(wnd1, SETFOCUS, TRUE, 0);
}

/*
 *  Load the notepad file into the editor text buffer
 */
static void LoadFile(WINDOW wnd, int tLen)
{
    char *Buf;
    FILE *fp;

    if ((Buf = malloc(tLen+1)) != NULL)    {
        if ((fp = fopen(wnd->extension, "rt")) != NULL)    {
            memset (Buf, 0, tLen+1);
            fread(Buf, tLen, 1, fp);
		    SendMessage(wnd, SETTEXT, (PARAM) Buf, 0);
            fclose(fp);
        }
		free(Buf);
    }
}

/*
 *  print the current notepad
 */
static void PrintPad(WINDOW wnd)
{
	unsigned char *text;

	/* ---------- print the file name ---------- */
	fputs("\r\n", stdprn);
	fputs(GetTitle(wnd), stdprn);
	fputs(":\r\n\n", stdprn);

	/* ---- get the address of the editor text ----- */
	text = GetText(wnd);

	/* ------- print the notepad text --------- */
	while (*text)	{
		if (*text == '\n')
			fputc('\r', stdprn);
		fputc(*text++, stdprn);
	}

	/* ------- follow with a form feed? --------- */
	if (YesNoBox("Form Feed?"))
		fputc('\f', stdprn);
}

static void SaveFile(WINDOW wnd, int Saveas)
{
	FILE *fp;
	if (wnd->extension == NULL || Saveas)	{
		char FileName[64];
		if (DlgSaveAs(FileName))	{
			if (wnd->extension != NULL)
				free(wnd->extension);
			if ((wnd->extension = malloc(strlen(FileName)+1)) != NULL)	{
				strcpy(wnd->extension, FileName);
				AddTitle(wnd, NameComponent(FileName));
				SendMessage(wnd, BORDER, 0, 0);
			}
		}
		else
			return;
	}
	if (wnd->extension != NULL)	{
		WINDOW mwnd = MomentaryMessage("Saving the file");
		if ((fp = fopen(wnd->extension, "wt")) != NULL)	{
			fwrite(GetText(wnd), strlen(GetText(wnd)), 1, fp);
			fclose(fp);
			wnd->TextChanged = FALSE;
		}
		SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
	}
}

static void ShowPosition(WINDOW wnd)
{
	char status[30];
	sprintf(status, "Line:%4d  Column: %2d",
		wnd->CurrLine, wnd->CurrCol);
	SendMessage(GetParent(wnd), ADDSTATUS, (PARAM) status, 0);
}

static int EditorProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn;
	switch (msg)	{
		case SETFOCUS:
			rtn = DefaultWndProc(wnd, msg, p1, p2);
			if ((int)p1 == FALSE)
				SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
			else 
				ShowPosition(wnd);
			return rtn;
		case KEYBOARD_CURSOR:
			rtn = DefaultWndProc(wnd, msg, p1, p2);
			ShowPosition(wnd);
			return rtn;
		case COMMAND:
			if ((int) p1 == ID_HELP)	{
				DisplayHelp(wnd, "MEMOPADDOC");
				return TRUE;
			}
			break;
		case CLOSE_WINDOW:
			if (wnd->TextChanged)	{
				char *cp = malloc(25+strlen(GetTitle(wnd)));
				SendMessage(wnd, SETFOCUS, TRUE, 0);
				if (cp != NULL)	{
					strcpy(cp, GetTitle(wnd));
					strcat(cp, "\nText changed. Save it?");
					if (YesNoBox(cp))
						SendMessage(GetParent(wnd), COMMAND, ID_SAVE, 0);
					free(cp);
				}
			}
			wndpos = 0;
			if (wnd->extension != NULL)	{
				free(wnd->extension);
				wnd->extension = NULL;
			}
			break;
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

static char *NameComponent(char *FileName)
{
	char *Fname;
	if ((Fname = strrchr(FileName, '\\')) == NULL)
		if ((Fname = strrchr(FileName, ':')) == NULL)
			Fname = FileName-1;
	return Fname + 1;
}

