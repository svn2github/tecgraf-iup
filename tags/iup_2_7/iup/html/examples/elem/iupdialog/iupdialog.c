/*IupDialog Example in C
Creates a dialog showing an icon, the IUP_DEFAULTESC attribute and a simple menu.*/

#include <stdio.h>
#include "iup.h"

/* defines icon's image */
static unsigned char img[] =
{
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,6,6,6,6,6,6,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,5,5,5,4,2,
1,3,5,5,6,6,6,6,6,6,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,5,5,6,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,5,5,6,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,6,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,6,6,6,6,6,5,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,6,6,5,5,5,5,6,6,5,5,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,6,6,6,6,6,6,5,5,6,6,6,6,6,6,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,6,6,6,6,6,6,5,5,5,6,6,6,6,5,5,5,6,6,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,2,
1,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,
1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

int quit_cb(void)
{
  return IUP_CLOSE;
}

int main(int argc, char **argv)
{
  Ihandle *dialog, *quit_bt, *icon;
  Ihandle *menu, *submenu, *options;

  IupOpen(&argc, &argv);

  /* Creating dialog's icon */
  icon = IupImage(32, 32, img);
  IupSetAttribute (icon, "1", "255 255 255");
  IupSetAttribute (icon, "2", "000 000 000");
  IupSetAttribute (icon, "3", "226 226 226");
  IupSetAttribute (icon, "4", "128 128 128");
  IupSetAttribute (icon, "5", "192 192 192");
  IupSetAttribute (icon, "6", "000 000 255");
  IupSetHandle ("icon", icon);

  /* Creating dialog's content */ 
  quit_bt = IupButton("Quit", NULL);
  IupSetCallback(quit_bt, "ACTION", (Icallback)quit_cb);
  IupSetHandle("quit", quit_bt);

  /* Creating dialog's menu */ 
  options = IupMenu(IupItem("Exit","quit_cb"),NULL);
  submenu = IupSubmenu("File", options);
  menu = IupMenu(submenu,NULL);
  IupSetHandle("menu", menu);

  /* Creating main dialog */ 
  dialog = IupDialog(IupVbox(quit_bt,NULL));
  IupSetAttribute(dialog, IUP_TITLE, "IupDialog");
  IupSetAttribute(dialog, IUP_MENU, "menu");
  IupSetAttribute(dialog, IUP_CURSOR, "CROSS");
  IupSetAttribute(dialog, IUP_ICON, "icon");
  IupSetAttribute(dialog, IUP_DEFAULTESC, "quit");

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
  IupMainLoop();
  IupDestroy(dialog);
  IupDestroy(icon);
  IupClose();

  return 0;
}