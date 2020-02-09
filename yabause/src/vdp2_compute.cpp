﻿/*
Copyright 2011-2015 Shinya Miyamoto(devmiyax)

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

extern "C"{
#include "ygl.h"
#include "yui.h"
#include "vidshared.h"

#include "common_glshader.h"
extern vdp2rotationparameter_struct  Vdp1ParaA;
}

#define YGLDEBUG

#define DEBUGWIP

static const GLchar vdp2blit_cs_start_f[] =
SHADER_VERSION_COMPUTE
"#ifdef GL_ES\n"
"precision highp float; \n"
"#endif\n"
"layout(local_size_x = 16, local_size_y = 16) in;\n"

"layout(rgba8, binding = 0) writeonly uniform image2D outSurface;\n"
"layout(binding = 1) uniform sampler2D s_texture0;  \n"
"layout(binding = 2) uniform sampler2D s_texture1;  \n"
"layout(binding = 3) uniform sampler2D s_texture2;  \n"
"layout(binding = 4) uniform sampler2D s_texture3;  \n"
"layout(binding = 5) uniform sampler2D s_texture4;  \n"
"layout(binding = 6) uniform sampler2D s_texture5;  \n"
"layout(binding = 7) uniform sampler2D s_back;  \n"
"layout(binding = 8) uniform sampler2D s_lncl;  \n"
"layout(binding = 9) uniform sampler2D s_vdp1FrameBuffer;\n"
"layout(binding = 10) uniform sampler2D s_win0;  \n"
"layout(binding = 11) uniform sampler2D s_win1;  \n"
"layout(binding = 12) uniform sampler2D s_color; \n"
"layout(std430, binding = 13) readonly buffer VDP2reg { int s_vdp2reg[]; }; \n"
"layout(std430, binding = 14) readonly buffer VDP2DrawInfo { \n"
"  float u_emu_height;\n"
"  float u_emu_vdp1_width;\n"
"  float u_emu_vdp2_width;\n"
"  float u_vheight; \n"
"  float vdp1Ratio_vec[2]; \n"
"  int fbon;  \n"
"  int screen_nb;  \n"
"  int ram_mode; \n"
"  int extended_cc; \n"
"  int u_lncl[7];  \n"
"  int mode[7];  \n"
"  int isRGB[6]; \n"
"  int isBlur[7]; \n"
"  int isShadow[6]; \n"
"  int use_sp_win;\n"
"  int use_trans_shadow; \n"
"  int tvSize_vec[2];\n"
"  int win_s[8]; \n"
"  int win_s_mode[8]; \n"
"  int win0[8]; \n"
"  int win0_mode[8]; \n"
"  int win1[8]; \n"
"  int win1_mode[8]; \n"
"  int win_op[8]; \n"
"};\n"
"layout(location = 15) uniform mat4 fbMat;\n"

"vec4 finalColor;\n"
"ivec2 texel = ivec2(gl_GlobalInvocationID.xy);\n"
"int PosX = texel.x;\n"
"int PosY = texel.y;\n"
"ivec2 tvSize = ivec2(tvSize_vec[0],tvSize_vec[1]);\n"
"vec2 vdp1Ratio = vec2(vdp1Ratio_vec[0],vdp1Ratio_vec[1]);\n"

"ivec2 size = imageSize(outSurface);\n"
"vec2 v_texcoord = vec2(float(texel.x)/float(size.x),float(texel.y)/float(size.y));\n"

"float getVdp2RegAsFloat(int id) {\n"
"  return float(s_vdp2reg[id])/255.0;\n"
"};\n";

const GLchar Yglprg_vdp2_drawfb_cs_cram_f[] =
"int getVDP2Reg(int id) {\n"
"  return s_vdp2reg[id];\n"
"}\n"
"FBCol getFB(int x, ivec2 addr){ \n"
"  vec4 lineCoord = vec4(texel.x, (u_vheight-texel.y), 0.0, 0.0);\n"
"  int line = int(lineCoord.y * u_emu_height)*24;\n"
"  vec3 u_coloroffset = vec3(getVdp2RegAsFloat(17 + line), getVdp2RegAsFloat(18 + line), getVdp2RegAsFloat(19 + line));\n"
"  vec3 u_coloroffset_sign = vec3(getVdp2RegAsFloat(20 + line), getVdp2RegAsFloat(21 + line), getVdp2RegAsFloat(22 + line));\n";

static const GLchar vdp2blit_cs_end_f[] =
"  if ((texel.x >= size.x) || (texel.y >= size.y)) return;\n";

static const GLchar vdp2blit_cs_final_f[] =
// "  finalColor = vec4(vec2(0),vec2(vdp1Ratio)/4.0);\n"
"  imageStore(outSurface,texel,finalColor);\n"
"}\n";

void initDrawShaderCode() {
  initVDP2DrawCode(vdp2blit_cs_start_f, Yglprg_vdp2_drawfb_cs_cram_f, vdp2blit_cs_end_f, vdp2blit_cs_final_f);
}

struct VDP2DrawInfo {
	float u_emu_height;
	float u_emu_vdp1_width;
	float u_emu_vdp2_width;
	float u_vheight;
	float vdp1Ratio[2];
	int fbon;
	int screen_nb;
	int ram_mode;
	int extended_cc;
	int u_lncl[7];
	int mode[7];
	int isRGB[6];
	int isBlur[7];
	int isShadow[6];
	int use_sp_win;
  int use_trans_shadow;
  int tvSize[2];
  int win_s[8];
  int win_s_mode[8];
  int win0[8];
  int win0_mode[8];
  int win1[8];
  int win1_mode[8];
  int win_op[8];
};

class VDP2Generator{
  GLuint prg_vdp2_composer[20] = {0};

  int tex_width_ = 0;
  int tex_height_ = 0;
  static VDP2Generator * instance_;
  GLuint scene_uniform = 0;
	GLuint ssbo_vdp2reg_ = 0;
  VDP2DrawInfo uniform;
	int struct_size_;

protected:
  VDP2Generator() {
    tex_width_ = 0;
    tex_height_ = 0;
		scene_uniform = 0;
		ssbo_vdp2reg_ = 0;
		struct_size_ = sizeof(VDP2DrawInfo);
		int am = sizeof(VDP2DrawInfo) % 4;
		if (am != 0) {
			struct_size_ += 4 - am;
		}
  }

public:
  static VDP2Generator * getInstance() {
    if (instance_ == nullptr) {
      instance_ = new VDP2Generator();
    }
    return instance_;
  }

  void resize(int width, int height) {
	if (tex_width_ == width && tex_height_ == height) return;

	DEBUGWIP("resize %d, %d\n",width,height);

	tex_width_ = width;
	tex_height_ = height;
  }

  //-----------------------------------------------
  void init( int width, int height ) {

		if (scene_uniform != 0) return; // always inisialized!

		tex_width_ = width;
		tex_height_ = height;

		initDrawShaderCode();

		glGenBuffers(1, &scene_uniform);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene_uniform);
		glBufferData(GL_SHADER_STORAGE_BUFFER, struct_size_, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &ssbo_vdp2reg_);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vdp2reg_);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 512*sizeof(int)*NB_VDP2_REG,(void*)YglGetVDP2RegPointer(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	}

  bool ErrorHandle(const char* name)
  {
  #ifdef DEBUG
    GLenum   error_code = glGetError();
    if (error_code == GL_NO_ERROR) {
      return  true;
    }
    do {
      const char* msg = "";
      switch (error_code) {
      case GL_INVALID_ENUM:      msg = "INVALID_ENUM";      break;
      case GL_INVALID_VALUE:     msg = "INVALID_VALUE";     break;
      case GL_INVALID_OPERATION: msg = "INVALID_OPERATION"; break;
      case GL_OUT_OF_MEMORY:     msg = "OUT_OF_MEMORY";     break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:  msg = "INVALID_FRAMEBUFFER_OPERATION"; break;
      default:  msg = "Unknown"; break;
      }
      YuiMsg("GLErrorLayer:ERROR:%04x'%s' %s\n", error_code, msg, name);
	  abort();
      error_code = glGetError();
    } while (error_code != GL_NO_ERROR);
    return  false;
	#else
	  return true;
	#endif
  }

	void update( int outputTex, YglPerLineInfo *bg, int* prioscreens, int* modescreens, int* isRGB, int* isShadow, int * isBlur, int* lncl, GLuint* vdp1fb,  int* Win_s, int* Win_s_mode, int* Win0, int* Win0_mode, int* Win1, int* Win1_mode, int* Win_op, Vdp2 *varVdp2Regs) {

    GLuint error;
    int local_size_x = 16;
    int local_size_y = 16;
	  int nbScreen = 6;

		int work_groups_x = (tex_width_) / local_size_x;
    int work_groups_y = (tex_height_) / local_size_y;

		int gltext[6] = {GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6};

    error = glGetError();

	  DEBUGWIP("prog %d\n", __LINE__);
		setupVDP2Prog(varVdp2Regs, 1);

    YglMatrix vdp1Mat;

    YglLoadIdentity(&vdp1Mat);

    if (Vdp1Regs->TVMR & 0x02) {
      YglMatrix translate, rotation, scale, fuse;
      //Translate to center of rotation
      YglLoadIdentity(&translate);
      translate.m[0][3] = (-Vdp1ParaA.Cx - (float)_Ygl->rwidth/2.0f )* (float)_Ygl->vdp1width/512.0f;
      translate.m[1][3] = (-Vdp1ParaA.Cy - (float)_Ygl->rheight/2.0f)* (float)_Ygl->vdp1height/256.0f;

      //Rotate and translate back to the (Xst,Yst) from (0,0)
      YglLoadIdentity(&rotation);
      rotation.m[0][0] = Vdp1ParaA.deltaX;
      rotation.m[0][1] = Vdp1ParaA.deltaY;
      rotation.m[0][3] = (Vdp1ParaA.Xst + Vdp1ParaA.Cx + (float)_Ygl->rwidth/2.0f)* (float)_Ygl->vdp1width/512.0f;
      rotation.m[1][0] = Vdp1ParaA.deltaXst;
      rotation.m[1][1] = Vdp1ParaA.deltaYst;
      rotation.m[1][3] = (Vdp1ParaA.Yst + Vdp1ParaA.Cy + (float)_Ygl->rheight/2.0f)* (float)_Ygl->vdp1height/256.0f;

      YglLoadIdentity(&scale);
      float scaleX = rotation.m[0][0]*rotation.m[0][0]+rotation.m[0][1]*rotation.m[0][1];
      float scaleY = rotation.m[1][0]*rotation.m[1][0]+rotation.m[1][1]*rotation.m[1][1];
      scale.m[0][0] = (scaleX!=0)?1/(scaleX):1.0;
      scale.m[1][1] = (scaleY!=0)?1/(scaleY):1.0;
      //merge transformations
      YglMatrixMultiply(&fuse, &rotation, &translate);
      YglMatrixMultiply(&vdp1Mat, &scale, &fuse);
    }

		uniform.u_emu_height = (float)_Ygl->rheight / (float)_Ygl->height;
    uniform.u_emu_vdp1_width = (float)(_Ygl->vdp1width) / (float)(_Ygl->rwidth);
    uniform.u_emu_vdp2_width = (float)(_Ygl->width) / (float)(_Ygl->rwidth);
    uniform.u_vheight = (float)_Ygl->height;
    uniform.vdp1Ratio[0] = _Ygl->vdp1expandW;
    uniform.vdp1Ratio[1] = _Ygl->vdp1expandH;
		uniform.fbon = (_Ygl->vdp1On[_Ygl->readframe] != 0);
		uniform.ram_mode = Vdp2Internal.ColorMode;
		uniform.extended_cc = ((varVdp2Regs->CCCTL & 0x400) != 0);
    memcpy(uniform.u_lncl,lncl, 7*sizeof(int));
    memcpy(uniform.mode, modescreens, 7*sizeof(int));
    memcpy(uniform.isRGB, isRGB, 6*sizeof(int));
    memcpy(uniform.isBlur, isBlur, 7*sizeof(int));
    memcpy(uniform.isShadow, isShadow, 6*sizeof(int));
		uniform.use_sp_win = ((varVdp2Regs->SPCTL>>4)&0x1);
    uniform.use_trans_shadow = ((varVdp2Regs->SDCTL>>8)&0x1);
    uniform.tvSize[0] = _Ygl->rwidth;
    uniform.tvSize[1] = _Ygl->rheight;
    memcpy(uniform.win_s, Win_s, (enBGMAX+1)*sizeof(int));
    memcpy(uniform.win_s_mode,Win_s_mode, (enBGMAX+1)*sizeof(int));
    memcpy(uniform.win0, Win0, (enBGMAX+1)*sizeof(int));
    memcpy(uniform.win0_mode, Win0_mode, (enBGMAX+1)*sizeof(int));
    memcpy(uniform.win1, Win1, (enBGMAX+1)*sizeof(int));
    memcpy(uniform.win1_mode, Win1_mode, (enBGMAX+1)*sizeof(int));
    memcpy(uniform.win_op, Win_op, (enBGMAX+1)*sizeof(int));

    glUniformMatrix4fv(15, 1, GL_FALSE, (GLfloat*)(vdp1Mat.m));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE7);
	  glBindTexture(GL_TEXTURE_2D, _Ygl->back_fbotex[0]);

	  glActiveTexture(GL_TEXTURE8);
	  glBindTexture(GL_TEXTURE_2D, _Ygl->lincolor_tex);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, vdp1fb[0]);

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, _Ygl->window_tex[0]);

		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, _Ygl->window_tex[1]);

		glActiveTexture(GL_TEXTURE12);
	  glBindTexture(GL_TEXTURE_2D, _Ygl->cram_tex);

#if 0
		glActiveTexture(GL_TEXTURE13);
	  glBindTexture(GL_TEXTURE_2D, _Ygl->vdp2reg_tex);
#else
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_vdp2reg_);
		u8* vdp2buf = YglGetVDP2RegPointer();
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 512*sizeof(int)*NB_VDP2_REG,(void*)vdp2buf);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssbo_vdp2reg_);
#endif

		int id = 0;
	  for (int i=0; i<nbScreen; i++) {
	    if (prioscreens[i] != 0) {
	      glActiveTexture(gltext[i]);
	      glBindTexture(GL_TEXTURE_2D, prioscreens[i]);
	      id++;
	    }
	  }
	  uniform.screen_nb = id;

		DEBUGWIP("Draw RBG0\n");
		glBindImageTexture(0, outputTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		ErrorHandle("glBindImageTexture 0");

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene_uniform);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, struct_size_, (void*)&uniform);
		ErrorHandle("glBufferSubData");
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, scene_uniform);

	  glDispatchCompute(work_groups_x, work_groups_y, 1);
		// glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	  ErrorHandle("glDispatchCompute");
  }

  //-----------------------------------------------
  void onFinish() {
  }

};

VDP2Generator * VDP2Generator::instance_ = nullptr;

extern "C" {
  void VDP2Generator_init(int width, int height) {
    if (_Ygl->vdp2_use_compute_shader == 0) return;

    VDP2Generator * instance = VDP2Generator::getInstance();
    instance->init( width, height);
  }
  void VDP2Generator_resize(int width, int height) {
    if (_Ygl->vdp2_use_compute_shader == 0) return;
    YGLDEBUG("VDP2Generator_resize\n");
	  VDP2Generator * instance = VDP2Generator::getInstance();
	  instance->resize(width, height);
  }
  void VDP2Generator_update(int tex, YglPerLineInfo *bg, int* prioscreens, int* modescreens, int* isRGB, int* isShadow, int * isBlur, int* lncl, GLuint* vdp1fb,  int* Win_s, int* Win_s_mode, int* Win0, int* Win0_mode, int* Win1, int* Win1_mode, int* Win_op, Vdp2 *varVdp2Regs ) {
    if (_Ygl->vdp2_use_compute_shader == 0) return;
    VDP2Generator * instance = VDP2Generator::getInstance();
    instance->update(tex, bg, prioscreens, modescreens, isRGB, isShadow, isBlur, lncl, vdp1fb, Win_s, Win_s_mode, Win0, Win0_mode, Win1, Win1_mode, Win_op, varVdp2Regs);
  }
  void VDP2Generator_onFinish() {

    if (_Ygl->vdp2_use_compute_shader == 0) return;
    VDP2Generator * instance = VDP2Generator::getInstance();
    instance->onFinish();
  }
}
