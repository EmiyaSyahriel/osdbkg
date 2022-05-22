////////////////////////
/// OGLX.HPP - OpenGL Extensions
/// Created at 12/05/2022 07:50 AM by EmiyaSyahriel
////////////////////////
#include "OSDBKG.HPP"
#include <vector>
#include <zlib.h>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

#define CPB(c, n, s) (((c) & (n)) << s)
#define ZERRCASE(c) case c : errname = #c; fail = true; break;
#define GLERRCASE(x) case x : errstr = #x ; break
#define CZL(e) if(osd_oglx_zlib_err_check(e, __FILE__, __LINE__)){ *out_size = -1; return nullptr; }
#define ZLIB_BUFSZ 1024
#define OGLX_SHCK_BUFSZ 1024
#define OGLX_SHCK_VALIDATE(i, is_prg) if(!osd_oglx_shader_check(i, is_prg)){ glDeleteShader(v);glDeleteShader(f);glDeleteProgram(p);return -1; }
#define MDLBUF osd_oglx_mdl_ldr_insert_buffer
#define B2F osd_oglx_mdl_b2f

// strcasecmp should be available on POSIX C (GNU), otherwise define it over non-standard stricmp
#ifndef __GNUC__
	#ifndef strcasecmp
		#define strcasecmp _stricmp
	#endif
#endif

bool osd_oglx_zlib_err_check(int err, const char* file, int line) {
	bool fail = false;
	const char* errname = "";
	switch (err) {
		ZERRCASE(Z_BUF_ERROR)
		ZERRCASE(Z_MEM_ERROR)
		ZERRCASE(Z_DATA_ERROR)
	}
	if (fail) {
		osd_log_E(tags.OGLX, strings.zlib_decompression_failed_fmt, errname, file, line);
	}
	return fail;
}

char* osd_oglx_decompress_tex(const char* source, size_t size, size_t* out_size) {
	std::vector<char> uncompressed;
	std::vector<char> unpaired;
	
	char a, b, c, a_, b_;
	int err;
	size_t i, max;
#pragma region INFLATE
	{
		z_stream d_stream;
		d_stream.zalloc = Z_NULL;
		d_stream.zfree = Z_NULL;
		d_stream.opaque = Z_NULL;
		d_stream.avail_in = 0;
		d_stream.next_in = Z_NULL;
		err = inflateInit(&d_stream); CZL(err);
		d_stream.avail_in = (uInt)size;
		d_stream.next_in = (Bytef*)source;
		for (;;) {
			Bytef dbuffer[ZLIB_BUFSZ] = {};
			d_stream.next_out = &dbuffer[0];
			d_stream.avail_out = ZLIB_BUFSZ;
			err = inflate(&d_stream, Z_NO_FLUSH);
			if (err == Z_STREAM_END) {
				uncompressed.insert(
					uncompressed.end(), &dbuffer[0],
					&dbuffer[ZLIB_BUFSZ - d_stream.avail_out]);
				break;
			}
			CZL(err);
			uncompressed.insert(
				uncompressed.end(), &dbuffer[0],
				&dbuffer[ZLIB_BUFSZ - d_stream.avail_out]);
		}
		err = inflateEnd(&d_stream);
		CZL(err);
	}
#pragma endregion Zlib xFlate decompression

#pragma region UNPAIR
	{
		max = uncompressed.size();
		for (i = 0; i < max; i++) {
			c = uncompressed[i];
			a_ = (c >> 4) & 0x0F;
			b_ = (c >> 0) & 0x0F;
			// Brighten
			a = CPB(a_, 0b1000, 4) | CPB(a_, 0b1000, 3) |
			    CPB(a_, 0b0100, 3) | CPB(a_, 0b0100, 2) |
			    CPB(a_, 0b0010, 2) | CPB(a_, 0b0010, 1) |
			    CPB(a_, 0b0001, 1) | CPB(a_, 0b0001, 0);
			b = CPB(b_, 0b1000, 4) | CPB(b_, 0b1000, 3) |
			    CPB(b_, 0b0100, 3) | CPB(b_, 0b0100, 2) |
			    CPB(b_, 0b0010, 2) | CPB(b_, 0b0010, 1) |
				CPB(b_, 0b0001, 1) | CPB(b_, 0b0001, 0);
			//a = a_;
			//b = b_;
			if (a_ != 0) 
				a_ = a_;
			unpaired.push_back(a);
			unpaired.push_back(b);
		}
	}
#pragma endregion 4 bit unpairing and expansion
	size_t rsize = unpaired.size() * sizeof(char);
	char* retval = new char[rsize];
	*out_size = rsize;
	retval = (char*)memcpy(retval, unpaired.data(), rsize);
	return retval;
}

void osd_oglx_check_gl_error(const char* file, int line) {
	if (states.disable_cgl) return;

	GLenum err=  glGetError();
	if (err != GL_NO_ERROR) {
		const char* errstr = "UNKNOWN";
		switch (err) {
			GLERRCASE(GL_INVALID_ENUM);
			GLERRCASE(GL_INVALID_VALUE);
			GLERRCASE(GL_INVALID_OPERATION);
			GLERRCASE(GL_STACK_OVERFLOW);
			GLERRCASE(GL_STACK_UNDERFLOW);
			GLERRCASE(GL_OUT_OF_MEMORY);
			GLERRCASE(GL_INVALID_FRAMEBUFFER_OPERATION);
		}
		osd_log_E(tags.OpenGL, strings.cgl_template, err, errstr, file, line);
	}
}

template <typename T>
T* osd_oglx_mdl_ldr_insert_buffer(const char* src, size_t *off, size_t count = 1) {
	size_t sz = sizeof(T) * count;
	memcpy(states.mdl_ldr_buf, src + (*off), sz);
	*off = (*off + sz);
	return (T*)states.mdl_ldr_buf;
}

inline float osd_oglx_mdl_b2f(uchar s) {
	int ss = s - 128;
	float r = ((float)ss / ((ss < 0) ? 128.0f : 127.0f));
	return r;
}

void osd_oglx_decompress_mdl(const char* source, mdl_data_t* model) {
	int header = ((int*)source)[0];
	if (header == consts.mdl_header_fourcc) { // 'OGLX' integer notation (int32_le)
		osd_log_D(tags.oglx_mdldr, "MDL Valid");
	}
	else {
		osd_log_W(tags.oglx_mdldr, "MDL FourCC is invalid (%8X, should be %8X), but well whatever no one cares anyway...", header, consts.mdl_header_fourcc);
	}
	int i = 0, j,k;
	size_t str_off = 4;
	short* cnt_dat = MDLBUF<short>(source, &str_off, 4);
	short pos_i = cnt_dat[0], nrm_i = cnt_dat[1], uvs_i = cnt_dat[2], obj_i = cnt_dat[3];
	osd_log_D(tags.oglx_mdldr, "MDL contains %i positions, %i normals, %i UVs, %i objects", pos_i, nrm_i, uvs_i, obj_i);
	for (i = 0; i < pos_i; i++) {
		uchar* vec3dat = MDLBUF<uchar>(source, &str_off, 3);
		uchar x = vec3dat[0],
			y = vec3dat[1],
			z = vec3dat[2];
		model->positions.push_back(glm::vec3(B2F(x), B2F(y), B2F(z)));
	}
	for (i = 0; i < nrm_i; i++) {
		uchar* vec3dat = MDLBUF<uchar>(source, &str_off, 3);
		uchar x = vec3dat[0],
			y = vec3dat[1],
			z = vec3dat[2];
		model->normal.push_back(glm::vec3(B2F(x), B2F(y), B2F(z)));
	}
	for (i = 0; i < uvs_i; i++) {
		uchar* vec3dat = MDLBUF<uchar>(source, &str_off, 2);
		uchar x = vec3dat[0],
			y = vec3dat[1];
		model->texcoord.push_back(glm::vec2(B2F(x), B2F(y)));
	}
	for (i = 0; i < obj_i; i++) {
		uchar* lens = MDLBUF<uchar>(source, &str_off, 2), 
			name_len = lens[0], face_len = lens[1];
		mdl_object_t obj;
		obj.name = new char[name_len + 1];
		obj.name[name_len] = 0; // terminate with null
		memcpy(obj.name, MDLBUF<char>(source, &str_off, name_len), name_len);
		for (j = 0; j < face_len; j++) {
			ushort* tri_dat = MDLBUF<ushort>(source, &str_off, 9);
			mdl_face_t face;
			for (k = 0; k < 3; k++) {
				face[k].v = tri_dat[(k * 3) + 0];
				face[k].vt = tri_dat[(k * 3) + 1];
				face[k].vn = tri_dat[(k * 3) + 2];
			}
			obj.faces.push_back(face);
		}
		model->objects.push_back(obj);
	}
}

void osd_vertattrib_main() {
	glVertexAttribPointer(states.shader_data.main.POSITION, 3, GL_FLOAT, GL_FALSE, sz(float, 8), (void*)sz(float, 0));
	glVertexAttribPointer(states.shader_data.main.NORMAL, 3, GL_FLOAT, GL_FALSE, sz(float, 8), (void*)sz(float, 3));
	glVertexAttribPointer(states.shader_data.main.TEXCOORD, 2, GL_FLOAT, GL_FALSE, sz(float, 8), (void*)sz(float, 6));
}

void osd_vertattrib_line() {
	glVertexAttribPointer(states.shader_data.line.POSITION, 3, GL_FLOAT, GL_FALSE, sz(float, 4), (void*)sz(float, 0));
	glVertexAttribPointer(states.shader_data.line.ALPHA, 1, GL_FLOAT, GL_FALSE, sz(float, 4), (void*)sz(float, 3));
}

char osd_oglx_shader_check_buf[OGLX_SHCK_BUFSZ];

bool osd_oglx_shader_check(GLuint cmp, bool is_program) {
	GLint v = GL_FALSE;
	const char* type = is_program ? "glexe" : "gldll";
	(is_program ? glGetProgramiv : glGetShaderiv)(cmp, is_program ? GL_LINK_STATUS : GL_COMPILE_STATUS, &v);
	if (v == GL_FALSE) {
		GLint len = 0;
		(is_program ? glGetProgramInfoLog : glGetShaderInfoLog)(cmp, OGLX_SHCK_BUFSZ, 0, osd_oglx_shader_check_buf);
		if (len > 0) {
			if(len < OGLX_SHCK_BUFSZ) osd_oglx_shader_check_buf[len] = 0;
			osd_log_E(tags.oglx_shcmp, "[%02X].%s : Error; %s", cmp, type, osd_oglx_shader_check_buf);
		}
	}
	else {
		osd_log_D(tags.oglx_shcmp, "[%02X].%s : %s Success", cmp, type, is_program ? "Linking" : "Compiling");
	}
	return true;
}

GLuint osd_oglx_compile_shader(const char* fs, const char* vs) {
	GLuint
		p = glCreateProgram(),
		v = glCreateShader(GL_VERTEX_SHADER),
		f = glCreateShader(GL_FRAGMENT_SHADER); CGL();
	GLint sz = -1;
	glShaderSource(v, 1, &vs, &sz); CGL();
	glShaderSource(f, 1, &fs, &sz); CGL();
	glCompileShader(v); CGL();
	glCompileShader(f); CGL();
	OGLX_SHCK_VALIDATE(v, false);
	OGLX_SHCK_VALIDATE(f, false);
	glAttachShader(p, v); CGL();
	glAttachShader(p, f); CGL();
	glLinkProgram(p); CGL();
	OGLX_SHCK_VALIDATE(p, true);
	return p;
}

void osd_oglx_assign_transform(GLint id, glm::mat4 matrix) {
	glUniformMatrix4fv(id, 1, GL_FALSE, &matrix[0][0]);
}

glm::mat4 osd_oglx_do_transform(glm::mat4 src, osdbkg_transform_action_t act) {
	switch(act.action) {
		case TRANSFORM::TRANSLATE: return glm::translate(src, glm::vec3(act.data[0], act.data[1], act.data[2]));
		case TRANSFORM::ROTATE: return glm::rotate(src, glm::radians(act.data[3]), glm::normalize(glm::vec3(act.data[0], act.data[1], act.data[2])));
		case TRANSFORM::SCALE_UNIFORM: return glm::scale(src, glm::vec3(act.data[0], act.data[0], act.data[0]));
		case TRANSFORM::SCALE: return glm::scale(src, glm::vec3(act.data[0], act.data[1], act.data[2]));
	}
	return src;
}

glm::mat4 osd_oglx_set_object_transform(GLint id, int trf_count, osdbkg_transform_action_t* acts) {
	float war = (float)states.screen.w / (float)states.screen.h;
	float har = (float)states.screen.h / (float)states.screen.w;

	glm::mat4 p = glm::perspective(glm::radians(glm::mix(40.0f, 70.0f, har)), war, 0.1f, 100.0f);

	glm::mat4 v = glm::lookAt(glm::vec3(1.0f, 0.0f, -10.0f), 
		glm::vec3(-2.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 m = glm::mat4(1.0f);
	for (int i = 0; i < trf_count; i++) {
		auto act = acts[i];
		m = osd_oglx_do_transform(m, acts[i]);
	}
	glm::mat4 retval = p * v * m;
	if (id != -1) {
		osd_oglx_assign_transform(id, retval);
	}
	return retval;
}

int osd_oglx_model_index(mdl_data_t* model, const char* name) {
	size_t sz = model->objects.size();
	for (int i = 0; i < sz; i++) {
		if (strcasecmp(model->objects[i].name, name) == 0) return i;
	}
	return -1;
}

void osd_oglx_swap_refract_buffer() {
#if ANDROID
#ifdef ANDROID_SWAP_BUFFERS
	bool is_1_active = states.textures.data.active_refract_texture == states.textures.data.refract_tex[0];
	GLuint to_write = is_1_active ? states.textures.data.refract_tex[0] : states.textures.data.refract_tex[1];
	GLuint to_read = is_1_active ? states.textures.data.refract_tex[1] : states.textures.data.refract_tex[0];

	GLint active_fb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &active_fb); CGL();
	if (active_fb == states.fbs.data.refract_fb) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, to_write, 0); CGL();
	}

	GLint active_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &active_program); CGL();
	if (active_program == states.shader.main) {
		glActiveTexture(GL_TEXTURE1); CGL();
		glBindTexture(GL_TEXTURE_2D, to_read); CGL();
		glUniform1i(states.shader_data.main._FramebufferTexture, 1); CGL();
	}
#else
	if(states.glexts.glTextureBarrier != nullptr){
		states.glexts.glTextureBarrier();
	}
#endif
#else
	glTextureBarrier();
#endif
}