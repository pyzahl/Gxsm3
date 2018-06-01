/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: pyremote.C
 * ========================================
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
 * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
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

const gchar *template_library_utils = R"V0G0N(
# Gxsm Python Script Library Template
# Generic Utilities

)V0G0N";


const gchar *template_library_control = R"V0G0N(
# Gxsm Python Script Library Template
# Instrument Control Operations

)V0G0N";


const gchar *template_library_scan = R"V0G0N(
# Gxsm Python Script Library Template
# Scan Operations

)V0G0N";


const gchar *template_library_probe = R"V0G0N(
# Gxsm Python Script Library Template
# Vector Probe Operations

)V0G0N";


const gchar *template_library_analysis = R"V0G0N(
# Gxsm Python Script Library Template
# Analysis functions

# fetch dimensions
def get_dimensions(ch):
    dims=gxsm.get_dimensions(ch)
    geo=gxsm.get_geometry(ch)
    diffs_f=gxsm.get_differentials(ch)
    return dims, geo, diffs

)V0G0N";


/*
const gchar *template_name = R"V0G0N(
...py script ...
)V0G0N";
*/
