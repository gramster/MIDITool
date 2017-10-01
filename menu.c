/* ------------- menu.c ------------- */

#include <stdio.h>
#include "dflat.h"

static struct PopDown *FindCmd(MENU *mn, int cmd)
{
	while (mn->Title != NULL)	{
		struct PopDown *pd = mn->Selections;
		while (pd->SelectionTitle != NULL)	{
			if (pd->ActionId == cmd)
				return pd;
			pd++;
		}
		mn++;
	}
	return NULL;
}

void ActivateCommand(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		pd->Attrib &= ~INACTIVE;
}

void DeactivateCommand(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		pd->Attrib |= INACTIVE;
}

int isActive(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		return !(pd->Attrib & INACTIVE);
	return FALSE;
}

int GetCommandToggle(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		return (pd->Attrib & CHECKED) != 0;
	return FALSE;
}

void SetCommandToggle(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		pd->Attrib |= CHECKED;
}

void ClearCommandToggle(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		pd->Attrib &= ~CHECKED;
}

void InvertCommandToggle(MENU *mn, int cmd)
{
	struct PopDown *pd = FindCmd(mn, cmd);
	if (pd != NULL)
		pd->Attrib ^= CHECKED;
}
