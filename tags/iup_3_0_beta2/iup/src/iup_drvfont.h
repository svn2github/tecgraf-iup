/** \file
 * \brief Driver Font Management
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_DRVFONT_H 
#define __IUP_DRVFONT_H

#ifdef __cplusplus
extern "C" {
#endif


/** \defgroup drvfont Driver Font Interface 
 * \par
 * Each driver must export the symbols defined here.
 * \par
 * See \ref iup_drvfont.h 
 * \ingroup drv */

/* Called only from IupOpen/IupClose. */
void iupdrvFontInit(void);
void iupdrvFontFinish(void);

/** Retrieve the character size for the selected font.
 * Should be used only to calculate the SIZE attribute.
 * \ingroup drvfont */
void iupdrvFontGetCharSize(Ihandle* ih, int *charwidth, int *charheight);

/** Retrieve the string width for the selected font.
 * \ingroup drvfont */
int iupdrvFontGetStringWidth(Ihandle* ih, const char* str);

/** Retrieve the multi-lined string size for the selected font. \n
 * Width is the maximum line width. \n
 * Height is charheight*number_of_lines (this will avoid line size variations).
 * \ingroup drvfont */
void iupdrvFontGetMultiLineStringSize(Ihandle* ih, const char* str, int *w, int *h);

/** Returns the System default font.
 * \ingroup drvfont */
char* iupdrvGetSystemFont(void);

/** STANDARDFONT attribute set function.
 * \ingroup drvfont */
int iupdrvSetStandardFontAttrib(Ihandle* ih, const char* value);



/** FONT attribute get function.
 * \ingroup drvfont */
char* iupGetFontAttrib(Ihandle* ih);

/** FONT attribute set function.
 * \ingroup drvfont */
int iupSetFontAttrib(Ihandle* ih, const char* value);

/** Parse the common font format description.
 * Returns a non zero value if successful.
 * \ingroup drvfont */
int iupFontParsePango(const char *value, char *typeface, int *size, int *bold, int *italic, int *underline, int *strikeout);

/** Parse the old IUP Windows font format description.
 * Returns a non zero value if successful.
 * \ingroup drvfont */
int iupFontParseWin(const char *value, char *fontname, int *height, int *bold, int *italic, int *underline, int *strikeout);

/** Parse the X-Windows font format description.
 * Returns a non zero value if successful.
 * \ingroup drvfont */
int iupFontParseX(const char *value, char *fontname, int *height, int *bold, int *italic, int *underline, int *strikeout);


/** Changes the FONT style only.
 * \ingroup attribfunc */
int iupSetFontStyleAttrib(Ihandle* ih, const char* value);

/** Changes the FONT size only.
 * \ingroup attribfunc */
int iupSetFontSizeAttrib(Ihandle* ih, const char* value);

/** Returns the FONT style.
 * \ingroup attribfunc */
char* iupGetFontStyleAttrib(Ihandle* ih);

/** Returns the FONT size.
 * \ingroup attribfunc */
char* iupGetFontSizeAttrib(Ihandle* ih);

/** Returns the FONT face.
 * \ingroup attribfunc */
char* iupGetFontFaceAttrib(Ihandle* ih);

/* Updates the STANDARDFONT attrib.
 * Called only from IupMap.
 */
void iupUpdateStandardFontAttrib(Ihandle* ih);

/* Used to map foreign names into native names */
const char* iupFontGetWinName(const char* name);
const char* iupFontGetXName(const char* name);
const char* iupFontGetPangoName(const char* name);



#ifdef __cplusplus
}
#endif

#endif
