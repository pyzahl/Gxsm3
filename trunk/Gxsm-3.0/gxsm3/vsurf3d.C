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

//#define __GXSM_PY_DEVEL
#ifdef __GXSM_PY_DEVEL
#define GLSL_DEV_DIR "/home/pzahl/SVN/Gxsm-3.0/gl-400/"
//#define GLSL_DEV_DIR "/home/percy/SVN/Gxsm-3.0/gl-400/"
#endif



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
        // define global include file -- included by compiler
	std::string const CMD_ARGS_FOR_SHADERS("");

        // surface terrain mode tesselation shaders
	std::string const TESSELATION_VERTEX_SHADER("tess-vertex.glsl");
	std::string const TESSELATION_CONTROL_SHADER("tess-control.glsl");
	std::string const TESSELATION_EVALUATION_SHADER("tess-evaluation.glsl");
	std::string const TESSELATION_GEOMETRY_SHADER("tess-geometry.glsl");
	std::string const TESSELATION_FRAGMENT_SHADER("tess-fragment.glsl");
	GLuint Tesselation_ProgramName(0);

        // simple text shaders
        std::string const S3D_VERTEX_SHADER("s3d-vertex.glsl");
	std::string const S3D_FRAGMENT_SHADER("s3d-fragment.glsl");
	GLuint S3D_ProgramName(0);

        // generic vertex tesselation shaders
	std::string const ICO_TESS_VERTEX_SHADER("ico-vertex.glsl");
	std::string const ICO_TESS_CONTROL_SHADER("ico-control.glsl");
	std::string const ICO_TESS_EVALUATION_SHADER("ico-evaluation.glsl");
	std::string const ICO_TESS_GEOMETRY_SHADER("ico-geometry.glsl");
	std::string const ICO_TESS_FRAGMENT_SHADER("ico-fragment.glsl");
	GLuint IcoTess_ProgramName(0);
        // make UBO for tranformation, fragment/lights, geometry, textures
        
	GLuint Uniform_screen_size(0); // vec2
	GLuint Uniform_tess_level(0); // float
	GLuint Uniform_lod_factor(0); // float
        GLsizei const TesselationTextureCount(2);
	GLuint TesselationTextureName[2];

        // Model View and Projection
        GLuint Uniform_ubo_list[3];
        GLuint const ModelViewMat_block(0); // => 0
        ubo::uniform_model_view Block_ModelViewMat(glm::mat4(1.),glm::mat4(1.));

        GLuint const SurfaceGeometry_block(1); // => 1
        ubo::uniform_surface_geometry Block_SurfaceGeometry(1.,0.1,0., glm::vec2(0.1));
        
        GLuint const FragmentShading_block(2); // => 2
        ubo::uniform_fragment_shading Block_FragmentShading
        (
         glm::vec4(-0.2,0.6,-1.0,0.0), // light dir
         glm::vec4(0.,-1.,-70.,0.0), // eye/cam

         glm::vec4(1.), // sun color
         glm::vec4(1.), // spekular
         glm::vec4(0.1), // ambient
         glm::vec4(0.3), // diffuse
         glm::vec4(0.5), // fog color
         glm::vec4(1.0), // material color
         glm::vec4(0.), // color offset

         0.01, // fog exp

         20.0, // shinyness
         1.5, // lightness
         0., // attn.
         1.0, 0.0, // transparency and offset
         
         0.3, // wrap
         0 // debug
         );

        // vertex shader Function references
        GLuint Uniform_vertexFlat(0);
        GLuint Uniform_vertexDirect(0);
        GLuint Uniform_vertexViewMode(0);
        GLuint Uniform_vertexY(0);
        GLuint Uniform_vertexXChannel(0);

        //-- evaluation shader Function references
        GLuint Uniform_evaluation_setup[2];
        GLuint Uniform_evaluationVertexFlat(0); // Vertex == match Vertex Mode above 
        GLuint Uniform_evaluationVertexDirect(0);
        GLuint Uniform_evaluationVertexViewMode(0);
        GLuint Uniform_evaluationVertexY(0);
        GLuint Uniform_evaluationVertexXChannel(0);
        GLuint Uniform_evaluationColorFlat(0); // Color
        GLuint Uniform_evaluationColorDirect(0);
        GLuint Uniform_evaluationColorViewMode(0);
        GLuint Uniform_evaluationColorXChannel(0);
        GLuint Uniform_evaluationColorY(0);

        // fragment shader Function references
        GLuint Uniform_shadeTerrain(0);
        GLuint Uniform_shadeDebugMode(0);
        GLuint Uniform_shadeLambertian(0);

#if 0        //
        GLuint Uniform_S3D_vertexDirect(0); // vertex
        GLuint Uniform_S3D_vertexSurface(0);
        GLuint Uniform_S3D_vertexHScaled(0);
        GLuint Uniform_S3D_vertexText(0);

        GLuint Uniform_S3D_shadeLambertian(0);
        GLuint Uniform_S3D_shadeText(0);
#endif

        GLuint Uniform_IcoTess_TessLevelInner(0);
        GLuint Uniform_IcoTess_TessLevelOuter(0);
        GLuint Uniform_IcoTess_IcoPositionS(0);
        GLuint Uniform_IcoTess_IcoColor(0);

        GLuint Uniform_TipPosition(0);
        GLuint Uniform_TipColor(0);
        
} //namespace

class base_plane{
public:
        base_plane (Mem2d *m=NULL, int w=128, double aspect=1.0){
                Validated = true;
                BaseGridW = w;
                BaseGridH = w; // adjusted by make_plane_vbo using aspect
                ArrayBufferName = 0;
                VertexArrayName = 0;

                make_plane_vbo (m, aspect);

                Validated = init_buffer ();

                Validated = init_vao ();
        };
        ~base_plane (){
		if (Validated){
                        g_free (indices);
                        g_free (vertex);
                        glDeleteVertexArrays(1, &VertexArrayName);
                        glDeleteBuffers(1, &IndexBufferName);
                        glDeleteBuffers(1, &ArrayBufferName);
                        checkError("make_plane::~delete");
                }
        };
        gboolean init_buffer (){
                checkError("make_plane:: init_buffer");
		if (!Validated) return false;

                glGenBuffers(1, &ArrayBufferName);
                glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                //glBufferData(GL_ARRAY_BUFFER, VertexObjectSize, vertex, GL_DYNAMIC_DRAW);
                glBufferData(GL_ARRAY_BUFFER, VertexObjectSize, vertex, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glGenBuffers(1, &IndexBufferName);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);
                //glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesObjectSize, indices, GL_DYNAMIC_DRAW);
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

                glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(0));

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                g_message ("base_plane init_vao end");

                return Validated && checkError("make_plane::init_vao");
        };
        gboolean draw (){
		if (!Validated) return false;
                
                glBindVertexArray (VertexArrayName);
                glEnableVertexAttribArray(semantic::attr::POSITION);
                glEnableVertexAttribArray (TesselationTextureName[0]);
                glEnableVertexAttribArray (TesselationTextureName[1]);
                glBindTexture (GL_TEXTURE_2D, TesselationTextureName[0]);
                glBindTexture (GL_TEXTURE_2D, TesselationTextureName[1]);

                glBindBuffer (GL_ARRAY_BUFFER, ArrayBufferName);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);
                glPatchParameteri (GL_PATCH_VERTICES, 4);

                glDrawElements (GL_PATCHES, IndicesCount, GL_UNSIGNED_INT, 0);

                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
                glBindBuffer (GL_ARRAY_BUFFER, 0);
                glBindVertexArray (0);

                return Validated && checkError("make_plane::draw");
        };
        
        // make plane
        void make_plane_vbo (Mem2d *mob=NULL, double aspect=1.0){
                // Surface Object Vertices
                BaseGridH = (GLuint)round((double)BaseGridW * aspect);
                
                g_message ("make_plane");
                VertexCount =  BaseGridW*BaseGridH;
                VertexObjectSize = VertexCount * sizeof(glm::vec2);
               
                g_message ("mkplane w=%d h=%d  nvertices=%d", BaseGridW, BaseGridH, VertexCount);
                vertex = g_new (glm::vec2, VertexCount);
                double s_factor = 1.0/(BaseGridW-1);
                double t_factor = 1.0/(BaseGridH-1);
                int offset;
                // surface vertices
                for (guint y=0; y<BaseGridH; ++y){
                        for (guint x=0; x<BaseGridW; ++x){
                                offset = y*BaseGridW+x;
                                double xd = x * s_factor; // 0..1
                                double yd = y * t_factor; // 0..1

                                vertex[offset] = glm::vec2 (xd-0.5, (yd-0.5)*aspect);
                                //g_message ("mkplane vtx -- x=%d y=TEX z=%d  [%d]",x,y, offset);
                        }
                }
                g_message ("mkplane -- Vertex Count=%d", offset);
        
                // Patch Indices
                int i_width = BaseGridW-1;
                int i_height = BaseGridH-1;
                IndicesCount = (((BaseGridW-1)*(BaseGridH-1))*4);
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
                g_message ("mkplane -- Indices Count=%d of %d", ii, IndicesCount);
        };

private:        
        bool Validated;
        glm::vec2 *vertex;
        glf::vertex_v1i *indices;
        GLuint BaseGridW;
        GLuint BaseGridH;
	GLuint ArrayBufferName;
	GLuint IndexBufferName;
	GLuint VertexArrayName;
	GLsizei VertexCount;
	GLsizei IndicesCount;
	GLsizeiptr VertexObjectSize;
	GLsizeiptr IndicesObjectSize;
};

class text_plane{
public:
        text_plane (){
                Validated = true;
                ArrayBufferName = 0;
                VertexArrayName = 0;

                g_message ("text_plane:: init object");

                if(FT_Init_FreeType(&ft)) {
                        g_warning ("Could not init freetype library.");
                }

                // fix me -- find out how to get path or install own??
                if(FT_New_Face(ft, "/usr/share/fonts/truetype/freefont/FreeSans.ttf", 0, &face)) {
                        g_warning ("Could not open font.");
                }

                FT_Set_Pixel_Sizes (face, 0, 48);
                g = face->glyph;

                Validated = init_vao ();
                Validated = init_texture ();

                checkError("text_plane::init");
                g_message ("text_plane:: init object completed");
        };
        ~text_plane (){
		if (Validated){
                        glDeleteTextures(1, &TextTextureName);
                        glDeleteVertexArrays(1, &VertexArrayName);
                        checkError("text_plane::~delete");
                }
        };
        gboolean init_texture (){
                checkError("text_plane:: init_buffer");
		if (!Validated) return false;

                glUseProgram (S3D_ProgramName);

                glActiveTexture (GL_TEXTURE2); // dedicated for text
                glGenTextures (1, &TextTextureName); // done globally
                glBindTexture (GL_TEXTURE_2D, TextTextureName);
                glUniform1i (glGetUniformLocation (S3D_ProgramName, "textTexture"), 2);

                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
                glBindTexture (GL_TEXTURE_2D, 0);
               
                return Validated && checkError("text_plane:: init_buffer completed");
        };
        gboolean init_vao (){
                g_message ("base_plane init_vao");
                checkError("make_plane::init_vao");
		if (!Validated) return false;

                // Build a vertex array object
                glGenVertexArrays(1, &VertexArrayName);
                glBindVertexArray(VertexArrayName);

                glGenBuffers (1, &ArrayBufferName);
                glBindBuffer (GL_ARRAY_BUFFER, ArrayBufferName);
                glVertexAttribPointer(semantic::s3d_text::POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4)+sizeof(glm::vec2), BUFFER_OFFSET(0));
                glVertexAttribPointer(semantic::s3d_text::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4)+sizeof(glm::vec2), BUFFER_OFFSET(sizeof(glm::vec4)));
                glBindBuffer (GL_ARRAY_BUFFER, 0);

                return Validated && checkError("make_plane::init_vao");
        };
        gboolean draw (const char *text, glm::vec3 pos, glm::vec3 ex, glm::vec3 ey) {
 		if (!Validated) return false;

                //g_message ("text_plane::draw  %s", text);

                GLfloat glyp_size = 48.;
                
                ex /= -glyp_size;
                ey /= glyp_size;
                
                glDisable (GL_CULL_FACE);
                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                glUseProgram (S3D_ProgramName);

                glUniform4f (glGetUniformLocation (S3D_ProgramName, "textColor"), 1,0.1,0.5,0.7);

                glBindTexture (GL_TEXTURE_2D, TextTextureName);

                glBindVertexArray(VertexArrayName);
                glBindBuffer (GL_ARRAY_BUFFER, ArrayBufferName);
                glEnableVertexAttribArray (semantic::s3d_text::POSITION);
                glEnableVertexAttribArray (semantic::s3d_text::TEXCOORD);

                checkError("make_plane::draw start");

                for(const char *p = text; *p; p++) {
                        if(FT_Load_Char (face, *p, FT_LOAD_RENDER))
                                continue;

                        glTexImage2D (
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

                        //glm::mat4
                        glm::vec3 pg = pos + (GLfloat)g->bitmap_left*ex + (GLfloat)g->bitmap_top*ey;
                        GLfloat w = g->bitmap.width;
                        GLfloat h = g->bitmap.rows;
                        typedef struct {
                                glm::vec4 p;
                                glm::vec2 t;
                        } glypvert;

                        glypvert box[4] = { // X,Y,Z,r,  Tx,Ty
                                { glm::vec4 (pg,               1), glm::vec2(0,0) },
                                { glm::vec4 (pg + w*ex,        1), glm::vec2(1,0) },
                                { glm::vec4 (pg - h*ey,        1), glm::vec2(0,1) },
                                { glm::vec4 (pg + w*ex - h*ey, 1), glm::vec2(1,1) }
                        };

                        glBindBuffer (GL_ARRAY_BUFFER, ArrayBufferName);
                        glBufferData (GL_ARRAY_BUFFER, sizeof (box), box, GL_DYNAMIC_DRAW);

                        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
                        glBindBuffer (GL_ARRAY_BUFFER, 0);

                        pos += (GLfloat)(g->advance.x/64)*ex + (GLfloat)(g->advance.y/64)*ey;
                }
                glBindBuffer (GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
                glBindTexture(GL_TEXTURE_2D, 0);

                glDisable (GL_BLEND);
                
                return Validated && checkError("text_plane::draw end");
        };
private:        
        bool Validated;
        FT_Library ft;
        FT_Face face;
        FT_GlyphSlot g;
	GLuint ArrayBufferName;
	GLuint VertexArrayName;
        GLuint TextTextureName;
	GLsizei VertexCount;
	GLsizeiptr VertexObjectSize;
	GLsizeiptr IndicesObjectSize;
};

class icosahedron{
public:
        icosahedron (){
                Validated = true;
                VertexArrayName = 0;
                ArrayBufferName = 0;
                IndexBufferName = 0;

                indices = NULL;
                
                g_message ("icosahedron:: init object");

                Validated = init_vao ();
                Validated = init_buffer ();
                Validated = init_texture ();

                checkError("icosahedron::init");
                g_message ("icosahedron:: init object completed");
        };
        ~icosahedron (){
                g_free (indices);
		if (Validated){
                        glDeleteVertexArrays(1, &VertexArrayName);
                        glDeleteBuffers(1, &IndexBufferName);
                        glDeleteBuffers(1, &ArrayBufferName);
                        //glDeleteTextures(1, &TextTextureName);
                        checkError("icosahedron::~delete");
                }
        };
        gboolean init_buffer (){
                checkError("make_plane:: init_buffer");
		if (!Validated) return false;

                VertexCount =  12;
                IndicesCount = 3*20;
                indices = g_new (glf::vertex_v1i, IndicesCount);
 
                // 20 Faces
                const int Faces[3*20] = {
                        2, 1, 0,
                        3, 2, 0,
                        4, 3, 0,
                        5, 4, 0,
                        1, 5, 0,
                        
                        11, 6,  7,
                        11, 7,  8,
                        11, 8,  9,
                        11, 9,  10,
                        11, 10, 6,
                        
                        1, 2, 6,
                        2, 3, 7,
                        3, 4, 8,
                        4, 5, 9,
                        5, 1, 10,
                        
                        2,  7, 6,
                        3,  8, 7,
                        4,  9, 8,
                        5, 10, 9,
                        1, 6, 10 };

                // copy into for sure GL compatible memory
                for (int i=0; i<IndicesCount; ++i)
                        indices[i].indices.x = Faces[i];
                
                const glm::vec4 Verts[12] = {
                        glm::vec4 (0.000f,  0.000f,  1.000f, 1),
                        
                        glm::vec4 (0.894f,  0.000f,  0.447f, 1),
                        glm::vec4 (0.276f,  0.851f,  0.447f, 1),
                        glm::vec4 (-0.724f,  0.526f,  0.447f, 1),
                        glm::vec4 (-0.724f, -0.526f,  0.447f, 1),
                        glm::vec4 (0.276f, -0.851f,  0.447f, 1),
                        
                        glm::vec4 (0.724f,  0.526f, -0.447f, 1),
                        glm::vec4 (-0.276f,  0.851f, -0.447f, 1),
                        glm::vec4 (-0.894f,  0.000f, -0.447f, 1),
                        glm::vec4 (-0.276f, -0.851f, -0.447f, 1),
                        glm::vec4 (0.724f, -0.526f, -0.447f, 1),
                        
                        glm::vec4 (0.000f,  0.000f, -1.000f, 1)
                };
                
                glGenBuffers(1, &ArrayBufferName);
                glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
                glBufferData(GL_ARRAY_BUFFER, VertexCount*sizeof(glm::vec4), Verts, GL_STATIC_DRAW);
                glEnableVertexAttribArray (semantic::attr::POSITION);
                glVertexAttribPointer (semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), BUFFER_OFFSET(0));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glGenBuffers(1, &IndexBufferName);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesCount*sizeof(glf::vertex_v1i), indices, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                return Validated && checkError("make_plane:: init_buffer");
        };
        gboolean init_texture (){
                checkError("icosahedron:: init_texture");
		if (!Validated) return false;

                //glUseProgram (S3D_ProgramName);
              
                return Validated && checkError("text_plane:: init_buffer completed");
        };
        gboolean init_vao (){
                g_message ("icosahedron init_vao");
                checkError("icosahedron::init_vao");
		if (!Validated) return false;

                // Build a vertex array object
                glGenVertexArrays(1, &VertexArrayName);
                glBindVertexArray(VertexArrayName);

                return Validated && checkError("make_plane::init_vao");
        };
        // vec4: (x,y,z, scale)
        gboolean draw (glm::vec4 pos_s, glm::vec4 color, gfloat tesslevel=2.) {
 		if (!Validated) return false;

                g_message ("icosahedron draw");

                //glDisable (GL_CULL_FACE);
                //glEnable (GL_BLEND);
                //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                glUseProgram (IcoTess_ProgramName);

                glUniform1f (Uniform_IcoTess_TessLevelInner, tesslevel);
                glUniform1f (Uniform_IcoTess_TessLevelOuter, tesslevel);  
                glUniform4fv (Uniform_IcoTess_IcoPositionS, 1, &(pos_s.x));
                glUniform4fv (Uniform_IcoTess_IcoColor, 1, &(color.x));

                //glBindTexture (GL_TEXTURE_2D, TextTextureName);

                glBindVertexArray (VertexArrayName); // VAO
                glBindBuffer (GL_ARRAY_BUFFER, ArrayBufferName);
                glEnableVertexAttribArray (semantic::attr::POSITION);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);

                glPatchParameteri (GL_PATCH_VERTICES, 3);
                glDrawElements (GL_PATCHES, IndicesCount, GL_UNSIGNED_INT, 0);
 
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
                glBindBuffer (GL_ARRAY_BUFFER, 0);
                glBindVertexArray (0);

                //glBindTexture(GL_TEXTURE_2D, 0);
                //glDisable (GL_BLEND);
                
                return Validated && checkError("icosahedron::draw end");
        };
private:
        glf::vertex_v1i *indices;
        bool Validated;
	GLuint ArrayBufferName;
	GLuint IndexBufferName;
	GLuint VertexArrayName;
	GLsizei VertexCount;
	GLsizei IndicesCount;
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

                TranslationOrigin  = glm::vec2(0, 0*aspect);
              	TranslationCurrent = TranslationOrigin;
                Translation3axis = glm::vec3(0.0f,0.0f,0.0f);

                DistanceOrigin  = glm::vec3(-60., 0., 0.);
              	DistanceCurrent = DistanceOrigin;


                RotationOrigin = glm::vec2(0.75,0);
                RotationCurrent = RotationOrigin;
                Rotation3axis = glm::vec3(0.0f,0.0f,0.0f);

                WindowSize  = glm::ivec2(500, 500);
                surface_plane = NULL;
                text_vao = NULL;
                ico_vao = NULL;
        };
        ~gl_400_primitive_tessellation(){
                //end (); // too late, glarea reference is gone! 
                if (surface_plane)
                        delete surface_plane;
                if (text_vao)
                        delete text_vao;
                if (ico_vao)
                        delete ico_vao;
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
                        GLuint VertexShader     = Compiler.create (GL_VERTEX_SHADER, getDataDirectory() + TESSELATION_VERTEX_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint ControlShader    = Compiler.create (GL_TESS_CONTROL_SHADER, getDataDirectory() + TESSELATION_CONTROL_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint EvaluationShader = Compiler.create (GL_TESS_EVALUATION_SHADER, getDataDirectory() + TESSELATION_EVALUATION_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint GeometryShader   = Compiler.create (GL_GEOMETRY_SHADER, getDataDirectory() + TESSELATION_GEOMETRY_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint FragmentShader   = Compiler.create (GL_FRAGMENT_SHADER, getDataDirectory() + TESSELATION_FRAGMENT_SHADER, CMD_ARGS_FOR_SHADERS);

                        if (!VertexShader || !ControlShader || !EvaluationShader || !GeometryShader || !FragmentShader)
                                Validated = false;
                        else {
                                Tesselation_ProgramName = glCreateProgram ();
                                glAttachShader (Tesselation_ProgramName, VertexShader);
                                glAttachShader (Tesselation_ProgramName, ControlShader);
                                glAttachShader (Tesselation_ProgramName, EvaluationShader);
                                glAttachShader (Tesselation_ProgramName, GeometryShader);
                                glAttachShader (Tesselation_ProgramName, FragmentShader);
                                glLinkProgram (Tesselation_ProgramName);
                        }
                        
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.check_program(Tesselation_ProgramName);
		}

 		if (Validated){
                        compiler Compiler;
                        GLuint VertexShader     = Compiler.create (GL_VERTEX_SHADER, getDataDirectory() + S3D_VERTEX_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint FragmentShader   = Compiler.create (GL_FRAGMENT_SHADER, getDataDirectory() + S3D_FRAGMENT_SHADER, CMD_ARGS_FOR_SHADERS);

                        if (!VertexShader || !FragmentShader)
                                Validated = false;
                        else {
                                S3D_ProgramName = glCreateProgram ();
                                glAttachShader (S3D_ProgramName, VertexShader);
                                glAttachShader (S3D_ProgramName, FragmentShader);
                                glLinkProgram (S3D_ProgramName);
                        }
                        
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.check_program(S3D_ProgramName);
		}

		if (Validated){
                        compiler Compiler;
                        GLuint VertexShader     = Compiler.create (GL_VERTEX_SHADER, getDataDirectory() + ICO_TESS_VERTEX_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint ControlShader    = Compiler.create (GL_TESS_CONTROL_SHADER, getDataDirectory() + ICO_TESS_CONTROL_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint EvaluationShader = Compiler.create (GL_TESS_EVALUATION_SHADER, getDataDirectory() + ICO_TESS_EVALUATION_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint GeometryShader   = Compiler.create (GL_GEOMETRY_SHADER, getDataDirectory() + ICO_TESS_GEOMETRY_SHADER, CMD_ARGS_FOR_SHADERS);
                        GLuint FragmentShader   = Compiler.create (GL_FRAGMENT_SHADER, getDataDirectory() + ICO_TESS_FRAGMENT_SHADER, CMD_ARGS_FOR_SHADERS);

                        if (!VertexShader || !ControlShader || !EvaluationShader || !GeometryShader || !FragmentShader)
                                Validated = false;
                        else {
                                IcoTess_ProgramName = glCreateProgram ();
                                glAttachShader (IcoTess_ProgramName, VertexShader);
                                glAttachShader (IcoTess_ProgramName, ControlShader);
                                glAttachShader (IcoTess_ProgramName, EvaluationShader);
                                glAttachShader (IcoTess_ProgramName, GeometryShader);
                                glAttachShader (IcoTess_ProgramName, FragmentShader);
                                glLinkProgram (IcoTess_ProgramName);
                        }
                        
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.check_program(IcoTess_ProgramName);
		}

                

		if(Validated){
                        Uniform_screen_size   = glGetUniformLocation (Tesselation_ProgramName, "screen_size"); // vec2
                        Uniform_lod_factor    = glGetUniformLocation (Tesselation_ProgramName, "lod_factor");  // float
                        Uniform_tess_level    = glGetUniformLocation (Tesselation_ProgramName, "tess_level");  // float

                        checkError("initProgram -- Tesselation_ProgramName: get uniform variable references...");

                        Uniform_IcoTess_TessLevelInner = glGetUniformLocation (IcoTess_ProgramName, "TessLevelInner");
                        Uniform_IcoTess_TessLevelOuter = glGetUniformLocation (IcoTess_ProgramName, "TessLevelOuter");
                        Uniform_IcoTess_IcoPositionS   = glGetUniformLocation (IcoTess_ProgramName, "IcoPositionS");
                        Uniform_IcoTess_IcoColor       = glGetUniformLocation (IcoTess_ProgramName, "IcoColor");

                        Uniform_TipPosition  = glGetUniformLocation (IcoTess_ProgramName, "TipPosition");
                        Uniform_TipColor     = glGetUniformLocation (IcoTess_ProgramName, "TipColor");

                        checkError("initProgram -- IcoTess_ProgramName: get uniform variable references...");

                        // get shaderFunction references
                        // Specifies the shader stage from which to query for subroutine uniform index. shadertype must be one of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER.

                        Uniform_vertexFlat        = glGetSubroutineIndex (Tesselation_ProgramName, GL_VERTEX_SHADER, "vertex_height_flat" );
                        Uniform_vertexDirect      = glGetSubroutineIndex (Tesselation_ProgramName, GL_VERTEX_SHADER, "vertex_height_direct" );
                        Uniform_vertexViewMode    = glGetSubroutineIndex (Tesselation_ProgramName, GL_VERTEX_SHADER, "vertex_height_z" );
                        Uniform_vertexY           = glGetSubroutineIndex (Tesselation_ProgramName, GL_VERTEX_SHADER, "vertex_height_y" );
                        Uniform_vertexXChannel    = glGetSubroutineIndex (Tesselation_ProgramName, GL_VERTEX_SHADER, "vertex_height_x" );

                        Uniform_evaluationVertexFlat     = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "height_flat" );
                        Uniform_evaluationVertexDirect   = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "height_direct" );
                        Uniform_evaluationVertexXChannel = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "height_z" );
                        Uniform_evaluationVertexY        = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "height_y" );
                        Uniform_evaluationVertexViewMode = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "height_x" );
                        
                        Uniform_evaluationColorFlat      = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "colorFlat" );
                        Uniform_evaluationColorDirect    = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "colorDirect" );
                        Uniform_evaluationColorViewMode  = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "colorViewMode" );
                        Uniform_evaluationColorXChannel  = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "colorXChannel" );
                        Uniform_evaluationColorY         = glGetSubroutineIndex (Tesselation_ProgramName, GL_TESS_EVALUATION_SHADER, "colorY" );

                        Uniform_shadeTerrain      = glGetSubroutineIndex (Tesselation_ProgramName, GL_FRAGMENT_SHADER, "shadeTerrain" );
                        Uniform_shadeDebugMode    = glGetSubroutineIndex (Tesselation_ProgramName, GL_FRAGMENT_SHADER, "shadeDebugMode" );
                        Uniform_shadeLambertian   = glGetSubroutineIndex (Tesselation_ProgramName, GL_FRAGMENT_SHADER, "shadeLambertian" );

#if 0
                        Uniform_S3D_vertexDirect    = glGetSubroutineIndex (S3D_ProgramName, GL_VERTEX_SHADER, "vertexDirect" );
                        Uniform_S3D_vertexSurface   = glGetSubroutineIndex (S3D_ProgramName, GL_VERTEX_SHADER, "vertexSurface" );
                        Uniform_S3D_vertexHScaled   = glGetSubroutineIndex (S3D_ProgramName, GL_VERTEX_SHADER, "vertexHScaled" );
                        Uniform_S3D_vertexText      = glGetSubroutineIndex (S3D_ProgramName, GL_VERTEX_SHADER, "vertexText" );

                        Uniform_S3D_shadeLambertian = glGetSubroutineIndex (S3D_ProgramName, GL_FRAGMENT_SHADER, "shadeLambertian" );
                        Uniform_S3D_shadeText       = glGetSubroutineIndex (S3D_ProgramName, GL_FRAGMENT_SHADER, "shadeText" );
#endif

                        checkError("initProgram -- get uniform subroutine references...");
		}

                if (Validated)
                        return Validated && checkError("initProgram");

		return Validated;
	};

        void updateModelViewM(){
                glBindBuffer (GL_UNIFORM_BUFFER, Uniform_ubo_list[ModelViewMat_block]);
                glBufferData (GL_UNIFORM_BUFFER, sizeof (ubo::uniform_model_view), &Block_ModelViewMat, GL_STATIC_DRAW);
                glBindBuffer (GL_UNIFORM_BUFFER, 0);
        };
        
        void updateSurfaceGeometry(){
                glBindBuffer (GL_UNIFORM_BUFFER,  Uniform_ubo_list[SurfaceGeometry_block]);
                glBufferData (GL_UNIFORM_BUFFER, sizeof (ubo::uniform_surface_geometry), &Block_SurfaceGeometry, GL_STATIC_DRAW);
                glBindBuffer (GL_UNIFORM_BUFFER, 0);
        };
        
        void updateFragmentShading(){
                glBindBuffer (GL_UNIFORM_BUFFER,  Uniform_ubo_list[FragmentShading_block]);
                glBufferData (GL_UNIFORM_BUFFER, sizeof (ubo::uniform_fragment_shading), &Block_FragmentShading, GL_STATIC_DRAW);
                glBindBuffer (GL_UNIFORM_BUFFER, 0);
        };
        
	void bind_block (GLuint program, GLuint block_id, const gchar* block_name, GLsizei block_size){
                GLuint uniformBlockIndexProg = glGetUniformBlockIndex (program, block_name);
                glUniformBlockBinding (program, uniformBlockIndexProg, block_id);
                glBindBufferRange(GL_UNIFORM_BUFFER, block_id, Uniform_ubo_list[block_id], 0, block_size);
                g_message ("UBO[%d] (%s) = %d <=> %d  {%u}", block_id,  block_name, uniformBlockIndexProg, Uniform_ubo_list[block_id], block_size);
        };

	bool initBuffer() {
		if (!Validated) return false;

                // setup common UBOs
                
                glGenBuffers(3, &Uniform_ubo_list[0]);
                updateModelViewM ();
                updateSurfaceGeometry ();
                updateFragmentShading ();

                // interlink UBOs
                bind_block (Tesselation_ProgramName, ModelViewMat_block, "ModelViewMatrices", sizeof(ubo::uniform_model_view));
                bind_block (Tesselation_ProgramName, SurfaceGeometry_block, "SurfaceGeometry", sizeof(ubo::uniform_surface_geometry));
                bind_block (Tesselation_ProgramName, FragmentShading_block, "FragmentShading", sizeof(ubo::uniform_fragment_shading));

                bind_block (S3D_ProgramName, ModelViewMat_block, "ModelViewMatrices", sizeof(ubo::uniform_model_view));
                bind_block (S3D_ProgramName, SurfaceGeometry_block, "SurfaceGeometry", sizeof(ubo::uniform_surface_geometry));
                bind_block (S3D_ProgramName, FragmentShading_block, "FragmentShading", sizeof(ubo::uniform_fragment_shading));
                
                // create surface base plane
                surface_plane = new base_plane ((s->get_scan ())->mem2d, 128, // 128,
                                                (s->get_scan ())->data.s.ry/(s->get_scan ())->data.s.rx
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
                glUseProgram (Tesselation_ProgramName);

                glGenTextures (TesselationTextureCount, TesselationTextureName);

                // sampler2D Surf3D_Z-Data vec4[][]
                glActiveTexture (GL_TEXTURE0);
                glBindTexture (GL_TEXTURE_2D, TesselationTextureName[0]);

                glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, numx, numy, 0, GL_RGBA, GL_FLOAT, Surf3D_Z_Data);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap (GL_TEXTURE_2D);
                glUniform1i (glGetUniformLocation (Tesselation_ProgramName, "Surf3D_Z_Data"), 0);

                // sampler1D GXSM_Palette vec4[][]
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, TesselationTextureName[1]);

                glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, GXSM_GPU_PALETTE_ENTRIES, 0, GL_RGBA, GL_FLOAT, Surf3D_Palette);
                glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap(GL_TEXTURE_1D);
                glUniform1i (glGetUniformLocation (Tesselation_ProgramName, "GXSM_Palette"), 1);

                glBindTexture(GL_TEXTURE_2D, 0);
                
		checkError("initTextures");
		if (!Validated) return false;

#if 1
                if (!text_vao)
                        text_vao = new text_plane ();

                if (!ico_vao)
                        ico_vao = new icosahedron ();
#endif
                
		return checkError("initText Plane VAO");
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
                
                glDeleteTextures(TesselationTextureCount, TesselationTextureName);
                glDeleteProgram(Tesselation_ProgramName);
                glDeleteProgram(S3D_ProgramName);

		return checkError("end");
	};

        // used for height mapping
        bool updateTexture (GLint line, GLsizei num_lines=1) { 
		if (!Validated) return false;

                glTextureSubImage2D(TesselationTextureName[0], 0, 0, line, numx, num_lines, GL_RGBA, GL_FLOAT, &Surf3D_Z_Data[line*numx]);
                // update palette -- only on request
                //glTextureSubImage1D(TesselationTextureName[1], 0, 0, GXSM_GPU_PALETTE_ENTRIES, GL_RGBA, GL_FLOAT, &Surf3D_Palette);

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

                glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClearBufferfv (GL_COLOR, 0, s->GLv_data.clear_color);
                glEnable (GL_DEPTH_TEST);

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
		Block_ModelViewMat.ModelView = this->modelView() * Model;
		Block_ModelViewMat.ModelViewProjection = Projection * Camera * Block_ModelViewMat.ModelView;

                /* clear the viewport; the viewport is automatically resized when
                 * the GtkGLArea gets a new size allocation
                 */

		glUseProgram (Tesselation_ProgramName);

                // Projection
                updateModelViewM ();

                // Geometry control
                Block_SurfaceGeometry.aspect = (s->get_scan ())->data.s.ry / (s->get_scan ())->data.s.rx;
                Block_SurfaceGeometry.height_scale  = s->GLv_data.hskl;
                Block_SurfaceGeometry.height_offset = 0.; // s->GLv_data.slice_offset;
                Block_SurfaceGeometry.delta = glm::vec2 (1.0f/(s->XPM_x-1), 1.0f/(s->XPM_y-1));
                updateSurfaceGeometry ();

                // Camera, Light and Shading
#define MAKE_GLM_VEC3(V) glm::vec4(V[0],V[1],V[2],0)
#define MAKE_GLM_VEC4(V) glm::vec4(V[0],V[1],V[2],V[3])
#define MAKE_GLM_VEC4A(O,A) glm::vec4(O,O,O,A)
                Block_FragmentShading.lightDirWorld = MAKE_GLM_VEC3(s->GLv_data.light_position[0]);
                Block_FragmentShading.eyePosWorld   = glm::vec4(cameraPosition(),0);

                Block_FragmentShading.sunColor      = MAKE_GLM_VEC3(s->GLv_data.light_specular[0]); // = vec3(1.0, 1.0, 0.7);
                Block_FragmentShading.specularColor = MAKE_GLM_VEC3(s->GLv_data.surf_mat_specular); // = vec3(1.0, 1.0, 0.7)*1.5;
                Block_FragmentShading.ambientColor  = MAKE_GLM_VEC3(s->GLv_data.light_global_ambient); // = vec3(0.05, 0.05, 0.15 );
                //Block_FragmentShading.ambientColor  = MAKE_GLM_VEC3(s->GLv_data.surf_mat_ambient); // = vec3(1.0, 1.0, 0.7)*1.5;
                Block_FragmentShading.diffuseColor  = MAKE_GLM_VEC3(s->GLv_data.surf_mat_diffuse); // = vec3(1.0, 1.0, 0.7)*1.5;
                Block_FragmentShading.fogColor      = MAKE_GLM_VEC3(s->GLv_data.fog_color); // = vec3(0.7, 0.8, 1.0)*0.7;
                Block_FragmentShading.materialColor = MAKE_GLM_VEC4(s->GLv_data.surf_mat_color); // vec4
                Block_FragmentShading.color_offset  = MAKE_GLM_VEC4A(s->GLv_data.ColorOffset, s->GLv_data.transparency_offset);

                Block_FragmentShading.fogExp = s->GLv_data.fog_density/100.; // = 0.1;

                Block_FragmentShading.shininess = 4.*(1.00001-s->GLv_data.surf_mat_shininess[0]/100.);
                Block_FragmentShading.lightness = s->GLv_data.ColorContrast;
                Block_FragmentShading.light_attenuation = 0.;

                Block_FragmentShading.transparency        = s->GLv_data.transparency;
                Block_FragmentShading.transparency_offset = s->GLv_data.transparency_offset;

                Block_FragmentShading.wrap = 0.3; // = 0.3;
                Block_FragmentShading.debug_color_source = (GLuint)s->GLv_data.shader_mode;
                updateFragmentShading ();

                // Tesseleation control -- lod is not yet used
                glUniform1f (Uniform_lod_factor, 4.0);
                glUniform1f (Uniform_tess_level, s->GLv_data.tess_level);
                
                checkError ("render -- set Uniforms, Blocks");

                // Specifies the shader stage from which to query for subroutine uniform index. shadertype must be one of GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER.

                // configure shaders for surface rendering modes (using tesselation) and select final shading mode
                // s->GLv_data.vertex_source[0] = 'D';
                switch (s->GLv_data.vertex_source[0]){
                case 'F': // Flat Vertex Height
                        glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexFlat);
                        Uniform_evaluation_setup[0] = Uniform_evaluationVertexFlat;
                        break;
                case 'D': // Direct Vertex Height
                        glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexDirect);
                        Uniform_evaluation_setup[0] = Uniform_evaluationVertexDirect;
                        break;
                case 'V': // View Mode Vertex Height
                        glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexViewMode);
                        Uniform_evaluation_setup[0] = Uniform_evaluationVertexViewMode;
                        break;
                case 'X': // X-Channel Direct Vertex Height
                        glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexXChannel);
                        Uniform_evaluation_setup[0] = Uniform_evaluationVertexXChannel;
                        break;
                case 'Y': // Y
                        glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexY);
                        Uniform_evaluation_setup[0] = Uniform_evaluationVertexY;
                        break;
                default: // fallback
                        glUniformSubroutinesuiv (GL_VERTEX_SHADER, 1, &Uniform_vertexDirect);
                        Uniform_evaluation_setup[0] = Uniform_evaluationVertexDirect;
                        break;
                }
                checkError ("render -- configure vertex_source");

                switch (s->GLv_data.ColorSrc[0]){
                case 'F': Uniform_evaluation_setup[1] = Uniform_evaluationColorFlat; break;
                case 'D': Uniform_evaluation_setup[1] = Uniform_evaluationColorDirect; break; // Direct Height Color
                case 'V': Uniform_evaluation_setup[1] = Uniform_evaluationColorViewMode; break; // View Mode Color
                case 'X': Uniform_evaluation_setup[1] = Uniform_evaluationColorXChannel; break; // X-Channel Direct Height Color
                case 'Y': Uniform_evaluation_setup[1] = Uniform_evaluationColorY; break; // X-Channel Direct Height Color
                default:  Uniform_evaluation_setup[1] = Uniform_evaluationColorDirect; break; // direct -- fall back
                }
                glUniformSubroutinesuiv (GL_TESS_EVALUATION_SHADER, 2, Uniform_evaluation_setup);

                checkError ("render -- configure color_source");
                
                switch (s->GLv_data.ShadeModel[0]){
		case 'T': glUniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &Uniform_shadeTerrain); break;
		case 'D': glUniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &Uniform_shadeDebugMode); break;
                default: glUniformSubroutinesuiv (GL_FRAGMENT_SHADER, 1, &Uniform_shadeLambertian); break;
                }
                checkError ("render -- configure shader");
                
                surface_plane->draw ();


                if (ico_vao && s->GLv_data.light[2][1] == 'n'){
                        checkError ("render -- draw ico");
                        glm::vec4 c=MAKE_GLM_VEC3(s->GLv_data.light_specular[2]);

                        double r[3];
                        s->GetXYZNormalized (r);
                        g_message ("R=%g %g %g", r[0], r[1], r[2]);
                        
                        //glm::vec4 Ra=glm::vec4 (0.,0.7,0., 0.2);
                        //ico_vao->draw (Ra,c,4);

                        glm::vec4 R=glm::vec4 (r[0],r[1],r[2], 0.1);
                        ico_vao->draw (R,c,4);
                }
                
#define MAKE_GLM_VEC3X(V) glm::vec3(V[0],V[1],V[2])
                if (text_vao && s->GLv_data.light[1][1] == 'n'){
                        checkError ("render -- draw text");
                        //text_vao->draw ("GXSM-3.0", MAKE_GLM_VEC3X(s->GLv_data.light_position[1]), glm::vec3 (0,1,0), glm::vec3 (-0.1, 0.1, 0.1));
                        glm::vec3 ex=glm::vec3 (0.1,0,0);
                        glm::vec3 ey=glm::vec3 (0,0.1,0);
                        glm::vec3 ez=glm::vec3 (0,0,0.1);
                        text_vao->draw ("GXSM-3.0 GL4.0", glm::vec3(0.5, -0.1, -0.75), 1.25f*ex, 1.25f*ez);
                        text_vao->draw ("X axis", glm::vec3(0, 0, -0.6), ex, ez);
                        text_vao->draw ("Y axis", glm::vec3(0.53, 0, 0), -ez, ex);
                        text_vao->draw ("Z axis", glm::vec3(0.53, 0, -0.6), -ey, 0.5f*(ex+ez));
                        text_vao->draw ("x -->", glm::vec3(0, 0, -0.5), ex, ey);
                        text_vao->draw ("<-- X top", glm::vec3(0, 0,  0.5), -ex, ey); 
                        text_vao->draw ("y -->", glm::vec3(0.5, 0, 0), ez, ey);
                        text_vao->draw ("Y right", glm::vec3(-0.5, 0, 0), -ez, ey);
                }
                
		return checkError("render");
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
                wxyz[1] = -dr*RotationCurrent.y;
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

        void set_surface_data (glm::vec4 *data, glm::vec4 *palette, int nx, int ny, int nv){
                Surf3D_Z_Data = data;
                Surf3D_Palette = palette;
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
        text_plane *text_vao;
        icosahedron *ico_vao;

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

        glm::vec4 *Surf3D_Z_Data;
        glm::vec4 *Surf3D_Palette;
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


double Surf3d::GetXYZNormalized(double *r){
        double x,y,z, za; 
        gapp->xsm->hardware->RTQuery ("zxy", z, x, y);
        za = z;
        z = gapp->xsm->Inst->VoltIn2Dig(z) / gapp->xsm->Inst->VZ();
        z = (z-scan->mem2d->data->zmin)/scan->mem2d->data->zrange;
        x = gapp->xsm->Inst->VoltIn2Dig(x) / gapp->xsm->Inst->VX();
        y = gapp->xsm->Inst->VoltIn2Dig(y) / gapp->xsm->Inst->VY();
        x = (x-scan->data.s.x0)/scan->data.s.rx;
        y = (y-scan->data.s.y0)/scan->data.s.ry;
        r[0] = x;
        r[1] = y;
        r[2] = z;
        return za;
}

double Surf3d::GetCurrent(){
        return 0.;
}
double Surf3d::GetForce(){
        return 0.;
}

void inline Surf3d::PutPointMode(int k, int j, int vi){
	int i;
	GLfloat val, xval;
        
	i = k + j*XPM_x + XPM_x*XPM_y*vi;
	if (i >= (int)size) return;

        // check for zero range
        if (! (scan->mem2d->data->zrange > 0.)){
                surface_z_data_buffer[i].x=0.;
                surface_z_data_buffer[i].y=0.;
                surface_z_data_buffer[i].z=0.;
                surface_z_data_buffer[i].w=0.;
                return;
        }

        // W CHANNEL <= surface height raw 0..1 adjusted 
        surface_z_data_buffer[i].w = (scan->mem2d->GetDataPkt(k,j,vi)-scan->mem2d->data->zmin)/scan->mem2d->data->zrange; // W component: Z=Height-value raw, scaled to 0..1

        // Z CHANNEL <= surface height via view transform function
        val = scan->mem2d->GetDataVMode (k,j,vi)/scan->mem2d->GetDataRange ();
        surface_z_data_buffer[i].z = val >= 0. ? val <= 1. ? val : 1. : 0.; // Clamp Z via view transform function

        // normal calculation on GPU based on height map -- obsolete now here
	// scan->mem2d->GetDataPkt_vec_normal_4F (k,j,vi, &surface_z_data_buffer[i], 1.0, XPM_x*GLv_data.hskl/scan->mem2d->data->zrange);

        // X CHANNEL <= UV-Map to index range of GXSM SCAN CHANNEL-X:
        if (mem2d_x){
                int u = (int) (k * (double)mem2d_x->GetNx () / (double)scan->mem2d->GetNx ());
                int v = (int) (j * (double)mem2d_x->GetNy () / (double)scan->mem2d->GetNy ());
                xval = (GLfloat) (mem2d_x->GetDataVMode (u,v)/mem2d_x->GetDataRange ());
        } else {
                xval = 0.5;
        }
        surface_z_data_buffer[i].x = xval >= 0. ? xval <= 1. ? xval : 1. : 0.; // X-Channel val in X component
        surface_z_data_buffer[i].y = val; // Y-Channel still available

        // push edges to zero?
        if (GLv_data.TickFrameOptions[0]=='2' || GLv_data.TickFrameOptions[0]=='3' ){
                if (k==0 || j==0 || k == XPM_x-1 || j == XPM_y-1){
                        surface_z_data_buffer[i].w = 0.;
                        surface_z_data_buffer[i].x = 0.;
                        surface_z_data_buffer[i].y = 0.;
                        surface_z_data_buffer[i].z = 0.;
                }
        }
        
        if (XPM_v > 1){
                // adjust slice height level, renormalize
                surface_z_data_buffer[i].w /= XPM_v;
                surface_z_data_buffer[i].w += (GLfloat)vi*GLv_data.slice_offset*0.5/XPM_v;
                surface_z_data_buffer[i].x /= XPM_v;
                surface_z_data_buffer[i].x += (GLfloat)vi*GLv_data.slice_offset*0.5/XPM_v;
                surface_z_data_buffer[i].y /= XPM_v;
                surface_z_data_buffer[i].y += (GLfloat)vi*GLv_data.slice_offset*0.5/XPM_v;
                surface_z_data_buffer[i].z /= XPM_v;
                surface_z_data_buffer[i].z += (GLfloat)vi*GLv_data.slice_offset*0.5/XPM_v;
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
        surface_z_data_buffer = NULL;
        
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
	gnome_res_set_auto_apply (v3dControl_pref_dlg, TRUE);
	gnome_res_set_height (v3dControl_pref_dlg, 700);
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
                gl_tess->set_surface_data (surface_z_data_buffer, ColorLookup, XPM_x, XPM_y, XPM_v);
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
        
	surface_z_data_buffer = g_new (glm::vec4, size * sizeof(glm::vec4));

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
}

void Surf3d::delete_surface_buffer(){ 
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D::delete_surface_buffer -- check for cleanup");

	if (surface_z_data_buffer){
                XSM_DEBUG (GL_DEBUG_L2, "SURF3D::delete_surface_buffer -- g_free: normal_z_buffer");
		g_free (surface_z_data_buffer);
                surface_z_data_buffer = NULL;
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
                gl_tess->get_distance (&GLv_data.dist);
                v3dControl_pref_dlg->block = TRUE;
                gnome_res_update_all (v3dControl_pref_dlg);
                v3dControl_pref_dlg->block = FALSE;
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
	if (v3dcontrol){
                v3dcontrol->rerender_scene ();
                v3dControl_pref_dlg->block = TRUE;
                gnome_res_update_all (v3dControl_pref_dlg);
                v3dControl_pref_dlg->block = FALSE;
        }
	return GLv_data.dist; 
}

double Surf3d::HeightSkl(double x){ 
	if (v3dcontrol){
                GLv_data.hskl += x; 
                v3dControl_pref_dlg->block = TRUE;
                gnome_res_update_all (v3dControl_pref_dlg);
                v3dControl_pref_dlg->block = FALSE;
                draw();
        }
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
