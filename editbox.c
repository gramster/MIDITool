/* ------------- editbox.c ------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "dflat.h"

#define EDITBUFFERLENGTH  4096
#define ENTRYBUFFERLENGTH 1024
#define GROWLENGTH        1024


#define EditBufLen(wnd) (isMultiLine(wnd) ? EDITBUFFERLENGTH : ENTRYBUFFERLENGTH)
#define WndCol (wnd->CurrCol-wnd->wleft)

#define CurrChar (TextLine(wnd, wnd->CurrLine)+wnd->CurrCol)

static void PasteText(WINDOW, char *, int);
static void SaveDeletedText(WINDOW, char *, int);
static void Forward(WINDOW);
static void Backward(WINDOW);
static void End(WINDOW);
static void Home(WINDOW);
static void Downward(WINDOW);
static void Upward(WINDOW);
static void StickEnd(WINDOW);
static void UpLine(WINDOW);
static void DownLine(WINDOW);
static void NextWord(WINDOW);
static void PrevWord(WINDOW);
static void ResetEditBox(WINDOW);
static void AddTextPointers(WINDOW, int, int);
static int TextLineNumber(WINDOW, char *);
#ifdef INCLUDE_DIALOG_BOXES
static void SearchTextBox(WINDOW);
static void ReplaceTextBox(WINDOW);
static void SearchNext(WINDOW, int);
#endif
#ifdef INCLUDE_MULTILINE
#define SetLinePointer(wnd, ln)	(wnd->CurrLine = ln)
#else
#define SetLinePointer(wnd, ln) (wnd->CurrLine = 0)
#endif

static int KeyBoardMarking, ButtonDown;
int TextMarking;
static int bx, by;

char *Clipboard;
int ClipboardLength;

int EditBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn;
	static int py = -1;
	int kx = (int) p1 - GetLeft(wnd);
	int ky = (int) p2 - GetTop(wnd);
	int c;
	RECT rc;
	char *lp;
	int len;
	int	mx;
	int	my;
	char *currchar = CurrChar;
	int PassOn = FALSE;

	rc = ClientRect(wnd);
	switch (msg)	{
		case CREATE_WINDOW:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			wnd->text = calloc(1, EditBufLen(wnd)+1);
			wnd->textlen = EditBufLen(wnd);
			ResetEditBox(wnd);
			return rtn;
		case ADDTEXT:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			currchar = CurrChar;
			if (!isMultiLine(wnd))	{
				wnd->CurrLine = 0;
				wnd->CurrCol = strlen((char *)p1);
				if (wnd->CurrCol >= ClientWidth(wnd))	{
					wnd->wleft = wnd->CurrCol - ClientWidth(wnd);
					wnd->CurrCol -= wnd->wleft;
				}
				wnd->BlkEndCol = wnd->CurrCol;
				PostMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			}
			BuildTextPointers(wnd);
			return rtn;
		case SETTEXT:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			wnd->CurrLine = 0;
			return rtn;
		case CLEARTEXT:
			ResetEditBox(wnd);
			ClearTextPointers(wnd);
			break;
		case KEYBOARD_CURSOR:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			if (wnd == inFocus)	{
				if (!CharInView(wnd, (int)p1+BorderAdj(wnd),
						(int)p2+TopBorderAdj(wnd)))
					SendMessage(NULLWND, HIDE_CURSOR, 0, 0);
				else 
					SendMessage(NULLWND, SHOW_CURSOR,
						GetCommandToggle(MainMenu, ID_INSERT), 0);
			}
			return rtn;
		case EB_GETTEXT:	{
			char *cp1 = (char *)p1;
			char *cp2 = wnd->text;
			while (p2-- && *cp2 && *cp2 != '\n')
				*cp1++ = *cp2++;
			*cp1 = '\0';
			return TRUE;
		}
		case EB_PUTTEXT:
			SendMessage(wnd, CLEARTEXT, 0, 0);
			SendMessage(wnd, ADDTEXT, p1, p2);
			return TRUE;
		case SETFOCUS:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			if (p1)	{
				SendMessage(NULLWND, SHOW_CURSOR, GetCommandToggle(MainMenu, ID_INSERT), 0);
				SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			}
			else
				SendMessage(NULLWND, HIDE_CURSOR, 0, 0);
			return rtn;
#ifdef INCLUDE_MULTILINE
		case SHIFT_CHANGED:
			if (!((int)p1 & (LEFTSHIFT | RIGHTSHIFT)) && KeyBoardMarking)	{
				SendMessage(wnd, BUTTON_RELEASED, 0, 0);
				KeyBoardMarking = FALSE;
			}
			break;
		case DOUBLE_CLICK:
			if (KeyBoardMarking)
				return TRUE;
			break;
#endif
		case LEFT_BUTTON:
#ifdef INCLUDE_SCROLLBARS
			if (HScrolling || VScrolling)
				break;
#endif
#ifdef INCLUDE_MULTILINE
			if (KeyBoardMarking || TextMarking)
				return TRUE;
#endif
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			if (!InsideRect(p1, p2, rc))
				break;

			mx = (int) p1 - GetClientLeft(wnd);
			my = (int) p2 - GetClientTop(wnd);

#ifdef INCLUDE_MULTILINE
			if (isMultiLine(wnd))	{

				if (BlockMarked(wnd))	{
					ClearBlock(wnd);
					SendMessage(wnd, PAINT, 0, 0);
				}

				if (wnd->wlines)	{
					if (my > wnd->wlines-1)
						break;
					lp = TextLine(wnd, my+wnd->wtop);
					len = (int) (strchr(lp, '\n') - lp);
					mx = min(mx, len);
					if (mx < wnd->wleft)	{
						mx = 0;
						SendMessage(wnd, KEYBOARD, HOME, 0);
					}
					ButtonDown = TRUE;
					bx = mx;
					by = my;
				}
				else
					mx = my = 0;

				wnd->WndRow = my;
				SetLinePointer(wnd, my+wnd->wtop);
			}
#endif
			if (isMultiLine(wnd) || !BlockMarked(wnd))
				wnd->CurrCol = mx+wnd->wleft;
			PostMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			return TRUE;
#ifdef INCLUDE_MULTILINE
		case MOUSE_MOVED:
			mx = (int) p1 - GetClientLeft(wnd);
			my = (int) p2 - GetClientTop(wnd);
			if (my > wnd->wlines-1)
				break;

			if (ButtonDown)	{
				SetAnchor(wnd, bx+wnd->wleft, by+wnd->wtop);
				TextMarking = TRUE;
				SendMessage(wnd, CAPTURE_MOUSE, TRUE, 0);
				ButtonDown = FALSE;
			}

			if (TextMarking
#ifdef INCLUDE_SYSTEM_MENUS
					&& !(WindowMoving || WindowSizing)
#endif
							)	{
				int ptop;
				int pbot;
				char *lp;
				int len;
				int y;
				int bbl, bel;
				RECT rc;
				rc = ClientRect(wnd);

				if (!InsideRect(p1, p2, rc))  	{
					int hs = -1, sc = -1;
					if (p1 < GetClientLeft(wnd))	{
						hs = FALSE;
						p1 = GetClientLeft(wnd);
					}
					if (p1 > GetClientRight(wnd))	{
						hs = TRUE;
						p1 = GetClientRight(wnd);
					}
					if (p2 < GetClientTop(wnd))	{
						sc = FALSE;
						p2 = GetClientTop(wnd);
					}
					if (p2 > GetClientBottom(wnd))	{
						sc = TRUE;
						p2 = GetClientBottom(wnd);
					}
					SendMessage(NULLWND, MOUSE_CURSOR, p1, p2);
					if (sc != -1)
						SendMessage(wnd, SCROLL, sc, 0);
					if (hs != -1)
						SendMessage(wnd, HORIZSCROLL, hs, 0);
					mx = (int) p1 - GetClientLeft(wnd);
					my = (int) p2 - GetClientTop(wnd);
				}

				ptop = min(wnd->BlkBegLine, wnd->BlkEndLine);
				pbot = max(wnd->BlkBegLine, wnd->BlkEndLine);

				lp = TextLine(wnd, wnd->wtop+my);
				len = (int) (strchr(lp, '\n') - lp);
				mx = min(mx, len-wnd->wleft);

				wnd->BlkEndCol = mx+wnd->wleft;
				wnd->BlkEndLine = my+wnd->wtop;

				bbl = min(wnd->BlkBegLine, wnd->BlkEndLine);
				bel = max(wnd->BlkBegLine, wnd->BlkEndLine);

				while (ptop < bbl)	{
					WriteTextLine(wnd, NULL, ptop, FALSE);
					ptop++;
				}
				for (y = bbl; y <= bel; y++)
					WriteTextLine(wnd, NULL, y, FALSE);
				while (pbot > bel)	{
					WriteTextLine(wnd, NULL, pbot, FALSE);
					--pbot;
				}
				return TRUE;
			}
			break;
		case BUTTON_RELEASED:
			if (!isMultiLine(wnd))
				break;
			ButtonDown = FALSE;
#ifdef INCLUDE_SCROLLBARS
			if (HScrolling || VScrolling)
				break;
#endif
			if (TextMarking
#ifdef INCLUDE_SYSTEM_MENUS
					&& !(WindowMoving || WindowSizing)
#endif
							)	{
				PostMessage(wnd, RELEASE_MOUSE, 0, 0);
				TextMarking = FALSE;
				if (wnd->BlkBegLine > wnd->BlkEndLine)	{
					swap(wnd->BlkBegLine, wnd->BlkEndLine);
					swap(wnd->BlkBegCol, wnd->BlkEndCol);
				}
				if (wnd->BlkBegLine == wnd->BlkEndLine &&
						wnd->BlkBegCol > wnd->BlkEndCol)
					swap(wnd->BlkBegCol, wnd->BlkEndCol);
				return TRUE;
			}
			else
				py = -1;
			break;
		case SCROLL:
			if (!isMultiLine(wnd))
				break;
			if ((rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2)) != FALSE)	{
				if (p1)	{
					/* -------- scrolling up --------- */
					if (wnd->WndRow == 0)	{
						DownLine(wnd);
						StickEnd(wnd);
					}
					else
						--wnd->WndRow;
				}
				else	{
					/* -------- scrolling down --------- */
					if (wnd->WndRow == ClientHeight(wnd)-1)	{
						UpLine(wnd);
						StickEnd(wnd);
					}
					else
						wnd->WndRow++;
				}
				SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			}
			return rtn;
#endif
		case HORIZSCROLL:
			if (p1 && wnd->CurrCol == wnd->wleft &&
					*currchar == '\n')
				return FALSE;
			if ((rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2)) != FALSE)	{
				if (wnd->CurrCol < wnd->wleft)
					wnd->CurrCol++;
				else if (WndCol == ClientWidth(wnd))
					--wnd->CurrCol;
				SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			}
			return rtn;
#ifdef INCLUDE_SYSTEM_MENUS
		case MOVE:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			return rtn;
		case SIZE:
			rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
			if (WndCol > ClientWidth(wnd)-1)
				wnd->CurrCol = ClientWidth(wnd)-1 + wnd->wleft;
			if (wnd->WndRow > ClientHeight(wnd)-1)	{
				wnd->WndRow = ClientHeight(wnd)-1;
				SetLinePointer(wnd, wnd->WndRow+wnd->wtop);
			}
			SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			return rtn;
#endif
		case KEYBOARD:
#ifdef INCLUDE_SYSTEM_MENUS
			if (WindowMoving || WindowSizing)
				break;
#endif
			if ((int)p2 & ALTKEY)
				break;
			c = (int) p1;
			switch (c)	{
				case CTRL_FWD:
				case CTRL_BS:
				case CTRL_HOME:
				case CTRL_END:
					break;
				case ESC:
				case F1:
				case F2:
				case F3:
				case F4:
				case F5:
				case F6:
				case F7:
				case F8:
				case F9:
				case F10:
				case SHIFT_INS:
				case SHIFT_DEL:
					PassOn = TRUE;
				default:
					if ((int)p2 & CTRLKEY)
						PassOn = TRUE;
				break;
			}
			if (PassOn)
				break;
#ifdef INCLUDE_MULTILINE
			if (isMultiLine(wnd))	{
				if ((int)p2 & (LEFTSHIFT | RIGHTSHIFT))	{
					SendMessage(NULLWND, CURRENT_KEYBOARD_CURSOR,
						(PARAM) &kx, (PARAM) &ky);

					kx -= GetClientLeft(wnd);
					ky -= GetClientTop(wnd);

					switch (c)	{
						case HOME:
						case END:
						case PGUP:
						case PGDN:
						case UP:
						case DN:
						case FWD:
						case BS:
						case CTRL_FWD:
						case CTRL_BS:
							if (!KeyBoardMarking)	{
								if (BlockMarked(wnd))	{
									ClearBlock(wnd);
									SendMessage(wnd, PAINT, 0, 0);
								}
								KeyBoardMarking = TextMarking = TRUE;
								SetAnchor(wnd, kx+wnd->wleft, ky+wnd->wtop);
							}
							break;
						default:
							break;
					}
				}
				else if (((c != DEL && c != RUBOUT) ||
						!isMultiLine(wnd)) && BlockMarked(wnd))	{
					ClearBlock(wnd);
					SendMessage(wnd, PAINT, 0, 0);
				}
			}
#endif
			switch (c)	{
#ifdef INCLUDE_MULTILINE
				case PGUP:
				case PGDN:
					if (!isMultiLine(wnd))
						break;
					BaseWndProc(EDITBOX, wnd, msg, p1, p2);
					SetLinePointer(wnd, wnd->wtop+wnd->WndRow);
					StickEnd(wnd);
					SendMessage(wnd, KEYBOARD_CURSOR,WndCol, wnd->WndRow);
					break;
#endif
				case HOME:
					Home(wnd);
					break;
				case END:
					End(wnd);
					break;
				case CTRL_FWD:
					NextWord(wnd);
					break;
				case CTRL_BS:
					PrevWord(wnd);
					break;
				case CTRL_HOME:
					if (!isMultiLine(wnd))	{
						Home(wnd);
						break;
					}
#ifdef INCLUDE_MULTILINE
					rtn = BaseWndProc(EDITBOX, wnd, msg, HOME, p2);
					Home(wnd);
					wnd->CurrLine = 0;
					wnd->WndRow = 0;
					SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
					return rtn;
#endif
				case CTRL_END:
					if (!isMultiLine(wnd))	{
						End(wnd);
						break;
					}
#ifdef INCLUDE_MULTILINE
					Home(wnd);
					rtn = BaseWndProc(EDITBOX, wnd, msg, END, p2);
					SetLinePointer(wnd, wnd->wlines-1);
					wnd->WndRow = min(ClientHeight(wnd)-1, wnd->wlines-1);
					End(wnd);
					SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
					return rtn;
#endif
#ifdef INCLUDE_MULTILINE
				case UP:
					if (!isMultiLine(wnd))
						break;
					Upward(wnd);
					break;
				case DN:
					if (!isMultiLine(wnd))
						break;
					Downward(wnd);
					break;
#endif
				case FWD:
					Forward(wnd);
					break;
				case BS:
					Backward(wnd);
					break;
			}
#ifdef INCLUDE_MULTILINE
			if (KeyBoardMarking)	{
				SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
				SendMessage(NULLWND, CURRENT_KEYBOARD_CURSOR,
					(PARAM) &kx, (PARAM) &ky);
				SendMessage(wnd, MOUSE_MOVED, kx, ky);
				return TRUE;
			}
#endif
			if (!TestAttribute(wnd, READONLY))	{
				switch (c)	{
					case HOME:
					case END:
					case PGUP:
					case PGDN:
					case UP:
					case DN:
					case FWD:
					case BS:
					case CTRL_FWD:
					case CTRL_BS:
					case CTRL_HOME:
					case CTRL_END:
					case RUBOUT:
						if (!isMultiLine(wnd) && BlockMarked(wnd))	{
							ClearBlock(wnd);
							SendMessage(wnd, PAINT, 0, 0);
						}
						if (c != RUBOUT)
							break;
						if (wnd->CurrLine == 0 && wnd->CurrCol == 0)
							break;
						Backward(wnd);
						currchar = CurrChar;
						SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
					case DEL:
						if (BlockMarked(wnd))	{
							SendMessage(wnd, COMMAND, ID_DELETETEXT, 0);
							return TRUE;
						}
						if (*(currchar+1) == '\0')
							return TRUE;
						if (*(currchar+1))	{
							int repaint = *currchar == '\n';
							strcpy(currchar, currchar+1);
							if (repaint)	{
								BuildTextPointers(wnd);
								SendMessage(wnd, PAINT, 0, 0);
							}
							else	{
								AddTextPointers(wnd, wnd->CurrLine+1, -1);
								WriteTextLine(wnd, NULL, wnd->WndRow+wnd->wtop, FALSE);
							}
						}
						wnd->TextChanged = TRUE;
						break;
					case INS:
						InvertCommandToggle(MainMenu, ID_INSERT);
						SendMessage(NULLWND, SHOW_CURSOR, GetCommandToggle(MainMenu, ID_INSERT), 0);
						break;
					case '\r':
#ifdef INCLUDE_MULTILINE
						if (isMultiLine(wnd))
							c = '\n';
#endif
					default:
						if (c == '\t')	{
							int insmd = GetCommandToggle(MainMenu, ID_INSERT);
							if (!isMultiLine(wnd))
								PostMessage(GetParent(wnd), msg, p1, p2);
#ifdef INCLUDE_MULTILINE
							else do	{
								if (!insmd && *(CurrChar+1) == '\0')
									break;
								SendMessage(wnd, KEYBOARD,
									insmd ? ' ' : FWD, 0);
							} while (wnd->CurrCol % cfg.Tabs);
#endif
							return TRUE;
						}
						if ((c != '\n' && c < ' ') || (c & 0x1000))
							/* ---- not recognized by editor --- */
							break;
#ifdef INCLUDE_MULTILINE
						if (!isMultiLine(wnd) && BlockMarked(wnd))	{
							ResetEditBox(wnd);
							ClearBlock(wnd);
							currchar = CurrChar;
						}
#endif
						if (*currchar == '\0')	{
							*currchar = '\n';
							*(currchar+1) = '\0';
							wnd->wlines++;
						}
						/* --- displayable char or newline --- */
						if (c == '\n' ||
								GetCommandToggle(MainMenu, ID_INSERT) ||
									*currchar == '\n')	{
							if (wnd->text[wnd->textlen-1] != '\0')	{
								wnd->textlen += GROWLENGTH;
								wnd->text = realloc(wnd->text, wnd->textlen+1);
								wnd->text[wnd->textlen-1] = '\0';
								currchar = CurrChar;
							}
							/* ------ insert mode ------ */
							memmove(currchar+1, currchar, strlen(currchar)+1);
							AddTextPointers(wnd, wnd->CurrLine+1, 1);
#ifdef INCLUDE_MULTILINE
							if (isMultiLine(wnd))
								wnd->textwidth = max(wnd->textwidth,
									(int) (TextLine(wnd, wnd->CurrLine+1)-
									TextLine(wnd, wnd->CurrLine)));
							else
#endif
								wnd->textwidth = max(wnd->textwidth,
									strlen(wnd->text));
							WriteTextLine(wnd, NULL,
								wnd->wtop+wnd->WndRow, FALSE);
						}
						/* ----- put the char in the buffer ----- */
						*currchar = c;
						wnd->TextChanged = TRUE;
						if (c == '\n')	{
							wnd->wleft = 0;
							BuildTextPointers(wnd);
							End(wnd);
							Forward(wnd);
							SendMessage(wnd, PAINT, 0, 0);
							break;
						}
						/* ---------- test end of window --------- */
						if (WndCol == ClientWidth(wnd)-1)	{
							int dif;
							char *cp = currchar;
							while (*cp != ' ' && cp != TextLine(wnd, wnd->CurrLine))
								--cp;
#ifdef INCLUDE_MULTILINE
							if (!isMultiLine(wnd) || cp == TextLine(wnd, wnd->CurrLine) ||
									!GetCommandToggle(MainMenu, ID_WRAP))
#endif
								SendMessage(wnd, HORIZSCROLL, TRUE, 0);
#ifdef INCLUDE_MULTILINE
							else	{
								dif = 0;
								if (c != ' ')	{
									dif = (int) (currchar - cp);
									wnd->CurrCol -= dif;
									SendMessage(wnd, KEYBOARD, DEL, 0);
									--dif;
								}
								SendMessage(wnd, KEYBOARD, '\r', 0);
								currchar = CurrChar;
								wnd->CurrCol = dif;
								if (c == ' ')
									break;
							}
#endif
						}
						/* ------ display the character ------ */
						SetStandardColor(wnd);
						PutWindowChar(wnd, WndCol+BorderAdj(wnd),
							wnd->WndRow+TopBorderAdj(wnd), c);
						/* ----- advance the pointers ------ */
						wnd->CurrCol++;
						break;
				}
			}
			SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
			return TRUE;
#ifdef INCLUDE_MULTILINE
		case COMMAND:	{
			char *bbl, *bel, *bb;
			int len;

			if (BlockMarked(wnd))	{
				bbl = TextLine(wnd, wnd->BlkBegLine) + wnd->BlkBegCol;
				bel = TextLine(wnd, wnd->BlkEndLine) + wnd->BlkEndCol;
				len = (int) (bel - bbl);
			}
			switch ((int)p1)	{
#ifdef INCLUDE_CLIPBOARD
				case ID_CUT:
				case ID_COPY:
					ClipboardLength = len;
					Clipboard = realloc(Clipboard, ClipboardLength);
					if (Clipboard != NULL)
						memmove(Clipboard, bbl, ClipboardLength);
#endif
				case ID_DELETETEXT:
					if (p1 != ID_COPY)	{
						if (p1 != ID_CUT)
							SaveDeletedText(wnd, bbl, len);
						wnd->TextChanged = TRUE;
						strcpy(bbl, bel);
						wnd->CurrLine = TextLineNumber(wnd, bbl - wnd->BlkBegCol);
						wnd->CurrCol = wnd->BlkBegCol;
						wnd->WndRow = wnd->BlkBegLine - wnd->wtop;
						if (wnd->WndRow < 0)	{
							wnd->wtop = wnd->BlkBegLine;
							wnd->WndRow = 0;
						}
						SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
					}
					ClearBlock(wnd);
					BuildTextPointers(wnd);
					SendMessage(wnd, PAINT, 0, 0);
					return TRUE;
#ifdef INCLUDE_CLIPBOARD
				case ID_CLEAR:
					SaveDeletedText(wnd, bbl, len);
					wnd->CurrLine = TextLineNumber(wnd, bbl);
					wnd->CurrCol = wnd->BlkBegCol;
					wnd->WndRow = wnd->BlkBegLine - wnd->wtop;
					if (wnd->WndRow < 0)	{
						wnd->WndRow = 0;
						wnd->wtop = wnd->BlkBegLine;
					}
					while (bbl < bel)	{
						char *cp = strchr(bbl, '\n');
						if (cp > bel)
							cp = bel;
						strcpy(bbl, cp);
						bel -= (int) (cp - bbl);
						bbl++;
					}
					ClearBlock(wnd);
					BuildTextPointers(wnd);
					SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
					SendMessage(wnd, PAINT, 0, 0);
					wnd->TextChanged = TRUE;
					return TRUE;
				case ID_PASTE:
					if (Clipboard != NULL)	{
						PasteText(wnd, Clipboard, ClipboardLength);
						wnd->TextChanged = TRUE;
					}
					return TRUE;
#endif
				case ID_UNDO:
					if (wnd->DeletedText != NULL)	{
						PasteText(wnd, wnd->DeletedText, wnd->DeletedLength);
						free(wnd->DeletedText);
						wnd->DeletedText = NULL;
					}
					return TRUE;
				case ID_PARAGRAPH:	{
					char *bl;
					int bc, ec;
					int fl, el;
					int Blocked;

					el = wnd->BlkEndLine;
					ec = wnd->BlkEndCol;
					if (!BlockMarked(wnd))	{
						Blocked = FALSE;
						/* ---- forming paragraph from cursor position --- */
						fl = wnd->wtop + wnd->WndRow;
						bl = TextLine(wnd, wnd->CurrLine);
						bc = wnd->CurrCol;
						Home(wnd);
						bbl = bel = bl;
						if (bc >= ClientWidth(wnd))
							bc = 0;
						/* ---- locate the end of the paragraph ---- */
						while (*bel)	{
							int blank = TRUE;
							char *bll = bel;
							/* --- blank line marks end of paragraph --- */
							while (*bel && *bel != '\n')	{
								if (*bel != ' ')
									blank = FALSE;
								bel++;
							}
							if (blank)	{
								bel = bll;
								break;
							}
							if (*bel)
								bel++;
						}
						if (bel == bbl)	{
							SendMessage(wnd, KEYBOARD, DN, 0);
							return TRUE;
						}
						if (*bel == '\0')
							--bel;
						if (*bel == '\n')
							--bel;
					}
					else	{
						Blocked = TRUE;
						/* ---- forming paragraph from marked block --- */
						fl = wnd->BlkBegLine;
						bc = wnd->CurrCol = wnd->BlkBegCol;
						wnd->CurrLine = fl;
						if (fl < wnd->wtop)
							wnd->wtop = fl;
						wnd->WndRow = fl - wnd->wtop;
						SendMessage(wnd, KEYBOARD, '\r', 0);
						el++, fl++;
						if (bc != 0)	{
							SendMessage(wnd, KEYBOARD, '\r', 0);
							el++, fl ++;
						}
						bc = 0;
						bl = TextLine(wnd, fl);
						wnd->CurrLine = fl;
						bbl = bl + bc;
						bel = TextLine(wnd, el) + ec;
					}

					/* --- change all newlines in block to spaces --- */
					while (CurrChar < bel)	{
						if (*CurrChar == '\n')	{
							*CurrChar = ' ';
							wnd->CurrLine++;
							wnd->CurrCol = 0;
						}
						else
							wnd->CurrCol++;
					}

					/* ---- insert newlines at new margin boundaries ---- */
					bb = bbl;
					while (bbl < bel)	{
						bbl++;
						if ((int)(bbl - bb) == ClientWidth(wnd)-1)	{
							while (*bbl != ' ' && bbl > bb)
								--bbl;
							if (*bbl != ' ')	{
								bbl = strchr(bbl, ' ');
								if (bbl == NULL || bbl >= bel)
									break;
							}
							*bbl = '\n';
							bb = bbl+1;
						}
					}
					ec = (int)(bel - bb);
					BuildTextPointers(wnd);

					if (Blocked)	{
						/* ---- position cursor at end of new paragraph ---- */
						if (el < wnd->wtop ||
								wnd->wtop + ClientHeight(wnd) < el)
							wnd->wtop = el-ClientHeight(wnd);
						if (wnd->wtop < 0)
							wnd->wtop = 0;
						wnd->WndRow = el - wnd->wtop;
						wnd->CurrLine = el;
						wnd->CurrCol = ec;
						SendMessage(wnd, KEYBOARD, '\r', 0);
						SendMessage(wnd, KEYBOARD, '\r', 0);
					}
					else	{
						/* --- put cursor back at beginning --- */
						wnd->CurrLine = TextLineNumber(wnd, bl);
						wnd->CurrCol = bc;
						if (fl < wnd->wtop)
							wnd->wtop = fl;
						wnd->WndRow = fl - wnd->wtop;
					}
					SendMessage(wnd, PAINT, 0, 0);
					SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
					wnd->TextChanged = TRUE;
					BuildTextPointers(wnd);
					return TRUE;
				}
#ifdef INCLUDE_DIALOG_BOXES
				case ID_SEARCH:
					SearchTextBox(wnd);
					break;
				case ID_REPLACE:
					ReplaceTextBox(wnd);
					break;
				case ID_SEARCHNEXT:
					SearchNext(wnd, TRUE);
					break;
#endif
				default:
					break;
			}
			break;
		}
#endif
		case CLOSE_WINDOW:
			SendMessage(NULLWND, HIDE_CURSOR, 0, 0);
			if (wnd->DeletedText != NULL)
				free(wnd->DeletedText);
			break;
		default:
			break;
	}
	return BaseWndProc(EDITBOX, wnd, msg, p1, p2);
}

#ifdef INCLUDE_MULTILINE
static void PasteText(WINDOW wnd, char *SaveTo, int len)
{
	int plen = strlen(wnd->text) + len;
	char *bl, *el;

	if (plen > wnd->textlen)	{
		wnd->text = realloc(wnd->text, plen+1);
		wnd->textlen = plen;
	}
	bl = CurrChar;
	el = bl+len;
	memmove(el,	bl,	strlen(bl)+1);
	memmove(bl, SaveTo, len);
	BuildTextPointers(wnd);
	SendMessage(wnd, PAINT, 0, 0);
}

static void SaveDeletedText(WINDOW wnd, char *bbl, int len)
{
	wnd->DeletedLength = len;
	if ((wnd->DeletedText = realloc(wnd->DeletedText, len)) != NULL)
		memmove(wnd->DeletedText, bbl, len);
}

#endif

static void Forward(WINDOW wnd)
{
	if (*(CurrChar+1) == '\0')
		return;
#ifdef INCLUDE_MULTILINE
	if (*CurrChar == '\n')	{
		Home(wnd);
		Downward(wnd);
	}
#endif
	else	{
		wnd->CurrCol++;
		if (WndCol == ClientWidth(wnd))	{
			wnd->wleft++;
			SendMessage(wnd, PAINT, 0, 0);
		}
	}
}

static void StickEnd(WINDOW wnd)
{
	char *cp = TextLine(wnd, wnd->CurrLine);
	int len = (int) (strchr(cp, '\n') - cp);

	wnd->CurrCol = min(len, wnd->CurrCol);
	if (wnd->wleft > wnd->CurrCol)	{
		wnd->wleft = max(0, wnd->CurrCol - 4);
		SendMessage(wnd, PAINT, 0, 0);
	}
	else if (wnd->CurrCol-wnd->wleft >= ClientWidth(wnd))	{
		wnd->wleft = wnd->CurrCol - (ClientWidth(wnd)-1);
		SendMessage(wnd, PAINT, 0, 0);
	}
}

#ifdef INCLUDE_MULTILINE
static void Downward(WINDOW wnd)
{
	if (isMultiLine(wnd) && wnd->WndRow+wnd->wtop+1 < wnd->wlines)	{
		DownLine(wnd);
		if (wnd->WndRow == ClientHeight(wnd)-1)
			SendMessage(wnd, SCROLL, TRUE, 0);
		wnd->WndRow++;
		StickEnd(wnd);
	}
}

static void DownLine(WINDOW wnd)
{
	wnd->CurrLine++;
}

static void UpLine(WINDOW wnd)
{
	if (wnd->CurrLine > 0)
		--wnd->CurrLine;
}

static void Upward(WINDOW wnd)
{
	if (isMultiLine(wnd) && wnd->CurrLine != 0)	{
		UpLine(wnd);
		if (wnd->WndRow == 0)
			SendMessage(wnd, SCROLL, FALSE, 0);
		--wnd->WndRow;
		StickEnd(wnd);
	}
}
#endif

static void Backward(WINDOW wnd)
{
	if (wnd->CurrCol)	{
		if (wnd->CurrCol-- <= wnd->wleft)	{
			if (wnd->wleft != 0)	{
				--wnd->wleft;
				SendMessage(wnd, PAINT, 0, 0);
				return;
			}
		}
		else
			return ;
	}
#ifdef INCLUDE_MULTILINE
	if (isMultiLine(wnd) && wnd->CurrLine != 0)	{
		Upward(wnd);
		End(wnd);
	}
#endif
}

static void End(WINDOW wnd)
{
	while (*CurrChar != '\n')
		++wnd->CurrCol;
	if (WndCol >= ClientWidth(wnd))	{
		wnd->wleft = wnd->CurrCol - (ClientWidth(wnd)-1);
		SendMessage(wnd, PAINT, 0, 0);
	}
}

static void Home(WINDOW wnd)
{
	wnd->CurrCol = 0;
	if (wnd->wleft != 0)	{
		wnd->wleft = 0;
		SendMessage(wnd, PAINT, 0, 0);
	}
}

#define isWhite(c) 	((c) == ' ' || (c) == '\n')

static void NextWord(WINDOW wnd)
{
	int savetop = wnd->wtop;
	int saveleft = wnd->wleft;
	ClearVisible(wnd);
	while (!isWhite(*CurrChar))	{
		if (*(CurrChar+1) == '\0')
			break;
		Forward(wnd);
	}
	while (isWhite(*CurrChar))	{
		if (*(CurrChar+1) == '\0')
			break;
		Forward(wnd);
	}
	SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
	SetVisible(wnd);
	if (wnd->wtop != savetop || wnd->wleft != saveleft)
		SendMessage(wnd, PAINT, 0, 0);
}

static void PrevWord(WINDOW wnd)
{
	int savetop = wnd->wtop;
	int saveleft = wnd->wleft;
	ClearVisible(wnd);
	Backward(wnd);
	while (isWhite(*CurrChar))	{
		if (wnd->CurrLine == 0 && wnd->CurrCol == 0)
			break;
		Backward(wnd);
	}
	while (!isWhite(*CurrChar))	{
		if (wnd->CurrLine == 0 && wnd->CurrCol == 0)
			break;
		Backward(wnd);
	}
	if (isWhite(*CurrChar))
		Forward(wnd);
	if (wnd->wleft != saveleft)
		if (wnd->CurrCol >= saveleft)
			if (wnd->CurrCol - saveleft < ClientWidth(wnd))
				wnd->wleft = saveleft;
	SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
	SetVisible(wnd);
	if (wnd->wtop != savetop || wnd->wleft != saveleft)
		SendMessage(wnd, PAINT, 0, 0);
}

static void ResetEditBox(WINDOW wnd)
{
	*wnd->text = '\0';
	wnd->wlines = 0;
	wnd->CurrLine = 0;
	wnd->CurrCol = 0;
	wnd->WndRow = 0;
	wnd->TextChanged = FALSE;
	wnd->wleft = 0;
	wnd->textwidth = 0;
}

static void AddTextPointers(WINDOW wnd, int lineno, int ct)
{
	while (lineno < wnd->wlines)
		*((wnd->TextPointers) + lineno++) += ct;
}

static int TextLineNumber(WINDOW wnd, char *lp)
{
	int lineno;
	char *cp;
	for (lineno = 0; lineno < wnd->wlines; lineno++)	{
		cp = wnd->text + *((wnd->TextPointers) + lineno);
		if (cp == lp)
			return lineno;
		if (lineno && cp > lp)
			return lineno - 1;
	}
	return 0;
}

#ifdef INCLUDE_MULTILINE
#ifdef INCLUDE_DIALOG_BOXES

extern DBOX SearchText;
extern DBOX ReplaceText;
static int checkcase = TRUE;
static int Replacing = FALSE;

static void ReplaceTextBox(WINDOW wnd)
{
	if (checkcase)
		SetCheckBox(&ReplaceText, ID_MATCHCASE);
	if (DialogBox(wnd, &ReplaceText, TRUE, NULL))	{
		checkcase = CheckBoxSetting(&ReplaceText, ID_MATCHCASE);
		Replacing = TRUE;
		SearchNext(wnd, FALSE);
	}
}

static void SearchTextBox(WINDOW wnd)
{
	if (checkcase)
		SetCheckBox(&SearchText, ID_MATCHCASE);
	if (DialogBox(wnd, &SearchText, TRUE, NULL))	{
		checkcase = CheckBoxSetting(&SearchText, ID_MATCHCASE);
		Replacing = FALSE;
		SearchNext(wnd, FALSE);
	}
}

static int SearchCmp(int a, int b)
{
	if (b == '\n')
		b = ' ';
	if (checkcase)
		return a != b;
	return tolower(a) != tolower(b);
}

static void SearchNext(WINDOW wnd, int incr)
{
	char *s1, *s2, *cp1 = CurrChar;
	DBOX *db = Replacing ? &ReplaceText : &SearchText;
	char *cp = GetEditBoxText(db, ID_SEARCHFOR);
	int rpl = TRUE, FoundOne = FALSE;
	while (rpl)	{
		rpl = Replacing ?
				CheckBoxSetting(&ReplaceText, ID_REPLACEALL) : FALSE;
		if (BlockMarked(wnd))	{
			ClearBlock(wnd);
			SendMessage(wnd, PAINT, 0, 0);
		}
		if (cp && cp1 && *cp && *cp1)	{
			if (incr)
				cp1++;
			while (*cp1)	{
				s1 = cp;
				s2 = cp1;
				while (*s1 && *s1 != '\n')	{
					if (SearchCmp(*s1,*s2))
						break;
					s1++, s2++;
				}
				if (*s1 == '\0' || *s1 == '\n')
					break;
				cp1++;
			}
			if (*s1 == 0 || *s1 == '\n')	{
				/* ----- hit at *cp1 ------- */
				FoundOne = TRUE;
				wnd->BlkEndLine = TextLineNumber(wnd, s2);
				wnd->BlkEndCol = (int)(s2 - TextLine(wnd, wnd->BlkEndLine));

				wnd->BlkBegLine = TextLineNumber(wnd, cp1);
				wnd->BlkBegCol = (int)(cp1 - TextLine(wnd, wnd->BlkBegLine));

				wnd->CurrCol = wnd->BlkBegCol;
				wnd->CurrLine = wnd->BlkBegLine;
				wnd->WndRow = wnd->CurrLine - wnd->wtop;

				if (WndCol > ClientWidth(wnd)-1)
					wnd->wleft = wnd->CurrCol;
				if (wnd->WndRow > ClientHeight(wnd)-1)	{
					wnd->wtop = wnd->CurrLine;
					wnd->WndRow = 0;
				}
				SendMessage(wnd, PAINT, 0, 0);
				PostMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
				if (Replacing)	{
					if (rpl || YesNoBox("Replace the text?"))	{
						char *cr = GetEditBoxText(db, ID_REPLACEWITH);
						int oldlen = strlen(cp)-1;
						int newlen = strlen(cr)-1;
						int dif;
						if (oldlen < newlen)	{
							dif = newlen-oldlen;
							if (wnd->textlen < strlen(wnd->text)+dif)	{
								int offset = (int)(cp1-wnd->text);
								wnd->textlen += dif;
								wnd->text = realloc(wnd->text, wnd->textlen);
								if (wnd->text == NULL)
									return;
								cp1 = wnd->text + offset;
							}
							memmove(cp1+dif, cp1, strlen(cp1)+1);
						}
						else if (oldlen > newlen)	{
							dif = oldlen-newlen;
							memmove(cp1, cp1+dif, strlen(cp1)+1);
						}
						strncpy(cp1, cr, newlen);
						wnd->TextChanged = TRUE;
						BuildTextPointers(wnd);
					}
					if (rpl)	{
						incr = TRUE;
						continue;
					}
					ClearBlock(wnd);
					SendMessage(wnd, PAINT, 0, 0);
				}
				return;
			}
			break;
		}
	}
	if (cp && *cp && !FoundOne)
		MessageBox("Search/Replace Text", "No match found");
}

#endif
#endif
