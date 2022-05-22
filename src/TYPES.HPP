////////////////////////
/// TYPES.HPP - Type definitons
/// Created at 12/05/2022 06:32 AM by EmiyaSyahriel
////////////////////////

#pragma once
#ifndef OSDBKG_TYPES
#define OSDBKG_TYPES

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <stdarg.h>
#include <vector>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef void (*PFNGLTEXTUREBARRIER)(void);

struct mdl_face_idx_t { int v, vt, vn; };
typedef std::array<mdl_face_idx_t, 3> mdl_face_t;
struct mdl_object_t {
    char* name = nullptr;
    std::vector<mdl_face_t> faces;
};

struct mdl_data_t {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normal;
    std::vector<glm::vec2> texcoord;
    std::vector<mdl_object_t> objects;
};

struct osdbkg_const_t {
    int mdl_header_fourcc;
    float mdl_screen_plane_v[32];
    int mdl_screen_plane_i[6];
};

struct osdbkg_tags_t {
    const char* OpenGL;
    const char* OGLX;
    const char* entry;
    const char* oglx_shcmp;
    const char* oglx_mdldr;
};

struct osdbkg_strings_t {
    const char* zlib_decompression_failed_fmt;
    const char* cgl_template;
    const char* fb_size_update;
    const char* dummy_text;
    struct shader_names_t {
        const char* POSITION, * NORMAL, * TEXCOORD, * ALPHA;
        const char* _Transform, * _Color, * _ColorAdd, * _MainTexture,
            * _FramebufferTexture, * _IOR, * _Refractive, * _ColorFog, * _AlphaBlend;
    } shader;
};

struct osdbkg_state_t {
    double currentTime = 0.0;
    bool disable_cgl = false;
    struct screen_size_t { int w, h; bool update; } screen;
    mdl_data_t model;
    char mdl_ldr_buf[256];
    char wait_buffer[32];
    bool slow_fb_swap = false;
    union state_fbs_u {
        GLuint raw[1];
        struct state_fbs_t {
            GLuint refract_fb;
        } data;
    } fbs;
    union state_textures_u {
        GLuint raw[3];
        struct state_textures_t {
            GLuint main_texture;
            GLuint refract_tex[2];
            GLuint active_refract_texture;
        } data;
    } textures;
    struct state_shaders_t {
        GLuint main, line;
    } shader;
    union state_buffers_u {
        GLuint raw[10];
        struct state_buffers_t {
            GLuint i_screenrect, v_screenrect,
                i_lightrace, v_lightrace,
                i_lighthead, v_lighthead,
                i_crystal, v_crystal,
                i_backcyl, v_backcyl;
        } data;
    } buffers;
    struct shader_names_t {
        struct shader_main_names_t {
            GLint POSITION, NORMAL, TEXCOORD;
            GLint _Transform, _ColorAdd, _Color, _ColorFog, _MainTexture,
                _FramebufferTexture, _IOR, _Refractive, _AlphaBlend;
        } main;
        struct shader_line_names_t {
            GLint POSITION, ALPHA;
            GLint _Transform, _Color;
        } line;
    } shader_data;
    struct glexts_t{
        PFNGLTEXTUREBARRIER glTextureBarrier;
    } glexts;
};

enum class TRANSFORM {
    TRANSLATE,
    ROTATE,
    SCALE_UNIFORM,
    SCALE
};

struct osdbkg_transform_action_t {
    TRANSFORM action;
    float data[4];
};

struct line_data_t {
    float x, y, z;
    float alpha;
};

struct vtx_data_t {
    float x, y, z;
    float m, n, o;
    float u, v;
};

struct tri_data_t {
    int a, b, c;
};

#endif