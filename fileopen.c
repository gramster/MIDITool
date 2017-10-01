/* ----------- fileopen.c ------------- */

#include <string.h>
#ifdef MSC
#include <direct.h>
#else
#include <dir.h>
#endif
#include <dos.h>
#include <ctype.h>
#include "dflat.h"

#ifdef INCLUDE_DIALOG_BOXES

static int DlgFileOpen(char *, char *, DBOX *);
static int DlgFnOpen(WINDOW, MESSAGE, PARAM, PARAM);
static void InitDlgBox(WINDOW);
static void StripPath(char *);
static int IncompleteFilename(char *);

static char OrigSpec[80];
static char FileSpec[80];
char FileName[80];

static int Saving;
extern DBOX FileOpen;
extern DBOX SaveAs;

/*
 * Dialog Box to select a file to open
 */
int DlgOpenFile(char *Fpath, char *Fname)
{
	return DlgFileOpen(Fpath, Fname, &FileOpen);
}

/*
 * Dialog Box to select a file to save as
 */
int DlgSaveAs(char *Fname)
{
	return DlgFileOpen(NULL, Fname, &SaveAs);
}

/* --------- generic file open ---------- */
static int DlgFileOpen(char *Fpath, char *Fname, DBOX *db)
{
	int  rtn;
	char savedir[80];

	getcwd(savedir, sizeof savedir);
	if (Fpath != NULL)	{
		strncpy(FileSpec, Fpath, sizeof(FileSpec)-1);
		Saving = FALSE;
	}
	else	{
		*FileSpec = '\0';
		Saving = TRUE;
	}
	strcpy(FileName, FileSpec);
	strcpy(OrigSpec, FileSpec);

	if ((rtn = DialogBox(NULLWND, db, TRUE, DlgFnOpen)) != FALSE)
		strcpy(Fname, FileName);
	else
		*Fname = '\0';

	setdisk(toupper(*savedir) - 'A');
	chdir(savedir);

	return rtn;
}

/*
 *  Process dialog box messages
 */
static int DlgFnOpen(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	switch (msg)	{
		case INITIATE_DIALOG:
			InitDlgBox(wnd);
			break;
		case COMMAND:
			switch ((int) p1)	{
				case ID_FILENAME:
					if (p2 != ENTERFOCUS)	{
						/* allow user to modify the file spec */
						GetItemText(wnd, ID_FILENAME,
								FileName, 65);
						if (IncompleteFilename(FileName) || Saving)	{
							strcpy(OrigSpec, FileName);
							StripPath(OrigSpec);
						}
						if (p2 != LEAVEFOCUS)
							SendMessage(wnd, COMMAND, ID_OK, 0);
					}
					return TRUE;
				case ID_OK:
					GetItemText(wnd, ID_FILENAME,
							FileName, 65);
					strcpy(FileSpec, FileName);
					if (IncompleteFilename(FileName))	{
						/* no file name yet */
						InitDlgBox(wnd);
						strcpy(OrigSpec, FileSpec);
						return TRUE;
					}
					else	{
						GetItemText(wnd, ID_PATH, FileName, 65);
						strcat(FileName, FileSpec);
					}
					break;
				case ID_FILES:
					switch ((int) p2)	{
						case LB_SELECTION:
							/* selected a different filename */
							GetDlgListText(wnd, FileName,
										ID_FILES);
							PutItemText(wnd, ID_FILENAME,
											FileName);
							break;
						case LB_CHOOSE:
							/* chose a file name */
							GetDlgListText(wnd, FileName,
									ID_FILES);
							SendMessage(wnd, COMMAND, ID_OK, 0);
							break;
						default:
							break;
					}
					return TRUE;
				case ID_DRIVE:
					switch ((int) p2)	{
						case ENTERFOCUS:
							if (Saving)
								*FileSpec = '\0';
							break;
						case LEAVEFOCUS:
							if (Saving)
								strcpy(FileSpec, FileName);
							break;
						case LB_SELECTION:	{
							char dd[25];
							/* selected different drive/dir */
							GetDlgListText(wnd, dd,
												ID_DRIVE);
							if (*(dd+2) == ':')
								*(dd+3) = '\0';
							else
								*(dd+strlen(dd)-1) = '\0';
							strcpy(FileName, dd+1);
							if (*(dd+2) != ':' && *OrigSpec != '\\')
								strcat(FileName, "\\");
							strcat(FileName, OrigSpec);
							if (*(FileName+1) != ':' && *FileName != '.')	{
								GetItemText(wnd, ID_PATH, FileSpec, 65);
								strcat(FileSpec, FileName);
							}
							else 
								strcpy(FileSpec, FileName);
							break;
						}
						case LB_CHOOSE:
							/* chose drive/dir */
							if (Saving)
								PutItemText(wnd, ID_FILENAME, "");
							InitDlgBox(wnd);
							return TRUE;
						default:
							break;
					}
					PutItemText(wnd, ID_FILENAME, FileSpec);
					return TRUE;


				default:
					break;
			}
		default:
			break;
	}
	return DefaultWndProc(wnd, msg, p1, p2);
}

/*
 *  Initialize the dialog box
 */
static void InitDlgBox(WINDOW wnd)
{
	if (*FileSpec && !Saving)
		PutItemText(wnd, ID_FILENAME, FileSpec);
	if (DlgDirList(wnd, FileSpec, ID_FILES, ID_PATH, 0))	{
	    StripPath(FileSpec);
		DlgDirList(wnd, "*.*", ID_DRIVE, 0, 0xc010);
	}
}

/*
 * Strip the drive and path information from a file spec
 */
static void StripPath(char *filespec)
{
	char *cp, *cp1;

	cp = strchr(filespec, ':');
	if (cp != NULL)
		cp++;
	else
		cp = filespec;
	while (TRUE)	{
		cp1 = strchr(cp, '\\');
		if (cp1 == NULL)
			break;
		cp = cp1+1;
	}
	strcpy(filespec, cp);
}


static int IncompleteFilename(char *s)
{
	int lc = strlen(s)-1;
	if (strchr(s, '?') || strchr(s, '*') || !*s)
		return TRUE;
	if (*(s+lc) == ':' || *(s+lc) == '\\')
		return TRUE;
	return FALSE;
}

#endif
