#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "iup.h"
#include "iupcontrols.h"
#include "iupgl.h"


#define MAX_NAMES 500

#ifdef USE_IM
#include "iupim.h"
#endif

/* IupImgLin internal function, used only here */
void iupImageLibLoadAll(void);

static void SaveImageC(const char* file_name, Ihandle* elem, const char* name, FILE *packfile)
{
  int y, x, width, height, channels, linesize;
  unsigned char* data;
  FILE* file;

  if (packfile)
    file = packfile;
  else
    file = fopen(file_name, "wb");

  if (!file)
  {
    IupMessage("Error!", "Failed to open file for writing.");
    return;
  }

  width = IupGetInt(elem, "WIDTH");
  height = IupGetInt(elem, "HEIGHT");
  channels = IupGetInt(elem, "CHANNELS");
  linesize = width*channels;

  data = (unsigned char*)IupGetAttribute(elem, "WID");

  fprintf(file, "static Ihandle* load_image_%s(void)\n", name);
  fprintf(file, "{\n");
  fprintf(file, "  unsigned char imgdata[] = {\n");

  for (y = 0; y < height; y++)
  {
    fprintf(file, "    ");

    for (x = 0; x < linesize; x++)
    {
      if (x != 0)
        fprintf(file, ", ");

      fprintf(file, "%d", (int)data[y*linesize+x]);
    }

    if (y == height-1)
      fprintf(file, "};\n\n");
    else
      fprintf(file, ",\n");
  }

  if (channels == 1)
  {
    int c;

    fprintf(file, "  Ihandle* image = IupImage(%d, %d, imgdata);\n\n", width, height);

    for (c = 0; c < 256; c++)
    {
      char str[20];
      char* color;

      sprintf(str, "%d", c);
      color = IupGetAttribute(elem, str);
      if (!color)
        break;

      fprintf(file, "  IupSetAttribute(image, \"%d\", \"%s\");\n", c, color);
    }

    fprintf(file, "\n");
  }
  else if (channels == 3)
    fprintf(file, "  Ihandle* image = IupImageRGB(%d, %d, imgdata);\n", width, height);
  else /* channels == 4 */
    fprintf(file, "  Ihandle* image = IupImageRGBA(%d, %d, imgdata);\n", width, height);

  fprintf(file, "  return image;\n");
  fprintf(file, "}\n\n");

  if (!packfile)
    fclose(file);
}

static void SaveImageLua(const char* file_name, Ihandle* elem, const char* name, FILE *packfile)
{
  int y, x, width, height, channels, linesize;
  unsigned char* data;
  FILE* file;

  if (packfile)
    file = packfile;
  else
    file = fopen(file_name, "wb");

  if (!file)
  {
    IupMessage("Error!", "Failed to open file for writing.");
    return;
  }
  width = IupGetInt(elem, "WIDTH");
  height = IupGetInt(elem, "HEIGHT");
  channels = IupGetInt(elem, "CHANNELS");
  linesize = width*channels;

  data = (unsigned char*)IupGetAttribute(elem, "WID");

  fprintf(file, "function load_image_%s()\n", name);

  if (channels == 1)
    fprintf(file, "  %s = iup.image\n", name);
  else if (channels == 3)
    fprintf(file, "  %s = iup.imagergb\n", name);
  else /* channels == 4 */
    fprintf(file, "  %s = iup.imagergba\n", name);

  fprintf(file, "  {\n");

  for (y = 0; y < height; y++)
  {
    fprintf(file, "    {");

    for (x = 0; x < linesize; x++)
    {
      fprintf(file, "%d", (int)data[y*linesize+x]);

      if (x != linesize-1)
        fprintf(file, ", ");
    }

    if (y == height-1)
      fprintf(file, "};\n");
    else
      fprintf(file, "},\n");
  }

  if (channels == 1)
  {
    int c;

    fprintf(file, "    colors = \n  {\n");

    for(c = 0; c < 256; c++)
    {
      unsigned int r, g, b;
      char str[20];
      char* color;

      sprintf(str, "%d", c);
      color = IupGetAttribute(elem, str);
      if (!color)
        break;

      if (strcmp(color, "BGCOLOR") == 0)
        fprintf(file, "    \"BGCOLOR\",\n");
      else
      {
        sscanf(color, "%d %d %d", &r, &g, &b);
        fprintf(file, "    \"%d %d %d\",\n", r, g, b);
      }
    }

    fprintf(file, "    }\n");
  }

  fprintf(file, "  }\n");
  fprintf(file, "  return %s\n", name);
  fprintf(file, "end\n\n");

  if (!packfile)
    fclose(file);
}

static void SaveImageLED(const char* file_name, Ihandle* elem, const char* name, FILE *packfile)
{
  int y, x, width, height, channels, linesize;
  unsigned char* data;
  FILE* file;

  if (packfile)
    file = packfile;
  else
    file = fopen(file_name, "wb");

  if (!file)
  {
    IupMessage("Error!", "Failed to open file for writing.");
    return;
  }

  width = IupGetInt(elem, "WIDTH");
  height = IupGetInt(elem, "HEIGHT");
  channels = IupGetInt(elem, "CHANNELS");
  linesize = width*channels;

  data = (unsigned char*)IupGetAttribute(elem, "WID");

  if (channels == 1)
  {
    int c;

    fprintf(file, "%s = IMAGE\n", name);

    fprintf(file, "[\n");
    for(c = 0; c < 256; c++)
    {
      unsigned int r, g, b;
      char str[20];
      char* color;

      sprintf(str, "%d", c);
      color = IupGetAttribute(elem, str);
      if (!color)
      {
        if (c < 16)
          continue;
        else
          break;
      }

      if (c != 0)
        fprintf(file, ",\n");

      if (strcmp(color, "BGCOLOR") == 0)
        fprintf(file, "  %d = \"BGCOLOR\"", c);
      else
      {
        sscanf(color, "%d %d %d", &r, &g, &b);
        fprintf(file, "  %d = \"%d %d %d\"", c, r, g, b);
      }
    }
    fprintf(file, "\n]\n");
  }
  else if (channels == 3)
    fprintf(file, "%s = IMAGERGB\n", name);
  else /* channels == 4 */
    fprintf(file, "%s = IMAGERGBA\n", name);

  fprintf(file, "(%d, %d,\n", width, height);

  for (y = 0; y < height; y++)
  {
    fprintf(file, "  ");
    for (x = 0; x < linesize; x++)
    {
      if (y == height-1 && x==linesize-1)
        fprintf(file, "%d", (int)data[y*linesize+x]);
      else
        fprintf(file, "%d, ", (int)data[y*linesize+x]);
    }
    fprintf(file, "\n");
  }

  fprintf(file, ")\n\n");

  if (!packfile)
    fclose(file);
}

static int close_cb(void)
{
  return IUP_CLOSE;
}

static int about_cb(void)
{
  IupMessage("About", "     IupView\n\n"
                      "Show dialogs and popup menus\n"
                      "defined in LED files.\n"
                      "Can show all defined images.\n"
                      "Can import and export images.\n"
                      );
  return IUP_DEFAULT;
}

static int inactivetoggle_cb(Ihandle* self, int v)
{
  Ihandle* tabs = (Ihandle*)IupGetAttribute(self, "TABS");
  Ihandle* child = IupGetNextChild(tabs, NULL);
  while(child)
  {
    IupSetAttribute(child, "ACTIVE", v?"NO":"YES");
    child = IupGetNextChild(tabs, child);
  }
  return IUP_DEFAULT;
}

static int imagebutton_cb(Ihandle* self)
{
  Ihandle* label = (Ihandle*)IupGetAttribute(self, "_INFO_LABEL");
  IupSetAttribute(label, "TITLE", IupGetAttribute(self, "_INFO"));
  return IUP_DEFAULT;
}

static int showallimages_cb(void)
{
  Ihandle *dialog, *box, *files, *tabs, *toggle, *label;
  Ihandle* params[500];
  char *names[MAX_NAMES];
  int i, n = 0, num_names = IupGetAllNames(names, MAX_NAMES); 
  files = IupUser();
  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
#if (IUP_VERSION_NUMBER < 300000)
    char* type = IupGetClassName(elem);
#else
    char* type = IupGetClassType(elem);
#endif

    if (strcmp(type, "image") == 0)
    {
      Ihandle *tbox, *lbox, *button;

      /* show only loaded images */
      char* file_title = IupGetAttribute(elem, "_FILE_TITLE");
      if (!file_title)
        continue;

      tbox = (Ihandle*)IupGetAttribute(files, file_title);
      if (!tbox)
      {
        tbox = IupVbox(NULL);
        IupSetAttribute(files, file_title, (char*)tbox);
        IupSetAttribute(tbox, "TABTITLE", file_title);
        params[n] = tbox;
        n++;
      }

      lbox = (Ihandle*)IupGetAttribute(tbox, file_title);
      if (!lbox || IupGetInt(lbox, "LINE_COUNT") == 10)
      {
        lbox = IupHbox(NULL);
        IupAppend(tbox, lbox);
        IupSetAttribute(tbox, file_title, (char*)lbox);
        IupSetAttribute(lbox, "LINE_COUNT", "0");
      }

      button = IupButton("", NULL);
      IupSetAttribute(button, "IMAGE", names[i]);
      IupSetfAttribute(button, "_INFO", "%s [%d,%d]", names[i], IupGetInt(elem, "WIDTH"), IupGetInt(elem, "HEIGHT"));
      IupSetCallback(button, "ACTION", (Icallback)imagebutton_cb);
      IupAppend(lbox, button);
      IupSetfAttribute(lbox, "LINE_COUNT", "%d", IupGetInt(lbox, "LINE_COUNT")+1);
    }
  }

  if (n == 0)
  {
    IupMessage("Error", "No images.");
    return IUP_DEFAULT;
  }

  params[n] = NULL;

  box = IupVbox(toggle = IupToggle("INACTIVE", NULL), 
                tabs = IupTabsv(params), 
                label = IupLabel(""),
                NULL);
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");
  IupSetAttribute(tabs, "ALIGNMENT", "NW");
  IupSetAttribute(tabs, "SIZE", "150x80");
  IupSetCallback(toggle, "ACTION", (Icallback)inactivetoggle_cb);
  IupSetAttribute(toggle, "TABS", (char*)tabs);
  IupSetAttribute(label, "EXPAND", "HORIZONTAL");

  dialog = IupDialog(box);
  IupSetAttribute(dialog, "TITLE", "All Images");
  IupSetCallback(dialog, "CLOSE_CB", (Icallback)close_cb);
  IupSetAttribute(dialog, "_INFO_LABEL", (char*)label);

  IupPopup(dialog, IUP_CENTER, IUP_CENTER);

  IupDestroy(dialog);
  IupDestroy(files);

  return IUP_DEFAULT;
}

static char* getfileformat(int all)
{
#define NUM_FORMATS 8
  int ret, count = NUM_FORMATS; 	
  static char *options[NUM_FORMATS] = {
    "led",
    "lua",
    "c",
    "h",
    "ico",
    "bmp",
    "gif",
    "png"
  };

  if (!all)
    count = 4;

  ret = IupListDialog(1,"File Format",count,options,1,10,count+1,NULL);
  if (ret == -1)
    return NULL;
  else
    return options[ret];
}

static char* StrUpper(const char* sstr)
{
  static char buf[10];
  char* dstr = &buf[0];
  for (; *sstr; sstr++, dstr++)
    *dstr = (char)toupper(*sstr);
  *dstr = 0;
  return buf;
}

static char* getfolder(void)
{
  static char folder[1024];
  Ihandle *filedlg = IupFileDlg(); 
 
  IupSetAttribute(filedlg, "DIALOGTYPE", "DIR");
  IupSetAttribute(filedlg, "TITLE", "Select Folder for Images");

  IupPopup(filedlg, IUP_CENTER, IUP_CENTER); 

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    strcpy(folder, IupGetAttribute(filedlg, "VALUE"));
    IupDestroy(filedlg);
    return folder;
  }

  IupDestroy(filedlg);
  return NULL;
}

static int saveallimages_cb(void)
{
  char *names[MAX_NAMES];
  char* folder;
  int i, n = 0, num_names = IupGetAllNames(names, MAX_NAMES); 

  char* imgtype = getfileformat(1);
  if (!imgtype)
    return IUP_DEFAULT;

  folder = getfolder();
  if (!folder)
    return IUP_DEFAULT;

  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
#if (IUP_VERSION_NUMBER < 300000)
    char* type = IupGetClassName(elem);
#else
    char* type = IupGetClassType(elem);
#endif

    if (strcmp(type, "image") == 0)
    {
      char file_name[1024] = "";

      /* save only loaded images */
      char* file_title = IupGetAttribute(elem, "_FILE_TITLE");
      if (!file_title)
        continue;

      strcpy(file_name, folder);
      strcat(file_name, "/");
      strcat(file_name, file_title);
      strcat(file_name, "_");
      strcat(file_name, names[i]);
      strcat(file_name, ".");
      strcat(file_name, imgtype);

      if (strcmp(imgtype, "led") == 0)
        SaveImageLED(file_name, elem, names[i], NULL);
      else if (strcmp(imgtype, "lua") == 0)
        SaveImageLua(file_name, elem, names[i], NULL);
      else if ((strcmp(imgtype, "c") == 0) || (strcmp(imgtype, "h") == 0))
        SaveImageC(file_name, elem, names[i], NULL);
#ifdef USE_IM
      else 
        IupSaveImage(elem, file_name, StrUpper(imgtype));
#endif
      n++;
    }
  }

  if (n == 0)
  {
    IupMessage("Error", "No images.");
    return IUP_DEFAULT;
  }

  return IUP_DEFAULT;
}

static int GetSaveAsFile(char* file, const char* imgtype)
{
  Ihandle *gf;
  int ret;
  char *value;

  gf = IupFileDlg();
  IupSetAttribute(gf, "DIALOGTYPE", "SAVE");
  IupSetfAttribute(gf, "TITLE", "Save %s File", imgtype);
  IupSetfAttribute(gf, "FILTER", "*.%s", imgtype);
  IupSetAttribute(gf, "FILE", file);
  IupPopup(gf, IUP_CENTER, IUP_CENTER);

  value = IupGetAttribute( gf, "VALUE" );
  if (value) strcpy( file, value );
  ret = IupGetInt( gf, "STATUS" );

  IupDestroy(gf);

  return ret;
}

static void replaceDot(char* file_name)
{
  /* replace all "." by "_" */
  /* replace all "-" by "_" */
  while(*file_name)
  {
    if (*file_name == '.') 
      *file_name = '_';
    if (*file_name == '-') 
      *file_name = '_';

    file_name++;
  }
}

static char* strdup_free(const char* str, char* str_ptr)
{
  int len = strlen(str);
  char* tmp = malloc(len+1);
  memcpy(tmp, str, len+1);
  free(str_ptr);
  return tmp;
}

static char* mainGetFileTitle(const char* file_name)
{
  int i, last = 1, len = strlen(file_name);
  char* file_title = malloc(len+1);
  char* dot, *ft_str = file_title;

  memcpy(file_title, file_name, len+1);
  
  dot = strchr(file_title, '.');
  if (dot) *dot = 0;

  for(i = len-1; i >= 0; i--)
  {
    if (last && file_title[i] == '.') 
    {
      /* cut last "." forward */
      file_title[i] = 0;
      last = 0;
    }

    if (file_title[i] == '\\' || file_title[i] == '/') 
    {
      replaceDot(file_title+i+1);
      return strdup_free(file_title+i+1, ft_str);
    }
  }

  replaceDot(file_title);
  return strdup_free(file_title, ft_str);
}

static int saveallimagesone_cb(void)
{
  char file_name[1024] = "*.";
  char *names[MAX_NAMES];
  int i, n = 0, num_names = IupGetAllNames(names, MAX_NAMES); 
  FILE* packfile = NULL;

  char* imgtype = getfileformat(0);
  if (!imgtype)
    return IUP_DEFAULT;

  strcat(file_name, imgtype);
  if (GetSaveAsFile(file_name, imgtype) == -1)
    return IUP_DEFAULT;

  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
#if (IUP_VERSION_NUMBER < 300000)
    char* type = IupGetClassName(elem);
#else
    char* type = IupGetClassType(elem);
#endif

    if (strcmp(type, "image") == 0)
    {
      /* save only loaded images */
      char* file_title = IupGetAttribute(elem, "_FILE_TITLE");
      if (!file_title)
      {
        names[i] = NULL;
        continue;
      }

      if (!packfile)
        packfile = fopen(file_name, "wb");

      if (strcmp(imgtype, "led") == 0)
        SaveImageLED(NULL, elem, names[i], packfile);
      else if (strcmp(imgtype, "lua") == 0)
        SaveImageLua(NULL, elem, names[i], packfile);
      else if ((strcmp(imgtype, "c") == 0) || (strcmp(imgtype, "h") == 0))
        SaveImageC(NULL, elem, names[i], packfile);

      n++;
    }
    else
      names[i] = NULL;
  }

  if (packfile)
  {
    if (strcmp(imgtype, "c") == 0)
    {
      char* title = mainGetFileTitle(file_name);
      fprintf(packfile, "void load_all_images_%s(void)\n{\n", title);
      free(title);

      for (i = 0; i < num_names; i++)
      {
        if (names[i])
          fprintf(packfile, "  IupSetHandle(\"IUP_%s\", load_image_%s());\n", names[i], names[i]);
      }
      fprintf(packfile, "}\n\n");
    }

    fclose(packfile);
  }

  if (n == 0)
  {
    IupMessage("Error", "No images.");
    return IUP_DEFAULT;
  }

  return IUP_DEFAULT;
}

static int saveimage_cb(Ihandle* self)
{
  Ihandle* list = (Ihandle*)IupGetAttribute(self, "mainList");
  char* name = IupGetAttribute(list, IupGetAttribute(list, "VALUE"));

  if (name) /* the list may be empty */
  {
    Ihandle* elem = IupGetHandle(name);
#if (IUP_VERSION_NUMBER < 300000)
    char* type = IupGetClassName(elem);
#else
    char* type = IupGetClassType(elem);
#endif

    if (strcmp(type, "image") == 0)
    {
      char file_name[1024];

      char* imgtype = getfileformat(1);
      if (!imgtype)
        return IUP_DEFAULT;

      sprintf(file_name, "%s.%s", name, imgtype);
      if (GetSaveAsFile(file_name, imgtype) != -1)
      {
        if (strcmp(imgtype, "led") == 0)
          SaveImageLED(file_name, elem, name, NULL);
        else if (strcmp(imgtype, "lua") == 0)
          SaveImageLua(file_name, elem, name, NULL);
        else if ((strcmp(imgtype, "c") == 0) || (strcmp(imgtype, "h") == 0))
          SaveImageC(file_name, elem, name, NULL);
#ifdef USE_IM
        else 
          IupSaveImage(elem, file_name, StrUpper(imgtype));
#endif
      }
    }
    else
      IupMessage("Error", "Not an image.");
  }
  else
    IupMessage("Error", "No elements.");

  return IUP_DEFAULT;
}

static int hideelem_cb(Ihandle* self)
{
  Ihandle* list = (Ihandle*)IupGetAttribute(self, "mainList");
  char* name = IupGetAttribute(list, IupGetAttribute(list, "VALUE"));

  if (name) /* the list may be empty */
  {
    Ihandle* elem = IupGetHandle(name);
    char* type = IupGetClassName(elem);

    if (strcmp(type, "dialog") == 0)
      IupHide(elem);
    else
    {
      Ihandle* dialog = IupGetDialog(elem);
      if (dialog)
        IupHide(dialog);
      else
        IupMessage("Error", "Will only hide dialogs.");
    }
  }
  else
    IupMessage("Error", "No elements.");

  return IUP_DEFAULT;
}

static int showelem_cb(Ihandle* self)
{
  Ihandle* list = (Ihandle*)IupGetAttribute(self, "mainList");
  char* name = IupGetAttribute(list, IupGetAttribute(list, "VALUE"));

  if (name) /* the list may be empty */
  {
    Ihandle* elem = IupGetHandle(name);
    char* type = IupGetClassName(elem);

    if (strcmp(type, "dialog") == 0)
      IupShow(elem);
    else
    {
      Ihandle* dialog = IupGetDialog(elem);
      if (dialog)
        IupShow(dialog);
      else
      {
        if (strcmp(type, "menu") == 0)
          IupPopup(elem, IUP_MOUSEPOS, IUP_MOUSEPOS);
        else
          IupMessage("Error", "Will only show dialogs and independent menus.");
      }
    }
  }
  else
    IupMessage("Error", "No elements.");

  return IUP_DEFAULT;
}

static int list_cb(Ihandle* self, char *t, int i, int v)
{
  if (v == 1)
  {                                                               
    char str_elem[100];
    Ihandle* elem = IupGetHandle(t);
    Ihandle* label = (Ihandle*)IupGetAttribute(self, "mainLabel");
    sprintf(str_elem, "FileTitle: %s - Type: %s", IupGetAttribute(elem, "_FILE_TITLE"), IupGetClassName(elem));
    IupStoreAttribute(label, "TITLE", str_elem);

    /* double click simulation */
    if (i != -1)
    {
      static clock_t start_time = 0;
      static int list_item = 0;
      clock_t cur_time;
                           
      cur_time = clock();
      if ((((int)cur_time-(int)start_time) < 600) && (list_item == i))
        showelem_cb(self);

      start_time = cur_time;
      list_item = i;
    }
  }
  return IUP_DEFAULT;
}

static void mainUpdateInternals(void)
{
  char *names[MAX_NAMES];
  int i, num_names = IupGetAllNames(names, MAX_NAMES); 
  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);

    if (!IupGetAttribute(elem, "_FILE_TITLE"))
      IupSetAttribute(elem, "_INTERNAL", "YES");
  }
}

static int destroyall_cb(Ihandle* self)
{
  char *names[MAX_NAMES];
  Ihandle* list = (Ihandle*)IupGetAttribute(self, "mainList");
  int i, num_names = IupGetAllNames(names, MAX_NAMES); 
  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
    
    if (elem && IupGetInt(elem, "_INTERNAL") == 0)
    {
      char* type = IupGetClassName(elem);

      if (strcmp(type, "dialog") == 0)
        IupDestroy(elem);
      else
      {
        Ihandle* dialog = IupGetDialog(elem);
        if (!dialog)
          IupDestroy(elem);
      }
    }
  }
  IupSetAttribute(list, "1", NULL);
  IupSetAttribute(list, "VALUE", "1");
  return IUP_DEFAULT;
}

static void mainUpdateList(Ihandle* self, const char* file_name)
{
  char *names[MAX_NAMES];
  char str_item[20];
  Ihandle* list = (Ihandle*)IupGetAttribute(self, "mainList");
  int i, n = 0, num_names = IupGetAllNames(names, MAX_NAMES); 
  char* file_title = mainGetFileTitle(file_name);
  for (i = 0; i < num_names; i++)
  {
    Ihandle* elem = IupGetHandle(names[i]);
    
    if (IupGetInt(elem, "_INTERNAL") == 0)
    {
      sprintf(str_item, "%d", n+1);
      IupSetAttribute(list, str_item, names[i]);

      if (!IupGetAttribute(elem, "_FILE_TITLE"))
        IupStoreAttribute(elem, "_FILE_TITLE", file_title);
      n++;
    }
  }
  sprintf(str_item, "%d", n+1);
  IupSetAttribute(list, str_item, NULL);

  if (n != 0)
  {
    IupSetAttribute(list, "VALUE", "1");
    list_cb(list, IupGetAttribute(list, "1"), -1, 1);
  }

  free(file_title);
}

static int loadimagelib_cb(Ihandle* self)
{
  mainUpdateInternals();

#if (IUP_VERSION_NUMBER < 300000)
  IupImageLibOpen();
#else
  iupImageLibLoadAll();
#endif  

  mainUpdateList(self, "ImageLib");

  return IUP_DEFAULT;
}

static int loadled_cb(Ihandle* self)
{
  char file_name[1024] = "./*.led";
  int ret = IupGetFile(file_name);
  if (ret == 0)
  {
    char* error;

    mainUpdateInternals();

    error = IupLoad(file_name);
    if (error)
      IupMessage("Error", error);
    else
      mainUpdateList(self, file_name);
  }
  return IUP_DEFAULT;
}

#ifdef USE_IM
#ifdef WIN32
static char* ParseFile(const char* dir, const char* FileName, int *offset)
{
  const char* file = FileName;
  while (*file != 0 && *file != '|')
    file++;

  if (file == FileName)
    return NULL;

  {
    int size = file - FileName + 1;
    int dir_size = strlen(dir);
    char* file_name = malloc(size+dir_size+1);
    memcpy(file_name, dir, dir_size);
    file_name[dir_size] = '\\';
    memcpy(file_name+dir_size+1, FileName, size-1);
    file_name[size+dir_size] = 0;
    *offset += size;
    return file_name;
  }
}

static char* ParseDir(const char* FileName, int *offset)
{
  const char* file = FileName;
  while (*file != 0 && *file != '|')
    file++;

  if (*file == 0)
    return NULL;

  {
    int size = file - FileName + 1;
    char* dir = malloc(size);
    memcpy(dir, FileName, size-1);
    dir[size-1] = 0;
    *offset = size;
    return dir;
  }
}
#endif

static void LoadImageFile(Ihandle* self, const char* file_name)
{
  Ihandle* new_image = IupLoadImage(file_name);
  if (new_image)
  {
    char* file_title = mainGetFileTitle(file_name);
    IupSetHandle(file_title, new_image);
    free(file_title);
    mainUpdateList(self, file_name);
  }
}

static int GetOpenFileName(char* file)
{
  Ihandle *gf;
  int ret;
  char *value;

  gf = IupFileDlg();
  IupSetAttribute(gf, "DIALOGTYPE", "OPEN");
  IupSetAttribute(gf, "TITLE", "Load Image File(s)");
  IupSetAttribute(gf, "MULTIPLEFILES", "YES");
  IupPopup(gf, IUP_CENTER, IUP_CENTER);

  value = IupGetAttribute(gf, "VALUE");
  if (value) strcpy(file, value);
  ret = IupGetInt(gf, "STATUS");

  IupDestroy(gf);

  return ret;
}

static int loadimage_cb(Ihandle* self)
{
  char FileName[2000] = "*.*";

  /* Retrieve a file name */
  if (GetOpenFileName(FileName) == -1)
   return IUP_DEFAULT;

  mainUpdateInternals();

#ifdef WIN32
  /* parse multiple files */
  {
    int offset;
    char* file_name;
    char* dir = ParseDir(FileName, &offset);
    if (dir)
    {
      while ((file_name = ParseFile(dir, FileName + offset, &offset)) != NULL)
      {
        LoadImageFile(self, file_name);
        free(file_name);
      }
      free(dir);
    }
    else
      LoadImageFile(self, FileName);
  }
#else
  LoadImageFile(self, FileName);
#endif

  return IUP_DEFAULT;
}
#endif

static int dropfile_cb(Ihandle *self, char* file_name)
{
  char* error;

  mainUpdateInternals();

  error = IupLoad(file_name);
  if (error)
    IupMessage("Error", error);
  else
    mainUpdateList(self, file_name);

  return IUP_DEFAULT;
}

static Ihandle* mainDialog(void)
{
  Ihandle *main_dialog, *box, *menu, *list, *label;

  menu = IupMenu(
    IupSubmenu("File", IupMenu(
      IupSetCallbacks(IupItem("Load Led...", NULL), "ACTION", (Icallback)loadled_cb, NULL),
      IupSetCallbacks(IupItem("Load Image Lib", NULL), "ACTION", (Icallback)loadimagelib_cb, NULL),
      IupSeparator(),
#ifdef USE_IM
      IupSetCallbacks(IupItem("Import Image(s)...", NULL), "ACTION", (Icallback)loadimage_cb, NULL),
#endif
      IupSeparator(),
      IupSetCallbacks(IupItem("Exit", NULL), "ACTION", (Icallback)close_cb, NULL),
      NULL)),
    IupSubmenu("Element", IupMenu(
      IupSetCallbacks(IupItem("Show...", NULL), "ACTION", (Icallback)showelem_cb, NULL),
      IupSetCallbacks(IupItem("Hide...", NULL), "ACTION", (Icallback)hideelem_cb, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("Destroy All...", NULL), "ACTION", (Icallback)destroyall_cb, NULL),
      IupSetCallbacks(IupItem("Show All Images...", NULL), "ACTION", (Icallback)showallimages_cb, NULL),
      IupSeparator(),
      IupSetCallbacks(IupItem("Save Image...", NULL), "ACTION", (Icallback)saveimage_cb, NULL),
      IupSetCallbacks(IupItem("Save All Images...", NULL), "ACTION", (Icallback)saveallimages_cb, NULL),
      IupSetCallbacks(IupItem("Save All Images (One File)...", NULL), "ACTION", (Icallback)saveallimagesone_cb, NULL),
      NULL)),
    IupSubmenu("Help", IupMenu(
      IupSetCallbacks(IupItem("About...", NULL), "ACTION", (Icallback)about_cb, NULL),
      NULL)),
    NULL
    );
  IupSetHandle("mainMenu", menu);

  box = IupVbox(
    IupLabel("Elements:"),
    list = IupList(NULL),
    IupSetAttributes(IupFill(), "SIZE=2"),
    IupFrame(label = IupLabel("")),
    NULL);
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetCallback(list, "ACTION", (Icallback)list_cb);

  IupSetAttribute(list, "SIZE", "150x80");
  IupSetAttribute(list, "EXPAND", "YES");
  IupSetAttribute(label, "EXPAND", "HORIZONTAL");
 
  main_dialog = IupDialog(box);
  IupSetAttribute(main_dialog, "TITLE", "IupView");
  IupSetAttribute(main_dialog, "MENU", "mainMenu");
  IupSetAttribute(main_dialog, "mainList", (char*)list);
  IupSetAttribute(main_dialog, "mainLabel", (char*)label);
  IupSetCallback(main_dialog, "CLOSE_CB", (Icallback)close_cb);
  IupSetCallback(main_dialog, "DROPFILES_CB", (Icallback)dropfile_cb);   
  
  IupSetAttribute(menu, "mainList", (char*)list);
  IupSetAttribute(menu, "mainLabel", (char*)label);

  return main_dialog;
}

int main (int argc, char **argv)
{
  Ihandle* main_dialog;

  IupOpen(&argc, &argv);
#ifndef USE_NO_OPENGL  
  IupGLCanvasOpen();
#endif  
  IupControlsOpen();
#if (IUP_VERSION_NUMBER >= 300000)
  IupImageLibOpen();
#endif  

  mainUpdateInternals();

  main_dialog = mainDialog();
  IupShow(main_dialog);

  IupMainLoop();
  destroyall_cb(main_dialog);
  IupDestroy(main_dialog);

  IupControlsClose();
  IupClose();
  return 0;
}

