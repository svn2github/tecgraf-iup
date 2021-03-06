/** \file
 * \brief Menu Resources
 *
 * See Copyright Notice in "iup.h"
 */

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_attrib.h"
#include "iup_dialog.h"
#include "iup_str.h"
#include "iup_label.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_image.h"
#include "iup_menu.h"

#include "iupmot_drv.h"
#include "iupmot_color.h"


static int mot_menu_exitloop = 0;

int iupdrvMenuPopup(Ihandle* ih, int x, int y)
{
  XButtonEvent ev;
  ev.x_root = x;
  ev.y_root = y;
  XmMenuPosition(ih->handle, &ev);

  XtManageChild(ih->handle);

  mot_menu_exitloop = 0;
  while (!mot_menu_exitloop)
    XtAppProcessEvent(iupmot_appcontext, XtIMAll);
  mot_menu_exitloop = 0;

  return IUP_NOERROR;
}

int iupdrvMenuGetMenuBarSize(Ihandle* ih)
{
  int ch;
  iupdrvFontGetCharSize(ih, NULL, &ch);
  return 5 + ch + 5;
}


/*******************************************************************************************/


static void motItemActivateCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  Icallback cb;

  if (XmIsToggleButton(ih->handle) && !iupAttribGetInt(ih, "AUTOTOGGLE") && !iupAttribGetInt(ih->parent, "RADIO"))
  {
    /* Motif by default will do autotoggle */
    XmToggleButtonSetState(ih->handle, !XmToggleButtonGetState(ih->handle),0);
  }

  cb = IupGetCallback(ih, "ACTION");
  if (cb && cb(ih)==IUP_CLOSE)
    IupExitLoop();

  (void)w;
  (void)call_data;
}

static void motItemArmCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  Icallback cb = IupGetCallback(ih, "HIGHLIGHT_CB");
  if (cb)
    cb(ih);

  (void)w;
  (void)call_data;
}

static void motMenuMapCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  Icallback cb = IupGetCallback(ih, "OPEN_CB");
  if (cb)
    cb(ih);

  (void)w;
  (void)call_data;
}

static void motMenuUnmapCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  Icallback cb = IupGetCallback(ih, "MENUCLOSE_CB");
  if (cb)
    cb(ih);

  (void)w;
  (void)call_data;
}

static void motPopupMenuUnmapCallback(Widget w, Ihandle* ih, XtPointer call_data)
{
  motMenuUnmapCallback(w, ih, call_data);
  mot_menu_exitloop = 1;
}


/*******************************************************************************************/


static void motMenuUnMapMethod(Ihandle* ih)
{
  if (iupMenuIsMenuBar(ih))
    XtDestroyWidget(ih->handle);
  else
    XtDestroyWidget(XtParent(ih->handle));  /* in this case the RowColumn widget is a child of a MenuShell. */
}

static int motMenuMapMethod(Ihandle* ih)
{
  int num_args = 0;
  Arg args[30];

  if (iupMenuIsMenuBar(ih))
  {
    /* top level menu used for MENU attribute in IupDialog (a menu bar) */

    ih->handle = XtVaCreateManagedWidget(
                   iupMenuGetChildIdStr(ih),
                   xmRowColumnWidgetClass, 
                   iupChildTreeGetNativeParentHandle(ih),
                   XmNrowColumnType, XmMENU_BAR,
                   XmNmarginHeight, 0,
                   XmNmarginWidth, 0,
                   XmNresizeWidth, False,
                   NULL);
    if (!ih->handle)
      return IUP_ERROR;
  }
  else
  {
    /* all XmCreate* functions here, also create RowColumn Widgets. */

    if (ih->parent)
    {
      /* parent is a submenu */

      if (iupAttribGetInt(ih, "RADIO"))
      {
        iupmotSetArg(args[num_args++], XmNpacking, XmPACK_COLUMN);
        iupmotSetArg(args[num_args++], XmNradioBehavior, TRUE);
      }

      ih->handle = XmCreatePulldownMenu(
                     ih->parent->handle, 
                     iupMenuGetChildIdStr(ih),
                     args,
                     num_args);
      if (!ih->handle)
        return IUP_ERROR;

      /* update the CascadeButton */
      XtVaSetValues(ih->parent->handle, XmNsubMenuId, ih->handle, NULL);  

      XtAddCallback(ih->handle, XmNmapCallback, (XtCallbackProc)motMenuMapCallback, (XtPointer)ih);
      XtAddCallback(ih->handle, XmNunmapCallback, (XtCallbackProc)motMenuUnmapCallback, (XtPointer)ih);
    }
    else
    {
      /* top level menu used for IupPopup */

      iupmotSetArg(args[num_args++], XmNpopupEnabled, XmPOPUP_AUTOMATIC);

      ih->handle = XmCreatePopupMenu(
                     iupmot_appshell, 
                     iupMenuGetChildIdStr(ih),
                     args,
                     num_args);
      if (!ih->handle)
        return IUP_ERROR;

      XtAddCallback(ih->handle, XmNmapCallback, (XtCallbackProc)motMenuMapCallback, (XtPointer)ih);
      XtAddCallback(ih->handle, XmNunmapCallback, (XtCallbackProc)motPopupMenuUnmapCallback, (XtPointer)ih);
    }
  }

  ih->serial = iupMenuGetChildId(ih); /* must be after using the string */

  return IUP_NOERROR;
}

void iupdrvMenuInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motMenuMapMethod;
  ic->UnMap = motMenuUnMapMethod;

  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iupdrvBaseSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
}


/*******************************************************************************************/


static int motItemSetTitleImageAttrib(Ihandle* ih, const char* value)
{
  iupmotSetPixmap(ih, value, XmNlabelPixmap, 0, "TITLEIMAGE");
  return 1;
}

static int motItemSetTitleAttrib(Ihandle* ih, const char* value)
{
  char *str;

  if (!value)
  {
    str = "     ";
    value = str;
  }
  else
    str = iupMenuGetTitle(ih, value);

  if (XmIsToggleButton(ih->handle))
  {
    char *p = strchr(str, '\t');
    if (p)
    {
      int offset = (p-str);
      char* new_value = iupStrDup(str);
      char* acc_value = new_value + offset + 1;
      new_value[offset] = 0;
      iupmotSetMnemonicTitle(ih, new_value);
      iupmotSetString(ih->handle, XmNacceleratorText, acc_value);
      free(new_value);

      if (str != value) free(str);
      return 1;
    }
  }

  iupmotSetMnemonicTitle(ih, str);

  if (str != value) free(str);
  return 1;
}

static int motItemSetValueAttrib(Ihandle* ih, const char* value)
{
  if (XmIsToggleButton(ih->handle))
  {
    if (iupAttribGetInt(ih->parent, "RADIO"))
      value = "ON";

    XmToggleButtonSetState(ih->handle, iupStrBoolean(value),0);
  }
  return 0;
}

static char* motItemGetValueAttrib(Ihandle* ih)
{
  if (XmIsToggleButton(ih->handle) && XmToggleButtonGetState(ih->handle))
    return "ON";
  else
    return "OFF";
}

static int motItemMapMethod(Ihandle* ih)
{
  int pos;

  if (!ih->parent)
    return IUP_ERROR;

  /* Menu bar can contain only CascadeButtons */
  if (iupMenuIsMenuBar(ih->parent))
  {
    ih->handle = XtVaCreateManagedWidget(
                   iupMenuGetChildIdStr(ih),
                   xmCascadeButtonWidgetClass, 
                   ih->parent->handle,
                   NULL);

    XtAddCallback(ih->handle, XmNactivateCallback, (XtCallbackProc)motItemActivateCallback, (XtPointer)ih);
    XtAddCallback(ih->handle, XmNcascadingCallback, (XtCallbackProc)motItemArmCallback, (XtPointer)ih);
  }
  else
  {
    int num_args = 0;
    Arg args[10];

    if (iupAttribGetInt(ih->parent, "RADIO"))
    {
      iupmotSetArg(args[num_args++], XmNtoggleMode, XmTOGGLE_BOOLEAN);
      iupmotSetArg(args[num_args++], XmNindicatorType, XmONE_OF_MANY_ROUND);
      iupmotSetArg(args[num_args++], XmNindicatorOn, XmINDICATOR_CHECK_BOX);
      iupmotSetArg(args[num_args++], XmNindicatorSize, 13);
      iupmotSetArg(args[num_args++], XmNselectColor, iupmotColorGetPixel(0, 0, 0));
    }
    else
    {
      if (iupAttribGetInt(ih, "HIDEMARK"))
        iupmotSetArg(args[num_args++], XmNindicatorOn, XmINDICATOR_NONE)
      else
        iupmotSetArg(args[num_args++], XmNindicatorOn, XmINDICATOR_CHECK)
      iupmotSetArg(args[num_args++], XmNlabelType, iupAttribGetStr(ih, "TITLEIMAGE")? XmPIXMAP: XmSTRING);
    }

    ih->handle = XtCreateManagedWidget(
                   iupMenuGetChildIdStr(ih),
                   xmToggleButtonWidgetClass, 
                   ih->parent->handle,
                   args,
                   num_args);

    XtAddCallback(ih->handle, XmNvalueChangedCallback, (XtCallbackProc)motItemActivateCallback, (XtPointer)ih);
    XtAddCallback(ih->handle, XmNarmCallback, (XtCallbackProc)motItemArmCallback, (XtPointer)ih);
  }

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupMenuGetChildId(ih); /* must be after using the string */

  XtAddCallback (ih->handle, XmNhelpCallback, (XtCallbackProc)iupmotHelpCallback, (XtPointer)ih);

  pos = IupGetChildPos(ih->parent, ih);
  XtVaSetValues(ih->handle, XmNpositionIndex, pos, NULL);   /* RowColumn Constraint */

  iupUpdateStandardFontAttrib(ih);

  return IUP_NOERROR;
}

void iupdrvItemInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motItemMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Common */
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iupdrvSetStandardFontAttrib, "DEFAULTFONT", IUP_NOT_MAPPED, IUP_INHERIT);  /* use inheritance to retrieve standard fonts */

  /* Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupBaseSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iupdrvBaseSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* IupItem only */
  iupClassRegisterAttribute(ic, "VALUE", motItemGetValueAttrib, motItemSetValueAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, motItemSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TITLEIMAGE", NULL, motItemSetTitleImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}


/*******************************************************************************************/


static int motSubmenuMapMethod(Ihandle* ih)
{
  int pos;

  if (!ih->parent)
    return IUP_ERROR;

  ih->handle = XtVaCreateManagedWidget(
                 iupMenuGetChildIdStr(ih),
                 xmCascadeButtonWidgetClass, 
                 ih->parent->handle,
                 NULL);

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupMenuGetChildId(ih); /* must be after using the string */

  pos = IupGetChildPos(ih->parent, ih);
  XtVaSetValues(ih->handle, XmNpositionIndex, pos, NULL);   /* RowColumn Constraint */

  XtAddCallback(ih->handle, XmNcascadingCallback, (XtCallbackProc)motItemArmCallback, (XtPointer)ih);

  iupUpdateStandardFontAttrib(ih);

  return IUP_NOERROR;
}

void iupdrvSubmenuInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motSubmenuMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Common */
  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, iupdrvSetStandardFontAttrib, "DEFAULTFONT", IUP_NOT_MAPPED, IUP_INHERIT);  /* use inheritance to retrieve standard fonts */

  /* Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, iupBaseSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iupdrvBaseSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* IupSubmenu only */
  iupClassRegisterAttribute(ic, "TITLE", NULL, motItemSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}


/*******************************************************************************************/


static int motSeparatorMapMethod(Ihandle* ih)
{
  int pos;

  if (!ih->parent)
    return IUP_ERROR;

  ih->handle = XtVaCreateManagedWidget(
                 iupMenuGetChildIdStr(ih),
                 xmSeparatorWidgetClass, 
                 ih->parent->handle,
                 NULL);

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupMenuGetChildId(ih); /* must be after using the string */

  pos = IupGetChildPos(ih->parent, ih);
  XtVaSetValues(ih->handle, XmNpositionIndex, pos, NULL);  /* RowColumn Constraint */

  return IUP_NOERROR;
}

void iupdrvSeparatorInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motSeparatorMapMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iupdrvBaseSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
}
