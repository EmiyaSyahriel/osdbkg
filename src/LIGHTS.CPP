////////////////////////
/// LIGHTS.CPP - Light Trace Object Renderer
/// Created at 15/05/2022 05:53 AM by EmiyaSyahriel
////////////////////////


#include "OSDBKG.HPP"
#include <glm/ext/matrix_transform.hpp>
#define LIGHT_COUNT 7
#define LIGHT_TAIL_COUNT 32

struct _light_data_t{
	bool use_matrix_light_calc = false;
	struct light_t {
		line_data_t line_v[LIGHT_TAIL_COUNT];
	} light[LIGHT_COUNT];
} _light_data;

void osd_lights_init() {
}

glm::vec3 osd_lights_at(float t) {
	glm::vec3 retval;
	if (_light_data.use_matrix_light_calc) {
		osdbkg_transform_action_t transforms[3] = {
			{TRANSFORM::TRANSLATE, { 0.0f, 1.0f, 0.0f }},
			{TRANSFORM::ROTATE, { 0.0f, 0.0f, 1.0f, (t * 1.0f) * 30.0f }},
			{TRANSFORM::ROTATE, { 0.0f, 0.0f, 1.0f, (t * 1.2f) * 30.0f }},
		};
		glm::mat4 mtx = osd_oglx_set_object_transform(-1, 3, transforms);
		glm::vec4 v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) * mtx;
		
		retval = glm::vec3(v.x, v.y, v.z);
	}
	else {
		float tz = (sinf(states.currentTime * 1.50f) + 1.0f) / 2.0f;
		float tt = (sinf(states.currentTime * 0.25f) + 1.0f) / 2.0f;
		// tt *= tt;
		float x = glm::mix(sinf(t * 1.25f), cosf(t + tt), tz);
		float y = glm::mix(cosf(t * 1.00f), sinf(t + tt), tz);
		float z = glm::mix(sinf(t * 0.75f), cosf(t - tt), tt);
		retval.x = x;
		retval.y = y;
		retval.z = z;
	}
	return retval;
}

void osd_lights_draw() {
	osdbkg_transform_action_t transforms[2] = {
		{TRANSFORM::TRANSLATE, { 0.0f, 0.0f, 0.0f }},
		{TRANSFORM::SCALE_UNIFORM, { 1.5f }},
	};
	glUseProgram(states.shader.line); CGL();
	osd_oglx_set_object_transform(states.shader_data.line._Transform, 2, transforms);
	glUniform3f(states.shader_data.line._Color, 0.0f, 1.0f, 1.0f); CGL();

	bool hasAttrib = false;
	glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_lightrace); CGL();

	float time = states.currentTime;
	for (int i = 0; i < LIGHT_COUNT; i++) {
		for (int j = 0; j < LIGHT_TAIL_COUNT; j++) {
			if (j == LIGHT_TAIL_COUNT - 1) {
				float t = (float)(i / LIGHT_COUNT) * 0.12f;
				t += (float)(time * glm::mix(2.2f, 8.0f, (float)i  / LIGHT_COUNT));
				glm::vec3 pos = osd_lights_at((-t) * 0.5f);
				_light_data.light[i].line_v[j].x = pos.x;
				_light_data.light[i].line_v[j].y = pos.y;
				_light_data.light[i].line_v[j].z = pos.z;
			}
			else {
				_light_data.light[i].line_v[j] = _light_data.light[i].line_v[j + 1];
			}
			float a = (float)j / LIGHT_TAIL_COUNT;
			_light_data.light[i].line_v[j].alpha = a * 2.0f;
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(_light_data.light[i].line_v), &_light_data.light[i].line_v, GL_DYNAMIC_DRAW); CGL();
		if (!hasAttrib) {
			osd_vertattrib_line(); CGL();
			hasAttrib = true;
		}
		glDrawArrays(GL_LINE_STRIP, 0, LIGHT_TAIL_COUNT); CGL();
		osd_oglx_swap_refract_buffer();
	}

	glUseProgram(states.shader.main); CGL();
	osd_oglx_set_object_transform(states.shader_data.main._Transform, 2, transforms);
	glUniform4f(states.shader_data.main._Color, 0.0f, 0.5f, 1.0f, 1.0f); CGL();
	glUniform4f(states.shader_data.main._ColorAdd, 0.9f, 1.0f, 1.0f, 1.25f); CGL();
	glUniform1f(states.shader_data.main._IOR, 1.0f); CGL();
	glUniform1f(states.shader_data.main._Refractive, 0.0f); CGL();
	glUniform1f(states.shader_data.main._AlphaBlend, 2.0f);
	vtx_data_t head_f[LIGHT_COUNT * 8] = {};
	tri_data_t head_i[LIGHT_COUNT * 4] = {};
	for (int i = 0; i < LIGHT_COUNT; i++) {
		line_data_t pt = _light_data.light[i].line_v[LIGHT_TAIL_COUNT - 1];
		float insz  = 0.1f;
		float ousz = 0.6f;
		int d = i * 8, dd = i * 4;
		head_i[dd + 0] = { d + 0, d + 1, d + 2 };
		head_i[dd + 1] = { d + 2, d + 1, d + 3 };
		head_i[dd + 2] = { d + 4, d + 5, d + 6 };
		head_i[dd + 3] = { d + 6, d + 5, d + 7 };

		head_f[d + 0] = { pt.x + insz, pt.y - insz, pt.z, 0.0f, 0.0f, 1.0f, 0.000f, 1.0f - 0.125f };
		head_f[d + 1] = { pt.x - insz, pt.y - insz, pt.z, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f - 0.125f };
		head_f[d + 2] = { pt.x + insz, pt.y + insz, pt.z, 0.0f, 0.0f, 1.0f, 0.000f, 1.0f - 0.000f };
		head_f[d + 3] = { pt.x - insz, pt.y + insz, pt.z, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f - 0.000f };
		head_f[d + 4] = { pt.x + ousz, pt.y - ousz, pt.z, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f - 0.250f };
		head_f[d + 5] = { pt.x - ousz, pt.y - ousz, pt.z, 0.0f, 0.0f, 1.0f, 0.375f, 1.0f - 0.250f };
		head_f[d + 6] = { pt.x + ousz, pt.y + ousz, pt.z, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f - 0.000f };
		head_f[d + 7] = { pt.x - ousz, pt.y + ousz, pt.z, 0.0f, 0.0f, 1.0f, 0.375f, 1.0f - 0.000f };
	}
	glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_lighthead); CGL();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, states.buffers.data.i_lighthead); CGL();
	glBufferData(GL_ARRAY_BUFFER, sizeof(head_f), head_f, GL_DYNAMIC_DRAW); CGL();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(head_i), head_i, GL_DYNAMIC_DRAW); CGL();
	osd_vertattrib_main(); CGL();
	glDrawElements(GL_TRIANGLES, LIGHT_COUNT * 12, GL_UNSIGNED_INT, 0); CGL();
	osd_oglx_swap_refract_buffer();
}