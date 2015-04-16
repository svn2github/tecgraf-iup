/** \file
* \brief iupmatrix edit
* Functions used to edit a node name in place.
*
* See Copyright Notice in "iup.h"
*/

#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupcontrols.h"

#include <cd.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"
#include "iup_childtree.h"

#include "iupmat_def.h"
#include "iupmat_scroll.h"
#include "iupmat_aux.h"
#include "iupmat_edit.h"
#include "iupmat_key.h"
#include "iupmat_getset.h"
#include "iupmat_draw.h"


static void iMatrixEditUpdateValue(Ihandle* ih)
{
  char *value = iupMatrixEditGetValue(ih);

  iupAttribSet(ih, "CELL_EDITED", "Yes");

  if (ih->data->undo_redo) iupAttribSetClassObject(ih, "UNDOPUSHBEGIN", "EDITCELL");

  iupMatrixSetValue(ih, ih->data->edit_lin, ih->data->edit_col, value, 1);

  if (ih->data->undo_redo) iupAttribSetClassObject(ih, "UNDOPUSHEND", NULL);

  iupBaseCallValueChangedCb(ih);

  iupAttribSet(ih, "CELL_EDITED", NULL);

  iupMatrixPrepareDrawData(ih);
  iupMatrixDrawCells(ih, ih->data->edit_lin, ih->data->edit_col, ih->data->edit_lin, ih->data->edit_col);
}

static int iMatrixEditCallEditionCbUpdateValue(Ihandle* ih, int mode, int update)
{
  int ret = iupMatrixAuxCallEditionCbLinCol(ih, ih->data->edit_lin, ih->data->edit_col, mode, update);

  if (update && ret == IUP_DEFAULT && mode == 0)  /* leaving edition mode */
    iMatrixEditUpdateValue(ih);

  return ret;
}

static int iMatrixEditFinish(Ihandle* ih, int setfocus, int update, int accept_ignore)
{
  if (ih->data->editing)
  {
    int ret;

    /* Avoid calling EDITION_CB twice. Usually because a killfocus. */
    if (iupAttribGet(ih, "_IUPMAT_CALL_EDITION"))
      return IUP_DEFAULT;

    iupAttribSet(ih, "_IUPMAT_CALL_EDITION", "1");
    ret = iMatrixEditCallEditionCbUpdateValue(ih, 0, update);
    iupAttribSet(ih, "_IUPMAT_CALL_EDITION", NULL);

    if (ret == IUP_IGNORE && accept_ignore)
      return IUP_IGNORE;

    ih->data->editing = 0;

    iupAttribSet(ih, "_IUPMAT_IGNOREFOCUS", "1");

    IupSetAttribute(ih->data->datah, "VISIBLE", "NO");
    IupSetAttribute(ih->data->datah, "ACTIVE",  "NO");

    if (setfocus)
    {
      IupSetFocus(ih);
      ih->data->has_focus = 1; /* set this so even if getfocus_cb is not called the focus is drawn */
    }

    iupAttribSet(ih, "_IUPMAT_IGNOREFOCUS", NULL);

#ifdef SunOS
    /* Usually when the edit control is hidden the matrix is automatically repainted by the system, except in SunOS. */
    iupMatrixDrawUpdate(ih);
#endif
  }

  return IUP_DEFAULT;
}

int iupMatrixEditConfirm(Ihandle* ih)
{
  return iMatrixEditFinish(ih, 1, 1, 1); /* setfocus + update + accept_ignore */
}

void iupMatrixEditHide(Ihandle* ih)
{
  iMatrixEditFinish(ih, 0, 1, 0); /* NO setfocus + update + NO accept_ignore */
}

void iupMatrixEditAbort(Ihandle* ih)
{
  iMatrixEditFinish(ih, 1, 0, 0); /* setfocus + NO update + NO accept_ignore */
}

static int iMatrixMenuItemAction(Ihandle* ih)
{
  Ihandle* ih_menu = ih->parent;
  Ihandle* ih_matrix = (Ihandle*)iupAttribGet(ih_menu, "_IUP_MATRIX");
  char* t = IupGetAttribute(ih, "TITLE"); 
  IFniinsii cb = (IFniinsii)IupGetCallback(ih_matrix, "DROPSELECT_CB");
  if(cb)
  {
    int i = IupGetChildPos(ih_menu, ih) + 1;
    cb(ih_matrix, ih->data->edit_lin, ih->data->edit_col, ih_menu, t, i, 1);
  }

  IupStoreAttribute(ih_menu, "VALUE", t);

  iMatrixEditCallEditionCbUpdateValue(ih_matrix, 0, 1);  /* always update, similar to iupMatrixEditConfirm */
  iupMatrixDrawUpdate(ih_matrix);

  return IUP_DEFAULT;
}

static void iMatrixEditInitMenu(Ihandle* menu)
{
  char *value;
  int i = 1;
  int v = IupGetInt(menu, "VALUE");

  do 
  {
    value = IupGetAttributeId(menu, "", i);
    if (value)
    {
      Ihandle* item = IupItem(value, NULL);

      char* img = IupGetAttributeId(menu, "IMAGE", i);
      if (img)
        IupSetAttribute(item, "IMAGE", img);

      IupSetCallback(item, "ACTION", (Icallback)iMatrixMenuItemAction);

      if (v == i)   /* if v==0 no mark will be shown */
        IupSetAttribute(item, "VALUE", "On");

      IupAppend(menu, item);
    }
    i++;
  } while (value);
}

static int iMatrixEditCallMenuDropCb(Ihandle* ih, int lin, int col)
{
  IFnnii cb = (IFnnii)IupGetCallback(ih, "MENUDROP_CB");
  if(cb)
  {
    Ihandle* menu = IupMenu(NULL);
    int ret;
    char* value = iupMatrixGetValueDisplay(ih, lin, col);
    if (!value) value = "";

    iupAttribSet(menu, "PREVIOUSVALUE", value);
    iupAttribSet(menu, "_IUP_MATRIX", (char*)ih);

    ret = cb(ih, menu, lin, col);
    if (ret == IUP_DEFAULT)
    {
      int x, y, w, h;

      ih->data->datah = menu;

      /* process menu to create items and set common callback */
      iMatrixEditInitMenu(menu);

      /* calc menu position */
      iupMatrixGetVisibleCellDim(ih, lin, col, &x, &y, &w, &h);

      x += IupGetInt(ih, "X");
      y += IupGetInt(ih, "Y");

      IupPopup(menu, x, y+h);
      IupDestroy(menu);

      /* restore a valid handle */
      ih->data->datah = ih->data->texth;

      return 1;
    }

    IupDestroy(menu);
  }

  return 0;
}

static int iMatrixEditCallDropdownCb(Ihandle* ih, int lin, int col)
{
  IFnnii cb = (IFnnii)IupGetCallback(ih, "DROP_CB");
  if(cb)
  {
    int ret;
    char* value = iupMatrixGetValueDisplay(ih, lin, col);
    if (!value) value = "";

    IupStoreAttribute(ih->data->droph, "PREVIOUSVALUE", value);
    IupSetAttribute(ih->data->droph, "VALUE", "1");

    ret = cb(ih, ih->data->droph, lin, col);

    /* check if the user set an invalid value */
    if (IupGetInt(ih->data->droph, "VALUE") == 0)
      IupSetAttribute(ih->data->droph, "VALUE", "1");

    if(ret == IUP_DEFAULT)
      return 1;
  }

  return 0;
}

static int iMatrixEditDropDownAction_CB(Ihandle* ih_list, char* t, int i, int v)
{
  Ihandle* ih_matrix = ih_list->parent;
  IFniinsii cb = (IFniinsii)IupGetCallback(ih_matrix, "DROPSELECT_CB");
  if(cb)
  {
    int ret = cb(ih_matrix, ih_matrix->data->edit_lin, ih_matrix->data->edit_col, ih_list, t, i, v);

    /* If the user returns IUP_CONTINUE in a dropselect_cb 
    the value is accepted and the matrix leaves edition mode. */
    if (ret == IUP_CONTINUE)
    {
      if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
        iupMatrixDrawUpdate(ih_matrix);
    }
  }

  return IUP_DEFAULT;
}

static void iMatrixEditChooseElement(Ihandle* ih)
{
  int drop = iMatrixEditCallDropdownCb(ih, ih->data->edit_lin, ih->data->edit_col);
  if(drop)
    ih->data->datah = ih->data->droph;
  else
  {
    char* value;

    ih->data->datah = ih->data->texth;

    /* dropdown values are set by the user in DROP_CB.
    text value is set here from cell contents. */
    value = iupMatrixGetValueDisplay(ih, ih->data->edit_lin, ih->data->edit_col);
    if (!value) value = "";
    IupStoreAttribute(ih->data->texth, "VALUE", value);
    IupStoreAttribute(ih->data->texth, "PREVIOUSVALUE", value);
  }
}

static int iMatrixEditDropDown_CB(Ihandle* ih_list, int state)
{
  /* In Motif if DROPDOWN=YES then when the dropdown button is clicked 
     the list looses its focus and when the dropped list is closed 
     the list regain the focus, also when that happen if the list looses its focus 
     to another control the kill focus callback is not called. */
  Ihandle* ih_matrix = ih_list->parent;
  if (state == 1)
    iupAttribSet(ih_matrix, "_IUPMAT_DROPDOWN", "1");

  return IUP_DEFAULT;
}

static int iMatrixEditKillFocus_CB(Ihandle* ih_edit)
{
  Ihandle* ih_matrix = ih_edit->parent;

  if (IupGetGlobal("MOTIFVERSION"))
  {
    if (iupAttribGet(ih_matrix, "_IUPMAT_DROPDOWN") ||  /* from iMatrixEditDropDown_CB, in Motif */
        iupAttribGet(ih_matrix, "_IUPMAT_DOUBLECLICK"))  /* from iMatrixMouseLeftPress, in Motif */
    {
      iupAttribSet(ih_matrix, "_IUPMAT_DOUBLECLICK", NULL);
      iupAttribSet(ih_matrix, "_IUPMAT_DROPDOWN", NULL);
      return IUP_DEFAULT;
    }
  }

  if (iupAttribGet(ih_matrix, "_IUPMAT_IGNOREFOCUS"))
    return IUP_DEFAULT;

  if (ih_matrix->data->edit_hide_onfocus)
  {
    ih_matrix->data->edit_hidden_byfocus = 1;
    iupMatrixEditHide(ih_matrix);
    ih_matrix->data->edit_hidden_byfocus = 0;
  }
  else
  {
    IFn cb = (IFn)IupGetCallback(ih_matrix, "EDITKILLFOCUS_CB");
    if (cb)
      cb(ih_matrix);
  }

  return IUP_DEFAULT;
}

int iupMatrixEditIsVisible(Ihandle* ih)
{
  if (!IupGetInt(ih, "ACTIVE"))
    return 0;

  if (!IupGetInt(ih->data->datah, "VISIBLE"))
    return 0;

  return 1;
}

void iupMatrixEditUpdatePos(Ihandle* ih)
{
  int w, h, x, y, visible;
  visible = iupMatrixGetVisibleCellDim(ih, ih->data->edit_lin, ih->data->edit_col, &x, &y, &w, &h);
  if (!visible && !ih->data->edit_hide_onfocus)
      IupSetAttribute(ih->data->datah, "VISIBLE", "NO");

  ih->data->datah->x = x;
  ih->data->datah->y = y;

  ih->data->datah->currentwidth = w;
  ih->data->datah->currentheight = h;
  iupClassObjectLayoutUpdate(ih->data->datah);

  if (visible && !ih->data->edit_hide_onfocus)
    IupSetAttribute(ih->data->datah, "VISIBLE", "YES");
}

int iupMatrixEditShow(Ihandle* ih)
{
  char* mask;

  /* work around for Windows when using Multiline */
  if (iupAttribGet(ih, "_IUPMAT_IGNORE_SHOW"))
  {
    iupAttribSet(ih, "_IUPMAT_IGNORE_SHOW", NULL);
    return 0;
  }

  /* there are no cells that can be edited */
  if (ih->data->columns.num <= 1 || ih->data->lines.num <= 1)
    return 0;

  if (ih->data->editing || iupMatrixEditIsVisible(ih))
  {
    if (ih->data->edit_hide_onfocus)
      return 0;

    iupMatrixEditHide(ih);
  }

  /* notify application */
  if (iMatrixEditCallEditionCbUpdateValue(ih, 1, 0) == IUP_IGNORE)  /* only place where mode=1 */
    return 0;

  ih->data->edit_lin = ih->data->lines.focus_cell;
  ih->data->edit_col = ih->data->columns.focus_cell;

  if (iMatrixEditCallMenuDropCb(ih, ih->data->edit_lin, ih->data->edit_col))
    return 0;

  ih->data->editing = 1;

  /* select edit control */
  iMatrixEditChooseElement(ih);

  /* position the cell to make it visible */
  /* If the focus is not visible, a scroll is done for that the focus to be visible */
  if (!iupMatrixAuxIsCellStartVisible(ih, ih->data->edit_lin, ih->data->edit_col))
    iupMatrixScrollToVisible(ih, ih->data->edit_lin, ih->data->edit_col);

  /* set attributes */
  iupMatrixPrepareDrawData(ih);
  IupStoreAttribute(ih->data->datah, "BGCOLOR", iupMatrixGetBgColorStr(ih, ih->data->edit_lin, ih->data->edit_col));
  IupStoreAttribute(ih->data->datah, "FGCOLOR", iupMatrixGetFgColorStr(ih, ih->data->edit_lin, ih->data->edit_col));
  IupSetAttribute(ih->data->datah, "FONT", iupMatrixGetFont(ih, ih->data->edit_lin, ih->data->edit_col));

  mask = IupGetAttributeId2(ih, "MASK", ih->data->edit_lin, ih->data->edit_col);
  if (mask)
  {
    IupSetAttribute(ih->data->datah, "MASKCASEI", IupGetAttributeId2(ih, "MASKCASEI", ih->data->edit_lin, ih->data->edit_col));
    IupSetAttribute(ih->data->datah, "MASK", mask);
  }
  else
  {
    mask = IupGetAttributeId2(ih, "MASKINT", ih->data->edit_lin, ih->data->edit_col);
    if (mask)
      IupSetAttribute(ih->data->datah, "MASKINT", mask);
    else
    {
      mask = IupGetAttributeId2(ih, "MASKFLOAT", ih->data->edit_lin, ih->data->edit_col);
      if (mask)
        IupSetAttribute(ih->data->datah, "MASKFLOAT", mask);
      else
        IupSetAttribute(ih->data->datah, "MASK", NULL);
    }
  }

  iupMatrixEditUpdatePos(ih);

  /* activate and show */
  IupSetAttribute(ih->data->datah, "ACTIVE",  "YES");
  IupSetAttribute(ih->data->datah, "VISIBLE", "YES");
  IupSetFocus(ih->data->datah);

  return 1;
}

static int iMatrixEditTextAction_CB(Ihandle* ih_text, int c, char* after)
{
  Ihandle* ih_matrix = ih_text->parent;
  IFniiiis cb = (IFniiiis) IupGetCallback(ih_matrix, "ACTION_CB");
  if (cb && iup_isprint(c)) /* only for keys that ARE ASCii characters */
  {
    int oldc = c;
    c = cb(ih_matrix, c, ih_matrix->data->edit_lin, ih_matrix->data->edit_col, 1, after);
    if (c == IUP_IGNORE || c == IUP_CLOSE || c == IUP_CONTINUE)
      return c;
    else if(c == IUP_DEFAULT)
      c = oldc;
    return c;
  }

  return IUP_DEFAULT;
}

static int iMatrixEditTextKeyAny_CB(Ihandle* ih_text, int c)
{
  Ihandle* ih_matrix = ih_text->parent;
  IFniiiis cb = (IFniiiis) IupGetCallback(ih_matrix, "ACTION_CB");
  if (cb && !iup_isprint(c)) /* only for keys that are NOT ASCii characters */
  {
    int oldc = c;
    c = cb(ih_matrix, c, ih_matrix->data->edit_lin, ih_matrix->data->edit_col, 1, IupGetAttribute(ih_text, "VALUE"));
    if(c == IUP_IGNORE || c == IUP_CLOSE || c == IUP_CONTINUE)
      return c;
    else if(c == IUP_DEFAULT)
      c = oldc;
  }

  switch (c)
  {
    case K_cUP:
    case K_cDOWN:
    case K_cLEFT:
    case K_cRIGHT:     
      if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
      {
        iupMatrixProcessKeyPress(ih_matrix, c);  
        return IUP_IGNORE;
      }
      break;
    case K_UP:
      if (!IupGetInt(ih_text, "MULTILINE") || IupGetInt(ih_text, "CARET") == 1)  /* if Multiline CARET will be "L,C" */
      {
        /* if at the first line of the text */
        if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
        {
          iupMatrixProcessKeyPress(ih_matrix, c);  
          return IUP_IGNORE;
        }
      }
      break;
    case K_DOWN:
      { 
        char* value = IupGetAttribute(ih_text, "VALUE");
        if (value)
        {
          /* if at the last line of the text */
          if (!IupGetInt(ih_text, "MULTILINE") || iupStrLineCount(value) == IupGetInt(ih_text, "CARET"))  /* if Multiline CARET will be "L,C" */
          {
            if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
            {
              iupMatrixProcessKeyPress(ih_matrix, c);  
              return IUP_IGNORE;
            }
          }
        }
      }
      break;
    case K_LEFT:
      if (IupGetInt(ih_text, "CARETPOS") == 0)
      {
        /* if at the first character */
        if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
        {
          iupMatrixProcessKeyPress(ih_matrix, c);  
          return IUP_IGNORE;
        }
      }
      break;
    case K_RIGHT:
      { 
        char* value = IupGetAttribute(ih_text, "VALUE");
        if (value)
        {
          /* if at the last character */
          if (IupGetInt(ih_text, "COUNT") == IupGetInt(ih_text, "CARETPOS"))
          {
            if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
            {
              iupMatrixProcessKeyPress(ih_matrix, c);  
              return IUP_IGNORE;
            }
          }
        }
      }
      break;
    case K_ESC:
      iupMatrixEditAbort(ih_matrix);
      return IUP_IGNORE;  /* always ignore to avoid the defaultesc behavior from here */
    case K_CR:
      if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
      {
        if (iupStrEqualNoCase(IupGetGlobal("DRIVER"), "Win32") && IupGetInt(ih_text, "MULTILINE"))
        {
          /* work around for Windows when using Multiline */
          iupAttribSet(ih_matrix, "_IUPMAT_IGNORE_SHOW", "1");
        }

        if (iupMatrixAuxCallLeaveCellCb(ih_matrix) != IUP_IGNORE)
        {
          iupMATRIX_ScrollKeyCr(ih_matrix);
          iupMatrixAuxCallEnterCellCb(ih_matrix);
        }
        iupMatrixDrawUpdate(ih_matrix);
      }
      return IUP_IGNORE;  /* always ignore to avoid the defaultenter behavior from here */
  }

  return IUP_CONTINUE;
}

static int iMatrixEditDropDownKeyAny_CB(Ihandle* ih_list, int c)
{
  Ihandle* ih_matrix = ih_list->parent;
  IFniiiis cb = (IFniiiis)IupGetCallback(ih_matrix, "ACTION_CB");
  if (cb)
  {
    int oldc = c;
    c = cb(ih_matrix, c, ih_matrix->data->edit_lin, ih_matrix->data->edit_col, 1, "");
    if (c == IUP_IGNORE || c == IUP_CLOSE  || c == IUP_CONTINUE)
      return c;
    else if(c == IUP_DEFAULT)
      c = oldc;
  }

  switch (c)
  {
    case K_CR:
      if (iupMatrixEditConfirm(ih_matrix) == IUP_DEFAULT)
      {
        if (iupMatrixAuxCallLeaveCellCb(ih_matrix) != IUP_IGNORE)
        {
          iupMATRIX_ScrollKeyCr(ih_matrix);
          iupMatrixAuxCallEnterCellCb(ih_matrix);
        }
        iupMatrixDrawUpdate(ih_matrix);
        return IUP_IGNORE;
      }
      break;
    case K_ESC:
      iupMatrixEditAbort(ih_matrix);
      return IUP_IGNORE;
  }

  return IUP_CONTINUE;
}

char* iupMatrixEditGetValue(Ihandle* ih)
{
  if (ih->data->datah == ih->data->droph)
    return IupGetAttribute(ih->data->datah, IupGetAttribute(ih->data->droph, "VALUE"));
  else
    return IupGetAttribute(ih->data->datah, "VALUE");
}

void iupMatrixEditCreate(Ihandle* ih)
{
  /******** EDIT *************/
  ih->data->texth = IupText(NULL);
  ih->data->texth->currentwidth = 20;  /* just to avoid initial size 0x0 */
  ih->data->texth->currentheight = 10;
  iupChildTreeAppend(ih, ih->data->texth);

  IupSetCallback(ih->data->texth, "ACTION",       (Icallback)iMatrixEditTextAction_CB);
  IupSetCallback(ih->data->texth, "K_ANY",        (Icallback)iMatrixEditTextKeyAny_CB);
  IupSetCallback(ih->data->texth, "KILLFOCUS_CB", (Icallback)iMatrixEditKillFocus_CB);
  IupSetAttribute(ih->data->texth, "VALUE",  "");
  IupSetAttribute(ih->data->texth, "VISIBLE", "NO");
  IupSetAttribute(ih->data->texth, "ACTIVE",  "NO");


  /******** DROPDOWN *************/
  ih->data->droph = IupList(NULL);
  ih->data->droph->currentwidth = 20;  /* just to avoid initial size 0x0 */
  ih->data->droph->currentheight = 10;
  iupChildTreeAppend(ih, ih->data->droph);

  if (IupGetGlobal("MOTIFVERSION"))
    IupSetCallback(ih->data->droph, "DROPDOWN_CB",  (Icallback)iMatrixEditDropDown_CB);

  IupSetCallback(ih->data->droph, "ACTION",       (Icallback)iMatrixEditDropDownAction_CB);
  IupSetCallback(ih->data->droph, "KILLFOCUS_CB", (Icallback)iMatrixEditKillFocus_CB);
  IupSetCallback(ih->data->droph, "K_ANY",        (Icallback)iMatrixEditDropDownKeyAny_CB);
  IupSetAttribute(ih->data->droph, "DROPDOWN", "YES");
  IupSetAttribute(ih->data->droph, "MULTIPLE", "NO");
  IupSetAttribute(ih->data->droph, "VISIBLE", "NO");
  IupSetAttribute(ih->data->droph, "ACTIVE",  "NO");
}
