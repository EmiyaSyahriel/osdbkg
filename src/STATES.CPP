////////////////////////
/// STATES.HPP - Static variable implementations
/// Created at 12/05/2022 06:58 AM by EmiyaSyahriel
////////////////////////

#include "OSDBKG.HPP"

const struct osdbkg_const_t consts = {
	0x584C474F,
	{
		// POSITION         NORMAL            UV
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
	},
	{ 0,1,2,3 }
};

const struct osdbkg_tags_t tags = {
	"OpenGL",
	"OGLX",
	"Entry",
	"glslcmpman",
	"mdl_loader"
};


float extern_M_PI = 3.141592653f;

struct osdbkg_state_t states;

const struct osdbkg_strings_t strings = {
	"Zlib decompression failed : %s %s:%i",
	"[%8x] %s | %s:%i",
	"Framebuffer size updated to %ix%i",
	"Me to me ga au shunkan..",
	{
		"POSITION", "NORMAL", "TEXCOORD","ALPHA",
		"_Transform","_Color","_ColorAdd","_MainTexture","_FramebufferTexture","_IOR","_Refractive","_ColorFog","_AlphaBlend"
	}
};