/* ------------- textbox.c ------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "dflat.h"

#ifdef INCLUDE_SCROLLBARS
static int ComputeVScrollBox(WINDOW);
static int ComputeHScrollBox(WINDOW);
static void MoveScrollBox(WINDOW, int);
#endif
static char *GetTextLine(WINDOW, int);

#ifdef INCLUDE_SCROLLBARS
int HScrolling = FALSE;
int VScrolling = FALSE;
#endif

int TextBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
#ifdef INCLUDE_SCROLLBARS
	int	mx = (int) p1 - GetLeft(wnd);
	int	my = (int) p2 - GetTop(wnd);
#endif
	switch (msg)	{
		case CREATE_WINDOW:
			wnd->HScrollBox = wnd->VScrollBox = TRUE;
			ClearTextPointers(wnd);
			break;
		case ADDTEXT:	{
			/* ======== need to assure that length !> 64K ======= */
			int adln = strlen((char *)p1);
			if (wnd->text != NULL)	{
				int txln = strlen(wnd->text);
				if (txln+adln > wnd->textlen)	{
					wnd->text = realloc(wnd->text, txln+adln+2);
					wnd->textlen = txln+adln;
				}
			}
			else	{
				if ((wnd->text = malloc(adln+2)) != NULL)
					*wnd->text = '\0';
				wnd->textlen = adln+1;
			}
			if (wnd->text != NULL)	{
				strcat(wnd->text, (char*) p1);
				strcat(wnd->text, "\n");
				BuildTextPointers(wnd);
			}
			break;
		}
		case SETTEXT:	{
			char *cp;
			unsigned int len;
			cp = (void *) p1;
			len = strlen(cp)+1;
			if (wnd->text == NULL || wnd->textlen < len)	{
				wnd->textlen = len;
				if ((wnd->text = realloc(wnd->text, len)) == NULL)
					break;
			}
			strcpy(wnd->text, cp);
			BuildTextPointers(wnd);
			break;
		}
		case CLEARTEXT:
			if (wnd->text != NULL)
				free(wnd->text);
			wnd->text = NULL;
			wnd->textlen = 0;
			wnd->wlines = 0;
			wnd->textwidth = 0;
			wnd->wtop = wnd->wleft = 0;
			ClearBlock(wnd);
			ClearTextPointers(wnd);
			break;
		case KEYBOARD:
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			switch ((int) p1)	{
				case UP:
					if (wnd->wtop)
						SendMessage(wnd, SCROLL, FALSE, 0);
					return TRUE;
				case DN:
					if (wnd->wtop+ClientHeight(wnd) < wnd->wlines)
						SendMessage(wnd, SCROLL, TRUE, 0);
					return TRUE;
				case FWD:
					SendMessage(wnd, HORIZSCROLL, TRUE, 0);
					return TRUE;
				case BS:
					SendMessage(wnd, HORIZSCROLL, FALSE, 0);
					return TRUE;
				case PGUP:
					if (wnd->wtop)	{
						wnd->wtop -= ClientHeight(wnd);
						if (wnd->wtop < 0)
							wnd->wtop = 0;
						SendMessage(wnd, PAINT, 0, 0);
						return TRUE;
					}
					return TRUE;
				case PGDN:
					if (wnd->wtop+ClientHeight(wnd) < wnd->wlines)	{
						wnd->wtop += ClientHeight(wnd);
						if (wnd->wtop > wnd->wlines-ClientHeight(wnd))
							wnd->wtop = wnd->wlines-ClientHeight(wnd);
						SendMessage(wnd, PAINT, 0, 0);
						return TRUE;
					}
					return TRUE;
				case HOME:
					if (wnd->wtop || wnd->wleft)	{
						wnd->wtop = wnd->wleft = 0;
						SendMessage(wnd, PAINT, 0, 0);
					}
					return TRUE;
				case END:
					if (wnd->wtop+ClientHeight(wnd) < wnd->wlines)	{
						wnd->wtop = wnd->wlines-ClientHeight(wnd);
						wnd->wleft = 0;
						SendMessage(wnd, PAINT, 0, 0);
					}
					return TRUE;
				default:
					break;
			}
			break;
		case LEFT_BUTTON:
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowSizing || WindowMoving)
				return FALSE;
#endif
#ifdef INCLUDE_SCROLLBARS
			if (TestAttribute(wnd, VSCROLLBAR) && (VScrolling ||
					mx == WindowWidth(wnd)-1))	{

				/* -------- in the right border ------- */
				if (my == 0 || my == ClientHeight(wnd)+1)
					/* ------ above or below the scroll bar ---- */
					break;

				/* ---------- in the scroll bar ----------- */

				VScrolling = TRUE;

				if (my == 1)	{
					/* -------- top scroll button --------- */
					SendMessage(wnd, SCROLL, FALSE, 0);
					return TRUE;
				}
				if (my == ClientHeight(wnd))	{
					/* -------- bottom scroll button --------- */
					SendMessage(wnd, SCROLL, TRUE, 0);
					return TRUE;
				}
				if (my-1 != wnd->VScrollBox)	{
					int dir = my-1 > wnd->VScrollBox;

					while (dir ? (my-1 > wnd->VScrollBox) :
								 (my-1 < wnd->VScrollBox))	{
						if (!SendMessage(wnd, SCROLL, dir, TRUE))
							break;
						wnd->VScrollBox = ComputeVScrollBox(wnd);
					}
					SendMessage(wnd, SHOW_WINDOW, 0, 0);
					return TRUE;
				}
			}
			if (TestAttribute(wnd, HSCROLLBAR) &&
				(HScrolling || my == WindowHeight(wnd)-1))	{
				/* -------- in the bottom border ------- */
				if (mx == 0 || my == ClientWidth(wnd)+1)
					/* ------  outside the scroll bar ---- */
					break;

				HScrolling = TRUE;

				if (mx == 1)	{
					SendMessage(wnd, HORIZSCROLL, FALSE, 0);
					return TRUE;
				}
				if (mx == WindowWidth(wnd)-2)	{
					SendMessage(wnd, HORIZSCROLL, TRUE, 0);
					return TRUE;
				}

				if (mx-1 != wnd->HScrollBox)	{
					int dir = mx-1 > wnd->HScrollBox;
					while (dir ? (mx-1 > wnd->HScrollBox) :
								 (mx-1 < wnd->HScrollBox))	{
						if (!SendMessage(wnd, HORIZSCROLL, dir, TRUE))
							break;
						wnd->HScrollBox = ComputeHScrollBox(wnd);
					}
					SendMessage(wnd, SHOW_WINDOW, 0, 0);
					return TRUE;
				}
			}
			break;
		case BUTTON_RELEASED:
			HScrolling = VScrolling = FALSE;
			break;
#endif
		case SCROLL:
			if (isVisible(wnd))	{
				if (p1)	{
					if (wnd->wtop+ClientHeight(wnd) >= wnd->wlines)
						return FALSE;
					wnd->wtop++;
				}
				else	{
					if (wnd->wtop == 0)
						return FALSE;
					--wnd->wtop;
				}
				if ((int) p2 == FALSE)	{
					RECT rc;
					rc = ClipRectangle(wnd, ClientRect(wnd));
					if (ValidRect(rc))	{
						scroll_window(wnd, rc, (int)p1);
						if (!(int)p1)
							WriteTextLine(wnd, NULL, wnd->wtop, FALSE);
						else	{
							int y = RectBottom(rc)-GetClientTop(wnd);
							WriteTextLine(wnd, NULL,
								wnd->wtop+y, FALSE);
						}
					}
#ifdef INCLUDE_SCROLLBARS
					if (TestAttribute(wnd, VSCROLLBAR))	{
						int vscrollbox = ComputeVScrollBox(wnd);
						if (vscrollbox != wnd->VScrollBox)
							MoveScrollBox(wnd, vscrollbox);
					}
#endif
				}
			}
			return TRUE;
		case HORIZSCROLL:
			if (isVisible(wnd))	{
				if (p1)	{
					if (wnd->wleft + ClientWidth(wnd)-1 >= wnd->textwidth)
						return FALSE;
					wnd->wleft++;
				}
				else	{
					if (wnd->wleft == 0)
						return FALSE;
					--wnd->wleft;
				}
				if ((int) p2 == FALSE)
					SendMessage(wnd, PAINT, 0, 0);
			}
			return TRUE;
		case PAINT:
			if (isVisible(wnd) && wnd->wlines)	{
				RECT rc, rcc;
				int y;
				char blankline[SCREENWIDTH+1];

				if ((RECT *)p1 == NULL)
					rc = RelativeWindowRect(wnd, WindowRect(wnd));
				else
					rc = *(RECT *)p1;

				if (TestAttribute(wnd, HASBORDER) &&
						RectRight(rc) >= WindowWidth(wnd)-1)	{
					if (RectLeft(rc) >= WindowWidth(wnd)-1)
						return TRUE;
					RectRight(rc) = WindowWidth(wnd)-2;
				}

				rcc = AdjustRectangle(wnd, rc);
				memset(blankline, ' ', SCREENWIDTH);
				blankline[RectRight(rcc)+1] = '\0';

				for (y = RectTop(rc); y <= RectBottom(rc); y++)	{
					int yy;
					if (TestAttribute(wnd, HASBORDER | HASTITLEBAR))	{
						if (y < TopBorderAdj(wnd))
							continue;
						if (y > WindowHeight(wnd)-2)
							continue;
					}
					yy = y-TopBorderAdj(wnd);
					if (yy < wnd->wlines-wnd->wtop)
						WriteTextLine(wnd, &rc, yy+wnd->wtop, FALSE);
					else	{
						SetStandardColor(wnd);
						writeline(wnd, blankline+RectLeft(rcc),
								RectLeft(rcc)+1, y, FALSE);
					}
				}
#ifdef INCLUDE_SCROLLBARS
				if (TestAttribute(wnd, VSCROLLBAR | HSCROLLBAR))	{
					int hscrollbox = ComputeHScrollBox(wnd);
					int vscrollbox = ComputeVScrollBox(wnd);
					if (hscrollbox != wnd->HScrollBox ||
							vscrollbox != wnd->VScrollBox)	{
						wnd->HScrollBox = hscrollbox;
						wnd->VScrollBox = vscrollbox;
						SendMessage(wnd, BORDER, p1, 0);
					}
				}
#endif
				return FALSE;
			}
			break;
		case CLOSE_WINDOW:
			SendMessage(wnd, CLEARTEXT, 0, 0);
			if (wnd->TextPointers != NULL)	{
				free(wnd->TextPointers);
				wnd->TextPointers = NULL;
			}
			break;
		default:
			break;
	}
	return BaseWndProc(TEXTBOX, wnd, msg, p1, p2);
}

#ifdef INCLUDE_SCROLLBARS
static int ComputeVScrollBox(WINDOW wnd)
{
	int pagelen = wnd->wlines - ClientHeight(wnd);
	int barlen = ClientHeight(wnd)-2;
	int lines_tick;
	int vscrollbox;

	if (pagelen < 1 || barlen < 1)
		vscrollbox = 1;
	else	{
		if (pagelen > barlen)
			lines_tick = pagelen / barlen;
		else
			lines_tick = barlen / pagelen;
		vscrollbox = 1 + (wnd->wtop / lines_tick);
		if (vscrollbox > ClientHeight(wnd)-2 ||
				wnd->wtop + ClientHeight(wnd) >= wnd->wlines)
			vscrollbox = ClientHeight(wnd)-2;
	}
	return vscrollbox;
}

static int ComputeHScrollBox(WINDOW wnd)
{
	int pagewidth = wnd->textwidth - ClientWidth(wnd);
	int barlen = ClientWidth(wnd)-2;
	int chars_tick;
	int hscrollbox;

	if (pagewidth < 1 || barlen < 1)
		hscrollbox = 1;
	else 	{
		if (pagewidth > barlen)
			chars_tick = pagewidth / barlen;
		else
			chars_tick = barlen / pagewidth;
		hscrollbox = 1 + (wnd->wleft / chars_tick);
		if (hscrollbox > ClientWidth(wnd)-2 ||
				wnd->wleft + ClientWidth(wnd) >= wnd->textwidth)
			hscrollbox = ClientWidth(wnd)-2;
	}
	return hscrollbox;
}
#endif

static char *GetTextLine(WINDOW wnd, int selection)
{
	char *line;
	int len = 0;
	char *cp, *cp1;
	cp = cp1 = TextLine(wnd, selection);
	while (*cp && *cp != '\n')	{
		len++;
		cp++;
	}
	line = malloc(len+6);
	if (line != NULL)	{
		memmove(line, cp1, len);
		line[len] = '\0';
	}
	return line;
}

void WriteTextLine(WINDOW wnd, RECT *rcc, int y, int reverse)
{
	int len = 0;
	int dif = 0;
	static unsigned char line[100];
	RECT rc;
	unsigned char *lp, *svlp;
	int lnlen;
	int i;
	int trunc = FALSE;

	if (y < wnd->wtop || y >= wnd->wtop+ClientHeight(wnd))
		return;

	if (rcc == NULL)
		rc = RelativeWindowRect(wnd, WindowRect(wnd));
	else
		rc = *rcc;

	if (RectLeft(rc) >= WindowWidth(wnd)-1)
		return;
	if (RectRight(rc) == 0)
		return;

	rc = AdjustRectangle(wnd, rc);

	lp = svlp = GetTextLine(wnd, y);
	lnlen = LineLength(lp);

	/* -------- insert block color change controls ------- */
	if (BlockMarked(wnd))	{
		int bbl = wnd->BlkBegLine;
		int bel = wnd->BlkEndLine;
		int bbc = wnd->BlkBegCol;
		int bec = wnd->BlkEndCol;
		int by = y;

		if (bbl > bel)	{
			swap(bbl, bel);
			swap(bbc, bec);
		}
		if (bbl == bel && bbc > bec)
			swap(bbc, bec);

		if (by >= bbl && by <= bel)	{
			/* ------ the block includes this line ----- */
			int blkbeg = 0;
			int blkend = lnlen;
			if (!(by > bbl && by < bel))	{
				/* --- the entire line is not in the block --- */
				if (by == bbl)
					/* ---- the block begins on this line ---- */
					blkbeg = bbc;
				if (by == bel)
					/* ---- the block ends on this line ---- */
					blkend = bec;
			}
			memmove(lp+blkend+1, lp+blkend, strlen(lp+blkend)+1);
			lp[blkend] = RESETCOLOR;
			memmove(lp+blkbeg+3, lp+blkbeg, strlen(lp+blkbeg)+1);
			lp[blkbeg] = CHANGECOLOR;
			SetReverseColor(wnd);
			lp[blkbeg+1] = foreground | 0x80;
			lp[blkbeg+2] = background | 0x80;
			lnlen += 4;
		}
	}

	for (i = 0; i < wnd->wleft+3; i++)	{
		if (*(lp+i) == '\0')
			break;
		if (*(unsigned char *)(lp + i) == RESETCOLOR)
			break;
	}
	if (*(lp+i) && i < wnd->wleft+3)	{
		if (wnd->wleft+4 > lnlen)
			trunc = TRUE;
		else 
			lp += 4;
	}
	else 	{
		for (i = 0; i < wnd->wleft; i++)	{
			if (*(lp+i) == '\0')
				break;
			if (*(unsigned char *)(lp + i) == CHANGECOLOR)	{
				*(lp+wnd->wleft+2) = *(lp+i+2);
				*(lp+wnd->wleft+1) = *(lp+i+1);
				*(lp+wnd->wleft) = *(lp+i);
				break;
			}
		}
	}

	if (!trunc)	{
		if (lnlen < wnd->wleft)
			lnlen = 0;
		else
			lp += wnd->wleft;


		if (y-wnd->wtop < RectTop(rc) || y-wnd->wtop > RectBottom(rc))
			return;

		if (lnlen > RectLeft(rc))	{
			int ct = RectLeft(rc);
			char *initlp = lp;
			while (ct)	{
				if (*(unsigned char *)lp == CHANGECOLOR)
					lp += 3;
				else if (*(unsigned char *)lp == RESETCOLOR)
					lp++;
				else
					lp++, --ct;
			}
			if (RectLeft(rc))	{
				char *lpp = lp;
				while (*lpp)	{
					if (*(unsigned char *)lpp == CHANGECOLOR)
						break;
					if (*(unsigned char *)lpp == RESETCOLOR)	{
						lpp = lp;
						while (lpp >= initlp)	{
							if (*(unsigned char *)lpp == CHANGECOLOR)	{
								lp -= 3;
								memmove(lp,lpp,3);
								break;
							}
							--lpp;
						}
						break;
					}
					lpp++;
				}
			}
			lnlen = LineLength(lp);
			len = min(lnlen, RectWidth(rc));
			lnlen = LineLength(lp);
			dif = strlen(lp) - lnlen;
			len += dif;
			if (len > 0)
				strncpy(line, lp, len);
		}
	}

	while (len < RectWidth(rc)+dif)
		line[len++] = ' ';
	line[len] = '\0';

	dif = 0;
	if (reverse)	{
		char *cp = line;
		SetReverseColor(wnd);
		while ((cp = strchr(cp, CHANGECOLOR)) != NULL)	{
			cp += 2;
			*cp++ = background | 0x80;
		}
		if (*(unsigned char *)line == CHANGECOLOR)
			dif = 3;
	}
	else
		SetStandardColor(wnd);
	writeline(wnd, line+dif,
				RectLeft(rc)+BorderAdj(wnd),
					y-wnd->wtop+TopBorderAdj(wnd), FALSE);
	if (svlp != NULL)
		free(svlp);
}

void SetAnchor(WINDOW wnd, int mx, int my)
{
	if (BlockMarked(wnd))	{
		ClearBlock(wnd);
		SendMessage(wnd, PAINT, 0, 0);
	}
	/* ------ set the anchor ------ */
	wnd->BlkBegLine = wnd->BlkEndLine = my;
	wnd->BlkBegCol = wnd->BlkEndCol = mx;
	WriteTextLine(wnd, NULL, my, FALSE);
}

void ClearTextPointers(WINDOW wnd)
{
	wnd->TextPointers = realloc(wnd->TextPointers, sizeof(int));
	if (wnd->TextPointers != NULL)
		*(wnd->TextPointers) = 0;
}

#define INITLINES 100

void BuildTextPointers(WINDOW wnd)
{
	char *cp = wnd->text, *cp1;
	int incrs = INITLINES;
	int off;
	wnd->textwidth = wnd->wlines = 0;
	while (*cp)	{
		if (incrs == INITLINES)	{
			incrs = 0;
			wnd->TextPointers = realloc(wnd->TextPointers,
					(wnd->wlines + INITLINES) * sizeof(int));
			if (wnd->TextPointers == NULL)
				break;
		}
		off = (unsigned int) (cp - wnd->text);
		*((wnd->TextPointers) + wnd->wlines) = off;
		wnd->wlines++;
		incrs++;
		cp1 = cp;
		while (*cp && *cp != '\n')
			cp++;
		wnd->textwidth = max(wnd->textwidth,
						(unsigned int) (cp - cp1));
		if (*cp)
			cp++;
	}
}

#ifdef INCLUDE_SCROLLBARS
static void MoveScrollBox(WINDOW wnd, int vscrollbox)
{
    foreground = FrameForeground(wnd);
    background = FrameBackground(wnd);
	PutWindowChar(wnd, WindowWidth(wnd)-1,
			wnd->VScrollBox+1, SCROLLBARCHAR);
	PutWindowChar(wnd, WindowWidth(wnd)-1,
			vscrollbox+1, SCROLLBOXCHAR);
	wnd->VScrollBox = vscrollbox;
}
#endif
