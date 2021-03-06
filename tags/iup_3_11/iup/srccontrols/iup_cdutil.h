/** \file
 * \brief cdiuputil. CD and IUP utilities for the IupControls
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_CDUTIL_H 
#define __IUP_CDUTIL_H 

#ifdef __cplusplus
extern "C" {
#endif

void cdIupCalcShadows(long bgcolor, long *light_shadow, long *mid_shadow, long *dark_shadow);
long cdIupConvertColor(const char *color);
void cdIupDrawSunkenRect(cdCanvas *canvas, int x1, int y1, int x2, int y2,  
                         long light_shadow, long mid_shadow, long dark_shadow);
void cdIupDrawRaiseRect(cdCanvas *canvas, int x1, int y1, int x2, int y2,  
                         long light_shadow, long mid_shadow, long dark_shadow);
void cdIupDrawVertSunkenMark(cdCanvas *canvas, int x, int y1, int y2, long light_shadow, long dark_shadow);
void cdIupDrawHorizSunkenMark(cdCanvas *canvas, int x1, int x2, int y, long light_shadow, long dark_shadow);

void IupCdDrawFocusRect(Ihandle* ih, cdCanvas *canvas, int x1, int y1, int x2, int y2);
void IupCdSetFont(Ihandle* ih, cdCanvas *canvas, const char* font);

void cdIupDrawFocusRect(cdCanvas *canvas, int x1, int y1, int x2, int y2);

#define cdIupInvertYAxis(_y, _h) ((_h) - (_y) - 1);

void cdIupDrawImage(cdCanvas *canvas, Ihandle *image, int x, int y, int make_inactive, long bgcolor);


#ifdef __cplusplus
}
#endif

#endif
