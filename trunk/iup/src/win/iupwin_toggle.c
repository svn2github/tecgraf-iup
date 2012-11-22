/** \file
 * \brief Toggle Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_toggle.h"
#include "iup_drv.h"
#include "iup_image.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_draw.h"


void iupdrvToggleAddCheckBox(int *x, int *y)
{
  (*x) += 16+8;
  if (!iupwin_comctl32ver6)
    (*x) += 4;
  if ((*y) < 16) (*y) = 16; /* minimum height */
}

static int winToggleGetCheck(Ihandle* ih)
{
  if (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && ih->data->flat)
    return iupAttribGet(ih, "_IUPWIN_TOGGLE_CHECK")!=NULL? BST_CHECKED: 0;
  else
    return (int)SendMessage(ih->handle, BM_GETCHECK, 0, 0L);
}

static void winToggleSetCheck(Ihandle* ih, int check)
{
  if (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && ih->data->flat)
  {
    if (check == BST_CHECKED)
      iupAttribSetStr(ih, "_IUPWIN_TOGGLE_CHECK", "1");
    else
      iupAttribSetStr(ih, "_IUPWIN_TOGGLE_CHECK", NULL);

    iupdrvRedrawNow(ih);
  }
  else
    SendMessage(ih->handle, BM_SETCHECK, check, 0L);
}

static int winToggleIsActive(Ihandle* ih)
{
  /* called only when (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat) */
  return iupAttribGetInt(ih, "_IUPWIN_ACTIVE");
}

static void winToggleSetBitmap(Ihandle* ih, const char* name, int make_inactive)
{
  /* called only when (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat) */
  if (name)
  {
    HBITMAP bitmap = iupImageGetImage(name, ih, make_inactive);
    SendMessage(ih->handle, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmap);
  }
  else
    SendMessage(ih->handle, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)NULL);  /* if not defined */
}

static void winToggleUpdateImage(Ihandle* ih, int active, int check)
{
  /* called only when (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat) */
  char* name;

  if (!active)
  {
    name = iupAttribGet(ih, "IMINACTIVE");
    if (name)
      winToggleSetBitmap(ih, name, 0);
    else
    {
      /* if not defined then automaticaly create one based on IMAGE */
      name = iupAttribGet(ih, "IMAGE");
      winToggleSetBitmap(ih, name, 1); /* make_inactive */
    }
  }
  else
  {
    /* must restore the normal image */
    if (check)
    {
      name = iupAttribGet(ih, "IMPRESS");
      if (name)
        winToggleSetBitmap(ih, name, 0);
      else
      {
        /* if not defined then automaticaly create one based on IMAGE */
        name = iupAttribGet(ih, "IMAGE");
        winToggleSetBitmap(ih, name, 0);
      }
    }
    else
    {
      name = iupAttribGet(ih, "IMAGE");
      if (name)
        winToggleSetBitmap(ih, name, 0);
    }
  }
}

static void winToggleGetAlignment(Ihandle* ih, int *horiz_alignment, int *vert_alignment)
{
  char value1[30]="", value2[30]="";

  iupStrToStrStr(iupAttribGetStr(ih, "ALIGNMENT"), value1, value2, ':');

  if (iupStrEqualNoCase(value1, "ARIGHT"))
    *horiz_alignment = IUP_ALIGN_ARIGHT;
  else if (iupStrEqualNoCase(value1, "ALEFT"))
    *horiz_alignment = IUP_ALIGN_ALEFT;
  else /* "ACENTER" */
    *horiz_alignment = IUP_ALIGN_ACENTER;

  if (iupStrEqualNoCase(value2, "ABOTTOM"))
    *vert_alignment = IUP_ALIGN_ABOTTOM;
  else if (iupStrEqualNoCase(value2, "ATOP"))
    *vert_alignment = IUP_ALIGN_ATOP;
  else /* "ACENTER" */
    *vert_alignment = IUP_ALIGN_ACENTER;
}

static void winToggleDrawImage(Ihandle* ih, HDC hDC, int rect_width, int rect_height, int border, UINT itemState)
{
  /* called only when (ih->data->type==IUP_TOGGLE_IMAGE && (iupwin_comctl32ver6 || ih->data->flat)) */
  int xpad = ih->data->horiz_padding + border, 
      ypad = ih->data->vert_padding + border;
  int horiz_alignment, vert_alignment;
  int x, y, width, height, bpp;
  HBITMAP hBitmap, hMask = NULL;
  char *name;
  int make_inactive = 0;

  if (itemState & ODS_DISABLED)
  {
    name = iupAttribGet(ih, "IMINACTIVE");
    if (!name)
    {
      name = iupAttribGet(ih, "IMAGE");
      make_inactive = 1;
    }
  }
  else
  {
    name = iupAttribGet(ih, "IMPRESS");
    if (!(itemState & ODS_SELECTED && name))
      name = iupAttribGet(ih, "IMAGE");
  }

  hBitmap = iupImageGetImage(name, ih, make_inactive);
  if (!hBitmap)
    return;

  /* must use this info, since image can be a driver image loaded from resources */
  iupdrvImageGetInfo(hBitmap, &width, &height, &bpp);

  winToggleGetAlignment(ih, &horiz_alignment, &vert_alignment);
  if (horiz_alignment == IUP_ALIGN_ARIGHT)
    x = rect_width - (width + 2*xpad);
  else if (horiz_alignment == IUP_ALIGN_ACENTER)
    x = (rect_width - (width + 2*xpad))/2;
  else  /* ALEFT */
    x = 0;

  if (vert_alignment == IUP_ALIGN_ABOTTOM)
    y = rect_height - (height + 2*ypad);
  else if (vert_alignment == IUP_ALIGN_ATOP)
    y = 0;
  else  /* ACENTER */
    y = (rect_height - (height + 2*ypad))/2;

  x += xpad;
  y += ypad;

  if (itemState & ODS_SELECTED && !iupwin_comctl32ver6)
  {
    if (iupAttribGet(ih, "_IUPWIN_PRESSED"))
    {
      x++;
      y++;
      iupAttribSetStr(ih, "_IUPWIN_PRESSED", NULL);
    }
  }

  if (bpp == 8)
    hMask = iupdrvImageCreateMask(IupGetHandle(name));

  iupwinDrawBitmap(hDC, hBitmap, hMask, x, y, width, height, bpp);

  if (hMask)
    DeleteObject(hMask);
}

static void winToggleDrawItem(Ihandle* ih, DRAWITEMSTRUCT *drawitem)
{ 
  /* called only when (ih->data->type==IUP_TOGGLE_IMAGE && (iupwin_comctl32ver6 || ih->data->flat)) */
  int width, height, border = 4, check, draw_border;
  HDC hDC;
  iupwinBitmapDC bmpDC;

  width = drawitem->rcItem.right - drawitem->rcItem.left;
  height = drawitem->rcItem.bottom - drawitem->rcItem.top;

  hDC = iupwinDrawCreateBitmapDC(&bmpDC, drawitem->hDC, 0, 0, width, height);

  iupwinDrawParentBackground(ih, hDC, &drawitem->rcItem);

  if (!iupwin_comctl32ver6)
  {
    if (drawitem->itemState&ODS_SELECTED)
      iupAttribSetStr(ih, "_IUPWIN_PRESSED", "1");
  }

  check = winToggleGetCheck(ih);
  if (check)
    drawitem->itemState |= ODS_SELECTED;
  else
    drawitem->itemState |= ODS_DEFAULT;  /* use default mark for NOT checked */

  if (!check && ih->data->flat)
  {
    if (drawitem->itemState & ODS_HOTLIGHT || iupAttribGet(ih, "_IUPWINTOG_ENTERWIN"))
      draw_border = 1;
    else
      draw_border = 0;
  }
  else
    draw_border = 1; /* when checked, even if flat thr border is drawn */

  if (draw_border)
    iupwinDrawButtonBorder(ih->handle, hDC, &drawitem->rcItem, drawitem->itemState);

  winToggleDrawImage(ih, hDC, width, height, border, drawitem->itemState);

  if (drawitem->itemState & ODS_FOCUS)
  {
    border--;
    iupdrvDrawFocusRect(ih, hDC, border, border, width-2*border, height-2*border);
  }

  iupwinDrawDestroyBitmapDC(&bmpDC);
}


/***********************************************************************************************/


static int winToggleSetImageAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type==IUP_TOGGLE_IMAGE)
  {
    if (value != iupAttribGet(ih, "IMAGE"))
      iupAttribSetStr(ih, "IMAGE", (char*)value);

    if (iupwin_comctl32ver6 || ih->data->flat)
      iupdrvRedrawNow(ih);
    else
    {
      int check = SendMessage(ih->handle, BM_GETCHECK, 0L, 0L);
      winToggleUpdateImage(ih, winToggleIsActive(ih), check);
    }
    return 1;
  }
  else
    return 0;
}

static int winToggleSetImInactiveAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type==IUP_TOGGLE_IMAGE)
  {
    if (value != iupAttribGet(ih, "IMINACTIVE"))
      iupAttribSetStr(ih, "IMINACTIVE", (char*)value);

    if (iupwin_comctl32ver6 || ih->data->flat)
      iupdrvRedrawNow(ih);
    else
    {
      int check = SendMessage(ih->handle, BM_GETCHECK, 0L, 0L);
      winToggleUpdateImage(ih, winToggleIsActive(ih), check);
    }
    return 1;
  }
  else
    return 0;
}

static int winToggleSetImPressAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type==IUP_TOGGLE_IMAGE)
  {
    if (value != iupAttribGet(ih, "IMPRESS"))
      iupAttribSetStr(ih, "IMPRESS", (char*)value);

    if (iupwin_comctl32ver6 || ih->data->flat)
      iupdrvRedrawNow(ih);
    else
    {
      int check = SendMessage(ih->handle, BM_GETCHECK, 0L, 0L);
      winToggleUpdateImage(ih, winToggleIsActive(ih), check);
    }
    return 1;
  }
  else
    return 0;
}

static int winToggleSetValueAttrib(Ihandle* ih, const char* value)
{
  Ihandle *radio;
  int check;

  if (iupStrEqualNoCase(value,"TOGGLE"))
    check = -1;
  else if (iupStrEqualNoCase(value,"NOTDEF"))
    check = BST_INDETERMINATE;
  else if (iupStrBoolean(value))
    check = BST_CHECKED;
  else
    check = BST_UNCHECKED;

  /* This is necessary because Windows does not handle the radio state 
     when a toggle is programatically changed. */
  radio = iupRadioFindToggleParent(ih);
  if (radio)
  {
    Ihandle* last_tg;

    int oldcheck = winToggleGetCheck(ih);
    if (check == -1)
      check = BST_CHECKED;

    last_tg = (Ihandle*)iupAttribGet(radio, "_IUPWIN_LASTTOGGLE");
    if (check)
    {
      if (iupObjectCheck(last_tg) && last_tg != ih)
          winToggleSetCheck(last_tg, BST_UNCHECKED);
      iupAttribSetStr(radio, "_IUPWIN_LASTTOGGLE", (char*)ih);
    }

    if (last_tg != ih && oldcheck != check)
      winToggleSetCheck(ih, check);
  }
  else
  {
    if (check == -1)
    {
      int oldcheck = winToggleGetCheck(ih);
      if (oldcheck)
        check = BST_UNCHECKED;
      else
        check = BST_CHECKED;
    }

    winToggleSetCheck(ih, check);
  }

  if (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat)
    winToggleUpdateImage(ih, winToggleIsActive(ih), check);

  return 0;
}

static char* winToggleGetValueAttrib(Ihandle* ih)
{
  int check = winToggleGetCheck(ih);
  if (check == BST_INDETERMINATE)
    return "NOTDEF";
  else if (check == BST_CHECKED)
    return "ON";
  else
    return "OFF";
}

static int winToggleSetActiveAttrib(Ihandle* ih, const char* value)
{
  /* update the inactive image if necessary */
  if (ih->data->type==IUP_TOGGLE_IMAGE)
  {
    if (iupwin_comctl32ver6 || ih->data->flat)
    {
      iupBaseSetActiveAttrib(ih, value);
      iupdrvRedrawNow(ih);
      return 0;
    }
    else
    {
      int active = iupStrBoolean(value);
      int check = SendMessage(ih->handle, BM_GETCHECK, 0, 0L);
      if (active)
        iupAttribSetStr(ih, "_IUPWIN_ACTIVE", "YES");
      else
        iupAttribSetStr(ih, "_IUPWIN_ACTIVE", "NO");
      winToggleUpdateImage(ih, active, check);
      return 0;
    }
  }

  return iupBaseSetActiveAttrib(ih, value);
}

static char* winToggleGetActiveAttrib(Ihandle* ih)
{
  if (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat)
    return iupAttribGet(ih, "_IUPWIN_ACTIVE");
  else
    return iupBaseGetActiveAttrib(ih);
}

static int winToggleSetTitleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_TOGGLE_TEXT)
  {
    iupwinSetMnemonicTitle(ih, 0, value);
    return iupdrvBaseSetTitleAttrib(ih, value);
  }
  return 0;
}

static int winToggleSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');

  if (ih->handle && iupwin_comctl32ver6 && ih->data->type==IUP_TOGGLE_IMAGE)
    iupdrvRedrawNow(ih);

  return 0;
}

static int winToggleSetUpdateAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  if (ih->handle)
    iupdrvPostRedraw(ih);  /* Post a redraw */
  return 1;
}

static int winToggleSetBgColorAttrib(Ihandle* ih, const char* value)
{
  (void)value;
  if (ih->data->type==IUP_TOGGLE_IMAGE)
  {
    /* update internal image cache for controls that have the IMAGE attribute */
    iupAttribSetStr(ih, "BGCOLOR", value);
    iupImageUpdateParent(ih);
    iupdrvRedrawNow(ih);
  }
  return 1;
}

static char* winToggleGetBgColorAttrib(Ihandle* ih)
{
  /* the most important use of this is to provide
     the correct background for images */
  if (iupwin_comctl32ver6 && ih->data->type==IUP_TOGGLE_IMAGE)
  {
    COLORREF cr;
    if (iupwinDrawGetThemeButtonBgColor(ih->handle, &cr))
    {
      char* str = iupStrGetMemory(20);
      sprintf(str, "%d %d %d", (int)GetRValue(cr), (int)GetGValue(cr), (int)GetBValue(cr));
      return str;
    }
  }

  if (ih->data->type == IUP_TOGGLE_TEXT)
    return iupBaseNativeParentGetBgColorAttrib(ih);
  else
    return IupGetGlobal("DLGBGCOLOR");
}


/****************************************************************************************/


static int winToggleCtlColor(Ihandle* ih, HDC hdc, LRESULT *result)
{
  COLORREF cr;

  SetBkMode(hdc, TRANSPARENT);

  if (iupwinGetColorRef(ih, "FGCOLOR", &cr))
    SetTextColor(hdc, cr);

  if (iupwinGetParentBgColor(ih, &cr))
  {
    SetDCBrushColor(hdc, cr);
    *result = (LRESULT)GetStockObject(DC_BRUSH);
    return 1;
  }
  return 0;
}

static int winToggleImageWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  /* called only when (ih->data->type==IUP_TOGGLE_IMAGE && iupwin_comctl32ver6) */

  if (msg_info->code == NM_CUSTOMDRAW)
  {
    NMCUSTOMDRAW *customdraw = (NMCUSTOMDRAW*)msg_info;

    if (customdraw->dwDrawStage==CDDS_PREERASE)
    {
      DRAWITEMSTRUCT drawitem;
      drawitem.itemState = 0;

      if (customdraw->uItemState & CDIS_DISABLED)
        drawitem.itemState |= ODS_DISABLED;
      else if (customdraw->uItemState & CDIS_SELECTED)
        drawitem.itemState |= ODS_SELECTED;
      else if (customdraw->uItemState & CDIS_HOT)
        drawitem.itemState |= ODS_HOTLIGHT;
      else if (customdraw->uItemState & CDIS_DEFAULT)
        drawitem.itemState |= ODS_DEFAULT;

      if (customdraw->uItemState & CDIS_FOCUS && (customdraw->uItemState & CDIS_SHOWKEYBOARDCUES))
        drawitem.itemState |= ODS_FOCUS;

      drawitem.hDC = customdraw->hdc;
      drawitem.rcItem = customdraw->rc;

      winToggleDrawItem(ih, (void*)&drawitem);  /* Simulate a WM_DRAWITEM */

      *result = CDRF_SKIPDEFAULT;
      return 1;
    }
  }

  return 0; /* result not used */
}

static int winToggleImageFlatProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  /* Called only when (ih->data->type==IUP_TOGGLE_IMAGE && ih->data->flat) */

  switch (msg)
  {
  case WM_MOUSELEAVE:
    iupAttribSetStr(ih, "_IUPWINTOG_ENTERWIN", NULL);
    iupdrvRedrawNow(ih);
    break;
  case WM_MOUSEMOVE:
    if (!iupAttribGet(ih, "_IUPWINTOG_ENTERWIN"))
    {
      iupAttribSetStr(ih, "_IUPWINTOG_ENTERWIN", "1");
      iupdrvRedrawNow(ih);
    }
    break;
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}

static int winToggleImageOldProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  /* Called only when (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat) */

  switch (msg)
  {
  case WM_MOUSEACTIVATE:
    if (!winToggleIsActive(ih))
    {
      *result = MA_NOACTIVATEANDEAT;
      return 1;
    }
    break;
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_ACTIVATE:
  case WM_SETFOCUS:
    if (!winToggleIsActive(ih)) 
    {
      *result = 0;
      return 1;
    }
    break;
  }

  if (msg == WM_LBUTTONDOWN)
    winToggleUpdateImage(ih, 1, 1);
  else if (msg == WM_LBUTTONUP)
    winToggleUpdateImage(ih, 1, 0);

  return iupwinBaseProc(ih, msg, wp, lp, result);
}

static int winToggleWmCommand(Ihandle* ih, WPARAM wp, LPARAM lp)
{
  (void)lp;

  switch (HIWORD(wp))
  {
  case BN_DOUBLECLICKED:
  case BN_CLICKED:
    {
      Ihandle *radio;
      IFni cb;
      int check = winToggleGetCheck(ih);

      if (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && !ih->data->flat)
      {
        int active = winToggleIsActive(ih);
        winToggleUpdateImage(ih, active, check);
        if (!active)
          return 0;
      }

      radio = iupRadioFindToggleParent(ih);
      if (radio)
      {
        /* This is necessary because Windows does not send a message
           when a toggle is unchecked in a Radio. 
           Also if the toggle is already checked in a radio, 
           a click will call the callback again. */

        Ihandle* last_tg = (Ihandle*)iupAttribGet(radio, "_IUPWIN_LASTTOGGLE");
        if (iupObjectCheck(last_tg) && last_tg != ih)
        {
          /* uncheck last toggle */
          winToggleSetCheck(last_tg, BST_UNCHECKED);

          cb = (IFni) IupGetCallback(last_tg, "ACTION");
          if (cb && cb(last_tg, 0) == IUP_CLOSE)
              IupExitLoop();

          if (iupObjectCheck(last_tg))
            iupBaseCallValueChangedCb(last_tg);
        }
        iupAttribSetStr(radio, "_IUPWIN_LASTTOGGLE", (char*)ih);

        if (last_tg != ih)
        {
          /* check new toggle */
          winToggleSetCheck(ih, BST_CHECKED);

          cb = (IFni)IupGetCallback(ih, "ACTION");
          if (cb && cb (ih, 1) == IUP_CLOSE)
              IupExitLoop();

          if (iupObjectCheck(ih))
            iupBaseCallValueChangedCb(ih);
        }
      }
      else
      {
        if (ih->data->type==IUP_TOGGLE_IMAGE && !iupwin_comctl32ver6 && ih->data->flat)
        {
          /* toggle the value manually */
          check = check? 0: BST_CHECKED;
          winToggleSetCheck(ih, check);
        }

        if (check == BST_INDETERMINATE)
          check = -1;

        cb = (IFni)IupGetCallback(ih, "ACTION");
        if (cb && cb (ih, check) == IUP_CLOSE)
            IupExitLoop();

        if (iupObjectCheck(ih))
          iupBaseCallValueChangedCb(ih);
      }
    }
  }

  return 0; /* not used */
}

static int winToggleMapMethod(Ihandle* ih)
{
  Ihandle* radio = iupRadioFindToggleParent(ih);
  char* value;
  int ownerdraw = 0;
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS |
                  BS_NOTIFY; /* necessary because of the base messages */

  if (!ih->parent)
    return IUP_ERROR;

  if (radio)
    ih->data->radio = 1;

  value = iupAttribGet(ih, "IMAGE");
  if (value)
  {
    ih->data->type = IUP_TOGGLE_IMAGE;
    if (!iupwin_comctl32ver6 && ih->data->flat)
    {
      dwStyle |= BS_OWNERDRAW;
      ownerdraw = 1;
    }
    else
      dwStyle |= BS_BITMAP|BS_PUSHLIKE;
  }
  else
  {
    ih->data->type = IUP_TOGGLE_TEXT;
    dwStyle |= BS_TEXT|BS_MULTILINE;

    if (iupAttribGetBoolean(ih, "RIGHTBUTTON"))
      dwStyle |= BS_RIGHTBUTTON;
  }

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (radio)
  {
    if (!ownerdraw)
      dwStyle |= BS_RADIOBUTTON;

    if (!iupAttribGet(radio, "_IUPWIN_LASTTOGGLE"))
    {
      /* this is the first toggle in the radio, and then set it with VALUE=ON */
      iupAttribSetStr(ih, "VALUE","ON");
    }
  }
  else if (!ownerdraw)
  {
    if (ih->data->type == IUP_TOGGLE_TEXT && iupAttribGetBoolean(ih, "3STATE"))
      dwStyle |= BS_AUTO3STATE;
    else
      dwStyle |= BS_AUTOCHECKBOX;
  }

  if (!iupwinCreateWindowEx(ih, "BUTTON", 0, dwStyle))
    return IUP_ERROR;

  /* Process WM_COMMAND */
  IupSetCallback(ih, "_IUPWIN_COMMAND_CB", (Icallback)winToggleWmCommand);

  /* Process background color */
  IupSetCallback(ih, "_IUPWIN_CTLCOLOR_CB", (Icallback)winToggleCtlColor);

  if (ih->data->type == IUP_TOGGLE_IMAGE)
  {
    if (iupwin_comctl32ver6)
    {
      IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winToggleImageWmNotify);  /* Process WM_NOTIFY */
      if (ih->data->flat)
        IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winToggleImageFlatProc);
    }
    else
    {
      if (ih->data->flat)
      {
        iupAttribSetStr(ih, "FLAT_ALPHA", "NO");
        IupSetCallback(ih, "_IUPWIN_DRAWITEM_CB", (Icallback)winToggleDrawItem);  /* Process WM_DRAWITEM */
        IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winToggleImageFlatProc);
      }
      else
      {
        IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winToggleImageOldProc);
        iupAttribSetStr(ih, "_IUPWIN_ACTIVE", "YES");
      }
    }
  }

  return IUP_NOERROR;
}

void iupdrvToggleInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = winToggleMapMethod;

  /* Driver Dependent Attribute functions */

  /* Overwrite Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", winToggleGetActiveAttrib, winToggleSetActiveAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_DEFAULT);

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", winToggleGetBgColorAttrib, winToggleSetBgColorAttrib, IUPAF_SAMEASSYSTEM, "DLGBGCOLOR", IUPAF_NO_SAVE|IUPAF_DEFAULT);  

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, winToggleSetUpdateAttrib, "DLGFGCOLOR", NULL, IUPAF_NOT_MAPPED);  /* force the new default value */  
  iupClassRegisterAttribute(ic, "TITLE", iupdrvBaseGetTitleAttrib, winToggleSetTitleAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);

  /* IupToggle only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", NULL, winToggleSetUpdateAttrib, IUPAF_SAMEASSYSTEM, "ACENTER:ACENTER", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, winToggleSetImageAttrib, NULL, NULL, IUPAF_IHANDLENAME|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMINACTIVE", NULL, winToggleSetImInactiveAttrib, NULL, NULL, IUPAF_IHANDLENAME|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMPRESS", NULL, winToggleSetImPressAttrib, NULL, NULL, IUPAF_IHANDLENAME|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", winToggleGetValueAttrib, winToggleSetValueAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PADDING", iupToggleGetPaddingAttrib, winToggleSetPaddingAttrib, IUPAF_SAMEASSYSTEM, "0x0", IUPAF_NOT_MAPPED);

  /* IupToggle Windows only */
  iupClassRegisterAttribute(ic, "RIGHTBUTTON", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);

  /* necessary because it uses an old HBITMAP solution when NOT using styles */
  if (!iupwin_comctl32ver6)  /* Used by iupdrvImageCreateImage */
    iupClassRegisterAttribute(ic, "FLAT_ALPHA", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  /* NOT supported */
  iupClassRegisterAttribute(ic, "MARKUP", NULL, NULL, NULL, NULL, IUPAF_NOT_SUPPORTED|IUPAF_DEFAULT);
}
