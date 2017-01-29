/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

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

#include <locale.h>
#include <libintl.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "gnome-res.h"

#include "view.h"
#include "mem2d.h"
#include "xsmmasks.h"
#include "glbvars.h"

#include "bench.h"
#include "util.h"

#include "app_v3dcontrol.h"

#include <gtk/gtk.h>
#include "action_id.h"

#include "vsurf3d_pref.C"

#include "xsmdebug.h"


// not yet done
#undef __ALTIVEC__

#ifdef __ALTIVEC__
#include <altivec.h> /* it's here: /usr/lib/gcc-lib/powerpc-linux/3.3.1/include/altivec.h */
#endif

#define ZRANGE_MAPPED 4096

// some vector utilities

#ifdef __ALTIVEC__
void copyvec3 (float4 &u, float4 &v){ u=v; }
#else
void copyvec3 (GLfloat u[3], GLfloat v[3]){
	for (int i=0; i<3; ++i)
		u[i] = v[i];
}
#endif

#ifdef __ALTIVEC__
void copyvec4 (float4 *u, float4 *v){ *u = *v; }
#else
void copyvec4 (GLfloat u[4], GLfloat v[4]){
	for (int i=0; i<4; ++i)
		u[i] = v[i];
}
#endif

#ifdef __ALTIVEC__
float dotprod (float4 *v1, float4 *v2){
	;
}
#else
GLfloat dotprod (GLfloat v1[3], GLfloat v2[3]){
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}
#endif

#ifdef __ALTIVEC__
inline void addtovec (float4 *u, float4 *v){
	u = vec_add_float4 (u, v);
}
#else
void addtovec (GLfloat u[3], GLfloat v[3]){
	for (int i=0; i<3; ++i)
		u[i] += v[i];
}
#endif

#ifdef __ALTIVEC__
#else
void mulvecwf (GLfloat u[3], GLfloat f){
	for (int i=0; i<3; ++i)
		u[i] *= f;
}
#endif

#ifdef __ALTIVEC__
#else
void normcrossprod (GLfloat v1[3], GLfloat v2[3], GLfloat n[3]){
	GLfloat d;
	n[0] = v1[1]*v2[2] - v1[2]*v2[1];
	n[1] = v1[2]*v2[0] - v1[0]*v2[2];
	n[2] = v1[0]*v2[1] - v1[1]*v2[0];
	d = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
	n[0] /= d;
	n[1] /= d;
	n[2] /= d;
}
#endif

#ifdef __ALTIVEC__
#else
GLfloat norm_copy(GLfloat u[3], GLfloat v[3]){
	GLfloat d;
	v[0] = u[0];
	v[1] = u[1];
	v[2] = u[2];
	d = sqrt(u[0]*u[0] + u[1]*u[1] + u[2]*u[2]);
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
        return d;
}
#endif

#ifdef __ALTIVEC__
#else
GLfloat norm (GLfloat v[3]){
	GLfloat d;
	d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
        return d;
}
#endif

#ifdef __ALTIVEC__
#else
void norm3pkte (GLfloat p1[3], GLfloat p2[3], GLfloat p3[3], GLfloat n[3]){
	GLfloat v1[3], v2[3];

	v1[0] = p2[0] - p1[0];
	v1[1] = p2[1] - p1[1];
	v1[2] = p2[2] - p1[2];

	v2[0] = p3[0] - p1[0];
	v2[1] = p3[1] - p1[1];
	v2[2] = p3[2] - p1[2];

	normcrossprod (v1, v2, n);
}
#endif


// v[0][] = vertex for normal to be found
// v[1,2,3,4][] = vertices around
// n[] averaged normal
#ifdef __ALTIVEC__
#else
void avgpolynorm (GLfloat v[5][3], GLfloat n[3]){
	GLfloat a[4], as;
	GLfloat u[4][3];
	GLfloat ni[3];
	
	n[0]=n[1]=n[2]=0.;

	for (int i=0; i<4; ++i)
		for (int j=0; j<3; ++j)
			u[i][j] = v[i+1][j] - v[0][j];
	
	as = 0.;
	for (int i=0; i<4; ++i)
		as += (a[i] = 1./cos (dotprod (u[i], u[(i+1)%4])));
		
	for (int i=0; i<4; ++i){
		normcrossprod (u[i], u[(i+1)%4], ni);
		mulvecwf (ni, a[i]);
		addtovec (n, ni);
	}
	mulvecwf (n, 1./as);
}
#endif



// Replaces gluPerspective. Sets the frustum to perspective mode.
// fovY     - Field of vision in degrees in the y direction
// aspect   - Aspect ratio of the viewport
// zNear    - The near clipping distance
// zFar     - The far clipping distance
 
void perspective_GL (GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
        const GLdouble pi = M_PI;
        GLdouble fW, fH;
        // Calculate the distance from 0 of the y clipping plane. Basically trig to calculate position of clipper at zNear.
        fH = tan( fovY / 360 * pi ) * zNear;

        // Calculate the distance from 0 of the x clipping plane based on the aspect ratio.
        fW = fH * aspect;

        // Finally call glFrustum, this is all gluPerspective does anyway!
        // This is why we calculate half the distance between the clipping planes
        // - glFrustum takes an offset from zero for each clipping planes distance. (Saves 2 divides)
        g_print ("perspective_GL: %g\n", fovY);
        glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

// Compat method: gluLookAt deprecated
void look_at_GL (GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ,
                 GLfloat lookAtX, GLfloat lookAtY, GLfloat lookAtZ,
                 GLfloat upX, GLfloat upY, GLfloat upZ) {
        GLfloat x[3];
        GLfloat y[3] = { upX, upY, upZ };
        GLfloat z[3] = { eyeX-lookAtX, eyeY-lookAtY, eyeZ-lookAtZ };
        
        //z = Vector3f(eyeX-lookAtX, eyeY-lookAtY, eyeZ-lookAtZ).normalize();
        //y = Vector3f(upX, upY, upZ);

        normcrossprod (y, z, x);
        normcrossprod (z, x, y);
        // norm (x);
        // norm (y);
        //x = y ^ z;
        //y = z ^ x;
        //x = x.normalize();
        //y = y.normalize();
        // mat is given transposed so OpenGL can handle it.
        //Matrix4x4 mat (new GLfloat[16]
        //            {x.getX(), y.getX(),   z.getX(),   0,
        //             x.getY(),  y.getY(),   z.getY(),   0,
        //             x.getZ(),  y.getZ(),   z.getZ(),   0,
        //             -eyeX,     -eyeY,      -eyeZ,      1});
        GLfloat mat[4][4] = {
                { x[0],  y[0],   z[0],   0. },
                { x[1],  y[1],   z[1],   0. },
                { x[2],  y[2],   z[2],   0. },
                { -eyeX, -eyeY, -eyeZ,   1. }
        };

        g_print ("look_at_GL: ");

        for (int i=0; i<4; ++i){
                g_print ("\n (");
                for (int j=0; j<4; ++j)
                        g_print ("%6g  ", mat[i][j]);
                g_print (")");
        }
        g_print ("\n");
        
        glMultMatrixf (*mat);
}


void surf3d_write_schema (){
	GnomeResPreferences *gl_pref = gnome_res_preferences_new (v3dControl_pref_def_const, GXSM_RES_GL_PATH);
	gchar *gschema = gnome_res_write_gschema (gl_pref);
                
	std::ofstream f;
	f.open (GXSM_RES_BASE_PATH_DOT ".gl.gschema.xml", std::ios::out);
	f << gschema
	  << std::endl;
	f.close ();

	g_free (gschema);
	
	gnome_res_destroy (gl_pref);
}

// ============================== Class Surf3d ==============================

Surf3d::Surf3d(Scan *sc, int ChNo):View(sc, ChNo){
	XSM_DEBUG (DBG_L2, "Surf3d::Surf3d");
	v3dcontrol = NULL;
        self = this;
	GLvarinit();
}

Surf3d::Surf3d():View(){
	v3dcontrol = NULL;
	GLvarinit();
}

Surf3d::~Surf3d(){
	XSM_DEBUG (DBG_L2, "Surf3d::~");
        self = NULL;
	if (v3dControl_pref_dlg)
		gnome_res_destroy (v3dControl_pref_dlg);
	hide();
	DelSmem();
	g_free (v3dControl_pref_def);
}

void Surf3d::hide(){
	if (v3dcontrol)
		delete v3dcontrol;
	v3dcontrol = NULL;
	XSM_DEBUG (DBG_L2, "Surf3d::hide");
}

void inline Surf3d::PutPointMode(int k, int j, int vi){
	int i;
	GLfloat val;

	XSM_DEBUG (DBG_L11, "Surf3d::PutPointMode " << k << "," << j << "," << vi << " Q=" << QuenchFac << std::endl << std::flush);
        //        std::cout << "PutPointMode " << k << "," << j << "," << vi << " Q=" << QuenchFac << std::endl << std::flush;
        
	i = (k/QuenchFac) + (j/QuenchFac)*XPM_x + XPM_x*XPM_y*vi;
        //        std::cout << "PutPointMode i=" << i << " size=" << (int)size << std::endl << std::flush;
	if (i >= (int)size) return;
        //        std::cout << "PutPointMode..." << std::endl << std::flush;

	surface[i] = (GLfloat)(scan->mem2d->GetDataVModeInterpol ((double)k,(double)j,(double)vi)/scan->mem2d->GetDataRange ());

	if (GLv_data.ColorSrc[0] != 'U'){
		switch (GLv_data.ColorSrc[0]){
		case 'H': 
			val = (GLfloat) (GLv_data.ColorOffset + GLv_data.ColorContrast * surface[i]);
			break;
		case 'X': 
			if (mem2d_x){
				// map to index range of Chan-X, assumes scan range is the same.
				int u = (int) (k * (double)mem2d_x->GetNx () / (double)scan->mem2d->GetNx ());
				int v = (int) (j * (double)mem2d_x->GetNy () / (double)scan->mem2d->GetNy ());
				val = (GLfloat) (GLv_data.ColorOffset + 
						 GLv_data.ColorContrast * mem2d_x->GetDataVMode (u,v)/mem2d_x->GetDataRange ());
			} else
				val = 1.;
			break;
		default: 
			val = 1.; 
			break;
		}

		switch (GLv_data.ColorMode[0]){
		case 'T': // Terrain Color
			calccolor(val*ZRANGE_MAPPED, surfacecolor[i]);
			break;
		case 'M': // Material Color
			surfacecolor[i][0] = GLv_data.surf_mat_color[0];
			surfacecolor[i][1] = GLv_data.surf_mat_color[1];
			surfacecolor[i][2] = GLv_data.surf_mat_color[2];
			break;
		case 'P': { // GXSM user Palette
			int ci = (int)(val*1024);
			ci = ci < 0 ? 0 : ci >= 1024 ? 1023 : ci;
			surfacecolor[i][0] = ColorLookup[ci][0];
			surfacecolor[i][1] = ColorLookup[ci][1];
			surfacecolor[i][2] = ColorLookup[ci][2];
		} break;
		case 'R': // RGBA Color
			switch (GLv_data.ColorSrc[0]){
			case 'H': 
				if (scan->mem2d->GetNv() == 4){
					surfacecolor[i][0] = scan->mem2d->GetDataPkt (k,j,0) / 255.;
					surfacecolor[i][1] = scan->mem2d->GetDataPkt (k,j,1) / 255.;
					surfacecolor[i][2] = scan->mem2d->GetDataPkt (k,j,2) / 255.;
				}
				break;
			case 'X': 
				if (mem2d_x){				
					int u = (int) (k * (double)mem2d_x->GetNx () / (double)scan->mem2d->GetNx ());
					int v = (int) (j * (double)mem2d_x->GetNy () / (double)scan->mem2d->GetNy ());
					if (mem2d_x->GetNv() == 4){
						surfacecolor[i][0] = mem2d_x->GetDataPkt (u,v,0) / 255.;
						surfacecolor[i][1] = mem2d_x->GetDataPkt (u,v,1) / 255.;
						surfacecolor[i][2] = mem2d_x->GetDataPkt (u,v,2) / 255.;
					}
				}
				break;
			}
			break;
		}  
		surfacecolor[i][0] *= GLv_data.ColorSat;
		surfacecolor[i][1] *= GLv_data.ColorSat;
		surfacecolor[i][2] *= GLv_data.ColorSat;
	}

	// set transparency/alpha blending
	val = (GLfloat) (GLv_data.transparency_offset + GLv_data.transparency * surface[i]);
	surfacecolor[i][3] = val > 1.? 1. : val < 0.? 0. : val;

	// adjust slice height level
	surface[i] *= GLv_data.hskl;  // (this is arbitrary scaling)
	surface[i] /= QuenchFac;
	surface[i] += (GLfloat)vi * GLv_data.slice_offset * (XPM_x+XPM_y)*0.5/XPM_v;

        //        std::cout << "PutPointMode OK!" << std::endl << std::flush;
}


void Surf3d::GLvarinit(){
	mem2d_x=NULL;
	
	glZeroFrameList=0; 
	glSurfaceList=NULL; 
	glSurfaceListRange=NULL;
	glVolumeXSList=NULL; 
	glVolumeXSListRange=NULL;
	glVolumeYSList=NULL; 
	glVolumeYSListRange=NULL;
	glSVolumeList=NULL; 
	glSVolumeListRange=NULL;
	
	XPM_x = XPM_y = 0;
	scrwidth=300;
	ZoomFac=1;
        QuenchFac=1;
        
	size=0L;
	valid=0;

	surface=NULL;
	surfacecolor=NULL;

// create preferences table from static table and replace pointers to
// functions with callbacks of this instance
	GnomeResEntryInfoType *res = v3dControl_pref_def_const;
	int n;
	for (n=0; res->type != GNOME_RES_LAST; ++res, ++n); 
	++n; // include last!

	v3dControl_pref_def = g_new (GnomeResEntryInfoType, n);

	res = v3dControl_pref_def_const;
	GnomeResEntryInfoType *rescopy = v3dControl_pref_def;
	for (; res->type != GNOME_RES_LAST; ++res, ++rescopy){
		memcpy (rescopy, res, sizeof (GnomeResEntryInfoType));
		rescopy->var = (void*)((long)res->var + (long)(&GLv_data));
		rescopy->changed_callback = GLupdate;
		rescopy->moreinfo = (gpointer) this;
	}
	memcpy (rescopy, res, sizeof (GnomeResEntryInfoType)); // do last!

	// read user values or make defaults if not present
	v3dControl_pref_dlg = gnome_res_preferences_new (v3dControl_pref_def, GXSM_RES_GL_PATH);
	gnome_res_set_apply_callback (v3dControl_pref_dlg, GLupdate, (gpointer)this);
	gnome_res_set_destroy_on_close (v3dControl_pref_dlg, FALSE);
	gnome_res_read_user_config (v3dControl_pref_dlg);
	
	ReadPalette (xsmres.Palette);
}

void Surf3d::GLupdate (void* data){
	XSM_DEBUG (DBG_L2, "SURF3D:::GLUPDATE" << std::flush);

        if (data){
        
                Surf3d *s = (Surf3d *) data;

                XSM_DEBUG (DBG_L2, "SURF3D:::GLUPDATE set color source" << std::flush);
                s->ColorSrc();

                XSM_DEBUG (DBG_L2, "SURF3D:::GLUPDATE GetSmem" << std::flush);
                s->GetSmem ();

                XSM_DEBUG (DBG_L2, "SURF3D:::GLUPDATE rerender" << std::flush);
                if (s->v3dcontrol)
                        s->v3dcontrol->rerender_scene ();
        }
        XSM_DEBUG (DBG_L2, "SURF3D:::GLUPDATE done." << std::flush);
}


int Surf3d::GetSmem(){ 
	XSM_DEBUG (DBG_L2, "SURF3D:::getSmem");
	if (surface) DelSmem();

	QuenchFac = 1 + (int) ((double) scan->mem2d->GetNx () / (double) scrwidth
			       * 4. * GLv_data.preV);

	if (v3dcontrol){
		gchar *titel = g_strdup_printf ("MesaGL: Ch%d: %s Q%d %s", 
						ChanNo+1, data->ui.name, 
						QuenchFac, scan->mem2d->GetEname());
		
		v3dcontrol->SetTitle(titel);
		g_free(titel);
	}

	XPM_x = scan->mem2d->GetNx()/QuenchFac;
	XPM_y = scan->mem2d->GetNy()/QuenchFac;
	XPM_v = scan->mem2d->GetNv();

	size = XPM_x * XPM_y * XPM_v;

	surface = new GLfloat[size];
	if(!surface) { XSM_SHOW_MESSAGES(ERR_NOMEM); DelSmem(); return -1; }

	surfacecolor = new GLfloat*[size];
	if(!surfacecolor) { XSM_SHOW_MESSAGES(ERR_NOMEM); DelSmem(); return -1; }
	for(int i=0; i<(int)size; i++){
		surfacecolor[i] = new GLfloat[4];
		if(!surfacecolor[i]) { 
			XSM_SHOW_MESSAGES(ERR_NOMEM); 
			DelSmem(); 
			return -1;
		}
	}

	setup_data_transformation();

	for(int v=0; v<scan->mem2d->GetNv(); ++v)
		for(int j=0; j<scan->mem2d->GetNy(); j+=QuenchFac)
			for(int k=0; k<scan->mem2d->GetNx(); k+=QuenchFac)
				PutPointMode (k,j,v);

	valid=2; // force GL lists to refresh

        if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return 0;
}

int Surf3d::DelSmem(){ 
	int vi;
	valid = 0;
	XSM_DEBUG (DBG_L2, "SURF3D:::DelSmem");

	if (surface)
		delete [] surface;
	surface = NULL;
	
	for (int i=0; i<(int)size; i++){
		if (surfacecolor[i])
			delete surfacecolor[i];
	}
	delete [] surfacecolor;
	surfacecolor = NULL;

	size = 0L;

	if (glIsList (glZeroFrameList))
		glDeleteLists (glZeroFrameList, 1);

	if (glSurfaceListRange){
		for(vi=0; vi<XPM_v; ++vi)
			if (glIsList (glSurfaceList[vi]))
				glDeleteLists (glSurfaceList[vi], glSurfaceListRange[vi]);
		delete [] glSurfaceListRange;
		glSurfaceListRange=NULL;
		glSurfaceList=NULL;
	}
	if (glVolumeXSListRange){
		for(vi=0; vi<XPM_v-1; ++vi)
			if (glIsList (glVolumeXSList[vi]))
				glDeleteLists (glVolumeXSList[vi], glVolumeXSListRange[vi]);
		delete [] glVolumeXSListRange;
		glVolumeXSListRange=NULL;
		glVolumeXSList=NULL;
	}
	if (glVolumeYSListRange){
		for(vi=0; vi<XPM_v-1; ++vi)
			if (glIsList (glVolumeYSList[vi]))
				glDeleteLists (glVolumeYSList[vi], glVolumeYSListRange[vi]);
		delete [] glVolumeYSListRange;	
		glVolumeYSListRange=NULL;
		glVolumeYSList=NULL;
	}
	if (glSVolumeListRange){
		for(vi=0; vi<XPM_v-1; ++vi)
			if (glIsList (glSVolumeList[vi]))
				glDeleteLists (glSVolumeList[vi], glSVolumeListRange[vi]);
		delete [] glSVolumeListRange;	
		glSVolumeListRange=NULL;
		glSVolumeList=NULL;
	}
	return 0;
}

void Surf3d::printstring(void *font, char *string)
{
 	int len,i;
 	len = (int)strlen(string);
 	for(i=0; i<len; i++)
 		;//glutBitmapCharacter(font,string[i]);
}

void Surf3d::RotateAbs(int n, double phi){
	GLv_data.rot[n] = phi;
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();
}

void Surf3d::Rotate(int n, double dphi){
	if(n>2){
		GLv_data.rot[1] += dphi;
		GLv_data.rot[2] += dphi;
	}
	else
		GLv_data.rot[n] += dphi;
}

double Surf3d::RotateX(double dphi){
	GLv_data.rot[0] += 5*dphi;
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.rot[0];
}

double Surf3d::RotateY(double dphi){
	GLv_data.rot[1] += 5*dphi;
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.rot[1];
}

double Surf3d::RotateZ(double dphi){
	GLv_data.rot[2] += 5*dphi;
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.rot[2];
}

void Surf3d::Translate(int n, double delta){ GLv_data.trans[n] += delta; }

double Surf3d::Zoom(double x){ 
	GLv_data.dist += 0.1*x; 
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.dist; 
}

double Surf3d::HeightSkl(double x){ 
	GLv_data.hskl += 0.1*x; 
	draw(); 
	return GLv_data.hskl; 
}

void Surf3d::ColorSrc(){
	mem2d_x=NULL;
	switch (GLv_data.ColorSrc[0]){
	case 'U': // Uniform Coloring
		draw();
		break;
	case 'X':  // from Channel X
		int ChSrc2;
		if ((ChSrc2=gapp->xsm->FindChan (ID_CH_M_X))>=0){
			if(gapp->xsm->scan[ChSrc2]){
				mem2d_x = gapp->xsm->scan[ChSrc2]->mem2d;
				draw (); // automatically maps to chan-x index range, assumes same size - user should know...
			}else{ XSM_SHOW_ALERT(ERR_SORRY, ERR_NO2SRC, HINT_MAKESRC2,1); }
		}else{ XSM_SHOW_ALERT(ERR_SORRY, ERR_NO2SRC, HINT_MAKESRC2,1); }
		break;
	case 'H': // from Height
		draw();
		break;
	}  
}

void Surf3d::GLModes(int n, int m){
	switch(n){
	case ID_GL_nZP: 
		GLv_data.ZeroPlane=m?1:0; break;
	case ID_GL_Mesh: 
		GLv_data.Mesh=m?1:0; break;
	case ID_GL_Ticks: 
		GLv_data.Ticks=m?1:0; break;
	case ID_GL_Cull: 
		GLv_data.Cull=m?1:0; break;
	case ID_GL_Smooth: 
		GLv_data.Smooth=m?1:0; break;
	}  
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();
}


void Surf3d::ReadPalette(char *name){
	if (name){
		std::ifstream cpal;
		char pline[256];
		int nx,ny;
		cpal.open(name, std::ios::in);
		if(cpal.good()){
			cpal.getline(pline, 255);
			cpal.getline(pline, 255);
			cpal >> nx >> ny;
			cpal.getline(pline, 255);
			cpal.getline(pline, 255);
			
			for (int i=0; i<1024; ++i){
				int r,g,b;
				cpal >> r >> g >> b;
				ColorLookup[i][0] = r/255.;
				ColorLookup[i][1] = g/255.;
				ColorLookup[i][2] = b/255.;
			}
			return;
		}
	}
	// default grey and fallback mode:
	for (int i=0; i<1024; ++i)
		ColorLookup[i][0] =
		ColorLookup[i][1] =
		ColorLookup[i][2] = i/1024.;
}

// height in [0..1] expected!
void Surf3d::calccolor(GLfloat height, GLfloat c[4])
{
	GLfloat color[4][3]={
		{1.0,1.0,1.0},
		{0.0,0.8,0.0},
		{1.0,1.0,0.3},
		{0.0,0.0,0.8}
	};
	GLfloat fact;
	
	if(height>=0.9) {
		c[0]=color[0][0]; c[1]=color[0][1]; c[2]=color[0][2];
		return;
	}
	
	if((height<0.9) && (height>=0.7)) {
		fact=(height-0.7)*5.0;
		c[0]=fact*color[0][0]+(1.0-fact)*color[1][0];
		c[1]=fact*color[0][1]+(1.0-fact)*color[1][1];
		c[2]=fact*color[0][2]+(1.0-fact)*color[1][2];
		return;
	}
	
	if((height<0.7) && (height>=0.6)) {
		fact=(height-0.6)*10.0;
		c[0]=fact*color[1][0]+(1.0-fact)*color[2][0];
		c[1]=fact*color[1][1]+(1.0-fact)*color[2][1];
		c[2]=fact*color[1][2]+(1.0-fact)*color[2][2];
		return;
	}
	
	if((height<0.6) && (height>=0.5)) {
		fact=(height-0.5)*10.0;
		c[0]=fact*color[2][0]+(1.0-fact)*color[3][0];
		c[1]=fact*color[2][1]+(1.0-fact)*color[3][1];
		c[2]=fact*color[2][2]+(1.0-fact)*color[3][2];
		return;
	}
	
	c[0]=color[3][0]; c[1]=color[3][1]; c[2]=color[3][2];
}


void Surf3d::setup_data_transformation(){
	scan->mem2d->SetDataPktMode (data->display.ViewFlg);
	scan->mem2d->SetDataRange (0, ZRANGE_MAPPED);
}

int Surf3d::update(int y1, int y2){
	XSM_DEBUG (DBG_L2, "SURF3D:::update y1 y2: " << y1 << "," << y2);

	if (!scan || size == 0) return -1;

	XSM_DEBUG (DBG_L2, "SURF3D:::update trafo");
	setup_data_transformation();

	XSM_DEBUG (DBG_L2, "SURF3D:::update data");
//	int v = scan->mem2d->GetLayer();
	for(int v=0; v<scan->mem2d->GetNv(); ++v)
		for(int j=y1; j < y2; j += QuenchFac)
			for(int k=0; k < scan->mem2d->GetNx (); k+=QuenchFac)
				PutPointMode (k,j,v);
	

	if (valid){
		valid=2; // force GL lists refresh
                XSM_DEBUG (DBG_L2, "SURF3D:::update rerender scene");
		if (v3dcontrol)
                        v3dcontrol->rerender_scene ();
	}

        XSM_DEBUG (DBG_L2, "SURF3D:::update done.");
	return 0;
}


#define DrawOneLine(x1,y1,z1,x2,y2,z2) \
                    glBegin(GL_LINES);  \
                    glVertex3f((x1),(y1),(z1));   glVertex3f((x2),(y2),(z2)); \
                    glEnd();

/* 
 * Tickmarks and other stuff....
 */

void Surf3d::GLdrawGimmicks(){
	// Variables for label-calculation
	double totalsize;
	double xt;
	double mag;
	double onetic;
	double offset; 
	double firststep; 
	double side = 1.0; // default side for labels
	GLfloat m[16],z;
	int i, stellen;

	char buf[80];
	
	// number of pixels = scan->mem2d->GetNx ()
	double x1 = scan->mem2d->data->GetXLookup((int)0);
	double y1 = scan->mem2d->data->GetYLookup((int)0);
	double x2 = scan->mem2d->data->GetXLookup(  scan->mem2d->GetNx()-1);
	double y2 = scan->mem2d->data->GetYLookup(  scan->mem2d->GetNy()-1);

	XSM_DEBUG (DBG_L2, "GL:::GLdrawGimmicks");
	XSM_DEBUG_GP (DBG_L2, "GL:::GLdrawGimmicks");

	glDisable (GL_BLEND); 
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, GLv_data.box_mat_color);

	if (GLv_data.Smooth)
		glEnable (GL_LINE_SMOOTH);

        // *** was disabled
        glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, GLv_data.box_mat_color);

	glLineWidth(0.5);

	if (GLv_data.TickFrameOptions[0]=='1' || GLv_data.TickFrameOptions[0]=='3'){
		// labels at tips of axis, is only confusing
		sprintf(buf,"(%f/%f)", x1, y1);
		;//glRasterPos3f(-(XPM_x/2.0),0.0,-(XPM_y/2.0)); printstring(GLUT_BITMAP_HELVETICA_12, buf);
		sprintf(buf,"(%f/%f)", x2, y1);
		;//glRasterPos3f( (XPM_x/2.0),0.0,-(XPM_y/2.0)); printstring(GLUT_BITMAP_HELVETICA_12, buf);
		sprintf(buf,"(%f/%f)", x1, y2);
		;//glRasterPos3f(-(XPM_x/2.0),0.0, (XPM_y/2.0)); printstring(GLUT_BITMAP_HELVETICA_12, buf);
		sprintf(buf,"(%f/%f)", x2, y2);
		glRasterPos3f( (XPM_x/2.0),0.0, (XPM_y/2.0)); printstring(GLUT_BITMAP_HELVETICA_12, buf);
	}
			
	glGetFloatv (GL_MODELVIEW_MATRIX, m);// Need this matrix for label side calcu.

	// X axis
	totalsize = (x2-x1);
	xt = 0.0;
	mag = log10(totalsize);
	onetic = pow(10, int(mag));
	offset = onetic*floor(x2/onetic); 
	firststep =  XPM_x/2.0  - (x2-offset)/totalsize*XPM_x ;
	
	i = 0;
	
	if (onetic >= 1)
		stellen = 0;
	else
		stellen = abs(int(mag+1));
	
	if (m[0] < 0) // put labels on other side
		side = -1.0;
	
	while (offset-i*onetic > x1){
		// There is something badly wrong with x-scaling,
		// When you crop a region, you dont get new offsets.
		xt = firststep - i*onetic/totalsize*XPM_x;
		if (GLv_data.TickFrameOptions[0]=='1' || GLv_data.TickFrameOptions[0]=='3'){
			sprintf(buf,"x=%.*f",stellen, offset-i*onetic);
			;//glRasterPos3f(xt, 0.0, 1.1*side*XPM_y/2.0); printstring(GLUT_BITMAP_HELVETICA_12, buf);
		}
		DrawOneLine(xt, 0.0, 1.1*(side*XPM_y/2.0), xt, 0.0, (side*XPM_y/2.0));
		i++;
	}
	
	// Y axis
	
	totalsize = (y1-y2); // y1-y2 is positive.
	xt = 0.0;
	mag = log10(totalsize);
	onetic = pow(10, int(mag));
	offset = onetic*floor(y1/onetic); 
	firststep = -XPM_y/2.0  + (y1-offset)/totalsize*XPM_y;
	i = 0;
	
	if (onetic >= 1)
			stellen = 0;
	else
			stellen = abs(int(mag+1));
		
	side = 1.0; // if matrix says so, change sides.
	if (m[8] > 0)
		side = -1.0;
	
	while (offset-i*onetic > y2){
		xt = firststep + i*onetic/totalsize*XPM_y;
		if (GLv_data.TickFrameOptions[0]=='1' || GLv_data.TickFrameOptions[0]=='3'){
			sprintf(buf,"y=%.*f",stellen, offset-i*onetic);
			;//glRasterPos3f(1.1*side*XPM_x/2.0 ,0.0, xt); printstring(GLUT_BITMAP_HELVETICA_12, buf);
		}
		DrawOneLine(1.1*(side*XPM_x/2.0), 0.0, xt, (side*XPM_x/2.0), 0.0, xt);
		i++;
	}

	// Z axis
	
	z = fabs (GLv_data.slice_offset * (XPM_x+XPM_y)*0.5);
	totalsize = fabs (XPM_v);
	xt = 0.0;
	mag = log10(totalsize);
	onetic = pow(10, int(mag));
	offset = onetic*floor(0./onetic); 
	firststep = 0.;
	i = 0;
	
	if (onetic >= 1)
			stellen = 0;
	else
			stellen = abs(int(mag+1));
		
	side = 1.0; // if matrix says so, change sides.
	if (m[8] > 0)
		side = -1.0;
	
	if (GLv_data.TickFrameOptions[0]=='1' || GLv_data.TickFrameOptions[0]=='3')
		while (i*onetic < XPM_v){
			xt = i*onetic/totalsize*z;
			sprintf(buf,"v=%.*f",stellen, i*onetic);
			;//glRasterPos3f(1.1*side*XPM_x/2.0 ,xt, -XPM_y/2.0); printstring(GLUT_BITMAP_HELVETICA_12, buf);
			DrawOneLine(1.1*side*XPM_x/2., xt, -XPM_y/2.0, side*XPM_x/2.0, xt, -XPM_y/2.0);
			DrawOneLine(side*XPM_x/2.0, xt, -1.1*XPM_y/2.0, side*XPM_x/2., xt, -XPM_y/2.0);
			i++;
		}

	// *** coordinate-axis ***
        // **** was disabled
	glColor4f(0.1,0.1,0.1,.7);

	// how long should my z-axis be? Scale with h?
	if (surface)
		z = surface[0] + GLv_data.slice_offset * (XPM_x+XPM_y)*0.5;
	else
		z = GLv_data.slice_offset * (XPM_x+XPM_y)*0.5;


	DrawOneLine(-XPM_x/2.0,-z/10.0, -XPM_y/2.0, -XPM_x/2.0,  z,-XPM_y/2.0);
	
	// Frame along x-y-Zero.
	DrawOneLine(-XPM_x/2.0,0.0, -XPM_y/2.0, (XPM_x/2.0),0.0,-(XPM_y/2.0));
	DrawOneLine(-XPM_x/2.0,0.0, -XPM_y/2.0, -(XPM_x/2.0),0.0, (XPM_y/2.0));

	DrawOneLine(+XPM_x/2.0,0.0, +XPM_y/2.0, (XPM_x/2.0),0.0,-(XPM_y/2.0));
	DrawOneLine(+XPM_x/2.0,0.0, +XPM_y/2.0, -(XPM_x/2.0),0.0, (XPM_y/2.0));

	if (GLv_data.TickFrameOptions[0] == '2' || GLv_data.TickFrameOptions[0] == '3'){
		DrawOneLine(XPM_x/2.0,0.0, -XPM_y/2.0, XPM_x/2.0,  z,-XPM_y/2.0);
		DrawOneLine(-XPM_x/2.0,0.0, XPM_y/2.0, -XPM_x/2.0,  z,XPM_y/2.0);
		DrawOneLine(XPM_x/2.0,0.0, XPM_y/2.0, XPM_x/2.0,  z,XPM_y/2.0);

		DrawOneLine(-XPM_x/2.0,z, -XPM_y/2.0, (XPM_x/2.0),z,-(XPM_y/2.0));
		DrawOneLine(-XPM_x/2.0,z, -XPM_y/2.0, -(XPM_x/2.0),z, (XPM_y/2.0));
		DrawOneLine(+XPM_x/2.0,z, +XPM_y/2.0, (XPM_x/2.0),z,-(XPM_y/2.0));
		DrawOneLine(+XPM_x/2.0,z, +XPM_y/2.0, -(XPM_x/2.0),z, (XPM_y/2.0));
	}
}

/* 
   Create Zero-pLane(s)
   Tesselate surface into list of triangles andcalculate normals for light processing
*/

void Surf3d::GLdrawsurface(int y_to_update, int refresh_all){
	int x,y,vi,idx,step,step2;
	int slice_pli[3];
#ifdef __ALTIVEC__
	float x0,y0;
	float v[4] __attribute__((aligned(16)));
	float n[4] __attribute__((aligned(16)));
	float4 v4[5], n4;
#else
	GLfloat v[5][3], n[3], tn[4][3];
#endif
	XSM_DEBUG (DBG_L2, "GL:::GLdrawsurface y_to_update=" << y_to_update);
	XSM_DEBUG_GP (DBG_L2, "GL:::GLdrawsurface y_to_update=%d\n", y_to_update);

	if (GLv_data.slice_plane_index[0] >= 0)
		slice_pli[0] = GLv_data.slice_plane_index[0]/QuenchFac;
	else
		slice_pli[0] = GLv_data.slice_plane_index[0];
	if (GLv_data.slice_plane_index[1] >= 0)
		slice_pli[1] = GLv_data.slice_plane_index[1]/QuenchFac;
	else
		slice_pli[1] = GLv_data.slice_plane_index[1];
	if (GLv_data.slice_plane_index[2] >= 0)
		slice_pli[2] = GLv_data.slice_plane_index[2];
	else
		slice_pli[2] = GLv_data.slice_plane_index[2];

        XSM_DEBUG_GP (DBG_L2, "GL:::GLdrawsurface -- ZP, stuff,...");
        
        if (GLv_data.ZeroPlane){
		if (glIsList (glZeroFrameList) && y_to_update == -1 && !refresh_all){
			XSM_DEBUG_GP (DBG_L4, "glCallList..." );
			glCallList (glZeroFrameList);
		}else{
			if (glIsList (glZeroFrameList))
				glDeleteLists (glZeroFrameList, 1);

			glZeroFrameList = glGenLists(1);
			glNewList (glZeroFrameList, GL_COMPILE_AND_EXECUTE);

			XSM_DEBUG_GP (DBG_L4, "NewGLList - Frame" );

			glBegin (GL_QUAD_STRIP);

			glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, GLv_data.box_mat_specular);
			glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, GLv_data.box_mat_shininess);
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, GLv_data.box_mat_diffuse);
			glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, GLv_data.box_mat_ambient);

			glNormal3f (-1.,0.,0.);
			for(x=y=1; y<XPM_y-1; y++){
				idx = x + y*XPM_x;
				
				v[0][0] = x-XPM_x/2;
				v[0][1] = surface[idx];
				v[0][2] = y-XPM_y/2;
				glVertex3fv (v[0]);
			
				v[0][1] = 0.;
				glVertex3fv (v[0]);
			}
			
			glNormal3f (0.,0.,1.);
			for(y=XPM_y-2,x=1; x<XPM_x-1; x++){
				idx = x + y*XPM_x;
				
				v[0][0] = x-XPM_x/2;
				v[0][1] = surface[idx];
				v[0][2] = y-XPM_y/2;
				glVertex3fv (v[0]);
				
				v[0][1] = 0.;
				glVertex3fv (v[0]);
			}
			glNormal3f (1.,0.,0.);
			for(x=XPM_x-2, y=XPM_y-2; y>1; y--){
				idx = x + y*XPM_x;
				
				v[0][0] = x-XPM_x/2;
				v[0][1] = surface[idx];
				v[0][2] = y-XPM_y/2;
				glVertex3fv (v[0]);
				
				v[0][1] = 0.;
				glVertex3fv (v[0]);
			}
			
			glNormal3f (0.,0.,-1.);
			for(y=1,x=XPM_x-2; x>1; x--){
				idx = x + y*XPM_x;
				
				v[0][0] = x-XPM_x/2;
				v[0][1] = surface[idx];
				v[0][2] = y-XPM_y/2;
				glVertex3fv (v[0]);
				
				v[0][1] = 0.;
				glVertex3fv (v[0]);
			}
			glEnd();
			
			glBegin(GL_QUADS);
			glNormal3f (0.,-1.,0.);
			glColor4f(0.1,0.7,1.0,0.4);
			glVertex3f(-(XPM_x)/2.0,0.,-(XPM_y)/2.0);
			glVertex3f(-(XPM_x)/2.0,0.,(XPM_y)/2.0);
			glVertex3f((XPM_x)/2.0,0.,(XPM_y)/2.0);
			glVertex3f((XPM_x)/2.0,0.,-(XPM_y)/2.0);
			glEnd();

			glEndList ();
		}
	}

// added glNormal() calculus
	if (!glSurfaceListRange)
		glSurfaceListRange = new GLuint[XPM_v];
	if (!glSurfaceList){
		glSurfaceList = new GLuint[XPM_v];
		for (vi=0; vi<XPM_v; ++vi)
			glSurfaceList[vi]=glSurfaceListRange[vi]=0;
	}

	if (!glVolumeXSListRange)
		glVolumeXSListRange = new GLuint[XPM_v-1];
	if (!glVolumeXSList){
		glVolumeXSList = new GLuint[XPM_v-1];
		for (vi=0; vi<XPM_v-1; ++vi)
			glVolumeXSList[vi]=glVolumeXSListRange[vi]=0;
	}

	if (!glVolumeYSListRange)
		glVolumeYSListRange = new GLuint[XPM_v-1];
	if (!glVolumeYSList){
		glVolumeYSList = new GLuint[XPM_v-1];
		for (vi=0; vi<XPM_v-1; ++vi)
			glVolumeYSList[vi]=glVolumeYSListRange[vi]=0;
	}

	if (!glSVolumeListRange)
		glSVolumeListRange = new GLuint[XPM_v-1];
	if (!glSVolumeList){
		glSVolumeList = new GLuint[XPM_v-1];
		for (vi=0; vi<XPM_v-1; ++vi)
			glSVolumeList[vi]=glSVolumeListRange[vi]=0;
	}

	if (surface){
                
                XSM_DEBUG_GP (DBG_L2, "GL:::GLdrawsurface -- surface");

                // *** as disabled
                glEnable (GL_COLOR_MATERIAL);
                glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                // ***
                
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, GLv_data.surf_mat_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, GLv_data.surf_mat_shininess);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GLv_data.surf_mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, GLv_data.surf_mat_ambient);

		if (GLv_data.TransparentSlices){
			glEnable (GL_BLEND); 
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable (GL_BLEND); 
		}

// use Shape File?
		int *x1, *x2;
		x1 = new int[XPM_y];
		x2 = new int[XPM_y];
		for (y=0; y<XPM_y; ++y) { x1[y]=1; x2[y]=XPM_x-1; }
		std::ifstream shape;
		shape.open ("shape.xx", std::ios::in);
		if (shape.good ()){
			for (y=0; y<XPM_y && shape.good(); ++y) { 
				shape >> x1[y] >> x2[y]; 
				x1[y]/=QuenchFac; x2[y]/=QuenchFac;
			}
		}

		// all layers
		step = (int)GLv_data.slice_start_n[2];
		step = step < 1? 1:step;
		step2 = (int)GLv_data.slice_start_n[3];
		step2 = step2 < 1? step : step2 > step? step:step2;

		tn[0][1] = 0.;

		for (vi=(int)(GLv_data.slice_start_n[1]>0? GLv_data.slice_start_n[0]:0.); 
		     vi<(int)(GLv_data.slice_start_n[1]>0? GLv_data.slice_start_n[0]+GLv_data.slice_start_n[1]*step:XPM_v)
			     && vi<XPM_v; vi+=step){

			if (GLv_data.slice_direction[0] == 'S'){
				// check generate and update gl-cache lists
				if (!(glIsList (glSVolumeList[vi]) && !refresh_all)){
					if (glIsList (glSVolumeList[vi]))
						glDeleteLists (glSVolumeList[vi], glSVolumeListRange[vi]);
					glSVolumeList[vi] = glGenLists (glSVolumeListRange[vi]=XPM_y-2);
				}

				// check if norms need to be calculated (only once)
				if (tn[0][1] == 0.){
					// Tetra: H1 0.2887, H2 0.5773, H 0.8660
					v[0][0] = - 0.5;
					v[0][1] = - 0.2887;
					v[0][2] = - 0.2887;
					
					v[1][0] = + 0.5;
					v[1][1] = v[0][1];
					v[1][2] = - 0.2887;
					
					v[2][0] = 0.;
					v[2][1] = v[0][1];
					v[2][2] = + 0.5773;
					
					v[3][0] = 0.;
					v[3][1] = v[0][1]      + 0.5773;
					v[3][2] = 0.;
					
					// trivial norm of bottom triangle
					tn[0][0] = 0.;
					tn[0][1] = -1.;
					tn[0][2] = 0.;
					
					norm3pkte (v[1], v[2], v[3], tn[1]);
					norm3pkte (v[2], v[3], v[0], tn[2]);
					norm3pkte (v[3], v[0], v[1], tn[3]);
				}
				// generate volume scatter objects (tetras)
				for (y=1; y<XPM_y-1; y++){
					if (x1[y] < 1 || x2[y] >= XPM_x) continue;

					if (glIsList (glSVolumeList[vi]) && y_to_update != y && !refresh_all)
						glCallList (glSVolumeList[vi]+y-1);
					else{
						if (glIsList (glSVolumeList[vi]+y-1))
							glDeleteLists (glSVolumeList[vi]+y-1, 1);

						glNewList (glSurfaceList[vi]+y-1, GL_COMPILE_AND_EXECUTE);

						// make surface strip
						for (x=x1[y]; x<x2[y]; x++){

							idx = x + y*XPM_x + vi*XPM_x*XPM_y;
					
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}
							glBegin (GL_TRIANGLE_STRIP);
							
							// Tetra: H1 0.2887, H2 0.5773, H 0.8660
							v[0][0] = x-XPM_x/2    - GLv_data.tskl*0.5;
							v[0][1] = surface[idx] - GLv_data.tskl*0.2887;
							v[0][2] = y-XPM_y/2    - GLv_data.tskl*0.2887;

							v[1][0] = x-XPM_x/2    + GLv_data.tskl*0.5;
							v[1][1] = v[0][1];
							v[1][2] = y-XPM_y/2    - GLv_data.tskl*0.2887;

							v[2][0] = x-XPM_x/2;
							v[2][1] = v[0][1];
							v[2][2] = y-XPM_y/2    + GLv_data.tskl*0.5773;

							v[3][0] = x-XPM_x/2;
							v[3][1] = v[0][1]      + GLv_data.tskl*0.5773;
							v[3][2] = y-XPM_y/2;

							glNormal3fv (tn[0]);
							glVertex3fv (v[0]);
							glVertex3fv (v[1]);
							glVertex3fv (v[2]);

							glNormal3fv (tn[1]);
							glVertex3fv (v[3]);

							glNormal3fv (tn[2]);
							glVertex3fv (v[0]);

							glNormal3fv (tn[3]);
							glVertex3fv (v[1]);
							
							glEnd ();
						}

						glEndList ();
					}
				}
			} else
			if (GLv_data.slice_direction[0] == 'Z' || GLv_data.slice_direction[0] == 'V' 
			    || ( slice_pli[2] >= 0 && slice_pli[2] >= vi && slice_pli[2] < vi+step)
				){
				
				// check generate and update gl-cache lists
				if (!(glIsList (glSurfaceList[vi]) && !refresh_all)){
					if (glIsList (glSurfaceList[vi]))
						glDeleteLists (glSurfaceList[vi], glSurfaceListRange[vi]);
					glSurfaceList[vi] = glGenLists (glSurfaceListRange[vi]=XPM_y-2);
				}
				
				// generate surface strips
				for (y=1; y<XPM_y-1; y++){
					if (x1[y] < 1 || x2[y] >= XPM_x) continue;

					if (glIsList (glSurfaceList[vi]) && y_to_update != y && !refresh_all)
						glCallList (glSurfaceList[vi]+y-1);
					else{
						if (glIsList (glSurfaceList[vi]+y-1))
							glDeleteLists (glSurfaceList[vi]+y-1, 1);

						glNewList (glSurfaceList[vi]+y-1, GL_COMPILE_AND_EXECUTE);

						glBegin (GL_QUAD_STRIP);

						// make surface strip
						for (x=x1[y]; x<x2[y]; x++){
				
							idx = x + y*XPM_x + vi*XPM_x*XPM_y;
					
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}
#ifdef __ALTIVEC__
							v[0] = x0 = x-XPM_x/2;
							v[1] = surface[idx];
							v[2] = y0 = y-XPM_y/2;
							v4[0] = vec_load (v);
					
							v[0] = x0+1;
							v[1] = surface[idx+1];
							v4[1] = vec_load (v);
					
							v[0] = x0;
							v[1] = surface[idx-XPM_x];
							v[2] = y0-1;
							v4[2] = vec_load (v);
					
							v[0] = x0-1;
							v[1] = surface[idx-1];
							v[2] = y0;
							v4[3] = vec_load (v);
					
							v[0] = x0;
							v[1] = surface[idx+XPM_x];
							v[2] = y0+1;
							v4[4] = vec_load (v);
					
							avgpolynorm (v4, n4);
							vec_store (n, n4);
							vec_store (v, v[0]);
					
							glNormal3fv (n);
							glVertex3fv (v);
#else
							v[0][0] = x-XPM_x/2;
							v[0][1] = surface[idx];
							v[0][2] = y-XPM_y/2;
					
							v[1][0] = v[0][0]+1;
							v[1][1] = surface[idx+1];
							v[1][2] = v[0][2];
					
							v[2][0] = v[0][0];
							v[2][1] = surface[idx-XPM_x];
							v[2][2] = v[0][2]-1;
					
							v[3][0] = v[0][0]-1;
							v[3][1] = surface[idx-1];
							v[3][2] = v[0][2];
					
							v[4][0] = v[0][0];
							v[4][1] = surface[idx+XPM_x];
							v[4][2] = v[0][2]+1;
					
							avgpolynorm (v, n);
					
							glNormal3fv (n);
							glVertex3fv (v[0]);

                                                        // xxxxxxxxxxxxxxxxx
                                                        // g_print ("v=[");
                                                        // for (int ii=0; ii<=4; ++ii) {
                                                        //         g_print ("[");
                                                        //         for (int jj=0; jj<3; ++jj)
                                                        //                 g_print ("%g, ", v[ii][jj]);
                                                        //         g_print ("],");
                                                        // }
                                                        // g_print ("]\n");
                                                        // xxxxxxxxxxxxxxxxx
#endif
					
							// next vertex y+1
							idx += XPM_x;
					
							// glTexCoord2f (x,y+1);
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}
#ifdef __ALTIVEC__
							v[0] = x0 = x-XPM_x/2;
							v[1] = surface[idx];
							v[2] = y0 = y-XPM_y/2;
							v4[0] = vec_load (v);
					
							v[0] = x0+1;
							v[1] = surface[idx+1];
							v4[1] = vec_load (v);
					
							v[0] = x0;
							v[1] = surface[idx-XPM_x];
							v[2] = y0-1;
							v4[2] = vec_load (v);
					
							v[0] = x0-1;
							v[1] = surface[idx-1];
							v[2] = y0;
							v4[3] = vec_load (v);
					
							v[0] = x0;
							v[1] = surface[idx+XPM_x];
							v[2] = y0+1;
							v4[4] = vec_load (v);
					
							avgpolynorm (v4, n4);
							vec_store (n, n4);
							vec_store (v, v[0]);
					
							glNormal3fv (n);
							glVertex3fv (v);
#else
							v[0][1] = surface[idx];
							v[0][2] = y-XPM_y/2+1;
					
							v[1][1] = surface[idx+1];
							v[1][2] = v[0][2];
					
							v[2][1] = surface[idx-XPM_x];
							v[2][2] = v[0][2]-1;
					
							v[3][1] = surface[idx-1];
							v[3][2] = v[0][2];
					
							v[4][1] = surface[idx+XPM_x];
							v[4][2] = v[0][2]+1;
					
							avgpolynorm (v, n);
					
							glNormal3fv (n);
							glVertex3fv (v[0]);

                                                        // xxxxxxxxxxxxxxxxx
                                                        // g_print ("v=[");
                                                        // for (int ii=0; ii<=4; ++ii) {
                                                        //         g_print ("[");
                                                        //         for (int jj=0; jj<3; ++jj)
                                                        //                 g_print ("%g, ", v[ii][jj]);
                                                        //         g_print ("],");
                                                        // }
                                                        // g_print ("]\n");
                                                        // xxxxxxxxxxxxxxxxx
#endif
						}
						glEnd ();

						glEndList ();
					}
				} // y loop
			}

			// add Volume "close-up" strips
			if (((GLv_data.slice_direction[0] == 'Y' || GLv_data.slice_direction[0] == 'V' || slice_pli[1] >= 0) && vi < XPM_v-step2)) {

				// generate/recall volume X-strips for all Y

				if (!(glIsList (glVolumeXSList[vi]) && !refresh_all)){
					if (glIsList (glVolumeXSList[vi]))
						glDeleteLists (glVolumeXSList[vi], glVolumeXSListRange[vi]);
					glVolumeXSList[vi] = glGenLists (glVolumeXSListRange[vi]=XPM_y-1);
				}

//				for (y=1; y<XPM_y; y++){
				for (y=(int)(GLv_data.slice_start_n[1]>0? GLv_data.slice_start_n[0]+1:1); 
				     y<(int)(GLv_data.slice_start_n[1]>0? GLv_data.slice_start_n[0]+1+GLv_data.slice_start_n[1]*step:XPM_y)
					     && y<XPM_y; y+=step){

					if ( slice_pli[1] >= 0 && (slice_pli[1] < y || slice_pli[1] > y))
					     continue;
					
					if (glIsList (glVolumeXSList[vi]) && y_to_update != y && !refresh_all)
						glCallList (glVolumeXSList[vi]+y-1);
					else{
						if (glIsList (glVolumeXSList[vi]+y-1))
							glDeleteLists (glVolumeXSList[vi]+y-1, 1);

						glNewList (glVolumeXSList[vi]+y-1, GL_COMPILE_AND_EXECUTE);

						glBegin (GL_QUAD_STRIP);

						// make volume X-strip
						for (x=1; x<XPM_x; x++){
				
							idx    = x + y*XPM_x + vi*XPM_x*XPM_y;
					
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}

							v[0][0] = x-XPM_x/2;
							v[0][1] = surface[idx];
							v[0][2] = y-XPM_y/2;
							
							n[0] = 0.;
							n[1] = 0.;
							n[2] = 1.;
					
							glNormal3fv (n);
							glVertex3fv (v[0]);
					
							// next vertex "z+1"
							idx += XPM_x*XPM_y*step2;
					
							// glTexCoord2f (x,y+1);
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}

							v[0][1] = surface[idx];
							n[2] = 1.;
							
							glNormal3fv (n);
							glVertex3fv (v[0]);
						}
						glEnd ();

						glEndList ();
					} // end y-strip generation: x loop
				} // y loop
			}

			if (((GLv_data.slice_direction[0] == 'X' || GLv_data.slice_direction[0] == 'V' || slice_pli[0] >= 0) && vi < XPM_v-step2)) {
				// generate/recall volume Y-strips for all X

				if (!(glIsList (glVolumeYSList[vi]) && !refresh_all)){
					if (glIsList (glVolumeYSList[vi]))
						glDeleteLists (glVolumeYSList[vi], glVolumeYSListRange[vi]);
					glVolumeYSList[vi] = glGenLists (glVolumeYSListRange[vi]=XPM_x-1);
				}

//				for (x=1; x<XPM_x; x++){
				for (x=(int)(GLv_data.slice_start_n[1]>0? GLv_data.slice_start_n[0]+1:1); 
				     x<(int)(GLv_data.slice_start_n[1]>0? GLv_data.slice_start_n[0]+1+GLv_data.slice_start_n[1]*step:XPM_y)
					     && x<XPM_x; x+=step){
						
					if ( slice_pli[0] >= 0 && (slice_pli[0] < x || slice_pli[0] > x))
					     continue;

					if (glIsList (glVolumeYSList[vi]) && !refresh_all)
						glCallList (glVolumeYSList[vi]+x-1);
					else{
						if (glIsList (glVolumeYSList[vi]+x-1))
							glDeleteLists (glVolumeYSList[vi]+x-1, 1);

						glNewList (glVolumeYSList[vi]+x-1, GL_COMPILE_AND_EXECUTE);
							
						glBegin (GL_QUAD_STRIP);
							
						// make volume Y-strip
						for (y=1; y<XPM_y; y++){
				
							idx    = x + y*XPM_x + vi*XPM_x*XPM_y;
					
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}
							v[0][0] = x-XPM_x/2;
							v[0][1] = surface[idx];
							v[0][2] = y-XPM_y/2;
							
							n[0] = 1;
							n[1] = 0.;
							n[2] = 0.;
					
							glNormal3fv (n);
							glVertex3fv (v[0]);
					
							// next vertex "z+1"
							idx += XPM_x*XPM_y*step2;
					
							// glTexCoord2f (x,y+1);
							if (GLv_data.ColorSrc[0] != 'U'){
								if  (GLv_data.Emission)
									glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, surfacecolor[idx]);
								else
									glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surfacecolor[idx]);
							}

							v[0][1] = surface[idx];
							n[0] = 1.;
							
							glNormal3fv (n);
							glVertex3fv (v[0]);
						}
						glEnd ();
						
						glEndList ();
					} // end x-strip generaytioon: y loop
				} // end x loop
			} // end if volume

		} // end loop overy layers: vi

		delete [] x2;
		delete [] x1;
	}
        XSM_DEBUG_GP (DBG_L2, "GL:::GLdrawsurface -- retesseration done.");
}



/* ::GLdrawscene
 * ======================================================================
 * (re)draw all: create/recall scene
 * - adjust scene settings
 * - create transform/rotate matrix
 * - draw surface
 * - draw extra stuff
 * - swap/flush buffer(s) to visual
 */
gboolean Surf3d::GLdrawscene(GdkGLContext *glcontext, int y_to_update, int refresh_all){
	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene (context)\n");

	if(!valid) return FALSE;
	if(!v3dcontrol) return FALSE;

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene set modes\n");

	if(GLv_data.Mesh)
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glClearColor (GLv_data.clear_color[0], 
		      GLv_data.clear_color[1],  
		      GLv_data.clear_color[2],
		      GLv_data.clear_color[3]);

        // WTF -- can't use any of that stuff any more since GL3.2
        // =======================================================
        // no ...f ()
        // no glBegin/End () ...
        // ==>
        // https://www.bassi.io/articles/2015/02/17/using-opengl-with-gtk/
        // http://gamedev.stackexchange.com/questions/34108/opengl-vbo-or-glbegin-glend
        // https://bcmpinc.wordpress.com/2015/08/15/starting-with-opengl-4-5/
	if (GLv_data.Cull)
		glEnable (GL_CULL_FACE);
	else
		glDisable (GL_CULL_FACE);

	if (GLv_data.Smooth)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	GLfloat light_position[4];
	GLenum light_no[3] = { GL_LIGHT0, GL_LIGHT1, GL_LIGHT2 };
	for (int i=0; i<3; ++i){
		if ( GLv_data.light[i][1] == 'n' ){ // "On" / "Off" ?
			copyvec4  (light_position, GLv_data.light_position[i]);
			mulvecwf  (light_position, (GLfloat)XPM_x);
			glLightfv (light_no[i], GL_POSITION, light_position);
			glLightfv (light_no[i], GL_AMBIENT,  GLv_data.light_ambient[i]);
			glLightfv (light_no[i], GL_SPECULAR, GLv_data.light_specular[i]);
			glLightfv (light_no[i], GL_DIFFUSE,  GLv_data.light_diffuse[i]);
			glEnable  (light_no[i]);
		}
		else
			glDisable (light_no[i]);
	}
	glEnable (GL_LIGHTING);
	glEnable (GL_DEPTH_TEST);

/*	Specifies a lighting model parameter.
                    GL_LIGHT_MODEL_AMBIENT,
                    GL_LIGHT_MODEL_COLOR_CONTROL,
                    GL_LIGHT_MODEL_LOCAL_VIEWER, and
                    GL_LIGHT_MODEL_TWO_SIDE are accepted.
*/              	
	glLightModelf (GL_LIGHT_MODEL_TWO_SIDE, 1);	

	if (GLv_data.Texture)
		glEnable (GL_TEXTURE_2D);
	else
		glDisable (GL_TEXTURE_2D);
		
	if (GLv_data.Fog){
		glEnable (GL_FOG);
		glFogi (GL_FOG_MODE,GL_EXP2);
		glFogfv (GL_FOG_COLOR, GLv_data.fog_color);
		glFogf (GL_FOG_DENSITY, GLv_data.fog_density/XPM_x);
	}
	else
		glDisable (GL_FOG);
		
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glPushMatrix ();

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene set camera");

        
	float x=XPM_x*GLv_data.dist;

        look_at_GL (0., -x, 0.,  0., x+1., 0.,   0.0,0.0,1.0);
		
	int ox,oy,oz;
	ox = (int)(GLv_data.trans[0]*XPM_x/2./100.);
	oy = (int)(GLv_data.trans[1]*XPM_y/2./100.);
	oz = (int)(GLv_data.trans[2]*(XPM_x+XPM_y)/4./100.);
		
	// setup scene in space
	
	glPushMatrix ();
	glTranslatef ((float)ox,(float)oz,(float)oy);
	
	glRotatef (GLv_data.rot[0], 1.0, 0.0, 0.0);
	glRotatef (GLv_data.rot[1], 0.0, 1.0, 0.0);
	glRotatef (GLv_data.rot[2], 0.0, 0.0, 1.0);


	// valid == 1 :  GLlists and data valid
	// valid == 2 :  GLliste need to refreshed, data is valid, but changed!

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene draw now...\n");

        // ************** OPENGL 4.5 TEST
        // An array of 3 vectors which represents 3 vertices
        static const GLfloat g_vertex_buffer_data[] = {
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                0.0f,  1.0f, 0.0f,
        };
#if 1  
        // https://open.gl/context
        glewExperimental = GL_TRUE;
        glewInit();
        //Make sure that you've set up your project correctly by calling the glGenBuffers function, which was loaded by GLEW for you!

#endif
	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** VB\n");

#if 1
        // This will identify our vertex buffer
        GLuint vertexbuffer;
        // Generate 1 buffer, put the resulting identifier in vertexbuffer
	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** VB1\n");
        glGenBuffers(1, &vertexbuffer);
        g_print ("%u\n", vertexbuffer);

        // The following commands will talk about our 'vertexbuffer' buffer
	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** VB2\n");
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

        // Give our vertices to OpenGL.
	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** VB3\n");
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** VA\n");

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** DRAW\n");

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDisableVertexAttribArray(0);
#endif
	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene ** done **\n");

        
        // *******************************
        
	GLdrawsurface (y_to_update, (glSurfaceListRange?(glSurfaceListRange[0]==0):0) || valid == 2 ? TRUE:refresh_all);

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene draw gimmicks...");

	if (GLv_data.Ticks)
		GLdrawGimmicks ();

	// data and lists are valid now!
	valid = 1;

	glPopMatrix ();
	glPopMatrix ();

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::GLdrawscene done.");

	return TRUE;
}


void realize_vsurf3d_event_cb (GtkGLArea *area){
	XSM_DEBUG_GP (DBG_L2, "GL:::REALIZE-EVENT");
        // We need to make the context current if we want to
        // call GL API
        gtk_gl_area_make_current (area);

        // If there were errors during the initialization or
        // when trying to make the context current, this
        // function will return a #GError for you to catch
        if (gtk_gl_area_get_error (area) != NULL)
                return;
#if 0
        // You can also use gtk_gl_area_set_error() in order
        // to show eventual initialization errors on the
        // GtkGLArea widget itself
        GError *internal_error = NULL;
        init_buffer_objects (&internal_error);
        if (internal_error != NULL)
                {
                        gtk_gl_area_set_error (area, internal_error);
                        g_error_free (error);
                        return;
                }

        init_shaders (&internal_error);
        if (internal_error != NULL)
                {
                        gtk_gl_area_set_error (area, internal_error);
                        g_error_free (internal_error);
                        return;
                }
#endif
}


// =========== GTK-GL-AREA CALLBACKS =================
// The “resize” signal handler
void resize_vsurf3d_event_cb (GtkGLArea *area,
                           gint       width,
                           gint       height,
                           Surf3d *s)
{
	XSM_DEBUG_GP (DBG_L2, "GL:::RESIZE-EVENT");

        if (s){
                if (!s->is_ready())
                        return;
        } else
                return;

        glViewport (0,0, s->scrwidth=width, s->scrheight=height);
	
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();

        perspective_GL (s->GLv_data.fov, /* field of view in degree */
                        ((GLfloat)s->scrwidth/(GLfloat)s->scrheight), /* aspect ratio */
                        s->GLv_data.Znear, /* Z near */
                        s->XPM_x * s->GLv_data.Zfar); /* Z far */

        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        s->GetSmem ();
}

// render_event :    redraw scene

static gboolean
render_vsurf3d_event_cb (GtkGLArea *area, GdkGLContext *context, Surf3d *s)
{
        XSM_DEBUG_GP (DBG_L2, "GL:::RENDER-EVENT");

        //glClearColor (0, 0, 0, 0);
        //glClear (GL_COLOR_BUFFER_BIT);
        // draw your object
        // ...
        //return TRUE;
        
        if (s){
                if (!s->is_ready())
                        return FALSE;
        } else
                return FALSE;

        XSM_DEBUG_GP (DBG_L2, "GL:::RENDER-EVENT -- OK, call GLdrawscene (context)");

	return s->GLdrawscene (context);
}


int Surf3d::draw(int zoomoverride){

	XSM_DEBUG_GP (DBG_L2, "SURF3D:::DRAW");

	if (!scan->mem2d) { 
		XSM_DEBUG (DBG_L2, "Surf3d: no mem2d !"); 
		return 1; 
	}
	
	if ( !v3dcontrol )
		v3dcontrol = new V3dControl ("3D GL-View", ChanNo, scan,
					     G_CALLBACK (resize_vsurf3d_event_cb),
					     G_CALLBACK (render_vsurf3d_event_cb),
                                             NULL, // ?? not need ?? need ?? //	     G_CALLBACK (realize_vsurf3d_event_cb),
					     self);
        //	if (scan && self)
        //	        GLupdate (self);
        
	return 0;
}

void Surf3d::preferences(){
	XSM_DEBUG (DBG_L2, "SURF3D:::PREFERENCES");

	if (v3dControl_pref_dlg)
		gnome_res_run_change_user_config (v3dControl_pref_dlg, "GL Scene Setup");
}
