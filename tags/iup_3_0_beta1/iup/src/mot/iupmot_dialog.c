/** \file
 * \brief IupDialog class
 *
 * See Copyright Notice in "iup.h"
 */

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/MwmUtil.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_class.h"
#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_dlglist.h"
#include "iup_attrib.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_drvinfo.h"
#include "iup_focus.h"
#include "iup_str.h"
#define _IUPDLG_PRIVATE
#include "iup_dialog.h"
#include "iup_image.h"

#include "iupmot_drv.h"
#include "iupmot_color.h"


Atom iupmot_wm_deletewindow = 0;  /* used also by IupMessageDlg */

static int motDialogSetBgColorAttrib(Ihandle* ih, const char* value);

/****************************************************************
                     Utilities
****************************************************************/


int iupdrvDialogIsVisible(Ihandle* ih)
{
  return iupdrvIsVisible(ih) || ih->data->show_state == IUP_MINIMIZE;
}

void iupdrvDialogUpdateSize(Ihandle* ih)
{
  Dimension width, height;
  XtVaGetValues(ih->handle, XmNwidth, &width, XmNheight, &height, NULL);
  ih->currentwidth = width;
  ih->currentheight = height;
}

void iupdrvDialogGetSize(InativeHandle* handle, int *w, int *h)
{
  Dimension width, height;
  XtVaGetValues(handle, XmNwidth, &width, XmNheight, &height, NULL);
  if (w) *w = width;
  if (h) *h = height;
}

void iupmotDialogSetVisual(Ihandle* ih, void* visual)
{
  Ihandle *dialog = IupGetDialog(ih);
  XtVaSetValues(dialog->handle, XmNvisual, visual, NULL);
}

void iupmotDialogResetVisual(Ihandle* ih)
{
  Ihandle *dialog = IupGetDialog(ih);
  XtVaSetValues(dialog->handle, XmNvisual, iupmot_visual, NULL);
}

void iupdrvDialogSetVisible(Ihandle* ih, int visible)
{
  if (visible)
  {
    XtMapWidget(ih->handle);
    XRaiseWindow(iupmot_display, XtWindow(ih->handle));
    while (!iupdrvDialogIsVisible(ih)); /* waits until window get mapped */
  }
  else
  {
    /* if iupdrvIsVisible reports hidden, then it should be minimized */ 
    if (!iupdrvIsVisible(ih))  /* can NOT hide a minimized window, so map it first. */
    {
      XtMapWidget(ih->handle);
      XRaiseWindow(iupmot_display, XtWindow(ih->handle));
      while (!iupdrvDialogIsVisible(ih)); /* waits until window get mapped */
    }

    XtUnmapWidget(ih->handle);
    while (iupdrvDialogIsVisible(ih)); /* waits until window gets unmapped */
  }
}

void iupdrvDialogGetPosition(InativeHandle* handle, int *x, int *y)
{
  Dimension cur_x, cur_y;
  XtVaGetValues(handle, XmNx, &cur_x, 
                        XmNy, &cur_y, 
                        NULL);
  if (x) *x = cur_x;
  if (y) *y = cur_y;
}

void iupdrvDialogSetPosition(Ihandle *ih, int x, int y)
{
  XtVaSetValues(ih->handle,
    XmNx, (XtArgVal)x,
    XmNy, (XtArgVal)y,
    NULL);
}

static int motDialogGetMenuSize(Ihandle* ih)
{
  if (ih->data->menu)
    return iupdrvMenuGetMenuBarSize(ih->data->menu);
  else
    return 0;
}

void iupdrvDialogGetDecoration(Ihandle* ih, int *border, int *caption, int *menu)
{
  static int native_border = 0;
  static int native_caption = 0;

  int has_caption = iupAttribGetIntDefault(ih, "MAXBOX")  ||
                    iupAttribGetIntDefault(ih, "MINBOX")  ||
                    iupAttribGetIntDefault(ih, "MENUBOX") || 
                    IupGetAttribute(ih, "TITLE");  /* must use IupGetAttribute to check from the native implementation */

  int has_border = has_caption ||
                   iupAttribGetIntDefault(ih, "RESIZE") ||
                   iupAttribGetIntDefault(ih, "BORDER");

  *menu = motDialogGetMenuSize(ih);

  if (ih->handle && iupdrvDialogIsVisible(ih))
  {
    int win_border, win_caption;
    if (iupdrvGetWindowDecor((void*)XtWindow(ih->handle), &win_border, &win_caption))
    {
      *border = 0;
      if (has_border)
        *border = win_border;

      *caption = 0;
      if (has_caption)
        *caption = win_caption;

      if (!native_border && *border)
        native_border = win_border;

      if (!native_caption && *caption)
        native_caption = win_caption;

      return;
    }
  }

  /* I could not set the size of the window including the decorations when the dialog is hidden */
  /* So we have to estimate the size of borders and caption when the dialog is hidden           */

  *border = 0;
  if (has_border)
  {
    if (native_border)
      *border = native_border;
    else
      *border = 5;
  }

  *caption = 0;
  if (has_caption)
  {
    if (native_caption)
      *caption = native_caption;
    else
      *caption = 20;
  }
}

static int motDialogQueryWMspecSupport(Atom feature)
{
  static Atom netsuppport = 0;
  Atom type;
  Atom *atoms;
  int format;
  unsigned long after, natoms, i;

  if (!netsuppport)
    netsuppport = XmInternAtom(iupmot_display, "_NET_SUPPORTED", False);

  /* get all the features */
  XGetWindowProperty(iupmot_display, RootWindow(iupmot_display, iupmot_screen),
                     netsuppport, 0, LONG_MAX, False, XA_ATOM, &type, &format, &natoms,
                     &after, (unsigned char **)&atoms);
  if (type != XA_ATOM || atoms == NULL)
  {
    if (atoms) XFree(atoms);
    return 0;
  }

  /* Lookup the feature we want */
  for (i = 0; i < natoms; i++)
  {
    if (atoms[i] == feature)
    {
      XFree(atoms);
      return 1;
    }
  }

  XFree(atoms);
  return 0;
}

static void motDialogSetWindowManagerStyle(Ihandle* ih)
{
  MwmHints hints;
  static Atom xwmhint = 0;
  if (!xwmhint)
    xwmhint = XmInternAtom(iupmot_display, "_MOTIF_WM_HINTS", False);

  hints.flags = (MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS);
  hints.functions = 0;
  hints.decorations = 0;
  hints.input_mode = 0;
  hints.status = 0;

  if (IupGetAttribute(ih, "TITLE")) {  /* must use IupGetAttribute to check from the native implementation */
    hints.functions   |= MWM_FUNC_MOVE;
    hints.decorations |= MWM_DECOR_TITLE;
  }

  if (iupAttribGetIntDefault(ih, "MENUBOX")) {
    hints.functions   |= MWM_FUNC_CLOSE;
    hints.decorations |= MWM_DECOR_MENU;
  }

  if (iupAttribGetIntDefault(ih, "MINBOX")) {
    hints.functions   |= MWM_FUNC_MINIMIZE;
    hints.decorations |= MWM_DECOR_MINIMIZE;
  }

  if (iupAttribGetIntDefault(ih, "MAXBOX")) {
    hints.functions   |= MWM_FUNC_MAXIMIZE;
    hints.decorations |= MWM_DECOR_MAXIMIZE;
  }

  if (iupAttribGetIntDefault(ih, "RESIZE")) {
    hints.functions   |= MWM_FUNC_RESIZE;
    hints.decorations |= MWM_DECOR_RESIZEH;
  }

  if (iupAttribGetIntDefault(ih, "BORDER"))
    hints.decorations |= MWM_DECOR_BORDER;

  XChangeProperty(iupmot_display, XtWindow(ih->handle),
      xwmhint, xwmhint,
      32, PropModeReplace,
      (const unsigned char *) &hints,
      PROP_MOTIF_WM_HINTS_ELEMENTS);
}

static void motDialogChangeWMState(Ihandle* ih, Atom state1, Atom state2, int operation)
{
  static Atom wmstate = 0;
  if (!wmstate)
    wmstate = XmInternAtom(iupmot_display, "_NET_WM_STATE", False);

  if (iupdrvDialogIsVisible(ih))
  {
    XEvent evt;
    evt.type = ClientMessage;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = True;
    evt.xclient.display = iupmot_display;
    evt.xclient.window = XtWindow(ih->handle);
    evt.xclient.message_type = wmstate;
    evt.xclient.format = 32;
    evt.xclient.data.l[0] = operation;
    evt.xclient.data.l[1] = state1;
    evt.xclient.data.l[2] = state2;
    evt.xclient.data.l[3] = 0;
    evt.xclient.data.l[4] = 0;
    
    XSendEvent(iupmot_display, RootWindow(iupmot_display, iupmot_screen), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &evt);
  }
  else
  {
    if (operation)
    {
      if (state1 && state2)
      {
        Atom atoms[2];
        atoms[0] = state1;
        atoms[0] = state2;

        XChangeProperty(iupmot_display, XtWindow(ih->handle),
            wmstate, XA_ATOM,
            32, PropModeReplace,
            (const unsigned char *)&atoms, 2);
      }
      else
      {
        XChangeProperty(iupmot_display, XtWindow(ih->handle),
            wmstate, XA_ATOM,
            32, PropModeReplace,
            (const unsigned char *)&state1, 1);
      }
    }
    else
    {
      /* TODO: This is not working. The property is not correctly removed. */
      /* XDeleteProperty(iupmot_display, XtWindow(ih->handle), wmstate); */

      /* Maybe the right way to do it is to retrieve all the atoms               */
      /* and change again with all atoms except the one to remove                */
    }
  }
}

static int motDialogSetFullScreen(Ihandle* ih, int fullscreen)
{
  static int support_fullscreen = -1;  /* WARNING: The WM can be changed dinamically */

  static Atom xwmfs = 0;
  if (!xwmfs)
    xwmfs = XmInternAtom(iupmot_display, "_NET_WM_STATE_FULLSCREEN", False);

  if (support_fullscreen == -1)
    support_fullscreen = motDialogQueryWMspecSupport(xwmfs);

  if (support_fullscreen)
  {
    motDialogChangeWMState(ih, xwmfs, 0, fullscreen);
    return 1;
  }

  return 0;
}

int iupdrvDialogSetPlacement(Ihandle* ih, int x, int y)
{
  char* placement;

  if (iupAttribGetInt(ih, "FULLSCREEN"))
    return 1;
  
  placement = iupAttribGetStr(ih, "PLACEMENT");
  if (!placement)
    return 0;

  if (iupStrEqualNoCase(placement, "MINIMIZED"))
  {
    if (iupdrvDialogIsVisible(ih))
      XIconifyWindow(iupmot_display, XtWindow(ih->handle), iupmot_screen);
    else
    {
      /* TODO: This is not working, so force a minimize after visible.  */
      /*XWMHints wm_hints;                                               */
      /*wm_hints.flags = StateHint;                                      */
      /*wm_hints.initial_state = IconicState;                            */
      /*XSetWMHints(iupmot_display, XtWindow(ih->handle), &wm_hints);  */

      XtMapWidget(ih->handle);
      XIconifyWindow(iupmot_display, XtWindow(ih->handle), iupmot_screen);
    }
  }
  else if (iupStrEqualNoCase(placement, "MAXIMIZED"))
  {
    static Atom maxatoms[2] = {0, 0};
    if (!(maxatoms[0]))
    {
      maxatoms[0] = XmInternAtom(iupmot_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
      maxatoms[1] = XmInternAtom(iupmot_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    }

    motDialogChangeWMState(ih, maxatoms[0], maxatoms[1], 1);
  }
  else if (iupStrEqualNoCase(placement, "FULL"))
  {
    int width, height;
    int border, caption, menu;
    iupdrvDialogGetDecoration(ih, &border, &caption, &menu);

    /* position the decoration outside the screen */
    x = -(border);
    y = -(border+caption+menu);

    /* the dialog client area will cover the task bar */
    iupdrvGetFullSize(&width, &height);

    height += menu; /* the menu is included in the client area size in Motif. */

    /* set the new size and position */
    /* The resize event will update the layout */
    XtVaSetValues(ih->handle,
      XmNx, (XtArgVal)x,  /* outside border */
      XmNy, (XtArgVal)y,
      XmNwidth, (XtArgVal)width,  /* client size */
      XmNheight, (XtArgVal)height,
      NULL);
  }

  iupAttribSetStr(ih, "PLACEMENT", NULL); /* reset to NORMAL */

  return 1;
}


/****************************************************************
                     Callbacks and Events
****************************************************************/


static void motDialogCBclose(Widget w, XtPointer client_data, XtPointer call_data)
{
  Icallback cb;
  Ihandle *ih = (Ihandle*)client_data;
  if (!ih) return;
  (void)call_data;
  (void)w;

  /* even when ACTIVE=NO the dialog gets this event */
  if (!iupdrvIsActive(ih))
    return;

  cb = IupGetCallback(ih, "CLOSE_CB");
  if (cb)
  {
    int ret = cb(ih);
    if (ret == IUP_IGNORE)
      return;
    if (ret == IUP_CLOSE)
      IupExitLoop();
  }

  IupHide(ih); /* default: close the window */
}

static void motDialogConfigureNotify(Widget w, XEvent *evt, String s [], Cardinal *card)
{
  IFnii cb;
  int border, caption, menu;
  XConfigureEvent *cevent = (XConfigureEvent *)evt;
  Ihandle* ih;
  (void)s;
  (void)card;

  XtVaGetValues(w, XmNuserData, &ih, NULL);
  if (!ih) return;

  if (ih->data->menu && ih->data->menu->handle)
  {
    XtVaSetValues(ih->data->menu->handle,
      XmNwidth, (XtArgVal)(cevent->width),
      NULL);
  }

  if (ih->data->ignore_resize) return; 

  iupdrvDialogGetDecoration(ih, &border, &caption, &menu);

  ih->currentwidth = cevent->width + 2*border;
  ih->currentheight = cevent->height + 2*border + caption; /* menu is inside the dialog_manager */

  cb = (IFnii)IupGetCallback(ih, "RESIZE_CB");
  if (cb) cb(ih, cevent->width, cevent->height - menu);  /* notify to the application size the client area size */

  ih->data->ignore_resize = 1;
  IupRefresh(ih);
  ih->data->ignore_resize = 0;
}

static void motDialogCBStructureNotifyEvent(Widget w, XtPointer data, XEvent *evt, Boolean *cont)
{
  Ihandle *ih = (Ihandle*)data;
  int state = -1;
  (void)cont;
  (void)w;

  switch(evt->type)
  {
    case MapNotify:
    {
      if (ih->data->show_state == IUP_MINIMIZE) /* it is a RESTORE. */
        state = IUP_RESTORE;
      break;
    }
    case UnmapNotify:
    {
      if (ih->data->show_state != IUP_HIDE) /* it is a MINIMIZE. */
        state = IUP_MINIMIZE;
      break;
    }
  }

  if (state < 0)
    return;

  if (ih->data->show_state != state)
  {
    IFni cb;
    ih->data->show_state = state;

    cb = (IFni)IupGetCallback(ih, "SHOW_CB");
    if (cb && cb(ih, state) == IUP_CLOSE) 
      IupExitLoop();
  }
}

static void motDialogDestroyCallback(Widget w, Ihandle *ih, XtPointer call_data)
{
  /* If the IUP dialog was not destroyed, destroy it here. */
  if (iupObjectCheck(ih))
    IupDestroy(ih);

  /* this callback is usefull to destroy children dialogs when the parent is destroyed. */
  /* The application is responsable for destroying the children before this happen.     */
  (void)w;
  (void)call_data;
}


/****************************************************************
                     Idialog
****************************************************************/

/* replace the common dialog SetPosition method because of 
   the menu that it is inside the dialog. */
static void motDialogSetPositionMethod(Ihandle* ih, int x, int y)
{
  /* x and y are always 0 for the dialog. */
  ih->x = x;
  ih->y = y;

  if (ih->firstchild)
  {
    int menu = motDialogGetMenuSize(ih);

    /* Child coordinates are relative to client left-top corner. */
    iupClassObjectSetPosition(ih->firstchild, 0, menu);
  }
}

static void* motDialogGetInnerNativeContainerMethod(Ihandle* ih, Ihandle* child)
{
  (void)child;
  return XtNameToWidget(ih->handle, "*dialog_manager");
}

static int motDialogMapMethod(Ihandle* ih)
{
  Widget dialog_manager;
  InativeHandle* parent;
  int mwm_decor = 0;
  int num_args = 0;
  Arg args[20];

  if (iupAttribGetInt(ih, "DIALOGFRAME")) 
  {
    iupAttribSetStr(ih, "RESIZE", "NO");
    iupAttribSetStr(ih, "MAXBOX", "NO");
    iupAttribSetStr(ih, "MINBOX", "NO");
  }

  /****************************/
  /* Create the dialog shell  */
  /****************************/

  if (iupAttribGetStr(ih, "TITLE"))
      mwm_decor |= MWM_DECOR_TITLE;
  if (iupAttribGetIntDefault(ih, "MENUBOX"))
      mwm_decor |= MWM_DECOR_MENU;
  if (iupAttribGetIntDefault(ih, "MINBOX"))
      mwm_decor |= MWM_DECOR_MINIMIZE;
  if (iupAttribGetIntDefault(ih, "MAXBOX"))
      mwm_decor |= MWM_DECOR_MAXIMIZE;
  if (iupAttribGetIntDefault(ih, "RESIZE"))
      mwm_decor |= MWM_DECOR_RESIZEH;
  if (iupAttribGetIntDefault(ih, "BORDER"))
      mwm_decor |= MWM_DECOR_BORDER;

  iupmotSetArg(args[num_args++], XmNmappedWhenManaged, False);  /* so XtRealizeWidget will not show the dialog */
  iupmotSetArg(args[num_args++], XmNdeleteResponse, XmDO_NOTHING);
  iupmotSetArg(args[num_args++], XmNallowShellResize, True); /* Used so the BulletinBoard can control the shell size */
  iupmotSetArg(args[num_args++], XmNtitle, "");
  iupmotSetArg(args[num_args++], XmNvisual, iupmot_visual);
  
  if (iupmotColorMap()) 
    iupmotSetArg(args[num_args++], XmNcolormap, iupmotColorMap())

  if (mwm_decor != 0x7E) 
    iupmotSetArg(args[num_args++], XmNmwmDecorations, mwm_decor)

  if (iupAttribGetIntDefault(ih, "SAVEUNDER"))
    iupmotSetArg(args[num_args++], XmNsaveUnder, True)

  parent = iupDialogGetNativeParent(ih);
  if (parent)
    ih->handle = XtCreatePopupShell(NULL, topLevelShellWidgetClass, (Widget)parent, args, num_args);
  else
    ih->handle = XtAppCreateShell(NULL, "dialog", topLevelShellWidgetClass, iupmot_display, args, num_args);

  if (!ih->handle)
    return IUP_ERROR;

  XmAddWMProtocolCallback(ih->handle, iupmot_wm_deletewindow, motDialogCBclose, (XtPointer)ih);

  XtAddEventHandler(ih->handle, FocusChangeMask, False, (XtEventHandler)iupmotFocusChangeEvent,      (XtPointer)ih);
  XtAddEventHandler(ih->handle, EnterWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, LeaveWindowMask, False, (XtEventHandler)iupmotEnterLeaveWindowEvent, (XtPointer)ih);
  XtAddEventHandler(ih->handle, StructureNotifyMask, False, (XtEventHandler)motDialogCBStructureNotifyEvent, (XtPointer)ih);

  XtAddCallback(ih->handle, XmNdestroyCallback, (XtCallbackProc)motDialogDestroyCallback, (XtPointer)ih);

  /*****************************/
  /* Create the dialog manager */
  /*****************************/

  dialog_manager = XtVaCreateManagedWidget(
              "dialog_manager",
              xmBulletinBoardWidgetClass,
              ih->handle,
              XmNmarginWidth, 0,
              XmNmarginHeight, 0,
              XmNwidth, 100,     /* set this to avoid size calculation problems */
              XmNheight, 100,
              XmNborderWidth, 0,
              XmNshadowThickness, 0,
              XmNnoResize, iupAttribGetIntDefault(ih, "RESIZE")? False: True,
              XmNresizePolicy, XmRESIZE_NONE, /* no automatic resize of children */
              XmNuserData, ih, /* used only in motDialogConfigureNotify                   */
              XmNnavigationType, XmTAB_GROUP,
              NULL);

  XtOverrideTranslations(dialog_manager, XtParseTranslationTable("<Configure>: iupDialogConfigure()"));
  XtAddCallback(dialog_manager, XmNhelpCallback, (XtCallbackProc)iupmotHelpCallback, (XtPointer)ih);
  XtAddEventHandler(dialog_manager, KeyPressMask, False,(XtEventHandler)iupmotKeyPressEvent, (XtPointer)ih);

  /* force the BGCOLOR to match the DLGBGCOLOR */
  motDialogSetBgColorAttrib(ih, IupGetGlobal("DLGBGCOLOR"));

  /* initialize the widget */
  XtRealizeWidget(ih->handle);

  /* child dialogs must be always on top of the parent */
  if (parent)
    XSetTransientForHint(iupmot_display, XtWindow(ih->handle), XtWindow(parent));

  if (mwm_decor != 0x7E)  /* some decoration was changed */
    motDialogSetWindowManagerStyle(ih);

  /* Ignore VISIBLE before mapping */
  iupAttribSetStr(ih, "VISIBLE", NULL);

  if (IupGetGlobal("_IUP_SET_DLGFGCOLOR"))
  {
    iupmotSetGlobalColorAttrib(dialog_manager, XmNforeground, "DLGFGCOLOR");
    IupSetGlobal("_IUP_SET_DLGFGCOLOR", NULL);
  }

  return IUP_NOERROR;
}

static void motDialogUnMapMethod(Ihandle* ih)
{
  Widget dialog_manager;
  if (ih->data->menu) 
  {
    ih->data->menu->handle = NULL; /* the dialog will destroy the native menu */
    IupDestroy(ih->data->menu);  
  }

  dialog_manager = XtNameToWidget(ih->handle, "*dialog_manager");
  XtVaSetValues(dialog_manager, XmNuserData, NULL, NULL);

  XtRemoveEventHandler(ih->handle, FocusChangeMask, False, (XtEventHandler)iupmotFocusChangeEvent, (XtPointer)ih);
  XtRemoveEventHandler(ih->handle, KeyPressMask, False, (XtEventHandler)iupmotKeyPressEvent, (XtPointer)ih);
  XtRemoveEventHandler(ih->handle, StructureNotifyMask, False, (XtEventHandler)motDialogCBStructureNotifyEvent, (XtPointer)ih);
  XtRemoveCallback(ih->handle, XmNdestroyCallback, (XtCallbackProc)motDialogDestroyCallback, (XtPointer)ih);

  XtRemoveEventHandler(dialog_manager, KeyPressMask, False, (XtEventHandler)iupmotKeyPressEvent, (XtPointer)ih);
  XtRemoveCallback(dialog_manager, XmNhelpCallback, (XtCallbackProc)iupmotHelpCallback, (XtPointer)ih);
  
  iupdrvBaseUnMapMethod(ih);
}

static void motDialogLayoutUpdateMethod(Ihandle *ih)
{
  int border, caption, menu;

  if (ih->data->ignore_resize ||
      iupAttribGetStr(ih, "_IUPMOT_FS_STYLE"))
    return;

  /* for dialogs the position is not updated here */
  ih->data->ignore_resize = 1;

  iupdrvDialogGetDecoration(ih, &border, &caption, &menu);

  if (!iupAttribGetIntDefault(ih, "RESIZE"))
  {
    int width = ih->currentwidth - 2*border;
    int height = ih->currentheight - 2*border - caption;
    XtVaSetValues(ih->handle,
      XmNwidth, width,
      XmNheight, height,
      XmNminWidth, width, 
      XmNminHeight, height, 
      XmNmaxWidth, width, 
      XmNmaxHeight, height, 
      NULL);
  }
  else
  {
    XtVaSetValues(ih->handle,
      XmNwidth, (XtArgVal)(ih->currentwidth - 2*border),     /* excluding the border */
      XmNheight, (XtArgVal)(ih->currentheight - 2*border - caption),
      NULL);
  }

  ih->data->ignore_resize = 0;
}


/****************************************************************************
                                   Attributes
****************************************************************************/


static int motDialogSetMinSizeAttrib(Ihandle* ih, const char* value)
{
  int decorwidth = 0, decorheight = 0;
  int min_w = 1, min_h = 1;          /* MINSIZE default value */
  iupStrToIntInt(value, &min_w, &min_h, 'x');

  /* The minmax size restricts the client area */
  iupDialogGetDecorSize(ih, &decorwidth, &decorheight);

  if (min_w > decorwidth)
    XtVaSetValues(ih->handle, XmNminWidth, min_w-decorwidth, NULL);
  if (min_h > decorheight)
    XtVaSetValues(ih->handle, XmNminHeight, min_h-decorheight, NULL);  

  return 1;
}

static int motDialogSetMaxSizeAttrib(Ihandle* ih, const char* value)
{
  int decorwidth = 0, decorheight = 0;
  int max_w = 65535, max_h = 65535;  /* MAXSIZE default value */
  iupStrToIntInt(value, &max_w, &max_h, 'x');

  /* The minmax size restricts the client area */
  iupDialogGetDecorSize(ih, &decorwidth, &decorheight);

  if (max_w > decorwidth)
    XtVaSetValues(ih->handle, XmNmaxWidth, max_w-decorwidth, NULL);  
  if (max_h > decorheight)
    XtVaSetValues(ih->handle, XmNmaxHeight, max_h-decorheight, NULL);  

  return 1;
}

static char* motDialogGetXAttrib(Ihandle *ih)
{
  char* str = iupStrGetMemory(20);
 
  int x;
  iupdrvDialogGetPosition(ih->handle, &x, NULL);

  sprintf(str, "%d", x);
  return str;
}

static char* motDialogGetYAttrib(Ihandle *ih)
{
  char* str = iupStrGetMemory(20);
 
  int y;
  iupdrvDialogGetPosition(ih->handle, &y, NULL);

  sprintf(str, "%d", y);
  return str;
}

static int motDialogSetTitleAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    value = "";

  XtVaSetValues(ih->handle, XmNtitle, value, 
                                    XmNiconName, value, 
                                    NULL);
  return 0;
}

static char* motDialogGetTitleAttrib(Ihandle* ih)
{
  char* title;
  XtVaGetValues(ih->handle, XmNtitle, &title, NULL);

  if (!title || title[0] == 0)
    return NULL;
  else
  {
    char* str = iupStrGetMemory(200);
    strcpy(str, title);
    return str;
  }
}    

static char* motDialogGetClientSizeAttrib(Ihandle *ih)
{
  char* str = iupStrGetMemory(20);
 
  Dimension manager_width, manager_height;
  Widget dialog_manager = XtNameToWidget(ih->handle, "*dialog_manager");
  XtVaGetValues(dialog_manager, XmNwidth,  &manager_width,
                         XmNheight, &manager_height, 
                         NULL);

  sprintf(str, "%dx%d", (int)manager_width, (int)manager_height - motDialogGetMenuSize(ih));
  return str;
}

static int motDialogSetBgColorAttrib(Ihandle* ih, const char* value)
{
  Pixel color = iupmotColorGetPixelStr(value);
  if (color != (Pixel)-1)
  {
    Widget dialog_manager = XtNameToWidget(ih->handle, "*dialog_manager");
    XtVaSetValues(dialog_manager, XmNbackground, color, NULL);
    XtVaSetValues(dialog_manager, XmNbackgroundPixmap, XmUNSPECIFIED_PIXMAP, NULL);
    return 1;
  }
  return 0; 
}

static int motDialogSetBackgroundAttrib(Ihandle* ih, const char* value)
{
  if (motDialogSetBgColorAttrib(ih, value))
    return 1;
  else                                     
  {
    Pixmap pixmap = (Pixmap)iupImageGetImage(value, ih, 0, "BACKGROUND");
    if (pixmap)
    {
      Widget dialog_manager = XtNameToWidget(ih->handle, "*dialog_manager");
      XtVaSetValues(dialog_manager, XmNbackgroundPixmap, pixmap, NULL);
      return 1;
    }
  }
  return 0; 
}

static int motDialogSetFullScreenAttrib(Ihandle* ih, const char* value)
{                       
  if (iupStrBoolean(value))
  {
    if (!iupAttribGetStr(ih, "_IUPMOT_FS_STYLE"))
    {
      int visible = iupdrvDialogIsVisible(ih);
      if (visible)
        iupAttribSetStr(ih, "_IUPMOT_FS_STYLE", "VISIBLE");
      else
        iupAttribSetStr(ih, "_IUPMOT_FS_STYLE", "HIDDEN");

      /* save the previous decoration attributes */
      /* during fullscreen these attributes can be consulted by the application */
      iupAttribStoreStr(ih, "_IUPMOT_FS_MAXBOX", iupAttribGetStr(ih, "MAXBOX"));
      iupAttribStoreStr(ih, "_IUPMOT_FS_MINBOX", iupAttribGetStr(ih, "MINBOX"));
      iupAttribStoreStr(ih, "_IUPMOT_FS_MENUBOX",iupAttribGetStr(ih, "MENUBOX"));
      iupAttribStoreStr(ih, "_IUPMOT_FS_RESIZE", iupAttribGetStr(ih, "RESIZE"));
      iupAttribStoreStr(ih, "_IUPMOT_FS_BORDER", iupAttribGetStr(ih, "BORDER"));
      iupAttribStoreStr(ih, "_IUPMOT_FS_TITLE",  IupGetAttribute(ih, "TITLE"));  /* must use IupGetAttribute to check from the native implementation */

      /* remove the decorations attributes */
      iupAttribSetStr(ih, "MAXBOX", "NO");
      iupAttribSetStr(ih, "MINBOX", "NO");
      iupAttribSetStr(ih, "MENUBOX", "NO");
      IupSetAttribute(ih, "TITLE", NULL); iupAttribSetStr(ih, "TITLE", NULL); /* remove from the hash table if we are during IupMap */
      iupAttribSetStr(ih, "RESIZE", "NO");
      iupAttribSetStr(ih, "BORDER", "NO");

      /* use WM fullscreen support */
      if (!motDialogSetFullScreen(ih, 1))
      {
        /* or configure fullscreen manually */
        int decor;
        int width, height;

        /* hide before changing decorations */
        if (visible)
        {
          iupAttribSetStr(ih, "_IUPMOT_SHOW_STATE", NULL);  /* To avoid a SHOW_CB notification */
          XtUnmapWidget(ih->handle);
        }

        /* save the previous position and size */
        iupAttribStoreStr(ih, "_IUPMOT_FS_X", IupGetAttribute(ih, "X"));  /* must use IupGetAttribute to check from the native implementation */
        iupAttribStoreStr(ih, "_IUPMOT_FS_Y", IupGetAttribute(ih, "Y"));
        iupAttribStoreStr(ih, "_IUPMOT_FS_SIZE", IupGetAttribute(ih, "RASTERSIZE"));

        /* save the previous decoration */
        XtVaGetValues(ih->handle, XmNmwmDecorations, &decor, NULL);
        iupAttribSetStr(ih, "_IUPMOT_FS_DECOR", (char*)decor);

        /* remove the decorations */
        XtVaSetValues(ih->handle, XmNmwmDecorations, (XtArgVal)0, NULL);
        motDialogSetWindowManagerStyle(ih);

        /* get full screen size */
        iupdrvGetFullSize(&width, &height);

        /* set position and size */
        XtVaSetValues(ih->handle, XmNwidth,  (XtArgVal)width,
                                          XmNheight, (XtArgVal)height, 
                                          XmNx,      (XtArgVal)0,
                                          XmNy,      (XtArgVal)0,
                                          NULL);

        /* layout will be updated in motDialogConfigureNotify */
        if (visible)
          XtMapWidget(ih->handle);
      }
    }
  }
  else
  {
    char* fs_style = iupAttribGetStr(ih, "_IUPMOT_FS_STYLE");
    if (fs_style)
    {
      /* can only switch back from full screen if window was visible */
      /* when fullscreen was set */
      if (iupStrEqualNoCase(fs_style, "VISIBLE"))
      {
        iupAttribSetStr(ih, "_IUPMOT_FS_STYLE", NULL);

        /* restore the decorations attributes */
        iupAttribStoreStr(ih, "MAXBOX", iupAttribGetStr(ih, "_IUPMOT_FS_MAXBOX"));
        iupAttribStoreStr(ih, "MINBOX", iupAttribGetStr(ih, "_IUPMOT_FS_MINBOX"));
        iupAttribStoreStr(ih, "MENUBOX",iupAttribGetStr(ih, "_IUPMOT_FS_MENUBOX"));
        IupSetAttribute(ih, "TITLE",  iupAttribGetStr(ih, "_IUPMOT_FS_TITLE"));   /* TITLE is not stored in the HashTable */
        iupAttribStoreStr(ih, "RESIZE", iupAttribGetStr(ih, "_IUPMOT_FS_RESIZE"));
        iupAttribStoreStr(ih, "BORDER", iupAttribGetStr(ih, "_IUPMOT_FS_BORDER"));

        if (!motDialogSetFullScreen(ih, 0))
        {
          int border, caption, menu, x, y;
          int visible = iupdrvDialogIsVisible(ih);
          if (visible)
            XtUnmapWidget(ih->handle);

          /* restore the decorations */
          XtVaSetValues(ih->handle, XmNmwmDecorations, (XtArgVal)(int)iupAttribGetStr(ih, "_IUPMOT_FS_DECOR"), NULL);
          motDialogSetWindowManagerStyle(ih);

          /* the dialog decoration will not be considered yet in the next XtVaSetValues */
          /* so compensate the decoration when restoring the position and size */
          iupdrvDialogGetDecoration(ih, &border, &caption, &menu);
          x = iupAttribGetInt(ih, "_IUPMOT_FS_X") - (border);
          y = iupAttribGetInt(ih, "_IUPMOT_FS_Y") - (border+caption+menu);

          /* restore position and size */
          XtVaSetValues(ih->handle, 
                      XmNx,      (XtArgVal)x, 
                      XmNy,      (XtArgVal)y, 
                      XmNwidth,  (XtArgVal)(IupGetInt(ih, "_IUPMOT_FS_SIZE") - (2*border)), 
                      XmNheight, (XtArgVal)(IupGetInt2(ih, "_IUPMOT_FS_SIZE") - (2*border+caption)), 
                      NULL);

          /* layout will be updated in motDialogConfigureNotify */
          if (visible)
            XtMapWidget(ih->handle);

          /* remove auxiliar attributes */
          iupAttribSetStr(ih, "_IUPMOT_FS_X", NULL);
          iupAttribSetStr(ih, "_IUPMOT_FS_Y", NULL);
          iupAttribSetStr(ih, "_IUPMOT_FS_SIZE", NULL);
          iupAttribSetStr(ih, "_IUPMOT_FS_DECOR", NULL);
        }

        /* remove auxiliar attributes */
        iupAttribSetStr(ih, "_IUPMOT_FS_MAXBOX", NULL);
        iupAttribSetStr(ih, "_IUPMOT_FS_MINBOX", NULL);
        iupAttribSetStr(ih, "_IUPMOT_FS_MENUBOX",NULL);
        iupAttribSetStr(ih, "_IUPMOT_FS_RESIZE", NULL);
        iupAttribSetStr(ih, "_IUPMOT_FS_BORDER", NULL);
        iupAttribSetStr(ih, "_IUPMOT_FS_TITLE",  NULL);
      }
    }
  }
  return 1;
}

static int motDialogSetIconAttrib(Ihandle* ih, const char *value)
{
  if (!value)
    XtVaSetValues(ih->handle, XmNiconPixmap, NULL, NULL);
  else
  {
    Pixmap icon = (Pixmap)iupImageGetIcon(value);
    if (icon)
      XtVaSetValues(ih->handle, XmNiconPixmap, icon, NULL);
  }
  return 1;
}

void iupdrvDialogInitClass(Iclass* ic)
{
  /* Driver Dependent Class methods */
  ic->Map = motDialogMapMethod;
  ic->UnMap = motDialogUnMapMethod;
  ic->LayoutUpdate = motDialogLayoutUpdateMethod;
  ic->SetPosition = motDialogSetPositionMethod;
  ic->GetInnerNativeContainerHandle = motDialogGetInnerNativeContainerMethod;

  if (!iupmot_wm_deletewindow)
  {
    /* Set up a translation table that captures "Resize" events
      (also called ConfigureNotify or Configure events). */
    XtActionsRec rec = {"iupDialogConfigure", motDialogConfigureNotify};
    XtAppAddActions(iupmot_appcontext, &rec, 1);

    /* register atom to intercept the close button in the window frame */
    iupmot_wm_deletewindow = XmInternAtom(iupmot_display, "WM_DELETE_WINDOW", False);
  }

  /* Driver Dependent Attribute functions */

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, motDialogSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* Overwrite Visual */
  iupClassRegisterAttribute(ic, "X", motDialogGetXAttrib, iupBaseNoSetAttrib, "0", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "Y", motDialogGetYAttrib, iupBaseNoSetAttrib, "0", IUP_MAPPED, IUP_NO_INHERIT);

  /* Base Container */
  iupClassRegisterAttribute(ic, "CLIENTSIZE", motDialogGetClientSizeAttrib, iupBaseNoSetAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  /* Special */
  iupClassRegisterAttribute(ic, "TITLE", motDialogGetTitleAttrib, motDialogSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  /* IupDialog only */
  iupClassRegisterAttribute(ic, "BACKGROUND", NULL, motDialogSetBackgroundAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "ICON", NULL, motDialogSetIconAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FULLSCREEN", NULL, motDialogSetFullScreenAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MINSIZE", NULL, motDialogSetMinSizeAttrib, "1x1", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MAXSIZE", NULL, motDialogSetMaxSizeAttrib, "65535x65535", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SAVEUNDER", NULL, NULL, "YES", IUP_MAPPED, IUP_NO_INHERIT);

  /* IupDialog X Only */
  iupClassRegisterAttribute(ic, "XWINDOW", iupmotGetXWindowAttrib, NULL, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}
