/** \file
 * \brief Frame Control Private Declarations
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_FRAME_H 
#define __IUP_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

void iupdrvFrameInitClass(Iclass* ic);
void iupdrvFrameGetDecorOffset(Ihandle* ih, int *x, int *y);
int iupFrameGetTitleHeight(Ihandle* ih);

#ifdef __cplusplus
}
#endif

#endif
