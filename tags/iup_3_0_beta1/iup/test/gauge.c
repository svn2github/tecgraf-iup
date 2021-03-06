#include <stdio.h>

#include "iup.h"
#include "iupcontrols.h"

float increment = 0.01f;
Ihandle *gauge;
Ihandle *btn_pause;
Ihandle *timer;

static unsigned char pixmap_play[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static unsigned char pixmap_restart[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static unsigned char pixmap_rewind[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static unsigned char pixmap_forward[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static unsigned char pixmap_pause[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

void createimg_s (void)
{ 
  Ihandle *img_restart, *img_play, *img_forward, *img_rewind, *img_pause;
  
  img_restart = IupImage(22,22, pixmap_restart);
  img_play = IupImage(22,22, pixmap_play);
  img_forward = IupImage(22,22, pixmap_forward);
  img_rewind = IupImage(22,22, pixmap_rewind);
  img_pause = IupImage(22,22, pixmap_pause);
  
  IupSetHandle ("img_restart", img_restart); 
  IupSetHandle ("img_play", img_play);
  IupSetHandle ("img_forward", img_forward); 
  IupSetHandle ("img_rewind", img_rewind);
  IupSetHandle ("img_pause", img_pause);
  
  IupSetAttribute (img_restart, "1", "0 0 0"); 
  IupSetAttribute (img_restart, "2", "BGCOLOR");
  IupSetAttribute (img_play, "1", "0 0 0"); 
  IupSetAttribute (img_play, "2", "BGCOLOR"); 
  IupSetAttribute (img_forward, "1", "0 0 0"); 
  IupSetAttribute (img_forward, "2", "BGCOLOR");
  IupSetAttribute (img_rewind, "1", "0 0 0"); 
  IupSetAttribute (img_rewind, "2", "BGCOLOR"); 
  IupSetAttribute (img_pause, "1", "0 0 0"); 
  IupSetAttribute (img_pause, "2", "BGCOLOR");
}

int time_cb(void)
{
  float value = IupGetFloat(gauge, "VALUE");
  value += increment;
  if (value > 1) value = 0; /* start over */
  IupSetfAttribute(gauge, "VALUE", "%f", (double)value);
  return IUP_DEFAULT;
}

int btn_pause_cb(void)
{
  if (!IupGetInt(timer, "RUN"))
  {
    IupSetAttribute(timer, "RUN", "YES");
    IupSetAttribute(btn_pause, "IMAGE", "img_pause");
  }
  else
  {
    IupSetAttribute(timer, "RUN", "NO");
    IupSetAttribute(btn_pause, "IMAGE", "img_play");
  }
  
  return IUP_DEFAULT;
}

int btn_restart_cb(void)
{
  IupSetAttribute(gauge, "VALUE", "0");
  return IUP_DEFAULT;
}

int btn_accelerate_cb(void)
{
  increment *= 2;
  return IUP_DEFAULT;
}

int btn_decelerate_cb(void)
{
  increment /= 2;
  return IUP_DEFAULT;
}

int btn_show1_cb(void)
{
  if (!IupGetInt(gauge, "DASHED"))
    IupSetAttribute(gauge, "DASHED", "YES");
  else
    IupSetAttribute(gauge, "DASHED", "NO");
  
  return IUP_DEFAULT;
}

int btn_show2_cb(void)
{
  if (!IupGetInt(gauge, "SHOW_TEXT"))
    IupSetAttribute(gauge, "SHOW_TEXT", "YES");
  else
    IupSetAttribute(gauge, "SHOW_TEXT", "NO");
  
  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  Ihandle *dlg, *vbox, *hbox;
  Ihandle *btn_restart, *btn_accelerate, *btn_decelerate, *btn_show1, *btn_show2;
  
  IupOpen(&argc, &argv);                  /* IUP initialization */
  IupControlsOpen();                      /* Initializes the controls library */

  timer = IupTimer();
  IupSetCallback(timer, "ACTION_CB", (Icallback)time_cb);
  IupSetAttribute(timer, "TIME", "100");
  
  gauge = IupGauge();
 
  IupSetAttribute(gauge, "EXPAND", "YES");
//  IupSetAttribute(gauge, "BGCOLOR", "255 0 128");
//  IupSetAttribute(gauge, "FGCOLOR", "0 128 0");

  btn_restart = IupButton(NULL, NULL);
  btn_pause = IupButton(NULL, NULL);
  btn_accelerate = IupButton(NULL, NULL);
  btn_decelerate = IupButton(NULL, NULL);
  btn_show1 = IupButton("Dashed", NULL);
  btn_show2 = IupButton("Text", NULL);

  createimg_s();

  IupSetAttribute(btn_restart, "IMAGE", "img_restart");
  IupSetAttribute(btn_restart, "TIP", "Restart" );
  IupSetAttribute(btn_pause, "IMAGE", "img_pause");
  IupSetAttribute(btn_pause, "TIP", "Play/Pause");
  IupSetAttribute(btn_accelerate, "IMAGE", "img_forward");
  IupSetAttribute(btn_accelerate, "TIP", "Accelerate");
  IupSetAttribute(btn_decelerate, "IMAGE", "img_rewind");
  IupSetAttribute(btn_decelerate, "TIP", "Decelerate");
  IupSetAttribute(btn_show1, "TIP", "Dashed or Continuous");
  IupSetAttribute(btn_show2, "TIP", "Text or None");

  hbox = IupHbox
         (
           IupFill(), 
           btn_pause,
           btn_restart,
           btn_decelerate,
           btn_accelerate,
           btn_show1,
           btn_show2,
           IupFill(),
           NULL
         );

  vbox = IupVbox(gauge, hbox, NULL);
  IupSetAttribute(vbox, "MARGIN", "10x10");
  IupSetAttribute(vbox, "GAP", "5");

  dlg = IupDialog(vbox);
  
  IupSetAttribute(dlg, "TITLE", "IupGauge Test");

  IupSetCallback(btn_pause, "ACTION", (Icallback) btn_pause_cb);
  IupSetCallback(btn_restart, "ACTION", (Icallback) btn_restart_cb);
  IupSetCallback(btn_accelerate, "ACTION", (Icallback) btn_accelerate_cb);
  IupSetCallback(btn_decelerate, "ACTION", (Icallback) btn_decelerate_cb);
  IupSetCallback(btn_show1, "ACTION", (Icallback) btn_show1_cb);
  IupSetCallback(btn_show2, "ACTION", (Icallback) btn_show2_cb);
  
  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);

  IupSetAttribute(timer, "RUN", "YES");

  IupMainLoop();
  
  IupDestroy(dlg);
  IupDestroy(IupGetHandle ("img_restart")); 
  IupDestroy(IupGetHandle ("img_play"));
  IupDestroy(IupGetHandle ("img_forward")); 
  IupDestroy(IupGetHandle ("img_rewind"));
  IupDestroy(IupGetHandle ("img_pause"));

  IupClose();

  return 0;
}
