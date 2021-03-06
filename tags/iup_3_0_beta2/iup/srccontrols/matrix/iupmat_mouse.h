/** \file
 * \brief iMatrixrix control
 * mouse events.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_MOUSE_H 
#define __IUPMAT_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int iupMatrixMouseButtonCB (Ihandle* ih, int b, int press, int x, int y, char* r);
int iupMatrixMouseMoveCB   (Ihandle* ih, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
