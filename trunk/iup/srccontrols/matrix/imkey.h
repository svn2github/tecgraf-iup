/** \file
 * \brief iupmatrix. keyboard control.
 *
 * See Copyright Notice in iup.h
 * $Id: imkey.h,v 1.1 2008-10-17 06:05:36 scuri Exp $
 */
 
#ifndef __IMKEY_H 
#define __IMKEY_H

#ifdef __cplusplus
extern "C" {
#endif

int  iMatrixKeyPressCB(Ihandle* ih, int c, int press);
int  iMatrixKey(Ihandle* ih, int c);

void iMatrixResetKeyCount  (void);
int  iMatrixGetHomeKeyCount(void);
int  iMatrixGetEndKeyCount (void);

#ifdef __cplusplus
}
#endif

#endif
