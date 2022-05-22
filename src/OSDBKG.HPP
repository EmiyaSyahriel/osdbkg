////////////////////////
/// OSDBKG.HPP - Main Header
/// Created at 12/05/2022 06:32 AM by EmiyaSyahriel
////////////////////////

#pragma once
#ifndef OSDBKG_OSDBKG_HPP
#define OSDBKG_OSDBKG_HPP

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif

#include "res/R.HPP"
#include <stdarg.h>
#include <vector>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#pragma region DEFINITIONS
#define CGL() osd_oglx_check_gl_error(__FILE__, __LINE__)
#define sz(f, i) (sizeof(f) * (i))
#pragma endregion Definitions

#include "TYPES.HPP"

#pragma region STATEVARS
#pragma warning (push)
#pragma warning (disable : 0147)

extern const osdbkg_const_t consts ;
extern const osdbkg_tags_t tags;
extern const osdbkg_strings_t strings;
extern osdbkg_state_t states ;

#ifndef M_PI
extern float extern_M_PI;
#define M_PI extern_M_PI
#endif

#pragma warning (pop)
#pragma endregion State Variables Declarations

#pragma region METHODS

#pragma region HELPER
void osd_helper_init();
void osd_dummy_things();
#pragma endregion Helpers

#pragma region OGLX
void osd_oglx_check_gl_error(const char* file, int line);
void osd_oglx_swap_refract_buffer();
char* osd_oglx_decompress_tex(const char* source, size_t size, size_t* out_size);
void osd_oglx_decompress_mdl(const char* source, mdl_data_t *model);
GLuint osd_oglx_compile_shader(const char* fs, const char* vs);
void osd_oglx_assign_transform(GLint id, glm::mat4 matrix);
glm::mat4 osd_oglx_set_object_transform(GLint id, int trf_count, osdbkg_transform_action_t* acts);
void osd_vertattrib_main();
void osd_vertattrib_line();
int osd_oglx_model_index(mdl_data_t* model, const char* name);

#if ANDROID
#define glCreateTextures(t, n, ptr) glGenTextures(n, ptr)
#define GL_STACK_OVERFLOW 0x0
#define GL_STACK_UNDERFLOW 0x1
#endif

#pragma endregion OpenGL Helper Methods

#pragma region LOG
extern void osd_log_write(const char* t, const char* tag, const char* fmt, va_list va);
void osd_log_D(const char* tag, const char* fmt, ...);
void osd_log_E(const char* tag, const char* fmt, ...);
void osd_log_W(const char* tag, const char* fmt, ...);
void osd_log_I(const char* tag, const char* fmt, ...);
#pragma endregion Logging Utilities

#pragma region OSD_BG
void osd_bg_init();
void osd_bg_draw();
#pragma endregion Background

#pragma region OSD_CRYSTAL
void osd_crystal_init();
void osd_crystal_draw(bool fore);
#pragma endregion Clock Crystal

#pragma region OSD_LIGHTS
void osd_lights_init();
void osd_lights_draw();
#pragma endregion Little Lights

#pragma region ENTRY
void osd_init();
void osd_draw(double deltaTime);
#pragma endregion Entry Functions

#pragma endregion Method Declarations

#endif // OSDBKG_OSDBKG_HPP