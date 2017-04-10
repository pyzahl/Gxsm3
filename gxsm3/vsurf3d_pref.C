/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
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

Surf3d_GLview_data GLvd_offset; // dummy

#define GET_GLV_OFFSET(ELEM) (void*)((long)(ELEM)-(long)(&GLvd_offset))

const gchar *TrueFalse_OptionsList[]  = { "true", "false", NULL };
const gchar *OnOff_OptionsList[]      = { "On", "Off", NULL };
const gchar *VertexSrc_OptionsList[]  = { "Flat", "Direct Height", "View Mode Height", "X-Channel", "Y", NULL };
const gchar *XYZ_OptionsList[]        = { "X", "Y", "Z", "Volume", "Scatter", NULL };
const gchar *ShadeModel_OptionsList[]  = { "Lambertian, use Palette",
					   "Terrain",
					   "Flat Material Color",
					   "RGBA map, Lambertian",
					   "Debug Shader",
					   NULL };
const gchar *ColorSrc_OptionsList[]   = { "Flat", "Direct Height", "View Mode Height", "X-Channel", "Y", NULL };
const gchar *TickFrame_OptionsList[]  = { "0: Simple", "1: XYZ with Labels", "2: XYZ Box", "3: XYZ w L Box", NULL };

const gchar *CScale_OptionsList[] = { "-3","3","0.001","0.1","4", NULL };
const gchar *ColorContrast_OptionsList[] = { "0","2","0.001","0.1","4", NULL };
const gchar *ColorOffset_OptionsList[]   = { "-1","1","0.01","0.1","3", NULL };

const gchar *Rot_OptionsList[]   = {"-180","180","1","1","0",NULL };
const gchar *FoV_OptionsList[]   = {"0","180","1","1","0",NULL };
const gchar *Dist_OptionsList[]  = {"0.1","1000","0.1","1","2",NULL };
const gchar *PerspF_OptionsList[]  = {"10","1000","1","1","0",NULL };
const gchar *PerspN_OptionsList[]  = {"0.1","100","1","1","2",NULL };
const gchar *Hskl_OptionsList[]  = {"-1","1","0.0001","0.002","4",NULL };
const gchar *Tskl_OptionsList[]  = {"-10","10","0.01","0.01","1",NULL };
const gchar *Slice_OptionsList[]  = {"-5","5","0.01","1","2",NULL };
const gchar *Shininess_OptionsList[]  = {"0","100","0.1","1","1",NULL };
const gchar *FogD_OptionsList[]  = {"0","100","0.001","0.01","2",NULL };
const gchar *shader_mode_OptionsList[]  = {"0","20","1","1","0",NULL };
const gchar *tess_level_OptionsList[]  = {"1","64","1.0","1.0","1",NULL };

GnomeResEntryInfoType v3dControl_pref_def_const[] = {
	GNOME_RES_ENTRY_FIRST_NAME("GXSM_V3DCONTROL_20030901000"),

// ============ View

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/RotationX", "Rotation X", "0", GET_GLV_OFFSET (&GLvd_offset.rot[0]), 
	  Rot_OptionsList, N_("View"),
	  N_("Rotation Angle in X"), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/RotationY", "Rotation Y", "-40", GET_GLV_OFFSET (&GLvd_offset.rot[1]), 
	  Rot_OptionsList, N_("View"),
	  N_("Rotation Angle in Y"), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/RotationZ", "Rotation Z", "0", GET_GLV_OFFSET (&GLvd_offset.rot[2]), 
	  Rot_OptionsList, N_("View"),
	  N_("Rotation Angle in Z"), NULL
		),
		
	GNOME_RES_ENTRY_SEPARATOR (N_("View"), NULL),

	GNOME_RES_ENTRY_FLOAT_VEC3
	( "V3dControl.View/Translation", "Translation", "0 0 0", GET_GLV_OFFSET (&GLvd_offset.trans), N_("View"),
	  N_("set translations vector X, Y, Z\n"
	     "additional hint: use middle mouse to\n"
	     "button and drag surface up/down!"), NULL
		),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/FoV", "FoV", "45.0", GET_GLV_OFFSET (&GLvd_offset.fov), 
	  FoV_OptionsList, N_("View"),
	  N_("Field of View"), NULL
		),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/Dist", "Distance", "10.0", GET_GLV_OFFSET (&GLvd_offset.dist), 
	  Dist_OptionsList, N_("View"),
	  N_("Distance"), NULL
		),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/PerspZfar", "PerspZfar", "100", GET_GLV_OFFSET (&GLvd_offset.Zfar), 
	  PerspF_OptionsList, N_("View"),
	  N_("Perspective: Z far distance relative to surface width (=1))"),
	     NULL
		  ),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/PerspZnear", "PerspZnear", "1", GET_GLV_OFFSET (&GLvd_offset.Znear), 
	  PerspN_OptionsList, N_("View"),
	  N_("Perspective: Z near distance (to center of surface)\n"),
	     NULL
		  ),
	  
	GNOME_RES_ENTRY_SEPARATOR (N_("View"), NULL),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/Hskl", "Height Skl", "1", GET_GLV_OFFSET (&GLvd_offset.hskl), 
	  Hskl_OptionsList, N_("View"),
	  N_("Height Scaling (Z scale)"), NULL
		),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/Tskl", "Tetra Skl", "1", GET_GLV_OFFSET (&GLvd_offset.tskl), 
	  Tskl_OptionsList, N_("View"),
	  N_("Tetra Scaling (Triangle scale)"), NULL
		),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.View/SliceOffset", "Slice Offset", "1", GET_GLV_OFFSET (&GLvd_offset.slice_offset), 
	  Slice_OptionsList, N_("View"),
	  N_("Volume Slice Offset Z"), NULL
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.View/VertexSource", "Direct Height", GET_GLV_OFFSET (&GLvd_offset.vertex_source[0]),
	  VertexSrc_OptionsList, N_("View"), 
	  N_("Vertex/Geometry Height Source for Surface Model.)")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.View/SliceDirection", "Z", GET_GLV_OFFSET (&GLvd_offset.slice_direction[0]),
	  XYZ_OptionsList, N_("View"), 
	  N_("Slicing Direction X,Y or Z (Z is default surface plane)")
		),

	GNOME_RES_ENTRY_FLOAT_VEC4
	( "V3dControl.View/SliceLimiter", "SliceLimiter", "0 0 1 0", GET_GLV_OFFSET (&GLvd_offset.slice_start_n[0]), 
	  N_("View"), N_("Slicing Control, set #num to 0 for all: [Start, #num, Step, Sep]"), NULL
		),

	GNOME_RES_ENTRY_FLOAT_VEC4
	( "V3dControl.View/SlicePlanes", "SlicePlanes", "-1 -1 -1 0", GET_GLV_OFFSET (&GLvd_offset.slice_plane_index[0]), 
	  N_("View"), N_("Slice Plane to add, set to -1 for off, else index [X, Y, Z, mode]"), NULL
		),

// ============ Light

	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/GlobalAmbient", "GlobalAmbient", "1 1 1 1", GET_GLV_OFFSET (&GLvd_offset.light_global_ambient), N_("Light"),
	  N_("Global Ambient Light [red, green, blue, alpha]"), NULL
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Light"), NULL),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.Light/Light0", "On", GET_GLV_OFFSET (&GLvd_offset.light[0][0]),
	  OnOff_OptionsList, N_("Light"), 
	  N_("Light0: switch On/Off")
		),
	GNOME_RES_ENTRY_FLOAT_VEC4
	( "V3dControl.Light/Light0Pos", "Light0Pos", "0.2 -1.0 0.0 1", GET_GLV_OFFSET (&GLvd_offset.light_position[0]), N_("Light"),
	  N_("Light0: Position, relative to surface width (=1) [X, Y, Z, 1]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light0Spec", "Light0Spec", "1 1 1 1", GET_GLV_OFFSET (&GLvd_offset.light_specular[0]), N_("Light"),
	  N_("Light0: Specular Light Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light0Diff", "Light0Diff", "0.4 0.4 0.4 1", GET_GLV_OFFSET (&GLvd_offset.light_diffuse[0]), N_("Light"),
	  N_("Light0: Diffuse Light Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light0Amb", "Light0Amb", "0.1 0.1 0.1 1", GET_GLV_OFFSET (&GLvd_offset.light_ambient[0]), N_("Light"),
	  N_("Light0: Ambient Light Color [red, green, blue, alpha]"), NULL
		),


	GNOME_RES_ENTRY_SEPARATOR (N_("Light"), NULL),

	GNOME_RES_ENTRY_AUTO_PATH_OPTION
	( GNOME_RES_STRING, "V3dControl.Light/Light1", "On", GET_GLV_OFFSET (&GLvd_offset.light[1][0]),
	  OnOff_OptionsList, N_("Light"), 
	  N_("Light1: switch On/Off")
		),
	GNOME_RES_ENTRY_FLOAT_VEC4
	( "V3dControl.Light/Light1Pos", "Light1Pos", "0.5 0 -0.5 0", GET_GLV_OFFSET (&GLvd_offset.light_position[1]), N_("Light"),
	  N_("Light1: Position, relative to surface width (=1) [X, Y, Z, 1]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light1Spec", "Light1Spec", "1 1 1 1", GET_GLV_OFFSET (&GLvd_offset.light_specular[1]), N_("Light"),
	  N_("Light1: Specular Light Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light1Diff", "Light1Diff", "0.4 0.4 0.4 1", GET_GLV_OFFSET (&GLvd_offset.light_diffuse[1]), N_("Light"),
	  N_("Light1: Diffuse Light Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light1Amb", "Light1Amb", "0.1 0.1 0.1 1", GET_GLV_OFFSET (&GLvd_offset.light_ambient[1]), N_("Light"),
	  N_("Light1: Ambient Light Color [red, green, blue, alpha]"), NULL
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Light"), NULL),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.Light/Light2", "Off", GET_GLV_OFFSET (&GLvd_offset.light[2][0]),
	  OnOff_OptionsList, N_("Light"), 
	  N_("Light2: switch On/Off")
		),
	GNOME_RES_ENTRY_FLOAT_VEC4
	( "V3dControl.Light/Light2Pos", "Light2Pos", "1 1 1 0", GET_GLV_OFFSET (&GLvd_offset.light_position[2]), N_("Light"),
	  N_("Light2: Position, relative to surface width (=1) [X, Y, Z, 0]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light2Spec", "Light2Spec", "1 1 1 1", GET_GLV_OFFSET (&GLvd_offset.light_specular[2]), N_("Light"),
	  N_("Light2: Specular Light Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light2Diff", "Light2Diff", "0.1 0.1 0.1 1", GET_GLV_OFFSET (&GLvd_offset.light_diffuse[2]), N_("Light"),
	  N_("Light2: Diffuse Light Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.Light/Light2Amb", "Light2Amb", "0.1 0.1 0.1 1", GET_GLV_OFFSET (&GLvd_offset.light_ambient[2]), N_("Light"),
	  N_("Light2: Ambient Light Color [red, green, blue, alpha]"), NULL
		),

// ============ Material Surface

	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatSurf/Ambient", "Ambient", "0.45 0.15 0.07 1", GET_GLV_OFFSET (&GLvd_offset.surf_mat_ambient), N_("Surface Material"),
	  N_("Surface Ambient Color:\n Specify the ambient RGBA reflectance of the material.\nUsed only in \"Uniform/Material Color Mode\"."), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatSurf/Diffuse", "Diffuse", "0.6 0.2 0.1 1", GET_GLV_OFFSET (&GLvd_offset.surf_mat_diffuse), N_("Surface Material"),
	  N_("Surface Diffuse Color:\n Specify the diffuse RGBA reflectance of the material.\nUsed only in \"Uniform/Material Color Mode\"."), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatSurf/Specular", "Specular", "1 1 0.3 1", GET_GLV_OFFSET (&GLvd_offset.surf_mat_specular), N_("Surface Material"),
	  N_("Surface Specular Color:\n Specify the specular RGBA reflectance of the material."), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatSurf/Shininess", "Shininess", "7", GET_GLV_OFFSET (&GLvd_offset.surf_mat_shininess[0]), 
	  Shininess_OptionsList, N_("Surface Material"),
	  N_("Surface Shininess"), NULL
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Surface Material"), NULL),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.MatSurf/ColorSrc", "Direct Height", 
	  GET_GLV_OFFSET (&GLvd_offset.ColorSrc[0]),
	  ColorSrc_OptionsList, N_("Surface Material"), 
	  N_("Select Surface Color Source.")
		),
	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.MatSurf/ShadeModel", "Lambertian, use Palette",
	  GET_GLV_OFFSET (&GLvd_offset.ShadeModel[0]),
	  ShadeModel_OptionsList, N_("Surface Material"), 
	  N_("Select Surface Shading Mode.")
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Surface Material"), NULL),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatSurf/ColorContrast", "Color Contrast", "2", 
	  GET_GLV_OFFSET (&GLvd_offset.ColorContrast), 
	  CScale_OptionsList, N_("Surface Material"),
	  N_("Color Contrast: to scale color source"), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatSurf/ColorOffset", "Color Offset", "0", 
	  GET_GLV_OFFSET (&GLvd_offset.ColorOffset), 
	  CScale_OptionsList, N_("Surface Material"),
	  N_("Color Contrast: shift color source"), NULL
		),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatSurf/ColorSaturation", "Color Saturation", "1", 
	  GET_GLV_OFFSET (&GLvd_offset.ColorSat), 
	  CScale_OptionsList, N_("Surface Material"),
	  N_("Color Saturation, 1=normal"), NULL
		),
	GNOME_RES_ENTRY_SEPARATOR (N_("Surface Material"), NULL),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatSurf/Transparency", "Transparency", "1", 
	  GET_GLV_OFFSET (&GLvd_offset.transparency), 
	  ColorContrast_OptionsList, N_("Surface Material"),
	  N_("Volume Model Transparency"), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatSurf/TransparencyOffset", "Trans. Offset", "0", 
	  GET_GLV_OFFSET (&GLvd_offset.transparency_offset), 
	  CScale_OptionsList, N_("Surface Material"),
	  N_("Volume Model Transparency Offset"), NULL
		),

// ============ Zeroplane/Box Material

	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatBox/Color", "Color", "0.6 0.1 0.1 1", GET_GLV_OFFSET (&GLvd_offset.box_mat_color), N_("Box Material"),
	  N_("Box Color [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatBox/Ambient", "Ambient", "0.6 0.1 0.1 1", GET_GLV_OFFSET (&GLvd_offset.box_mat_ambient), N_("Box Material"),
	  N_("Box Ambient [red, green, blue, alpha]"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatBox/Specular", "Specular", "1 1 1 1", GET_GLV_OFFSET (&GLvd_offset.box_mat_specular), N_("Box Material"),
	  N_("Box Specular"), NULL
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.MatBox/Diffuse", "Diffuse", "0 0 0 1", GET_GLV_OFFSET (&GLvd_offset.box_mat_diffuse), N_("Box Material"),
	  N_("Box Diffuse"), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.MatBox/Shininess", "Shininess", "20", GET_GLV_OFFSET (&GLvd_offset.box_mat_shininess[0]), 
	  Shininess_OptionsList, N_("Box Material"),
	  N_("Box Shininess"), NULL
		),

// ============ Rendering Options

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.RenderOp/ShadingMode", "Fragment Debug Shading Mode Selector", "0.0", GET_GLV_OFFSET (&GLvd_offset.shader_mode), 
	  shader_mode_OptionsList, N_("Render Opt."),
	  N_("Fragment Shading Mode Selector for Debug Shader Only:\n"
	     "0: (ambient+specular+diffuse)*color\n"
	     "1: (ambient+diffuse)*color\n"
	     "2: diffuse\n"
	     "3: (ambient+specular)*color\n"
	     "4: color\n"
	     "5: applyFog (specular+diffuse)*color, distance, viewDir)\n"
	     "*** only in debug mode/adjustfragment shader! ***\n"
	     "*11: vec4(vec3(height/100.)*0.5+0.5, 1.0)\n"
	     "*12: vec4(normal*0.5+0.5, 1.0)\n"
	     "*13: specular*color\n"
	     "*14: vec4(vec3(specular,normal.z,diffuse),1.0)\n"
	     "*15: vec4(vec3(dist/100.), 1.0)\n"
	     "*16: vec4(matColor, 1.0)\n"
	     ), NULL
	  ),

	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.RenderOp/TessLevel", "Tesselation Level", "32.0", GET_GLV_OFFSET (&GLvd_offset.tess_level), 
	  tess_level_OptionsList, N_("Render Opt."),
	  N_("Tesseletion Level Max:\n"
	     " 32: normal, 1: no tesselation"), NULL
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Render Opt."), NULL),

	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.RenderOp/ClearColor", "ClearColor", "0.6 0.7 0.7 1.0", GET_GLV_OFFSET (&GLvd_offset.clear_color), N_("Render Opt."),
	  N_("GL Clear Color, e.g. background  [red, green, blue, alpha]"), NULL
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Render Opt."), NULL),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Fog", "false", GET_GLV_OFFSET (&GLvd_offset.Fog),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable fog")
		),
	GNOME_RES_ENTRY_COLORSEL
	( "V3dControl.RenderOp/FogColor", "FogColor", "0.6 0.7 0.7 1.0", GET_GLV_OFFSET (&GLvd_offset.fog_color), N_("Render Opt."),
	  N_("Fog Color [red, green, blue, alpha].\n"
	     "Fog blends a fog color with each rasterized pixel fragment's\n"
	     "posttexturing color using a blending factor f. Factor f is computed in\n"
	     "one of three ways (here: f=exp(-density*z)), depending on the fog\n"
	     "mode. Let z be the distance in eye coordinates from the origin to the\n"
	     "fragment being fogged.\n"
	     "Note: you usually want to use the background (clear) color."), NULL
		),
	GNOME_RES_ENTRY_FLOATSLIDER
	( "V3dControl.RenderOp/FogDensity", "Fog Density", "0.8", GET_GLV_OFFSET (&GLvd_offset.fog_density), 
	  FogD_OptionsList, N_("Render Opt."),
	  N_("Density of fog, see fog color help!\n"
	     "Effect depends on distance to object and size of object."), NULL
		),

	GNOME_RES_ENTRY_SEPARATOR (N_("Render Opt."), NULL),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Tex", "false", GET_GLV_OFFSET (&GLvd_offset.Texture),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable texture (sorry not implemented today)")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Mesh", "false", GET_GLV_OFFSET (&GLvd_offset.Mesh),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable mesh mode (use mesh or solid model)")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/TransparentSlices", "false", GET_GLV_OFFSET (&GLvd_offset.TransparentSlices),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable transparency for normal non voluem view/slices")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Emission", "false", GET_GLV_OFFSET (&GLvd_offset.Emission),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable emission mode")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Cull", "false", GET_GLV_OFFSET (&GLvd_offset.Cull),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable cull face mode (surface back is not drawn!)\n"
	     "Note: if enabled you will not see your surface,\n"
	     "if you are looking from below!")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Smooth", "true", GET_GLV_OFFSET (&GLvd_offset.Smooth),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable Smooth Shading (GL color model is smooth or flat)\n"
	     "Note: smooth shading is much slower but looks also much better.")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Ortho", "false", GET_GLV_OFFSET (&GLvd_offset.Ortho),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable ortho mode (not implemented)")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/ZeroPlane", "true", GET_GLV_OFFSET (&GLvd_offset.ZeroPlane),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable Zero Plane/Box")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_BOOL, "V3dControl.RenderOp/Tickmarks", "false", GET_GLV_OFFSET (&GLvd_offset.Ticks),
	  TrueFalse_OptionsList, N_("Render Opt."), 
	  N_("enable/disable 3D Tickmarks drawing\n (Note: not yet available!!)")
		),

	GNOME_RES_ENTRY_OPTION
	( GNOME_RES_STRING, "V3dControl.RenderOp/TickFrameOptions", "0: Simple", GET_GLV_OFFSET (&GLvd_offset.TickFrameOptions[0]),
	  TickFrame_OptionsList, N_("Render Opt."), 
	  N_("Tick Frame Options)")
		),

	GNOME_RES_ENTRY_LAST
};

