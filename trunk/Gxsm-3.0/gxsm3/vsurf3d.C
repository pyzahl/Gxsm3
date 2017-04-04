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

/*
  https://github.com/NVIDIAGameWorks/GraphicsSamples/blob/master/samples/es3aep-kepler/TerrainTessellation/TerrainTessellation.cpp/
*/

#include <locale.h>
#include <libintl.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "config.h"
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

// ------------------------------------------------------------
// glm, gli, GL support includes
// ------------------------------------------------------------
#include "ogl_framework.hpp"

#define GL_DEBUG_L2 0
#define GL_DEBUG_L3 1

// ------------------------------------------------------------
// gschema creator for from internal recources
// ------------------------------------------------------------
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

#define __GXSM_PY_DEVEL
#ifdef __GXSM_PY_DEVEL
//#define GLSL_DEV_DIR "/home/pzahl/SVN/Gxsm-3.0/gl-400/"
#define GLSL_DEV_DIR "/home/percy/SVN/Gxsm-3.0/gl-400/"
#endif


// ------------------------------------------------------------
// glsl data and code locations
// ------------------------------------------------------------
std::string getDataDirectory()
{
#ifdef __GXSM_PY_DEVEL
        return std::string(GLSL_DEV_DIR);
#else
	return std::string(PACKAGE_GL400_DIR) + "/";
#endif
}

std::string getBinaryDirectory()
{
#ifdef __GXSM_PY_DEVEL
        return std::string(GLSL_DEV_DIR);
#else
	return std::string(PACKAGE_GL400_DIR) + "/";
#endif
}

// ------------------------------------------------------------
// GL 4.0 required -- GL 3D support with GPU tesselation
// ------------------------------------------------------------
namespace
{
	std::string const SAMPLE_VERTEX_SHADER("tess-vertex.glsl");
	std::string const SAMPLE_CONTROL_SHADER("tess-control.glsl");
	std::string const SAMPLE_EVALUATION_SHADER("tess-evaluation.glsl");
	std::string const SAMPLE_GEOMETRY_SHADER("tess-geometry.glsl");
	std::string const SAMPLE_FRAGMENT_SHADER("tess-fragment.glsl");

	GLuint ProgramName(0);

	GLint Uniform_screen_size(0); // vec2
	GLint Uniform_tess_level(0); // float
	GLint Uniform_lod_factor(0); // float
        GLint Uniform_height_scale(0);
        GLint Uniform_height_offset(0);
        GLint Uniform_aspect(0);
        GLint Uniform_color_source(0);
        GLsizei const TextureCount(2);
	GLuint TextureName[2];
        
        // Model View and Projection
	GLint Uniform_ModelViewProjection(0); // mat4
	GLint Uniform_ModelView(0); // mat4

        // Lightening
        GLint Uniform_lightDirWorld(0); // vec3
        GLint Uniform_eyePosWorld(0);   // vec3

        GLint Uniform_sunColor(0); // = vec3(1.0, 1.0, 0.7);
        GLint Uniform_specularColor(0); // = vec3(1.0, 1.0, 0.7)*1.5;
        GLint Uniform_ambientColor(0); // = vec3(1.0, 1.0, 0.7)*1.5;
        GLint Uniform_diffuseColor(0); // = vec3(1.0, 1.0, 0.7)*1.5;
        GLint Uniform_fogColor(0); // = vec3(0.7, 0.8, 1.0)*0.7;

        GLint Uniform_fogExp(0); // = 0.1;

        GLint Uniform_shininess(0); // = 100.0;
        GLint Uniform_lightness(0); // = 100.0;
        GLint Uniform_color_offset(0); // = 100.0;
	GLint Uniform_delta(0); // vec2 float -- for normal computation, should be 1/NX, 1/NY
        GLint Uniform_wrap(0); // = 0.3;

        GLuint Uniform_vertexDirect(0); // vertex Function references
        GLuint Uniform_vertexSurface(0);
        GLuint Uniform_vertexHScaled(0);

        GLuint Uniform_evaluationDirect(0); // evaluation Function references
        GLuint Uniform_evaluationSurface(0);
        GLuint Uniform_evaluationHScaled(0);

        GLuint Uniform_shadeTerrain(0); // shader Function references
        GLuint Uniform_shadeDebugMode(0);
        GLuint Uniform_shadeLambertian(0);
} //namespace

class base_plane{
public:
        base_plane (Mem2d *m=NULL, int w=128, double aspect=1.0, GLfloat *box_color=NULL){
                Validated = true;
                BaseGridW = w;
                BaseGridH = w; // adjusted by make_plane_vbo using aspect
                ArrayBufferName = 0;
                VertexArrayName = 0;

                make_plane_vbo (m, aspect, box_color);

                Validated = init_buffer ();

                Validated = init_vao ();
        };
        ~base_plane (){
		if (Validated){
                        g_free (indices);
                        g_free (vertex);
                        glDeleteVertexArrays(1, &VertexArrayName);
                        glDeleteBuffers(1, &IndexBufferName);
                        checkError("make_plane::~delete");
                }
        };
        gboolean init_buffer (){
                checkError("make_plane:: init_buffer");
		if (!Validated) return false;

                glGenBuffers(1, &ArrayBufferName);
                glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                glBufferData(GL_ARRAY_BUFFER, VertexObjectSize, vertex, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glGenBuffers(1, &IndexBufferName);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesObjectSize, indices, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                return Validated && checkError("make_plane:: init_buffer");
        };
        gboolean init_vao (){
                g_message ("base_plane init_vao");
                checkError("make_plane::init_vao");
		if (!Validated) return false;

                // Build a vertex array object
                glGenVertexArrays(1, &VertexArrayName);
                glBindVertexArray(VertexArrayName);

                glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                glVertexAttribPointer(semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v3fn3fc4f), BUFFER_OFFSET(0));
                glVertexAttribPointer(semantic::attr::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v3fn3fc4f), BUFFER_OFFSET(sizeof(glm::vec3)));
                glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v3fn3fc4f), BUFFER_OFFSET(2*sizeof(glm::vec3)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glEnableVertexAttribArray(semantic::attr::POSITION);
                glEnableVertexAttribArray(semantic::attr::NORMAL);
                glEnableVertexAttribArray(semantic::attr::COLOR);
                glBindVertexArray(0);

                g_message ("base_plane init_vao end");

                return Validated && checkError("make_plane::init_vao");
        };
        gboolean draw (gboolean draw_box = false){
		if (!Validated) return false;
                
                glBindVertexArray(VertexArrayName);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);
                glPatchParameteri(GL_PATCH_VERTICES, 4);

                if (draw_box)
                        glDrawElements(GL_PATCHES, IndicesCount, GL_UNSIGNED_INT, 0);
                else
                        glDrawElements(GL_PATCHES, IndicesCountSurface, GL_UNSIGNED_INT, 0); // surface only

                return Validated && checkError("make_plane::draw");
        };
        
        // make plane with box
        void make_plane_vbo (Mem2d *mob=NULL, double aspect=1.0, GLfloat *box_color=NULL){
                gboolean option = box_color != NULL;
                
                // Surface Object Vertices
                BaseGridH = (GLuint)round((double)BaseGridW * aspect);
                
                g_message ("mkplane -- vtx surf -- %s", option? "with box":"-/-");
                VertexCount =  BaseGridW*BaseGridH + (option? 2*BaseGridW + 2*BaseGridH : 0);
                VertexObjectSize = VertexCount * sizeof(glf::vertex_v3fn3fc4f);

               
                g_message ("mkplane w=%d h=%d  nvertices=%d", BaseGridW, BaseGridH, VertexCount);
                vertex = g_new (glf::vertex_v3fn3fc4f, VertexCount);
                double s_factor = 1.0/(BaseGridW-1);
                double t_factor = 1.0/(BaseGridH-1);
                int offset;
                // surface vertices
                for (guint y=0; y<BaseGridH; ++y){
                        gboolean edgeflag_y = y == 0 ? true : y == (BaseGridH-1)? true : false;
                        for (guint x=0; x<BaseGridW; ++x){
                                offset = y*BaseGridW+x;
                                gboolean edgeflag_x = x == 0 ? true : x == (BaseGridW-1)? true : false;
                                glm::vec4 normal_z (edgeflag_x ? x==0 ? -1.0f:1.0f : 0.0f,
                                                    edgeflag_x || edgeflag_y ? 0.0f : 2.0f,
                                                    edgeflag_y ? y==0 ? -1.0f:1.0f : 0.0f,
                                                    0.0f);
                                double xd = x * s_factor;
                                double yd = y * t_factor;
                                double vd = 0.; // vi
                                GLfloat z=0.;
#if 1
                                if (mob)
                                        z = (GLfloat)((mob->GetDataPkt((int)round(xd*(mob->GetNx()-1)), (int)round(yd*(mob->GetNy()-1)), vd) - mob->data->zmin) / mob->data->zrange);
                                vertex[offset].Position = glm::vec3 (-0.5+xd, 101., -(0.5+yd)*aspect); // > y=100 => vertex z is taken from "terrain" height field 
                                vertex[offset].Normals  = glm::vec3 (normal_z.x, normal_z.y, normal_z.z);
                                vertex[offset].Color    = glm::vec4 (z, z, z, -1.0);
#else
                                if (mob)
                                        mob->GetDataPkt_vec_normal_4F (xd*(mob->GetNx()-1), yd*(mob->GetNy()-1), vd, &normal_z, 1.); //GLv_data.hskl);
                                vertex[offset].Position = glm::vec3 (-0.5+xd, normal_z.w, -(0.5+yd)*aspect);
                                vertex[offset].Normals  = glm::vec3 (normal_z.x, normal_z.y, normal_z.z);
                                vertex[offset].Color    = glm::vec4 (normal_z.w, normal_z.w, normal_z.w, -1.0);
#endif
                                //g_message ("mkplane vtx -- x=%d y=TEX z=%d  [%d]",x,y, offset);
                        }
                }

                if (option){
                        // box bottom z=0 vertices
                        g_message ("mkplane -- vtx box");
                        offset = BaseGridW*BaseGridH;
                        for (int s=0; s<4; ++s){
                                int num = s%2 ? BaseGridW : BaseGridH;
                                //g_message ("mkplane -- vtx s=%d",s);
                                glm::vec3 normal(s%2 ? 0.: s==1 ? 1.:-1.,
                                                 s%2 ? s==0 ? -1.:1. : 0.,
                                                 0.);
                                for (int l=0; l<num; ++l){
                                        int x = s%2==0 ? l : s==1 ? BaseGridW-1:0;
                                        int y = s%2==0 ? s==0 ? 0:BaseGridH-1 : l;
                                        //g_message ("mkplane box [%d] vtx -- x=%d y=-1 z=%d  [%d]",s, x,y, offset);
                                        double xd = x * s_factor;
                                        double yd = y * t_factor;
                                        vertex[offset].Position = glm::vec3 (-0.5+xd, -1.0, -(0.5+yd)*aspect); // box z=0 base
                                        vertex[offset].Normals  = normal; // fix corner normals!
                                        vertex[offset].Color    = glm::vec4(box_color[0], box_color[1], box_color[2], box_color[3]);
                                        ++offset;
                                }
                        }
                }
                g_message ("mkplane -- Vertex Count=%d", offset);
        
                // Patch Indices
                int i_width = BaseGridW-1;
                int i_height = BaseGridH-1;
                IndicesCount = 4*(i_width*i_height + (option ? i_width*2 + i_height*2 + 1 : 0));
                IndicesCountSurface = (((BaseGridW-1)*(BaseGridH-1))*4); // surface ony
                IndicesObjectSize = IndicesCount * sizeof(glf::vertex_v1i);

                indices = g_new (glf::vertex_v1i, IndicesCount);
                // surface patches
                int ii=0;
                for (int y=0; y<i_height; ++y){
                        for (int x=0; x<i_width; ++x){
                                int p1 = x+y*BaseGridW;
                                int p2 = p1+BaseGridW;
                                int p4 = p1+1;
                                int p3 = p2+1;
                                indices[ii++].indices.x = p1;
                                indices[ii++].indices.x = p2;
                                indices[ii++].indices.x = p3;
                                indices[ii++].indices.x = p4;
                        }
                }

                if (option){
                        g_message ("mkplane -- idx box");
                        // box patches
                        int pb0=BaseGridW*BaseGridH;
                        for (int s=0; s<4; ++s){
                                g_message ("mkplane -- idx s=%d",s);
                                int num = s%2 ? i_width : i_height;
                                for (int l=0; l<num; ++l){
                                        int x = s%2==0 ? l : s==1 ? BaseGridW-1:0;
                                        int y = s%2==0 ? s==0 ? 0:BaseGridH-1 : l;
                                        int p1 = x+y*BaseGridW; // ok for ..x..
                                        int p2 = pb0+l; //ok
                                        int p3 = p2+1;
                                        int p4 = p1+(s%2==0 ? 1:BaseGridW);
                                        indices[ii++].indices.x = p1;
                                        indices[ii++].indices.x = p2;
                                        indices[ii++].indices.x = p3;
                                        indices[ii++].indices.x = p4;
#if 0
                                        g_message ("mkplane idx [%d] -- x=%d y=%d  p1..4= %d %d %d %d",ii, x,y, p1,p2,p3,p4);
                                        g_message ("  vertex[p1].Position = (%g %g %g)", vertex[p1].Position.x, vertex[p1].Position.y, vertex[p1].Position.z);
                                        p1=p2;
                                        g_message ("  vertex[p2].Position = (%g %g %g)", vertex[p1].Position.x, vertex[p1].Position.y, vertex[p1].Position.z);
                                        p1=p3;
                                        g_message ("  vertex[p3].Position = (%g %g %g)", vertex[p1].Position.x, vertex[p1].Position.y, vertex[p1].Position.z);
                                        p1=p4;
                                        g_message ("  vertex[p4].Position = (%g %g %g)", vertex[p1].Position.x, vertex[p1].Position.y, vertex[p1].Position.z);
#endif
                                }
                                pb0+=num+1;
                        }
                        g_message ("mkplane -- idx floor");
                        // box floor
                        int pbf=BaseGridW*BaseGridH;
                        indices[ii++].indices.x = pbf; pbf+=BaseGridW;
                        indices[ii++].indices.x = pbf; pbf+=BaseGridH;
                        indices[ii++].indices.x = pbf; pbf+=BaseGridW;
                        indices[ii++].indices.x = pbf;
#if 0
                        g_message ("mkplane floor idx [%d] -- p1..4: %d %d %d %d", ii, indices[ii-4].indices.x, indices[ii-3].indices.x, indices[ii-2].indices.x, indices[ii-1].indices.x);
                        g_message ("  vertex[p1].Position = (%g %g %g)", vertex[indices[ii-4].indices.x].Position.x, vertex[indices[ii-4].indices.x].Position.y, vertex[indices[ii-4].indices.x].Position.z);
                        g_message ("  vertex[p2].Position = (%g %g %g)", vertex[indices[ii-3].indices.x].Position.x, vertex[indices[ii-3].indices.x].Position.y, vertex[indices[ii-3].indices.x].Position.z);
                        g_message ("  vertex[p3].Position = (%g %g %g)", vertex[indices[ii-2].indices.x].Position.x, vertex[indices[ii-2].indices.x].Position.y, vertex[indices[ii-2].indices.x].Position.z);
                        g_message ("  vertex[p4].Position = (%g %g %g)", vertex[indices[ii-1].indices.x].Position.x, vertex[indices[ii-1].indices.x].Position.y, vertex[indices[ii-1].indices.x].Position.z);
#endif
                }
                g_message ("mkplane -- Indices Count=%d of %d", ii, IndicesCount);
        };

private:        
        bool Validated;
        glf::vertex_v3fn3fc4f *vertex;
        glf::vertex_v1i *indices;
        GLuint BaseGridW;
        GLuint BaseGridH;
	GLuint ArrayBufferName;
	GLuint IndexBufferName;
	GLuint VertexArrayName;
	GLsizei VertexCount;
	GLsizei IndicesCount;
        GLsizei IndicesCountSurface;
	GLsizeiptr VertexObjectSize;
	GLsizeiptr IndicesObjectSize;
};

class text_plane{
public:
        text_plane (){
                Validated = true;
                ArrayBufferName = 0;
                VertexArrayName = 0;

                if(FT_Init_FreeType(&ft)) {
                        g_warning ("Could not init freetype library.");
                }


                if(FT_New_Face(ft, "FreeSans.ttf", 0, &face)) {
                        g_warning ("Could not open font.");
                }

                FT_Set_Pixel_Sizes(face, 0, 48);
                g = face->glyph;

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                Validated = init_buffer ();
                Validated = init_vao ();
        };
        ~text_plane (){
		if (Validated){
                        glDeleteTextures(1, &tex);
                        glDeleteVertexArrays(1, &VertexArrayName);
                        checkError("text_plane::~delete");
                }
        };
        gboolean init_buffer (){
                checkError("text_plane:: init_buffer");
		if (!Validated) return false;

                glActiveTexture(GL_TEXTURE2); // dedicated for text
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);
                glUniform1i (glGetUniformLocation (ProgramName, "text_tex"), 2);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                //GLuint vbo;
                //glGenBuffers(1, &vbo);
                //glEnableVertexAttribArray(attribute_coord);
                //glBindBuffer(GL_ARRAY_BUFFER, vbo);
                //glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
                //---
                glGenBuffers(1, &ArrayBufferName);
                glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                //-->> glBufferData(GL_ARRAY_BUFFER, VertexObjectSize, vertex, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                return Validated && checkError("text_plane:: init_buffer");
        };
        gboolean init_vao (){
                g_message ("base_plane init_vao");
                checkError("make_plane::init_vao");
		if (!Validated) return false;

                // Build a vertex array object
                glGenVertexArrays(1, &VertexArrayName);
                glBindVertexArray(VertexArrayName);

                glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                glVertexAttribPointer(semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v3fn3fc4f), BUFFER_OFFSET(0));
                glVertexAttribPointer(semantic::attr::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v3fn3fc4f), BUFFER_OFFSET(sizeof(glm::vec3)));
                glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v3fn3fc4f), BUFFER_OFFSET(2*sizeof(glm::vec3)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glEnableVertexAttribArray(semantic::attr::POSITION);
                glEnableVertexAttribArray(semantic::attr::NORMAL);
                glEnableVertexAttribArray(semantic::attr::COLOR);
                glBindVertexArray(0);

                g_message ("text_plane init_vao end");

                return Validated && checkError("make_plane::init_vao");
        };
        gboolean draw_text (const char *text, float x, float y, float sx, float sy) {
 		if (!Validated) return false;
                const char *p;

                for(p = text; *p; p++) {
                        if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
                                continue;
 
                        glActiveTexture(GL_TEXTURE2); // dedicated for text
                        glTexImage2D(
                                     GL_TEXTURE_2D,
                                     0,
                                     GL_RED,
                                     g->bitmap.width,
                                     g->bitmap.rows,
                                     0,
                                     GL_RED,
                                     GL_UNSIGNED_BYTE,
                                     g->bitmap.buffer
                                     );
 
                        float x2 = x + g->bitmap_left * sx;
                        float y2 = -y - g->bitmap_top * sy;
                        float w = g->bitmap.width * sx;
                        float h = g->bitmap.rows * sy;
 
                        GLfloat box[4][4] = {
                                {x2,     -y2    , 0, 0},
                                {x2 + w, -y2    , 1, 0},
                                {x2,     -y2 - h, 0, 1},
                                {x2 + w, -y2 - h, 1, 1},
                        };
 
                        glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                        glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                        x += (g->advance.x/64) * sx;
                        y += (g->advance.y/64) * sy;
                }
                return Validated && checkError("make_plane::draw");
        };
private:        
        bool Validated;
        FT_Library ft;
        FT_Face face;
        FT_GlyphSlot g;;
        //glf::vertex_v3fn3fc4f *vertex;
        GLuint tex;
	GLuint ArrayBufferName;
	GLuint VertexArrayName;
	GLsizei VertexCount;
	GLsizeiptr VertexObjectSize;
	GLsizeiptr IndicesObjectSize;
};



// ------------------------------------------------------------
// core GL configuration management
// ------------------------------------------------------------
class gl_400_primitive_tessellation{
public:
        gl_400_primitive_tessellation (GtkGLArea *area, Surf3d *surf){
                Validated = true;
                glarea = area;
                s = surf;
                Major=4;
                Minor=0;

                MouseOrigin  = glm::ivec2(0, 0);
                MouseCurrent = glm::ivec2(0, 0);

                double aspect = (s->get_scan ())->data.s.ry / (s->get_scan ())->data.s.rx;

                TranslationOrigin  = glm::vec2(0, 1.0*aspect);
              	TranslationCurrent = TranslationOrigin;
                Translation3axis = glm::vec3(0.0f,0.0f,0.0f);

                DistanceOrigin  = glm::vec3(-60., 0., 0.);
              	DistanceCurrent = DistanceOrigin;


                RotationOrigin = glm::vec2(0.75,0);
                RotationCurrent = RotationOrigin;
                Rotation3axis = glm::vec3(0.0f,0.0f,0.0f);

                WindowSize  = glm::ivec2(500, 500);
                surface_plane = NULL;
        };
        ~gl_400_primitive_tessellation(){
                //end (); // too late, glarea reference is gone! 
                if (surface_plane)
                        delete surface_plane;
        };

private:
        
        glm::vec3 cameraPosition() const {
                return glm::vec3(0.,  this->DistanceCurrent.y, this->DistanceCurrent.x);
        };
        glm::vec3 modelPosition() const {
                return glm::vec3(this->TranslationCurrent.x, 0., this->TranslationCurrent.y);
        };
        glm::mat4 modelView() const {
                // rotate model 1st around it's origin
                glm::mat4 ModelRotateX = glm::rotate(glm::mat4(1.0f), -this->RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f)); // X
                glm::mat4 ModelRotateY = glm::rotate(ModelRotateX, this->RotationCurrent.x, glm::vec3(0.f, 0.f, 1.f)); // GL Z is Screen depth = surface Y
                glm::mat4 ModelRotateZ = glm::rotate(ModelRotateY, this->Rotation3axis.z, glm::vec3(0.f, 1.f, 0.f)); // GL Y is Screen Y = surface Z (I hate it)
                // then translate
                glm::mat4 ModelTranslate = glm::translate(ModelRotateZ,  modelPosition());
                // final ModelView
                return ModelTranslate;
        };

	bool initProgram() {
		if (Validated){
                        compiler Compiler;

                        GLuint VertexShader     = Compiler.create (GL_VERTEX_SHADER, getDataDirectory() + SAMPLE_VERTEX_SHADER);
                        GLuint ControlShader    = Compiler.create (GL_TESS_CONTROL_SHADER, getDataDirectory() + SAMPLE_CONTROL_SHADER);
                        GLuint EvaluationShader = Compiler.create (GL_TESS_EVALUATION_SHADER, getDataDirectory() + SAMPLE_EVALUATION_SHADER);
                        GLuint GeometryShader   = Compiler.create (GL_GEOMETRY_SHADER, getDataDirectory() + SAMPLE_GEOMETRY_SHADER);
                        GLuint FragmentShader   = Compiler.create (GL_FRAGMENT_SHADER, getDataDirectory() + SAMPLE_FRAGMENT_SHADER);

                        if (!VertexShader || !ControlShader || !EvaluationShader || !GeometryShader || !FragmentShader)
                                Validated = false;
                        else {
                                ProgramName = glCreateProgram ();
                                glAttachShader (ProgramName, VertexShader);
                                glAttachShader (ProgramName, ControlShader);
                                glAttachShader (ProgramName, EvaluationShader);
                                glAttachShader (ProgramName, GeometryShader);
                                glAttachShader (ProgramName, FragmentShader);
                                glLinkProgram (ProgramName);
                        }
                        
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.check_program(ProgramName);
		}

		if(Validated){
                        Uniform_ModelViewProjection = glGetUniformLocation(ProgramName, "ModelViewProjection"); // mat4 -- projection
                        Uniform_ModelView     = glGetUniformLocation (ProgramName, "ModelView"); // mat4 -- projection
                        Uniform_lightDirWorld = glGetUniformLocation (ProgramName, "lightDirWorld"); // vec3
                        Uniform_eyePosWorld   = glGetUniformLocation (ProgramName, "eyePosWorld"); // vec3
                        Uniform_sunColor      = glGetUniformLocation (ProgramName, "sunColor"); // = vec3(1.0, 1.0, 0.7);
                        Uniform_specularColor = glGetUniformLocation (ProgramName, "specularColor"); // = vec3(1.0, 1.0, 0.7)*1.5;
                        Uniform_ambientColor  = glGetUniformLocation (ProgramName, "ambientColor"); // = vec3(1.0, 1.0, 0.7)*1.5;
                        Uniform_diffuseColor  = glGetUniformLocation (ProgramName, "diffuseColor"); // = vec3(1.0, 1.0, 0.7)*1.5;
                        Uniform_fogColor      = glGetUniformLocation (ProgramName, "fogColor"); // = vec3(0.7, 0.8, 1.0)*0.7;

                        Uniform_fogExp        = glGetUniformLocation (ProgramName, "fogExp"); // = 0.1;

                        Uniform_shininess     = glGetUniformLocation (ProgramName, "shininess"); // = 100.0;
                        Uniform_lightness     = glGetUniformLocation (ProgramName, "lightness"); // = 1.0;
                        Uniform_color_offset  = glGetUniformLocation (ProgramName, "color_offset"); // = vec4(0,0,0,0)
                        Uniform_ambientColor  = glGetUniformLocation (ProgramName, "ambientColor"); // = vec3(0.05, 0.05, 0.15 );
                        Uniform_wrap          = glGetUniformLocation (ProgramName, "wrap"); // = 0.3;
                        Uniform_delta         = glGetUniformLocation (ProgramName, "delta"); //  1/nx,1/ny

                        Uniform_height_scale  = glGetUniformLocation (ProgramName, "height_scale");  // float
                        Uniform_height_offset = glGetUniformLocation (ProgramName, "height_offset");  // float
                        Uniform_aspect        = glGetUniformLocation (ProgramName, "aspect");  // float
                        Uniform_color_source  = glGetUniformLocation (ProgramName, "color_source");  // int
                        Uniform_screen_size   = glGetUniformLocation (ProgramName, "screen_size"); // vec2
                        Uniform_lod_factor    = glGetUniformLocation (ProgramName, "lod_factor");  // float
                        Uniform_tess_level    = glGetUniformLocation (ProgramName, "tess_level");  // float

                        checkError("initProgram -- get uniform variable references...");

                        // get shaderFunction references
                        // Specifies the shader stage from which to query for subroutine uniform index. shadertype must be one of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER.
                        Uniform_vertexDirect      = glGetSubroutineIndex (ProgramName, GL_VERTEX_SHADER, "vertexDirect" );
                        Uniform_vertexSurface     = glGetSubroutineIndex (ProgramName, GL_VERTEX_SHADER, "vertexSurface" );
                        Uniform_vertexHScaled     = glGetSubroutineIndex (ProgramName, GL_VERTEX_SHADER, "vertexHScaled" );

                        Uniform_evaluationDirect  = glGetSubroutineIndex (ProgramName, GL_TESS_EVALUATION_SHADER, "evaluationDirect" );
                        Uniform_evaluationSurface = glGetSubroutineIndex (ProgramName, GL_TESS_EVALUATION_SHADER, "evaluationSurface" );
                        Uniform_evaluationHScaled = glGetSubroutineIndex (ProgramName, GL_TESS_EVALUATION_SHADER, "evaluationHScaled" );

                        Uniform_shadeTerrain      = glGetSubroutineIndex (ProgramName, GL_FRAGMENT_SHADER, "shadeTerrain" );
                        Uniform_shadeDebugMode    = glGetSubroutineIndex (ProgramName, GL_FRAGMENT_SHADER, "shadeDebugMode" );
                        Uniform_shadeLambertian   = glGetSubroutineIndex (ProgramName, GL_FRAGMENT_SHADER, "shadeLambertian" );

                        checkError("initProgram -- get uniform subroutine references...");
		}

                if (Validated)
                        return Validated && checkError("initProgram");

		return Validated;
	};

	bool initBuffer() {
		if (!Validated) return false;

                surface_plane = new base_plane ((s->get_scan ())->mem2d, 128, // 128,
                                                (s->get_scan ())->data.s.ry/(s->get_scan ())->data.s.rx,
                                                s->GLv_data.box_mat_color
                                                );
                
		return checkError("initBuffer");
	};

	bool initVertexArray() {
		if (!Validated) return false;
                // base_plane takes care of this itself
                return checkError("initVertexArray");
	};


        bool initTextures() {
		if (!Validated) return false;

                // sampler2D diffuse
                glUseProgram (ProgramName);

                glGenTextures(TextureCount, TextureName);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, TextureName[0]);

                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, numx, numy, 0, GL_RGBA, GL_FLOAT, Surf3D_Color);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap(GL_TEXTURE_2D);
                glUniform1i (glGetUniformLocation (ProgramName, "diffuse"), 0);

                // sampler2D terrain
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, TextureName[1]);
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, numx, numy, 0, GL_RGBA, GL_FLOAT, Surf3D_Normal_Z);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap(GL_TEXTURE_2D);
                glUniform1i (glGetUniformLocation (ProgramName, "terrain"), 1);

                glBindTexture(GL_TEXTURE_2D, 0);
                
		return checkError("initTextures");
        };
        
        static void GLMessageHandler (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) { 
                g_message ("Source : %d; Type: %d; ID : %d; Severity : %d; length : %d\n==> %s",
                           source, type, id, severity, length,
                           message);
        };

        bool initDebugOutput()
        {
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
                glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
                glDebugMessageCallbackARB ((GLDEBUGPROCARB)&GLMessageHandler, NULL);

                return checkError("initDebugOutput");
        };

public:
	int version(int major, int minor) const{ return major * 100 + minor * 10; }
        bool checkExtension(char const * ExtensionName) const {
                GLint ExtensionCount = 0;
                glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
                for(GLint i = 0; i < ExtensionCount; ++i)
                        if(std::string((char const*)glGetStringi(GL_EXTENSIONS, i)) == std::string(ExtensionName))
                                return true;
                printf("Failed to find Extension: \"%s\"\n", ExtensionName);
                return false;
        };

        bool checkGLVersion(GLint MajorVersionRequire, GLint MinorVersionRequire) const {
                GLint MajorVersionContext = 0;
                GLint MinorVersionContext = 0;
                glGetIntegerv(GL_MAJOR_VERSION, &MajorVersionContext);
                glGetIntegerv(GL_MINOR_VERSION, &MinorVersionContext);
                printf("OpenGL Version Needed %d.%d ( %d.%d Found )\n",
                       MajorVersionRequire, MinorVersionRequire,
                       MajorVersionContext, MinorVersionContext);
                return version (MajorVersionContext, MinorVersionContext) 
                        >=  version (MajorVersionRequire, MinorVersionRequire);
        };

	bool begin() {
		if (!Validated) return false;

                /* we need to ensure that the GdkGLContext is set before calling GL API */
                gtk_gl_area_make_current (glarea);

                glewExperimental = GL_TRUE;
                glewInit();

		if (version (Major, Minor) >= version (3, 0))
			Validated = checkGLVersion (Major, Minor);
                else
                        Validated = false;
                
                if(Validated)
                        Validated = initProgram();
                if(Validated)
                        Validated = initBuffer();
                if(Validated)
                        Validated = initVertexArray();
                if(Validated)
                        Validated = initTextures();
                //if(Validated)
                //        Validated = initDebugOutput();
                
                if (Validated)
                        return Validated && checkError("begin");

                return Validated;
	};

	bool end() {
		if (!Validated) return false;

                /* we need to ensure that the GdkGLContext is set before calling GL API */
                gtk_gl_area_make_current (glarea);

                if (surface_plane)
                        delete surface_plane;
                
                glDeleteTextures(TextureCount, TextureName);
                glDeleteProgram(ProgramName);

		return checkError("end");
	};

        // used for height mapping
        bool updateTexture (GLint line, GLsizei num_lines=1) { 
		if (!Validated) return false;

                glTextureSubImage2D(TextureName[0], 0, 0, line, numx, num_lines, GL_RGBA, GL_FLOAT, &Surf3D_Color[line*numx]);
                glTextureSubImage2D(TextureName[1], 0, 0, line, numx, num_lines, GL_RGBA, GL_FLOAT, &Surf3D_Normal_Z[line*numx]);

		return checkError("initTextures");
        };

	bool render() {
		if (!Validated) return false;

                g_message ("Render (GL coord system): Camera at = (0, %g, %g),"
                           " Translate = (%g, 0, %g),"
                           " Rotate = (%g, %g, %g),"
                           " Height-Scale = %g"
                           "",
                           DistanceCurrent.y, DistanceCurrent.x,
                           TranslationCurrent.x, TranslationCurrent.y,
                           RotationCurrent.y, Rotation3axis.z, RotationCurrent.x, 
                           s->GLv_data.hskl);

                glClearBufferfv (GL_COLOR, 0, s->GLv_data.clear_color);
                glEnable (GL_DEPTH_TEST);
                glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glPolygonMode (GL_FRONT_AND_BACK, s->GLv_data.Mesh ? GL_LINE : GL_FILL);
                // glShadeModel(s->GLv_data.Smooth ? GL_SMOOTH : GL_FLAT); // depricated

                if (s->GLv_data.Cull){
                        glEnable (GL_CULL_FACE);
                        glCullFace (GL_BACK);
                } else {
                        glDisable (GL_CULL_FACE);
                }
                
                if (s->GLv_data.Smooth)
                        ; //glEnable (GL_LINE_SMOOTH);


                // https://glm.g-truc.net/0.9.4/api/a00151.html
                float aspect = WindowSize.x/WindowSize.y;
                // GLfloat fov=45.0f, GLfloat near=0.1f, GLfloat far=100.0f
                glm::mat4 Projection = glm::perspective (glm::radians (s->GLv_data.fov/57.3f), aspect, s->GLv_data.Znear, s->GLv_data.Zfar);
                glm::mat4 Camera = glm::lookAt (cameraPosition(), // cameraPosition, the position of your camera, in world space
                                                glm::vec3(0,0,0), //cameraTarget, where you want to look at, in world space
                                                glm::vec3(0,1,0) // upVector, probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
                                                );

		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 ModelView = this->modelView() * Model;
		glm::mat4 ModelViewProjection = Projection * Camera * ModelView;

                /* clear the viewport; the viewport is automatically resized when
                 * the GtkGLArea gets a new size allocation
                 */
                // glViewport (0, 0, WindowSize.x, WindowSize.y);

		glUseProgram (ProgramName);
                // Projection
                glUniformMatrix4fv (Uniform_ModelViewProjection, 1, GL_FALSE, &ModelViewProjection[0][0]);
                glUniformMatrix4fv (Uniform_ModelView, 1, GL_FALSE, &ModelView[0][0]);
                glm::vec3 eye = cameraPosition(); // eye / camera
                glUniform3fv (Uniform_lightDirWorld, 1, &s->GLv_data.light_position[0][0]);
                glUniform3fv (Uniform_eyePosWorld, 1, &eye.x); //s->GLv_data.light_position[1][0]);

                // Light
                //glm::vec3 light_color = 1.5f*glm::vec3(s->GLv_data.surf_mat_color[0], s->GLv_data.surf_mat_color[1], s->GLv_data.surf_mat_color[2]);
                glUniform3fv (Uniform_sunColor, 1, &s->GLv_data.light_specular[0][0]); // = vec3(1.0, 1.0, 0.7);
                glUniform3fv (Uniform_specularColor, 1, &s->GLv_data.surf_mat_specular[0]); // = vec3(1.0, 1.0, 0.7)*1.5;
                glUniform3fv (Uniform_ambientColor, 1, &s->GLv_data.surf_mat_ambient[0]); // = vec3(1.0, 1.0, 0.7)*1.5;
                glUniform3fv (Uniform_diffuseColor, 1, &s->GLv_data.surf_mat_diffuse[0]); // = vec3(1.0, 1.0, 0.7)*1.5;
                glUniform3fv (Uniform_fogColor, 1, &s->GLv_data.fog_color[0]); // = vec3(0.7, 0.8, 1.0)*0.7;

                glUniform1f  (Uniform_fogExp, s->GLv_data.fog_density/100.); // = 0.1;

                glUniform1f  (Uniform_shininess, 4.*(1.00001-s->GLv_data.surf_mat_shininess[0]/100.));
                glUniform1f  (Uniform_lightness, s->GLv_data.ColorContrast);
                glUniform4f  (Uniform_color_offset, s->GLv_data.ColorOffset, s->GLv_data.ColorOffset, s->GLv_data.ColorOffset, s->GLv_data.transparency_offset);
                glUniform3fv (Uniform_ambientColor, 1, &s->GLv_data.light_global_ambient[0]); // = vec3(0.05, 0.05, 0.15 );
                glUniform1f  (Uniform_wrap, 0.3); //&s->GLv_data.xxx); // = 0.3;
                glUniform2f  (Uniform_delta, 2./s->XPM_x, 2./s->XPM_x);

                // Geometry control
                glUniform1f (Uniform_height_scale, s->GLv_data.hskl);
                glUniform1f (Uniform_height_offset, s->GLv_data.slice_offset);
                glUniform1f (Uniform_aspect, (s->get_scan ())->data.s.ry/(s->get_scan ())->data.s.rx);
                
                glUniform1i (Uniform_color_source, (int)s->GLv_data.shader_mode);
                glUniform1f (Uniform_lod_factor, 4.0);
                glUniform1f (Uniform_tess_level, s->GLv_data.tess_level);
                
                glEnableVertexAttribArray(TextureName[0]);
                glEnableVertexAttribArray(TextureName[1]);
                glBindTexture(GL_TEXTURE_2D, TextureName[0]);
                glBindTexture(GL_TEXTURE_2D, TextureName[1]);

                checkError ("set Uniforms");

                // Specifies the shader stage from which to query for subroutine uniform index. shadertype must be one of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER.

                // configure shaders for surface terrain mode tesselation and select final shading mode
                glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexSurface);
                glUniformSubroutinesuiv (GL_TESS_EVALUATION_SHADER, 1, &Uniform_evaluationSurface);

                switch (s->GLv_data.ShadeModel[0]){
                case 'R':
                case 'L': glUniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &Uniform_shadeLambertian); break;
		case 'T': glUniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &Uniform_shadeTerrain); break;
		case 'D': glUniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &Uniform_shadeDebugMode); break;
                }
                
                checkError ("set Uniforms Subroutines");
                
                surface_plane->draw (s->GLv_data.TickFrameOptions[0]=='2' || s->GLv_data.TickFrameOptions[0]=='3'); // surface with box or plane surface only
                glUniform1f (Uniform_tess_level, 1.);

		return checkError("render");
                
		//return true;
	};

        void resize (gint w, gint h){
                WindowSize  = glm::ivec2(w, h);
        };

        void cursorPositionCallback(int mouse, double x, double y){
                if (mouse == 'Z'){ // incremental
                        DistanceCurrent = (DistanceOrigin += glm::vec3(x, y, 0.));
                        return;
                }
                if (mouse == 'i'){
                        MouseOrigin = glm::ivec2(x, y);
                        return;
                }
                if (mouse == 't'){
                        TranslationOrigin = TranslationCurrent;
                        return;
                }
                if (mouse == 'r'){
                        RotationOrigin = RotationCurrent;
                        return;
                }
                if (mouse == 'm'){
                        DistanceOrigin = DistanceCurrent;
                        return;
                }

                MouseCurrent = glm::ivec2(x, y);
                DistanceCurrent    = mouse == 'M' ? DistanceOrigin + glm::vec3(glm::vec2((MouseCurrent - MouseOrigin)) / 10.f, 0) : DistanceOrigin;
                TranslationCurrent = mouse == 'T' ? TranslationOrigin + (MouseCurrent - MouseOrigin) / 100.f : TranslationOrigin;
                RotationCurrent    = mouse == 'R' ? RotationOrigin + glm::radians(MouseCurrent - MouseOrigin) : RotationOrigin;
        };

        void get_rotation (float *wxyz){
                float dr = 180./M_PI;
                wxyz[0] = dr*RotationCurrent.x;
                wxyz[1] = dr*RotationCurrent.y;
        };
        void set_rotation (float *wxyz){
                float dr = M_PI/180.;
                Rotation3axis = glm::vec3(wxyz[0]*dr, -wxyz[1]*dr, wxyz[2]*dr);
                RotationOrigin = glm::vec2(wxyz[0]*dr, -wxyz[1]*dr);
                cursorPositionCallback('x',0,0);
                //g_message ("Rx %f Ry %f", RotationOrigin.x,RotationOrigin.y);
        };
        void get_translation (float *rxyz){
                rxyz[0] = TranslationCurrent.x;
                rxyz[1] = TranslationCurrent.y;
        };
        void set_translation (float *rxyz){
                Translation3axis = glm::vec3(rxyz[0], rxyz[1], rxyz[2]);
                TranslationOrigin = glm::vec2(rxyz[0], rxyz[1]);
                cursorPositionCallback('x',0,0);
                //g_message ("Tx %f Ty %f", TranslationOrigin.x,TranslationOrigin.y);
        };
        void get_distance (float *d){
                d[0] = DistanceCurrent.y;
        };
        void set_distance (float *d){
                DistanceCurrent = DistanceOrigin = glm::vec3(DistanceCurrent.x, d[0], DistanceCurrent.z);
        };

        void set_surface_data (glm::vec4 *data, glm::vec4 *color, int nx, int ny, int nv){
                Surf3D_Normal_Z = data;
                Surf3D_Color    = color;
                numx = nx;
                numy = ny;
                numv = nv;
        };

private:
        bool Validated;
        Surf3d *s;
        GtkGLArea *glarea;
        int Major, Minor; // minimal version needed

        base_plane *surface_plane;
        
        glm::vec2 WindowSize;
	glm::vec2 MouseOrigin;
	glm::vec2 MouseCurrent;
	glm::vec2 TranslationOrigin;
	glm::vec2 TranslationCurrent;
	glm::vec3 DistanceOrigin;
	glm::vec3 DistanceCurrent;
	glm::vec2 RotationOrigin;
	glm::vec2 RotationCurrent;
	glm::vec3 Rotation3axis;
	glm::vec3 Translation3axis;

        glm::vec4 *Surf3D_Normal_Z;
        glm::vec4 *Surf3D_Color;
	int numx, numy, numv;
};



// ------------------------------------------------------------
// Class Surf3d -- derived form generic GXSM view class
// handling 3D visulation modes and data prepartations
// using gtk-glarea provided via GUI part managed by app_v3dcontrol
// ------------------------------------------------------------

Surf3d::Surf3d(Scan *sc, int ChNo):View(sc, ChNo){
	XSM_DEBUG (GL_DEBUG_L2, "Surf3d::Surf3d(sc,ch)");
	v3dcontrol = NULL;
        self = this;
	GLvarinit();
        //	g_free (v3dControl_pref_def);
}

Surf3d::Surf3d():View(){
        XSM_DEBUG (GL_DEBUG_L2, "Surf3d::Surf3d()");
        v3dcontrol = NULL;
        self = this;
        GLvarinit();
}

Surf3d::~Surf3d(){
        XSM_DEBUG (GL_DEBUG_L2, "Surf3d::~");

        if (gl_tess)
                delete gl_tess;

        self = NULL;
        if (v3dControl_pref_dlg)
                gnome_res_destroy (v3dControl_pref_dlg);
        hide();

        delete_surface_buffer ();
        g_free (v3dControl_pref_def);
}

void Surf3d::hide(){
	if (v3dcontrol)
		delete v3dcontrol;
	v3dcontrol = NULL;
	XSM_DEBUG (GL_DEBUG_L2, "Surf3d::hide");
}


void inline Surf3d::PutPointMode(int k, int j, int vi){
	int i;
	GLfloat val;
        
	i = k + j*XPM_x + XPM_x*XPM_y*vi;
	if (i >= (int)size) return;

        // check for zero range
        if (! (scan->mem2d->data->zrange > 0.)){
                surface_normal_z_buffer[i].x=surface_normal_z_buffer[i].y=0.; surface_normal_z_buffer[i].z=1.;
                surface_normal_z_buffer[i].w=0.;
                surface_color_buffer[i].x = surface_normal_z_buffer[i].w;
                surface_color_buffer[i].y = surface_normal_z_buffer[i].w;
                surface_color_buffer[i].z = surface_normal_z_buffer[i].w;
                surface_color_buffer[i].w = 1.;
                return;
        }

        
	// mem2d:: inline void GetDataPkt_vec_normal_4F(int x, int y, int v, float *vec4){ 
	// mem2d:: inline void GetDataPktVModeInterpol_vec_normal_4F(double x, double y, double v, float *vec4){ 

        //if (i==0)
        //        g_message ("Z Range: %f .. %f", scan->mem2d->data->zmin, scan->mem2d->data->zmax);


        //scan->mem2d->GetDataPktVModeInterpol_vec_normal_4F ((double)k,(double)j,(double)vi, &surface_normal_z_buffer[i]);
	//scan->mem2d->GetDataPktVMode_vec_normal_4F (k,j,vi, &surface_normal_z_buffer[i], 1./MAXCOLOR);
#if 1
        surface_color_buffer[i].x = 0.;
        surface_color_buffer[i].y = 1.;
        surface_color_buffer[i].z = 0.;
        surface_normal_z_buffer[i].w = (scan->mem2d->GetDataPkt(k,j,vi)-scan->mem2d->data->zmin)/scan->mem2d->data->zrange;
#else
	scan->mem2d->GetDataPkt_vec_normal_4F (k,j,vi, &surface_normal_z_buffer[i], 1.0, XPM_x*GLv_data.hskl/scan->mem2d->data->zrange);
        // normals are not recalculated by GPU based on actual scaled Z -- so here kind of obsolete -- clean up or leave?? TDB
        // shift Norm to 0 .. 1
        surface_normal_z_buffer[i].x *= 0.5; surface_normal_z_buffer[i].x += 0.5;
        surface_normal_z_buffer[i].y *= 0.5; surface_normal_z_buffer[i].x += 0.5;
        surface_normal_z_buffer[i].z *= 0.5; surface_normal_z_buffer[i].x += 0.5;
        // normalize Z to 0 .. 1
        surface_normal_z_buffer[i].w -= scan->mem2d->data->zmin;
        surface_normal_z_buffer[i].w /= scan->mem2d->data->zrange;
#endif
        
        if(0){
                if (k==0 || j==0 || k == XPM_x-1 || j == XPM_y-1)
                        surface_normal_z_buffer[i].w = -1.;
        }
        
	if (GLv_data.ColorSrc[0] != 'U'){
		switch (GLv_data.ColorSrc[0]){
		case 'H': 
			val = (GLfloat) (surface_normal_z_buffer[i].w);
			break;
		case 'X': 
			if (mem2d_x){
				// map to index range of Chan-X, assumes scan range is the same.
				int u = (int) (k * (double)mem2d_x->GetNx () / (double)scan->mem2d->GetNx ());
				int v = (int) (j * (double)mem2d_x->GetNy () / (double)scan->mem2d->GetNy ());
				val = (GLfloat) (mem2d_x->GetDataVMode (u,v)/mem2d_x->GetDataRange ());
			} else
				val = 0.5;
			break;
		default: 
			val = 0.5; 
			break;
		}

		switch (GLv_data.ShadeModel[0]){
		case 'T': // Terrain Color
			calccolor(val*maxcolors, surface_color_buffer[i]);
			break;
		case 'F': // Material Color
			surface_color_buffer[i].x = GLv_data.surf_mat_color[0];
			surface_color_buffer[i].y = GLv_data.surf_mat_color[1];
			surface_color_buffer[i].z = GLv_data.surf_mat_color[2];
			break;
                case 'D':
		case 'L': { // GXSM user Palette
			int ci = (int)(val*maxcolors);
			ci = ci < 0 ? 0 : ci >= maxcolors ? (maxcolors-1) : ci;
			surface_color_buffer[i].x = ColorLookup[ci][0];
			surface_color_buffer[i].y = ColorLookup[ci][1];
			surface_color_buffer[i].z = ColorLookup[ci][2];
		} break;
		case 'R': // RGBA Color
			switch (GLv_data.ColorSrc[0]){
			case 'H': 
				if (scan->mem2d->GetNv() == 4){
					surface_color_buffer[i].x = scan->mem2d->GetDataPkt (k,j,0) / 255.;
					surface_color_buffer[i].y = scan->mem2d->GetDataPkt (k,j,1) / 255.;
					surface_color_buffer[i].z = scan->mem2d->GetDataPkt (k,j,2) / 255.;
				}
				break;
			case 'X': 
				if (mem2d_x){				
					int u = (int) (k * (double)mem2d_x->GetNx () / (double)scan->mem2d->GetNx ());
					int v = (int) (j * (double)mem2d_x->GetNy () / (double)scan->mem2d->GetNy ());
					if (mem2d_x->GetNv() == 4){
						surface_color_buffer[i].x = mem2d_x->GetDataPkt (u,v,0) / 255.;
						surface_color_buffer[i].y = mem2d_x->GetDataPkt (u,v,1) / 255.;
						surface_color_buffer[i].z = mem2d_x->GetDataPkt (u,v,2) / 255.;
					}
				}
				break;
			}
			break;
		}  
		surface_color_buffer[i].x *= GLv_data.ColorSat;
		surface_color_buffer[i].y *= GLv_data.ColorSat;
		surface_color_buffer[i].z *= GLv_data.ColorSat;
	}

	// set transparency/alpha blending
	val = (GLfloat) (GLv_data.transparency_offset + GLv_data.transparency * surface_normal_z_buffer[i].w);
	surface_color_buffer[i].w = val > 1.? 1. : val < 0.? 0. : val;

        if (XPM_v > 1){
                // adjust slice height level, renormalize
                surface_normal_z_buffer[i].w /= XPM_v;
                surface_normal_z_buffer[i].w += (GLfloat)vi*GLv_data.slice_offset*0.5/XPM_v;
        }
}


void Surf3d::GLvarinit(){
	mem2d_x=NULL;
        gl_tess = NULL;
	
	XPM_x = XPM_y = 0;
	scrwidth=500;
	scrheight=500;
	ZoomFac=1;
	size=0;
        surface_normal_z_buffer = NULL;
	surface_color_buffer = NULL;

	ReadPalette (xsmres.Palette);

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
	
}

void Surf3d::GLupdate (void* data){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLUPDATE" << std::flush);

        if (data){
        
                Surf3d *s = (Surf3d *) data;

                s->check_dimension_changed ();

                XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLUPDATE set color source" << std::flush);
                s->ColorSrc();

                XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLUPDATE rerender" << std::flush);
                if (s->gl_tess)
                        s->gl_tess->set_rotation (s->GLv_data.rot);

                if (s->v3dcontrol)
                        s->v3dcontrol->rerender_scene ();
        }
        XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLUPDATE done." << std::flush);
}

void Surf3d::set_gl_data (){
        if (gl_tess){
                XSM_DEBUG (GL_DEBUG_L2, "Surf3d::set_gl_data." << std::flush);
                delete_surface_buffer ();
                create_surface_buffer ();
                gl_tess->set_surface_data (surface_normal_z_buffer, surface_color_buffer, XPM_x, XPM_y, XPM_v);
                XSM_DEBUG (GL_DEBUG_L2, "Surf3d::set_gl_data OK." << std::flush);
        }
}

gboolean Surf3d::check_dimension_changed(){
        if (XPM_x != scan->mem2d->GetNx() || XPM_y != scan->mem2d->GetNy() || XPM_v != scan->mem2d->GetNv()){
                g_message ("Reshaping GL scene/surface");
                if (gl_tess){
                        g_message ("Calling gl_tess->end()");
                        gl_tess->end ();

                        g_message ("Recalculating buffers");
                        set_gl_data ();
        
                        g_message ("Calling  gl_tess->begin()");
                        if (! gl_tess->begin()){
                                gchar *message = g_strdup_printf
                                        ("FAILURE GL-TESS BEGIN/INIT failed:\n"
                                         " --> GL VERSION requirements for GL 4.0 not satified?\n"
                                         " --> GL GLSL program code error or not found/installed?");

                                g_critical (message);
                                gapp->warning (message);
                                g_free (message);

                                delete gl_tess;
                                gl_tess = NULL;
                        }
                }
                return true;
        } else
                return false;

}

void Surf3d::create_surface_buffer(){ 
	XSM_DEBUG (GL_DEBUG_L2, "Surf3d::create_surface_buffer");

	if (v3dcontrol){
		gchar *titel = g_strdup_printf ("GL: Ch%d: %s TF%d %s", 
						ChanNo+1, data->ui.name, 
						(int)GLv_data.tess_level,
                                                scan->mem2d->GetEname());
		
		v3dcontrol->SetTitle(titel);
		g_free(titel);
	}

	XPM_x = scan->mem2d->GetNx();
	XPM_y = scan->mem2d->GetNy();
	XPM_v = scan->mem2d->GetNv();

	size = XPM_x * XPM_y * XPM_v;

	XSM_DEBUG_GP (GL_DEBUG_L3, "Surf3d::create_surface_buffer  **nXYV (%d, %d, %d)\n", XPM_x, XPM_y, XPM_v);
        
	surface_normal_z_buffer = g_new (glm::vec4, size * sizeof(glm::vec4));
	surface_color_buffer    = g_new (glm::vec4, size * sizeof(glm::vec4));

	XSM_DEBUG (GL_DEBUG_L3, "Surf3d::create_surface_buffer  ** g_new completed");

	setup_data_transformation();

	XSM_DEBUG (GL_DEBUG_L3, "Surf3d::create_surface_buffer  ** computing surface and normals");
        // grab and prepare data buffers
        for(int v=0; v<scan->mem2d->GetNv(); ++v){
                scan->mem2d->data->update_ranges (v);
		for(int j=0; j<scan->mem2d->GetNy(); ++j)
			for(int k=0; k<scan->mem2d->GetNx(); ++k)
				PutPointMode (k,j,v);
        }
	XSM_DEBUG (GL_DEBUG_L3, "Surf3d::create_surface_buffer  ** completed");
#if 0
        // dump to file
        gchar *fn = g_strdup_printf("gxsm_normal_z_%dx%d.terrain", XPM_x, XPM_y);
        FILE* terrain = fopen(fn, "wb");
        g_free (fn);
        fwrite ((void*)surface_normal_z_buffer, sizeof(glm::vec4), (size_t)size, terrain);
        fclose (terrain);

        fn = g_strdup_printf("gxsm_color_rgb_%dx%d.diffuse", XPM_x, XPM_y);
        FILE* diffuse = fopen(fn, "wb");
        g_free (fn);
        fwrite ((void*)surface_color_buffer, sizeof(glm::vec4), (size_t)size, diffuse);
        fclose (diffuse);
#endif   
}

void Surf3d::delete_surface_buffer(){ 
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D::delete_surface_buffer -- check for cleanup");

	if (surface_normal_z_buffer){
                XSM_DEBUG (GL_DEBUG_L2, "SURF3D::delete_surface_buffer -- g_free: normal_z_buffer");
		g_free (surface_normal_z_buffer);
                surface_normal_z_buffer = NULL;
        }

	if (surface_color_buffer){
                XSM_DEBUG (GL_DEBUG_L2, "SURF3D::delete_surface_buffer -- g_free: color_buffer");
		g_free (surface_color_buffer);
                surface_color_buffer = NULL;
        }
	size = 0;
}

void Surf3d::printstring(void *font, char *string)
{
 	int len,i;
 	len = (int)strlen(string);
 	for(i=0; i<len; i++)
 		;//glutBitmapCharacter(font,string[i]);
}

void Surf3d::MouseControl (int mouse, double x, double y){
        if (gl_tess){
                gl_tess->cursorPositionCallback(mouse, x, y);
                gl_tess->get_rotation (GLv_data.rot);
                gl_tess->get_translation (GLv_data.trans);
        }
}

void Surf3d::RotateAbs(int n, double phi){
	GLv_data.rot[n] = phi;
        if (gl_tess)
                gl_tess->set_rotation (GLv_data.rot);
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

        if (gl_tess)
                gl_tess->set_rotation (GLv_data.rot);
}

double Surf3d::RotateX(double dphi){
	GLv_data.rot[0] += 5*dphi;
        if (gl_tess)
                gl_tess->set_rotation (GLv_data.rot);
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.rot[0];
}

double Surf3d::RotateY(double dphi){
	GLv_data.rot[1] += 5*dphi;
        if (gl_tess)
                gl_tess->set_rotation (GLv_data.rot);
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.rot[1];
}

double Surf3d::RotateZ(double dphi){
	GLv_data.rot[2] += 5*dphi;
        if (gl_tess)
                gl_tess->set_rotation (GLv_data.rot);
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.rot[2];
}

void Surf3d::Translate(int n, double delta){
        GLv_data.trans[n] += delta;
        if (gl_tess)
                gl_tess->set_translation (GLv_data.trans);
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();
}

double Surf3d::Zoom(double x){ 
	GLv_data.dist += x; 
        if (gl_tess)
                gl_tess->set_distance (&GLv_data.dist);
	if (v3dcontrol)
                v3dcontrol->rerender_scene ();

	return GLv_data.dist; 
}

double Surf3d::HeightSkl(double x){ 
	GLv_data.hskl += x; 
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
        maxcolors = 8192;
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

                        maxcolors = MIN(nx, 8192);

			for (int i=0; i<maxcolors; ++i){
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
        maxcolors = 4096;
	for (int i=0; i<maxcolors; ++i)
		ColorLookup[i][0] = ColorLookup[i][1] = ColorLookup[i][2] = i/(double)maxcolors;
}

// height in [0..1] expected!
void Surf3d::calccolor(GLfloat height, glm::vec4 &c)
{
	GLfloat color[4][3]={
		{1.0,1.0,1.0},
		{0.0,0.8,0.0},
		{1.0,1.0,0.3},
		{0.0,0.0,0.8}
	};
	GLfloat fact;
	
	if(height>=0.9) {
		c = glm::vec4 (color[0][0], color[0][1], color[0][2], 1);
		return;
	}
	
	if((height<0.9) && (height>=0.7)) {
		fact=(height-0.7)*5.0;
		c = glm::vec4 (fact*color[0][0]+(1.0-fact)*color[1][0],
                               fact*color[0][1]+(1.0-fact)*color[1][1],
                               fact*color[0][2]+(1.0-fact)*color[1][2],
                               1);
		return;
	}
	
	if((height<0.7) && (height>=0.6)) {
		fact=(height-0.6)*10.0;
		c = glm::vec4 (fact*color[1][0]+(1.0-fact)*color[2][0],
                               fact*color[1][1]+(1.0-fact)*color[2][1],
                               fact*color[1][2]+(1.0-fact)*color[2][2],
                               1);
                return;
	}
	
	if((height<0.6) && (height>=0.5)) {
		fact=(height-0.5)*10.0;
		c = glm::vec4 (fact*color[2][0]+(1.0-fact)*color[3][0],
                               fact*color[2][1]+(1.0-fact)*color[3][1],
                               fact*color[2][2]+(1.0-fact)*color[3][2],
                               1);
		return;
	}
	
	c = glm::vec4 (color[3][0], color[3][1], color[3][2], 1);
}


void Surf3d::setup_data_transformation(){
	scan->mem2d->SetDataPktMode (data->display.ViewFlg);
	scan->mem2d->SetDataRange (0, maxcolors);
}

int Surf3d::update(int y1, int y2){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update y1 y2: " << y1 << "," << y2);

        if (y1 == 0)
                GLupdate (this);

        
	if (!scan || size == 0) return -1;

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update trafo");
	setup_data_transformation();

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update data");
//	int v = scan->mem2d->GetLayer();
	for(int v=0; v<scan->mem2d->GetNv(); ++v){
                scan->mem2d->data->update_ranges (v);
		for(int j=y1; j < y2; ++j)
			for(int k=0; k < scan->mem2d->GetNx (); ++k)
				PutPointMode (k,j,v);
	}

        if (v3dcontrol){
                if (gl_tess)
                        gl_tess->updateTexture (y1, y2-y1);
                v3dcontrol->rerender_scene ();
        }
        
        XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update done.");
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
#if 0
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

	XSM_DEBUG (GL_DEBUG_L2, "GL:::GLdrawGimmicks");
	XSM_DEBUG_GP (GL_DEBUG_L2, "GL:::GLdrawGimmicks");

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
		;//***************FIXME glRasterPos3f( (XPM_x/2.0),0.0, (XPM_y/2.0)); printstring(GLUT_BITMAP_HELVETICA_12, buf);
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
#endif
}


/* 
   Create Zero-pLane(s)
   Tesselate surface into list of triangles andcalculate normals for light processing
*/

void Surf3d::GLdrawsurface(int y_to_update, int refresh_all){

        if (gl_tess){
                if (refresh_all)
                        gl_tess->updateTexture (0, XPM_y);
                else
                        gl_tess->updateTexture (y_to_update);
        }
        
#if 0
	int slice_pli[3];
	XSM_DEBUG (GL_DEBUG_L2, "GL:::GLdrawsurface y_to_update=" << y_to_update);

	if (GLv_data.slice_plane_index[0] >= 0)
		slice_pli[0] = GLv_data.slice_plane_index[0];
	else
		slice_pli[0] = GLv_data.slice_plane_index[0];
	if (GLv_data.slice_plane_index[1] >= 0)
		slice_pli[1] = GLv_data.slice_plane_index[1];
	else
		slice_pli[1] = GLv_data.slice_plane_index[1];
	if (GLv_data.slice_plane_index[2] >= 0)
		slice_pli[2] = GLv_data.slice_plane_index[2];
	else
		slice_pli[2] = GLv_data.slice_plane_index[2];

        XSM_DEBUG_GP (GL_DEBUG_L2, "GL:::GLdrawsurface -- ZP, stuff,...");
        
        if (GLv_data.ZeroPlane){
        }                

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


        
        // add Volume "close-up" strips
        if (((GLv_data.slice_direction[0] == 'Y' || GLv_data.slice_direction[0] == 'V' || slice_pli[1] >= 0) && vi < XPM_v-step2)) {
        }
        if (((GLv_data.slice_direction[0] == 'X' || GLv_data.slice_direction[0] == 'V' || slice_pli[0] >= 0) && vi < XPM_v-step2)) {
	}
        XSM_DEBUG_GP (GL_DEBUG_L2, "GL:::GLdrawsurface -- retesseration done.");
#endif
}



void realize_vsurf3d_cb (GtkGLArea *area, Surf3d *s){
	XSM_DEBUG (GL_DEBUG_L2, "GL:::REALIZE-EVENT");
        // We need to make the context current if we want to
        // call GL API

        // If there were errors during the initialization or
        // when trying to make the context current, this
        // function will return a #GError for you to catch

        if (gtk_gl_area_get_error (area) != NULL)
                return;

        s->gl_tess = new gl_400_primitive_tessellation (area, s);
        s->set_gl_data ();
        
	if (! s->gl_tess->begin()){
                gchar *message = g_strdup_printf
                        ("FAILURE GL-TESS BEGIN/INIT failed:\n"
                         " --> GL VERSION requirements for GL 4.0 not satified?\n"
                         " --> GL GLSL program code error or not found/installed?");

                g_critical (message);
                gapp->warning (message);
                g_free (message);

                delete s->gl_tess;
                s->gl_tess = NULL;
        }
        XSM_DEBUG (GL_DEBUG_L2, "GL:::REALIZE-EVENT  (realize_vsurf3d_cb) completed.");
}


// =========== GTK-GL-AREA CALLBACKS =================
// The “resize” signal handler
void resize_vsurf3d_cb (GtkGLArea *area,
                        gint       width,
                        gint       height,
                        Surf3d *s)
{
	XSM_DEBUG (GL_DEBUG_L2, "GL:::RESIZE-EVENT");
        if (!s) return;
        // if (!s->is_ready()) return;
	XSM_DEBUG (GL_DEBUG_L2, "GL:::RESIZE-EVENT -- updating");

        s->scrwidth=width;
        s->scrheight=height;
        if (s->gl_tess)
                s->gl_tess->resize (width, height);
}

// render_event :    redraw scene

static gboolean
render_vsurf3d_cb (GtkGLArea *area, GdkGLContext *context, Surf3d *s)
{
        XSM_DEBUG (GL_DEBUG_L2, "GL:::RENDER-EVENT enter");

        if (!s) return FALSE;
        // if (!s->is_ready()) return FALSE;
        if (!s->gl_tess) return FALSE;
        
        XSM_DEBUG (GL_DEBUG_L2, "GL:::RENDER-EVENT -- execute GPU tesseleation");

        //s->gl_tess->set_rotation (s->GLv_data.rot);
        //s->gl_tess->get_translation (s->GLv_data.trans);
        return s->gl_tess->render ();
}


int Surf3d::draw(int zoomoverride){

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::DRAW");

	if (!scan->mem2d) { 
		XSM_DEBUG (GL_DEBUG_L2, "Surf3d: no mem2d !"); 
		return 1; 
	}
	
	if ( !v3dcontrol )
		v3dcontrol = new V3dControl ("3D Surface View (using GL/GPU)", ChanNo, scan,
					     G_CALLBACK (resize_vsurf3d_cb),
					     G_CALLBACK (render_vsurf3d_cb),
                                             G_CALLBACK (realize_vsurf3d_cb),
					     self);

        v3dcontrol->rerender_scene ();
        
	return 0;
}

void Surf3d::preferences(){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::PREFERENCES");

	if (v3dControl_pref_dlg)
		gnome_res_run_change_user_config (v3dControl_pref_dlg, "GL Scene Setup");
}
