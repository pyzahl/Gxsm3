/* Gxsm - Gnome X Scanning Microscopy Project
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * DSP tools for Linux
 *
 * Copyright (C) 1999,2000,2001 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * WWW Home: http://gxsm.sf.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/*@
 *@	File:	dpramdef.h
 *@	Datum:	25.10.1995
 *@	Author:	P.Zahl
 *@	Zweck:	definition der Aufteilung des DPRAMs
 *@             
 *@
 *@	History:
 *@	=====================================================================
 *@	V1.00 Basisversion
 *@@    24.4.96 PZ: DPRAM16/32 Implementation
 *@@    24.4.96 PZ: Def. von Datenübertragungsmodi
 *@@    14.12.97 PZ: Namensaenderung in xsmcmd.h
 *@@    12.11.99 PZ: Splitting: DPRAM Zuweisungen / CmdIds
 */

/*
 *      DSP Datenuebertragungsmodi: nur noch DPRAM32 Mode !
 */

#define DSP_DPRAMLEN      0x0800
#define DSP_DPRAMFREE     0x07f0 /* abzüglich Mailbox */

/* OSZI Size */
#define DSP_OSZI_CTRL     0x0002
#define DSP_OSZI_LEN      0x0100
#define DSP_OSZI_MSK      (DSP_OSZI_LEN-1)

#define DSP_LCD_LEN       0x10
#define DSP_DIO_LEN       0x01

/* ================================================= */
/* DPRAM BEREICHE */
/* SEM0 */
/* Mailbox at 0x7f0 (default) */
/* SEM1 */
#define DSP_CTRL_REGION   0x0000
#define DSP_CTRL_CMD      DSP_CTRL_REGION
#define DSP_CTRL_PARAM    (DSP_CTRL_CMD  +0x0001)
#define DSP_CTRL_REG_END  (DSP_CTRL_PARAM+0x0010)

#define DSP_CTRL_REG_LEN  (DSP_CTRL_REG_END-DSP_CTRL_REGION)

/* SEM2 */
#define DSP_USR_REGION    DSP_CTRL_REG_END
#define DSP_USR_OSZI_CTRL DSP_USR_REGION
#define DSP_USR_OSZI_DATA (DSP_USR_OSZI_CTRL + DSP_OSZI_CTRL)
#define DSP_USR_LCD       (DSP_USR_OSZI_DATA + DSP_OSZI_LEN)
#define DSP_USR_DIO       (DSP_USR_LCD + DSP_LCD_LEN)
#define DSP_USR_REG_END   (DSP_USR_DIO + DSP_DIO_LEN)

#define DSP_USR_REG_LEN   (DSP_USR_REG_END-DSP_USR_REGION)
#define MK_USR_OFFSET(X)  (X-DSP_USR_REGION)

/* SEM3 */
#define DSP_DATA_REGION   DSP_USR_REG_END
#define DSP_DATA_REG_END  DSP_DPRAMFREE

#define DSP_DATA_REG_LEN  (DSP_DATA_REG_END-DSP_DATA_REGION)

/* ================================================= */
/* Datenbuffer Start */
#define DSP_BUFFER_START  DSP_DATA_REGION
#define DSP_OSZIBUF_START (DSP_USR_OSZI_DATA)
#define DSP_LCDBUFFER     (DSP_USR_LCD)
#define DSP_LCDBUFFERLEN  (DSP_LCD_LEN)
