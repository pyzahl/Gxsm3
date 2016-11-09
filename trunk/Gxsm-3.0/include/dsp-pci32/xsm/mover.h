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
 *@	File:	mover.h
 *@	Datum:	23.12.1998
 *@	Author:	P.Zahl
 *@	Zweck:	DSP Programm für AFM/STM/SARLS/SPA
 *@     
 *@     Apparaturn Configuration Selection File
 *@    
 *@     $Header: /home/ventiotec/gxsm-cvs/Gxsm-2.0/include/dsp-pci32/xsm/mover.h,v 1.1 2003-04-05 12:57:25 zahl Exp $
 */

/* Besocke Typ... */
#define MD__AFMADJ  0x4000  /* AFM Adjust Modus */
#define MOVER_MSK   (STMMode | MD__AFMADJ)
//#define MOVER_MSK ((STMMode & ~(MD_PID)) | MD__AFMADJ) // disable PID

/* Mover Steuerung */
long afm_mover_mode=0;    /* Mode s.u. */
long afm_piezo_amp=10000; /* Rampenamplitude in DA */
long afm_piezo_speed=2;   /* Ramp-Speed  U = (count+=speed)^2 */
long afm_piezo_steps=10;  /* Anzahl Schritte je Kommando */
long afm_u_piezo;
long afm_u_piezomax;
unsigned long AFM_MV_count=0;  /* Zeitbasis für Rampe */
unsigned long AFM_MV_Scount=0; /* Schrittzähler */
int AFM_MV_dir=0;
int afm_mover_flg=0;

int afm_mover_app=0;

/* Mover Modes */
#define AFM_MOV_RESET   0
#define AFM_MOV_XP      1
#define AFM_MOV_XM      2
#define AFM_MOV_YP      3
#define AFM_MOV_YM      4
#define AFM_MOV_QM      5
#define AFM_MOV_QP      6

#define SetupMover(direction) { \
		AFM_MV_count = 0L; \
		AFM_MV_Scount = 0L; \
		afm_mover_mode = direction; \
		LetzterSTMMode = STMMode; \
		STMMode = MOVER_MSK; \
		DSPack; \
	}

/* END */
