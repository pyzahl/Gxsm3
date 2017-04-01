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

//#define __GXSM_PY_DEVEL
#ifdef __GXSM_PY_DEVEL
#define GLSL_DEV_DIR "/home/pzahl/SVN/Gxsm-3.0/gl-400/"
//#define GLSL_DEV_DIR "/home/percy/SVN/Gxsm-3.0/gl-400/"
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
        GLsizei const TextureCount(2);
	GLuint TextureName[2];
        
        // Model View and Projection
	GLint Uniform_ModelViewProjection(0); // mat4
	GLint Uniform_ModelView(0); // mat4

        // Lightening
        GLint Uniform_lightDirWorld(0); // vec3
        GLint Uniform_eyePosWorld(0);   // vec3

        GLint Uniform_sunColor(0); // = vec3(1.0, 1.0, 0.7);
        GLint Uniform_lightColor(0); // = vec3(1.0, 1.0, 0.7)*1.5;
        GLint Uniform_fogColor(0); // = vec3(0.7, 0.8, 1.0)*0.7;

        GLint Uniform_fogExp(0); // = 0.1;

        GLint Uniform_shininess(0); // = 100.0;
        GLint Uniform_ambientColor(0); // = vec3(0.05, 0.05, 0.15 );
	GLint Uniform_delta(0); // vec2 float -- for normal computation, should be 1/NX, 1/NY
        GLint Uniform_wrap(0); // = 0.3;

} //namespace

class base_plane{
public:
        base_plane (Mem2d *m=NULL, int w=128, double aspect=1.0, int o=0){
                Validated = true;
                BaseGridW = w;
                BaseGridH = w; // adjusted by make_plane_vbo using aspect
                ArrayBufferName = 0;
                VertexArrayName = 0;

                make_plane_vbo (m, aspect, o);

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
        gboolean draw (){
		if (!Validated) return false;
                
                glBindVertexArray(VertexArrayName);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferName);
                glPatchParameteri(GL_PATCH_VERTICES, 4);

                //if (s->GLv_data.TickFrameOptions[0]=='2' || s->GLv_data.TickFrameOptions[0]=='3') // surface with box
                glDrawElements(GL_PATCHES, IndicesCount, GL_UNSIGNED_INT, 0);
                //else
                // glDrawElements(GL_PATCHES, IndicesCountSurface, GL_UNSIGNED_INT, 0); // surface only

                return Validated && checkError("make_plane::draw");
        };
        
        // make plane with box
        void make_plane_vbo (Mem2d *mob=NULL, double aspect=1.0, int option=0){
                // Surface Object Vertices
                option=0;

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
                        for (guint x=0; x<BaseGridW; ++x){
                                offset = y*BaseGridW+x;
                                glm::vec4 normal_z (0.0f,0.0f,1.0f,0.0f);
                                double xd = x * s_factor;
                                double yd = y * t_factor;
                                double vd = 0.; // vi
                                if (mob)
                                        mob->GetDataPkt_vec_normal_4F (xd*mob->GetNx(), yd*mob->GetNy(), vd, &normal_z, 1.); //GLv_data.hskl);
                                //g_message ("mkplane vtx -- x=%d y=%d   [%d]",x,y, offset);
                                vertex[offset].Position = glm::vec3 (-0.5+xd, -(0.5+yd)*aspect, normal_z.w);
                                vertex[offset].Normals  = glm::vec3 (normal_z.x, normal_z.y, normal_z.z);
                                vertex[offset].Color    = glm::vec4 (normal_z.w, normal_z.w, normal_z.w, 1.0);
                        }
                }

#if 0
                if (option){
                        // box bottom z=0 vertices
                        //g_message ("mkplane -- vtx box");
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
                                        //g_message ("mkplane vtx -- x=%d y=%d   [%d]",x,y, offset);
                                        double xd = x * s_factor;
                                        double yd = y * t_factor;
                                        vertex[offset].Position = glm::vec3 (-0.5+xd, -0.5+yd, -1.0); // box z=0 base
                                        vertex[offset].Normals  = normal; // fix corner normals!
                                        vertex[offset].Color    = glm::vec4 (GLv_data.box_mat_color[0], GLv_data.box_mat_color[1], GLv_data.box_mat_color[2], GLv_data.box_mat_color[3]);
                                        ++offset;
                                }
                        }
                }
#endif
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
#if 0
                if (option){
                        g_message ("mkplane -- idx box");
                        // box patches
                        int pb0=BaseGridW*BaseGridH;
                        for (int s=0; s<4; ++s){
                                //g_message ("mkplane -- idx s=%d",s);
                                int num = s%2 ? i_width : i_height;
                                for (int l=0; l<num; ++l){
                                        int x = s%2==0 ? l : s==1 ? BaseGridW-1:0;
                                        int y = s%2==0 ? s==0 ? 0:BaseGridH-1 : l;
                                        int p1 = x+y*BaseGridW;
                                        int p2 = pb0+l;
                                        int p3 = p2+1;
                                        int p4 = p1+(s%2==0 ? 1:num);
                                        //g_message ("mkplane idx -- x=%d y=%d  p: %d %d %d %d",x,y, p1,p2,p3,p4);
                                        indices[ii++].indices.x = p1;
                                        indices[ii++].indices.x = p2;
                                        indices[ii++].indices.x = p3;
                                        indices[ii++].indices.x = p4;
                                }
                                pb0+=num;
                        }
                        g_message ("mkplane -- idx floor");
                        // box floor
                        int pbf=pb0;
                        indices[ii++].indices.x = pbf; pbf+=BaseGridW;
                        indices[ii++].indices.x = pbf; pbf+=BaseGridH;
                        indices[ii++].indices.x = pbf; pbf+=BaseGridW;
                        indices[ii++].indices.x = pbf;
                }
#endif
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

                TranslationOrigin  = glm::vec2(0, -0.5);
              	TranslationCurrent = TranslationOrigin;
                Translation3axis = glm::vec3(0.0f,0.0f,0.0f);

                DistanceOrigin  = glm::vec3(10., -40., 0.);
              	DistanceCurrent = DistanceOrigin;


                RotationOrigin = glm::ivec2(0,0);
                RotationCurrent = RotationOrigin;
                Rotation3axis = glm::vec3(0.0f,0.0f,0.0f);

                WindowSize  = glm::ivec2(500, 500);
                surface_plane = NULL;
        };
        ~gl_400_primitive_tessellation(){
                end ();
        };

private:
        
        glm::vec3 cameraPosition() const {
                return glm::vec3(0., this->DistanceCurrent.x, this->DistanceCurrent.y);
        };
        glm::vec3 modelPosition() const {
                return glm::vec3(-this->TranslationCurrent.x, -this->TranslationCurrent.y, 0.);
        };
        glm::mat4 modelView() const {
                // rotate model 1st around it's origin
                glm::mat4 ModelRotateX = glm::rotate(glm::mat4(1.0f), this->RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
                glm::mat4 ModelRotateY = glm::rotate(ModelRotateX, -this->RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
                glm::mat4 ModelRotateZ = glm::rotate(ModelRotateY, this->Rotation3axis.z, glm::vec3(0.f, 0.f, 1.f));
                // then translate
                glm::mat4 ModelTranslate = glm::translate(ModelRotateZ,  modelPosition());
                // final ModelView
                return ModelTranslate;
        };

	bool initProgram() {
		if (Validated){
                        compiler Compiler;

                        GLuint VertexShader     = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + SAMPLE_VERTEX_SHADER);
                        GLuint ControlShader    = Compiler.create(GL_TESS_CONTROL_SHADER, getDataDirectory() + SAMPLE_CONTROL_SHADER);
                        GLuint EvaluationShader = Compiler.create(GL_TESS_EVALUATION_SHADER, getDataDirectory() + SAMPLE_EVALUATION_SHADER);
                        GLuint GeometryShader   = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + SAMPLE_GEOMETRY_SHADER);
                        GLuint FragmentShader   = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + SAMPLE_FRAGMENT_SHADER);

                        if (!VertexShader || !ControlShader || !EvaluationShader || !GeometryShader || !FragmentShader)
                                Validated = false;
                        else {
                                ProgramName = glCreateProgram();
                                glAttachShader(ProgramName, VertexShader);
                                glAttachShader(ProgramName, ControlShader);
                                glAttachShader(ProgramName, EvaluationShader);
                                glAttachShader(ProgramName, GeometryShader);
                                glAttachShader(ProgramName, FragmentShader);
                                glLinkProgram(ProgramName);
                        }
                        
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.check_program(ProgramName);
		}

		if(Validated){
                        Uniform_ModelViewProjection = glGetUniformLocation(ProgramName, "ModelViewProjection"); // mat4 -- projection
                        Uniform_ModelView     = glGetUniformLocation(ProgramName, "ModelView"); // mat4 -- projection
                        Uniform_lightDirWorld = glGetUniformLocation(ProgramName, "lightDirWorld"); // vec3
                        Uniform_eyePosWorld   = glGetUniformLocation(ProgramName, "eyePosWorld"); // vec3
                        Uniform_sunColor      = glGetUniformLocation(ProgramName, "sunColor"); // = vec3(1.0, 1.0, 0.7);
                        Uniform_lightColor    = glGetUniformLocation(ProgramName, "lightColor"); // = vec3(1.0, 1.0, 0.7)*1.5;
                        Uniform_fogColor      = glGetUniformLocation(ProgramName, "fogColor"); // = vec3(0.7, 0.8, 1.0)*0.7;

                        Uniform_fogExp        = glGetUniformLocation(ProgramName, "fogExp"); // = 0.1;

                        Uniform_shininess     = glGetUniformLocation(ProgramName, "shininess"); // = 100.0;
                        Uniform_ambientColor  = glGetUniformLocation(ProgramName, "ambientColor"); // = vec3(0.05, 0.05, 0.15 );
                        Uniform_wrap          = glGetUniformLocation(ProgramName, "wrap"); // = 0.3;
                        Uniform_delta         = glGetUniformLocation(ProgramName, "delta"); //  1/nx,1/ny

                        Uniform_height_scale  = glGetUniformLocation(ProgramName, "height_scale");  // float
                        Uniform_height_offset = glGetUniformLocation(ProgramName, "height_offset");  // float
                        Uniform_aspect        = glGetUniformLocation(ProgramName, "aspect");  // float
                        Uniform_screen_size   = glGetUniformLocation(ProgramName, "screen_size"); // vec2
                        Uniform_lod_factor    = glGetUniformLocation(ProgramName, "lod_factor");  // float
                        Uniform_tess_level    = glGetUniformLocation(ProgramName, "tess_level");  // float
                        checkError("initProgram -- get uniform variable references...");
		}

                if (Validated)
                        return Validated && checkError("initProgram");

		return Validated;
	};

	bool initBuffer() {
		if (!Validated) return false;

                surface_plane = new base_plane ((s->get_scan ())->mem2d, 32, (s->get_scan ())->data.s.ry/(s->get_scan ())->data.s.rx);
                
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
                Validated = false;

                /* we need to ensure that the GdkGLContext is set before calling GL API */
                gtk_gl_area_make_current (glarea);

                if (surface_plane)
                        delete surface_plane;
                
                glDeleteTextures(TextureCount, TextureName);
                glDeleteProgram(ProgramName);

		return checkError("end");
	};

	bool render() {
		if (!Validated) return false;

                g_message ("Render: Camera at = (0, %g, %g),"
                           " Translate = (%g, %g, 0),"
                           " Rotate = (%g, %g, %g),"
                           " Height-Scale = %g"
                           "",
                           DistanceCurrent.x, DistanceCurrent.y,
                           TranslationCurrent.x, TranslationCurrent.y,
                           RotationCurrent.x, RotationCurrent.y, Rotation3axis.z,
                           s->GLv_data.hskl);

                
                glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable (GL_DEPTH_TEST);

                glPolygonMode (GL_FRONT_AND_BACK, s->GLv_data.Mesh ? GL_LINE : GL_FILL);
                glShadeModel(s->GLv_data.Smooth ? GL_SMOOTH : GL_FLAT);

                if (s->GLv_data.Cull)
                        glEnable (GL_CULL_FACE);
                else
                        glDisable (GL_CULL_FACE);

                if (s->GLv_data.Smooth)
                        ; //glEnable (GL_LINE_SMOOTH);


                // https://glm.g-truc.net/0.9.4/api/a00151.html
                float aspect = WindowSize.x/WindowSize.y;
                // GLfloat fov=45.0f, GLfloat near=0.1f, GLfloat far=100.0f
                glm::mat4 Projection = glm::perspective (glm::radians (s->GLv_data.fov/57.3f), aspect, s->GLv_data.Znear, s->GLv_data.Zfar);
                glm::mat4 Camera = glm::lookAt (cameraPosition(), // cameraPosition, the position of your camera, in world space
                                                glm::vec3(0,0,0), //cameraTarget, where you want to look at, in world space
                                                glm::vec3(0,0,1) // upVector, probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
                                                );

		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 ModelView = this->modelView() * Model;
		glm::mat4 ModelViewProjection = Projection * Camera * ModelView;

                /* clear the viewport; the viewport is automatically resized when
                 * the GtkGLArea gets a new size allocation
                 */
                // glViewport (0, 0, WindowSize.x, WindowSize.y);

		//glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);
                glClearBufferfv (GL_COLOR, 0, s->GLv_data.clear_color);

		glUseProgram (ProgramName);
                // Projection
                glUniformMatrix4fv (Uniform_ModelViewProjection, 1, GL_FALSE, &ModelViewProjection[0][0]);
                glUniformMatrix4fv (Uniform_ModelView, 1, GL_FALSE, &ModelView[0][0]);
                glm::vec3 eye = cameraPosition(); // eye / camera
                //glm::vec3 sun = glm::vec3(1.0f,0.0f,-3.0f); // sun light direction world
                glUniform3fv (Uniform_lightDirWorld, 1, &s->GLv_data.light_position[0][0]);
                glUniform3fv (Uniform_eyePosWorld, 1, &eye.x); //s->GLv_data.light_position[1][0]);

                // Light
                glm::vec3 light_color = 1.5f*glm::vec3(s->GLv_data.surf_mat_color[0], s->GLv_data.surf_mat_color[1], s->GLv_data.surf_mat_color[2]);
                glUniform3fv (Uniform_sunColor, 1, &s->GLv_data.light_specular[0][0]); // = vec3(1.0, 1.0, 0.7);
                glUniform3fv (Uniform_lightColor, 1, &light_color.x); // = vec3(1.0, 1.0, 0.7)*1.5;
                glUniform3fv (Uniform_fogColor, 1, &s->GLv_data.fog_color[0]); // = vec3(0.7, 0.8, 1.0)*0.7;

                glUniform1f  (Uniform_fogExp, s->GLv_data.fog_density); // = 0.1;

                glUniform1f  (Uniform_shininess, s->GLv_data.surf_mat_shininess[0]/100.); // = 100.0;
                glUniform3fv (Uniform_ambientColor, 1, &s->GLv_data.light_global_ambient[0]); // = vec3(0.05, 0.05, 0.15 );
                glUniform1f  (Uniform_wrap, 0.3); //&s->GLv_data.xxx); // = 0.3;
                glUniform2f  (Uniform_delta, 2./s->XPM_x, 2./s->XPM_x);

                // Geometry control
                glUniform1f (Uniform_height_scale, s->GLv_data.hskl);
                glUniform1f (Uniform_height_offset, s->GLv_data.slice_offset);
                glUniform1f (Uniform_aspect, (s->get_scan ())->data.s.ry/(s->get_scan ())->data.s.rx);
                glUniform1f (Uniform_lod_factor, 4.0);
                glUniform1f (Uniform_tess_level, s->GLv_data.tess_level);
                
                glEnableVertexAttribArray(TextureName[0]);
                glEnableVertexAttribArray(TextureName[1]);
                glBindTexture(GL_TEXTURE_2D, TextureName[0]);
                glBindTexture(GL_TEXTURE_2D, TextureName[1]);

                surface_plane->draw ();
                
		return true;
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
        
	i = (k/QuenchFac) + (j/QuenchFac)*XPM_x + XPM_x*XPM_y*vi;
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

	scan->mem2d->GetDataPkt_vec_normal_4F (k,j,vi, &surface_normal_z_buffer[i], 1.0, XPM_x*GLv_data.hskl/scan->mem2d->data->zrange);

        if(0){
                if (k==0 || j==0 || k == XPM_x-1 || j == XPM_y-1)
                        surface_normal_z_buffer[i].w = -1.;
        }
        
        // shift Norm to 0 .. 1
        surface_normal_z_buffer[i].x *= 0.5; surface_normal_z_buffer[i].x += 0.5;
        surface_normal_z_buffer[i].y *= 0.5; surface_normal_z_buffer[i].x += 0.5;
        surface_normal_z_buffer[i].z *= 0.5; surface_normal_z_buffer[i].x += 0.5;
        // normalize Z to 0 .. 1
        surface_normal_z_buffer[i].w -= scan->mem2d->data->zmin;
        surface_normal_z_buffer[i].w /= scan->mem2d->data->zrange;

        
	if (GLv_data.ColorSrc[0] != 'U'){
		switch (GLv_data.ColorSrc[0]){
		case 'H': 
			val = (GLfloat) (GLv_data.ColorOffset + GLv_data.ColorContrast * surface_normal_z_buffer[i].w);
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
			calccolor(val*1024, surface_color_buffer[i]);
			break;
		case 'M': // Material Color
			surface_color_buffer[i].x = GLv_data.surf_mat_color[0];
			surface_color_buffer[i].y = GLv_data.surf_mat_color[1];
			surface_color_buffer[i].z = GLv_data.surf_mat_color[2];
			break;
		case 'P': { // GXSM user Palette
			int ci = (int)(val*1024);
			ci = ci < 0 ? 0 : ci >= 1024 ? 1023 : ci;
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

	surface_color_buffer[i].x = surface_normal_z_buffer[i].w;
	surface_color_buffer[i].y = surface_normal_z_buffer[i].w;
	surface_color_buffer[i].z = surface_normal_z_buffer[i].w;
	surface_color_buffer[i].w = 1.;

	// adjust slice height level
	//surface_normal_z_buffer[i].w += (GLfloat)vi * GLv_data.slice_offset * (XPM_x+XPM_y)*0.5/XPM_v;

#if 0
        if (i%100==0)
                g_message ("%6d [%d,%d,%d]: n=(%8f, %8f, %8f) z=%8f   c=(%4f, %4f, %4f, %4f)",
                           i, k,j,vi,
                           surface_normal_z_buffer[i].x, surface_normal_z_buffer[i].y, surface_normal_z_buffer[i].z,
                           surface_normal_z_buffer[i].w,
                           surface_color_buffer[i].x,
                           surface_color_buffer[i].y,
                           surface_color_buffer[i].z,
                           surface_color_buffer[i].w
                           );
#endif   
        //        std::cout << "PutPointMode OK!" << std::endl << std::flush;
}


void Surf3d::GLvarinit(){
	mem2d_x=NULL;
        gl_tess = NULL;
	
	XPM_x = XPM_y = 0;
	scrwidth=500;
	scrheight=500;
	ZoomFac=1;
        QuenchFac=1;
	size=0;
        surface_normal_z_buffer = NULL;
	surface_color_buffer = NULL;

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
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLUPDATE" << std::flush);

        if (data){
        
                Surf3d *s = (Surf3d *) data;

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

void Surf3d::create_surface_buffer(){ 
	XSM_DEBUG (GL_DEBUG_L2, "Surf3d::create_surface_buffer");

	QuenchFac = 1; //1 + (int) ((double) scan->mem2d->GetNx () / (double) scrwidth * 4. * GLv_data.preV);

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

	XSM_DEBUG_GP (GL_DEBUG_L3, "Surf3d::create_surface_buffer  ** QF=%d size = %d, %d, %d\n", QuenchFac,  XPM_x, XPM_y, XPM_v);
        
	surface_normal_z_buffer = g_new (glm::vec4, size * sizeof(glm::vec4));
	surface_color_buffer    = g_new (glm::vec4, size * sizeof(glm::vec4));

	XSM_DEBUG (GL_DEBUG_L3, "Surf3d::create_surface_buffer  ** g_new completed");

	setup_data_transformation();

	XSM_DEBUG (GL_DEBUG_L3, "Surf3d::create_surface_buffer  ** computing surface and normals");
        // grab and prepare data buffers
        for(int v=0; v<scan->mem2d->GetNv(); ++v){
                scan->mem2d->data->update_ranges (v);
		for(int j=0; j<scan->mem2d->GetNy(); j+=QuenchFac)
			for(int k=0; k<scan->mem2d->GetNx(); k+=QuenchFac)
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
	scan->mem2d->SetDataRange (0, 1024);
}

int Surf3d::update(int y1, int y2){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update y1 y2: " << y1 << "," << y2);

	if (!scan || size == 0) return -1;

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update trafo");
	setup_data_transformation();

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::update data");
//	int v = scan->mem2d->GetLayer();
	for(int v=0; v<scan->mem2d->GetNv(); ++v){
                scan->mem2d->data->update_ranges (v);
		for(int j=y1; j < y2; j += QuenchFac)
			for(int k=0; k < scan->mem2d->GetNx (); k+=QuenchFac)
				PutPointMode (k,j,v);
	}

        if (v3dcontrol)
                v3dcontrol->rerender_scene ();

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
#if 0
	int slice_pli[3];
	XSM_DEBUG (GL_DEBUG_L2, "GL:::GLdrawsurface y_to_update=" << y_to_update);

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



/* ::GLdrawscene
 * ======================================================================
 * (re)draw all: create/recall scene
 * - adjust scene settings
 * - create transform/rotate matrix
 * - draw surface
 * - draw extra stuff
 * - swap/flush buffer(s) to visual
 */

// https://www.bassi.io/articles/2015/02/17/using-opengl-with-gtk/
// http://gamedev.stackexchange.com/questions/34108/opengl-vbo-or-glbegin-glend
// https://bcmpinc.wordpress.com/2015/08/15/starting-with-opengl-4-5/

gboolean Surf3d::GLdrawscene(GdkGLContext *glcontext, int y_to_update, int refresh_all){
	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene");

	if(!v3dcontrol) return FALSE;
        
	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene set modes\n");

        
#if 0
	if(GLv_data.Mesh)
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);


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

#endif
        
#if 0
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glPushMatrix ();

	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene set camera");

        
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

#endif
        
#if 0
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLdrawscene draw now...\n");
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** DRAW TEST TRIANGLE\n");
        // Draw the triangle !
        //glClear (GL_COLOR_BUFFER_BIT);
        glDrawArrays (GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 2 triangle
        //glutSwapBuffers ();
        //glDisableVertexAttribArray (0);
#endif
#if 0
        
        // *******************************
        
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** done **\n");
	GLdrawsurface (y_to_update, (glSurfaceListRange?(glSurfaceListRange[0]==0):0) || valid == 2 ? TRUE:refresh_all);

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::GLdrawscene draw gimmicks...");

	if (GLv_data.Ticks)
		GLdrawGimmicks ();

	// data and lists are valid now!
	valid = 1;

#endif

#if 0
	glPopMatrix ();
	glPopMatrix ();
#endif
	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene done.");
	return TRUE;
}

#if 0
void Surf3d::init_buffer_objects (GError **internal_error){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::init_buffer_objects");

        //    RGBA buffer as Normal XYZ, Z in vec4* surface_normal_z_buffer	

        // ************** OPENGL 4.5 TEST
        // An array of 3 vectors which represents 3 vertices
        vec3 g_vertex_buffer_data[] = {
                vec3 (-100.0f, -100.0f, 0.0f),
                vec3 (100.0f, -100.0f, 0.0f),
                vec3 (0.0f,  100.0f, 0.0f),
                vec3 (-100.0f, -100.0f, 30.0f),
                vec3 (100.0f, -100.0f, 30.0f),
                vec3 (0.0f,  100.0f, 30.0f)
        };

        GLuint vao[1];
        glGenVertexArrays (1, vao);
        glBindVertexArray (vao[0]);

        // This will identify our vertex buffer
        GLuint vertexbuffer[1];
        // Generate 1 buffer, put the resulting identifier in vertexbuffer
	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** VB1\n");
        glGenBuffers(1, vertexbuffer);

        // The following commands will talk about our 'vertexbuffer' buffer
	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** VB2\n");
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);

        // Give our vertices to OpenGL.
	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** VB3\n");
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** VA\n");

        // init shader?? + use program
        GLuint loc = glGetAttribLocation (program, "vPosition");
        glEnableVertexAttribArray (loc);
        glVertexAttribPointer (loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glClearColor (0.0, 0.0, 0.0, 1.0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray (loc);
        glVertexAttribPointer (
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );

	XSM_DEBUG_GP (GL_DEBUG_L2, "SURF3D:::GLdrawscene ** DRAW\n");
}

void Surf3d::init_shaders (GError **internal_error){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::init_shaders");

        program = load_shaders (
                                "vs3d_gl_vertex_shader.glsl",
                                "vs3d_gl_fragment_shader.glsl",
                                "vs3d_gl_control_shader.glsl",
                                "vs3d_gl_eval_shader.glsl"
                                );

}
#endif

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
                         " --> GL GLSL program code not found/installed?");

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
        // return s->GLdrawscene (context);
}


int Surf3d::draw(int zoomoverride){

	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::DRAW");

	if (!scan->mem2d) { 
		XSM_DEBUG (GL_DEBUG_L2, "Surf3d: no mem2d !"); 
		return 1; 
	}
	
	if ( !v3dcontrol )
		v3dcontrol = new V3dControl ("3D GL-View", ChanNo, scan,
					     G_CALLBACK (resize_vsurf3d_cb),
					     G_CALLBACK (render_vsurf3d_cb),
                                             G_CALLBACK (realize_vsurf3d_cb),
					     self);

        v3dcontrol->rerender_scene ();

        //if (scan && self)
        //        GLupdate (self);
        
	return 0;
}

void Surf3d::preferences(){
	XSM_DEBUG (GL_DEBUG_L2, "SURF3D:::PREFERENCES");

	if (v3dControl_pref_dlg)
		gnome_res_run_change_user_config (v3dControl_pref_dlg, "GL Scene Setup");
}





/*

The terrain in this video is rendered from a 4096x4096 pixel heightmap, normalmap and texture. It consists of 128x128 patches, each of which can be divided up to 64 times. This gives a virtual triangle resolution of up to 67 million triangles.
Since the GTX-460 has a performance of about 600 million triangles per second, if all triangles where put on screen the result would be around 10 frames per second.
Using tessellation shaders and a LODing technique, I can get framerates between 360-500 FPS on this scene.

Why GPU LODing?
LODing has a long history and traditionally it was all done on the CPU and then pushed to the GPU. By far and large all traditional approaches to LODing of terrain have these things in common.
They are complex and difficult to implement, for instance Overgrowth by Wolfire eschews the use of CPU LODing in favor of TINs.
Noticable temporal and/or spatial artefacts
Not compatible with TINs
Shift load from the (very powerful) GPU to the CPU
Rely on a lot of preprocessing
By moving the level of detail implementation into the graphics card, these issues can be addressed much more satisfactory.
New OpenGL Functionality
Opengl 4 introduced three new pipeline stages between Vertex shading and Geometry shading
Tessellation Control: Is Programmable. Decides how often a patch is subdivided.
Tessellator: Is configurable. Takes the data from control and produces new primitives.
Tessellation Evaluation: Is programmable. Receives the output of the tessellator, can modify each output vertex.
It also introduced a couple of new primitives, the most important of which is GL_PATCHES which can have between 1 - 32 vertices per patch.
Libraries
I use a texture for the terrain color, a heightmap texture for the terrain elevation and a normalmap texture for normals. The terrain is generated with Lithosphere. For windowing, opengl context and input I use Pyglet and an abstraction library for advanced OpengGL usage: gletools.
Setup
// The normal and heightmap are baked together into a GL_RGBA32F texture where RGB is the normal and Alpha is the terrain height. With these textures bound and the shader program, a plane consisting of 128x128 quad patches is rendered using a Vertexbuffer object. The plane covers the world coordinates from 0,0,0 to 1,1,0. The Z axis in worldspace is assumed to be terrain elevation.
Vertex Shader
// For each vertex sent in a lookup into the terrain texture is made using x and y positions as texture coordinates. The obtained height is written to the z component of the resulting gl_Position.
// -- glsl

in vec4 position;
uniform sampler2D terrain;

void main(void){
    vec2 texcoord = position.xy;
    float height = texture(terrain, texcoord).a;
    vec4 displaced = vec4(
        position.x, position.y,
        height, 1.0);
    gl_Position = displaced;
}

Tessellation control shader
Defines that this shader produces 4 vertices of output.
glsl
layout(vertices = 4) out;
Accept three uniforms, the screen size in pixels, the modelview/projection matrix and a LOD factor
glsl
uniform vec2 screen_size;
uniform mat4 mvp;
uniform float lod_factor;

// A helper function to project a world space vertex to device normal space -- glsl

vec4 project(vec4 vertex){
    vec4 result = mvp * vertex;
    result /= result.w;
    return result;
}

// This helper function converts a device normal space vector to screen space -- glsl

vec2 screen_space(vec4 vertex){
    return (clamp(vertex.xy, -1.3, 1.3)+1) * (screen_size*0.5);
}

// The LOD calculation as a function of distance in screen space -- glsl

float level(vec2 v0, vec2 v1){
     return clamp(distance(v0, v1)/lod_factor, 1, 64);
 }

// To improve performance this function is used to test a vertex in device normal space against the view frustum -- glsl

bool offscreen(vec4 vertex){
    if(vertex.z < -0.5){
        return true;
    }   
    return any(
        lessThan(vertex.xy, vec2(-1.7)) ||
        greaterThan(vertex.xy, vec2(1.7))
    );  
}

// The main function is called for each vertex in the patch. gl_InvocationID identifies which vertex is being processed.
// An estimate is made whether a primitive is on screen, and if not, all tessellation levels are set to zero which causes this patch to be skipped.
// If the patch is on screen then each edge is subdivided such as to approximate the given lod_factor (in pixels per edge).
// -- glsl

void main(){
     #define id gl_InvocationID
     gl_out[id].gl_Position = gl_in[id].gl_Position;
     if(id == 0){
         vec4 v0 = project(gl_in[0].gl_Position);
         vec4 v1 = project(gl_in[1].gl_Position);
         vec4 v2 = project(gl_in[2].gl_Position);
         vec4 v3 = project(gl_in[3].gl_Position);

         if(all(bvec4(
             offscreen(v0),
             offscreen(v1),
             offscreen(v2),
             offscreen(v3)
         ))){
             gl_TessLevelInner[0] = 0;
             gl_TessLevelInner[1] = 0;
             gl_TessLevelOuter[0] = 0;
             gl_TessLevelOuter[1] = 0;
             gl_TessLevelOuter[2] = 0;
             gl_TessLevelOuter[3] = 0;
         }
         else{
             vec2 ss0 = screen_space(v0);
             vec2 ss1 = screen_space(v1);
             vec2 ss2 = screen_space(v2);
             vec2 ss3 = screen_space(v3);

             float e0 = level(ss1, ss2);
             float e1 = level(ss0, ss1);
             float e2 = level(ss3, ss0);
             float e3 = level(ss2, ss3);

             gl_TessLevelInner[0] = mix(e1, e2, 0.5);
             gl_TessLevelInner[1] = mix(e0, e3, 0.5);
             gl_TessLevelOuter[0] = e0;
             gl_TessLevelOuter[1] = e1;
             gl_TessLevelOuter[2] = e2;
             gl_TessLevelOuter[3] = e3;
         }
     }
 }

// The tessellation is split into the inner part (gl_TessLevelInner) which governs how often the inside of a patch is divided, and the outer part (glTessLevelOuter) which governs how often an edge is divided.
The gl_TessLevel* and gl_InvocationID variables for quad sized patches correlate in the following way:
gl_TessLevelInner[0] orientation corresponds to the edges identified by gl_TessLevelOuter[1] and glTessLevelOuter[2]
gl_TessLevelInner[1] orientation corresponds to the edges identified by gl_TessLevelOuter[0] and glTessLevelOuter[3]
gl_TessLevelOuter[0] corresponds to gl_InvocationID 1 and 2
gl_TessLevelOuter[1] corresponds to gl_InvocationID 0 and 1
gl_TessLevelOuter[2] corresponds to gl_InvocationID 3 and 0
gl_TessLevelOuter[2] corresponds to gl_InvocationID 2 and 3
Tessellation Evaluation Shader
Controls the Tessellator, telling it to produce smoothly sliding divisions (determined by the tessellation levels) at odd spacings. fractional_even_spacing and equal_spacing is also available.
glsl
layout(quads, fractional_odd_spacing, ccw) in;
It produces a texture coordinate and a depth for use in the fragment shader, and it samples the terrain map again.
glsl
out vec2 texcoord;
out float depth;

uniform sampler2D terrain;
uniform mat4 mvp;
The Evaluation main function is called once for each vertex of the tessellated output. The coordinate is given as UV vector relative to the positions of the patches control points.
After the position calculation, the texcoord is used to make the lookup into the terrain heightmap. Again the obtained height is used as Z component in the resulting gl_Position.
glsl
void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
    vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
    vec4 position = mix(a, b, v);
    texcoord = position.xy;
    float height = texture(terrain, texcoord).a;
    gl_Position = mvp * vec4(texcoord, height, 1.0);
    depth = gl_Position.z;
}

A geometry shader stage is not required and the fragment shader is business as usual and you can look it up in the full source.
Source Code
This terrain LOD implementation is available in the gletools examples or you can view the application and shader source directly. I use a small preprocessor to split the shader source in its respective (vertex, control, evaluator, fragment) components and add the version tag for each.
I cannot share the terrain data at this point because it is 400mb in size.
Advantages
The implementation is reasonably easy with about 120 lines of shading code.
A good quality of tessellation can be achieved
Instead of regularly divided meshes, TINs could just as easily be used
Keeps all the load on the GPU
No preprocessing required
Issues and Limitations
The current algorithm for selecting the LOD factor tends to select too few division on an edge when the view vector is collinear to a patches edge and the patch is near to the viewpoint. This leads to odd (but small) artifacts.
A patch can be divided maximally by 64 further divisions per edge. Very large terrains would require a large number of input patches. This would lead to faraway patches being smaller then the desired primitive size, and near patches would be partially over tessellated.
Loading larger terrains into vram is not feasible with the current approach.
I do not have an ATI card, and I cannot comment if this code runs on it. It is largely unknown how many people as a gaming/creative professional audience would be able to use OpenGL 4 compatible hardware and drivers right now.
Further Work
A better measure for the LOD factor could be developed that both preserves screen-space even division, but takes into account edges with too few divisions.
Multi stage GPU tessellation using transform and feedback buffers could be used. This would allow fine grained control over tessellation without producing massive amounts of patches.
The technique of texture clipmapping/megatexturing could be used to hold texture data many times larger then the available VRAM
Acknowledgements
The Rastergrid blog has some very good entries on new opengl functionality and rendering techniques.
These two entries by The Little Grasshopper where instrumental for me to understand tessellation shading.
The Redbook and the Orangebook where valuable sources of reference information and I'm looking forward to see these books in OpenGL 4.1 updated version.
Me

Florian Boesch
Born 1978
I like writing software, toying with 3d things and reading science fiction.
Living in Basel, Switzerland
pyalot@gmail.com


#version 400

vertex:
    in vec4 position;
    uniform sampler2D terrain;
    
    void main(void){
        vec2 texcoord = position.xy;
        float height = texture(terrain, texcoord).a;
        vec4 displaced = vec4(position.x, position.y, height, 1.0);
        gl_Position = displaced;
    }

control:
    layout(vertices = 4) out;

    uniform vec2 screen_size;
    uniform mat4 mvp;
    uniform float lod_factor;
    
    bool offscreen(vec4 vertex){
        if(vertex.z < -0.5){
            return true;
        }
        return any(
            lessThan(vertex.xy, vec2(-1.7)) ||
            greaterThan(vertex.xy, vec2(1.7))
        );
    }
    
    vec4 project(vec4 vertex){
        vec4 result = mvp * vertex;
        result /= result.w;
        return result;
    }

    vec2 screen_space(vec4 vertex){
        return (clamp(vertex.xy, -1.3, 1.3)+1) * (screen_size*0.5);
    }

    float level(vec2 v0, vec2 v1){
        return clamp(distance(v0, v1)/lod_factor, 1, 64);
    }

    void main(){
        if(gl_InvocationID == 0){
            vec4 v0 = project(gl_in[0].gl_Position);
            vec4 v1 = project(gl_in[1].gl_Position);
            vec4 v2 = project(gl_in[2].gl_Position);
            vec4 v3 = project(gl_in[3].gl_Position);

            if(all(bvec4(offscreen(v0), offscreen(v1), offscreen(v2), offscreen(v3)))){
                gl_TessLevelInner[0] = 0;
                gl_TessLevelInner[1] = 0;
                gl_TessLevelOuter[0] = 0;
                gl_TessLevelOuter[1] = 0;
                gl_TessLevelOuter[2] = 0;
                gl_TessLevelOuter[3] = 0;
            }
            else{
                vec2 ss0 = screen_space(v0);
                vec2 ss1 = screen_space(v1);
                vec2 ss2 = screen_space(v2);
                vec2 ss3 = screen_space(v3);

                float e0 = level(ss1, ss2);
                float e1 = level(ss0, ss1);
                float e2 = level(ss3, ss0);
                float e3 = level(ss2, ss3);

                gl_TessLevelInner[0] = mix(e1, e2, 0.5);
                gl_TessLevelInner[1] = mix(e0, e3, 0.5);
                gl_TessLevelOuter[0] = e0;
                gl_TessLevelOuter[1] = e1;
                gl_TessLevelOuter[2] = e2;
                gl_TessLevelOuter[3] = e3;
            }
        }
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    }

eval:
    layout(quads, fractional_odd_spacing, ccw) in;
    out vec2 texcoord;
    out float depth;

    uniform sampler2D terrain;
    uniform mat4 mvp;

    void main(){
        float u = gl_TessCoord.x;
        float v = gl_TessCoord.y;

        vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
        vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
        vec4 position = mix(a, b, v);
        texcoord = position.xy;
        float height = texture(terrain, texcoord).a;
        gl_Position = mvp * vec4(texcoord, height, 1.0);
        depth = gl_Position.z;
    }

fragment:
    in vec2 texcoord;
    in float depth;
    out vec4 fragment;

    uniform sampler2D diffuse;
    uniform sampler2D terrain;
    uniform sampler2D noise_tile;

    vec3 incident = normalize(vec3(1.0, 0.2, 0.5));
    vec4 light = vec4(1.0, 0.95, 0.9, 1.0) * 1.1;

    void main(){
        vec3 normal = normalize(texture(terrain, texcoord).xyz);
        vec4 color = texture(diffuse, texcoord);
        float noise_factor = texture(noise_tile, texcoord*32).r+0.1;

        float dot_surface_incident = max(0, dot(normal, incident));

        color = color * light * noise_factor * (max(0.1, dot_surface_incident)+0.05)*1.5;
        fragment = mix(color, color*0.5+vec4(0.5, 0.5, 0.5, 1.0), depth*2.0);
    }

//---

from contextlib import nested
import math

import pyglet
from pyglet.gl import *

from gletools import ShaderProgram, Matrix, Texture, Sampler2D, DepthTest

from util import View, make_plane

config = Config(buffers=2, samples=4)
window = pyglet.window.Window(config=config, fullscreen=True, vsync=False)
view = View(window)

size = 1024*4
diffuse = Texture.raw_open('data/patches/snowy_mountains.diffuse', 
    width=size, height=size, format=GL_RGBA32F,
    mipmap=4, filter=GL_LINEAR_MIPMAP_LINEAR, clamp='st',
    unit=GL_TEXTURE0,
)
terrain = Texture.raw_open('data/patches/snowy_mountains.terrain',
    width=size, height=size, format=GL_RGBA32F,
    unit=GL_TEXTURE1, clamp='st',
)
noise_tile = Texture.open('data/tilable_noise.png', 
    mipmap=4, filter=GL_LINEAR_MIPMAP_LINEAR, unit=GL_TEXTURE2,
)

program = ShaderProgram.open('terrain.shader',
    diffuse = Sampler2D(GL_TEXTURE0),
    terrain = Sampler2D(GL_TEXTURE1),
    noise_tile = Sampler2D(GL_TEXTURE2),
    lod_factor = 4.0,
)

vbo = make_plane(128, 128)
fps = pyglet.clock.ClockDisplay(color=(144.0/255.0,195.0/255.0,6.0/255.0,0.5))

@window.event
def on_draw():
    window.clear()

    model = Matrix().rotatex(-0.25).translate(-0.5, -0.5, 0.0)
    projection = Matrix.perspective(window.width, window.height, 65, 0.0001, 100.0)
    modelview = view.matrix * model

    program.vars.mvp = projection * modelview
    program.vars.screen_size = float(window.width), float(window.height)

    with nested(DepthTest, diffuse, terrain, noise_tile, program):
        glPatchParameteri(GL_PATCH_VERTICES, 4);
        vbo.draw(GL_PATCHES)
   
    fps.draw()

if __name__ == '__main__':
    glEnable(GL_CULL_FACE)
    glCullFace(GL_BACK)
    glClearColor(1,1,1,1)
    pyglet.app.run()


*/
