/** \file
 * \brief attributes enviroment management
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <limits.h>
#include <stdarg.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_str.h"
#include "iup_ledlex.h"
#include "iup_attrib.h"
#include "iup_assert.h"


int IupGetAllAttributes(Ihandle* ih, char** names, int n)
{
  char *name;
  int i = 0;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return 0;

  if (!names || !n)
    return iupTableCount(ih->attrib);

  name = iupTableFirst(ih->attrib);
  while (name)
  {
    names[i] = name;
    i++;
    if (i == n)
      break;

    name = iupTableNext(ih->attrib);
  }

  return i;
}

char* IupGetAttributes(Ihandle *ih)
{
  char *buffer;
  char *name, *value;
  char sb[128];

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  buffer = iupStrGetMemory(10240);
  buffer[0] = 0;

  name = iupTableFirst(ih->attrib);
  while (name)
  {
    if (!iupAttribIsInternal(name))
    {
      if (buffer[0] != 0)
        strcat(buffer,",");

      value = iupTableGetCurr(ih->attrib);
      if (iupAttribIsPointer(ih, name))
      {
        sprintf(sb, "%p", (void*) value);
        value = sb;
      }
      strcat(buffer, name);
      strcat(buffer,"=\"");
      strcat(buffer, value);
      strcat(buffer,"\"");
    }

    name = iupTableNext(ih->attrib);
  }

  return buffer;
}

void iupAttribUpdateFromParent(Ihandle* ih)
{
  Iclass* ic = ih->iclass;
  char *name = iupTableFirst(ic->attrib_func);
  while (name)
  {
    /* if inheritable and NOT defined at the element */
    if (iupClassObjectCurAttribIsInherit(ic) && !iupAttribGet(ih, name))
    {
      /* check in the parent tree if the attribute is defined */
      Ihandle* parent = ih->parent;
      while (parent)
      {
        char* value = iupTableGet(parent->attrib, name);
        if (value)
        {
          int inherit;
          /* set on the class */
          iupClassObjectSetAttribute(ih, name, value, &inherit);
          break;
        }
        parent = parent->parent;
      }
    }

    name = iupTableNext(ic->attrib_func);
  }
}

static void iAttribNotifyChildren(Ihandle *ih, const char* name, const char *value)
{
  int inherit;
  Ihandle* child = ih->firstchild;
  while (child)
  {
    if (!iupTableGet(child->attrib, name))
    {
      /* set on the class */
      iupClassObjectSetAttribute(child, name, value, &inherit);

      if (inherit)  /* inherit can be different for the child */
        iAttribNotifyChildren(child, name, value);
    }

    child = child->brother;
  }
}

void iupAttribUpdate(Ihandle* ih)
{
  char** name_array;
  char *name, *value;
  int count, i = 0, inherit, store;

  count = iupTableCount(ih->attrib);
  if (!count)
    return;

  name_array = (char**)malloc(count * sizeof(char*));

  /* store the names before updating so we can add or remove attributes during the update */
  name = iupTableFirst(ih->attrib);
  while (name)
  {
    name_array[i] = name;
    name = iupTableNext(ih->attrib);
    i++;
  }

  /* for all defined attributes updates the native system */
  for (i = 0; i < count; i++)
  {
    name = name_array[i];
    if (!iupAttribIsInternal(name))
    {
      /* retrieve from the table */
      value = iupTableGet(ih->attrib, name);

      /* set on the class */
      store = iupClassObjectSetAttribute(ih, name, value, &inherit);

      if (inherit)
        iAttribNotifyChildren(ih, name, value);

      if (store == 0)
        iupTableRemove(ih->attrib, name); /* remove from the table acording to the class SetAttribute */
    }
  }

  free(name_array);
}

void IupSetAttribute(Ihandle *ih, const char* name, const char *value)
{
  int inherit;

  iupASSERT(name!=NULL);
  if (!name)
    return;

  if (!ih)
  {
    IupSetGlobal(name, value);
    return;
  }

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (iupAttribIsInternal(name))
    iupAttribSetStr(ih, name, value);
  else
  {
    if (iupClassObjectSetAttribute(ih, name, value, &inherit)!=0) /* store strings and pointers */
      iupAttribSetStr(ih, name, value);

    if (inherit)
      iAttribNotifyChildren(ih, name, value);
  }
}

void IupStoreAttribute(Ihandle *ih, const char* name, const char *value)
{
  int inherit;

  if (!name)
    return;

  if (!ih)
  {
    IupStoreGlobal(name, value);
    return;
  }

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (iupAttribIsInternal(name))
    iupAttribStoreStr(ih, name, value);
  else
  {
    if (iupClassObjectSetAttribute(ih, name, value, &inherit)==1) /* store only strings */
      iupAttribStoreStr(ih, name, value);

    if (inherit)
      iAttribNotifyChildren(ih, name, value);
  }
}

char* IupGetAttribute(Ihandle *ih, const char* name)
{
  int inherit;
  char *value, *def_value;

  iupASSERT(name!=NULL);
  if (!name)
    return NULL;

  if (!ih)
    return IupGetGlobal(name);

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return NULL;

  value = iupClassObjectGetAttribute(ih, name, &def_value, &inherit);
  if (!value)
    value = iupAttribGet(ih, name);

  if (!value && !iupAttribIsInternal(name))
  {
    if (inherit)
    {
      while (!value)
      {
        ih = ih->parent;
        if (!ih)
          break;

        value = iupAttribGet(ih, name);
      }
    }

    if (!value)
      value = def_value;
  }

  return value;
}

float IupGetFloat(Ihandle *ih, const char* name)
{
  float f = 0;
  char *value = IupGetAttribute(ih, name);
  if (value)
    iupStrToFloat(value, &f);
  return f;
}

int IupGetInt(Ihandle *ih, const char* name)
{
  int i = 0;
  char *value = IupGetAttribute(ih, name);
  if (value)
  {
    if (!iupStrToInt(value, &i))
    {
      if (iupStrBoolean(value))
        i = 1;
    }
  }
  return i;
}

int IupGetInt2(Ihandle *ih, const char* name)
{
  int i1 = 0, i2 = 0;
  char *value = IupGetAttribute(ih, name);
  if (value)
  {
    if (!iupStrToIntInt(value, &i1, &i2, 'x'))
      iupStrToIntInt(value, &i1, &i2, ':');
  }
  return i2;
}

int IupGetIntInt(Ihandle *ih, const char* name, int *i1, int *i2)
{
  int _i1 = 0, _i2 = 0;
  char *value = IupGetAttribute(ih, name);
  if (value)
  {
    int count = iupStrToIntInt(value, &_i1, &_i2, 'x');
    if (!count) count = iupStrToIntInt(value, &_i1, &_i2, ':');
    if (i1) *i1 = _i1;
    if (i2) *i2 = _i2;
    return count;
  }
  return 0;
}

void IupSetfAttribute(Ihandle *ih, const char* name, const char* f, ...)
{
  static char value[SHRT_MAX];
  va_list arglist;
  va_start(arglist, f);
  vsprintf(value, f, arglist);
  va_end(arglist);
  IupStoreAttribute(ih, name, value);
}

void iupAttribSetHandleName(Ihandle *ih)
{
  char str_name[100];
  sprintf(str_name, "_IUP_NAME(%p)", ih);
  IupSetHandle(str_name, ih);
}

void IupSetAttributeHandle(Ihandle *ih, const char* name, Ihandle *ih_named)
{
  int inherit;
  char* handle_name;
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;
  iupASSERT(name!=NULL);
  if (!name)
    return;

  handle_name = IupGetName(ih_named);
  if (!handle_name)
  {
    iupAttribSetHandleName(ih_named);
    handle_name = IupGetName(ih_named);
  }

  iupClassObjectSetAttribute(ih, name, handle_name, &inherit);
  iupAttribStoreStr(ih, name, handle_name);
}

Ihandle* IupGetAttributeHandle(Ihandle *ih, const char* name)
{
  char* handle_name = iupAttribGetInherit(ih, name);
  if (handle_name)
    return IupGetHandle(handle_name);
  return NULL;
}

Ihandle* IupSetAtt(const char* handle_name, Ihandle* ih, const char* name, ...)
{
  const char *attr, *val;
  va_list arg;
  va_start (arg, name);
  attr = name;
  while (attr)
  {
    val = va_arg(arg, const char*);
    IupSetAttribute(ih, attr, val);
    attr = va_arg(arg, const char*);
  }
  va_end(arg);
  if (handle_name) IupSetHandle(handle_name, ih);
  return ih;
}

void iupAttribSetStr(Ihandle* ih, const char* name, const char* value)
{
  if (!value)
    iupTableRemove(ih->attrib, name);
  else
    iupTableSet(ih->attrib, name, (void*)value, IUPTABLE_POINTER);
}

void iupAttribStoreStr(Ihandle* ih, const char* name, const char* value)
{
  if (!value)
    iupTableRemove(ih->attrib, name);
  else
    iupTableSet(ih->attrib, name, (void*)value, IUPTABLE_STRING);
}

void iupAttribSetStrf(Ihandle *ih, const char* name, const char* f, ...)
{
  static char value[SHRT_MAX];
  va_list arglist;
  va_start(arglist, f);
  vsprintf(value, f, arglist);
  va_end(arglist);
  iupAttribStoreStr(ih, name, value);
}

void iupAttribSetInt(Ihandle *ih, const char* name, int num)
{
  iupAttribSetStrf(ih, name, "%d", num);
}

void iupAttribSetFloat(Ihandle *ih, const char* name, float num)
{
  iupAttribSetStrf(ih, name, "%f", (double)num);
}

int iupAttribGetInt(Ihandle* ih, const char* name)
{
  int i = 0;
  char *value = iupAttribGetStr(ih, name);
  if (value)
  {
    if (!iupStrToInt(value, &i))
    {
      if (iupStrBoolean(value))
        i = 1;
    }
  }
  return i;
}

float iupAttribGetFloat(Ihandle* ih, const char* name)
{
  float f = 0;
  char *value = iupAttribGetStr(ih, name);
  if (value)
    iupStrToFloat(value, &f);
  return f;
}

char* iupAttribGet(Ihandle* ih, const char* name)
{
  if (!ih || !name)
    return NULL;
  return iupTableGet(ih->attrib, name);
}

char* iupAttribGetStr(Ihandle* ih, const char* name)
{
  char* value;
  if (!ih || !name)
    return NULL;

  value = iupTableGet(ih->attrib, name);

  if (!value && !iupAttribIsInternal(name))
  {
    int inherit;
    char *def_value;
    iupClassObjectGetAttributeInfo(ih, name, &def_value, &inherit);

    if (inherit)
    {
      while (!value)
      {
        ih = ih->parent;
        if (!ih)
          break;

        value = iupAttribGet(ih, name);
      }
    }

    if (!value)
      value = def_value;
  }

  return value;
}

char* iupAttribGetInherit(Ihandle* ih, const char* name)
{
  char* value;
  if (!ih || !name)
    return NULL;

  value = iupAttribGet(ih, name);   /* Check on the element first */
  while (!value)
  {
    ih = ih->parent;   /* iheritance here independs on the attribute */
    if (!ih)
      return NULL;

    value = iupAttribGet(ih, name);
  }
  return value;
}

char* iupAttribGetInheritNativeParent(Ihandle* ih, const char* name)
{
  char* value;
  if (!ih || !name)
    return NULL;

  value = NULL;    /* Do NOT check on the element first */
  while (!value)
  {
    ih = iupChildTreeGetNativeParent(ih);
    if (!ih)
      return NULL;

    value = iupAttribGet(ih, name);
  }

  return value;
}

static const char* env_str = NULL;
static void iAttribCapture(char* env_buffer, char* dlm)
{
  int i=0;
  int c;
  do
  {
    c = *env_str; ++env_str;
    if (i < 256)
      env_buffer[i++] = (char) c;
  } while (c && !strchr(dlm,c));
  env_buffer[i-1]='\0';                                /* discard delimiter */
}

static int iAttribToken(char* env_buffer)
{
  for (;;)
  {
    int c = *env_str; ++env_str;
    switch (c)
    {
    case 0:
      return IUPLEX_TK_END;

    case ' ':          /* ignore whitespace */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
    case '\v':
      continue;

    case '=':          /* attribuicao */
      return IUPLEX_TK_SET;

    case ',':
      return IUPLEX_TK_COMMA;

    case '\"':          /* string */
      iAttribCapture(env_buffer, "\"");
      return IUPLEX_TK_NAME;

    default:
      if (c > 32)          /* identifier */
      {
        --env_str;                     /* unget first character of env_buffer */
        iAttribCapture(env_buffer, "=, \t\n\r\f\v"); /* get env_buffer until delimiter */
        --env_str;                     /* unget delimiter */
        return IUPLEX_TK_NAME;
      }
    }
  }
}

static void iAttribParse(Ihandle *ih, const char* str)
{
  char env_buffer[256];
  char* name=NULL;
  char* value=NULL;
  char state = 'a';               /* get attribute */
  int end = 0;

  env_str = str;

  for (;;)
  {
    switch (iAttribToken(env_buffer))
    {
    case IUPLEX_TK_END:                 /* procedimento igual ao IUPLEX_TK_COMMA */
      end = 1;
    case IUPLEX_TK_COMMA:
      if (name)
      {
        IupStoreAttribute(ih, name, value);
        free(name);
      }
      if (end)
        return;
      name = value = NULL;
      state = 'a';
      break;

    case IUPLEX_TK_SET:
      state = 'v';                /* get value */
      break;

    case IUPLEX_TK_NAME:
      if (state == 'a')
        name = iupStrDup(env_buffer);
      else
        value = env_buffer;
      break;
    }
  }
}

Ihandle* IupSetAttributes(Ihandle *ih, const char* str)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return ih;
  if (str)
    iAttribParse(ih, str);
  return ih;
}

int iupAttribIsPointer(Ihandle* ih, const char* name)
{
  return iupClassObjectAttribIsNotString(ih, name);
}

typedef int (*Iconvertxytopos)(Ihandle* ih, int x, int y);

int IupConvertXYToPos(Ihandle* ih, int x, int y)
{
  Iconvertxytopos drvConvertXYToPos;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return -1;

  if (!ih->handle)
    return -1;

  drvConvertXYToPos = (Iconvertxytopos)IupGetCallback(ih, "_IUP_XY2POS_CB");
  if (drvConvertXYToPos)
    return drvConvertXYToPos(ih, x, y);

  return -1;
}
