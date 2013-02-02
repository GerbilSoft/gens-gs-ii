/*
** The OpenGL Extension Wrangler Library
** Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>
** Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>
** Copyright (C) 2002, Lev Povalahev
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without 
** modification, are permitted provided that the following conditions are met:
** 
** * Redistributions of source code must retain the above copyright notice, 
**   this list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice, 
**   this list of conditions and the following disclaimer in the documentation 
**   and/or other materials provided with the distribution.
** * The name of the author may be used to endorse or promote products 
**   derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Mesa 3-D graphics library
 * Version:  7.0
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
** Copyright (c) 2007 The Khronos Group Inc.
** 
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
** 
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
** 
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#ifndef __glew_h__
#define __glew_h__
#define __GLEW_H__

#if defined(__gl_h_) || defined(__GL_H__) || defined(__X_GL_H)
#error gl.h included before glew.h
#endif
#if defined(__REGAL_H__)
#error Regal.h included before glew.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
#error glext.h included before glew.h
#endif
#if defined(__gl_ATI_h_)
#error glATI.h included before glew.h
#endif

#define __gl_h_
#define __GL_H__
#define __REGAL_H__
#define __X_GL_H
#define __glext_h_
#define __GLEXT_H_
#define __gl_ATI_h_

#if defined(_WIN32)

/*
 * GLEW does not include <windows.h> to avoid name space pollution.
 * GL needs GLAPI and GLAPIENTRY, GLU needs APIENTRY, CALLBACK, and wchar_t
 * defined properly.
 */
/* <windef.h> */
#ifndef APIENTRY
#define GLEW_APIENTRY_DEFINED
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#    define APIENTRY __stdcall
#  elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#    define APIENTRY __stdcall
#  else
#    define APIENTRY
#  endif
#endif
#ifndef GLAPI
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#    define GLAPI extern
#  endif
#endif
/* <winnt.h> */
#ifndef CALLBACK
#define GLEW_CALLBACK_DEFINED
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#    define CALLBACK __attribute__ ((__stdcall__))
#  elif (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#  else
#    define CALLBACK
#  endif
#endif
/* <wingdi.h> and <winnt.h> */
#ifndef WINGDIAPI
#define GLEW_WINGDIAPI_DEFINED
#define WINGDIAPI __declspec(dllimport)
#endif
/* <ctype.h> */
#if (defined(_MSC_VER) || defined(__BORLANDC__)) && !defined(_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#  define _WCHAR_T_DEFINED
#endif
/* <stddef.h> */
#if !defined(_W64)
#  if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && defined(_MSC_VER) && _MSC_VER >= 1300
#    define _W64 __w64
#  else
#    define _W64
#  endif
#endif
#if !defined(_PTRDIFF_T_DEFINED) && !defined(_PTRDIFF_T_) && !defined(__MINGW64__)
#  ifdef _WIN64
typedef __int64 ptrdiff_t;
#  else
typedef _W64 int ptrdiff_t;
#  endif
#  define _PTRDIFF_T_DEFINED
#  define _PTRDIFF_T_
#endif

#ifndef GLAPI
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#    define GLAPI extern
#  else
#    define GLAPI WINGDIAPI
#  endif
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

#ifndef GLEWAPIENTRY
#define GLEWAPIENTRY APIENTRY
#endif

/*
 * GLEW_STATIC is defined for static library.
 * GLEW_BUILD  is defined for building the DLL library.
 */

#ifdef GLEW_STATIC
#  define GLEWAPI extern
#else
#  ifdef GLEW_BUILD
#    define GLEWAPI extern __declspec(dllexport)
#  else
#    define GLEWAPI extern __declspec(dllimport)
#  endif
#endif

#else /* _UNIX */

/*
 * Needed for ptrdiff_t in turn needed by VBO.  This is defined by ISO
 * C.  On my system, this amounts to _3 lines_ of included code, all of
 * them pretty much harmless.  If you know of a way of detecting 32 vs
 * 64 _targets_ at compile time you are free to replace this with
 * something that's portable.  For now, _this_ is the portable solution.
 * (mem, 2004-01-04)
 */

#include <stddef.h>

/* SGI MIPSPro doesn't like stdint.h in C++ mode          */
/* ID: 3376260 Solaris 9 has inttypes.h, but not stdint.h */

#if (defined(__sgi) || defined(__sun)) && !defined(__GNUC__)
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#define GLEW_APIENTRY_DEFINED
#define APIENTRY

/*
 * GLEW_STATIC is defined for static library.
 */

#ifdef GLEW_STATIC
#  define GLEWAPI extern
#else
#  if defined(__GNUC__) && __GNUC__>=4
#   define GLEWAPI extern __attribute__ ((visibility("default")))
#  elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#   define GLEWAPI extern __global
#  else
#   define GLEWAPI extern
#  endif
#endif

/* <glu.h> */
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef GLEWAPIENTRY
#define GLEWAPIENTRY
#endif

#endif /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------- GL_VERSION_1_1 ---------------------------- */

#ifndef GL_VERSION_1_1
#define GL_VERSION_1_1 1

typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned long GLulong;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
#if defined(_MSC_VER) && _MSC_VER < 1400
typedef __int64 GLint64EXT;
typedef unsigned __int64 GLuint64EXT;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
typedef signed long long GLint64EXT;
typedef unsigned long long GLuint64EXT;
#else
#  if defined(__MINGW32__) || defined(__CYGWIN__)
#include <inttypes.h>
#  endif
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64EXT;
#endif
typedef GLint64EXT  GLint64;
typedef GLuint64EXT GLuint64;
typedef struct __GLsync *GLsync;

typedef char GLchar;

#define GL_ZERO 0
#define GL_FALSE 0
#define GL_LOGIC_OP 0x0BF1
#define GL_NONE 0
#define GL_TEXTURE_COMPONENTS 0x1003
#define GL_NO_ERROR 0
#define GL_POINTS 0x0000
#define GL_CURRENT_BIT 0x00000001
#define GL_TRUE 1
#define GL_ONE 1
#define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_POINT_BIT 0x00000002
#define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#define GL_LINE_STRIP 0x0003
#define GL_LINE_BIT 0x00000004
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_POLYGON_BIT 0x00000008
#define GL_POLYGON 0x0009
#define GL_POLYGON_STIPPLE_BIT 0x00000010
#define GL_PIXEL_MODE_BIT 0x00000020
#define GL_LIGHTING_BIT 0x00000040
#define GL_FOG_BIT 0x00000080
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_ACCUM 0x0100
#define GL_LOAD 0x0101
#define GL_RETURN 0x0102
#define GL_MULT 0x0103
#define GL_ADD 0x0104
#define GL_NEVER 0x0200
#define GL_ACCUM_BUFFER_BIT 0x00000200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408
#define GL_AUX0 0x0409
#define GL_AUX1 0x040A
#define GL_AUX2 0x040B
#define GL_AUX3 0x040C
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_OUT_OF_MEMORY 0x0505
#define GL_2D 0x0600
#define GL_3D 0x0601
#define GL_3D_COLOR 0x0602
#define GL_3D_COLOR_TEXTURE 0x0603
#define GL_4D_COLOR_TEXTURE 0x0604
#define GL_PASS_THROUGH_TOKEN 0x0700
#define GL_POINT_TOKEN 0x0701
#define GL_LINE_TOKEN 0x0702
#define GL_POLYGON_TOKEN 0x0703
#define GL_BITMAP_TOKEN 0x0704
#define GL_DRAW_PIXEL_TOKEN 0x0705
#define GL_COPY_PIXEL_TOKEN 0x0706
#define GL_LINE_RESET_TOKEN 0x0707
#define GL_EXP 0x0800
#define GL_VIEWPORT_BIT 0x00000800
#define GL_EXP2 0x0801
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_COEFF 0x0A00
#define GL_ORDER 0x0A01
#define GL_DOMAIN 0x0A02
#define GL_CURRENT_COLOR 0x0B00
#define GL_CURRENT_INDEX 0x0B01
#define GL_CURRENT_NORMAL 0x0B02
#define GL_CURRENT_TEXTURE_COORDS 0x0B03
#define GL_CURRENT_RASTER_COLOR 0x0B04
#define GL_CURRENT_RASTER_INDEX 0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#define GL_CURRENT_RASTER_POSITION 0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#define GL_CURRENT_RASTER_DISTANCE 0x0B09
#define GL_POINT_SMOOTH 0x0B10
#define GL_POINT_SIZE 0x0B11
#define GL_POINT_SIZE_RANGE 0x0B12
#define GL_POINT_SIZE_GRANULARITY 0x0B13
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_WIDTH 0x0B21
#define GL_LINE_WIDTH_RANGE 0x0B22
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_LINE_STIPPLE 0x0B24
#define GL_LINE_STIPPLE_PATTERN 0x0B25
#define GL_LINE_STIPPLE_REPEAT 0x0B26
#define GL_LIST_MODE 0x0B30
#define GL_MAX_LIST_NESTING 0x0B31
#define GL_LIST_BASE 0x0B32
#define GL_LIST_INDEX 0x0B33
#define GL_POLYGON_MODE 0x0B40
#define GL_POLYGON_SMOOTH 0x0B41
#define GL_POLYGON_STIPPLE 0x0B42
#define GL_EDGE_FLAG 0x0B43
#define GL_CULL_FACE 0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE 0x0B46
#define GL_LIGHTING 0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_SHADE_MODEL 0x0B54
#define GL_COLOR_MATERIAL_FACE 0x0B55
#define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#define GL_COLOR_MATERIAL 0x0B57
#define GL_FOG 0x0B60
#define GL_FOG_INDEX 0x0B61
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_FOG_MODE 0x0B65
#define GL_FOG_COLOR 0x0B66
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_TEST 0x0B71
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_FUNC 0x0B74
#define GL_ACCUM_CLEAR_VALUE 0x0B80
#define GL_STENCIL_TEST 0x0B90
#define GL_STENCIL_CLEAR_VALUE 0x0B91
#define GL_STENCIL_FUNC 0x0B92
#define GL_STENCIL_VALUE_MASK 0x0B93
#define GL_STENCIL_FAIL 0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF 0x0B97
#define GL_STENCIL_WRITEMASK 0x0B98
#define GL_MATRIX_MODE 0x0BA0
#define GL_NORMALIZE 0x0BA1
#define GL_VIEWPORT 0x0BA2
#define GL_MODELVIEW_STACK_DEPTH 0x0BA3
#define GL_PROJECTION_STACK_DEPTH 0x0BA4
#define GL_TEXTURE_STACK_DEPTH 0x0BA5
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_TEXTURE_MATRIX 0x0BA8
#define GL_ATTRIB_STACK_DEPTH 0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH 0x0BB1
#define GL_ALPHA_TEST 0x0BC0
#define GL_ALPHA_TEST_FUNC 0x0BC1
#define GL_ALPHA_TEST_REF 0x0BC2
#define GL_DITHER 0x0BD0
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND 0x0BE2
#define GL_LOGIC_OP_MODE 0x0BF0
#define GL_INDEX_LOGIC_OP 0x0BF1
#define GL_COLOR_LOGIC_OP 0x0BF2
#define GL_AUX_BUFFERS 0x0C00
#define GL_DRAW_BUFFER 0x0C01
#define GL_READ_BUFFER 0x0C02
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_INDEX_CLEAR_VALUE 0x0C20
#define GL_INDEX_WRITEMASK 0x0C21
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_INDEX_MODE 0x0C30
#define GL_RGBA_MODE 0x0C31
#define GL_DOUBLEBUFFER 0x0C32
#define GL_STEREO 0x0C33
#define GL_RENDER_MODE 0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_POINT_SMOOTH_HINT 0x0C51
#define GL_LINE_SMOOTH_HINT 0x0C52
#define GL_POLYGON_SMOOTH_HINT 0x0C53
#define GL_FOG_HINT 0x0C54
#define GL_TEXTURE_GEN_S 0x0C60
#define GL_TEXTURE_GEN_T 0x0C61
#define GL_TEXTURE_GEN_R 0x0C62
#define GL_TEXTURE_GEN_Q 0x0C63
#define GL_PIXEL_MAP_I_TO_I 0x0C70
#define GL_PIXEL_MAP_S_TO_S 0x0C71
#define GL_PIXEL_MAP_I_TO_R 0x0C72
#define GL_PIXEL_MAP_I_TO_G 0x0C73
#define GL_PIXEL_MAP_I_TO_B 0x0C74
#define GL_PIXEL_MAP_I_TO_A 0x0C75
#define GL_PIXEL_MAP_R_TO_R 0x0C76
#define GL_PIXEL_MAP_G_TO_G 0x0C77
#define GL_PIXEL_MAP_B_TO_B 0x0C78
#define GL_PIXEL_MAP_A_TO_A 0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#define GL_UNPACK_LSB_FIRST 0x0CF1
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_SWAP_BYTES 0x0D00
#define GL_PACK_LSB_FIRST 0x0D01
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_MAP_COLOR 0x0D10
#define GL_MAP_STENCIL 0x0D11
#define GL_INDEX_SHIFT 0x0D12
#define GL_INDEX_OFFSET 0x0D13
#define GL_RED_SCALE 0x0D14
#define GL_RED_BIAS 0x0D15
#define GL_ZOOM_X 0x0D16
#define GL_ZOOM_Y 0x0D17
#define GL_GREEN_SCALE 0x0D18
#define GL_GREEN_BIAS 0x0D19
#define GL_BLUE_SCALE 0x0D1A
#define GL_BLUE_BIAS 0x0D1B
#define GL_ALPHA_SCALE 0x0D1C
#define GL_ALPHA_BIAS 0x0D1D
#define GL_DEPTH_SCALE 0x0D1E
#define GL_DEPTH_BIAS 0x0D1F
#define GL_MAX_EVAL_ORDER 0x0D30
#define GL_MAX_LIGHTS 0x0D31
#define GL_MAX_CLIP_PLANES 0x0D32
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_PIXEL_MAP_TABLE 0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH 0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH 0x0D36
#define GL_MAX_NAME_STACK_DEPTH 0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH 0x0D39
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH 0x0D3B
#define GL_SUBPIXEL_BITS 0x0D50
#define GL_INDEX_BITS 0x0D51
#define GL_RED_BITS 0x0D52
#define GL_GREEN_BITS 0x0D53
#define GL_BLUE_BITS 0x0D54
#define GL_ALPHA_BITS 0x0D55
#define GL_DEPTH_BITS 0x0D56
#define GL_STENCIL_BITS 0x0D57
#define GL_ACCUM_RED_BITS 0x0D58
#define GL_ACCUM_GREEN_BITS 0x0D59
#define GL_ACCUM_BLUE_BITS 0x0D5A
#define GL_ACCUM_ALPHA_BITS 0x0D5B
#define GL_NAME_STACK_DEPTH 0x0D70
#define GL_AUTO_NORMAL 0x0D80
#define GL_MAP1_COLOR_4 0x0D90
#define GL_MAP1_INDEX 0x0D91
#define GL_MAP1_NORMAL 0x0D92
#define GL_MAP1_TEXTURE_COORD_1 0x0D93
#define GL_MAP1_TEXTURE_COORD_2 0x0D94
#define GL_MAP1_TEXTURE_COORD_3 0x0D95
#define GL_MAP1_TEXTURE_COORD_4 0x0D96
#define GL_MAP1_VERTEX_3 0x0D97
#define GL_MAP1_VERTEX_4 0x0D98
#define GL_MAP2_COLOR_4 0x0DB0
#define GL_MAP2_INDEX 0x0DB1
#define GL_MAP2_NORMAL 0x0DB2
#define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#define GL_MAP2_VERTEX_3 0x0DB7
#define GL_MAP2_VERTEX_4 0x0DB8
#define GL_MAP1_GRID_DOMAIN 0x0DD0
#define GL_MAP1_GRID_SEGMENTS 0x0DD1
#define GL_MAP2_GRID_DOMAIN 0x0DD2
#define GL_MAP2_GRID_SEGMENTS 0x0DD3
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#define GL_SELECTION_BUFFER_POINTER 0x0DF3
#define GL_SELECTION_BUFFER_SIZE 0x0DF4
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TRANSFORM_BIT 0x00001000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_BORDER 0x1005
#define GL_DONT_CARE 0x1100
#define GL_FASTEST 0x1101
#define GL_NICEST 0x1102
#define GL_AMBIENT 0x1200
#define GL_DIFFUSE 0x1201
#define GL_SPECULAR 0x1202
#define GL_POSITION 0x1203
#define GL_SPOT_DIRECTION 0x1204
#define GL_SPOT_EXPONENT 0x1205
#define GL_SPOT_CUTOFF 0x1206
#define GL_CONSTANT_ATTENUATION 0x1207
#define GL_LINEAR_ATTENUATION 0x1208
#define GL_QUADRATIC_ATTENUATION 0x1209
#define GL_COMPILE 0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_2_BYTES 0x1407
#define GL_3_BYTES 0x1408
#define GL_4_BYTES 0x1409
#define GL_DOUBLE 0x140A
#define GL_CLEAR 0x1500
#define GL_AND 0x1501
#define GL_AND_REVERSE 0x1502
#define GL_COPY 0x1503
#define GL_AND_INVERTED 0x1504
#define GL_NOOP 0x1505
#define GL_XOR 0x1506
#define GL_OR 0x1507
#define GL_NOR 0x1508
#define GL_EQUIV 0x1509
#define GL_INVERT 0x150A
#define GL_OR_REVERSE 0x150B
#define GL_COPY_INVERTED 0x150C
#define GL_OR_INVERTED 0x150D
#define GL_NAND 0x150E
#define GL_SET 0x150F
#define GL_EMISSION 0x1600
#define GL_SHININESS 0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_COLOR_INDEXES 0x1603
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702
#define GL_COLOR 0x1800
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_COLOR_INDEX 0x1900
#define GL_STENCIL_INDEX 0x1901
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_BITMAP 0x1A00
#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_RENDER 0x1C00
#define GL_FEEDBACK 0x1C01
#define GL_SELECT 0x1C02
#define GL_FLAT 0x1D00
#define GL_SMOOTH 0x1D01
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_S 0x2000
#define GL_ENABLE_BIT 0x00002000
#define GL_T 0x2001
#define GL_R 0x2002
#define GL_Q 0x2003
#define GL_MODULATE 0x2100
#define GL_DECAL 0x2101
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_ENV_COLOR 0x2201
#define GL_TEXTURE_ENV 0x2300
#define GL_EYE_LINEAR 0x2400
#define GL_OBJECT_LINEAR 0x2401
#define GL_SPHERE_MAP 0x2402
#define GL_TEXTURE_GEN_MODE 0x2500
#define GL_OBJECT_PLANE 0x2501
#define GL_EYE_PLANE 0x2502
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP 0x2900
#define GL_REPEAT 0x2901
#define GL_POLYGON_OFFSET_UNITS 0x2A00
#define GL_POLYGON_OFFSET_POINT 0x2A01
#define GL_POLYGON_OFFSET_LINE 0x2A02
#define GL_R3_G3_B2 0x2A10
#define GL_V2F 0x2A20
#define GL_V3F 0x2A21
#define GL_C4UB_V2F 0x2A22
#define GL_C4UB_V3F 0x2A23
#define GL_C3F_V3F 0x2A24
#define GL_N3F_V3F 0x2A25
#define GL_C4F_N3F_V3F 0x2A26
#define GL_T2F_V3F 0x2A27
#define GL_T4F_V4F 0x2A28
#define GL_T2F_C4UB_V3F 0x2A29
#define GL_T2F_C3F_V3F 0x2A2A
#define GL_T2F_N3F_V3F 0x2A2B
#define GL_T2F_C4F_N3F_V3F 0x2A2C
#define GL_T4F_C4F_N3F_V4F 0x2A2D
#define GL_CLIP_PLANE0 0x3000
#define GL_CLIP_PLANE1 0x3001
#define GL_CLIP_PLANE2 0x3002
#define GL_CLIP_PLANE3 0x3003
#define GL_CLIP_PLANE4 0x3004
#define GL_CLIP_PLANE5 0x3005
#define GL_LIGHT0 0x4000
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_LIGHT1 0x4001
#define GL_LIGHT2 0x4002
#define GL_LIGHT3 0x4003
#define GL_LIGHT4 0x4004
#define GL_LIGHT5 0x4005
#define GL_LIGHT6 0x4006
#define GL_LIGHT7 0x4007
#define GL_HINT_BIT 0x00008000
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_POLYGON_OFFSET_FACTOR 0x8038
#define GL_ALPHA4 0x803B
#define GL_ALPHA8 0x803C
#define GL_ALPHA12 0x803D
#define GL_ALPHA16 0x803E
#define GL_LUMINANCE4 0x803F
#define GL_LUMINANCE8 0x8040
#define GL_LUMINANCE12 0x8041
#define GL_LUMINANCE16 0x8042
#define GL_LUMINANCE4_ALPHA4 0x8043
#define GL_LUMINANCE6_ALPHA2 0x8044
#define GL_LUMINANCE8_ALPHA8 0x8045
#define GL_LUMINANCE12_ALPHA4 0x8046
#define GL_LUMINANCE12_ALPHA12 0x8047
#define GL_LUMINANCE16_ALPHA16 0x8048
#define GL_INTENSITY 0x8049
#define GL_INTENSITY4 0x804A
#define GL_INTENSITY8 0x804B
#define GL_INTENSITY12 0x804C
#define GL_INTENSITY16 0x804D
#define GL_RGB4 0x804F
#define GL_RGB5 0x8050
#define GL_RGB8 0x8051
#define GL_RGB10 0x8052
#define GL_RGB12 0x8053
#define GL_RGB16 0x8054
#define GL_RGBA2 0x8055
#define GL_RGBA4 0x8056
#define GL_RGB5_A1 0x8057
#define GL_RGBA8 0x8058
#define GL_RGB10_A2 0x8059
#define GL_RGBA12 0x805A
#define GL_RGBA16 0x805B
#define GL_TEXTURE_RED_SIZE 0x805C
#define GL_TEXTURE_GREEN_SIZE 0x805D
#define GL_TEXTURE_BLUE_SIZE 0x805E
#define GL_TEXTURE_ALPHA_SIZE 0x805F
#define GL_TEXTURE_LUMINANCE_SIZE 0x8060
#define GL_TEXTURE_INTENSITY_SIZE 0x8061
#define GL_PROXY_TEXTURE_1D 0x8063
#define GL_PROXY_TEXTURE_2D 0x8064
#define GL_TEXTURE_PRIORITY 0x8066
#define GL_TEXTURE_RESIDENT 0x8067
#define GL_TEXTURE_BINDING_1D 0x8068
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_VERTEX_ARRAY 0x8074
#define GL_NORMAL_ARRAY 0x8075
#define GL_COLOR_ARRAY 0x8076
#define GL_INDEX_ARRAY 0x8077
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_EDGE_FLAG_ARRAY 0x8079
#define GL_VERTEX_ARRAY_SIZE 0x807A
#define GL_VERTEX_ARRAY_TYPE 0x807B
#define GL_VERTEX_ARRAY_STRIDE 0x807C
#define GL_NORMAL_ARRAY_TYPE 0x807E
#define GL_NORMAL_ARRAY_STRIDE 0x807F
#define GL_COLOR_ARRAY_SIZE 0x8081
#define GL_COLOR_ARRAY_TYPE 0x8082
#define GL_COLOR_ARRAY_STRIDE 0x8083
#define GL_INDEX_ARRAY_TYPE 0x8085
#define GL_INDEX_ARRAY_STRIDE 0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE 0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE 0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE 0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE 0x808C
#define GL_VERTEX_ARRAY_POINTER 0x808E
#define GL_NORMAL_ARRAY_POINTER 0x808F
#define GL_COLOR_ARRAY_POINTER 0x8090
#define GL_INDEX_ARRAY_POINTER 0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER 0x8093
#define GL_COLOR_INDEX1_EXT 0x80E2
#define GL_COLOR_INDEX2_EXT 0x80E3
#define GL_COLOR_INDEX4_EXT 0x80E4
#define GL_COLOR_INDEX8_EXT 0x80E5
#define GL_COLOR_INDEX12_EXT 0x80E6
#define GL_COLOR_INDEX16_EXT 0x80E7
#define GL_EVAL_BIT 0x00010000
#define GL_LIST_BIT 0x00020000
#define GL_TEXTURE_BIT 0x00040000
#define GL_SCISSOR_BIT 0x00080000
#define GL_ALL_ATTRIB_BITS 0x000fffff
#define GL_CLIENT_ALL_ATTRIB_BITS 0xffffffff

GLAPI void GLAPIENTRY glAccum (GLenum op, GLfloat value);
GLAPI void GLAPIENTRY glAlphaFunc (GLenum func, GLclampf ref);
GLAPI GLboolean GLAPIENTRY glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences);
GLAPI void GLAPIENTRY glArrayElement (GLint i);
GLAPI void GLAPIENTRY glBegin (GLenum mode);
GLAPI void GLAPIENTRY glBindTexture (GLenum target, GLuint texture);
GLAPI void GLAPIENTRY glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
GLAPI void GLAPIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor);
GLAPI void GLAPIENTRY glCallList (GLuint list);
GLAPI void GLAPIENTRY glCallLists (GLsizei n, GLenum type, const GLvoid *lists);
GLAPI void GLAPIENTRY glClear (GLbitfield mask);
GLAPI void GLAPIENTRY glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLAPI void GLAPIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GLAPI void GLAPIENTRY glClearDepth (GLclampd depth);
GLAPI void GLAPIENTRY glClearIndex (GLfloat c);
GLAPI void GLAPIENTRY glClearStencil (GLint s);
GLAPI void GLAPIENTRY glClipPlane (GLenum plane, const GLdouble *equation);
GLAPI void GLAPIENTRY glColor3b (GLbyte red, GLbyte green, GLbyte blue);
GLAPI void GLAPIENTRY glColor3bv (const GLbyte *v);
GLAPI void GLAPIENTRY glColor3d (GLdouble red, GLdouble green, GLdouble blue);
GLAPI void GLAPIENTRY glColor3dv (const GLdouble *v);
GLAPI void GLAPIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue);
GLAPI void GLAPIENTRY glColor3fv (const GLfloat *v);
GLAPI void GLAPIENTRY glColor3i (GLint red, GLint green, GLint blue);
GLAPI void GLAPIENTRY glColor3iv (const GLint *v);
GLAPI void GLAPIENTRY glColor3s (GLshort red, GLshort green, GLshort blue);
GLAPI void GLAPIENTRY glColor3sv (const GLshort *v);
GLAPI void GLAPIENTRY glColor3ub (GLubyte red, GLubyte green, GLubyte blue);
GLAPI void GLAPIENTRY glColor3ubv (const GLubyte *v);
GLAPI void GLAPIENTRY glColor3ui (GLuint red, GLuint green, GLuint blue);
GLAPI void GLAPIENTRY glColor3uiv (const GLuint *v);
GLAPI void GLAPIENTRY glColor3us (GLushort red, GLushort green, GLushort blue);
GLAPI void GLAPIENTRY glColor3usv (const GLushort *v);
GLAPI void GLAPIENTRY glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
GLAPI void GLAPIENTRY glColor4bv (const GLbyte *v);
GLAPI void GLAPIENTRY glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
GLAPI void GLAPIENTRY glColor4dv (const GLdouble *v);
GLAPI void GLAPIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GLAPI void GLAPIENTRY glColor4fv (const GLfloat *v);
GLAPI void GLAPIENTRY glColor4i (GLint red, GLint green, GLint blue, GLint alpha);
GLAPI void GLAPIENTRY glColor4iv (const GLint *v);
GLAPI void GLAPIENTRY glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
GLAPI void GLAPIENTRY glColor4sv (const GLshort *v);
GLAPI void GLAPIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
GLAPI void GLAPIENTRY glColor4ubv (const GLubyte *v);
GLAPI void GLAPIENTRY glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
GLAPI void GLAPIENTRY glColor4uiv (const GLuint *v);
GLAPI void GLAPIENTRY glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
GLAPI void GLAPIENTRY glColor4usv (const GLushort *v);
GLAPI void GLAPIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GLAPI void GLAPIENTRY glColorMaterial (GLenum face, GLenum mode);
GLAPI void GLAPIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void GLAPIENTRY glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
GLAPI void GLAPIENTRY glCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
GLAPI void GLAPIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GLAPI void GLAPIENTRY glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GLAPI void GLAPIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void GLAPIENTRY glCullFace (GLenum mode);
GLAPI void GLAPIENTRY glDeleteLists (GLuint list, GLsizei range);
GLAPI void GLAPIENTRY glDeleteTextures (GLsizei n, const GLuint *textures);
GLAPI void GLAPIENTRY glDepthFunc (GLenum func);
GLAPI void GLAPIENTRY glDepthMask (GLboolean flag);
GLAPI void GLAPIENTRY glDepthRange (GLclampd zNear, GLclampd zFar);
GLAPI void GLAPIENTRY glDisable (GLenum cap);
GLAPI void GLAPIENTRY glDisableClientState (GLenum array);
GLAPI void GLAPIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count);
GLAPI void GLAPIENTRY glDrawBuffer (GLenum mode);
GLAPI void GLAPIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
GLAPI void GLAPIENTRY glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void GLAPIENTRY glEdgeFlag (GLboolean flag);
GLAPI void GLAPIENTRY glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer);
GLAPI void GLAPIENTRY glEdgeFlagv (const GLboolean *flag);
GLAPI void GLAPIENTRY glEnable (GLenum cap);
GLAPI void GLAPIENTRY glEnableClientState (GLenum array);
GLAPI void GLAPIENTRY glEnd (void);
GLAPI void GLAPIENTRY glEndList (void);
GLAPI void GLAPIENTRY glEvalCoord1d (GLdouble u);
GLAPI void GLAPIENTRY glEvalCoord1dv (const GLdouble *u);
GLAPI void GLAPIENTRY glEvalCoord1f (GLfloat u);
GLAPI void GLAPIENTRY glEvalCoord1fv (const GLfloat *u);
GLAPI void GLAPIENTRY glEvalCoord2d (GLdouble u, GLdouble v);
GLAPI void GLAPIENTRY glEvalCoord2dv (const GLdouble *u);
GLAPI void GLAPIENTRY glEvalCoord2f (GLfloat u, GLfloat v);
GLAPI void GLAPIENTRY glEvalCoord2fv (const GLfloat *u);
GLAPI void GLAPIENTRY glEvalMesh1 (GLenum mode, GLint i1, GLint i2);
GLAPI void GLAPIENTRY glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
GLAPI void GLAPIENTRY glEvalPoint1 (GLint i);
GLAPI void GLAPIENTRY glEvalPoint2 (GLint i, GLint j);
GLAPI void GLAPIENTRY glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer);
GLAPI void GLAPIENTRY glFinish (void);
GLAPI void GLAPIENTRY glFlush (void);
GLAPI void GLAPIENTRY glFogf (GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glFogfv (GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glFogi (GLenum pname, GLint param);
GLAPI void GLAPIENTRY glFogiv (GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glFrontFace (GLenum mode);
GLAPI void GLAPIENTRY glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLAPI GLuint GLAPIENTRY glGenLists (GLsizei range);
GLAPI void GLAPIENTRY glGenTextures (GLsizei n, GLuint *textures);
GLAPI void GLAPIENTRY glGetBooleanv (GLenum pname, GLboolean *params);
GLAPI void GLAPIENTRY glGetClipPlane (GLenum plane, GLdouble *equation);
GLAPI void GLAPIENTRY glGetDoublev (GLenum pname, GLdouble *params);
GLAPI GLenum GLAPIENTRY glGetError (void);
GLAPI void GLAPIENTRY glGetFloatv (GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetIntegerv (GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetLightiv (GLenum light, GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glGetMapdv (GLenum target, GLenum query, GLdouble *v);
GLAPI void GLAPIENTRY glGetMapfv (GLenum target, GLenum query, GLfloat *v);
GLAPI void GLAPIENTRY glGetMapiv (GLenum target, GLenum query, GLint *v);
GLAPI void GLAPIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetMaterialiv (GLenum face, GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glGetPixelMapfv (GLenum map, GLfloat *values);
GLAPI void GLAPIENTRY glGetPixelMapuiv (GLenum map, GLuint *values);
GLAPI void GLAPIENTRY glGetPixelMapusv (GLenum map, GLushort *values);
GLAPI void GLAPIENTRY glGetPointerv (GLenum pname, GLvoid* *params);
GLAPI void GLAPIENTRY glGetPolygonStipple (GLubyte *mask);
GLAPI const GLubyte * GLAPIENTRY glGetString (GLenum name);
GLAPI void GLAPIENTRY glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetTexEnviv (GLenum target, GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params);
GLAPI void GLAPIENTRY glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetTexGeniv (GLenum coord, GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
GLAPI void GLAPIENTRY glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI void GLAPIENTRY glHint (GLenum target, GLenum mode);
GLAPI void GLAPIENTRY glIndexMask (GLuint mask);
GLAPI void GLAPIENTRY glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void GLAPIENTRY glIndexd (GLdouble c);
GLAPI void GLAPIENTRY glIndexdv (const GLdouble *c);
GLAPI void GLAPIENTRY glIndexf (GLfloat c);
GLAPI void GLAPIENTRY glIndexfv (const GLfloat *c);
GLAPI void GLAPIENTRY glIndexi (GLint c);
GLAPI void GLAPIENTRY glIndexiv (const GLint *c);
GLAPI void GLAPIENTRY glIndexs (GLshort c);
GLAPI void GLAPIENTRY glIndexsv (const GLshort *c);
GLAPI void GLAPIENTRY glIndexub (GLubyte c);
GLAPI void GLAPIENTRY glIndexubv (const GLubyte *c);
GLAPI void GLAPIENTRY glInitNames (void);
GLAPI void GLAPIENTRY glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer);
GLAPI GLboolean GLAPIENTRY glIsEnabled (GLenum cap);
GLAPI GLboolean GLAPIENTRY glIsList (GLuint list);
GLAPI GLboolean GLAPIENTRY glIsTexture (GLuint texture);
GLAPI void GLAPIENTRY glLightModelf (GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glLightModelfv (GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glLightModeli (GLenum pname, GLint param);
GLAPI void GLAPIENTRY glLightModeliv (GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glLightf (GLenum light, GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glLighti (GLenum light, GLenum pname, GLint param);
GLAPI void GLAPIENTRY glLightiv (GLenum light, GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glLineStipple (GLint factor, GLushort pattern);
GLAPI void GLAPIENTRY glLineWidth (GLfloat width);
GLAPI void GLAPIENTRY glListBase (GLuint base);
GLAPI void GLAPIENTRY glLoadIdentity (void);
GLAPI void GLAPIENTRY glLoadMatrixd (const GLdouble *m);
GLAPI void GLAPIENTRY glLoadMatrixf (const GLfloat *m);
GLAPI void GLAPIENTRY glLoadName (GLuint name);
GLAPI void GLAPIENTRY glLogicOp (GLenum opcode);
GLAPI void GLAPIENTRY glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
GLAPI void GLAPIENTRY glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
GLAPI void GLAPIENTRY glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
GLAPI void GLAPIENTRY glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
GLAPI void GLAPIENTRY glMapGrid1d (GLint un, GLdouble u1, GLdouble u2);
GLAPI void GLAPIENTRY glMapGrid1f (GLint un, GLfloat u1, GLfloat u2);
GLAPI void GLAPIENTRY glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
GLAPI void GLAPIENTRY glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
GLAPI void GLAPIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glMateriali (GLenum face, GLenum pname, GLint param);
GLAPI void GLAPIENTRY glMaterialiv (GLenum face, GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glMatrixMode (GLenum mode);
GLAPI void GLAPIENTRY glMultMatrixd (const GLdouble *m);
GLAPI void GLAPIENTRY glMultMatrixf (const GLfloat *m);
GLAPI void GLAPIENTRY glNewList (GLuint list, GLenum mode);
GLAPI void GLAPIENTRY glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz);
GLAPI void GLAPIENTRY glNormal3bv (const GLbyte *v);
GLAPI void GLAPIENTRY glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz);
GLAPI void GLAPIENTRY glNormal3dv (const GLdouble *v);
GLAPI void GLAPIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz);
GLAPI void GLAPIENTRY glNormal3fv (const GLfloat *v);
GLAPI void GLAPIENTRY glNormal3i (GLint nx, GLint ny, GLint nz);
GLAPI void GLAPIENTRY glNormal3iv (const GLint *v);
GLAPI void GLAPIENTRY glNormal3s (GLshort nx, GLshort ny, GLshort nz);
GLAPI void GLAPIENTRY glNormal3sv (const GLshort *v);
GLAPI void GLAPIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void GLAPIENTRY glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLAPI void GLAPIENTRY glPassThrough (GLfloat token);
GLAPI void GLAPIENTRY glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat *values);
GLAPI void GLAPIENTRY glPixelMapuiv (GLenum map, GLsizei mapsize, const GLuint *values);
GLAPI void GLAPIENTRY glPixelMapusv (GLenum map, GLsizei mapsize, const GLushort *values);
GLAPI void GLAPIENTRY glPixelStoref (GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glPixelStorei (GLenum pname, GLint param);
GLAPI void GLAPIENTRY glPixelTransferf (GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glPixelTransferi (GLenum pname, GLint param);
GLAPI void GLAPIENTRY glPixelZoom (GLfloat xfactor, GLfloat yfactor);
GLAPI void GLAPIENTRY glPointSize (GLfloat size);
GLAPI void GLAPIENTRY glPolygonMode (GLenum face, GLenum mode);
GLAPI void GLAPIENTRY glPolygonOffset (GLfloat factor, GLfloat units);
GLAPI void GLAPIENTRY glPolygonStipple (const GLubyte *mask);
GLAPI void GLAPIENTRY glPopAttrib (void);
GLAPI void GLAPIENTRY glPopClientAttrib (void);
GLAPI void GLAPIENTRY glPopMatrix (void);
GLAPI void GLAPIENTRY glPopName (void);
GLAPI void GLAPIENTRY glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities);
GLAPI void GLAPIENTRY glPushAttrib (GLbitfield mask);
GLAPI void GLAPIENTRY glPushClientAttrib (GLbitfield mask);
GLAPI void GLAPIENTRY glPushMatrix (void);
GLAPI void GLAPIENTRY glPushName (GLuint name);
GLAPI void GLAPIENTRY glRasterPos2d (GLdouble x, GLdouble y);
GLAPI void GLAPIENTRY glRasterPos2dv (const GLdouble *v);
GLAPI void GLAPIENTRY glRasterPos2f (GLfloat x, GLfloat y);
GLAPI void GLAPIENTRY glRasterPos2fv (const GLfloat *v);
GLAPI void GLAPIENTRY glRasterPos2i (GLint x, GLint y);
GLAPI void GLAPIENTRY glRasterPos2iv (const GLint *v);
GLAPI void GLAPIENTRY glRasterPos2s (GLshort x, GLshort y);
GLAPI void GLAPIENTRY glRasterPos2sv (const GLshort *v);
GLAPI void GLAPIENTRY glRasterPos3d (GLdouble x, GLdouble y, GLdouble z);
GLAPI void GLAPIENTRY glRasterPos3dv (const GLdouble *v);
GLAPI void GLAPIENTRY glRasterPos3f (GLfloat x, GLfloat y, GLfloat z);
GLAPI void GLAPIENTRY glRasterPos3fv (const GLfloat *v);
GLAPI void GLAPIENTRY glRasterPos3i (GLint x, GLint y, GLint z);
GLAPI void GLAPIENTRY glRasterPos3iv (const GLint *v);
GLAPI void GLAPIENTRY glRasterPos3s (GLshort x, GLshort y, GLshort z);
GLAPI void GLAPIENTRY glRasterPos3sv (const GLshort *v);
GLAPI void GLAPIENTRY glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLAPI void GLAPIENTRY glRasterPos4dv (const GLdouble *v);
GLAPI void GLAPIENTRY glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLAPI void GLAPIENTRY glRasterPos4fv (const GLfloat *v);
GLAPI void GLAPIENTRY glRasterPos4i (GLint x, GLint y, GLint z, GLint w);
GLAPI void GLAPIENTRY glRasterPos4iv (const GLint *v);
GLAPI void GLAPIENTRY glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w);
GLAPI void GLAPIENTRY glRasterPos4sv (const GLshort *v);
GLAPI void GLAPIENTRY glReadBuffer (GLenum mode);
GLAPI void GLAPIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
GLAPI void GLAPIENTRY glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
GLAPI void GLAPIENTRY glRectdv (const GLdouble *v1, const GLdouble *v2);
GLAPI void GLAPIENTRY glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
GLAPI void GLAPIENTRY glRectfv (const GLfloat *v1, const GLfloat *v2);
GLAPI void GLAPIENTRY glRecti (GLint x1, GLint y1, GLint x2, GLint y2);
GLAPI void GLAPIENTRY glRectiv (const GLint *v1, const GLint *v2);
GLAPI void GLAPIENTRY glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
GLAPI void GLAPIENTRY glRectsv (const GLshort *v1, const GLshort *v2);
GLAPI GLint GLAPIENTRY glRenderMode (GLenum mode);
GLAPI void GLAPIENTRY glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
GLAPI void GLAPIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
GLAPI void GLAPIENTRY glScaled (GLdouble x, GLdouble y, GLdouble z);
GLAPI void GLAPIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z);
GLAPI void GLAPIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void GLAPIENTRY glSelectBuffer (GLsizei size, GLuint *buffer);
GLAPI void GLAPIENTRY glShadeModel (GLenum mode);
GLAPI void GLAPIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask);
GLAPI void GLAPIENTRY glStencilMask (GLuint mask);
GLAPI void GLAPIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
GLAPI void GLAPIENTRY glTexCoord1d (GLdouble s);
GLAPI void GLAPIENTRY glTexCoord1dv (const GLdouble *v);
GLAPI void GLAPIENTRY glTexCoord1f (GLfloat s);
GLAPI void GLAPIENTRY glTexCoord1fv (const GLfloat *v);
GLAPI void GLAPIENTRY glTexCoord1i (GLint s);
GLAPI void GLAPIENTRY glTexCoord1iv (const GLint *v);
GLAPI void GLAPIENTRY glTexCoord1s (GLshort s);
GLAPI void GLAPIENTRY glTexCoord1sv (const GLshort *v);
GLAPI void GLAPIENTRY glTexCoord2d (GLdouble s, GLdouble t);
GLAPI void GLAPIENTRY glTexCoord2dv (const GLdouble *v);
GLAPI void GLAPIENTRY glTexCoord2f (GLfloat s, GLfloat t);
GLAPI void GLAPIENTRY glTexCoord2fv (const GLfloat *v);
GLAPI void GLAPIENTRY glTexCoord2i (GLint s, GLint t);
GLAPI void GLAPIENTRY glTexCoord2iv (const GLint *v);
GLAPI void GLAPIENTRY glTexCoord2s (GLshort s, GLshort t);
GLAPI void GLAPIENTRY glTexCoord2sv (const GLshort *v);
GLAPI void GLAPIENTRY glTexCoord3d (GLdouble s, GLdouble t, GLdouble r);
GLAPI void GLAPIENTRY glTexCoord3dv (const GLdouble *v);
GLAPI void GLAPIENTRY glTexCoord3f (GLfloat s, GLfloat t, GLfloat r);
GLAPI void GLAPIENTRY glTexCoord3fv (const GLfloat *v);
GLAPI void GLAPIENTRY glTexCoord3i (GLint s, GLint t, GLint r);
GLAPI void GLAPIENTRY glTexCoord3iv (const GLint *v);
GLAPI void GLAPIENTRY glTexCoord3s (GLshort s, GLshort t, GLshort r);
GLAPI void GLAPIENTRY glTexCoord3sv (const GLshort *v);
GLAPI void GLAPIENTRY glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GLAPI void GLAPIENTRY glTexCoord4dv (const GLdouble *v);
GLAPI void GLAPIENTRY glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GLAPI void GLAPIENTRY glTexCoord4fv (const GLfloat *v);
GLAPI void GLAPIENTRY glTexCoord4i (GLint s, GLint t, GLint r, GLint q);
GLAPI void GLAPIENTRY glTexCoord4iv (const GLint *v);
GLAPI void GLAPIENTRY glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q);
GLAPI void GLAPIENTRY glTexCoord4sv (const GLshort *v);
GLAPI void GLAPIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void GLAPIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param);
GLAPI void GLAPIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glTexGend (GLenum coord, GLenum pname, GLdouble param);
GLAPI void GLAPIENTRY glTexGendv (GLenum coord, GLenum pname, const GLdouble *params);
GLAPI void GLAPIENTRY glTexGenf (GLenum coord, GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glTexGeni (GLenum coord, GLenum pname, GLint param);
GLAPI void GLAPIENTRY glTexGeniv (GLenum coord, GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void GLAPIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void GLAPIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param);
GLAPI void GLAPIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
GLAPI void GLAPIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param);
GLAPI void GLAPIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
GLAPI void GLAPIENTRY glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void GLAPIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void GLAPIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z);
GLAPI void GLAPIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z);
GLAPI void GLAPIENTRY glVertex2d (GLdouble x, GLdouble y);
GLAPI void GLAPIENTRY glVertex2dv (const GLdouble *v);
GLAPI void GLAPIENTRY glVertex2f (GLfloat x, GLfloat y);
GLAPI void GLAPIENTRY glVertex2fv (const GLfloat *v);
GLAPI void GLAPIENTRY glVertex2i (GLint x, GLint y);
GLAPI void GLAPIENTRY glVertex2iv (const GLint *v);
GLAPI void GLAPIENTRY glVertex2s (GLshort x, GLshort y);
GLAPI void GLAPIENTRY glVertex2sv (const GLshort *v);
GLAPI void GLAPIENTRY glVertex3d (GLdouble x, GLdouble y, GLdouble z);
GLAPI void GLAPIENTRY glVertex3dv (const GLdouble *v);
GLAPI void GLAPIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z);
GLAPI void GLAPIENTRY glVertex3fv (const GLfloat *v);
GLAPI void GLAPIENTRY glVertex3i (GLint x, GLint y, GLint z);
GLAPI void GLAPIENTRY glVertex3iv (const GLint *v);
GLAPI void GLAPIENTRY glVertex3s (GLshort x, GLshort y, GLshort z);
GLAPI void GLAPIENTRY glVertex3sv (const GLshort *v);
GLAPI void GLAPIENTRY glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLAPI void GLAPIENTRY glVertex4dv (const GLdouble *v);
GLAPI void GLAPIENTRY glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLAPI void GLAPIENTRY glVertex4fv (const GLfloat *v);
GLAPI void GLAPIENTRY glVertex4i (GLint x, GLint y, GLint z, GLint w);
GLAPI void GLAPIENTRY glVertex4iv (const GLint *v);
GLAPI void GLAPIENTRY glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w);
GLAPI void GLAPIENTRY glVertex4sv (const GLshort *v);
GLAPI void GLAPIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void GLAPIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

#define GLEW_VERSION_1_1 GLEW_GET_VAR(__GLEW_VERSION_1_1)

#endif /* GL_VERSION_1_1 */

/* ---------------------------------- GLU ---------------------------------- */

#ifndef GLEW_NO_GLU
/* this is where we can safely include GLU */
#  if defined(__APPLE__) && defined(__MACH__)
#    include <OpenGL/glu.h>
#  else
#    include <GL/glu.h>
#  endif
#endif

/* ----------------------------- GL_VERSION_1_2 ---------------------------- */

#ifndef GL_VERSION_1_2
#define GL_VERSION_1_2 1

#define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#define GL_RESCALE_NORMAL 0x803A
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_PACK_SKIP_IMAGES 0x806B
#define GL_PACK_IMAGE_HEIGHT 0x806C
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_TEXTURE_3D 0x806F
#define GL_PROXY_TEXTURE_3D 0x8070
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#define GL_SINGLE_COLOR 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E

typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (GLAPIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);

#define glCopyTexSubImage3D GLEW_GET_FUN(__glewCopyTexSubImage3D)
#define glDrawRangeElements GLEW_GET_FUN(__glewDrawRangeElements)
#define glTexImage3D GLEW_GET_FUN(__glewTexImage3D)
#define glTexSubImage3D GLEW_GET_FUN(__glewTexSubImage3D)

#define GLEW_VERSION_1_2 GLEW_GET_VAR(__GLEW_VERSION_1_2)

#endif /* GL_VERSION_1_2 */

/* ---------------------------- GL_VERSION_1_2_1 --------------------------- */

#ifndef GL_VERSION_1_2_1
#define GL_VERSION_1_2_1 1

#define GLEW_VERSION_1_2_1 GLEW_GET_VAR(__GLEW_VERSION_1_2_1)

#endif /* GL_VERSION_1_2_1 */

/* ----------------------------- GL_VERSION_1_3 ---------------------------- */

#ifndef GL_VERSION_1_3
#define GL_VERSION_1_3 1

#define GL_MULTISAMPLE 0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_MAX_TEXTURE_UNITS 0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#define GL_SUBTRACT 0x84E7
#define GL_COMPRESSED_ALPHA 0x84E9
#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#define GL_COMPRESSED_INTENSITY 0x84EC
#define GL_COMPRESSED_RGB 0x84ED
#define GL_COMPRESSED_RGBA 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#define GL_NORMAL_MAP 0x8511
#define GL_REFLECTION_MAP 0x8512
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_RGB_SCALE 0x8573
#define GL_ADD_SIGNED 0x8574
#define GL_INTERPOLATE 0x8575
#define GL_CONSTANT 0x8576
#define GL_PRIMARY_COLOR 0x8577
#define GL_PREVIOUS 0x8578
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_RGB 0x8582
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_SOURCE2_ALPHA 0x858A
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_RGB 0x8592
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_OPERAND2_ALPHA 0x859A
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_DOT3_RGB 0x86AE
#define GL_DOT3_RGBA 0x86AF
#define GL_MULTISAMPLE_BIT 0x20000000

typedef void (GLAPIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint lod, GLvoid *img);
typedef void (GLAPIENTRY * PFNGLLOADTRANSPOSEMATRIXDPROC) (const GLdouble m[16]);
typedef void (GLAPIENTRY * PFNGLLOADTRANSPOSEMATRIXFPROC) (const GLfloat m[16]);
typedef void (GLAPIENTRY * PFNGLMULTTRANSPOSEMATRIXDPROC) (const GLdouble m[16]);
typedef void (GLAPIENTRY * PFNGLMULTTRANSPOSEMATRIXFPROC) (const GLfloat m[16]);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DPROC) (GLenum target, GLdouble s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DVPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FPROC) (GLenum target, GLfloat s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FVPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IPROC) (GLenum target, GLint s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IVPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SPROC) (GLenum target, GLshort s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SVPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DVPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IPROC) (GLenum target, GLint s, GLint t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IVPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SPROC) (GLenum target, GLshort s, GLshort t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SVPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DVPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FVPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IVPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SVPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DVPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FVPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IVPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SVPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);

#define glActiveTexture GLEW_GET_FUN(__glewActiveTexture)
#define glClientActiveTexture GLEW_GET_FUN(__glewClientActiveTexture)
#define glCompressedTexImage1D GLEW_GET_FUN(__glewCompressedTexImage1D)
#define glCompressedTexImage2D GLEW_GET_FUN(__glewCompressedTexImage2D)
#define glCompressedTexImage3D GLEW_GET_FUN(__glewCompressedTexImage3D)
#define glCompressedTexSubImage1D GLEW_GET_FUN(__glewCompressedTexSubImage1D)
#define glCompressedTexSubImage2D GLEW_GET_FUN(__glewCompressedTexSubImage2D)
#define glCompressedTexSubImage3D GLEW_GET_FUN(__glewCompressedTexSubImage3D)
#define glGetCompressedTexImage GLEW_GET_FUN(__glewGetCompressedTexImage)
#define glLoadTransposeMatrixd GLEW_GET_FUN(__glewLoadTransposeMatrixd)
#define glLoadTransposeMatrixf GLEW_GET_FUN(__glewLoadTransposeMatrixf)
#define glMultTransposeMatrixd GLEW_GET_FUN(__glewMultTransposeMatrixd)
#define glMultTransposeMatrixf GLEW_GET_FUN(__glewMultTransposeMatrixf)
#define glMultiTexCoord1d GLEW_GET_FUN(__glewMultiTexCoord1d)
#define glMultiTexCoord1dv GLEW_GET_FUN(__glewMultiTexCoord1dv)
#define glMultiTexCoord1f GLEW_GET_FUN(__glewMultiTexCoord1f)
#define glMultiTexCoord1fv GLEW_GET_FUN(__glewMultiTexCoord1fv)
#define glMultiTexCoord1i GLEW_GET_FUN(__glewMultiTexCoord1i)
#define glMultiTexCoord1iv GLEW_GET_FUN(__glewMultiTexCoord1iv)
#define glMultiTexCoord1s GLEW_GET_FUN(__glewMultiTexCoord1s)
#define glMultiTexCoord1sv GLEW_GET_FUN(__glewMultiTexCoord1sv)
#define glMultiTexCoord2d GLEW_GET_FUN(__glewMultiTexCoord2d)
#define glMultiTexCoord2dv GLEW_GET_FUN(__glewMultiTexCoord2dv)
#define glMultiTexCoord2f GLEW_GET_FUN(__glewMultiTexCoord2f)
#define glMultiTexCoord2fv GLEW_GET_FUN(__glewMultiTexCoord2fv)
#define glMultiTexCoord2i GLEW_GET_FUN(__glewMultiTexCoord2i)
#define glMultiTexCoord2iv GLEW_GET_FUN(__glewMultiTexCoord2iv)
#define glMultiTexCoord2s GLEW_GET_FUN(__glewMultiTexCoord2s)
#define glMultiTexCoord2sv GLEW_GET_FUN(__glewMultiTexCoord2sv)
#define glMultiTexCoord3d GLEW_GET_FUN(__glewMultiTexCoord3d)
#define glMultiTexCoord3dv GLEW_GET_FUN(__glewMultiTexCoord3dv)
#define glMultiTexCoord3f GLEW_GET_FUN(__glewMultiTexCoord3f)
#define glMultiTexCoord3fv GLEW_GET_FUN(__glewMultiTexCoord3fv)
#define glMultiTexCoord3i GLEW_GET_FUN(__glewMultiTexCoord3i)
#define glMultiTexCoord3iv GLEW_GET_FUN(__glewMultiTexCoord3iv)
#define glMultiTexCoord3s GLEW_GET_FUN(__glewMultiTexCoord3s)
#define glMultiTexCoord3sv GLEW_GET_FUN(__glewMultiTexCoord3sv)
#define glMultiTexCoord4d GLEW_GET_FUN(__glewMultiTexCoord4d)
#define glMultiTexCoord4dv GLEW_GET_FUN(__glewMultiTexCoord4dv)
#define glMultiTexCoord4f GLEW_GET_FUN(__glewMultiTexCoord4f)
#define glMultiTexCoord4fv GLEW_GET_FUN(__glewMultiTexCoord4fv)
#define glMultiTexCoord4i GLEW_GET_FUN(__glewMultiTexCoord4i)
#define glMultiTexCoord4iv GLEW_GET_FUN(__glewMultiTexCoord4iv)
#define glMultiTexCoord4s GLEW_GET_FUN(__glewMultiTexCoord4s)
#define glMultiTexCoord4sv GLEW_GET_FUN(__glewMultiTexCoord4sv)
#define glSampleCoverage GLEW_GET_FUN(__glewSampleCoverage)

#define GLEW_VERSION_1_3 GLEW_GET_VAR(__GLEW_VERSION_1_3)

#endif /* GL_VERSION_1_3 */

/* ----------------------------- GL_VERSION_1_4 ---------------------------- */

#ifndef GL_VERSION_1_4
#define GL_VERSION_1_4 1

#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_MIRRORED_REPEAT 0x8370
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FOG_COORDINATE 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_R_TO_TEXTURE 0x884E

typedef void (GLAPIENTRY * PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (GLAPIENTRY * PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GLAPIENTRY * PFNGLFOGCOORDPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLFOGCOORDDPROC) (GLdouble coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDDVPROC) (const GLdouble *coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDFPROC) (GLfloat coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDFVPROC) (const GLfloat *coord);
typedef void (GLAPIENTRY * PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
typedef void (GLAPIENTRY * PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei drawcount);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3BPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3BVPROC) (const GLbyte *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3DPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3DVPROC) (const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3FPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3FVPROC) (const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3IPROC) (GLint red, GLint green, GLint blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3IVPROC) (const GLint *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3SPROC) (GLshort red, GLshort green, GLshort blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3SVPROC) (const GLshort *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3UBPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3UBVPROC) (const GLubyte *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3UIPROC) (GLuint red, GLuint green, GLuint blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3UIVPROC) (const GLuint *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3USPROC) (GLushort red, GLushort green, GLushort blue);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3USVPROC) (const GLushort *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLORPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2DPROC) (GLdouble x, GLdouble y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2DVPROC) (const GLdouble *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2FPROC) (GLfloat x, GLfloat y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2FVPROC) (const GLfloat *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2IPROC) (GLint x, GLint y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2IVPROC) (const GLint *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2SPROC) (GLshort x, GLshort y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2SVPROC) (const GLshort *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3DPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3DVPROC) (const GLdouble *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3FPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3FVPROC) (const GLfloat *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3IPROC) (GLint x, GLint y, GLint z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3IVPROC) (const GLint *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3SPROC) (GLshort x, GLshort y, GLshort z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3SVPROC) (const GLshort *p);

#define glBlendColor GLEW_GET_FUN(__glewBlendColor)
#define glBlendEquation GLEW_GET_FUN(__glewBlendEquation)
#define glBlendFuncSeparate GLEW_GET_FUN(__glewBlendFuncSeparate)
#define glFogCoordPointer GLEW_GET_FUN(__glewFogCoordPointer)
#define glFogCoordd GLEW_GET_FUN(__glewFogCoordd)
#define glFogCoorddv GLEW_GET_FUN(__glewFogCoorddv)
#define glFogCoordf GLEW_GET_FUN(__glewFogCoordf)
#define glFogCoordfv GLEW_GET_FUN(__glewFogCoordfv)
#define glMultiDrawArrays GLEW_GET_FUN(__glewMultiDrawArrays)
#define glMultiDrawElements GLEW_GET_FUN(__glewMultiDrawElements)
#define glPointParameterf GLEW_GET_FUN(__glewPointParameterf)
#define glPointParameterfv GLEW_GET_FUN(__glewPointParameterfv)
#define glPointParameteri GLEW_GET_FUN(__glewPointParameteri)
#define glPointParameteriv GLEW_GET_FUN(__glewPointParameteriv)
#define glSecondaryColor3b GLEW_GET_FUN(__glewSecondaryColor3b)
#define glSecondaryColor3bv GLEW_GET_FUN(__glewSecondaryColor3bv)
#define glSecondaryColor3d GLEW_GET_FUN(__glewSecondaryColor3d)
#define glSecondaryColor3dv GLEW_GET_FUN(__glewSecondaryColor3dv)
#define glSecondaryColor3f GLEW_GET_FUN(__glewSecondaryColor3f)
#define glSecondaryColor3fv GLEW_GET_FUN(__glewSecondaryColor3fv)
#define glSecondaryColor3i GLEW_GET_FUN(__glewSecondaryColor3i)
#define glSecondaryColor3iv GLEW_GET_FUN(__glewSecondaryColor3iv)
#define glSecondaryColor3s GLEW_GET_FUN(__glewSecondaryColor3s)
#define glSecondaryColor3sv GLEW_GET_FUN(__glewSecondaryColor3sv)
#define glSecondaryColor3ub GLEW_GET_FUN(__glewSecondaryColor3ub)
#define glSecondaryColor3ubv GLEW_GET_FUN(__glewSecondaryColor3ubv)
#define glSecondaryColor3ui GLEW_GET_FUN(__glewSecondaryColor3ui)
#define glSecondaryColor3uiv GLEW_GET_FUN(__glewSecondaryColor3uiv)
#define glSecondaryColor3us GLEW_GET_FUN(__glewSecondaryColor3us)
#define glSecondaryColor3usv GLEW_GET_FUN(__glewSecondaryColor3usv)
#define glSecondaryColorPointer GLEW_GET_FUN(__glewSecondaryColorPointer)
#define glWindowPos2d GLEW_GET_FUN(__glewWindowPos2d)
#define glWindowPos2dv GLEW_GET_FUN(__glewWindowPos2dv)
#define glWindowPos2f GLEW_GET_FUN(__glewWindowPos2f)
#define glWindowPos2fv GLEW_GET_FUN(__glewWindowPos2fv)
#define glWindowPos2i GLEW_GET_FUN(__glewWindowPos2i)
#define glWindowPos2iv GLEW_GET_FUN(__glewWindowPos2iv)
#define glWindowPos2s GLEW_GET_FUN(__glewWindowPos2s)
#define glWindowPos2sv GLEW_GET_FUN(__glewWindowPos2sv)
#define glWindowPos3d GLEW_GET_FUN(__glewWindowPos3d)
#define glWindowPos3dv GLEW_GET_FUN(__glewWindowPos3dv)
#define glWindowPos3f GLEW_GET_FUN(__glewWindowPos3f)
#define glWindowPos3fv GLEW_GET_FUN(__glewWindowPos3fv)
#define glWindowPos3i GLEW_GET_FUN(__glewWindowPos3i)
#define glWindowPos3iv GLEW_GET_FUN(__glewWindowPos3iv)
#define glWindowPos3s GLEW_GET_FUN(__glewWindowPos3s)
#define glWindowPos3sv GLEW_GET_FUN(__glewWindowPos3sv)

#define GLEW_VERSION_1_4 GLEW_GET_VAR(__GLEW_VERSION_1_4)

#endif /* GL_VERSION_1_4 */

/* ----------------------------- GL_VERSION_1_5 ---------------------------- */

#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1

#define GL_FOG_COORD_SRC GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORD GL_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY GL_FOG_COORDINATE_ARRAY
#define GL_SRC0_RGB GL_SOURCE0_RGB
#define GL_FOG_COORD_ARRAY_POINTER GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORD_ARRAY_TYPE GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_SRC1_ALPHA GL_SOURCE1_ALPHA
#define GL_CURRENT_FOG_COORD GL_CURRENT_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY_STRIDE GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_SRC0_ALPHA GL_SOURCE0_ALPHA
#define GL_SRC1_RGB GL_SOURCE1_RGB
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_SRC2_ALPHA GL_SOURCE2_ALPHA
#define GL_SRC2_RGB GL_SOURCE2_RGB
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914

typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

typedef void (GLAPIENTRY * PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void (GLAPIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (GLAPIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void (GLAPIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef void (GLAPIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void (GLAPIENTRY * PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint* ids);
typedef void (GLAPIENTRY * PFNGLENDQUERYPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (GLAPIENTRY * PFNGLGENQUERIESPROC) (GLsizei n, GLuint* ids);
typedef void (GLAPIENTRY * PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void (GLAPIENTRY * PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid** params);
typedef void (GLAPIENTRY * PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
typedef void (GLAPIENTRY * PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint* params);
typedef void (GLAPIENTRY * PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint* params);
typedef void (GLAPIENTRY * PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLboolean (GLAPIENTRY * PFNGLISBUFFERPROC) (GLuint buffer);
typedef GLboolean (GLAPIENTRY * PFNGLISQUERYPROC) (GLuint id);
typedef GLvoid* (GLAPIENTRY * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean (GLAPIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);

#define glBeginQuery GLEW_GET_FUN(__glewBeginQuery)
#define glBindBuffer GLEW_GET_FUN(__glewBindBuffer)
#define glBufferData GLEW_GET_FUN(__glewBufferData)
#define glBufferSubData GLEW_GET_FUN(__glewBufferSubData)
#define glDeleteBuffers GLEW_GET_FUN(__glewDeleteBuffers)
#define glDeleteQueries GLEW_GET_FUN(__glewDeleteQueries)
#define glEndQuery GLEW_GET_FUN(__glewEndQuery)
#define glGenBuffers GLEW_GET_FUN(__glewGenBuffers)
#define glGenQueries GLEW_GET_FUN(__glewGenQueries)
#define glGetBufferParameteriv GLEW_GET_FUN(__glewGetBufferParameteriv)
#define glGetBufferPointerv GLEW_GET_FUN(__glewGetBufferPointerv)
#define glGetBufferSubData GLEW_GET_FUN(__glewGetBufferSubData)
#define glGetQueryObjectiv GLEW_GET_FUN(__glewGetQueryObjectiv)
#define glGetQueryObjectuiv GLEW_GET_FUN(__glewGetQueryObjectuiv)
#define glGetQueryiv GLEW_GET_FUN(__glewGetQueryiv)
#define glIsBuffer GLEW_GET_FUN(__glewIsBuffer)
#define glIsQuery GLEW_GET_FUN(__glewIsQuery)
#define glMapBuffer GLEW_GET_FUN(__glewMapBuffer)
#define glUnmapBuffer GLEW_GET_FUN(__glewUnmapBuffer)

#define GLEW_VERSION_1_5 GLEW_GET_VAR(__GLEW_VERSION_1_5)

#endif /* GL_VERSION_1_5 */

/* ----------------------------- GL_VERSION_2_0 ---------------------------- */

#ifndef GL_VERSION_2_0
#define GL_VERSION_2_0 1

#define GL_BLEND_EQUATION_RGB GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5

typedef void (GLAPIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (GLAPIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum, GLenum);
typedef void (GLAPIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (GLAPIENTRY * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (GLAPIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
typedef void (GLAPIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (GLAPIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (GLAPIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void (GLAPIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum* bufs);
typedef void (GLAPIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void (GLAPIENTRY * PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void (GLAPIENTRY * PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void (GLAPIENTRY * PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders);
typedef GLint (GLAPIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* param);
typedef void (GLAPIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (GLAPIENTRY * PFNGLGETSHADERSOURCEPROC) (GLuint obj, GLsizei maxLength, GLsizei* length, GLchar* source);
typedef void (GLAPIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* param);
typedef GLint (GLAPIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint* params);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint, GLenum, GLvoid**);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBDVPROC) (GLuint, GLenum, GLdouble*);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBFVPROC) (GLuint, GLenum, GLfloat*);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBIVPROC) (GLuint, GLenum, GLint*);
typedef GLboolean (GLAPIENTRY * PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean (GLAPIENTRY * PFNGLISSHADERPROC) (GLuint shader);
typedef void (GLAPIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
typedef void (GLAPIENTRY * PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
typedef void (GLAPIENTRY * PFNGLSTENCILMASKSEPARATEPROC) (GLenum, GLuint);
typedef void (GLAPIENTRY * PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (GLAPIENTRY * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (GLAPIENTRY * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (GLAPIENTRY * PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (GLAPIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void (GLAPIENTRY * PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GLAPIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GLAPIENTRY * PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GLAPIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GLAPIENTRY * PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

#define glAttachShader GLEW_GET_FUN(__glewAttachShader)
#define glBindAttribLocation GLEW_GET_FUN(__glewBindAttribLocation)
#define glBlendEquationSeparate GLEW_GET_FUN(__glewBlendEquationSeparate)
#define glCompileShader GLEW_GET_FUN(__glewCompileShader)
#define glCreateProgram GLEW_GET_FUN(__glewCreateProgram)
#define glCreateShader GLEW_GET_FUN(__glewCreateShader)
#define glDeleteProgram GLEW_GET_FUN(__glewDeleteProgram)
#define glDeleteShader GLEW_GET_FUN(__glewDeleteShader)
#define glDetachShader GLEW_GET_FUN(__glewDetachShader)
#define glDisableVertexAttribArray GLEW_GET_FUN(__glewDisableVertexAttribArray)
#define glDrawBuffers GLEW_GET_FUN(__glewDrawBuffers)
#define glEnableVertexAttribArray GLEW_GET_FUN(__glewEnableVertexAttribArray)
#define glGetActiveAttrib GLEW_GET_FUN(__glewGetActiveAttrib)
#define glGetActiveUniform GLEW_GET_FUN(__glewGetActiveUniform)
#define glGetAttachedShaders GLEW_GET_FUN(__glewGetAttachedShaders)
#define glGetAttribLocation GLEW_GET_FUN(__glewGetAttribLocation)
#define glGetProgramInfoLog GLEW_GET_FUN(__glewGetProgramInfoLog)
#define glGetProgramiv GLEW_GET_FUN(__glewGetProgramiv)
#define glGetShaderInfoLog GLEW_GET_FUN(__glewGetShaderInfoLog)
#define glGetShaderSource GLEW_GET_FUN(__glewGetShaderSource)
#define glGetShaderiv GLEW_GET_FUN(__glewGetShaderiv)
#define glGetUniformLocation GLEW_GET_FUN(__glewGetUniformLocation)
#define glGetUniformfv GLEW_GET_FUN(__glewGetUniformfv)
#define glGetUniformiv GLEW_GET_FUN(__glewGetUniformiv)
#define glGetVertexAttribPointerv GLEW_GET_FUN(__glewGetVertexAttribPointerv)
#define glGetVertexAttribdv GLEW_GET_FUN(__glewGetVertexAttribdv)
#define glGetVertexAttribfv GLEW_GET_FUN(__glewGetVertexAttribfv)
#define glGetVertexAttribiv GLEW_GET_FUN(__glewGetVertexAttribiv)
#define glIsProgram GLEW_GET_FUN(__glewIsProgram)
#define glIsShader GLEW_GET_FUN(__glewIsShader)
#define glLinkProgram GLEW_GET_FUN(__glewLinkProgram)
#define glShaderSource GLEW_GET_FUN(__glewShaderSource)
#define glStencilFuncSeparate GLEW_GET_FUN(__glewStencilFuncSeparate)
#define glStencilMaskSeparate GLEW_GET_FUN(__glewStencilMaskSeparate)
#define glStencilOpSeparate GLEW_GET_FUN(__glewStencilOpSeparate)
#define glUniform1f GLEW_GET_FUN(__glewUniform1f)
#define glUniform1fv GLEW_GET_FUN(__glewUniform1fv)
#define glUniform1i GLEW_GET_FUN(__glewUniform1i)
#define glUniform1iv GLEW_GET_FUN(__glewUniform1iv)
#define glUniform2f GLEW_GET_FUN(__glewUniform2f)
#define glUniform2fv GLEW_GET_FUN(__glewUniform2fv)
#define glUniform2i GLEW_GET_FUN(__glewUniform2i)
#define glUniform2iv GLEW_GET_FUN(__glewUniform2iv)
#define glUniform3f GLEW_GET_FUN(__glewUniform3f)
#define glUniform3fv GLEW_GET_FUN(__glewUniform3fv)
#define glUniform3i GLEW_GET_FUN(__glewUniform3i)
#define glUniform3iv GLEW_GET_FUN(__glewUniform3iv)
#define glUniform4f GLEW_GET_FUN(__glewUniform4f)
#define glUniform4fv GLEW_GET_FUN(__glewUniform4fv)
#define glUniform4i GLEW_GET_FUN(__glewUniform4i)
#define glUniform4iv GLEW_GET_FUN(__glewUniform4iv)
#define glUniformMatrix2fv GLEW_GET_FUN(__glewUniformMatrix2fv)
#define glUniformMatrix3fv GLEW_GET_FUN(__glewUniformMatrix3fv)
#define glUniformMatrix4fv GLEW_GET_FUN(__glewUniformMatrix4fv)
#define glUseProgram GLEW_GET_FUN(__glewUseProgram)
#define glValidateProgram GLEW_GET_FUN(__glewValidateProgram)
#define glVertexAttrib1d GLEW_GET_FUN(__glewVertexAttrib1d)
#define glVertexAttrib1dv GLEW_GET_FUN(__glewVertexAttrib1dv)
#define glVertexAttrib1f GLEW_GET_FUN(__glewVertexAttrib1f)
#define glVertexAttrib1fv GLEW_GET_FUN(__glewVertexAttrib1fv)
#define glVertexAttrib1s GLEW_GET_FUN(__glewVertexAttrib1s)
#define glVertexAttrib1sv GLEW_GET_FUN(__glewVertexAttrib1sv)
#define glVertexAttrib2d GLEW_GET_FUN(__glewVertexAttrib2d)
#define glVertexAttrib2dv GLEW_GET_FUN(__glewVertexAttrib2dv)
#define glVertexAttrib2f GLEW_GET_FUN(__glewVertexAttrib2f)
#define glVertexAttrib2fv GLEW_GET_FUN(__glewVertexAttrib2fv)
#define glVertexAttrib2s GLEW_GET_FUN(__glewVertexAttrib2s)
#define glVertexAttrib2sv GLEW_GET_FUN(__glewVertexAttrib2sv)
#define glVertexAttrib3d GLEW_GET_FUN(__glewVertexAttrib3d)
#define glVertexAttrib3dv GLEW_GET_FUN(__glewVertexAttrib3dv)
#define glVertexAttrib3f GLEW_GET_FUN(__glewVertexAttrib3f)
#define glVertexAttrib3fv GLEW_GET_FUN(__glewVertexAttrib3fv)
#define glVertexAttrib3s GLEW_GET_FUN(__glewVertexAttrib3s)
#define glVertexAttrib3sv GLEW_GET_FUN(__glewVertexAttrib3sv)
#define glVertexAttrib4Nbv GLEW_GET_FUN(__glewVertexAttrib4Nbv)
#define glVertexAttrib4Niv GLEW_GET_FUN(__glewVertexAttrib4Niv)
#define glVertexAttrib4Nsv GLEW_GET_FUN(__glewVertexAttrib4Nsv)
#define glVertexAttrib4Nub GLEW_GET_FUN(__glewVertexAttrib4Nub)
#define glVertexAttrib4Nubv GLEW_GET_FUN(__glewVertexAttrib4Nubv)
#define glVertexAttrib4Nuiv GLEW_GET_FUN(__glewVertexAttrib4Nuiv)
#define glVertexAttrib4Nusv GLEW_GET_FUN(__glewVertexAttrib4Nusv)
#define glVertexAttrib4bv GLEW_GET_FUN(__glewVertexAttrib4bv)
#define glVertexAttrib4d GLEW_GET_FUN(__glewVertexAttrib4d)
#define glVertexAttrib4dv GLEW_GET_FUN(__glewVertexAttrib4dv)
#define glVertexAttrib4f GLEW_GET_FUN(__glewVertexAttrib4f)
#define glVertexAttrib4fv GLEW_GET_FUN(__glewVertexAttrib4fv)
#define glVertexAttrib4iv GLEW_GET_FUN(__glewVertexAttrib4iv)
#define glVertexAttrib4s GLEW_GET_FUN(__glewVertexAttrib4s)
#define glVertexAttrib4sv GLEW_GET_FUN(__glewVertexAttrib4sv)
#define glVertexAttrib4ubv GLEW_GET_FUN(__glewVertexAttrib4ubv)
#define glVertexAttrib4uiv GLEW_GET_FUN(__glewVertexAttrib4uiv)
#define glVertexAttrib4usv GLEW_GET_FUN(__glewVertexAttrib4usv)
#define glVertexAttribPointer GLEW_GET_FUN(__glewVertexAttribPointer)

#define GLEW_VERSION_2_0 GLEW_GET_VAR(__GLEW_VERSION_2_0)

#endif /* GL_VERSION_2_0 */

/* ----------------------------- GL_VERSION_2_1 ---------------------------- */

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1

#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_FLOAT_MAT2x3 0x8B65
#define GL_FLOAT_MAT2x4 0x8B66
#define GL_FLOAT_MAT3x2 0x8B67
#define GL_FLOAT_MAT3x4 0x8B68
#define GL_FLOAT_MAT4x2 0x8B69
#define GL_FLOAT_MAT4x3 0x8B6A
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB_ALPHA 0x8C42
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_SLUMINANCE_ALPHA 0x8C44
#define GL_SLUMINANCE8_ALPHA8 0x8C45
#define GL_SLUMINANCE 0x8C46
#define GL_SLUMINANCE8 0x8C47
#define GL_COMPRESSED_SRGB 0x8C48
#define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#define GL_COMPRESSED_SLUMINANCE 0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B

typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

#define glUniformMatrix2x3fv GLEW_GET_FUN(__glewUniformMatrix2x3fv)
#define glUniformMatrix2x4fv GLEW_GET_FUN(__glewUniformMatrix2x4fv)
#define glUniformMatrix3x2fv GLEW_GET_FUN(__glewUniformMatrix3x2fv)
#define glUniformMatrix3x4fv GLEW_GET_FUN(__glewUniformMatrix3x4fv)
#define glUniformMatrix4x2fv GLEW_GET_FUN(__glewUniformMatrix4x2fv)
#define glUniformMatrix4x3fv GLEW_GET_FUN(__glewUniformMatrix4x3fv)

#define GLEW_VERSION_2_1 GLEW_GET_VAR(__GLEW_VERSION_2_1)

#endif /* GL_VERSION_2_1 */

/* ----------------------------- GL_VERSION_3_0 ---------------------------- */

#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1

#define GL_MAX_CLIP_DISTANCES GL_MAX_CLIP_PLANES
#define GL_CLIP_DISTANCE5 GL_CLIP_PLANE5
#define GL_CLIP_DISTANCE1 GL_CLIP_PLANE1
#define GL_CLIP_DISTANCE3 GL_CLIP_PLANE3
#define GL_COMPARE_REF_TO_TEXTURE GL_COMPARE_R_TO_TEXTURE_ARB
#define GL_CLIP_DISTANCE0 GL_CLIP_PLANE0
#define GL_CLIP_DISTANCE4 GL_CLIP_PLANE4
#define GL_CLIP_DISTANCE2 GL_CLIP_PLANE2
#define GL_MAX_VARYING_COMPONENTS GL_MAX_VARYING_FLOATS
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_CONTEXT_FLAGS 0x821E
#define GL_DEPTH_BUFFER 0x8223
#define GL_STENCIL_BUFFER 0x8224
#define GL_COMPRESSED_RED 0x8225
#define GL_COMPRESSED_RG 0x8226
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#define GL_CLAMP_VERTEX_COLOR 0x891A
#define GL_CLAMP_FRAGMENT_COLOR 0x891B
#define GL_CLAMP_READ_COLOR 0x891C
#define GL_FIXED_ONLY 0x891D
#define GL_TEXTURE_RED_TYPE 0x8C10
#define GL_TEXTURE_GREEN_TYPE 0x8C11
#define GL_TEXTURE_BLUE_TYPE 0x8C12
#define GL_TEXTURE_ALPHA_TYPE 0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
#define GL_TEXTURE_INTENSITY_TYPE 0x8C15
#define GL_TEXTURE_DEPTH_TYPE 0x8C16
#define GL_TEXTURE_1D_ARRAY 0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#define GL_RGB9_E5 0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#define GL_TEXTURE_SHARED_SIZE 0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_PRIMITIVES_GENERATED 0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD 0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#define GL_SEPARATE_ATTRIBS 0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI 0x8D70
#define GL_RGB32UI 0x8D71
#define GL_RGBA16UI 0x8D76
#define GL_RGB16UI 0x8D77
#define GL_RGBA8UI 0x8D7C
#define GL_RGB8UI 0x8D7D
#define GL_RGBA32I 0x8D82
#define GL_RGB32I 0x8D83
#define GL_RGBA16I 0x8D88
#define GL_RGB16I 0x8D89
#define GL_RGBA8I 0x8D8E
#define GL_RGB8I 0x8D8F
#define GL_RED_INTEGER 0x8D94
#define GL_GREEN_INTEGER 0x8D95
#define GL_BLUE_INTEGER 0x8D96
#define GL_ALPHA_INTEGER 0x8D97
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_BGR_INTEGER 0x8D9A
#define GL_BGRA_INTEGER 0x8D9B
#define GL_SAMPLER_1D_ARRAY 0x8DC0
#define GL_SAMPLER_2D_ARRAY 0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#define GL_UNSIGNED_INT_VEC4 0x8DC8
#define GL_INT_SAMPLER_1D 0x8DC9
#define GL_INT_SAMPLER_2D 0x8DCA
#define GL_INT_SAMPLER_3D 0x8DCB
#define GL_INT_SAMPLER_CUBE 0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#define GL_QUERY_WAIT 0x8E13
#define GL_QUERY_NO_WAIT 0x8E14
#define GL_QUERY_BY_REGION_WAIT 0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT 0x8E16

typedef void (GLAPIENTRY * PFNGLBEGINCONDITIONALRENDERPROC) (GLuint, GLenum);
typedef void (GLAPIENTRY * PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum);
typedef void (GLAPIENTRY * PFNGLBINDFRAGDATALOCATIONPROC) (GLuint, GLuint, const GLchar*);
typedef void (GLAPIENTRY * PFNGLCLAMPCOLORPROC) (GLenum, GLenum);
typedef void (GLAPIENTRY * PFNGLCLEARBUFFERFIPROC) (GLenum, GLint, GLfloat, GLint);
typedef void (GLAPIENTRY * PFNGLCLEARBUFFERFVPROC) (GLenum, GLint, const GLfloat*);
typedef void (GLAPIENTRY * PFNGLCLEARBUFFERIVPROC) (GLenum, GLint, const GLint*);
typedef void (GLAPIENTRY * PFNGLCLEARBUFFERUIVPROC) (GLenum, GLint, const GLuint*);
typedef void (GLAPIENTRY * PFNGLCOLORMASKIPROC) (GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (GLAPIENTRY * PFNGLDISABLEIPROC) (GLenum, GLuint);
typedef void (GLAPIENTRY * PFNGLENABLEIPROC) (GLenum, GLuint);
typedef void (GLAPIENTRY * PFNGLENDCONDITIONALRENDERPROC) (void);
typedef void (GLAPIENTRY * PFNGLENDTRANSFORMFEEDBACKPROC) (void);
typedef void (GLAPIENTRY * PFNGLGETBOOLEANI_VPROC) (GLenum, GLuint, GLboolean*);
typedef GLint (GLAPIENTRY * PFNGLGETFRAGDATALOCATIONPROC) (GLuint, const GLchar*);
typedef const GLubyte* (GLAPIENTRY * PFNGLGETSTRINGIPROC) (GLenum, GLuint);
typedef void (GLAPIENTRY * PFNGLGETTEXPARAMETERIIVPROC) (GLenum, GLenum, GLint*);
typedef void (GLAPIENTRY * PFNGLGETTEXPARAMETERIUIVPROC) (GLenum, GLenum, GLuint*);
typedef void (GLAPIENTRY * PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMUIVPROC) (GLuint, GLint, GLuint*);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBIIVPROC) (GLuint, GLenum, GLint*);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBIUIVPROC) (GLuint, GLenum, GLuint*);
typedef GLboolean (GLAPIENTRY * PFNGLISENABLEDIPROC) (GLenum, GLuint);
typedef void (GLAPIENTRY * PFNGLTEXPARAMETERIIVPROC) (GLenum, GLenum, const GLint*);
typedef void (GLAPIENTRY * PFNGLTEXPARAMETERIUIVPROC) (GLenum, GLenum, const GLuint*);
typedef void (GLAPIENTRY * PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint, GLsizei, const GLchar **, GLenum);
typedef void (GLAPIENTRY * PFNGLUNIFORM1UIPROC) (GLint, GLuint);
typedef void (GLAPIENTRY * PFNGLUNIFORM1UIVPROC) (GLint, GLsizei, const GLuint*);
typedef void (GLAPIENTRY * PFNGLUNIFORM2UIPROC) (GLint, GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLUNIFORM2UIVPROC) (GLint, GLsizei, const GLuint*);
typedef void (GLAPIENTRY * PFNGLUNIFORM3UIPROC) (GLint, GLuint, GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLUNIFORM3UIVPROC) (GLint, GLsizei, const GLuint*);
typedef void (GLAPIENTRY * PFNGLUNIFORM4UIPROC) (GLint, GLuint, GLuint, GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLUNIFORM4UIVPROC) (GLint, GLsizei, const GLuint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI1IPROC) (GLuint, GLint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI1IVPROC) (GLuint, const GLint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI1UIPROC) (GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI1UIVPROC) (GLuint, const GLuint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI2IPROC) (GLuint, GLint, GLint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI2IVPROC) (GLuint, const GLint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI2UIPROC) (GLuint, GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI2UIVPROC) (GLuint, const GLuint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI3IPROC) (GLuint, GLint, GLint, GLint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI3IVPROC) (GLuint, const GLint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI3UIPROC) (GLuint, GLuint, GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI3UIVPROC) (GLuint, const GLuint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4BVPROC) (GLuint, const GLbyte*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4IPROC) (GLuint, GLint, GLint, GLint, GLint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4IVPROC) (GLuint, const GLint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4SVPROC) (GLuint, const GLshort*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4UBVPROC) (GLuint, const GLubyte*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4UIPROC) (GLuint, GLuint, GLuint, GLuint, GLuint);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4UIVPROC) (GLuint, const GLuint*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBI4USVPROC) (GLuint, const GLushort*);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint, GLint, GLenum, GLsizei, const GLvoid*);

#define glBeginConditionalRender GLEW_GET_FUN(__glewBeginConditionalRender)
#define glBeginTransformFeedback GLEW_GET_FUN(__glewBeginTransformFeedback)
#define glBindFragDataLocation GLEW_GET_FUN(__glewBindFragDataLocation)
#define glClampColor GLEW_GET_FUN(__glewClampColor)
#define glClearBufferfi GLEW_GET_FUN(__glewClearBufferfi)
#define glClearBufferfv GLEW_GET_FUN(__glewClearBufferfv)
#define glClearBufferiv GLEW_GET_FUN(__glewClearBufferiv)
#define glClearBufferuiv GLEW_GET_FUN(__glewClearBufferuiv)
#define glColorMaski GLEW_GET_FUN(__glewColorMaski)
#define glDisablei GLEW_GET_FUN(__glewDisablei)
#define glEnablei GLEW_GET_FUN(__glewEnablei)
#define glEndConditionalRender GLEW_GET_FUN(__glewEndConditionalRender)
#define glEndTransformFeedback GLEW_GET_FUN(__glewEndTransformFeedback)
#define glGetBooleani_v GLEW_GET_FUN(__glewGetBooleani_v)
#define glGetFragDataLocation GLEW_GET_FUN(__glewGetFragDataLocation)
#define glGetStringi GLEW_GET_FUN(__glewGetStringi)
#define glGetTexParameterIiv GLEW_GET_FUN(__glewGetTexParameterIiv)
#define glGetTexParameterIuiv GLEW_GET_FUN(__glewGetTexParameterIuiv)
#define glGetTransformFeedbackVarying GLEW_GET_FUN(__glewGetTransformFeedbackVarying)
#define glGetUniformuiv GLEW_GET_FUN(__glewGetUniformuiv)
#define glGetVertexAttribIiv GLEW_GET_FUN(__glewGetVertexAttribIiv)
#define glGetVertexAttribIuiv GLEW_GET_FUN(__glewGetVertexAttribIuiv)
#define glIsEnabledi GLEW_GET_FUN(__glewIsEnabledi)
#define glTexParameterIiv GLEW_GET_FUN(__glewTexParameterIiv)
#define glTexParameterIuiv GLEW_GET_FUN(__glewTexParameterIuiv)
#define glTransformFeedbackVaryings GLEW_GET_FUN(__glewTransformFeedbackVaryings)
#define glUniform1ui GLEW_GET_FUN(__glewUniform1ui)
#define glUniform1uiv GLEW_GET_FUN(__glewUniform1uiv)
#define glUniform2ui GLEW_GET_FUN(__glewUniform2ui)
#define glUniform2uiv GLEW_GET_FUN(__glewUniform2uiv)
#define glUniform3ui GLEW_GET_FUN(__glewUniform3ui)
#define glUniform3uiv GLEW_GET_FUN(__glewUniform3uiv)
#define glUniform4ui GLEW_GET_FUN(__glewUniform4ui)
#define glUniform4uiv GLEW_GET_FUN(__glewUniform4uiv)
#define glVertexAttribI1i GLEW_GET_FUN(__glewVertexAttribI1i)
#define glVertexAttribI1iv GLEW_GET_FUN(__glewVertexAttribI1iv)
#define glVertexAttribI1ui GLEW_GET_FUN(__glewVertexAttribI1ui)
#define glVertexAttribI1uiv GLEW_GET_FUN(__glewVertexAttribI1uiv)
#define glVertexAttribI2i GLEW_GET_FUN(__glewVertexAttribI2i)
#define glVertexAttribI2iv GLEW_GET_FUN(__glewVertexAttribI2iv)
#define glVertexAttribI2ui GLEW_GET_FUN(__glewVertexAttribI2ui)
#define glVertexAttribI2uiv GLEW_GET_FUN(__glewVertexAttribI2uiv)
#define glVertexAttribI3i GLEW_GET_FUN(__glewVertexAttribI3i)
#define glVertexAttribI3iv GLEW_GET_FUN(__glewVertexAttribI3iv)
#define glVertexAttribI3ui GLEW_GET_FUN(__glewVertexAttribI3ui)
#define glVertexAttribI3uiv GLEW_GET_FUN(__glewVertexAttribI3uiv)
#define glVertexAttribI4bv GLEW_GET_FUN(__glewVertexAttribI4bv)
#define glVertexAttribI4i GLEW_GET_FUN(__glewVertexAttribI4i)
#define glVertexAttribI4iv GLEW_GET_FUN(__glewVertexAttribI4iv)
#define glVertexAttribI4sv GLEW_GET_FUN(__glewVertexAttribI4sv)
#define glVertexAttribI4ubv GLEW_GET_FUN(__glewVertexAttribI4ubv)
#define glVertexAttribI4ui GLEW_GET_FUN(__glewVertexAttribI4ui)
#define glVertexAttribI4uiv GLEW_GET_FUN(__glewVertexAttribI4uiv)
#define glVertexAttribI4usv GLEW_GET_FUN(__glewVertexAttribI4usv)
#define glVertexAttribIPointer GLEW_GET_FUN(__glewVertexAttribIPointer)

#define GLEW_VERSION_3_0 GLEW_GET_VAR(__GLEW_VERSION_3_0)

#endif /* GL_VERSION_3_0 */

/* ----------------------------- GL_VERSION_3_1 ---------------------------- */

#ifndef GL_VERSION_3_1
#define GL_VERSION_3_1 1

#define GL_TEXTURE_RECTANGLE 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE 0x84F8
#define GL_SAMPLER_2D_RECT 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW 0x8B64
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE 0x8C2B
#define GL_TEXTURE_BINDING_BUFFER 0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_BUFFER_FORMAT 0x8C2E
#define GL_SAMPLER_BUFFER 0x8DC2
#define GL_INT_SAMPLER_2D_RECT 0x8DCD
#define GL_INT_SAMPLER_BUFFER 0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT 0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER 0x8DD8
#define GL_RED_SNORM 0x8F90
#define GL_RG_SNORM 0x8F91
#define GL_RGB_SNORM 0x8F92
#define GL_RGBA_SNORM 0x8F93
#define GL_R8_SNORM 0x8F94
#define GL_RG8_SNORM 0x8F95
#define GL_RGB8_SNORM 0x8F96
#define GL_RGBA8_SNORM 0x8F97
#define GL_R16_SNORM 0x8F98
#define GL_RG16_SNORM 0x8F99
#define GL_RGB16_SNORM 0x8F9A
#define GL_RGBA16_SNORM 0x8F9B
#define GL_SIGNED_NORMALIZED 0x8F9C
#define GL_PRIMITIVE_RESTART 0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX 0x8F9E
#define GL_BUFFER_ACCESS_FLAGS 0x911F
#define GL_BUFFER_MAP_LENGTH 0x9120
#define GL_BUFFER_MAP_OFFSET 0x9121

typedef void (GLAPIENTRY * PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum, GLint, GLsizei, GLsizei);
typedef void (GLAPIENTRY * PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum, GLsizei, GLenum, const GLvoid*, GLsizei);
typedef void (GLAPIENTRY * PFNGLPRIMITIVERESTARTINDEXPROC) (GLuint);
typedef void (GLAPIENTRY * PFNGLTEXBUFFERPROC) (GLenum, GLenum, GLuint);

#define glDrawArraysInstanced GLEW_GET_FUN(__glewDrawArraysInstanced)
#define glDrawElementsInstanced GLEW_GET_FUN(__glewDrawElementsInstanced)
#define glPrimitiveRestartIndex GLEW_GET_FUN(__glewPrimitiveRestartIndex)
#define glTexBuffer GLEW_GET_FUN(__glewTexBuffer)

#define GLEW_VERSION_3_1 GLEW_GET_VAR(__GLEW_VERSION_3_1)

#endif /* GL_VERSION_3_1 */

/* ----------------------------- GL_VERSION_3_2 ---------------------------- */

#ifndef GL_VERSION_3_2
#define GL_VERSION_3_2 1

#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_LINES_ADJACENCY 0x000A
#define GL_LINE_STRIP_ADJACENCY 0x000B
#define GL_TRIANGLES_ADJACENCY 0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY 0x000D
#define GL_PROGRAM_POINT_SIZE 0x8642
#define GL_GEOMETRY_VERTICES_OUT 0x8916
#define GL_GEOMETRY_INPUT_TYPE 0x8917
#define GL_GEOMETRY_OUTPUT_TYPE 0x8918
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES 0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS 0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
#define GL_CONTEXT_PROFILE_MASK 0x9126

typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum, GLenum, GLuint, GLint);
typedef void (GLAPIENTRY * PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum, GLenum, GLint64 *);
typedef void (GLAPIENTRY * PFNGLGETINTEGER64I_VPROC) (GLenum, GLuint, GLint64 *);

#define glFramebufferTexture GLEW_GET_FUN(__glewFramebufferTexture)
#define glGetBufferParameteri64v GLEW_GET_FUN(__glewGetBufferParameteri64v)
#define glGetInteger64i_v GLEW_GET_FUN(__glewGetInteger64i_v)

#define GLEW_VERSION_3_2 GLEW_GET_VAR(__GLEW_VERSION_3_2)

#endif /* GL_VERSION_3_2 */

/* ----------------------------- GL_VERSION_3_3 ---------------------------- */

#ifndef GL_VERSION_3_3
#define GL_VERSION_3_3 1

#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR 0x88FE
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#define GL_RGB10_A2UI 0x906F

typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);

#define glVertexAttribDivisor GLEW_GET_FUN(__glewVertexAttribDivisor)

#define GLEW_VERSION_3_3 GLEW_GET_VAR(__GLEW_VERSION_3_3)

#endif /* GL_VERSION_3_3 */

/* ----------------------------- GL_VERSION_4_0 ---------------------------- */

#ifndef GL_VERSION_4_0
#define GL_VERSION_4_0 1

#define GL_GEOMETRY_SHADER_INVOCATIONS 0x887F
#define GL_SAMPLE_SHADING 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE 0x8C37
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS 0x8F9F
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY 0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY 0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW 0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY 0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F

typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONSEPARATEIPROC) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONIPROC) (GLuint buf, GLenum mode);
typedef void (GLAPIENTRY * PFNGLBLENDFUNCSEPARATEIPROC) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef void (GLAPIENTRY * PFNGLBLENDFUNCIPROC) (GLuint buf, GLenum src, GLenum dst);
typedef void (GLAPIENTRY * PFNGLMINSAMPLESHADINGPROC) (GLclampf value);

#define glBlendEquationSeparatei GLEW_GET_FUN(__glewBlendEquationSeparatei)
#define glBlendEquationi GLEW_GET_FUN(__glewBlendEquationi)
#define glBlendFuncSeparatei GLEW_GET_FUN(__glewBlendFuncSeparatei)
#define glBlendFunci GLEW_GET_FUN(__glewBlendFunci)
#define glMinSampleShading GLEW_GET_FUN(__glewMinSampleShading)

#define GLEW_VERSION_4_0 GLEW_GET_VAR(__GLEW_VERSION_4_0)

#endif /* GL_VERSION_4_0 */

/* ----------------------------- GL_VERSION_4_1 ---------------------------- */

#ifndef GL_VERSION_4_1
#define GL_VERSION_4_1 1

#define GLEW_VERSION_4_1 GLEW_GET_VAR(__GLEW_VERSION_4_1)

#endif /* GL_VERSION_4_1 */

/* ----------------------------- GL_VERSION_4_2 ---------------------------- */

#ifndef GL_VERSION_4_2
#define GL_VERSION_4_2 1

#define GL_COMPRESSED_RGBA_BPTC_UNORM 0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F

#define GLEW_VERSION_4_2 GLEW_GET_VAR(__GLEW_VERSION_4_2)

#endif /* GL_VERSION_4_2 */

/* ----------------------------- GL_VERSION_4_3 ---------------------------- */

#ifndef GL_VERSION_4_3
#define GL_VERSION_4_3 1

#define GL_NUM_SHADING_LANGUAGE_VERSIONS 0x82E9
#define GL_VERTEX_ATTRIB_ARRAY_LONG 0x874E

#define GLEW_VERSION_4_3 GLEW_GET_VAR(__GLEW_VERSION_4_3)

#endif /* GL_VERSION_4_3 */

/* ------------------------ GL_ARB_fragment_program ------------------------ */

#ifndef GL_ARB_fragment_program
#define GL_ARB_fragment_program 1

#define GL_FRAGMENT_PROGRAM_ARB 0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB 0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB 0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB 0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB 0x8872

#define GLEW_ARB_fragment_program GLEW_GET_VAR(__GLEW_ARB_fragment_program)

#endif /* GL_ARB_fragment_program */

/* ------------------------- GL_ARB_fragment_shader ------------------------ */

#ifndef GL_ARB_fragment_shader
#define GL_ARB_fragment_shader 1

#define GL_FRAGMENT_SHADER_ARB 0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B

#define GLEW_ARB_fragment_shader GLEW_GET_VAR(__GLEW_ARB_fragment_shader)

#endif /* GL_ARB_fragment_shader */

/* -------------------------- GL_ARB_multitexture -------------------------- */

#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture 1

#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1
#define GL_TEXTURE2_ARB 0x84C2
#define GL_TEXTURE3_ARB 0x84C3
#define GL_TEXTURE4_ARB 0x84C4
#define GL_TEXTURE5_ARB 0x84C5
#define GL_TEXTURE6_ARB 0x84C6
#define GL_TEXTURE7_ARB 0x84C7
#define GL_TEXTURE8_ARB 0x84C8
#define GL_TEXTURE9_ARB 0x84C9
#define GL_TEXTURE10_ARB 0x84CA
#define GL_TEXTURE11_ARB 0x84CB
#define GL_TEXTURE12_ARB 0x84CC
#define GL_TEXTURE13_ARB 0x84CD
#define GL_TEXTURE14_ARB 0x84CE
#define GL_TEXTURE15_ARB 0x84CF
#define GL_TEXTURE16_ARB 0x84D0
#define GL_TEXTURE17_ARB 0x84D1
#define GL_TEXTURE18_ARB 0x84D2
#define GL_TEXTURE19_ARB 0x84D3
#define GL_TEXTURE20_ARB 0x84D4
#define GL_TEXTURE21_ARB 0x84D5
#define GL_TEXTURE22_ARB 0x84D6
#define GL_TEXTURE23_ARB 0x84D7
#define GL_TEXTURE24_ARB 0x84D8
#define GL_TEXTURE25_ARB 0x84D9
#define GL_TEXTURE26_ARB 0x84DA
#define GL_TEXTURE27_ARB 0x84DB
#define GL_TEXTURE28_ARB 0x84DC
#define GL_TEXTURE29_ARB 0x84DD
#define GL_TEXTURE30_ARB 0x84DE
#define GL_TEXTURE31_ARB 0x84DF
#define GL_ACTIVE_TEXTURE_ARB 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB 0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB 0x84E2

typedef void (GLAPIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);

#define glActiveTextureARB GLEW_GET_FUN(__glewActiveTextureARB)
#define glClientActiveTextureARB GLEW_GET_FUN(__glewClientActiveTextureARB)
#define glMultiTexCoord1dARB GLEW_GET_FUN(__glewMultiTexCoord1dARB)
#define glMultiTexCoord1dvARB GLEW_GET_FUN(__glewMultiTexCoord1dvARB)
#define glMultiTexCoord1fARB GLEW_GET_FUN(__glewMultiTexCoord1fARB)
#define glMultiTexCoord1fvARB GLEW_GET_FUN(__glewMultiTexCoord1fvARB)
#define glMultiTexCoord1iARB GLEW_GET_FUN(__glewMultiTexCoord1iARB)
#define glMultiTexCoord1ivARB GLEW_GET_FUN(__glewMultiTexCoord1ivARB)
#define glMultiTexCoord1sARB GLEW_GET_FUN(__glewMultiTexCoord1sARB)
#define glMultiTexCoord1svARB GLEW_GET_FUN(__glewMultiTexCoord1svARB)
#define glMultiTexCoord2dARB GLEW_GET_FUN(__glewMultiTexCoord2dARB)
#define glMultiTexCoord2dvARB GLEW_GET_FUN(__glewMultiTexCoord2dvARB)
#define glMultiTexCoord2fARB GLEW_GET_FUN(__glewMultiTexCoord2fARB)
#define glMultiTexCoord2fvARB GLEW_GET_FUN(__glewMultiTexCoord2fvARB)
#define glMultiTexCoord2iARB GLEW_GET_FUN(__glewMultiTexCoord2iARB)
#define glMultiTexCoord2ivARB GLEW_GET_FUN(__glewMultiTexCoord2ivARB)
#define glMultiTexCoord2sARB GLEW_GET_FUN(__glewMultiTexCoord2sARB)
#define glMultiTexCoord2svARB GLEW_GET_FUN(__glewMultiTexCoord2svARB)
#define glMultiTexCoord3dARB GLEW_GET_FUN(__glewMultiTexCoord3dARB)
#define glMultiTexCoord3dvARB GLEW_GET_FUN(__glewMultiTexCoord3dvARB)
#define glMultiTexCoord3fARB GLEW_GET_FUN(__glewMultiTexCoord3fARB)
#define glMultiTexCoord3fvARB GLEW_GET_FUN(__glewMultiTexCoord3fvARB)
#define glMultiTexCoord3iARB GLEW_GET_FUN(__glewMultiTexCoord3iARB)
#define glMultiTexCoord3ivARB GLEW_GET_FUN(__glewMultiTexCoord3ivARB)
#define glMultiTexCoord3sARB GLEW_GET_FUN(__glewMultiTexCoord3sARB)
#define glMultiTexCoord3svARB GLEW_GET_FUN(__glewMultiTexCoord3svARB)
#define glMultiTexCoord4dARB GLEW_GET_FUN(__glewMultiTexCoord4dARB)
#define glMultiTexCoord4dvARB GLEW_GET_FUN(__glewMultiTexCoord4dvARB)
#define glMultiTexCoord4fARB GLEW_GET_FUN(__glewMultiTexCoord4fARB)
#define glMultiTexCoord4fvARB GLEW_GET_FUN(__glewMultiTexCoord4fvARB)
#define glMultiTexCoord4iARB GLEW_GET_FUN(__glewMultiTexCoord4iARB)
#define glMultiTexCoord4ivARB GLEW_GET_FUN(__glewMultiTexCoord4ivARB)
#define glMultiTexCoord4sARB GLEW_GET_FUN(__glewMultiTexCoord4sARB)
#define glMultiTexCoord4svARB GLEW_GET_FUN(__glewMultiTexCoord4svARB)

#define GLEW_ARB_multitexture GLEW_GET_VAR(__GLEW_ARB_multitexture)

#endif /* GL_ARB_multitexture */

/* ------------------------- GL_ARB_shader_objects ------------------------- */

#ifndef GL_ARB_shader_objects
#define GL_ARB_shader_objects 1

#define GL_PROGRAM_OBJECT_ARB 0x8B40
#define GL_SHADER_OBJECT_ARB 0x8B48
#define GL_OBJECT_TYPE_ARB 0x8B4E
#define GL_OBJECT_SUBTYPE_ARB 0x8B4F
#define GL_FLOAT_VEC2_ARB 0x8B50
#define GL_FLOAT_VEC3_ARB 0x8B51
#define GL_FLOAT_VEC4_ARB 0x8B52
#define GL_INT_VEC2_ARB 0x8B53
#define GL_INT_VEC3_ARB 0x8B54
#define GL_INT_VEC4_ARB 0x8B55
#define GL_BOOL_ARB 0x8B56
#define GL_BOOL_VEC2_ARB 0x8B57
#define GL_BOOL_VEC3_ARB 0x8B58
#define GL_BOOL_VEC4_ARB 0x8B59
#define GL_FLOAT_MAT2_ARB 0x8B5A
#define GL_FLOAT_MAT3_ARB 0x8B5B
#define GL_FLOAT_MAT4_ARB 0x8B5C
#define GL_SAMPLER_1D_ARB 0x8B5D
#define GL_SAMPLER_2D_ARB 0x8B5E
#define GL_SAMPLER_3D_ARB 0x8B5F
#define GL_SAMPLER_CUBE_ARB 0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB 0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB 0x8B62
#define GL_SAMPLER_2D_RECT_ARB 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB 0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB 0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB 0x8B81
#define GL_OBJECT_LINK_STATUS_ARB 0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB 0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB 0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB 0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB 0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88

typedef char GLcharARB;
typedef unsigned int GLhandleARB;

typedef void (GLAPIENTRY * PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (GLAPIENTRY * PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB (GLAPIENTRY * PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef GLhandleARB (GLAPIENTRY * PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (GLAPIENTRY * PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef void (GLAPIENTRY * PFNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef void (GLAPIENTRY * PFNGLGETACTIVEUNIFORMARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint *size, GLenum *type, GLcharARB *name);
typedef void (GLAPIENTRY * PFNGLGETATTACHEDOBJECTSARBPROC) (GLhandleARB containerObj, GLsizei maxCount, GLsizei* count, GLhandleARB *obj);
typedef GLhandleARB (GLAPIENTRY * PFNGLGETHANDLEARBPROC) (GLenum pname);
typedef void (GLAPIENTRY * PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *infoLog);
typedef void (GLAPIENTRY * PFNGLGETOBJECTPARAMETERFVARBPROC) (GLhandleARB obj, GLenum pname, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint* params);
typedef void (GLAPIENTRY * PFNGLGETSHADERSOURCEARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *source);
typedef GLint (GLAPIENTRY * PFNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB* name);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMFVARBPROC) (GLhandleARB programObj, GLint location, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMIVARBPROC) (GLhandleARB programObj, GLint location, GLint* params);
typedef void (GLAPIENTRY * PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (GLAPIENTRY * PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB ** string, const GLint *length);
typedef void (GLAPIENTRY * PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
typedef void (GLAPIENTRY * PFNGLUNIFORM1FVARBPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
typedef void (GLAPIENTRY * PFNGLUNIFORM1IVARBPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM2FARBPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (GLAPIENTRY * PFNGLUNIFORM2FVARBPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM2IARBPROC) (GLint location, GLint v0, GLint v1);
typedef void (GLAPIENTRY * PFNGLUNIFORM2IVARBPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM3FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GLAPIENTRY * PFNGLUNIFORM3FVARBPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM3IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GLAPIENTRY * PFNGLUNIFORM3IVARBPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM4FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GLAPIENTRY * PFNGLUNIFORM4FVARBPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORM4IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GLAPIENTRY * PFNGLUNIFORM4IVARBPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GLAPIENTRY * PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef void (GLAPIENTRY * PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);

#define glAttachObjectARB GLEW_GET_FUN(__glewAttachObjectARB)
#define glCompileShaderARB GLEW_GET_FUN(__glewCompileShaderARB)
#define glCreateProgramObjectARB GLEW_GET_FUN(__glewCreateProgramObjectARB)
#define glCreateShaderObjectARB GLEW_GET_FUN(__glewCreateShaderObjectARB)
#define glDeleteObjectARB GLEW_GET_FUN(__glewDeleteObjectARB)
#define glDetachObjectARB GLEW_GET_FUN(__glewDetachObjectARB)
#define glGetActiveUniformARB GLEW_GET_FUN(__glewGetActiveUniformARB)
#define glGetAttachedObjectsARB GLEW_GET_FUN(__glewGetAttachedObjectsARB)
#define glGetHandleARB GLEW_GET_FUN(__glewGetHandleARB)
#define glGetInfoLogARB GLEW_GET_FUN(__glewGetInfoLogARB)
#define glGetObjectParameterfvARB GLEW_GET_FUN(__glewGetObjectParameterfvARB)
#define glGetObjectParameterivARB GLEW_GET_FUN(__glewGetObjectParameterivARB)
#define glGetShaderSourceARB GLEW_GET_FUN(__glewGetShaderSourceARB)
#define glGetUniformLocationARB GLEW_GET_FUN(__glewGetUniformLocationARB)
#define glGetUniformfvARB GLEW_GET_FUN(__glewGetUniformfvARB)
#define glGetUniformivARB GLEW_GET_FUN(__glewGetUniformivARB)
#define glLinkProgramARB GLEW_GET_FUN(__glewLinkProgramARB)
#define glShaderSourceARB GLEW_GET_FUN(__glewShaderSourceARB)
#define glUniform1fARB GLEW_GET_FUN(__glewUniform1fARB)
#define glUniform1fvARB GLEW_GET_FUN(__glewUniform1fvARB)
#define glUniform1iARB GLEW_GET_FUN(__glewUniform1iARB)
#define glUniform1ivARB GLEW_GET_FUN(__glewUniform1ivARB)
#define glUniform2fARB GLEW_GET_FUN(__glewUniform2fARB)
#define glUniform2fvARB GLEW_GET_FUN(__glewUniform2fvARB)
#define glUniform2iARB GLEW_GET_FUN(__glewUniform2iARB)
#define glUniform2ivARB GLEW_GET_FUN(__glewUniform2ivARB)
#define glUniform3fARB GLEW_GET_FUN(__glewUniform3fARB)
#define glUniform3fvARB GLEW_GET_FUN(__glewUniform3fvARB)
#define glUniform3iARB GLEW_GET_FUN(__glewUniform3iARB)
#define glUniform3ivARB GLEW_GET_FUN(__glewUniform3ivARB)
#define glUniform4fARB GLEW_GET_FUN(__glewUniform4fARB)
#define glUniform4fvARB GLEW_GET_FUN(__glewUniform4fvARB)
#define glUniform4iARB GLEW_GET_FUN(__glewUniform4iARB)
#define glUniform4ivARB GLEW_GET_FUN(__glewUniform4ivARB)
#define glUniformMatrix2fvARB GLEW_GET_FUN(__glewUniformMatrix2fvARB)
#define glUniformMatrix3fvARB GLEW_GET_FUN(__glewUniformMatrix3fvARB)
#define glUniformMatrix4fvARB GLEW_GET_FUN(__glewUniformMatrix4fvARB)
#define glUseProgramObjectARB GLEW_GET_FUN(__glewUseProgramObjectARB)
#define glValidateProgramARB GLEW_GET_FUN(__glewValidateProgramARB)

#define GLEW_ARB_shader_objects GLEW_GET_VAR(__GLEW_ARB_shader_objects)

#endif /* GL_ARB_shader_objects */

/* ---------------------- GL_ARB_shading_language_100 ---------------------- */

#ifndef GL_ARB_shading_language_100
#define GL_ARB_shading_language_100 1

#define GL_SHADING_LANGUAGE_VERSION_ARB 0x8B8C

#define GLEW_ARB_shading_language_100 GLEW_GET_VAR(__GLEW_ARB_shading_language_100)

#endif /* GL_ARB_shading_language_100 */

/* ------------------------ GL_ARB_texture_rectangle ----------------------- */

#ifndef GL_ARB_texture_rectangle
#define GL_ARB_texture_rectangle 1

#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#define GL_SAMPLER_2D_RECT_ARB 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB 0x8B64

#define GLEW_ARB_texture_rectangle GLEW_GET_VAR(__GLEW_ARB_texture_rectangle)

#endif /* GL_ARB_texture_rectangle */

/* ------------------------- GL_ARB_vertex_program ------------------------- */

#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1

#define GL_COLOR_SUM_ARB 0x8458
#define GL_VERTEX_PROGRAM_ARB 0x8620
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB 0x8625
#define GL_CURRENT_VERTEX_ATTRIB_ARB 0x8626
#define GL_PROGRAM_LENGTH_ARB 0x8627
#define GL_PROGRAM_STRING_ARB 0x8628
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_MAX_PROGRAM_MATRICES_ARB 0x862F
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_CURRENT_MATRIX_ARB 0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB 0x8645
#define GL_PROGRAM_ERROR_POSITION_ARB 0x864B
#define GL_PROGRAM_BINDING_ARB 0x8677
#define GL_MAX_VERTEX_ATTRIBS_ARB 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB 0x886A
#define GL_PROGRAM_ERROR_STRING_ARB 0x8874
#define GL_PROGRAM_FORMAT_ASCII_ARB 0x8875
#define GL_PROGRAM_FORMAT_ARB 0x8876
#define GL_PROGRAM_INSTRUCTIONS_ARB 0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB 0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB 0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB 0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB 0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB 0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB 0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB 0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB 0x88B7
#define GL_MATRIX0_ARB 0x88C0
#define GL_MATRIX1_ARB 0x88C1
#define GL_MATRIX2_ARB 0x88C2
#define GL_MATRIX3_ARB 0x88C3
#define GL_MATRIX4_ARB 0x88C4
#define GL_MATRIX5_ARB 0x88C5
#define GL_MATRIX6_ARB 0x88C6
#define GL_MATRIX7_ARB 0x88C7
#define GL_MATRIX8_ARB 0x88C8
#define GL_MATRIX9_ARB 0x88C9
#define GL_MATRIX10_ARB 0x88CA
#define GL_MATRIX11_ARB 0x88CB
#define GL_MATRIX12_ARB 0x88CC
#define GL_MATRIX13_ARB 0x88CD
#define GL_MATRIX14_ARB 0x88CE
#define GL_MATRIX15_ARB 0x88CF
#define GL_MATRIX16_ARB 0x88D0
#define GL_MATRIX17_ARB 0x88D1
#define GL_MATRIX18_ARB 0x88D2
#define GL_MATRIX19_ARB 0x88D3
#define GL_MATRIX20_ARB 0x88D4
#define GL_MATRIX21_ARB 0x88D5
#define GL_MATRIX22_ARB 0x88D6
#define GL_MATRIX23_ARB 0x88D7
#define GL_MATRIX24_ARB 0x88D8
#define GL_MATRIX25_ARB 0x88D9
#define GL_MATRIX26_ARB 0x88DA
#define GL_MATRIX27_ARB 0x88DB
#define GL_MATRIX28_ARB 0x88DC
#define GL_MATRIX29_ARB 0x88DD
#define GL_MATRIX30_ARB 0x88DE
#define GL_MATRIX31_ARB 0x88DF

typedef void (GLAPIENTRY * PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (GLAPIENTRY * PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint* programs);
typedef void (GLAPIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (GLAPIENTRY * PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (GLAPIENTRY * PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint* programs);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMENVPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble* params);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMENVPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble* params);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMSTRINGARBPROC) (GLenum target, GLenum pname, void* string);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint* params);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index, GLenum pname, GLvoid** pointer);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBDVARBPROC) (GLuint index, GLenum pname, GLdouble* params);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBIVARBPROC) (GLuint index, GLenum pname, GLint* params);
typedef GLboolean (GLAPIENTRY * PFNGLISPROGRAMARBPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLPROGRAMENVPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble* params);
typedef void (GLAPIENTRY * PFNGLPROGRAMENVPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat* params);
typedef void (GLAPIENTRY * PFNGLPROGRAMLOCALPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLPROGRAMLOCALPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble* params);
typedef void (GLAPIENTRY * PFNGLPROGRAMLOCALPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat* params);
typedef void (GLAPIENTRY * PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const void* string);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DVARBPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FVARBPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SVARBPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DVARBPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FVARBPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x, GLshort y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SVARBPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DVARBPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SVARBPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NBVARBPROC) (GLuint index, const GLbyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NIVARBPROC) (GLuint index, const GLint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NSVARBPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index, const GLubyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index, const GLuint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index, const GLushort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4BVARBPROC) (GLuint index, const GLbyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DVARBPROC) (GLuint index, const GLdouble* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FVARBPROC) (GLuint index, const GLfloat* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4IVARBPROC) (GLuint index, const GLint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SVARBPROC) (GLuint index, const GLshort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UBVARBPROC) (GLuint index, const GLubyte* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UIVARBPROC) (GLuint index, const GLuint* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4USVARBPROC) (GLuint index, const GLushort* v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

#define glBindProgramARB GLEW_GET_FUN(__glewBindProgramARB)
#define glDeleteProgramsARB GLEW_GET_FUN(__glewDeleteProgramsARB)
#define glDisableVertexAttribArrayARB GLEW_GET_FUN(__glewDisableVertexAttribArrayARB)
#define glEnableVertexAttribArrayARB GLEW_GET_FUN(__glewEnableVertexAttribArrayARB)
#define glGenProgramsARB GLEW_GET_FUN(__glewGenProgramsARB)
#define glGetProgramEnvParameterdvARB GLEW_GET_FUN(__glewGetProgramEnvParameterdvARB)
#define glGetProgramEnvParameterfvARB GLEW_GET_FUN(__glewGetProgramEnvParameterfvARB)
#define glGetProgramLocalParameterdvARB GLEW_GET_FUN(__glewGetProgramLocalParameterdvARB)
#define glGetProgramLocalParameterfvARB GLEW_GET_FUN(__glewGetProgramLocalParameterfvARB)
#define glGetProgramStringARB GLEW_GET_FUN(__glewGetProgramStringARB)
#define glGetProgramivARB GLEW_GET_FUN(__glewGetProgramivARB)
#define glGetVertexAttribPointervARB GLEW_GET_FUN(__glewGetVertexAttribPointervARB)
#define glGetVertexAttribdvARB GLEW_GET_FUN(__glewGetVertexAttribdvARB)
#define glGetVertexAttribfvARB GLEW_GET_FUN(__glewGetVertexAttribfvARB)
#define glGetVertexAttribivARB GLEW_GET_FUN(__glewGetVertexAttribivARB)
#define glIsProgramARB GLEW_GET_FUN(__glewIsProgramARB)
#define glProgramEnvParameter4dARB GLEW_GET_FUN(__glewProgramEnvParameter4dARB)
#define glProgramEnvParameter4dvARB GLEW_GET_FUN(__glewProgramEnvParameter4dvARB)
#define glProgramEnvParameter4fARB GLEW_GET_FUN(__glewProgramEnvParameter4fARB)
#define glProgramEnvParameter4fvARB GLEW_GET_FUN(__glewProgramEnvParameter4fvARB)
#define glProgramLocalParameter4dARB GLEW_GET_FUN(__glewProgramLocalParameter4dARB)
#define glProgramLocalParameter4dvARB GLEW_GET_FUN(__glewProgramLocalParameter4dvARB)
#define glProgramLocalParameter4fARB GLEW_GET_FUN(__glewProgramLocalParameter4fARB)
#define glProgramLocalParameter4fvARB GLEW_GET_FUN(__glewProgramLocalParameter4fvARB)
#define glProgramStringARB GLEW_GET_FUN(__glewProgramStringARB)
#define glVertexAttrib1dARB GLEW_GET_FUN(__glewVertexAttrib1dARB)
#define glVertexAttrib1dvARB GLEW_GET_FUN(__glewVertexAttrib1dvARB)
#define glVertexAttrib1fARB GLEW_GET_FUN(__glewVertexAttrib1fARB)
#define glVertexAttrib1fvARB GLEW_GET_FUN(__glewVertexAttrib1fvARB)
#define glVertexAttrib1sARB GLEW_GET_FUN(__glewVertexAttrib1sARB)
#define glVertexAttrib1svARB GLEW_GET_FUN(__glewVertexAttrib1svARB)
#define glVertexAttrib2dARB GLEW_GET_FUN(__glewVertexAttrib2dARB)
#define glVertexAttrib2dvARB GLEW_GET_FUN(__glewVertexAttrib2dvARB)
#define glVertexAttrib2fARB GLEW_GET_FUN(__glewVertexAttrib2fARB)
#define glVertexAttrib2fvARB GLEW_GET_FUN(__glewVertexAttrib2fvARB)
#define glVertexAttrib2sARB GLEW_GET_FUN(__glewVertexAttrib2sARB)
#define glVertexAttrib2svARB GLEW_GET_FUN(__glewVertexAttrib2svARB)
#define glVertexAttrib3dARB GLEW_GET_FUN(__glewVertexAttrib3dARB)
#define glVertexAttrib3dvARB GLEW_GET_FUN(__glewVertexAttrib3dvARB)
#define glVertexAttrib3fARB GLEW_GET_FUN(__glewVertexAttrib3fARB)
#define glVertexAttrib3fvARB GLEW_GET_FUN(__glewVertexAttrib3fvARB)
#define glVertexAttrib3sARB GLEW_GET_FUN(__glewVertexAttrib3sARB)
#define glVertexAttrib3svARB GLEW_GET_FUN(__glewVertexAttrib3svARB)
#define glVertexAttrib4NbvARB GLEW_GET_FUN(__glewVertexAttrib4NbvARB)
#define glVertexAttrib4NivARB GLEW_GET_FUN(__glewVertexAttrib4NivARB)
#define glVertexAttrib4NsvARB GLEW_GET_FUN(__glewVertexAttrib4NsvARB)
#define glVertexAttrib4NubARB GLEW_GET_FUN(__glewVertexAttrib4NubARB)
#define glVertexAttrib4NubvARB GLEW_GET_FUN(__glewVertexAttrib4NubvARB)
#define glVertexAttrib4NuivARB GLEW_GET_FUN(__glewVertexAttrib4NuivARB)
#define glVertexAttrib4NusvARB GLEW_GET_FUN(__glewVertexAttrib4NusvARB)
#define glVertexAttrib4bvARB GLEW_GET_FUN(__glewVertexAttrib4bvARB)
#define glVertexAttrib4dARB GLEW_GET_FUN(__glewVertexAttrib4dARB)
#define glVertexAttrib4dvARB GLEW_GET_FUN(__glewVertexAttrib4dvARB)
#define glVertexAttrib4fARB GLEW_GET_FUN(__glewVertexAttrib4fARB)
#define glVertexAttrib4fvARB GLEW_GET_FUN(__glewVertexAttrib4fvARB)
#define glVertexAttrib4ivARB GLEW_GET_FUN(__glewVertexAttrib4ivARB)
#define glVertexAttrib4sARB GLEW_GET_FUN(__glewVertexAttrib4sARB)
#define glVertexAttrib4svARB GLEW_GET_FUN(__glewVertexAttrib4svARB)
#define glVertexAttrib4ubvARB GLEW_GET_FUN(__glewVertexAttrib4ubvARB)
#define glVertexAttrib4uivARB GLEW_GET_FUN(__glewVertexAttrib4uivARB)
#define glVertexAttrib4usvARB GLEW_GET_FUN(__glewVertexAttrib4usvARB)
#define glVertexAttribPointerARB GLEW_GET_FUN(__glewVertexAttribPointerARB)

#define GLEW_ARB_vertex_program GLEW_GET_VAR(__GLEW_ARB_vertex_program)

#endif /* GL_ARB_vertex_program */

/* -------------------------- GL_ARB_vertex_shader ------------------------- */

#ifndef GL_ARB_vertex_shader
#define GL_ARB_vertex_shader 1

#define GL_VERTEX_SHADER_ARB 0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB 0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A

typedef void (GLAPIENTRY * PFNGLBINDATTRIBLOCATIONARBPROC) (GLhandleARB programObj, GLuint index, const GLcharARB* name);
typedef void (GLAPIENTRY * PFNGLGETACTIVEATTRIBARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint (GLAPIENTRY * PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB* name);

#define glBindAttribLocationARB GLEW_GET_FUN(__glewBindAttribLocationARB)
#define glGetActiveAttribARB GLEW_GET_FUN(__glewGetActiveAttribARB)
#define glGetAttribLocationARB GLEW_GET_FUN(__glewGetAttribLocationARB)

#define GLEW_ARB_vertex_shader GLEW_GET_VAR(__GLEW_ARB_vertex_shader)

#endif /* GL_ARB_vertex_shader */

/* ------------------------- GL_ATI_fragment_shader ------------------------ */

#ifndef GL_ATI_fragment_shader
#define GL_ATI_fragment_shader 1

#define GL_RED_BIT_ATI 0x00000001
#define GL_2X_BIT_ATI 0x00000001
#define GL_4X_BIT_ATI 0x00000002
#define GL_GREEN_BIT_ATI 0x00000002
#define GL_COMP_BIT_ATI 0x00000002
#define GL_BLUE_BIT_ATI 0x00000004
#define GL_8X_BIT_ATI 0x00000004
#define GL_NEGATE_BIT_ATI 0x00000004
#define GL_BIAS_BIT_ATI 0x00000008
#define GL_HALF_BIT_ATI 0x00000008
#define GL_QUARTER_BIT_ATI 0x00000010
#define GL_EIGHTH_BIT_ATI 0x00000020
#define GL_SATURATE_BIT_ATI 0x00000040
#define GL_FRAGMENT_SHADER_ATI 0x8920
#define GL_REG_0_ATI 0x8921
#define GL_REG_1_ATI 0x8922
#define GL_REG_2_ATI 0x8923
#define GL_REG_3_ATI 0x8924
#define GL_REG_4_ATI 0x8925
#define GL_REG_5_ATI 0x8926
#define GL_CON_0_ATI 0x8941
#define GL_CON_1_ATI 0x8942
#define GL_CON_2_ATI 0x8943
#define GL_CON_3_ATI 0x8944
#define GL_CON_4_ATI 0x8945
#define GL_CON_5_ATI 0x8946
#define GL_CON_6_ATI 0x8947
#define GL_CON_7_ATI 0x8948
#define GL_MOV_ATI 0x8961
#define GL_ADD_ATI 0x8963
#define GL_MUL_ATI 0x8964
#define GL_SUB_ATI 0x8965
#define GL_DOT3_ATI 0x8966
#define GL_DOT4_ATI 0x8967
#define GL_MAD_ATI 0x8968
#define GL_LERP_ATI 0x8969
#define GL_CND_ATI 0x896A
#define GL_CND0_ATI 0x896B
#define GL_DOT2_ADD_ATI 0x896C
#define GL_SECONDARY_INTERPOLATOR_ATI 0x896D
#define GL_NUM_FRAGMENT_REGISTERS_ATI 0x896E
#define GL_NUM_FRAGMENT_CONSTANTS_ATI 0x896F
#define GL_NUM_PASSES_ATI 0x8970
#define GL_NUM_INSTRUCTIONS_PER_PASS_ATI 0x8971
#define GL_NUM_INSTRUCTIONS_TOTAL_ATI 0x8972
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI 0x8973
#define GL_NUM_LOOPBACK_COMPONENTS_ATI 0x8974
#define GL_COLOR_ALPHA_PAIRING_ATI 0x8975
#define GL_SWIZZLE_STR_ATI 0x8976
#define GL_SWIZZLE_STQ_ATI 0x8977
#define GL_SWIZZLE_STR_DR_ATI 0x8978
#define GL_SWIZZLE_STQ_DQ_ATI 0x8979
#define GL_SWIZZLE_STRQ_ATI 0x897A
#define GL_SWIZZLE_STRQ_DQ_ATI 0x897B

typedef void (GLAPIENTRY * PFNGLALPHAFRAGMENTOP1ATIPROC) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef void (GLAPIENTRY * PFNGLALPHAFRAGMENTOP2ATIPROC) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef void (GLAPIENTRY * PFNGLALPHAFRAGMENTOP3ATIPROC) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef void (GLAPIENTRY * PFNGLBEGINFRAGMENTSHADERATIPROC) (void);
typedef void (GLAPIENTRY * PFNGLBINDFRAGMENTSHADERATIPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLCOLORFRAGMENTOP1ATIPROC) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef void (GLAPIENTRY * PFNGLCOLORFRAGMENTOP2ATIPROC) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef void (GLAPIENTRY * PFNGLCOLORFRAGMENTOP3ATIPROC) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef void (GLAPIENTRY * PFNGLDELETEFRAGMENTSHADERATIPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLENDFRAGMENTSHADERATIPROC) (void);
typedef GLuint (GLAPIENTRY * PFNGLGENFRAGMENTSHADERSATIPROC) (GLuint range);
typedef void (GLAPIENTRY * PFNGLPASSTEXCOORDATIPROC) (GLuint dst, GLuint coord, GLenum swizzle);
typedef void (GLAPIENTRY * PFNGLSAMPLEMAPATIPROC) (GLuint dst, GLuint interp, GLenum swizzle);
typedef void (GLAPIENTRY * PFNGLSETFRAGMENTSHADERCONSTANTATIPROC) (GLuint dst, const GLfloat* value);

#define glAlphaFragmentOp1ATI GLEW_GET_FUN(__glewAlphaFragmentOp1ATI)
#define glAlphaFragmentOp2ATI GLEW_GET_FUN(__glewAlphaFragmentOp2ATI)
#define glAlphaFragmentOp3ATI GLEW_GET_FUN(__glewAlphaFragmentOp3ATI)
#define glBeginFragmentShaderATI GLEW_GET_FUN(__glewBeginFragmentShaderATI)
#define glBindFragmentShaderATI GLEW_GET_FUN(__glewBindFragmentShaderATI)
#define glColorFragmentOp1ATI GLEW_GET_FUN(__glewColorFragmentOp1ATI)
#define glColorFragmentOp2ATI GLEW_GET_FUN(__glewColorFragmentOp2ATI)
#define glColorFragmentOp3ATI GLEW_GET_FUN(__glewColorFragmentOp3ATI)
#define glDeleteFragmentShaderATI GLEW_GET_FUN(__glewDeleteFragmentShaderATI)
#define glEndFragmentShaderATI GLEW_GET_FUN(__glewEndFragmentShaderATI)
#define glGenFragmentShadersATI GLEW_GET_FUN(__glewGenFragmentShadersATI)
#define glPassTexCoordATI GLEW_GET_FUN(__glewPassTexCoordATI)
#define glSampleMapATI GLEW_GET_FUN(__glewSampleMapATI)
#define glSetFragmentShaderConstantATI GLEW_GET_FUN(__glewSetFragmentShaderConstantATI)

#define GLEW_ATI_fragment_shader GLEW_GET_VAR(__GLEW_ATI_fragment_shader)

#endif /* GL_ATI_fragment_shader */

/* ---------------------- GL_ATI_text_fragment_shader ---------------------- */

#ifndef GL_ATI_text_fragment_shader
#define GL_ATI_text_fragment_shader 1

#define GL_TEXT_FRAGMENT_SHADER_ATI 0x8200

#define GLEW_ATI_text_fragment_shader GLEW_GET_VAR(__GLEW_ATI_text_fragment_shader)

#endif /* GL_ATI_text_fragment_shader */

/* ------------------------------ GL_EXT_bgra ------------------------------ */

#ifndef GL_EXT_bgra
#define GL_EXT_bgra 1

#define GL_BGR_EXT 0x80E0
#define GL_BGRA_EXT 0x80E1

#define GLEW_EXT_bgra GLEW_GET_VAR(__GLEW_EXT_bgra)

#endif /* GL_EXT_bgra */

/* -------------------------- GL_EXT_packed_pixels ------------------------- */

#ifndef GL_EXT_packed_pixels
#define GL_EXT_packed_pixels 1

#define GL_UNSIGNED_BYTE_3_3_2_EXT 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT 0x8034
#define GL_UNSIGNED_INT_8_8_8_8_EXT 0x8035
#define GL_UNSIGNED_INT_10_10_10_2_EXT 0x8036

#define GLEW_EXT_packed_pixels GLEW_GET_VAR(__GLEW_EXT_packed_pixels)

#endif /* GL_EXT_packed_pixels */

/* ------------------------ GL_EXT_texture_rectangle ----------------------- */

#ifndef GL_EXT_texture_rectangle
#define GL_EXT_texture_rectangle 1

#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_EXT 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_EXT 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT 0x84F8

#define GLEW_EXT_texture_rectangle GLEW_GET_VAR(__GLEW_EXT_texture_rectangle)

#endif /* GL_EXT_texture_rectangle */

/* -------------------------- GL_EXT_vertex_shader ------------------------- */

#ifndef GL_EXT_vertex_shader
#define GL_EXT_vertex_shader 1

#define GL_VERTEX_SHADER_EXT 0x8780
#define GL_VERTEX_SHADER_BINDING_EXT 0x8781
#define GL_OP_INDEX_EXT 0x8782
#define GL_OP_NEGATE_EXT 0x8783
#define GL_OP_DOT3_EXT 0x8784
#define GL_OP_DOT4_EXT 0x8785
#define GL_OP_MUL_EXT 0x8786
#define GL_OP_ADD_EXT 0x8787
#define GL_OP_MADD_EXT 0x8788
#define GL_OP_FRAC_EXT 0x8789
#define GL_OP_MAX_EXT 0x878A
#define GL_OP_MIN_EXT 0x878B
#define GL_OP_SET_GE_EXT 0x878C
#define GL_OP_SET_LT_EXT 0x878D
#define GL_OP_CLAMP_EXT 0x878E
#define GL_OP_FLOOR_EXT 0x878F
#define GL_OP_ROUND_EXT 0x8790
#define GL_OP_EXP_BASE_2_EXT 0x8791
#define GL_OP_LOG_BASE_2_EXT 0x8792
#define GL_OP_POWER_EXT 0x8793
#define GL_OP_RECIP_EXT 0x8794
#define GL_OP_RECIP_SQRT_EXT 0x8795
#define GL_OP_SUB_EXT 0x8796
#define GL_OP_CROSS_PRODUCT_EXT 0x8797
#define GL_OP_MULTIPLY_MATRIX_EXT 0x8798
#define GL_OP_MOV_EXT 0x8799
#define GL_OUTPUT_VERTEX_EXT 0x879A
#define GL_OUTPUT_COLOR0_EXT 0x879B
#define GL_OUTPUT_COLOR1_EXT 0x879C
#define GL_OUTPUT_TEXTURE_COORD0_EXT 0x879D
#define GL_OUTPUT_TEXTURE_COORD1_EXT 0x879E
#define GL_OUTPUT_TEXTURE_COORD2_EXT 0x879F
#define GL_OUTPUT_TEXTURE_COORD3_EXT 0x87A0
#define GL_OUTPUT_TEXTURE_COORD4_EXT 0x87A1
#define GL_OUTPUT_TEXTURE_COORD5_EXT 0x87A2
#define GL_OUTPUT_TEXTURE_COORD6_EXT 0x87A3
#define GL_OUTPUT_TEXTURE_COORD7_EXT 0x87A4
#define GL_OUTPUT_TEXTURE_COORD8_EXT 0x87A5
#define GL_OUTPUT_TEXTURE_COORD9_EXT 0x87A6
#define GL_OUTPUT_TEXTURE_COORD10_EXT 0x87A7
#define GL_OUTPUT_TEXTURE_COORD11_EXT 0x87A8
#define GL_OUTPUT_TEXTURE_COORD12_EXT 0x87A9
#define GL_OUTPUT_TEXTURE_COORD13_EXT 0x87AA
#define GL_OUTPUT_TEXTURE_COORD14_EXT 0x87AB
#define GL_OUTPUT_TEXTURE_COORD15_EXT 0x87AC
#define GL_OUTPUT_TEXTURE_COORD16_EXT 0x87AD
#define GL_OUTPUT_TEXTURE_COORD17_EXT 0x87AE
#define GL_OUTPUT_TEXTURE_COORD18_EXT 0x87AF
#define GL_OUTPUT_TEXTURE_COORD19_EXT 0x87B0
#define GL_OUTPUT_TEXTURE_COORD20_EXT 0x87B1
#define GL_OUTPUT_TEXTURE_COORD21_EXT 0x87B2
#define GL_OUTPUT_TEXTURE_COORD22_EXT 0x87B3
#define GL_OUTPUT_TEXTURE_COORD23_EXT 0x87B4
#define GL_OUTPUT_TEXTURE_COORD24_EXT 0x87B5
#define GL_OUTPUT_TEXTURE_COORD25_EXT 0x87B6
#define GL_OUTPUT_TEXTURE_COORD26_EXT 0x87B7
#define GL_OUTPUT_TEXTURE_COORD27_EXT 0x87B8
#define GL_OUTPUT_TEXTURE_COORD28_EXT 0x87B9
#define GL_OUTPUT_TEXTURE_COORD29_EXT 0x87BA
#define GL_OUTPUT_TEXTURE_COORD30_EXT 0x87BB
#define GL_OUTPUT_TEXTURE_COORD31_EXT 0x87BC
#define GL_OUTPUT_FOG_EXT 0x87BD
#define GL_SCALAR_EXT 0x87BE
#define GL_VECTOR_EXT 0x87BF
#define GL_MATRIX_EXT 0x87C0
#define GL_VARIANT_EXT 0x87C1
#define GL_INVARIANT_EXT 0x87C2
#define GL_LOCAL_CONSTANT_EXT 0x87C3
#define GL_LOCAL_EXT 0x87C4
#define GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87C5
#define GL_MAX_VERTEX_SHADER_VARIANTS_EXT 0x87C6
#define GL_MAX_VERTEX_SHADER_INVARIANTS_EXT 0x87C7
#define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87C8
#define GL_MAX_VERTEX_SHADER_LOCALS_EXT 0x87C9
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87CA
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT 0x87CB
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT 0x87CC
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87CD
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT 0x87CE
#define GL_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87CF
#define GL_VERTEX_SHADER_VARIANTS_EXT 0x87D0
#define GL_VERTEX_SHADER_INVARIANTS_EXT 0x87D1
#define GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87D2
#define GL_VERTEX_SHADER_LOCALS_EXT 0x87D3
#define GL_VERTEX_SHADER_OPTIMIZED_EXT 0x87D4
#define GL_X_EXT 0x87D5
#define GL_Y_EXT 0x87D6
#define GL_Z_EXT 0x87D7
#define GL_W_EXT 0x87D8
#define GL_NEGATIVE_X_EXT 0x87D9
#define GL_NEGATIVE_Y_EXT 0x87DA
#define GL_NEGATIVE_Z_EXT 0x87DB
#define GL_NEGATIVE_W_EXT 0x87DC
#define GL_ZERO_EXT 0x87DD
#define GL_ONE_EXT 0x87DE
#define GL_NEGATIVE_ONE_EXT 0x87DF
#define GL_NORMALIZED_RANGE_EXT 0x87E0
#define GL_FULL_RANGE_EXT 0x87E1
#define GL_CURRENT_VERTEX_EXT 0x87E2
#define GL_MVP_MATRIX_EXT 0x87E3
#define GL_VARIANT_VALUE_EXT 0x87E4
#define GL_VARIANT_DATATYPE_EXT 0x87E5
#define GL_VARIANT_ARRAY_STRIDE_EXT 0x87E6
#define GL_VARIANT_ARRAY_TYPE_EXT 0x87E7
#define GL_VARIANT_ARRAY_EXT 0x87E8
#define GL_VARIANT_ARRAY_POINTER_EXT 0x87E9
#define GL_INVARIANT_VALUE_EXT 0x87EA
#define GL_INVARIANT_DATATYPE_EXT 0x87EB
#define GL_LOCAL_CONSTANT_VALUE_EXT 0x87EC
#define GL_LOCAL_CONSTANT_DATATYPE_EXT 0x87ED

typedef void (GLAPIENTRY * PFNGLBEGINVERTEXSHADEREXTPROC) (void);
typedef GLuint (GLAPIENTRY * PFNGLBINDLIGHTPARAMETEREXTPROC) (GLenum light, GLenum value);
typedef GLuint (GLAPIENTRY * PFNGLBINDMATERIALPARAMETEREXTPROC) (GLenum face, GLenum value);
typedef GLuint (GLAPIENTRY * PFNGLBINDPARAMETEREXTPROC) (GLenum value);
typedef GLuint (GLAPIENTRY * PFNGLBINDTEXGENPARAMETEREXTPROC) (GLenum unit, GLenum coord, GLenum value);
typedef GLuint (GLAPIENTRY * PFNGLBINDTEXTUREUNITPARAMETEREXTPROC) (GLenum unit, GLenum value);
typedef void (GLAPIENTRY * PFNGLBINDVERTEXSHADEREXTPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLDELETEVERTEXSHADEREXTPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLENABLEVARIANTCLIENTSTATEEXTPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLENDVERTEXSHADEREXTPROC) (void);
typedef void (GLAPIENTRY * PFNGLEXTRACTCOMPONENTEXTPROC) (GLuint res, GLuint src, GLuint num);
typedef GLuint (GLAPIENTRY * PFNGLGENSYMBOLSEXTPROC) (GLenum dataType, GLenum storageType, GLenum range, GLuint components);
typedef GLuint (GLAPIENTRY * PFNGLGENVERTEXSHADERSEXTPROC) (GLuint range);
typedef void (GLAPIENTRY * PFNGLGETINVARIANTBOOLEANVEXTPROC) (GLuint id, GLenum value, GLboolean *data);
typedef void (GLAPIENTRY * PFNGLGETINVARIANTFLOATVEXTPROC) (GLuint id, GLenum value, GLfloat *data);
typedef void (GLAPIENTRY * PFNGLGETINVARIANTINTEGERVEXTPROC) (GLuint id, GLenum value, GLint *data);
typedef void (GLAPIENTRY * PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC) (GLuint id, GLenum value, GLboolean *data);
typedef void (GLAPIENTRY * PFNGLGETLOCALCONSTANTFLOATVEXTPROC) (GLuint id, GLenum value, GLfloat *data);
typedef void (GLAPIENTRY * PFNGLGETLOCALCONSTANTINTEGERVEXTPROC) (GLuint id, GLenum value, GLint *data);
typedef void (GLAPIENTRY * PFNGLGETVARIANTBOOLEANVEXTPROC) (GLuint id, GLenum value, GLboolean *data);
typedef void (GLAPIENTRY * PFNGLGETVARIANTFLOATVEXTPROC) (GLuint id, GLenum value, GLfloat *data);
typedef void (GLAPIENTRY * PFNGLGETVARIANTINTEGERVEXTPROC) (GLuint id, GLenum value, GLint *data);
typedef void (GLAPIENTRY * PFNGLGETVARIANTPOINTERVEXTPROC) (GLuint id, GLenum value, GLvoid **data);
typedef void (GLAPIENTRY * PFNGLINSERTCOMPONENTEXTPROC) (GLuint res, GLuint src, GLuint num);
typedef GLboolean (GLAPIENTRY * PFNGLISVARIANTENABLEDEXTPROC) (GLuint id, GLenum cap);
typedef void (GLAPIENTRY * PFNGLSETINVARIANTEXTPROC) (GLuint id, GLenum type, GLvoid *addr);
typedef void (GLAPIENTRY * PFNGLSETLOCALCONSTANTEXTPROC) (GLuint id, GLenum type, GLvoid *addr);
typedef void (GLAPIENTRY * PFNGLSHADEROP1EXTPROC) (GLenum op, GLuint res, GLuint arg1);
typedef void (GLAPIENTRY * PFNGLSHADEROP2EXTPROC) (GLenum op, GLuint res, GLuint arg1, GLuint arg2);
typedef void (GLAPIENTRY * PFNGLSHADEROP3EXTPROC) (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3);
typedef void (GLAPIENTRY * PFNGLSWIZZLEEXTPROC) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
typedef void (GLAPIENTRY * PFNGLVARIANTPOINTEREXTPROC) (GLuint id, GLenum type, GLuint stride, GLvoid *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTBVEXTPROC) (GLuint id, GLbyte *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTDVEXTPROC) (GLuint id, GLdouble *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTFVEXTPROC) (GLuint id, GLfloat *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTIVEXTPROC) (GLuint id, GLint *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTSVEXTPROC) (GLuint id, GLshort *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTUBVEXTPROC) (GLuint id, GLubyte *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTUIVEXTPROC) (GLuint id, GLuint *addr);
typedef void (GLAPIENTRY * PFNGLVARIANTUSVEXTPROC) (GLuint id, GLushort *addr);
typedef void (GLAPIENTRY * PFNGLWRITEMASKEXTPROC) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);

#define glBeginVertexShaderEXT GLEW_GET_FUN(__glewBeginVertexShaderEXT)
#define glBindLightParameterEXT GLEW_GET_FUN(__glewBindLightParameterEXT)
#define glBindMaterialParameterEXT GLEW_GET_FUN(__glewBindMaterialParameterEXT)
#define glBindParameterEXT GLEW_GET_FUN(__glewBindParameterEXT)
#define glBindTexGenParameterEXT GLEW_GET_FUN(__glewBindTexGenParameterEXT)
#define glBindTextureUnitParameterEXT GLEW_GET_FUN(__glewBindTextureUnitParameterEXT)
#define glBindVertexShaderEXT GLEW_GET_FUN(__glewBindVertexShaderEXT)
#define glDeleteVertexShaderEXT GLEW_GET_FUN(__glewDeleteVertexShaderEXT)
#define glDisableVariantClientStateEXT GLEW_GET_FUN(__glewDisableVariantClientStateEXT)
#define glEnableVariantClientStateEXT GLEW_GET_FUN(__glewEnableVariantClientStateEXT)
#define glEndVertexShaderEXT GLEW_GET_FUN(__glewEndVertexShaderEXT)
#define glExtractComponentEXT GLEW_GET_FUN(__glewExtractComponentEXT)
#define glGenSymbolsEXT GLEW_GET_FUN(__glewGenSymbolsEXT)
#define glGenVertexShadersEXT GLEW_GET_FUN(__glewGenVertexShadersEXT)
#define glGetInvariantBooleanvEXT GLEW_GET_FUN(__glewGetInvariantBooleanvEXT)
#define glGetInvariantFloatvEXT GLEW_GET_FUN(__glewGetInvariantFloatvEXT)
#define glGetInvariantIntegervEXT GLEW_GET_FUN(__glewGetInvariantIntegervEXT)
#define glGetLocalConstantBooleanvEXT GLEW_GET_FUN(__glewGetLocalConstantBooleanvEXT)
#define glGetLocalConstantFloatvEXT GLEW_GET_FUN(__glewGetLocalConstantFloatvEXT)
#define glGetLocalConstantIntegervEXT GLEW_GET_FUN(__glewGetLocalConstantIntegervEXT)
#define glGetVariantBooleanvEXT GLEW_GET_FUN(__glewGetVariantBooleanvEXT)
#define glGetVariantFloatvEXT GLEW_GET_FUN(__glewGetVariantFloatvEXT)
#define glGetVariantIntegervEXT GLEW_GET_FUN(__glewGetVariantIntegervEXT)
#define glGetVariantPointervEXT GLEW_GET_FUN(__glewGetVariantPointervEXT)
#define glInsertComponentEXT GLEW_GET_FUN(__glewInsertComponentEXT)
#define glIsVariantEnabledEXT GLEW_GET_FUN(__glewIsVariantEnabledEXT)
#define glSetInvariantEXT GLEW_GET_FUN(__glewSetInvariantEXT)
#define glSetLocalConstantEXT GLEW_GET_FUN(__glewSetLocalConstantEXT)
#define glShaderOp1EXT GLEW_GET_FUN(__glewShaderOp1EXT)
#define glShaderOp2EXT GLEW_GET_FUN(__glewShaderOp2EXT)
#define glShaderOp3EXT GLEW_GET_FUN(__glewShaderOp3EXT)
#define glSwizzleEXT GLEW_GET_FUN(__glewSwizzleEXT)
#define glVariantPointerEXT GLEW_GET_FUN(__glewVariantPointerEXT)
#define glVariantbvEXT GLEW_GET_FUN(__glewVariantbvEXT)
#define glVariantdvEXT GLEW_GET_FUN(__glewVariantdvEXT)
#define glVariantfvEXT GLEW_GET_FUN(__glewVariantfvEXT)
#define glVariantivEXT GLEW_GET_FUN(__glewVariantivEXT)
#define glVariantsvEXT GLEW_GET_FUN(__glewVariantsvEXT)
#define glVariantubvEXT GLEW_GET_FUN(__glewVariantubvEXT)
#define glVariantuivEXT GLEW_GET_FUN(__glewVariantuivEXT)
#define glVariantusvEXT GLEW_GET_FUN(__glewVariantusvEXT)
#define glWriteMaskEXT GLEW_GET_FUN(__glewWriteMaskEXT)

#define GLEW_EXT_vertex_shader GLEW_GET_VAR(__GLEW_EXT_vertex_shader)

#endif /* GL_EXT_vertex_shader */

/* ------------------------ GL_NV_texture_rectangle ------------------------ */

#ifndef GL_NV_texture_rectangle
#define GL_NV_texture_rectangle 1

#define GL_TEXTURE_RECTANGLE_NV 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV 0x84F8

#define GLEW_NV_texture_rectangle GLEW_GET_VAR(__GLEW_NV_texture_rectangle)

#endif /* GL_NV_texture_rectangle */

/* ------------------------------------------------------------------------- */

#if defined(GLEW_MX) && defined(_WIN32)
#define GLEW_FUN_EXPORT
#else
#define GLEW_FUN_EXPORT GLEWAPI
#endif /* GLEW_MX */

#if defined(GLEW_MX)
#define GLEW_VAR_EXPORT
#else
#define GLEW_VAR_EXPORT GLEWAPI
#endif /* GLEW_MX */

#if defined(GLEW_MX) && defined(_WIN32)
struct GLEWContextStruct
{
#endif /* GLEW_MX */

GLEW_FUN_EXPORT PFNGLCOPYTEXSUBIMAGE3DPROC __glewCopyTexSubImage3D;
GLEW_FUN_EXPORT PFNGLDRAWRANGEELEMENTSPROC __glewDrawRangeElements;
GLEW_FUN_EXPORT PFNGLTEXIMAGE3DPROC __glewTexImage3D;
GLEW_FUN_EXPORT PFNGLTEXSUBIMAGE3DPROC __glewTexSubImage3D;

GLEW_FUN_EXPORT PFNGLACTIVETEXTUREPROC __glewActiveTexture;
GLEW_FUN_EXPORT PFNGLCLIENTACTIVETEXTUREPROC __glewClientActiveTexture;
GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXIMAGE1DPROC __glewCompressedTexImage1D;
GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXIMAGE2DPROC __glewCompressedTexImage2D;
GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXIMAGE3DPROC __glewCompressedTexImage3D;
GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC __glewCompressedTexSubImage1D;
GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC __glewCompressedTexSubImage2D;
GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC __glewCompressedTexSubImage3D;
GLEW_FUN_EXPORT PFNGLGETCOMPRESSEDTEXIMAGEPROC __glewGetCompressedTexImage;
GLEW_FUN_EXPORT PFNGLLOADTRANSPOSEMATRIXDPROC __glewLoadTransposeMatrixd;
GLEW_FUN_EXPORT PFNGLLOADTRANSPOSEMATRIXFPROC __glewLoadTransposeMatrixf;
GLEW_FUN_EXPORT PFNGLMULTTRANSPOSEMATRIXDPROC __glewMultTransposeMatrixd;
GLEW_FUN_EXPORT PFNGLMULTTRANSPOSEMATRIXFPROC __glewMultTransposeMatrixf;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1DPROC __glewMultiTexCoord1d;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1DVPROC __glewMultiTexCoord1dv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1FPROC __glewMultiTexCoord1f;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1FVPROC __glewMultiTexCoord1fv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1IPROC __glewMultiTexCoord1i;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1IVPROC __glewMultiTexCoord1iv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1SPROC __glewMultiTexCoord1s;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1SVPROC __glewMultiTexCoord1sv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2DPROC __glewMultiTexCoord2d;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2DVPROC __glewMultiTexCoord2dv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2FPROC __glewMultiTexCoord2f;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2FVPROC __glewMultiTexCoord2fv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2IPROC __glewMultiTexCoord2i;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2IVPROC __glewMultiTexCoord2iv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2SPROC __glewMultiTexCoord2s;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2SVPROC __glewMultiTexCoord2sv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3DPROC __glewMultiTexCoord3d;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3DVPROC __glewMultiTexCoord3dv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3FPROC __glewMultiTexCoord3f;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3FVPROC __glewMultiTexCoord3fv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3IPROC __glewMultiTexCoord3i;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3IVPROC __glewMultiTexCoord3iv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3SPROC __glewMultiTexCoord3s;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3SVPROC __glewMultiTexCoord3sv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4DPROC __glewMultiTexCoord4d;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4DVPROC __glewMultiTexCoord4dv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4FPROC __glewMultiTexCoord4f;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4FVPROC __glewMultiTexCoord4fv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4IPROC __glewMultiTexCoord4i;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4IVPROC __glewMultiTexCoord4iv;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4SPROC __glewMultiTexCoord4s;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4SVPROC __glewMultiTexCoord4sv;
GLEW_FUN_EXPORT PFNGLSAMPLECOVERAGEPROC __glewSampleCoverage;

GLEW_FUN_EXPORT PFNGLBLENDCOLORPROC __glewBlendColor;
GLEW_FUN_EXPORT PFNGLBLENDEQUATIONPROC __glewBlendEquation;
GLEW_FUN_EXPORT PFNGLBLENDFUNCSEPARATEPROC __glewBlendFuncSeparate;
GLEW_FUN_EXPORT PFNGLFOGCOORDPOINTERPROC __glewFogCoordPointer;
GLEW_FUN_EXPORT PFNGLFOGCOORDDPROC __glewFogCoordd;
GLEW_FUN_EXPORT PFNGLFOGCOORDDVPROC __glewFogCoorddv;
GLEW_FUN_EXPORT PFNGLFOGCOORDFPROC __glewFogCoordf;
GLEW_FUN_EXPORT PFNGLFOGCOORDFVPROC __glewFogCoordfv;
GLEW_FUN_EXPORT PFNGLMULTIDRAWARRAYSPROC __glewMultiDrawArrays;
GLEW_FUN_EXPORT PFNGLMULTIDRAWELEMENTSPROC __glewMultiDrawElements;
GLEW_FUN_EXPORT PFNGLPOINTPARAMETERFPROC __glewPointParameterf;
GLEW_FUN_EXPORT PFNGLPOINTPARAMETERFVPROC __glewPointParameterfv;
GLEW_FUN_EXPORT PFNGLPOINTPARAMETERIPROC __glewPointParameteri;
GLEW_FUN_EXPORT PFNGLPOINTPARAMETERIVPROC __glewPointParameteriv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3BPROC __glewSecondaryColor3b;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3BVPROC __glewSecondaryColor3bv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3DPROC __glewSecondaryColor3d;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3DVPROC __glewSecondaryColor3dv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3FPROC __glewSecondaryColor3f;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3FVPROC __glewSecondaryColor3fv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3IPROC __glewSecondaryColor3i;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3IVPROC __glewSecondaryColor3iv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3SPROC __glewSecondaryColor3s;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3SVPROC __glewSecondaryColor3sv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3UBPROC __glewSecondaryColor3ub;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3UBVPROC __glewSecondaryColor3ubv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3UIPROC __glewSecondaryColor3ui;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3UIVPROC __glewSecondaryColor3uiv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3USPROC __glewSecondaryColor3us;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLOR3USVPROC __glewSecondaryColor3usv;
GLEW_FUN_EXPORT PFNGLSECONDARYCOLORPOINTERPROC __glewSecondaryColorPointer;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2DPROC __glewWindowPos2d;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2DVPROC __glewWindowPos2dv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2FPROC __glewWindowPos2f;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2FVPROC __glewWindowPos2fv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2IPROC __glewWindowPos2i;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2IVPROC __glewWindowPos2iv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2SPROC __glewWindowPos2s;
GLEW_FUN_EXPORT PFNGLWINDOWPOS2SVPROC __glewWindowPos2sv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3DPROC __glewWindowPos3d;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3DVPROC __glewWindowPos3dv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3FPROC __glewWindowPos3f;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3FVPROC __glewWindowPos3fv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3IPROC __glewWindowPos3i;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3IVPROC __glewWindowPos3iv;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3SPROC __glewWindowPos3s;
GLEW_FUN_EXPORT PFNGLWINDOWPOS3SVPROC __glewWindowPos3sv;

GLEW_FUN_EXPORT PFNGLBEGINQUERYPROC __glewBeginQuery;
GLEW_FUN_EXPORT PFNGLBINDBUFFERPROC __glewBindBuffer;
GLEW_FUN_EXPORT PFNGLBUFFERDATAPROC __glewBufferData;
GLEW_FUN_EXPORT PFNGLBUFFERSUBDATAPROC __glewBufferSubData;
GLEW_FUN_EXPORT PFNGLDELETEBUFFERSPROC __glewDeleteBuffers;
GLEW_FUN_EXPORT PFNGLDELETEQUERIESPROC __glewDeleteQueries;
GLEW_FUN_EXPORT PFNGLENDQUERYPROC __glewEndQuery;
GLEW_FUN_EXPORT PFNGLGENBUFFERSPROC __glewGenBuffers;
GLEW_FUN_EXPORT PFNGLGENQUERIESPROC __glewGenQueries;
GLEW_FUN_EXPORT PFNGLGETBUFFERPARAMETERIVPROC __glewGetBufferParameteriv;
GLEW_FUN_EXPORT PFNGLGETBUFFERPOINTERVPROC __glewGetBufferPointerv;
GLEW_FUN_EXPORT PFNGLGETBUFFERSUBDATAPROC __glewGetBufferSubData;
GLEW_FUN_EXPORT PFNGLGETQUERYOBJECTIVPROC __glewGetQueryObjectiv;
GLEW_FUN_EXPORT PFNGLGETQUERYOBJECTUIVPROC __glewGetQueryObjectuiv;
GLEW_FUN_EXPORT PFNGLGETQUERYIVPROC __glewGetQueryiv;
GLEW_FUN_EXPORT PFNGLISBUFFERPROC __glewIsBuffer;
GLEW_FUN_EXPORT PFNGLISQUERYPROC __glewIsQuery;
GLEW_FUN_EXPORT PFNGLMAPBUFFERPROC __glewMapBuffer;
GLEW_FUN_EXPORT PFNGLUNMAPBUFFERPROC __glewUnmapBuffer;

GLEW_FUN_EXPORT PFNGLATTACHSHADERPROC __glewAttachShader;
GLEW_FUN_EXPORT PFNGLBINDATTRIBLOCATIONPROC __glewBindAttribLocation;
GLEW_FUN_EXPORT PFNGLBLENDEQUATIONSEPARATEPROC __glewBlendEquationSeparate;
GLEW_FUN_EXPORT PFNGLCOMPILESHADERPROC __glewCompileShader;
GLEW_FUN_EXPORT PFNGLCREATEPROGRAMPROC __glewCreateProgram;
GLEW_FUN_EXPORT PFNGLCREATESHADERPROC __glewCreateShader;
GLEW_FUN_EXPORT PFNGLDELETEPROGRAMPROC __glewDeleteProgram;
GLEW_FUN_EXPORT PFNGLDELETESHADERPROC __glewDeleteShader;
GLEW_FUN_EXPORT PFNGLDETACHSHADERPROC __glewDetachShader;
GLEW_FUN_EXPORT PFNGLDISABLEVERTEXATTRIBARRAYPROC __glewDisableVertexAttribArray;
GLEW_FUN_EXPORT PFNGLDRAWBUFFERSPROC __glewDrawBuffers;
GLEW_FUN_EXPORT PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray;
GLEW_FUN_EXPORT PFNGLGETACTIVEATTRIBPROC __glewGetActiveAttrib;
GLEW_FUN_EXPORT PFNGLGETACTIVEUNIFORMPROC __glewGetActiveUniform;
GLEW_FUN_EXPORT PFNGLGETATTACHEDSHADERSPROC __glewGetAttachedShaders;
GLEW_FUN_EXPORT PFNGLGETATTRIBLOCATIONPROC __glewGetAttribLocation;
GLEW_FUN_EXPORT PFNGLGETPROGRAMINFOLOGPROC __glewGetProgramInfoLog;
GLEW_FUN_EXPORT PFNGLGETPROGRAMIVPROC __glewGetProgramiv;
GLEW_FUN_EXPORT PFNGLGETSHADERINFOLOGPROC __glewGetShaderInfoLog;
GLEW_FUN_EXPORT PFNGLGETSHADERSOURCEPROC __glewGetShaderSource;
GLEW_FUN_EXPORT PFNGLGETSHADERIVPROC __glewGetShaderiv;
GLEW_FUN_EXPORT PFNGLGETUNIFORMLOCATIONPROC __glewGetUniformLocation;
GLEW_FUN_EXPORT PFNGLGETUNIFORMFVPROC __glewGetUniformfv;
GLEW_FUN_EXPORT PFNGLGETUNIFORMIVPROC __glewGetUniformiv;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBPOINTERVPROC __glewGetVertexAttribPointerv;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBDVPROC __glewGetVertexAttribdv;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBFVPROC __glewGetVertexAttribfv;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBIVPROC __glewGetVertexAttribiv;
GLEW_FUN_EXPORT PFNGLISPROGRAMPROC __glewIsProgram;
GLEW_FUN_EXPORT PFNGLISSHADERPROC __glewIsShader;
GLEW_FUN_EXPORT PFNGLLINKPROGRAMPROC __glewLinkProgram;
GLEW_FUN_EXPORT PFNGLSHADERSOURCEPROC __glewShaderSource;
GLEW_FUN_EXPORT PFNGLSTENCILFUNCSEPARATEPROC __glewStencilFuncSeparate;
GLEW_FUN_EXPORT PFNGLSTENCILMASKSEPARATEPROC __glewStencilMaskSeparate;
GLEW_FUN_EXPORT PFNGLSTENCILOPSEPARATEPROC __glewStencilOpSeparate;
GLEW_FUN_EXPORT PFNGLUNIFORM1FPROC __glewUniform1f;
GLEW_FUN_EXPORT PFNGLUNIFORM1FVPROC __glewUniform1fv;
GLEW_FUN_EXPORT PFNGLUNIFORM1IPROC __glewUniform1i;
GLEW_FUN_EXPORT PFNGLUNIFORM1IVPROC __glewUniform1iv;
GLEW_FUN_EXPORT PFNGLUNIFORM2FPROC __glewUniform2f;
GLEW_FUN_EXPORT PFNGLUNIFORM2FVPROC __glewUniform2fv;
GLEW_FUN_EXPORT PFNGLUNIFORM2IPROC __glewUniform2i;
GLEW_FUN_EXPORT PFNGLUNIFORM2IVPROC __glewUniform2iv;
GLEW_FUN_EXPORT PFNGLUNIFORM3FPROC __glewUniform3f;
GLEW_FUN_EXPORT PFNGLUNIFORM3FVPROC __glewUniform3fv;
GLEW_FUN_EXPORT PFNGLUNIFORM3IPROC __glewUniform3i;
GLEW_FUN_EXPORT PFNGLUNIFORM3IVPROC __glewUniform3iv;
GLEW_FUN_EXPORT PFNGLUNIFORM4FPROC __glewUniform4f;
GLEW_FUN_EXPORT PFNGLUNIFORM4FVPROC __glewUniform4fv;
GLEW_FUN_EXPORT PFNGLUNIFORM4IPROC __glewUniform4i;
GLEW_FUN_EXPORT PFNGLUNIFORM4IVPROC __glewUniform4iv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX2FVPROC __glewUniformMatrix2fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX3FVPROC __glewUniformMatrix3fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX4FVPROC __glewUniformMatrix4fv;
GLEW_FUN_EXPORT PFNGLUSEPROGRAMPROC __glewUseProgram;
GLEW_FUN_EXPORT PFNGLVALIDATEPROGRAMPROC __glewValidateProgram;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1DPROC __glewVertexAttrib1d;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1DVPROC __glewVertexAttrib1dv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1FPROC __glewVertexAttrib1f;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1FVPROC __glewVertexAttrib1fv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1SPROC __glewVertexAttrib1s;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1SVPROC __glewVertexAttrib1sv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2DPROC __glewVertexAttrib2d;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2DVPROC __glewVertexAttrib2dv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2FPROC __glewVertexAttrib2f;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2FVPROC __glewVertexAttrib2fv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2SPROC __glewVertexAttrib2s;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2SVPROC __glewVertexAttrib2sv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3DPROC __glewVertexAttrib3d;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3DVPROC __glewVertexAttrib3dv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3FPROC __glewVertexAttrib3f;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3FVPROC __glewVertexAttrib3fv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3SPROC __glewVertexAttrib3s;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3SVPROC __glewVertexAttrib3sv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NBVPROC __glewVertexAttrib4Nbv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NIVPROC __glewVertexAttrib4Niv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NSVPROC __glewVertexAttrib4Nsv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUBPROC __glewVertexAttrib4Nub;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUBVPROC __glewVertexAttrib4Nubv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUIVPROC __glewVertexAttrib4Nuiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUSVPROC __glewVertexAttrib4Nusv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4BVPROC __glewVertexAttrib4bv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4DPROC __glewVertexAttrib4d;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4DVPROC __glewVertexAttrib4dv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4FPROC __glewVertexAttrib4f;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4FVPROC __glewVertexAttrib4fv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4IVPROC __glewVertexAttrib4iv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4SPROC __glewVertexAttrib4s;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4SVPROC __glewVertexAttrib4sv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4UBVPROC __glewVertexAttrib4ubv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4UIVPROC __glewVertexAttrib4uiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4USVPROC __glewVertexAttrib4usv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer;

GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX2X3FVPROC __glewUniformMatrix2x3fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX2X4FVPROC __glewUniformMatrix2x4fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX3X2FVPROC __glewUniformMatrix3x2fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX3X4FVPROC __glewUniformMatrix3x4fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX4X2FVPROC __glewUniformMatrix4x2fv;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX4X3FVPROC __glewUniformMatrix4x3fv;

GLEW_FUN_EXPORT PFNGLBEGINCONDITIONALRENDERPROC __glewBeginConditionalRender;
GLEW_FUN_EXPORT PFNGLBEGINTRANSFORMFEEDBACKPROC __glewBeginTransformFeedback;
GLEW_FUN_EXPORT PFNGLBINDFRAGDATALOCATIONPROC __glewBindFragDataLocation;
GLEW_FUN_EXPORT PFNGLCLAMPCOLORPROC __glewClampColor;
GLEW_FUN_EXPORT PFNGLCLEARBUFFERFIPROC __glewClearBufferfi;
GLEW_FUN_EXPORT PFNGLCLEARBUFFERFVPROC __glewClearBufferfv;
GLEW_FUN_EXPORT PFNGLCLEARBUFFERIVPROC __glewClearBufferiv;
GLEW_FUN_EXPORT PFNGLCLEARBUFFERUIVPROC __glewClearBufferuiv;
GLEW_FUN_EXPORT PFNGLCOLORMASKIPROC __glewColorMaski;
GLEW_FUN_EXPORT PFNGLDISABLEIPROC __glewDisablei;
GLEW_FUN_EXPORT PFNGLENABLEIPROC __glewEnablei;
GLEW_FUN_EXPORT PFNGLENDCONDITIONALRENDERPROC __glewEndConditionalRender;
GLEW_FUN_EXPORT PFNGLENDTRANSFORMFEEDBACKPROC __glewEndTransformFeedback;
GLEW_FUN_EXPORT PFNGLGETBOOLEANI_VPROC __glewGetBooleani_v;
GLEW_FUN_EXPORT PFNGLGETFRAGDATALOCATIONPROC __glewGetFragDataLocation;
GLEW_FUN_EXPORT PFNGLGETSTRINGIPROC __glewGetStringi;
GLEW_FUN_EXPORT PFNGLGETTEXPARAMETERIIVPROC __glewGetTexParameterIiv;
GLEW_FUN_EXPORT PFNGLGETTEXPARAMETERIUIVPROC __glewGetTexParameterIuiv;
GLEW_FUN_EXPORT PFNGLGETTRANSFORMFEEDBACKVARYINGPROC __glewGetTransformFeedbackVarying;
GLEW_FUN_EXPORT PFNGLGETUNIFORMUIVPROC __glewGetUniformuiv;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBIIVPROC __glewGetVertexAttribIiv;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBIUIVPROC __glewGetVertexAttribIuiv;
GLEW_FUN_EXPORT PFNGLISENABLEDIPROC __glewIsEnabledi;
GLEW_FUN_EXPORT PFNGLTEXPARAMETERIIVPROC __glewTexParameterIiv;
GLEW_FUN_EXPORT PFNGLTEXPARAMETERIUIVPROC __glewTexParameterIuiv;
GLEW_FUN_EXPORT PFNGLTRANSFORMFEEDBACKVARYINGSPROC __glewTransformFeedbackVaryings;
GLEW_FUN_EXPORT PFNGLUNIFORM1UIPROC __glewUniform1ui;
GLEW_FUN_EXPORT PFNGLUNIFORM1UIVPROC __glewUniform1uiv;
GLEW_FUN_EXPORT PFNGLUNIFORM2UIPROC __glewUniform2ui;
GLEW_FUN_EXPORT PFNGLUNIFORM2UIVPROC __glewUniform2uiv;
GLEW_FUN_EXPORT PFNGLUNIFORM3UIPROC __glewUniform3ui;
GLEW_FUN_EXPORT PFNGLUNIFORM3UIVPROC __glewUniform3uiv;
GLEW_FUN_EXPORT PFNGLUNIFORM4UIPROC __glewUniform4ui;
GLEW_FUN_EXPORT PFNGLUNIFORM4UIVPROC __glewUniform4uiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI1IPROC __glewVertexAttribI1i;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI1IVPROC __glewVertexAttribI1iv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI1UIPROC __glewVertexAttribI1ui;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI1UIVPROC __glewVertexAttribI1uiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI2IPROC __glewVertexAttribI2i;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI2IVPROC __glewVertexAttribI2iv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI2UIPROC __glewVertexAttribI2ui;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI2UIVPROC __glewVertexAttribI2uiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI3IPROC __glewVertexAttribI3i;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI3IVPROC __glewVertexAttribI3iv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI3UIPROC __glewVertexAttribI3ui;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI3UIVPROC __glewVertexAttribI3uiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4BVPROC __glewVertexAttribI4bv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4IPROC __glewVertexAttribI4i;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4IVPROC __glewVertexAttribI4iv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4SVPROC __glewVertexAttribI4sv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4UBVPROC __glewVertexAttribI4ubv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4UIPROC __glewVertexAttribI4ui;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4UIVPROC __glewVertexAttribI4uiv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBI4USVPROC __glewVertexAttribI4usv;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBIPOINTERPROC __glewVertexAttribIPointer;

GLEW_FUN_EXPORT PFNGLDRAWARRAYSINSTANCEDPROC __glewDrawArraysInstanced;
GLEW_FUN_EXPORT PFNGLDRAWELEMENTSINSTANCEDPROC __glewDrawElementsInstanced;
GLEW_FUN_EXPORT PFNGLPRIMITIVERESTARTINDEXPROC __glewPrimitiveRestartIndex;
GLEW_FUN_EXPORT PFNGLTEXBUFFERPROC __glewTexBuffer;

GLEW_FUN_EXPORT PFNGLFRAMEBUFFERTEXTUREPROC __glewFramebufferTexture;
GLEW_FUN_EXPORT PFNGLGETBUFFERPARAMETERI64VPROC __glewGetBufferParameteri64v;
GLEW_FUN_EXPORT PFNGLGETINTEGER64I_VPROC __glewGetInteger64i_v;

GLEW_FUN_EXPORT PFNGLVERTEXATTRIBDIVISORPROC __glewVertexAttribDivisor;

GLEW_FUN_EXPORT PFNGLBLENDEQUATIONSEPARATEIPROC __glewBlendEquationSeparatei;
GLEW_FUN_EXPORT PFNGLBLENDEQUATIONIPROC __glewBlendEquationi;
GLEW_FUN_EXPORT PFNGLBLENDFUNCSEPARATEIPROC __glewBlendFuncSeparatei;
GLEW_FUN_EXPORT PFNGLBLENDFUNCIPROC __glewBlendFunci;
GLEW_FUN_EXPORT PFNGLMINSAMPLESHADINGPROC __glewMinSampleShading;

GLEW_FUN_EXPORT PFNGLACTIVETEXTUREARBPROC __glewActiveTextureARB;
GLEW_FUN_EXPORT PFNGLCLIENTACTIVETEXTUREARBPROC __glewClientActiveTextureARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1DARBPROC __glewMultiTexCoord1dARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1DVARBPROC __glewMultiTexCoord1dvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1FARBPROC __glewMultiTexCoord1fARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1FVARBPROC __glewMultiTexCoord1fvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1IARBPROC __glewMultiTexCoord1iARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1IVARBPROC __glewMultiTexCoord1ivARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1SARBPROC __glewMultiTexCoord1sARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD1SVARBPROC __glewMultiTexCoord1svARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2DARBPROC __glewMultiTexCoord2dARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2DVARBPROC __glewMultiTexCoord2dvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2FARBPROC __glewMultiTexCoord2fARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2FVARBPROC __glewMultiTexCoord2fvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2IARBPROC __glewMultiTexCoord2iARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2IVARBPROC __glewMultiTexCoord2ivARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2SARBPROC __glewMultiTexCoord2sARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD2SVARBPROC __glewMultiTexCoord2svARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3DARBPROC __glewMultiTexCoord3dARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3DVARBPROC __glewMultiTexCoord3dvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3FARBPROC __glewMultiTexCoord3fARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3FVARBPROC __glewMultiTexCoord3fvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3IARBPROC __glewMultiTexCoord3iARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3IVARBPROC __glewMultiTexCoord3ivARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3SARBPROC __glewMultiTexCoord3sARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD3SVARBPROC __glewMultiTexCoord3svARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4DARBPROC __glewMultiTexCoord4dARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4DVARBPROC __glewMultiTexCoord4dvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4FARBPROC __glewMultiTexCoord4fARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4FVARBPROC __glewMultiTexCoord4fvARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4IARBPROC __glewMultiTexCoord4iARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4IVARBPROC __glewMultiTexCoord4ivARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4SARBPROC __glewMultiTexCoord4sARB;
GLEW_FUN_EXPORT PFNGLMULTITEXCOORD4SVARBPROC __glewMultiTexCoord4svARB;

GLEW_FUN_EXPORT PFNGLATTACHOBJECTARBPROC __glewAttachObjectARB;
GLEW_FUN_EXPORT PFNGLCOMPILESHADERARBPROC __glewCompileShaderARB;
GLEW_FUN_EXPORT PFNGLCREATEPROGRAMOBJECTARBPROC __glewCreateProgramObjectARB;
GLEW_FUN_EXPORT PFNGLCREATESHADEROBJECTARBPROC __glewCreateShaderObjectARB;
GLEW_FUN_EXPORT PFNGLDELETEOBJECTARBPROC __glewDeleteObjectARB;
GLEW_FUN_EXPORT PFNGLDETACHOBJECTARBPROC __glewDetachObjectARB;
GLEW_FUN_EXPORT PFNGLGETACTIVEUNIFORMARBPROC __glewGetActiveUniformARB;
GLEW_FUN_EXPORT PFNGLGETATTACHEDOBJECTSARBPROC __glewGetAttachedObjectsARB;
GLEW_FUN_EXPORT PFNGLGETHANDLEARBPROC __glewGetHandleARB;
GLEW_FUN_EXPORT PFNGLGETINFOLOGARBPROC __glewGetInfoLogARB;
GLEW_FUN_EXPORT PFNGLGETOBJECTPARAMETERFVARBPROC __glewGetObjectParameterfvARB;
GLEW_FUN_EXPORT PFNGLGETOBJECTPARAMETERIVARBPROC __glewGetObjectParameterivARB;
GLEW_FUN_EXPORT PFNGLGETSHADERSOURCEARBPROC __glewGetShaderSourceARB;
GLEW_FUN_EXPORT PFNGLGETUNIFORMLOCATIONARBPROC __glewGetUniformLocationARB;
GLEW_FUN_EXPORT PFNGLGETUNIFORMFVARBPROC __glewGetUniformfvARB;
GLEW_FUN_EXPORT PFNGLGETUNIFORMIVARBPROC __glewGetUniformivARB;
GLEW_FUN_EXPORT PFNGLLINKPROGRAMARBPROC __glewLinkProgramARB;
GLEW_FUN_EXPORT PFNGLSHADERSOURCEARBPROC __glewShaderSourceARB;
GLEW_FUN_EXPORT PFNGLUNIFORM1FARBPROC __glewUniform1fARB;
GLEW_FUN_EXPORT PFNGLUNIFORM1FVARBPROC __glewUniform1fvARB;
GLEW_FUN_EXPORT PFNGLUNIFORM1IARBPROC __glewUniform1iARB;
GLEW_FUN_EXPORT PFNGLUNIFORM1IVARBPROC __glewUniform1ivARB;
GLEW_FUN_EXPORT PFNGLUNIFORM2FARBPROC __glewUniform2fARB;
GLEW_FUN_EXPORT PFNGLUNIFORM2FVARBPROC __glewUniform2fvARB;
GLEW_FUN_EXPORT PFNGLUNIFORM2IARBPROC __glewUniform2iARB;
GLEW_FUN_EXPORT PFNGLUNIFORM2IVARBPROC __glewUniform2ivARB;
GLEW_FUN_EXPORT PFNGLUNIFORM3FARBPROC __glewUniform3fARB;
GLEW_FUN_EXPORT PFNGLUNIFORM3FVARBPROC __glewUniform3fvARB;
GLEW_FUN_EXPORT PFNGLUNIFORM3IARBPROC __glewUniform3iARB;
GLEW_FUN_EXPORT PFNGLUNIFORM3IVARBPROC __glewUniform3ivARB;
GLEW_FUN_EXPORT PFNGLUNIFORM4FARBPROC __glewUniform4fARB;
GLEW_FUN_EXPORT PFNGLUNIFORM4FVARBPROC __glewUniform4fvARB;
GLEW_FUN_EXPORT PFNGLUNIFORM4IARBPROC __glewUniform4iARB;
GLEW_FUN_EXPORT PFNGLUNIFORM4IVARBPROC __glewUniform4ivARB;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX2FVARBPROC __glewUniformMatrix2fvARB;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX3FVARBPROC __glewUniformMatrix3fvARB;
GLEW_FUN_EXPORT PFNGLUNIFORMMATRIX4FVARBPROC __glewUniformMatrix4fvARB;
GLEW_FUN_EXPORT PFNGLUSEPROGRAMOBJECTARBPROC __glewUseProgramObjectARB;
GLEW_FUN_EXPORT PFNGLVALIDATEPROGRAMARBPROC __glewValidateProgramARB;

GLEW_FUN_EXPORT PFNGLBINDPROGRAMARBPROC __glewBindProgramARB;
GLEW_FUN_EXPORT PFNGLDELETEPROGRAMSARBPROC __glewDeleteProgramsARB;
GLEW_FUN_EXPORT PFNGLDISABLEVERTEXATTRIBARRAYARBPROC __glewDisableVertexAttribArrayARB;
GLEW_FUN_EXPORT PFNGLENABLEVERTEXATTRIBARRAYARBPROC __glewEnableVertexAttribArrayARB;
GLEW_FUN_EXPORT PFNGLGENPROGRAMSARBPROC __glewGenProgramsARB;
GLEW_FUN_EXPORT PFNGLGETPROGRAMENVPARAMETERDVARBPROC __glewGetProgramEnvParameterdvARB;
GLEW_FUN_EXPORT PFNGLGETPROGRAMENVPARAMETERFVARBPROC __glewGetProgramEnvParameterfvARB;
GLEW_FUN_EXPORT PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC __glewGetProgramLocalParameterdvARB;
GLEW_FUN_EXPORT PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC __glewGetProgramLocalParameterfvARB;
GLEW_FUN_EXPORT PFNGLGETPROGRAMSTRINGARBPROC __glewGetProgramStringARB;
GLEW_FUN_EXPORT PFNGLGETPROGRAMIVARBPROC __glewGetProgramivARB;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBPOINTERVARBPROC __glewGetVertexAttribPointervARB;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBDVARBPROC __glewGetVertexAttribdvARB;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBFVARBPROC __glewGetVertexAttribfvARB;
GLEW_FUN_EXPORT PFNGLGETVERTEXATTRIBIVARBPROC __glewGetVertexAttribivARB;
GLEW_FUN_EXPORT PFNGLISPROGRAMARBPROC __glewIsProgramARB;
GLEW_FUN_EXPORT PFNGLPROGRAMENVPARAMETER4DARBPROC __glewProgramEnvParameter4dARB;
GLEW_FUN_EXPORT PFNGLPROGRAMENVPARAMETER4DVARBPROC __glewProgramEnvParameter4dvARB;
GLEW_FUN_EXPORT PFNGLPROGRAMENVPARAMETER4FARBPROC __glewProgramEnvParameter4fARB;
GLEW_FUN_EXPORT PFNGLPROGRAMENVPARAMETER4FVARBPROC __glewProgramEnvParameter4fvARB;
GLEW_FUN_EXPORT PFNGLPROGRAMLOCALPARAMETER4DARBPROC __glewProgramLocalParameter4dARB;
GLEW_FUN_EXPORT PFNGLPROGRAMLOCALPARAMETER4DVARBPROC __glewProgramLocalParameter4dvARB;
GLEW_FUN_EXPORT PFNGLPROGRAMLOCALPARAMETER4FARBPROC __glewProgramLocalParameter4fARB;
GLEW_FUN_EXPORT PFNGLPROGRAMLOCALPARAMETER4FVARBPROC __glewProgramLocalParameter4fvARB;
GLEW_FUN_EXPORT PFNGLPROGRAMSTRINGARBPROC __glewProgramStringARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1DARBPROC __glewVertexAttrib1dARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1DVARBPROC __glewVertexAttrib1dvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1FARBPROC __glewVertexAttrib1fARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1FVARBPROC __glewVertexAttrib1fvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1SARBPROC __glewVertexAttrib1sARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB1SVARBPROC __glewVertexAttrib1svARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2DARBPROC __glewVertexAttrib2dARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2DVARBPROC __glewVertexAttrib2dvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2FARBPROC __glewVertexAttrib2fARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2FVARBPROC __glewVertexAttrib2fvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2SARBPROC __glewVertexAttrib2sARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB2SVARBPROC __glewVertexAttrib2svARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3DARBPROC __glewVertexAttrib3dARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3DVARBPROC __glewVertexAttrib3dvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3FARBPROC __glewVertexAttrib3fARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3FVARBPROC __glewVertexAttrib3fvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3SARBPROC __glewVertexAttrib3sARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB3SVARBPROC __glewVertexAttrib3svARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NBVARBPROC __glewVertexAttrib4NbvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NIVARBPROC __glewVertexAttrib4NivARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NSVARBPROC __glewVertexAttrib4NsvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUBARBPROC __glewVertexAttrib4NubARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUBVARBPROC __glewVertexAttrib4NubvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUIVARBPROC __glewVertexAttrib4NuivARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4NUSVARBPROC __glewVertexAttrib4NusvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4BVARBPROC __glewVertexAttrib4bvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4DARBPROC __glewVertexAttrib4dARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4DVARBPROC __glewVertexAttrib4dvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4FARBPROC __glewVertexAttrib4fARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4FVARBPROC __glewVertexAttrib4fvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4IVARBPROC __glewVertexAttrib4ivARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4SARBPROC __glewVertexAttrib4sARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4SVARBPROC __glewVertexAttrib4svARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4UBVARBPROC __glewVertexAttrib4ubvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4UIVARBPROC __glewVertexAttrib4uivARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIB4USVARBPROC __glewVertexAttrib4usvARB;
GLEW_FUN_EXPORT PFNGLVERTEXATTRIBPOINTERARBPROC __glewVertexAttribPointerARB;

GLEW_FUN_EXPORT PFNGLBINDATTRIBLOCATIONARBPROC __glewBindAttribLocationARB;
GLEW_FUN_EXPORT PFNGLGETACTIVEATTRIBARBPROC __glewGetActiveAttribARB;
GLEW_FUN_EXPORT PFNGLGETATTRIBLOCATIONARBPROC __glewGetAttribLocationARB;

GLEW_FUN_EXPORT PFNGLALPHAFRAGMENTOP1ATIPROC __glewAlphaFragmentOp1ATI;
GLEW_FUN_EXPORT PFNGLALPHAFRAGMENTOP2ATIPROC __glewAlphaFragmentOp2ATI;
GLEW_FUN_EXPORT PFNGLALPHAFRAGMENTOP3ATIPROC __glewAlphaFragmentOp3ATI;
GLEW_FUN_EXPORT PFNGLBEGINFRAGMENTSHADERATIPROC __glewBeginFragmentShaderATI;
GLEW_FUN_EXPORT PFNGLBINDFRAGMENTSHADERATIPROC __glewBindFragmentShaderATI;
GLEW_FUN_EXPORT PFNGLCOLORFRAGMENTOP1ATIPROC __glewColorFragmentOp1ATI;
GLEW_FUN_EXPORT PFNGLCOLORFRAGMENTOP2ATIPROC __glewColorFragmentOp2ATI;
GLEW_FUN_EXPORT PFNGLCOLORFRAGMENTOP3ATIPROC __glewColorFragmentOp3ATI;
GLEW_FUN_EXPORT PFNGLDELETEFRAGMENTSHADERATIPROC __glewDeleteFragmentShaderATI;
GLEW_FUN_EXPORT PFNGLENDFRAGMENTSHADERATIPROC __glewEndFragmentShaderATI;
GLEW_FUN_EXPORT PFNGLGENFRAGMENTSHADERSATIPROC __glewGenFragmentShadersATI;
GLEW_FUN_EXPORT PFNGLPASSTEXCOORDATIPROC __glewPassTexCoordATI;
GLEW_FUN_EXPORT PFNGLSAMPLEMAPATIPROC __glewSampleMapATI;
GLEW_FUN_EXPORT PFNGLSETFRAGMENTSHADERCONSTANTATIPROC __glewSetFragmentShaderConstantATI;

GLEW_FUN_EXPORT PFNGLBEGINVERTEXSHADEREXTPROC __glewBeginVertexShaderEXT;
GLEW_FUN_EXPORT PFNGLBINDLIGHTPARAMETEREXTPROC __glewBindLightParameterEXT;
GLEW_FUN_EXPORT PFNGLBINDMATERIALPARAMETEREXTPROC __glewBindMaterialParameterEXT;
GLEW_FUN_EXPORT PFNGLBINDPARAMETEREXTPROC __glewBindParameterEXT;
GLEW_FUN_EXPORT PFNGLBINDTEXGENPARAMETEREXTPROC __glewBindTexGenParameterEXT;
GLEW_FUN_EXPORT PFNGLBINDTEXTUREUNITPARAMETEREXTPROC __glewBindTextureUnitParameterEXT;
GLEW_FUN_EXPORT PFNGLBINDVERTEXSHADEREXTPROC __glewBindVertexShaderEXT;
GLEW_FUN_EXPORT PFNGLDELETEVERTEXSHADEREXTPROC __glewDeleteVertexShaderEXT;
GLEW_FUN_EXPORT PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC __glewDisableVariantClientStateEXT;
GLEW_FUN_EXPORT PFNGLENABLEVARIANTCLIENTSTATEEXTPROC __glewEnableVariantClientStateEXT;
GLEW_FUN_EXPORT PFNGLENDVERTEXSHADEREXTPROC __glewEndVertexShaderEXT;
GLEW_FUN_EXPORT PFNGLEXTRACTCOMPONENTEXTPROC __glewExtractComponentEXT;
GLEW_FUN_EXPORT PFNGLGENSYMBOLSEXTPROC __glewGenSymbolsEXT;
GLEW_FUN_EXPORT PFNGLGENVERTEXSHADERSEXTPROC __glewGenVertexShadersEXT;
GLEW_FUN_EXPORT PFNGLGETINVARIANTBOOLEANVEXTPROC __glewGetInvariantBooleanvEXT;
GLEW_FUN_EXPORT PFNGLGETINVARIANTFLOATVEXTPROC __glewGetInvariantFloatvEXT;
GLEW_FUN_EXPORT PFNGLGETINVARIANTINTEGERVEXTPROC __glewGetInvariantIntegervEXT;
GLEW_FUN_EXPORT PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC __glewGetLocalConstantBooleanvEXT;
GLEW_FUN_EXPORT PFNGLGETLOCALCONSTANTFLOATVEXTPROC __glewGetLocalConstantFloatvEXT;
GLEW_FUN_EXPORT PFNGLGETLOCALCONSTANTINTEGERVEXTPROC __glewGetLocalConstantIntegervEXT;
GLEW_FUN_EXPORT PFNGLGETVARIANTBOOLEANVEXTPROC __glewGetVariantBooleanvEXT;
GLEW_FUN_EXPORT PFNGLGETVARIANTFLOATVEXTPROC __glewGetVariantFloatvEXT;
GLEW_FUN_EXPORT PFNGLGETVARIANTINTEGERVEXTPROC __glewGetVariantIntegervEXT;
GLEW_FUN_EXPORT PFNGLGETVARIANTPOINTERVEXTPROC __glewGetVariantPointervEXT;
GLEW_FUN_EXPORT PFNGLINSERTCOMPONENTEXTPROC __glewInsertComponentEXT;
GLEW_FUN_EXPORT PFNGLISVARIANTENABLEDEXTPROC __glewIsVariantEnabledEXT;
GLEW_FUN_EXPORT PFNGLSETINVARIANTEXTPROC __glewSetInvariantEXT;
GLEW_FUN_EXPORT PFNGLSETLOCALCONSTANTEXTPROC __glewSetLocalConstantEXT;
GLEW_FUN_EXPORT PFNGLSHADEROP1EXTPROC __glewShaderOp1EXT;
GLEW_FUN_EXPORT PFNGLSHADEROP2EXTPROC __glewShaderOp2EXT;
GLEW_FUN_EXPORT PFNGLSHADEROP3EXTPROC __glewShaderOp3EXT;
GLEW_FUN_EXPORT PFNGLSWIZZLEEXTPROC __glewSwizzleEXT;
GLEW_FUN_EXPORT PFNGLVARIANTPOINTEREXTPROC __glewVariantPointerEXT;
GLEW_FUN_EXPORT PFNGLVARIANTBVEXTPROC __glewVariantbvEXT;
GLEW_FUN_EXPORT PFNGLVARIANTDVEXTPROC __glewVariantdvEXT;
GLEW_FUN_EXPORT PFNGLVARIANTFVEXTPROC __glewVariantfvEXT;
GLEW_FUN_EXPORT PFNGLVARIANTIVEXTPROC __glewVariantivEXT;
GLEW_FUN_EXPORT PFNGLVARIANTSVEXTPROC __glewVariantsvEXT;
GLEW_FUN_EXPORT PFNGLVARIANTUBVEXTPROC __glewVariantubvEXT;
GLEW_FUN_EXPORT PFNGLVARIANTUIVEXTPROC __glewVariantuivEXT;
GLEW_FUN_EXPORT PFNGLVARIANTUSVEXTPROC __glewVariantusvEXT;
GLEW_FUN_EXPORT PFNGLWRITEMASKEXTPROC __glewWriteMaskEXT;

#if defined(GLEW_MX) && !defined(_WIN32)
struct GLEWContextStruct
{
#endif /* GLEW_MX */

GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_1_1;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_1_2;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_1_2_1;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_1_3;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_1_4;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_1_5;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_2_0;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_2_1;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_3_0;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_3_1;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_3_2;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_3_3;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_4_0;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_4_1;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_4_2;
GLEW_VAR_EXPORT GLboolean __GLEW_VERSION_4_3;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_fragment_program;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_fragment_shader;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_multitexture;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_shader_objects;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_shading_language_100;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_texture_rectangle;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_vertex_program;
GLEW_VAR_EXPORT GLboolean __GLEW_ARB_vertex_shader;
GLEW_VAR_EXPORT GLboolean __GLEW_ATI_fragment_shader;
GLEW_VAR_EXPORT GLboolean __GLEW_ATI_text_fragment_shader;
GLEW_VAR_EXPORT GLboolean __GLEW_EXT_bgra;
GLEW_VAR_EXPORT GLboolean __GLEW_EXT_packed_pixels;
GLEW_VAR_EXPORT GLboolean __GLEW_EXT_texture_rectangle;
GLEW_VAR_EXPORT GLboolean __GLEW_EXT_vertex_shader;
GLEW_VAR_EXPORT GLboolean __GLEW_NV_texture_rectangle;

#ifdef GLEW_MX
}; /* GLEWContextStruct */
#endif /* GLEW_MX */

/* ------------------------------------------------------------------------- */

/* error codes */
#define GLEW_OK 0
#define GLEW_NO_ERROR 0
#define GLEW_ERROR_NO_GL_VERSION 1  /* missing GL version */
#define GLEW_ERROR_GL_VERSION_10_ONLY 2  /* Need at least OpenGL 1.1 */
#define GLEW_ERROR_GLX_VERSION_11_ONLY 3  /* Need at least GLX 1.2 */

/* string codes */
#define GLEW_VERSION 1
#define GLEW_VERSION_MAJOR 2
#define GLEW_VERSION_MINOR 3
#define GLEW_VERSION_MICRO 4

/* API */
#ifdef GLEW_MX

typedef struct GLEWContextStruct GLEWContext;
GLEWAPI GLenum GLEWAPIENTRY glewContextInit (GLEWContext *ctx);
GLEWAPI GLboolean GLEWAPIENTRY glewContextIsSupported (const GLEWContext *ctx, const char *name);

#define glewInit() glewContextInit(glewGetContext())
#define glewIsSupported(x) glewContextIsSupported(glewGetContext(), x)
#define glewIsExtensionSupported(x) glewIsSupported(x)

#define GLEW_GET_VAR(x) (*(const GLboolean*)&(glewGetContext()->x))
#ifdef _WIN32
#  define GLEW_GET_FUN(x) glewGetContext()->x
#else
#  define GLEW_GET_FUN(x) x
#endif

#else /* GLEW_MX */

GLEWAPI GLenum GLEWAPIENTRY glewInit (void);
GLEWAPI GLboolean GLEWAPIENTRY glewIsSupported (const char *name);
#define glewIsExtensionSupported(x) glewIsSupported(x)

#define GLEW_GET_VAR(x) (*(const GLboolean*)&x)
#define GLEW_GET_FUN(x) x

#endif /* GLEW_MX */

GLEWAPI GLboolean glewExperimental;
GLEWAPI GLboolean GLEWAPIENTRY glewGetExtension (const char *name);
GLEWAPI const GLubyte * GLEWAPIENTRY glewGetErrorString (GLenum error);
GLEWAPI const GLubyte * GLEWAPIENTRY glewGetString (GLenum name);

#ifdef __cplusplus
}
#endif

#ifdef GLEW_APIENTRY_DEFINED
#undef GLEW_APIENTRY_DEFINED
#undef APIENTRY
#undef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifdef GLEW_CALLBACK_DEFINED
#undef GLEW_CALLBACK_DEFINED
#undef CALLBACK
#endif

#ifdef GLEW_WINGDIAPI_DEFINED
#undef GLEW_WINGDIAPI_DEFINED
#undef WINGDIAPI
#endif

#undef GLAPI
/* #undef GLEWAPI */

#endif /* __glew_h__ */
