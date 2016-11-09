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
 *@	File:	spacmd.h
 *@	Datum:	28.10.1997
 *@	Author:	P.Zahl
 *@	Zweck:	SPA DSP Komando ID's
 *@             DSP Datenuebertragungsmodi: PORT_IO/DPRAM16/DPRAM32
 *@
 *@	History:
 *@	=====================================================================
 *@	V1.00 Basisversion
 *@@    28.10.97 PZ: Neu
 *@@    12.11.99 PZ: Splitting: DPRAM Zuweisungen / CmdIds
 */

#include "dpramdef.h" /* DPRAM Aufteilung */


/*
 * DSP Komandos zur Komunikation
 */


#define DSP_CMD_INIT      		0x01	/* DSP Init (Dummy) */
#define DSP_CMD_OKTODATA		0x10	/* Oktopolparameter */
#define DSP_CMD_OKTO_SET		0x11	/* Setze Ux Uy auf X0, Y0 Werte*/
#define DSP_CMD_SPACTRL_SET		0x12	/* SPA-LEED Control Unit Values */
#define DSP_CMD_SCAN_PARAM		0x20	/* Scanparameter */
#define DSP_CMD_SCAN_START		0x21	/* Start Scan in Mem A */
#define DSP_CMD_SWAPDPRAM	        0x22	/* Swap Memory1,2,.. from buffer to dpram */
#define DSP_CMD_GETCNT   		0x23	/* Get Count at X,Y */
#define DSP_CMD_SCAN2D   		0x24	/* small 2D Scan (Focus) */

#define DSP_CMD_OSZI_RUN 		0x40	/* Oszi Start */
#define DSP_CMD_OSZI_STOP 		0x41	/* Oszi Stop  */
#define DSP_CMD_OSZI_PARAM 		0x42	/* Oszi Parameter  */
#define DSP_CMD_OSZI_RESET 		0x43	/* Oszi Time Reset  */

#define DSP_CMD_SSIODCAL                0x50    /* SSIOD Calib. fahren */
#define DSP_CMD_SSIODGAIN               0x51    /* SSIOD Gain Set */

#define DSP_CMD_PH_PARAM                0x60    /* Puls Heating control */

#define DSP_CMD_GETINFO			0xfe	/* Reports info now */
#define DSP_CMD_READY			0xfe	/* Dummy: nur ACK Antwort -- not used any longer */


/* SPA DnD */
#define DSP_X0     0
#define DSP_Y0     1
#define DSP_len    2
#define DSP_N      3
#define DSP_alpha  4
#define DSP_ms     5
#define DSP_E      6
#define DSP_NX     7
#define DSP_NY     8
#define DSP_LXY    9
#define DSP_MXX   10
#define DSP_MXY   11
#define DSP_MYX   12
#define DSP_MYY   13


#define DSP_SPACTRL_EXTRACTOR    0
#define DSP_SPACTRL_CHANHV       1
#define DSP_SPACTRL_CHANREPELLER 2
#define DSP_SPACTRL_CRYFOCUS     3
#define DSP_SPACTRL_FILAMENT     4
#define DSP_SPACTRL_GUNFOCUS     5
#define DSP_SPACTRL_GUNANODE     6
#define DSP_SPACTRL_SMPDIST      7
#define DSP_SPACTRL_SMPTEMP      8
#define DSP_SPACTRL_GROWING      9

/* BF */
#define DSP_BF_LEN 0
#define DSP_BF_N   1
#define DSP_BF_MS  2


/* Oszi */
#define DSP_OSZI_TMGRENZ  500
/* ab timebase >= 500ms kein DPRAM Array mehr, sondern Zeit und Chaneldata
                  mirror
 */

#define DSP_OSZI_TIMEBASE  0
#define DSP_OSZI_CHANNELS  1
#define DSP_OSZI_NAVG      2
#define DSP_OSZI_NSAMPLES  3
#define DSP_OSZI_MODE      4

/* Spezialpositionen für Oszi */
#define DSP_OSZI_CH12      15
#define DSP_OSZI_CH34      16
#define DSP_OSZI_CH56      17
#define DSP_OSZI_CH78      18
#define DSP_OSZI_TM        19
#define DSP_OSZI_DATAREADY 20
#define DSP_OSZI_CNT       21

/* SSIOD */
#define DSP_SSIOD_N        0
#define DSP_SSIOD_DN       1
#define DSP_SSIOD_X1       2

/* PH Param */
#define DSP_PH_STATE       0
#define DSP_PH_TDELTA      1
#define DSP_PH_TON         2
#define DSP_PH_TOFF        3

/* Höchste Nummer für Parameterübergabe */
#define DSP_LASTCMD        12

