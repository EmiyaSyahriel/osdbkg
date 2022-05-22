////////////////////////
/// ENTRY.CPP - Entries
/// Created at 12/05/2022 06:57 AM by EmiyaSyahriel
////////////////////////

#include "OSDBKG.HPP"

#ifdef ANDROID
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#endif

#include <glm/gtc/matrix_transform.hpp>

#define PUT_SHADER_ATTR(type, var) states.shader_data.type.var = glGetAttribLocation (states.shader.type, strings.shader.var)
#define PUT_SHADER_UNIF(type, var) states.shader_data.type.var = glGetUniformLocation(states.shader.type, strings.shader.var)

void osd_prepare_buffers() {
	glGenBuffers(sizeof(states.buffers.raw) / sizeof(states.buffers.raw[0]), states.buffers.raw);
	
	{
		glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_screenrect);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, states.buffers.data.i_screenrect);
		glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(float), consts.mdl_screen_plane_v, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(int), consts.mdl_screen_plane_i, GL_STATIC_DRAW);
	}
}

template <typename T>
void osd_checkload_extension_fn(const char* ext_name, const char* target_ext_name, const char* proc_name, T* func){
	if(strcmp(ext_name, target_ext_name) == 0){
		*func =
#if __unix__ || ANDROID
				(T) eglGetProcAddress(proc_name);
#elif WIN32
		(T)wglGetProcAddress(proc_name);
#endif
		;
		bool avail = func == nullptr;

		osd_log_D(tags.entry, R"_/(GL Extension "%s" (function "%s") is %s)_/",
				  target_ext_name, proc_name, avail ? "available" : "not available");
	}
}

void osd_prepare_extension(){
	int extsz = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extsz);
	// osd_log_D(tags.entry, "Device GL driver has %i extensions", extsz);
	for(int i  =0;i < extsz; i++){
		const char* extnm = (const char*)glGetStringi(GL_EXTENSIONS, i);
		// osd_log_D(tags.entry, "\t\t--%s", extnm);
#ifdef ANDROID
		osd_checkload_extension_fn(extnm, "GL_NV_texture_barrier", "glTextureBarrier", &states.glexts.glTextureBarrier);
#endif
	}
}

void osd_prepare_framebuffers() {
	glGenFramebuffers(1, states.fbs.raw); CGL();
	glBindFramebuffer(GL_FRAMEBUFFER, states.fbs.data.refract_fb); CGL();
	for (int i = 0; i < 2; i++) {
		states.textures.data.active_refract_texture = states.textures.data.refract_tex[0];
		glBindTexture(GL_TEXTURE_2D, states.textures.data.active_refract_texture); CGL();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, states.screen.w, states.screen.h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr); CGL();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CGL();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CGL();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); CGL();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); CGL();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, states.textures.data.active_refract_texture, 0); CGL();
	}
}

void osd_prepare_textures() {
	size_t r;
	glCreateTextures(GL_TEXTURE_2D, 3, states.textures.raw); CGL();
	char* d = osd_oglx_decompress_tex(R.teximage_tex, R.teximage_tex_sz, &r);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, states.textures.data.main_texture); CGL();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, d); CGL();
	glGenerateMipmap(GL_TEXTURE_2D); CGL();
	delete d;
}

void osd_prepare_shaders() {
    states.shader.main = osd_oglx_compile_shader(R.main_frag, R.main_vert);
    states.shader.line = osd_oglx_compile_shader(R.line_frag, R.line_vert);

	// Main Shader
	{
		PUT_SHADER_ATTR(main , POSITION);
		PUT_SHADER_ATTR(main , NORMAL);
		PUT_SHADER_ATTR(main , TEXCOORD);
		PUT_SHADER_UNIF(main , _Transform);
		PUT_SHADER_UNIF(main , _Color);
		PUT_SHADER_UNIF(main , _ColorAdd);
		PUT_SHADER_UNIF(main , _ColorFog);
		PUT_SHADER_UNIF(main , _MainTexture);
		PUT_SHADER_UNIF(main , _FramebufferTexture);
		PUT_SHADER_UNIF(main , _IOR);
		PUT_SHADER_UNIF(main , _Refractive);
		PUT_SHADER_UNIF(main , _AlphaBlend);
		glUseProgram(states.shader.main);
		glEnableVertexAttribArray(states.shader_data.main.POSITION);
		glEnableVertexAttribArray(states.shader_data.main.NORMAL);
		glEnableVertexAttribArray(states.shader_data.main.TEXCOORD);
		osd_vertattrib_main();
	}

	// Line Shader
	{
		PUT_SHADER_ATTR(line, POSITION);
		PUT_SHADER_ATTR(line, ALPHA);
		PUT_SHADER_UNIF(line, _Transform);
		PUT_SHADER_UNIF(line, _Color);
		glUseProgram(states.shader.line);
		glEnableVertexAttribArray(states.shader_data.line.POSITION);
		glEnableVertexAttribArray(states.shader_data.line.ALPHA);
		osd_vertattrib_line();
	}
}

void osd_prepare_models() {
	osd_oglx_decompress_mdl(R.models_obj, &states.model);
}

void osd_init(){
	osd_prepare_extension();
	osd_helper_init();
	osd_prepare_textures();
	osd_prepare_framebuffers();
	osd_prepare_buffers();
	osd_prepare_models();
	osd_prepare_shaders();
	osd_bg_init();
	osd_crystal_init();
	osd_lights_init();
	osd_log_D(tags.entry, "%s", "INIT END, SUCCESS OR NOT IDK");
}

void check_fb_size_update() {
	if (states.screen.update) {
		int len = sizeof(states.textures.data.refract_tex) / sizeof(states.textures.data.refract_tex[0]);
		for (int i = 0; i < len; i++) {
			glBindTexture(GL_TEXTURE_2D, states.textures.data.refract_tex[i]); CGL();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, states.screen.w, states.screen.h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr); CGL();
			osd_log_D(tags.entry, strings.fb_size_update, states.screen.w, states.screen.h);
		}
	}
}

void osd_draw_offscreen() {
	for (int i = 0; i < 2; i++) {
		osd_oglx_swap_refract_buffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); CGL();
		glClear(GL_COLOR_BUFFER_BIT); CGL();
	}
	osd_bg_draw();
	osd_crystal_draw(false);
	osd_lights_draw();
	osd_crystal_draw(true);
}

void osd_fb_to_screen() {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); CGL();
	glClear(GL_COLOR_BUFFER_BIT); CGL();
	glUseProgram(states.shader.main); CGL();
	glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_screenrect); CGL();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, states.buffers.data.i_screenrect); CGL();
	glUniform1f(states.shader_data.main._AlphaBlend, 0.0f);

	osd_oglx_swap_refract_buffer();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, states.textures.data.active_refract_texture);

	glUniform4f(states.shader_data.main._Color, 1.0f, 1.0f, 1.0f, 1.0f); CGL();
	glUniform4f(states.shader_data.main._ColorAdd, 0.0f, 0.0f, 0.0f, 0.0f); CGL();
	glUniform1f(states.shader_data.main._Refractive, 0.0f); CGL();
	osd_oglx_assign_transform(states.shader_data.main._Transform, glm::mat4(1.0f)); CGL();
	glUniform1i(states.shader_data.main._MainTexture, 0);
	glUniform1i(states.shader_data.main._FramebufferTexture, 1);
	osd_vertattrib_main();
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr); CGL();
	osd_oglx_swap_refract_buffer();
}

void osd_wait_fb_complete() {
	if (!states.slow_fb_swap) return; // Ignore if framebuffer swapping is not slow
	while (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		CGL();
		osd_log_D(tags.OpenGL, "Waiting for OpenGL Framebuffer to complete...");
		osd_dummy_things(); // Doing something while waiting 
	}
}

void osd_swap_fb(GLuint fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb); CGL();
	osd_wait_fb_complete();
	glViewport(0, 0, states.screen.w, states.screen.h); CGL();
}

void osd_draw(double deltaTime){
    states.currentTime += deltaTime;
	glEnable(GL_BLEND);
#ifndef ANDROID
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
#endif
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	check_fb_size_update(); CGL();
	osd_swap_fb(states.fbs.data.refract_fb);
	osd_draw_offscreen();
	osd_swap_fb(0);
	osd_fb_to_screen();
	osd_dummy_things();

	glDisable(GL_BLEND);
#ifndef ANDROID
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_MULTISAMPLE);
#endif
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_ONE, GL_ZERO);
}