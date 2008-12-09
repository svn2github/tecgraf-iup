/** \file
 * \brief Label Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>

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
#include "iup_image.h"

#include "iupmot_drv.h"


static int motLabelSetTitleAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_LABEL_TEXT)
  {
    iupmotSetMnemonicTitle(ih, value);
    return 1;
  }

  return 0;
}

static int motLabelSetBgColorAttrib(Ihandle* ih, const char* value)
{
  /* ignore given value, must use only from parent */
  value = iupAttribGetStrNativeParent(ih, "BGCOLOR");
  if (!value) value = IupGetGlobal("DLGBGCOLOR");
  if (iupdrvBaseSetBgColorAttrib(ih, value))
    return 1;
  return 0; 
}

static int motLabelSetBackgroundAttrib(Ihandle* ih, const char* value)
{
  /* ignore given value, must use only from parent */
  value = iupAttribGetStrNativeParent(ih, "BACKGROUND");

  if (iupdrvBaseSetBgColorAttrib(ih, value))
    return 1;
  else
  {
    Pixmap pixmap = (Pixmap)iupImageGetImage(value, ih, 0, "BACKGROUND");
    if (pixmap)
    {
      XtVaSetValues(ih->handle, XmNbackgroundPixmap, pixmap, NULL);
      return 1;
    }
  }
  return 0; 
}

static int motLabelSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type != IUP_LABEL_SEP_HORIZ && ih->data->type != IUP_LABEL_SEP_VERT)
  {
    unsigned char align;
    char value1[30]="", value2[30]="";

    iupStrToStrStr(value, value1, value2, ':');   /* value2 is ignored, NOT supported in Motif */

    if (iupStrEqualNoCase(value1, "ARIGHT"))
      align = XmALIGNMENT_END;
    else if (iupStrEqualNoCase(value1, "ACENTER"))
      align = XmALIGNMENT_CENTER;
    else /* "ALEFT" */
      align = XmALIGNMENT_BEGINNING;

    XtVaSetValues(ih->handle, XmNalignment, align, NULL);
    return 1;
  }
  else
    return 0;
}

static int motLabelSetImageAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_LABEL_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNlabelPixmap, 0, "IMAGE");

    if (!iupdrvIsActive(ih))
    {
      if (!iupAttribGetStr(ih, "IMINACTIVE"))
      {
        /* if not active and IMINACTIVE is not defined 
           then automaticaly create one based on IMAGE */
        iupmotSetPixmap(ih, value, XmNlabelInsensitivePixmap, 1, "IMINACTIVE"); /* make_inactive */
      }
    }
    return 1;
  }
  else
    return 0;
}

static int motLabelSetImInactiveAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->type == IUP_LABEL_IMAGE)
  {
    iupmotSetPixmap(ih, value, XmNlabelInsensitivePixmap, 0, "IMINACTIVE");
    return 1;
  }
  else
    return 0;
}

static int motLabelSetActiveAttrib(Ihandle* ih, const char* value)
{
  /* update the inactive image if necessary */
  if (ih->data->type == IUP_LABEL_IMAGE && !iupStrBoolean(value))
  {
    if (!iupAttribGetStr(ih, "IMINACTIVE"))
    {
      /* if not defined then automaticaly create one based on IMAGE */
      char* name = iupAttribGetStr(ih, "IMAGE");
      iupmotSetPixmap(ih, name, XmNlabelInsensitivePixmap, 1, "IMINACTIVE"); /* make_inactive */
    }
  }

  return iupBaseSetActiveAttrib(ih, value);
}

static int motLabelSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle && ih->data->type != IUP_LABEL_SEP_HORIZ && ih->data->type != IUP_LABEL_SEP_VERT)
  {
    XtVaSetValues(ih->handle, XmNmarginHeight, ih->data->vert_padding,
                              XmNmarginWidth, ih->data->horiz_padding, NULL);
  }
  return 0;
}

static int motLabelMapMethod(Ihandle* ih)
{
  char* value;
  int num_args = 0;
  Arg args[20];
  WidgetClass widget_class;

  value = iupAttribGetStr(ih, "SEPARATOR");
  if (value)
  {
    widget_class = xmSeparatorWidgetClass;
    if (iupStrEqualNoCase(value, "HORIZONTAL"))
    {
      ih->data->type = IUP_LABEL_SEP_HORIZ;
      iupmotSetArg(args[num_args++], XmNorientation, XmHORIZONTAL)
    }
    else /* "VERTICAL" */
    {
      ih->data->type = IUP_LABEL_SEP_VERT;
      iupmotSetArg(args[num_args++], XmNorientation, XmVERTICAL)
    }
  }
  else
  {
    value = iupAttribGetStr(ih, "IMAGE");
    widget_class = xmLabelWidgetClass;
    if (value)
    {
      ih->data->type = IUP_LABEL_IMAGE;
      iupmotSetArg(args[num_args++], XmNlabelType, XmPIXMAP) 
    }
    else
    {
      ih->data->type = IUP_LABEL_TEXT;
      iupmotSetArg(args[num_args++], XmNlabelType, XmSTRING) 
    }
  }

  /* Core */
  iupmotSetArg(args[num_args++], XmNmappedWhenManaged, False);  /* not visible when managed */
  iupmotSetArg(args[num_args++], XmNx, 0);  /* x-position */
  iupmotSetArg(args[num_args++], XmNy, 0);  /* y-position */
  iupmotSetArg(args[num_args++], XmNwidth, 10);  /* default width to avoid 0 */
  iupmotSetArg(args[num_args++], XmNheight, 10); /* default height to avoid 0 */
  /* Primitive */
  iupmotSetArg(args[num_args++], XmNtraversalOn, False);
  iupmotSetArg(args[num_args++], XmNhighlightThickness, 0);
  /* Label */
  iupmotSetArg(args[num_args++], XmNrecomputeSize, False);  /* no automatic resize from text */
  iupmotSetArg(args[num_args++], XmNmarginHeight, 0);  /* default padding */
  iupmotSetArg(args[num_args++], XmNmarginWidth, 0);
  iupmotSetArg(args[num_args++], XmNmarginTop, 0);     /* no extra margins */
  iupmotSetArg(args[num_args++], XmNmarginLeft, 0);
  iupmotSetArg(args[num_args++], XmNmarginBottom, 0);
  iupmotSetArg(args[num_args++], XmNmarginRight, 0);
  
  ih->handle = XtCreateManagedWidget(
    iupDialogGetChildIdStr(ih),  /* child identifier */
    widget_class,                /* widget class */
    iupChildTreeGetNativeParentHandle(ih), /* widget parent */
    args, num_args);

  if (!ih->handle)
    return IUP_ERROR;

  ih->serial = iupDialogGetChildId(ih); /* must be after using the string */

  /* Drag Source is enabled by default in label */
  iupmotDisableDragSource(ih->handle);

  /* initialize the widget */
  XtRealizeWidget(ih->handle);

  /* ensure the default values, that are different from the native ones */
  motLabelSetAlignmentAttrib(ih, iupAttribGetStrDefault(ih, "ALIGNMENT"));

  if (ih->data->type == IUP_LABEL_TEXT)
    iupmotSetString(ih->handle, XmNlabelString, "");

  return IUP_NOERROR;
}

void iupdrvLabelInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = motLabelMapMethod;

  /* Driver Dependent Attribute functions */

  /* Overwrite Visual */
  iupClassRegisterAttribute(ic, "ACTIVE", iupBaseGetActiveAttrib, motLabelSetActiveAttrib, "YES", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "BGCOLOR", iupmotGetBgColorAttrib, motLabelSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "BACKGROUND", NULL, motLabelSetBackgroundAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iupdrvBaseSetFgColorAttrib, "0 0 0", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "TITLE", NULL, motLabelSetTitleAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  /* IupLabel only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", NULL, motLabelSetAlignmentAttrib, "ALEFT:ACENTER", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMAGE", NULL, motLabelSetImageAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "IMINACTIVE", NULL, motLabelSetImInactiveAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  iupClassRegisterAttribute(ic, "PADDING", iupLabelGetPaddingAttrib, motLabelSetPaddingAttrib, "0x0", IUP_NOT_MAPPED, IUP_INHERIT);
}
