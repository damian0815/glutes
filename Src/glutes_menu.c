/*
 * glutes_menu.c
 *
 * Menu management methods for Win32 based systems.
 *
 * Copyright (c) 2005 Joachim Pouderoux. All Rights Reserved.
 * Creation date: Fri Jan 28 2005
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * the Software, and to permit persons to whom the Software is furnished 
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JOACHIM POUDEROUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifndef USE_GLMENU

SFG_WinMenu *fgMappedMenu;
SFG_WinMenu *fgCurrentMenu = NULL;

SFG_Window *fgMenuWindow;
SFG_WinMenuItem *fgItemSelected;
unsigned fgMenuButton;

HMENU fgHMenu;

static SFG_WinMenu **menuList = NULL;
static int menuListSize = 0;
static UINT UniqueMenuHandler = 1;


void fgSetMenu(SFG_WinMenu * menu)
{
	fgCurrentMenu = menu;
}

static void unmapMenu(SFG_WinMenu * menu)
{
	if (menu->Cascade) 
	{
		unmapMenu(menu->Cascade);
		menu->Cascade = NULL;
	}
	menu->Anchor = NULL;
	menu->Highlighted = NULL;
}

void fgFinishMenu(SFG_Window *win, int x, int y)
{	
	unmapMenu(fgMappedMenu);
	
	if(fgState.MenuStatusCallback) 
	{
		fgSetWindow(fgMenuWindow);
		fgSetMenu(fgMappedMenu);
		
		/* Setting fgMappedMenu to NULL permits operations that
		change menus or destroy the menu window again. */
		fgMappedMenu = NULL;
		
		fgState.MenuStatusCallback(GLUT_MENU_NOT_IN_USE, x, y);
	}
	/* Setting fgMappedMenu to NULL permits operations that
	change menus or destroy the menu window again. */
	fgMappedMenu = NULL;
	
	/* If an item is selected and it is not a submenu trigger,
	generate menu callback. */
	if(fgItemSelected && !fgItemSelected->IsTrigger) 
	{
		fgSetWindow(fgMenuWindow);
		/* When menu callback is triggered, current menu should be
		set to the callback menu. */
		fgSetMenu(fgItemSelected->Menu);
		fgItemSelected->Menu->Select(fgItemSelected->Value);
	}
	fgMenuWindow = NULL;
}

void fgStartMenu(SFG_WinMenu * menu, SFG_Window * window,
			int x, int y, int x_win, int y_win)
{
	assert(fgMappedMenu == NULL);
	fgMappedMenu = menu;
	fgMenuWindow = window;
	fgItemSelected = NULL;
	if(fgState.MenuStatusCallback) 
	{
		fgSetMenu(menu);
		fgSetWindow(window);
		fgState.MenuStatusCallback(GLUT_MENU_IN_USE, x_win, y_win);
	}


	eglSwapBuffers(fgDisplay.eglDisplay, window->Window.Surface);
	RedrawWindow(window->Window.Handle, NULL, NULL, RDW_INTERNALPAINT);

#ifndef _WIN32_WCE
	TrackPopupMenu(menu->Handle, TPM_LEFTALIGN 
		| (fgMenuButton == TPM_RIGHTBUTTON) ? TPM_RIGHTBUTTON : TPM_LEFTBUTTON, 
		x, y, 0, fgStructure.Window->Window.Handle, NULL);
#else
	TrackPopupMenu(menu->Handle, TPM_LEFTALIGN, 
		x, y, 0, fgStructure.Window->Window.Handle, NULL);
#endif
	
}

SFG_WinMenuItem * fgGetUniqueMenuItem(SFG_WinMenu * menu, UINT unique)
{
	SFG_WinMenuItem *item;
	int i;
	
	i = menu->Num;
	item = menu->List;
	while (item) 
	{
		if (item->Unique == unique)
			return item;
	
		if (item->IsTrigger) 
		{
			SFG_WinMenuItem *subitem;
			subitem = fgGetUniqueMenuItem(menuList[item->Value], unique);
			if (subitem)
				return subitem;
		}
		i--;
		item = item->Next;
	}
	return NULL;
}

SFG_WinMenuItem * fgGetMenuItem(SFG_WinMenu * menu, HMENU win, int *which)
{
	SFG_WinMenuItem *item;
	int i;
	
	i = menu->Num;
	item = menu->List;
	while (item) 
	{
		if (item->Handle == win) 
		{
			*which = i;
			return item;
		}
		if (item->IsTrigger) 
		{
			SFG_WinMenuItem *subitem;
			
			subitem = fgGetMenuItem(menuList[item->Value],
				win, which);
			if (subitem)
				return subitem;
		}
		i--;
		item = item->Next;
	}
	return NULL;
}

SFG_WinMenu * fgGetMenu(HMENU win)
{
	SFG_WinMenu *menu = fgMappedMenu;
	while (menu) 
	{
		if (win == menu->Handle) 
			return menu;
		menu = menu->Cascade;
	}
	return NULL;
}

SFG_WinMenu * fgGetMenuByNum(int menunum)
{
	if (menunum < 1 || menunum > menuListSize)
		return NULL;
	return menuList[menunum - 1];
}

static int getUnusedMenuSlot(void)
{
	int i;
	
	/* Look for allocated, unused slot. */
	for (i = 0; i < menuListSize; i++) 
	{
		if (!menuList[i])
			return i;
	}
	/* Allocate a new slot. */
	menuListSize++;
	if (menuList) 
		menuList = (SFG_WinMenu **)	realloc(menuList, menuListSize * sizeof(SFG_WinMenu *));
	else 
		menuList = (SFG_WinMenu **) malloc(sizeof(SFG_WinMenu *));

	if (!menuList) 
		fgError("out of memory.");

	menuList[menuListSize - 1] = NULL;
	return menuListSize - 1;
}

static void menuModificationError(void)
{
	/* XXX Remove the warning after GLUT 3.0. */
	fgWarning("The following is a new check for GLUT 3.0; update your code.");
	fgError("menu manipulation not allowed while menus in use.");
}

int FGAPIENTRY glutCreateMenu(FGCBMenu selectFunc)
{
	SFG_WinMenu *menu;
	int menuid;
	
	if (fgMappedMenu) 
		menuModificationError();
	
	menuid = getUnusedMenuSlot();
	menu = (SFG_WinMenu *) malloc(sizeof(SFG_WinMenu));
	if (!menu)
		fgError("out of memory.");

	menu->ID = menuid;
	menu->Num = 0;
	menu->SubMenus = 0;
	menu->Select = selectFunc;
	menu->List = NULL;
	menu->Cascade = NULL;
	menu->Highlighted = NULL;
	menu->Anchor = NULL;
	menu->Handle = CreatePopupMenu();

	menuList[menuid] = menu;
	fgSetMenu(menu);
	return menuid + 1;
}

int FGAPIENTRY fgCreateMenuWithExit(FGCBMenu selectFunc, void (__cdecl *exitfunc)(int))
{
	//fgExitFunc = exitfunc;
	return glutCreateMenu(selectFunc);
}

void FGAPIENTRY glutDestroyMenu(int menunum)
{
	SFG_WinMenu *menu = fgGetMenuByNum(menunum);
	SFG_WinMenuItem *item, *next;
	
	if (fgMappedMenu) 
		menuModificationError();

	assert(menu->ID == menunum - 1);
	DestroyMenu(menu->Handle);
	menuList[menunum - 1] = NULL;
	/* free all menu entries */
	item = menu->List;
	while (item) 
	{
		assert(item->Menu == menu);
		next = item->Next;
		free(item->Label);
		free(item);
		item = next;
	}
	if (fgCurrentMenu == menu)
		fgCurrentMenu = NULL;

	free(menu);
}

int FGAPIENTRY glutGetMenu(void)
{
	if (fgCurrentMenu) 
		return fgCurrentMenu->ID + 1;
	return 0;
}

void FGAPIENTRY glutSetMenu(int menuid)
{
	SFG_WinMenu *menu;
	
	if (menuid < 1 || menuid > menuListSize) {
		fgWarning("glutSetMenu attempted on bogus menu.");
		return;
	}
	menu = menuList[menuid - 1];
	if (!menu) {
		fgWarning("glutSetMenu attempted on bogus menu.");
		return;
	}
	fgSetMenu(menu);
}

static void setMenuItem(SFG_WinMenuItem * item, const char *label,
			int value, int IsTrigger)
{
	SFG_WinMenu *menu;
#ifdef UNICODE
	WCHAR *wlabel;
#endif
	
	menu = item->Menu;
	item->Label = _strdup(label);
	if (!item->Label)
		fgError("out of memory.");
	
	item->IsTrigger = IsTrigger;
	item->Len = (int) strlen(label);
	item->Value = value;
	item->Unique = UniqueMenuHandler++;
	
#ifdef UNICODE
	wlabel = (WCHAR*)malloc((item->Len + 1) * sizeof(WCHAR));
	mbstowcs(wlabel, label, item->Len + 1);
#endif
	
	if (IsTrigger) 
		AppendMenu(menu->Handle, MF_POPUP, (UINT)item->Handle, 
#ifdef UNICODE
		wlabel
#else
		label
#endif
		);
	else 
		AppendMenu(menu->Handle, MF_STRING, item->Unique,
#ifdef UNICODE
		wlabel
#else
		label
#endif
		);
#ifdef UNICODE
	free(wlabel);
#endif
	
}

void FGAPIENTRY glutAddMenuEntry(const char *label, int value)
{
	SFG_WinMenuItem *entry;
	
	if (fgMappedMenu)
		menuModificationError();
	
	entry = (SFG_WinMenuItem *) malloc(sizeof(SFG_WinMenuItem));
	if (!entry)
		fgError("out of memory.");
	
	entry->Menu = fgCurrentMenu;
	setMenuItem(entry, label, value, FALSE);
	fgCurrentMenu->Num++;
	entry->Next = fgCurrentMenu->List;
	fgCurrentMenu->List = entry;
}

void FGAPIENTRY glutAddSubMenu(const char *label, int menu)
{
	SFG_WinMenuItem *submenu;
	SFG_WinMenu     *popupmenu;
	
	if (fgMappedMenu) 
		menuModificationError();
	
	submenu = (SFG_WinMenuItem *) malloc(sizeof(SFG_WinMenuItem));
	if (!submenu)
		fgError("out of memory.");
	
	fgCurrentMenu->SubMenus++;
	submenu->Menu = fgCurrentMenu;
	popupmenu = fgGetMenuByNum(menu);
	if (popupmenu)
		submenu->Handle = popupmenu->Handle;
	
	setMenuItem(submenu, label, /* base 0 */ menu - 1, TRUE);
	fgCurrentMenu->Num++;
	submenu->Next = fgCurrentMenu->List;
	fgCurrentMenu->List = submenu;
}

void FGAPIENTRY glutChangeToMenuEntry(int num, const char *label, int value)
{
	SFG_WinMenuItem *item;
	int i;
#ifdef UNICODE
	WCHAR *wlabel;
#endif
	if (fgMappedMenu)
		menuModificationError();
	
	i = fgCurrentMenu->Num;
	item = fgCurrentMenu->List;
	while (item) 
	{
		if (i == num) 
		{
			if (item->IsTrigger) 
			{
			/* If changing a submenu trigger to a menu entry, we
				need to account for submenus.  */
				item->Menu->SubMenus--;
			}
			
			free(item->Label);
			item->Label = _strdup(label);
			if (!item->Label) 
				fgError("out of memory");
			item->IsTrigger = FALSE;
			item->Len = (int) strlen(label);
			item->Value = value;
			item->Unique = UniqueMenuHandler++;
			
#ifdef UNICODE
			wlabel = (WCHAR*)malloc((item->Len + 1) * sizeof(WCHAR));
			mbstowcs(wlabel, label, item->Len + 1);
#endif
			RemoveMenu(fgCurrentMenu->Handle, (UINT) i - 1, MF_BYPOSITION);
			InsertMenu(fgCurrentMenu->Handle, (UINT) i - 1, MF_BYPOSITION | MFT_STRING, item->Unique, 
#ifdef UNICODE
				wlabel
#else
				label
#endif
				);
#ifdef UNICODE
			free(wlabel);
#endif
			
			return;
		}
		i--;
		item = item->Next;
	}
	fgWarning("Current menu has no %d item.", num);
}

void FGAPIENTRY glutChangeToSubMenu(int num, const char *label, int menu)
{
	SFG_WinMenu *popupmenu;
	SFG_WinMenuItem *item;
	int i;
#ifdef UNICODE
	WCHAR *wlabel;
#endif
	
	if (fgMappedMenu)
		menuModificationError();
	
	i = fgCurrentMenu->Num;
	item = fgCurrentMenu->List;
	while (item) 
	{
		if (i == num) 
		{
			if (!item->IsTrigger) 
			{
			/* If changing a menu entry to as submenu trigger, we
				need to account for submenus.  */
				item->Menu->SubMenus++;
			}
			free(item->Label);
			
			item->Label = _strdup(label);
			if (!item->Label)
				fgError("out of memory");
			item->IsTrigger = TRUE;
			item->Len = (int) strlen(label);
			item->Value = menu - 1;
			item->Unique = UniqueMenuHandler++;
			popupmenu = fgGetMenuByNum(menu);
			if (popupmenu)
				item->Handle = popupmenu->Handle;
#ifdef UNICODE
			wlabel = (WCHAR*)malloc((item->Len + 1) * sizeof(WCHAR));
			mbstowcs(wlabel, label, item->Len + 1);
#endif			
			RemoveMenu(fgCurrentMenu->Handle, (UINT) i - 1, MF_BYPOSITION);
			InsertMenu(fgCurrentMenu->Handle, (UINT) i - 1, MF_BYPOSITION | MFT_STRING | MF_POPUP, (UINT)item->Handle, 
#ifdef UNICODE
				wlabel
#else
				label
#endif
				);
#ifdef UNICODE
			free(wlabel);
#endif
			
			return;
		}
		i--;
		item = item->Next;
	}
	fgWarning("Current menu has no %d item.", num);
}

void FGAPIENTRY glutRemoveMenuItem(int num)
{
	SFG_WinMenuItem *item, **prev;
	int i;
	
	if (fgMappedMenu)
		menuModificationError();
	
	i = fgCurrentMenu->Num;
	prev = &fgCurrentMenu->List;
	item = fgCurrentMenu->List;
	while (item) 
	{
		if (i == num) 
		{
			/* Found the menu item in list to remove. */
			fgCurrentMenu->Num--;
			
			/* Patch up menu's item list. */
			*prev = item->Next;
			
			RemoveMenu(fgCurrentMenu->Handle, (UINT) i - 1, MF_BYPOSITION);
			
			free(item->Label);
			free(item);
			return;
		}
		i--;
		prev = &item->Next;
		item = item->Next;
	}
	fgWarning("Current menu has no %d item.", num);
}

void FGAPIENTRY glutAttachMenu(int button)
{
/*	if (fgStructure.Window->Window.Handle == fgGameModeWindow) 
	{
		fgWarning("cannot attach menus in game mode.");
		return;
	}*/
	if (fgMappedMenu)
		menuModificationError();
	
	if (fgStructure.Window->Menu[button] < 1)
		fgStructure.Window->ButtonUses++;
	
	fgStructure.Window->Menu[button] = fgCurrentMenu->ID + 1;
}

void FGAPIENTRY glutDetachMenu(int button)
{
	if (fgMappedMenu)
		menuModificationError();
	
	if (fgStructure.Window->Menu[button] > 0) 
	{
		fgStructure.Window->ButtonUses--;
		fgStructure.Window->Menu[button] = 0;
	}
}

/*
 * Set and retrieve the menu's user data
 */
void* FGAPIENTRY glutGetMenuData( void )
{
	if (fgCurrentMenu)
		return fgCurrentMenu->UserData;
	return NULL;
}

void FGAPIENTRY glutSetMenuData(void* data)
{
    if (fgCurrentMenu)
		fgCurrentMenu->UserData = data;
}

#endif // USE_GLMENU