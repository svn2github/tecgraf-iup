#undef GTK_DISABLE_DEPRECATED
#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_progressbar.h"
#include "iup_drv.h"

#include "iupgtk_drv.h"


static int gtkProgressBarSetMarqueeAttrib(Ihandle* ih, const char* value)
{
  GtkProgress* progress = (GtkProgress*)ih->handle;

  if (iupStrBoolean(value))
  {
    ih->data->marquee = 1;
    gtk_progress_set_activity_mode(progress, TRUE);
  }
  else
  {
    gtk_progress_set_activity_mode(progress, FALSE);
    ih->data->marquee = 0;
  }

  return 1;
}

static int gtkProgressBarSetValueAttrib(Ihandle* ih, const char* value)
{
  GtkProgressBar* pbar = (GtkProgressBar*)ih->handle;

  if (!value)
    ih->data->value = 0;
  else
    ih->data->value = atof(value);
  iProgressBarCropValue(ih);

  if (ih->data->marquee)
    gtk_progress_bar_pulse(pbar);
  else
    gtk_progress_bar_set_fraction(pbar, (ih->data->value - ih->data->vmin) / (ih->data->vmax - ih->data->vmin));

  return 0;
}

static int gtkProgressBarSetDashedAttrib(Ihandle* ih, const char* value)
{
  GtkProgressBar* pbar = (GtkProgressBar*)ih->handle;

  /* gtk_progress_bar_set_bar_style is deprecated */
  if (iupStrBoolean(value))
  {
    ih->data->dashed = 1;
    gtk_progress_bar_set_bar_style(pbar, GTK_PROGRESS_DISCRETE);
  }
  else /* Default */
  {
    ih->data->dashed = 0;
    gtk_progress_bar_set_bar_style(pbar, GTK_PROGRESS_CONTINUOUS);
  }

  return 0;
}

static int gtkProgressBarMapMethod(Ihandle* ih)
{
  ih->handle = gtk_progress_bar_new();
  if (!ih->handle)
    return IUP_ERROR;

  /* add to the parent, all GTK controls must call this. */
  iupgtkBaseAddToParent(ih);

  gtk_widget_realize(ih->handle);

  if (iupStrEqualNoCase(iupAttribGetStr(ih, "ORIENTATION"), "VERTICAL"))
  {
    gtk_progress_bar_set_orientation((GtkProgressBar*)ih->handle, GTK_PROGRESS_BOTTOM_TO_TOP);

    if (ih->currentheight < ih->currentwidth)
    {
      int tmp = ih->currentheight;
      ih->currentheight = ih->currentwidth;
      ih->currentwidth = tmp;
    }
  }
  else
    gtk_progress_bar_set_orientation((GtkProgressBar*)ih->handle, GTK_PROGRESS_LEFT_TO_RIGHT);

  return IUP_NOERROR;
}

void iupdrvProgressBarInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkProgressBarMapMethod;

  /* Driver Dependent Attribute functions */
  
  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, iupdrvBaseSetBgColorAttrib, "DLGBGCOLOR", IUP_MAPPED, IUP_INHERIT);
  
  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iupdrvBaseSetFgColorAttrib, "0 0 0", IUP_MAPPED, IUP_INHERIT);

  /* IupProgressBar only */
  iupClassRegisterAttribute(ic, "VALUE",  iProgressBarGetValueAttrib,  gtkProgressBarSetValueAttrib,  NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DASHED", iProgressBarGetDashedAttrib, gtkProgressBarSetDashedAttrib, "NO", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "ORIENTATION", NULL, NULL, "HORIZONTAL", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MARQUEE",     NULL, gtkProgressBarSetMarqueeAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
}
