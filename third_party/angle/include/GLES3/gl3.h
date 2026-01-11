#ifndef __gl3_h_
#define __gl3_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "../KHR/khrplatform.h"

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

// OpenGL ES 3.0 constants
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_FALSE                          0
#define GL_TRUE                           1

#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408

#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

#define GL_CW                             0x0900
#define GL_CCW                            0x0901

#define GL_CULL_FACE                      0x0B44
#define GL_DEPTH_TEST                     0x0B71
#define GL_STENCIL_TEST                   0x0B90
#define GL_DITHER                         0x0BD0
#define GL_BLEND                          0x0BE2
#define GL_SCISSOR_TEST                   0x0C11

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

#define GL_TEXTURE_2D                     0x0DE1

#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_TEXTURE0                       0x84C0

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84

#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41

#define GL_RGBA                           0x1908
#define GL_RGB                            0x1907

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32F             0x8CAC

#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20

// Function declarations
void glActiveTexture(GLenum texture);
void glAttachShader(GLuint program, GLuint shader);
void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
void glBindBuffer(GLenum target, GLuint buffer);
void glBindFramebuffer(GLenum target, GLuint framebuffer);
void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
void glBindTexture(GLenum target, GLuint texture);
void glBindVertexArray(GLuint array);
void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glBlendEquation(GLenum mode);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GLenum glCheckFramebufferStatus(GLenum target);
void glClear(GLbitfield mask);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClearDepthf(GLfloat d);
void glClearStencil(GLint s);
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void glCompileShader(GLuint shader);
GLuint glCreateProgram(void);
GLuint glCreateShader(GLenum type);
void glCullFace(GLenum mode);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);
void glDeleteProgram(GLuint program);
void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
void glDeleteShader(GLuint shader);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
void glDepthFunc(GLenum func);
void glDepthMask(GLboolean flag);
void glDetachShader(GLuint program, GLuint shader);
void glDisable(GLenum cap);
void glDisableVertexAttribArray(GLuint index);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
void glEnable(GLenum cap);
void glEnableVertexAttribArray(GLuint index);
void glFinish(void);
void glFlush(void);
void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void glFrontFace(GLenum mode);
void glGenBuffers(GLsizei n, GLuint *buffers);
void glGenerateMipmap(GLenum target);
void glGenFramebuffers(GLsizei n, GLuint *framebuffers);
void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
void glGenTextures(GLsizei n, GLuint *textures);
void glGenVertexArrays(GLsizei n, GLuint *arrays);
void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GLint glGetAttribLocation(GLuint program, const GLchar *name);
void glGetBooleanv(GLenum pname, GLboolean *data);
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
GLenum glGetError(void);
void glGetFloatv(GLenum pname, GLfloat *data);
void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
void glGetIntegerv(GLenum pname, GLint *data);
void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
const GLubyte *glGetString(GLenum name);
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
void glGetUniformfv(GLuint program, GLint location, GLfloat *params);
void glGetUniformiv(GLuint program, GLint location, GLint *params);
GLint glGetUniformLocation(GLuint program, const GLchar *name);
void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
void glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer);
void glHint(GLenum target, GLenum mode);
GLboolean glIsBuffer(GLuint buffer);
GLboolean glIsEnabled(GLenum cap);
GLboolean glIsFramebuffer(GLuint framebuffer);
GLboolean glIsProgram(GLuint program);
GLboolean glIsRenderbuffer(GLuint renderbuffer);
GLboolean glIsShader(GLuint shader);
GLboolean glIsTexture(GLuint texture);
void glLineWidth(GLfloat width);
void glLinkProgram(GLuint program);
void glPixelStorei(GLenum pname, GLint param);
void glPolygonOffset(GLfloat factor, GLfloat units);
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void glSampleCoverage(GLfloat value, GLboolean invert);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
void glStencilFunc(GLenum func, GLint ref, GLuint mask);
void glStencilMask(GLuint mask);
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
void glUniform1f(GLint location, GLfloat v0);
void glUniform1i(GLint location, GLint v0);
void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
void glUniform2i(GLint location, GLint v0, GLint v1);
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
void glUniform1iv(GLint location, GLsizei count, const GLint *value);
void glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
void glUniform2iv(GLint location, GLsizei count, const GLint *value);
void glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
void glUniform3iv(GLint location, GLsizei count, const GLint *value);
void glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
void glUniform4iv(GLint location, GLsizei count, const GLint *value);
void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUseProgram(GLuint program);
void glValidateProgram(GLuint program);
void glVertexAttrib1f(GLuint index, GLfloat x);
void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef __cplusplus
}
#endif

#endif
